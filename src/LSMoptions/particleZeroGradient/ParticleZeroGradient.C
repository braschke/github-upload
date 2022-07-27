/*---------------------------------------------------------------------------*\
      _________________________________________________________
     /                                                        /|
    /                                                        / |
   |--------------------------------------------------------|  |
   |        _    ____ ____  _____                           |  |
   |       / \  | __ ) ___||  ___|__   __ _ _ __ ___        |  |
   |      / _ \ |  _ \___ \| |_ / _ \ / _` | '_ ` _ \       |  |
   |     / ___ \| |_) |__) |  _| (_) | (_| | | | | | |      |  |
   |    /_/   \_\____/____/|_|  \___/ \__,_|_| |_| |_|      |  |
   |                                                        |  |
   |    Arbitrary  Body  Simulation    for    OpenFOAM      | /
   |________________________________________________________|/

-------------------------------------------------------------------------------

Author

    Markus Buerger
    Chair of Fluid Mechanics
    markus.buerger@uni-wuppertal.de

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "ParticleZeroGradient.H"
#include "fvMesh.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"

#include "zeroGradientFvPatchField.H" //Vora: need to declare in version 5.

#include "makros.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

//namespace Foam
//{
//    namespace fv
//    {
//        defineTypeNameAndDebug(ParticleZeroGradient<Type>, 0);
//        addToRunTimeSelectionTable
//        (
//            option,
//            particleZeroGradient,
//            dictionary
//        );
//    }
//
//}
//

namespace Foam
{
namespace fv
{
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
template<class Type>
ParticleZeroGradient<Type>::ParticleZeroGradient
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    cellSetOption(name, modelType, dict, mesh),
    rhoName_("none"),
    ibFacesPtr_(0),
    voidFracPtr_(0),
    compressible_(false)
{
    read(dict);
#if 1
    // eventually preload fields into registry
    voidFrac();
#endif
}

template<class Type>
ParticleZeroGradient<Type>::~ParticleZeroGradient()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
template<class Type>
void ParticleZeroGradient<Type>::constrain
(
    fvMatrix<Type>& eqn,
    const label
)
{
    Info << endl << "fvOption 'particleZeroGradient' constraining '"
         << eqn.psi().name() << "' ... " << flush;

    if( mesh().changing() )
      discardFaces();

    const Field<Type>&  psi   = eqn.psi().internalField();
    const List<ibFace>& faces = ibFaces();

    const labelList& owner     = mesh().owner();
    const labelList& neighbour = mesh().neighbour();

    labelList   whichCells(faces.size());
    Field<Type> values(faces.size());

    label count = 0;
    forAll(faces, faceI)
    {
        ibFace face = faces[faceI];

        label solidCell =  (face.second()) ? owner[face.first()] : neighbour[face.first()];
        label fluidCell = !(face.second()) ? owner[face.first()] : neighbour[face.first()];

        whichCells[count] = solidCell;
        values[count]     = psi[fluidCell]; // copy value for zero grad
        ++count;
    }

    eqn.setValues(whichCells, values);

    Info << "done." << endl;

}

template<class Type>
List< ibFace >& ParticleZeroGradient<Type>::ibFaces()
{
  if( !ibFacesPtr_.valid() )
  {
    const volScalarField& vF = voidFrac();

    // Find faces of immersed boundary (IB) and their adjacent solid cells


    const labelList& owner     = mesh().owner();
    const labelList& neighbour = mesh().neighbour();

    label count = 0;
    forAll(owner, faceI)
    {
      scalar diff = vF[owner[faceI]] - vF[neighbour[faceI]];

      if( magSqr(diff) > VSMALL )
        ++count; // face belongs to IB
    }

    ibFacesPtr_.reset(
                       new List<ibFace>(count)
                     );
    List<ibFace>& faces = ibFacesPtr_();

    count = 0;
    forAll(owner, faceI)
    {
      scalar diff = vF[owner[faceI]] - vF[neighbour[faceI]];

      if( magSqr(diff) <= VSMALL )
        continue; // face is either inside or outside IB

      // Now, face is part of IB
      if(diff > 0)
      {
        // owner cell is solid cell
        ibFace face = ibFace(faceI, true);
        faces[count] = face;
      }
      else
      {
        // neighbour cell is solid cell
        ibFace face = ibFace(faceI, false);
        faces[count] = face;
      }

      ++count;
    }

  }
  return ibFacesPtr_();
}



template<class Type>
bool ParticleZeroGradient<Type>::read(const dictionary& dict)
{
    if (cellSetOption::read(dict))
    {
        coeffs_.lookup("fieldNames") >> fieldNames_;
        applied_.setSize(fieldNames_.size(), false);
        rhoName_ = coeffs_.lookupOrDefault<word>("rhoName", "rhoInf");

        compressible_ = (rhoName_ != "rhoInf") ? true : false;

        checkData();

        if (debug)
        {
        }

        Info << nl << "fvOption 'particleZeroGradient' instantiated:" << endl
             << "\tgas model: " << (compressible_ ? "" : "in") << "compressible" << nl << endl;

        return true;
    }
    else
    {
        return false;
    }
}

template<class Type>
void ParticleZeroGradient<Type>::discardFaces()
{
    ibFacesPtr_.clear();
}

template<class Type>
void ParticleZeroGradient<Type>::checkData()
{
    // set inflow type
    switch (selectionMode())
    {
        case smCellSet:
        case smCellZone:
        case smAll:

            break;

        default:
        {
            FatalErrorIn("void particleZeroGradient::checkData()")
                << exit(FatalError);
        }
    }
}


template<class Type>
const volScalarField& ParticleZeroGradient<Type>::voidFrac()
{
  if(! mesh_.foundObject<volScalarField>("voidFrac") )
  {
    Info << nl << "Field 'voidFrac' not found in registry. "
         << "Assuming static run. Loading field into registry." << endl;

    voidFracPtr_.reset
    (
         new volScalarField
         (
           IOobject
           (
               "voidFrac",
               mesh_.time().timeName(),
               mesh_,
               IOobject::READ_IF_PRESENT,
               IOobject::AUTO_WRITE
           ),
           mesh_,
           dimensioned<scalar>("fluid", dimensionSet(0, 0, 0, 0, 0), 0.0),
           zeroGradientFvPatchField<scalar>::typeName
         )
    );
  }

  const volScalarField& vF = mesh_.lookupObject<volScalarField>("voidFrac");

  return vF;
}

template<class Type>
tmp<Foam::volScalarField> ParticleZeroGradient<Type>::rho() const
{
    if (compressible_)
    {
        return mesh_.lookupObject<volScalarField>(rhoName_);
    }
    else
    {
        return volScalarField::null();
    }
}

} // end namepsace fv
} // end namespace
// ************************************************************************* //
