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

        particleZeroGradientCoeffs
        {
            fieldNames      (U);    // names of fields on which to apply source
            rhoName         rho;    // density field if compressible case
        }


\*---------------------------------------------------------------------------*/

#ifndef ParticleZeroGradient_H
#define ParticleZeroGradient_H

#include "cellSetOption.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{
       typedef Tuple2<label, bool> ibFace;

/*---------------------------------------------------------------------------*\
                 Class particleZeroGradient Declaration
\*---------------------------------------------------------------------------*/
template<class Type>

class ParticleZeroGradient
:
    public cellSetOption
{

public:



protected:
    // Protected data

        //- Name of density field
        word rhoName_;

        autoPtr<List<ibFace> > ibFacesPtr_;

        // dummy pointers
        autoPtr<volScalarField> voidFracPtr_;

        //- flag indicating compressible flow solver
        bool compressible_;


    // Protected Member Functions

        List<ibFace>& ibFaces();

        void discardFaces();

        const volScalarField& voidFrac();


        //- Check data
        void checkData();


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        ParticleZeroGradient(const ParticleZeroGradient&);

        //- Disallow default bitwise assignment
        void operator=(const ParticleZeroGradient&);


public:

    //- Runtime type information
    TypeName("particleZeroGradient");


    // Constructors

        //- Construct from components
        ParticleZeroGradient
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~ParticleZeroGradient();


    // Member Functions
        //- Return the density field [kg/m3]
        inline tmp<volScalarField> rho() const;


        // Source term addition

        // Set values directly
        // Vector
        virtual void constrain(fvMatrix<Type>& eqn, const label fieldI);

        // IO

            //- Read dictionary
            virtual bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // end namespace fv
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
#include "ParticleZeroGradient.C"

#endif

// ************************************************************************* //
