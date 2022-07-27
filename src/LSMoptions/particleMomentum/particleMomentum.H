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

        particleMomentumCoeffs
        {
            fieldNames      (U);    // names of fields on which to apply source
            rhoName         rho;    // density field if compressible case
        }


\*---------------------------------------------------------------------------*/

#ifndef particleMomentum_H
#define particleMomentum_H

#include "cellSetOption.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                 Class particleMomentum Declaration
\*---------------------------------------------------------------------------*/

class particleMomentum
:
    public cellSetOption
{

public:

       typedef Tuple2<label, bool> ibFace;


protected:
    // Protected data

        //- Name of density field
        word rhoName_;

        bool                   transientIB_;
        autoPtr<List<ibFace> > ibFacesPtr_;

        // dummy pointers
        autoPtr<volVectorField> particleVeloPtr_;
        autoPtr<volScalarField> voidFracPtr_;

        //- flag indicating compressible flow solver
        bool compressible_;

        //- flag indicating slip boundary conditions at IB
        bool slipBC_;


    // Protected Member Functions

        Foam::List<Foam::fv::particleMomentum::ibFace>& ibFaces();

        void discardFaces();

        const volVectorField& particleVelo();
        const volScalarField& voidFrac();

        void setSlip(fvMatrix<vector>& eqn);
        void correctSlip();

        //- Check data
        void checkData();


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        particleMomentum(const particleMomentum&);

        //- Disallow default bitwise assignment
        void operator=(const particleMomentum&);


public:

    //- Runtime type information
    TypeName("particleMomentum");


    // Constructors

        //- Construct from components
        particleMomentum
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~particleMomentum();


    // Member Functions
        //- Return the density field [kg/m3]
        inline tmp<volScalarField> rho() const;


        // Source term addition

        // Set values directly
        // Vector
        virtual void constrain(fvMatrix<vector>& eqn, const label fieldI);
        virtual void correct(volVectorField& U);

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
