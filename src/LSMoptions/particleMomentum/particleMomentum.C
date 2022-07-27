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

#include "particleMomentum.H"
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
        defineTypeNameAndDebug(particleMomentum, 0);
        addToRunTimeSelectionTable
        (
            option,
            particleMomentum,
            dictionary
        );
    }

}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::particleMomentum::particleMomentum
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
    slipBC_(false)
{
    read(dict);
#if 1
    // eventually preload fields into registry
    voidFrac();
    particleVelo();
#endif
}

Foam::fv::particleMomentum::~particleMomentum()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fv::particleMomentum::constrain
(
    fvMatrix<vector>& eqn,
    const label
)
{
    Info << endl << "fvOption 'particleMomentum' constraining '"
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

    if(slipBC_)
      setSlip(eqn);

    Info << "done." << endl;

}

Foam::List<Foam::fv::particleMomentum::ibFace>& Foam::fv::particleMomentum::ibFaces()
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


void Foam::fv::particleMomentum::setSlip(fvMatrix<vector>& eqn)
{
    const vectorField&  U = eqn.psi().internalField();

    const List<ibFace>&  faces     = ibFaces();
    const labelList&     owner     = mesh().owner();
    const labelList&     neighbour = mesh().neighbour();
    const vectorField&   sf        = mesh().Sf();

//    whichCells.reset(faces.size());
    vectorField slipValues(mesh_.nCells(), vector::zero);

    forAll(faces, ibFaceI)
    {
      bool   ownerNeighbour = faces[ibFaceI].second();
      label  faceI          = faces[ibFaceI].first();
      label  solidCellI;
      label  fluidCellI;
      vector faceVec        = sf[faceI];

      if(ownerNeighbour) // solid cell is owner or neighbour?
      {
        // solid cell is owner
        solidCellI = owner[faceI];
        fluidCellI = neighbour[faceI];
      }
      else
      {
        // solid cell is neighbour
        solidCellI = neighbour[faceI];
        fluidCellI = owner[faceI];
      }

      // add tangential component
      // tangential = total - normal
      slipValues[solidCellI] += U[fluidCellI] - (U[fluidCellI] & faceVec) * faceVec;
    }

    label count = 0;
    forAll(slipValues, valueI)
    {
      if(magSqr(slipValues[valueI]) > SMALL)
        ++count;
    }
    vectorField values(count);
    labelList   whichCells(count);
    count = 0;
    forAll(slipValues, valueI)
    {
      if(magSqr(slipValues[valueI]) > SMALL)
      {
        values[count] = slipValues[valueI];
        whichCells[count] = valueI;
        ++count;
      }
    }

    //_DBO_(values)
    eqn.setValues(whichCells, values);
}


void Foam::fv::particleMomentum::correct(volVectorField& velo)
{
    if(!slipBC_)
      return;

//    vectorField&  U = velo.internalField(); // Original for Openfoam 3

//Vora:
// one needs to replace instances of "boundaryField()" and "internalField()" with "boundaryFieldRef()" and "internalFieldRef()" if one wants to modify them in the code. The non-"Ref" calls are const now.
// internalField and internalFieldRef have been replaced by direct calls to the field variable. Eg: U.internalField() becomes U() for a const reference to the internal field, or U.ref() for non-const access.

    vectorField&  U = velo.ref(); //for non-const access.
//  const  vectorField&  U = velo(); // for const reference.

    const List<ibFace>&  faces     = ibFaces();
    const labelList&     owner     = mesh().owner();
    const labelList&     neighbour = mesh().neighbour();
    const vectorField&   sf        = mesh().Sf();

    vectorField slipValues(mesh_.nCells(), vector::zero);

    forAll(faces, ibFaceI)
    {
      bool   ownerNeighbour = faces[ibFaceI].second();
      label  faceI          = faces[ibFaceI].first();
      label  solidCellI;
      label  fluidCellI;
      vector faceVec        = sf[faceI];

      if(ownerNeighbour) // solid cell is owner or neighbour?
      {
        // solid cell is owner
          solidCellI = owner[faceI];
          fluidCellI = neighbour[faceI];
      }
      else
      {
        // solid cell is neighbour
        solidCellI = neighbour[faceI];
        fluidCellI = owner[faceI];
      }

      // add tangential component
      // tangential = total - normal
      slipValues[solidCellI] += U[fluidCellI] - (U[fluidCellI] & faceVec) * faceVec;
    }

    forAll(slipValues, valueI)
    {
      if(magSqr(slipValues[valueI]) > SMALL)
        U[valueI] = slipValues[valueI];
    }
}


bool Foam::fv::particleMomentum::read(const dictionary& dict)
{
    if (cellSetOption::read(dict))
    {
        coeffs_.lookup("fieldNames") >> fieldNames_;
        applied_.setSize(fieldNames_.size(), false);
        rhoName_ = coeffs_.lookupOrDefault<word>("rhoName", "rhoInf");

        transientIB_ = coeffs_.lookupOrDefault("transientIB", true);
        compressible_ = (rhoName_ != "rhoInf") ? true : false;
        slipBC_ = coeffs_.lookupOrDefault<Switch>("slipBC", false);

        checkData();

        if (debug)
        {
        }

        Info << nl << "fvOption 'particleMomentum' instantiated:" << endl
             << "\tgas model: " << (compressible_ ? "" : "in") << "compressible" << endl
             << "\tboundary conditions: " << (slipBC_ ? "" : "no-") << "slip" << nl << endl;

        return true;
    }
    else
    {
        return false;
    }
}


void Foam::fv::particleMomentum::discardFaces()
{
    ibFacesPtr_.clear();
}

void Foam::fv::particleMomentum::checkData()
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
            FatalErrorIn("void particleMomentum::checkData()")
                << exit(FatalError);
        }
    }
}

const Foam::volVectorField& Foam::fv::particleMomentum::particleVelo()
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

const Foam::volScalarField& Foam::fv::particleMomentum::voidFrac()
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


Foam::tmp<Foam::volScalarField> Foam::fv::particleMomentum::rho() const
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
