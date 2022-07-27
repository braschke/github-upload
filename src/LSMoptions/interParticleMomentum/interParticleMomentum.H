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

	The interParticleMomentum source term is based off of particleMomentum.
	In the constrain phase, it sets the field velocity to particle velocity.
	In the correct phase, it again does the same.

    Sources described by:

        interParticleMomentumCoeffs
        {
            fieldNames      (U);    // names of fields on which to apply source
            rhoName         rho;    // density field if compressible case
        }


\*---------------------------------------------------------------------------*/

#ifndef interParticleMomentum_H
#define interParticleMomentum_H

#include "cellSetOption.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                 Class interParticleMomentum Declaration
\*---------------------------------------------------------------------------*/

class interParticleMomentum
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

        //- flag indicating correction for PISO loop
        bool correctVelocity_;


    // Protected Member Functions

        Foam::List<Foam::fv::interParticleMomentum::ibFace>& ibFaces();

        void discardFaces();

        const volVectorField& particleVelo();
        const volScalarField& voidFrac();

        //- Check data
        void checkData();


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        interParticleMomentum(const interParticleMomentum&);

        //- Disallow default bitwise assignment
        void operator=(const interParticleMomentum&);


public:

    //- Runtime type information
    TypeName("interParticleMomentum");


    // Constructors

        //- Construct from components
        interParticleMomentum
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~interParticleMomentum();


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
