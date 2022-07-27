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

#include "particleFixedValue.H"
#include "fvMesh.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"
#include "zeroGradientFvPatchField.H" //Vora: need to declare in version 5.
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace fv
    {
        defineTypeNameAndDebug(particleFixedValue, 0);
        addToRunTimeSelectionTable
        (
            option,
            particleFixedValue,
            dictionary
        );
    }

}
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::particleFixedValue::particleFixedValue
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    cellSetOption(name, modelType, dict, mesh),
    fixedV_(0),
    voidFracPtr_(0)
{
    read(dict);
#if 1
    // eventually preload fields into registry
    voidFrac();
#endif
}

Foam::fv::particleFixedValue::~particleFixedValue()
{}
// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const Foam::volScalarField& Foam::fv::particleFixedValue::voidFrac()
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


void Foam::fv::particleFixedValue::constrain
(
    fvMatrix<scalar>& eqn,
    const label
)
{

	Info << endl << "fvOption 'particleFixedValue' constraining '"
	         << eqn.psi().name() << "' ... " << flush;

    const scalarField& vF = voidFrac();

    label count= 0;
    forAll(vF, labelI)
    {
      if(vF[labelI] > 0.)
        ++count;
    }

    labelList   whichCells(count);
    scalarField values(count, fixedV_);

    count = 0;
    forAll(vF, labelI)
    {
      if(vF[labelI] > 0.)
      {
        whichCells[count] = labelI;
        ++count;
      }
    }

    eqn.setValues(whichCells, values);
}


bool Foam::fv::particleFixedValue::read(const dictionary& dict)
{
    if (cellSetOption::read(dict))
    {
        coeffs_.lookup("fieldNames") >> fieldNames_;
        applied_.setSize(fieldNames_.size(), false);

        fixedV_ = readScalar( coeffs_.lookup("value") );

        if (debug)
        {
        }

        Info << nl << "fvOption 'particleFixedValue' instantiated:" << endl
             << "\tfixed value: " << fixedV_ << nl << endl;

        return true;
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //
