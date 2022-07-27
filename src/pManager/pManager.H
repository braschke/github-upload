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

 #ifndef pManager_H
 #define pManager_H
//#ifndef functionObjects_pManager_H
//#define functionObjects_pManager_H


#include "makros.H"


#include "coordinateSystem.H"
#include "coordinateSystems.H"
#include "primitiveFieldsFwd.H"
#include "volFieldsFwd.H"
#include "HashSet.H"
#include "Tuple2.H"
#include "OFstream.H"
#include "IFstream.H"
#include "Switch.H"
#include "pointFieldFwd.H"
#include "pointIndexHit.H"
#include "treeDataCell.H"

#include "volumetricParticle.H"
#include "population.H"
#include "potential.H"
#include "londonPotential.H"
#include "meshToMesh.H"

struct details;


#include "contact.H"
#include "contactModel.H"
#include "bgGrid.H"

#include "myMeshSearch.H"

#include "fvMeshFunctionObject.H"
#include "regionFunctionObject.H"

#include "RAPID.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


namespace Foam
{
// Forward declaration of classes
class objectRegistry;
class dictionary;
class mapPolyMesh;
class fvMesh;

namespace functionObjects //Vora
{
// class objectRegistry;
// class dictionary;
// class mapPolyMesh;

/*---------------------------------------------------------------------------*\
                           Class forces Declaration
\*---------------------------------------------------------------------------*/

#define MAX_POPULATIONS 32

class pManager
:
    public regionFunctionObject
//     public fvMeshFunctionObject 
//     public logFiles
{
protected:

    // Private data

        //- Name of this set of forces,
        //  Also used as the name of the probes directory.
        word name_;

        const dictionary& dict_;

        const objectRegistry& obr_;

        //- on/off switch
        bool active_;

        //- Switch to send output to Info as well as to file
        Switch log_;

        //- Print values in local coordinate system
        Switch writeLocal_;

        // Read from dictionary

            //- Patches to integrate forces over
            labelHashSet patchSet_;

            //- Name of pressure field
            word pName_;

            //- Name of velocity field
            word UName_;

            //- Name of temperature field
            word TName_;

            //- Name of electromagnetic field
            word EMName_;

            //- Name of polarization field
            word PolName_;

            //- Name of surface charge field
            word sigmaName_;

            //- Name of density field (optional)
            word rhoName_;

            //- The name of the force density (fD) field
            word fDName_;

            //- Reference density needed for incompressible calculations
            scalar rhoRef_;

            //- Reference pressure
            scalar pRef_;

            //- fluid structure interaction switch
            Switch fsi_;

            //- electro magnetics interaction switch
            Switch em_;

            //- temperature interaction switch
            Switch thermoForces_;

            //- move particles
            Switch moveParticles_;

            //- resolve particle-particle-collision
            Switch ppCollisions_;

            //- Is it possible for agglomerates to break up?
            Switch breakAgglomerates_;

            //- Used to define the range of considered particles in an agglomerate
            // for breakage decision
            int breakAgglomeratesIterations_;

            //- shape of the region for collision checks around a structure's cg
            word collisionRegionForStructures_;

            //- Defines how the adhesion integration is performed
            word adhesionIntegrationType_;

            //- Defines how the stresstensor is calculated
            word stresstensorInterpolationMethod_;

            //- Defines the weighting of neighbourcellValues for NeighbourInterpolation (stress)
            word stresstensorNeighbourWeighting_;

            //- Defines the weighting of neighbourcellValues for NeibghbourInterpolation (pressure)
            word pressureNeighbourWeighting_;

            //- Defines the weighting of neighbourcellValues for NeibghbourInterpolation (pressure)
            word cLciWnlci_weighting_;

            //- Defines how the pressurefield is calculated
            word pressureInterpolationMethod_;

            //- 2: Print kinetic energy of all objects plus the sum, 1: just the sum, 0: none
            unsigned int printKinetic_;


             //- Void fraction for immersed boundary
            autoPtr<volScalarField> voidFracPtr_;
            //- velocities of immersed bodies
            autoPtr<volVectorField> particleVeloPtr_;
            //- Deposition fraction for deposited particles
            autoPtr<volScalarField> depositPtr_;
            //- Distance to wall
            autoPtr<volScalarField> wallDistPtr_;
            //- Vector pointing to nearest wall
            autoPtr<volVectorField> wallNPtr_;

            //- for post processing purposes
            Switch                              writeDevRhoReff_;
            mutable autoPtr<volSymmTensorField> devRhoReffPtr_;
            mutable bool                        validDevRhoReff_;

            List<const vectorField*>   activeFacecentres_;
            List<const vectorField*>   activeFacevectors_;

            List<volumetricParticle*>  particleList_;
            // Container for contacts
            contactHash_t              contactHash_;
            // Contact detection radius
            scalar                     contactRadiusFactor_;
            // Relaxation factor for particle movement
            scalar                     moveParticlesRelax_;
            // Number of subcycles for particle movement
            label                      moveParticlesSubcycles_;
            // Desired precision for kinetic energy loop in impact force resolution
            scalar                     kinEnPrecision_;
            // for firstSeondOutsideWeighted weight for firstOutside -- stressTensor
            scalar                     stresstensorFirstSecondWeight_;
            // for firstSecondOutsideWeighted weight for firstOutside --pressure
            scalar                     pressureFirstSecondWeight_;
            // Number of maximum kinetic energy loops in impact force resolution
            int                        kinEnLoopsMax_;
            // iterative solid-fluid-coupling
            scalar                     iterativeCouplingInitialRelax_;
            scalar                     iterativeCouplingCurrentRelax_;
            label                      iterativeCouplingSubcycles_;
//            List<word>                 iterativeCouplingFields_;
            bool                       iterativeCouplingIsSubCycle_;
            label                      iterativeCouplingCurrSubcycle_;
            TimeState                  iterativeCouplingSavedTime_;

            // Tableau of potential pointers for each combination of populations
            autoPtr<Potential> potTable[MAX_POPULATIONS][MAX_POPULATIONS];
            // Tableau of contact model pointers for each combination of populations
            autoPtr<contactModel> contactModelTable[MAX_POPULATIONS][MAX_POPULATIONS];


            List<Population>    popList_;
            label          nPopulations_;
            label            nParticles_;

            //- back ground grid
            autoPtr<bgGrid>    bgGridPtr_;
            //- granularity of background grid
            scalar bgGranularity_;

            // trace and react to mesh updates
            bool   meshGeomChanged_;

            // mesh search class
            myMeshSearch  myMS_;

    // Private Member Functions
            Potential*     potTableGet(label i, label j);
            void           potTableSet(label i, label j, autoPtr<Potential> pot);
            contactModel*  cMTableGet(label i, label j);
            void           cMTableSet(label i, label j, autoPtr<contactModel> pot);

        void        renewFaceLists();
        void        subCyclingSaveState();
        void        subCyclingPreCollisionSaveState();
        void        subCyclingRestoreState();

        // iterative solid-fluid-coupling
        void        iterativeCouplingParticleSavePoints();
        void        iterativeCouplingParticleSaveForces();
        void        iterativeCouplingParticleRestorePoints();
        void        iterativeCouplingParticleRestoreForces();
        void        iterativeCouplingParticleRelaxForces();

        //- Disallow default bitwise copy construct
        pManager(const pManager&);

        //- Disallow default bitwise assignment
        void operator=(const pManager&);


        //- Get (demand driven) reference to background grid
        const bgGrid& backGroundGrid();

        void writeParticlePropertiesAndGeometries();
        void writePopulationJournal();
        void endOfExecution();
        // Read any particles already present from particleJournal
        void emergeParticles();
        void distributeParticles();
        void deleteParticlesBB();
        template<class Type>
        void distributeForces(
                               Field<Type>& (volumetricParticle::* fieldGetter) ()
                             );
        void distributePointVelocity();
        void injectParticles();
        void preLoadParticleFields();
        void move(scalar relax, int subiteration);

        void checkForContacts();
        void checkForCollisions(scalar currentRelax);
        bool areParticlesFarApart(volumetricParticle* firstPrt, volumetricParticle* otherPrt);
        void collisionCheckVer1(RAPID_model& m1, volumetricParticle& otherPrt);
        void collisionCheckVer2(volumetricParticle& firstPrt, volumetricParticle& otherPrt);
        void reassignContactPartners(volumetricParticle& particle);
        void reassignContactPartners(volumetricParticle& particle, int iterations);
        void checkAndUndoPenetration();
        void breakAgglomerates(scalar currentRelax);
        void undoPenetration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, int elem_id1, int elem_id2);
        void undoPenetration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector distVec);
        void undoPenetrationWithStructure(volumetricParticle& firstPrt, volumetricParticle& otherPrt, List<int> pFaces, vector structNormal, vector structCf);
        void mergeStl();
        void mergeSTL(volumetricParticle *prt1, volumetricParticle *prt2);
        void calcAdhesiveForcesBetweenContactPartners();
        void integrateAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt);
        void integrateAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector, int elem_id1, int elem_id2);
        void integrateAdhesionGeneral(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector);
        void integrateAdhesionForFlatContact(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector);
        void integrateAdhesionTriangleIntegration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector, int elem_id1, int elem_id2 );
        void integrateForcesDuringCollision(volumetricParticle* firstPrt, volumetricParticle* otherPrt, List<int> reducedList1, List<int> reducedList2);
        vector testAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt);

        void calcSolidForces();
        void resetForces();
        void resetSolidForces();
        void reduceParticleList(List<volumetricParticle*>& prtList);
        void processContacts();
        void mapParticleMomentumToFluid();
        void mapParticleMomentumToFluidOverNeighbours();
        void mapParticleSigmaToFluid();
        void mapParticlePermittivityToFluid();
        void mapParticleDepositToFluid();
        void mapFluidForcesToParticles();
        void mapFluidForcesToSubCellParticle(volumetricParticle* pPtr);
        void mapThermophoreticForcesToParticles();
        void mapElectroMagneticForcesToParticles();
        volScalarField& voidFrac();
        volScalarField& deposit();
        volScalarField& wallDistance();
        volVectorField& particleVelo();
        volVectorField& wallN();

        const volSymmTensorField& devRhoReff() const;
        tmp<volScalarField>       rho()        const;
        scalar                    rho(const volScalarField& p) const;

        // handle contacts and move particles due to all forces available
        void moveSolids();
        // iterative coupling: store and restore all relevant fields from CFD
        void iterativeCouplingCFDSaveFields();
        void iterativeCouplingCFDRestoreFields();
        // iterative coupling: relax
        void iterativeCouplingCFDRelax();
        // helper for subcycling
        bool   isIterativeCoupling();
        bool   isFirstSubiteration();
        bool   isLastSubiteration();
        bool   isSecondOrLaterSubiteration();
        scalar subiterationProgress();
        label  subiterationsLeft();

        // trace and react to mesh updates
        void   updateGeomProperties();


        // Display kinetic energies
        void printKineticEnergy() const;

        // soot(particle) burning
        void pOxidation();

public:

    //- Runtime type information
    TypeName("pManager");

    //- Construct for given objectRegistry and dictionary.
    //  Allow the possibility to load fields from files
        pManager                                           // Vora
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict,
            const bool loadFromFiles = false
        );


    //- Destructor
    virtual ~pManager();

    void     clearOut();

    // Member Functions

        //- Return name of the set of forces
        virtual const word& name() const
        {
            return name_;
        }

        //- Read the forces data
        virtual bool read(const dictionary&); //Vora: OF-5.x
//         virtual void myRead(const dictionary&);                //Vora: OF-3

        //- print out particle statistics
        virtual void printStats(const word&) const;

        //- Preparation of fields and injection during first or continuation time loop
        virtual void start();

        //- each time-loop
        virtual bool execute();                            //Vora: OF-5.x
//         virtual void execute();//Vora: OF-3

        //- Execute at the final time-loop, currently does nothing
        virtual bool end();//Vora: OF-5.x
//         virtual void end();//Vora: OF-3

        //- Execute when Time::operator++ was called
        void timeSet();

        //- Write the forces
        virtual bool write();//Vora: OF-5.x
//         virtual void write();//Vora: OF-3

        //- Update for changes of mesh
        virtual void updateMesh(const mapPolyMesh&)
        {}

        //- Update for changes of mesh
        virtual void movePoints(const polyMesh&)
        {}

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


} //End namespace functionObjects //Vora    
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
