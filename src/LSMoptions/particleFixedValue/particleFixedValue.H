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


    Sources described by:

        particleFixedValueCoeffs
        {
            fieldNames      (T);    // names of fields on which to apply source
            value            t;    // fixed value
        }


\*---------------------------------------------------------------------------*/

#ifndef particleFixedValue_H
#define particleFixedValue_H

#include "cellSetOption.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                 Class particleFixedValue Declaration
\*---------------------------------------------------------------------------*/

class particleFixedValue
:
    public cellSetOption
{

public:

       typedef Tuple2<label, bool> ibFace;

protected:

    // Protected data

        //- fixed Value value
        scalar fixedV_;

        // dummy pointers
        autoPtr<volScalarField> voidFracPtr_;


    // Protected Member Functions

        const volScalarField& voidFrac();

        Foam::List<Foam::fv::particleFixedValue::ibFace>& ibFaces();



private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        particleFixedValue(const particleFixedValue&);

        //- Disallow default bitwise assignment
        void operator=(const particleFixedValue&);


public:

    //- Runtime type information
    TypeName("particleFixedValue");


    // Constructors

        //- Construct from components
        particleFixedValue
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~particleFixedValue();


    // Member Functions

        //- Constrain energy equation to fix the temperature
        virtual void constrain(fvMatrix<scalar>& eqn, const label fieldI);


        // IO

            //- Read dictionary
            virtual bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
