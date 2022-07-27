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

    $Date: 2014-04-07 10:20:44 +0200 (Mo, 07 Apr 2014) $

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

#ifndef particleSurfaceScalarSource_H
#define particleSurfaceScalarSource_H

#include "cellSetOption.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                 Class particleSurfaceScalarSource Declaration
\*---------------------------------------------------------------------------*/

class particleSurfaceScalarSource
:
    public cellSetOption
{

public:



protected:

    // Protected data

        //- Name of density field
        word rhoName_;

        // volume fraction at which kinetic has reached maximum
        // rr = volFrac/volFracInf_ * (species reaction rate)
        dimensionedScalar volFracInf_;

        // specific (according to surface area) reaction rate
        // dimensions of reaction rate: m^3/s
        // dimensions of specific reaction rate: m^3/s / m^2 = m/s
        dimensionedScalar specificReactionRate_;

        bool                 transientIB_;
        autoPtr<scalarField> ibAdjacentFluidCellsPtr_;
        Map<label>           zeroGradMap_;



    // Protected Member Functions

        scalarField& ibAdjacentFluidCells();

        void discardAdjacentVolume();



private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        particleSurfaceScalarSource(const particleSurfaceScalarSource&);

        //- Disallow default bitwise assignment
        void operator=(const particleSurfaceScalarSource&);


public:

    //- Runtime type information
    TypeName("particleSurfaceScalarSource");


    // Constructors

        //- Construct from components
        particleSurfaceScalarSource
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~particleSurfaceScalarSource();


    // Member Functions

        //- Constrain energy equation to fix the temperature
        virtual void constrain(fvMatrix<scalar>& eqn, const label fieldI);

       //- Source term to fvMatrix<vector>
       virtual void addSup(fvMatrix<scalar>& eqn, const label fieldI);

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
