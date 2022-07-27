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

#ifndef volumetricParticle_H
#define volumetricParticle_H

#include "fvMesh.H"
#include "triSurface.H"
#include "pointMesh.H"
#include "volMesh.H"
#include "fvPatchField.H"
#include "GeometricField.H"
#include "Time.H"
#include "triSurfaceSearch.H"

#include "contact.H"
#include "constraint.H"


#include "mpi.h"

#include "../pManager/bgGrid/bgGrid.H"

// Struct used for contacts with structures (see volumetricParticle::moveWithContactPartners)
struct structureContact
{
  Foam::vector	contactPoint;
  Foam::vector	avgNormal;
};

namespace Foam
{

class Population;

class volumetricParticle
{
public:
  enum stateType {
                   free,
                   master,
                   slave
                 };

  struct facePair
  {
  	Foam::label elem_id1; // Own face
  	Foam::label elem_id2; // Other face
  };

  scalar	radEVS_; // Radius of the particle's EVS
  scalar	dk_; // Contact radius of the sphere
  vector    contactNormal_; // Surface normal of the contact face (wall or other stl) in checkForContacts
  bool		deposited_; //true if particle has been deposited

  // Data structures for the agglomeration constraint
  // The faces and contactVectors for the partner at contactPartners[i]
  // are stored in the corresponding contactFaces[i]
  // and contactVectors_[i].
  List<facePair> contactFaces_;
  List<vector> contactVectors_;
  List<vector> contactNormals_;
  point			contactPoint_;
  List<structureContact>	structureContacts_;
  List<volumetricParticle*> contactPartners_;
  List<volumetricParticle*> unassignedPartners_;
  bool						movedWithContactPartners_;

  const bgGrid*	bg_; // bgGrid of particle, used for mpi communication
  label           bgSearchRadius_;
  MPI_Comm particleComm_; // Communicator for all the processors knowing about the particle
  List<label> particleProcs_; // List of processors knowing about the particle


  // Variables used for the agglomeration constraint.
  scalar commonMass_;
  point commonCg_;
  vector commonForce_;
  vector commonTorque_;
  vector velocityAtCgSubCellSize_;
  symmTensor commonJ_;
  vector commonVelo_;
  vector commonOmega_;
  bool calculatedAgglo_;

  fileName                    meshPath_;
  scalar           scale_;

  volumetricParticle(
                      const word     &idStr,
                      const fileName &pathToMesh,
                            label     popId,
                           scalar     rho,
                      const Time     &runTime,
                      const	bgGrid&	  bg,
					  	  	label  	  bgSearchRadius_,
                           stateType  state = master,
                      const word     &surfacePatchName = "surface",
                            bool      writeForceField = false,
                            bool      writePressureForceField = false,
                            bool      writePressureForceDensityField = false,
                            bool      writeStressForceField = false,
                            bool      writeStressForceDensityField = false,
					        Population* myPop = NULL
                    );

  ~volumetricParticle();

  Population* myPop_;

  label popId() const { return populationId_; }

  word  idStr() const { return idStr_; }

  vector getExternalAcc() { return externalAcc_; }
  vector getExternalOmegaAcc() { return externalOmegaAcc_; }

  stateType getState() const { return state_; }
  bool isFree  () const { return (state_ == free); }
  bool isMaster() const { return (state_ == master); }
  bool isSlave () const { return (state_ == slave); }
  bool isPointParticle();
  void setFree();
  void setMaster();
  void setSlave();

  void valuesToList(List<scalar>& list) const;
  void valuesToList(scalar* list) const;
  void listToValues(const List<scalar>& list);
  void listToValues(const scalar* list);
  void defineValues(
                             scalar  initialScale,
                       const vector &initialDisplacement,
                       const vector &velocity,
                       const vector &angularVelocity,
                       const vector &initialRotation
                     );

  void processSkiJump();

  triSurfaceSearch&  triSurfSearch() const;

        vectorField&       fluidForceField();
        vectorField&       savedFluidForceField();
        vectorField&       solidForceField();
        vectorField&       thermoForceField();
        vectorField&       savedThermoForceField();
        vectorField&       electromagForceField();
        vectorField&       contactForceField();
        vectorField&       savedElectromagForceField();
  const scalarField&       voidFracField();
        symmTensorField&    stressField();
        scalarField&       pressureField();
        scalarField&       thermoField();
        vectorField&       electromagField();
        scalarField&       sigmaField();
        vectorField&       polarField();
        vectorField&        shareVectorField();
        scalarField&       shareScalarField();
  void                      calcFluidForces(scalar pRef, scalar rho);
  void                      calcThermoForces(scalar thermophoreticFactor);
  void                      calcElectroMagForces(const scalar objectCharge);
  void                      calcElectroMagForces();
  void                      calcSurfaceCharge();
  void                      resetForces();
  void                      resetSolidForces();
  const vectorField&        Cf() const;
  const vectorField&        Sf() const;
  const vectorField&   normals() const;
  const pointField&         points() const;
  triSurface&         triSurf() const;
        vector              getFaceVelocity(label idx) const;
        vector              getPointVelocity(const point& p) const;
        point               cg() const { return cg_; }

        void                isInside(
                                      const pointField& samples,
                                      const boundBox& bb,
                                      List<bool>& inside
                                    ) const;
void scaleSTL(scalar scaleFactor, scalar densityFactor); // for soot burn
scalar massRatio_;
scalar sTemp_;
scalar sArea_;

struct vecCmpt
{
  vector  n;          // unit normal vector
  vector  t;          // unit tangent vector
  scalar  normal;     // normal component
  scalar  tangential; // tangential component
  scalar  mag;        // || normal*n + tangential*t ||
};

vecCmpt       getRelVelocityComponents(
                                        label idx,
                                  const volumetricParticle *slave,
                                        label slaveIdx
                                      ) const;

vecCmpt       getRelVelocityComponents(
                                        label idx,
                                        const vector &pos
                                      ) const;

vecCmpt       getDistanceComponents   (
                                        label idx,
                                  const volumetricParticle *slave,
                                        label slaveIdx
                                      ) const;

vecCmpt       getDistanceComponents   (
                                        label idx,
                                        const vector &pos
                                      ) const;

void          defineContact           (
                                        label idx,
                                  const volumetricParticle *slave,
                                        label slaveIdx,
                                        contactKinetic  &cK,
                                        contactMechanic &cM
                                      ) const;

void          defineContact           (
                                        label idx,
                                        const vector &pos,
                                        contactKinetic  &cK,
                                        contactMechanic &cM
                                      ) const;

scalar  getGenTime() const { return genTime_; }
scalar  getAge() const { return (time_.value() - genTime_); }

void	setExternals(vector exAcc, vector exOAcc, vector exF, vector exT); // External acceleration, force and torque
vector& getCg();
tensor& getOrientation();
vector& getTotalForce();
vector& getTotalTorque();
vector& getAverageVelocity();
vector& getVelocity();
vector getKineticEnergy();
vector getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad=false);
scalar& getRho();
scalar& getMass();
symmTensor& getJ();
symmTensor& getAggloJ();
vector& getAverageOmega();
vector& getOmega();
vector& getAcceleration();
vector& getOmegaAcc();
vector& getDispl();
vector& getDisplNext();
vector& getRotNext();

void move(scalar relax, const List<autoPtr<Constraint> >& constraintList, int subiteration);
void moveVolumetricParticle(scalar relax, const List<autoPtr<Constraint> >& constraintList);
void movePointParticle(scalar relax, const List<autoPtr<Constraint> >& constraintList, int subiteration);
bool isPartner(volumetricParticle* partner);
void deleteParticlesFromList(List<volumetricParticle*>& deleteList);
void moveWithContactPartners(scalar relax);
void reduceJitter();
void addContactPartnersPartners();
void recursivePartners(HashTable<volumetricParticle*> &allPartners);
void bounce(vector n_vec, scalar k_pl, scalar mu);
void moveSlave(const vector &e, const vector & displ, scalar relax);
void subCyclingSaveState();
void subCyclingSavePosition();
void subCyclingSavePreCollision();
void subCyclingSaveVelocities();
void subCyclingRestoreState();
void subCyclingRestorePreCollision();
void saveIntermediateState();
void restoreIntermediateState();
void iterativeCouplingSavePoints();
void iterativeCouplingSaveForces();
void iterativeCouplingRestorePoints();
void iterativeCouplingRestoreForces();
void iterativeCouplingRelaxForces(scalar relax);

  void writeGeometry() const;
  void writeProperties();
  void printParticleData();
  void endOfExecution();

  void calcTotalLoad();
  void calcTotalLoadGeneral();
  void calcTotalLoadConditioned();
  void calcTotalLoadNoAdhesion();
  void calcTotalLoadSubCellSize();
  void calcCollisionLoad();
  void subtractContactsFromTotalLoad();
  void calcAcceleration();
  void calcVelocity(scalar relax);
  void setContactPointsWithStructure( List<int>& faceList, vector avgNormal, vector avgCenter );

  void orientationToEulerAxis(vector& e) const;

void reloadSTL();
void resolveAdhesion(volumetricParticle& partner);
void findClosestFaces(volumetricParticle& partner, label& myFace, label& prtFace);


template <typename Type>
void distributeValue(Type value);

void kinetic();
void rotateContactComponents(tensor& rot);

void calcAggloJ();

private:

  void scaleMesh();
  void calcMassAndCG();
  void calcJ();
//  void calcTotalLoad();
//  void calcAcceleration();
//  void calcVelocity();


  void parallelAdjustmentConstraints( vector tempV, vector tempVAvg, vector tempOmega, vector tempOmegaAvg);

  pointField& savedSubcyclingPoints();
  pointField& savedPreCollPoints();
  pointField& savedIntermediatePoints();
  pointField& savedIterativeCouplingPoints();


  void prepareFiles() const;

  void               discardTriSurfSearch() const;
  void               discardFields();

  word                        idStr_;
  label                       populationId_;

  scalar                      genTime_;

  stateType                   state_;
  const Time                 &time_;
  //fileName                    meshPath_;
  mutable autoPtr<triSurface>         stlPtr_;
  mutable autoPtr<vectorField>        sfPtr_;
  mutable autoPtr<vectorField>   normalsPtr_;
  mutable bool                      validSf_;
  mutable bool                 validNormals_;
  bool                        topoChanged_;
  autoPtr<vectorField>        fluidForcePtr_;
  autoPtr<vectorField>        solidForcePtr_;
  autoPtr<vectorField>        thermoForcePtr_;
  autoPtr<vectorField>        electromagForcePtr_;
  autoPtr<vectorField>        contactForcePtr_;
  autoPtr<vectorField>        savedFluidForcePtr_;
  autoPtr<vectorField>        savedSolidForcePtr_;
  autoPtr<vectorField>        savedThermoForcePtr_;
  autoPtr<vectorField>        savedElectromagForcePtr_;
  autoPtr<symmTensorField>    stressPtr_;
  autoPtr<scalarField>        pressurePtr_;
  autoPtr<scalarField>        thermoPtr_;
  autoPtr<vectorField>        electromagPtr_;
  autoPtr<scalarField>        sigmaPtr_;
  autoPtr<pointField>         shareVectorPtr_;
  autoPtr<scalarField>        shareScalarPtr_;
  autoPtr<vectorField>        polarPtr_;
  word                        surfacePatchName_;
  bool                        writeForceField_;
  bool                        writePressureForceField_;
  bool                        writePressureForceDensityField_;
  bool                        writeStressForceField_;
  bool                        writeStressForceDensityField_;

  mutable autoPtr<triSurfaceSearch>   triSurfaceSearchPtr_;

  MPI_Comm                    myProcs;

  // Geometrical properties:
  //scalar           scale_; // scale factor according to orig. particle mesh
  scalar             rho_; // density
  scalar            mass_; // total mass
  point               cg_; // centre of gravity
  vector           displ_; // displacement with reference to orig. particle mesh
  symmTensor           J_; // moment of inertia tensor
  symmTensor           aggloJ_; // moment of inertia tensor of the particle's agglomerate
  vector           displNext_; // the displacement for the current movement, used in ::kinetic
  vector           rotNext_;   // the rotation axis for the current movement, used in ::kinetic

  // Movement: velocity, angular velocity,
  // acceleration and angular acceleration
  // and old values respectively
  vector           velo_;
  vector           veloAvg_;
  vector           omega_;
  vector           omegaAvg_;
  vector           acc_;
  vector           omegaAcc_;
  vector           totalForce_;
  vector           totalTorque_;
  vector           externalAcc_;
  vector		   externalOmegaAcc_;
  vector           externalForce_;
  vector           externalTorque_;
  tensor           orientation_;

  // for subcycling (prefix sc): trailing '0' indicates saved state from
  // the beginning of the time step
  point               sc_cg0_;
  vector              sc_displ0_;
  symmTensor          sc_J0_;
  vector              sc_velo0_;
  vector              sc_veloAvg0_;
  vector              sc_omega0_;
  vector              sc_omegaAvg0_;
  vector              sc_totalForce0_;
  vector              sc_totalTorque0_;
  tensor              sc_orientation0_;
  autoPtr<pointField> savedSubcyclingPointsPtr_;
  // for intermediate (prefix it) saving of state
  point               it_cg0_;
  vector              it_displ0_;
  vector              it_displNext0_;
  vector              it_rotNext0_;
  symmTensor          it_J0_;
  vector              it_velo0_;
  vector              it_veloAvg0_;
  vector              it_omega0_;
  vector              it_omegaAvg0_;
  vector              it_totalForce0_;
  vector              it_totalTorque0_;
  tensor              it_orientation0_;
  autoPtr<pointField> savedIntermediatePointsPtr_;
  // for smoother resolution of penetration in rebounding collisions these
  // save states are used separately to those above (prefix preColl)
  point               preColl_cg0_;
  vector              preColl_displ0_;
  symmTensor          preColl_J0_;
  tensor              preColl_orientation0_;
  autoPtr<pointField> savedPreCollPointsPtr_;
  // for iterativeCoupling (prefix ic): trailing '0' indicates saved state from
  // the beginning of the time step
  point               ic_cg0_;
  vector              ic_displ0_;
  symmTensor          ic_J0_;
  vector              ic_velo0_;
  vector              ic_omega0_;
  tensor              ic_orientation0_;
  autoPtr<pointField> savedIterativeCouplingPointsPtr_;

};


} // namespace Foam

#endif // volumetricParticle_H
