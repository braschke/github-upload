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

    Ashish Vinayak
    Chair of Fluid Mechanics
    vinayak@uni-wuppertal.de

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "interParticleMomentum.H"

#include "fvMesh.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"

#include "zeroGradientFvPatchField.H" //Vora: need to declare in version 5.

#include "makros.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace fv
    {
        defineTypeNameAndDebug(interParticleMomentum, 0);
        addToRunTimeSelectionTable
        (
            option,
            interParticleMomentum,
            dictionary
        );
    }

}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::interParticleMomentum::interParticleMomentum
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    cellSetOption(name, modelType, dict, mesh),
    rhoName_("none"),
    transientIB_(true),
    ibFacesPtr_(0),
    particleVeloPtr_(0),
    voidFracPtr_(0),
    compressible_(false),
    correctVelocity_(true)
{
    read(dict);
#if 1
    // eventually preload fields into registry
    voidFrac();
    particleVelo();
#endif
}

Foam::fv::interParticleMomentum::~interParticleMomentum()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fv::interParticleMomentum::constrain
(
    fvMatrix<vector>& eqn,
    const label
)
{
    Info << endl << "fvOption 'interParticleMomentum' constraining '"
         << eqn.psi().name() << "' ... " << flush;

    if(transientIB_)
      discardFaces();

    const vectorField& pV = particleVelo();
    const scalarField& vF = voidFrac();

    label count= 0;
    forAll(vF, labelI)
    {
      if(vF[labelI] > 0.)
        ++count;
    }

    labelList   whichCells(count);
    vectorField values(count);

    count = 0;
    forAll(vF, labelI)
    {
      if(vF[labelI] > 0.)
      {
        whichCells[count] = labelI;
        values[count]     = pV[labelI];
        ++count;
      }
    }

    eqn.setValues(whichCells, values);

    Info << "done." << endl;

}

Foam::List<Foam::fv::interParticleMomentum::ibFace>& Foam::fv::interParticleMomentum::ibFaces()
{
  if( !ibFacesPtr_.valid() )
  {
    const volScalarField& vF = voidFrac();

    // 1) Find faces of immersed boundary (IB) and their adjacent solid cells
    // 2) Source term is
    //      applied to adjacent solid cells
    //      dependent on velocity of adjacent fluid

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
        // Add face area to neighbouring fluid cell
        ibFace face = ibFace(faceI, true);
        faces[count] = face;
      }
      else
      {
        // neighbour cell is solid cell
        // Add face area to owner fluid cell
        ibFace face = ibFace(faceI, false);
        faces[count] = face;
      }

      ++count;
    }

  }
  return ibFacesPtr_();
}

void Foam::fv::interParticleMomentum::correct(volVectorField& velo)
{
    if(!correctVelocity_)
      return;

    Info << endl << "fvOption 'interParticleMomentum' correcting '"
    	 << "' ... " << flush;

    if(transientIB_)
      discardFaces();

    const vectorField& pV = particleVelo();
    const scalarField& vF = voidFrac();

    label count= 0;
    forAll(vF, labelI)
    {
      if(vF[labelI] > 0.)
          velo[labelI]     = pV[labelI];
    }

    Info << "done." << endl;
}


bool Foam::fv::interParticleMomentum::read(const dictionary& dict)
{
    if (cellSetOption::read(dict))
    {
        coeffs_.lookup("fieldNames") >> fieldNames_;
        applied_.setSize(fieldNames_.size(), false);
        rhoName_ = coeffs_.lookupOrDefault<word>("rhoName", "rhoInf");

        transientIB_ = coeffs_.lookupOrDefault("transientIB", true);
        compressible_ = (rhoName_ != "rhoInf") ? true : false;
        coeffs_.lookup("correctVelocity") >> correctVelocity_;

        checkData();

        if (debug)
        {
        }

        Info << nl << "fvOption 'interParticleMomentum' instantiated:" << endl
             << "\tgas model: " << (compressible_ ? "" : "in") << "compressible" << endl
             << "\twith PISO correction loop: " << (correctVelocity_ ? "" : "no-") << "correction" << nl << endl;

        return true;
    }
    else
    {
        return false;
    }
}


void Foam::fv::interParticleMomentum::discardFaces()
{
    ibFacesPtr_.clear();
}

void Foam::fv::interParticleMomentum::checkData()
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
            FatalErrorIn("void interParticleMomentum::checkData()")
                << exit(FatalError);
        }
    }
}

const Foam::volVectorField& Foam::fv::interParticleMomentum::particleVelo()
{
  if(! mesh_.foundObject<volVectorField>("particleVelo") )
  {
    Info << nl << "Field 'particleVelo' not found in registry. "
         << "Assuming static run. Loading field into registry." << endl;

    particleVeloPtr_.reset
    (
         new volVectorField
         (
           IOobject
           (
               "particleVelo",
               mesh_.time().timeName(),
               mesh_,
               IOobject::READ_IF_PRESENT,
               IOobject::AUTO_WRITE
           ),
           mesh_,
           dimensioned<vector>("fluid", dimensionSet(0, 1, -1, 0, 0), vector(0., 0., 0.)),
           zeroGradientFvPatchField<vector>::typeName
         )
    );
  }

  const volVectorField& pV = mesh_.lookupObject<volVectorField>("particleVelo");

  return pV;
}

const Foam::volScalarField& Foam::fv::interParticleMomentum::voidFrac()
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


Foam::tmp<Foam::volScalarField> Foam::fv::interParticleMomentum::rho() const
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


// ************************************************************************* //
