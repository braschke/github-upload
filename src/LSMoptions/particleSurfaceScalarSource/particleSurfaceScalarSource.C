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

    $Date: 2014-04-16 13:48:25 +0200 (Mi, 16 Apr 2014) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "particleSurfaceScalarSource.H"
#include "fvMesh.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"

#include "makros.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace fv
    {
        defineTypeNameAndDebug(particleSurfaceScalarSource, 0);
        addToRunTimeSelectionTable
        (
            option,
            particleSurfaceScalarSource,
            dictionary
        );
    }

}
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::particleSurfaceScalarSource::particleSurfaceScalarSource
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    cellSetOption(name, modelType, dict, mesh),
    volFracInf_(
                 "volFracInf",
                 dimensionSet(0, 0, 0, 0, 0),
                 1
               ),
    specificReactionRate_(
                           "specificReactionRate",
                           dimensionSet(0, 1, -1, 0, 0),
                           1.0
                         ),
    transientIB_(true),
    ibAdjacentFluidCellsPtr_(0),
    zeroGradMap_(0)
{
    read(dict);
}

Foam::fv::particleSurfaceScalarSource::~particleSurfaceScalarSource()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fv::particleSurfaceScalarSource::constrain
(
    fvMatrix<scalar>& eqn,
    const label
)
{
  // Prevent solve of scalar equation within IB
  const scalarField& vF   = mesh_.lookupObject<volScalarField>("voidFrac").internalField();
  const scalarField& psi  = eqn.psi().internalField();

  label count= 0;
  forAll(vF, labelI)
  {
    if(vF[labelI] > 0.)
      ++count;
  }

  labelList   whichCells(count);
  scalarField values(count);

  count = 0;
  forAll(vF, labelI)
  {
    if(vF[labelI] > 0.)
    {
      whichCells[count] = labelI;
      // Leave value 'as is'.
      values[count]     = psi[labelI];

      // fix for zeroGradient
      if( zeroGradMap_.found( labelI ) )
        values[count] = psi[ zeroGradMap_[ labelI ] ];

      ++count;
    }
  }

  eqn.setValues(whichCells, values);
}

void Foam::fv::particleSurfaceScalarSource::addSup
(
    fvMatrix<scalar>& eqn,
    const label fieldI
)
{

    if(transientIB_)
      discardAdjacentVolume();

    volScalarField reactionSource
    (
        IOobject
        (
            "reactionSource",
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_,
        dimensionedScalar("zero", dimless/dimTime, 0)
    );

    const scalarField& ibAFC = ibAdjacentFluidCells();
    
    const DimensionedField< scalar, volMesh > & cellVols = mesh_.V();

    Info << endl << "particleSurfaceScalarSource '" << name()
    << "': active surface area = " << sum(ibAFC)
    << "; applied to field '" << fieldNames_[fieldI] << "'"
    << endl;

    const volScalarField& species = mesh_.lookupObject<volScalarField>(fieldNames_[fieldI]);
	_PDBO_(species.internalField());

//OF-3: Vora
/*
    reactionSource.internalField() =
               ibAFC*specificReactionRate_.value()// -> abs. volume rate
               /cellVols.field();                 // -> volume fraction rate

    reactionSource.internalField() *=
                           min(
                                max(
                                     species.internalField()/volFracInf_.value(),
                                     0.
                                   ),
                                1.
                              ); // volume fraction \in [0,1]


    // add source to rhs of eqn
    eqn -= reactionSource;
*/

//Vora: OF 5
// one needs to replace instances of "boundaryField()" and "internalField()" with "boundaryFieldRef()" and "internalFieldRef()" if one wants to modify them in the code. The non-"Ref" calls are const now.
// internalField and internalFieldRef have been replaced by direct calls to the field variable. Eg: U.internalField() becomes U() for a const reference to the internal field, or U.ref() for non-const access.

//    static_cast<Foam::Field<double>>(reactionSource.internalField()) =
//               ibAFC*specificReactionRate_.value()// -> abs. volume rate
//               /cellVols.field();                 // -> volume fraction rate
//    reactionSource.ref() =
//               ibAFC*specificReactionRate_.value()// -> abs. volume rate
//               /cellVols.field();                 // -> volume fraction rate
    (reactionSource.ref()).field() =
               ibAFC*specificReactionRate_.value()// -> abs. volume rate
               /cellVols.field();                 // -> volume fraction rate

    reactionSource.ref() *=   min(
                                max(
                                     species.internalField()/volFracInf_.value(),
                                     0.
                                   ),
                                1.
                              ); // volume fraction \in [0,1]
_PDBO_(species.internalField());

    // add source to rhs of eqn
    eqn -= reactionSource;


    if (mesh_.time().outputTime())
    {
        reactionSource.write();
    }

}


void Foam::fv::particleSurfaceScalarSource::discardAdjacentVolume()
{
  ibAdjacentFluidCellsPtr_.clear();
  zeroGradMap_.clear();
}

Foam::scalarField& Foam::fv::particleSurfaceScalarSource::ibAdjacentFluidCells()
{
  if( !ibAdjacentFluidCellsPtr_.valid() )
  {
    ibAdjacentFluidCellsPtr_.reset(
                                    new Foam::scalarField(mesh().nCells(), 0.0)
                                  );

    zeroGradMap_.clear(); // Just to make sure! Map is empty anyway.

    scalarField& ibAFC = ibAdjacentFluidCellsPtr_();

    const volScalarField& vF = mesh_.lookupObject<volScalarField>("voidFrac");

    const surfaceScalarField& faceAreas = mesh_.magSf();

    // 1) Find faces of immersed boundary (IB) and their adjacent fluid cells
    // 2) Source term is
    //      applied to adjacent fluid cells
    //      specific to face area
    //      dependent of concentration of psi

    const labelList& owner     = mesh().owner();
    const labelList& neighbour = mesh().neighbour();

    forAll(owner, faceI)
    {
      scalar diff = vF[owner[faceI]] - vF[neighbour[faceI]];

      if( magSqr(diff) <= VSMALL )
        continue; // face is either inside or outside IB

      // Now, face is part of IB
      if(diff > 0)
      {
        // neighbour cell is fluid cell
        // Add face area to neighbouring fluid cell
        ibAFC[neighbour[faceI]] += faceAreas[faceI];
        // map index of neighbour to owner
        zeroGradMap_.insert(owner[faceI], neighbour[faceI]);
      }
      else
      {
        // owner cell is fluid cell
        // Add face area to owner fluid cell
        ibAFC[owner[faceI]] += faceAreas[faceI];
        // map index of owner to neighbour
        zeroGradMap_.insert(neighbour[faceI], owner[faceI]);
      }
    }

  }

  return ibAdjacentFluidCellsPtr_();
}



bool Foam::fv::particleSurfaceScalarSource::read(const dictionary& dict)
{
    if (cellSetOption::read(dict))
    {
        coeffs_.lookup("fieldNames") >> fieldNames_;
        applied_.setSize(fieldNames_.size(), false);

        volFracInf_.readIfPresent(coeffs_);
        specificReactionRate_.readIfPresent(coeffs_);
        transientIB_ = coeffs_.lookupOrDefault("transientIB", true);

        if (debug)
        {
        }

        Info << nl << "fvOption 'particleSurfaceScalarSource' instantiated:" << endl
             << "\tvolFracInf_: " << volFracInf_ << endl
             << "\tspecificReactionRate: " << specificReactionRate_ << endl
             << "\ttransientIB: " << (transientIB_ ? "true" : "false") << nl << endl;

        return true;
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //
