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

#ifndef population_H
#define population_H

#include "fileName.H"
#include "vector.H"
#include "HashTable.H"
#include "treeDataCell.H"
#include "LList.H"

#include "volumetricParticle.H"
#include "LSMIOdictionary.H"
#include "bgGrid.H"
#include "injector.H"
#include "constraint.H"

#include "myMeshSearch.H"


namespace Foam
{

class Population
{
public:

  typedef Tuple2<word, List<scalar> >  journalEntry;


  Population();

  ~Population();

  word  name() { return name_; }
  label id()   { return id_; }

  void  read(
               const objectRegistry &obr,
               const bgGrid &bg,
               const myMeshSearch *myMSPtr,
               const dictionary &dict,
               const word &popName,
               label popId
            );

  label restoreFromJournal();
  void distributeParticles(
                            const bgGrid& bg
                          );
  void distributeParticlesMPI(
                            const bgGrid& bg
                          );

  template<typename Type>
  void distributeForcesNew(
                         Field<Type>& (volumetricParticle::* fieldGetter) ()
                       );

  /*template<typename Type>
  void distributeForces(
                         Field<Type>& (volumetricParticle::* fieldGetter) ()
                       );*/

  void distributePointVelocity();

  void distributeParticleValues();

  template<typename Type>
  void dynComm4FieldDist(
                         const bgGrid& bg,
                         Field<Type>& (volumetricParticle::* fieldGetter) ()
                       );

  void writeParticlePropertiesAndGeometries(const bgGrid& bg);
  void writeJournal();
  void move(scalar relax, int subiteration);
  void checkForContacts(List<volumetricParticle*> allParticles);
  // compute adhesion and friction near the wall
  void checkNearWallEffects();
  void subCyclingSaveState();
  void subCyclingRestoreState();
  void iterativeCouplingSavePoints();
  void iterativeCouplingSaveForces();
  void iterativeCouplingRestorePoints();
  void iterativeCouplingRestoreForces();
  void iterativeCouplingRelaxForces(scalar relax);
  void inject();
  void deleteParticle(word &key);
  void deleteParticlesBB();
  void endOfExecution();
  label size() const { return container_.size(); }
  const HashTable<volumetricParticle*>& container() const { return container_; }
  const List<autoPtr<Constraint> >& constraintList() const { return constraintList_; }
  bool isStructure() const { return isStructure_; }


  volumetricParticle* findParticleByIdStr(const word& idStr);

  bool withThermoPhoresis() const { return thermophoresis_; }
  bool withElectroMagnetic() const { return electromag_; }
  bool withContactCheck() const {return contactCheck_; }
  bool withNearWallEffects() const { return nearWallEffects_; }
  bool isPointParticle() const { return isPointParticle_; }
  bool mapPointParticleMomentum() const { return mapPointParticleMomentum_; }
  bool collidesWithOwnPopulation() const { return collidesWithOwnPopulation_; }
  scalar thermophoreticFactor() const { return thermophoreticFactor_; }
  scalar objectCharge() const { return objectCharge_; }
  scalar epsilonr() const { return epsilonr_; }//returns particles relative permittivity
  scalar a0() const { return a0_; } // returns contact radius
  scalar H() const { return H_; } // return hamaker constant
  scalar elasticity() const { return elasticity_; } // return elasticity constant
  scalar kpl() const { return kpl_; } // return restitution coeff.
  scalar ppl() const { return ppl_; } // return plastic deformation pressure
  scalar mu() const { return mu_; } // return friction coeff.
  scalar collDist() const { return collDist_; } // return distance for collisions
  scalar mappingDist() const { return mappingDist_; } // return distance for voidFraction mapping
  scalar contactPointDist() const { return contactPointDist_; } // Return min. distance bewteen two contact points
  scalar collForceConservation() const { return collForceConservation_; } // return energy conservation factor for collisions
  double adhesionReductionFactor() const { return adhesionReductionFactor_; }
  word calcTotalLoadType() const { return calcTotalLoadType_; }

  void nParticlesProc() const;
  void printStats() const;

  void pOxidation(const bgGrid& bg); //SootBurn
  void pOxidation(); //SootBurn with constant temperature
  bool withOxidation() const { return oxidation_;}
  scalar getAf() const { return af_;}
  scalar getActEnergy() const { return aEnergy_;}
  scalar getRmol() const { return rGasConst_;}
  scalar getRTemp() const { return rTemp_;}
  Switch generatePostprocFiles() const { return generatePostprocFiles_; }

protected:

  volumetricParticle*  instantiateParticle(
                                            const List<scalar>& valueList,
                                            bool inMeshOnly = false
                                          );
  volumetricParticle*  instantiateParticle(
                                            const word& keyName,
                                            const List<scalar>& valueList,
                                            bool inMeshOnly = false
                                          );
  volumetricParticle*  instantiateParticle(
                                            const word& keyName,
                                            const scalar* valueList,
                                            bool inMeshOnly = false
                                          );
  void  renameParticle(const word& oldName, const word& newName);
  label generateParticleId();
  label degenerateParticleId();
  label getParticleId();

  const objectRegistry      *obr_;
  const dictionary         *dict_;
  word                      name_;
  word          surfacePatchName_;
  word			calcTotalLoadType_;
  label                       id_;
  fileName              meshPath_;
  fileName            outputPath_;
  fileName          polyMeshPath_;
  label           nextParticleId_;
  label           bgSearchRadius_;
  scalar                     rho_;
  vector                 gravity_;
  vector             externalAcc_;
  vector             externalOmegaAcc_;
  vector           externalForce_;
  vector          externalTorque_;
  vector		  minVecBB_;
  vector		  maxVecBB_;
  Switch          thermophoresis_;
  Switch	electromag_;
  Switch	isStructure_;
  Switch    contactCheck_;
  Switch    nearWallEffects_;
  Switch	oxidation_; //Vora
  Switch	isPointParticle_;
  Switch    mapPointParticleMomentum_;
  Switch	collidesWithOwnPopulation_;
  Switch	distributeToAll_;
  scalar    thermophoreticFactor_;
  scalar	objectCharge_;
  scalar	epsilonr_;
  scalar    a0_;
  scalar    H_;
  double	adhesionReductionFactor_;
  scalar    kpl_;
  scalar    ppl_;
  scalar 	collForceConservation_;
  scalar	elasticity_;
  scalar    mu_; // Friction coeff.
  scalar	collDist_; // Unscaled distance for collision detection#
  scalar	mappingDist_; // Unscaled distance for mapping of voidFraction
  scalar	contactPointDist_;
  scalar	af_;//Pre-exponential factor or frequency factor (Af) for oxidation [1/(Pa*sec)]
  scalar	aEnergy_;//Activation energy for oxidation (Ea) in [J/mol]
  scalar	rGasConst_; //molar Gas constant - 8.3144598(48) [J/(K*mol)]
  scalar	rTemp_; // reference temperature for oxidation if no temperature field is calculated

  HashTable<volumetricParticle*> container_;
  const bgGrid*               bg_;
  autoPtr<Injector>     injector_;
  List<autoPtr<Constraint> > constraintList_;

  label           nBornParticles_;

  Switch         writeProperties_; // Should folder with particleProperties be generated in each written time step
  Switch         writeForceField_;
  Switch         writePressureForceField_;
  Switch         writePressureForceDensityField_;
  Switch         writeStressForceField_;
  Switch         writeStressForceDensityField_;
  Switch         generatePostprocFiles_; // Should stl-softlinks be generated for STL visualization in ParaView

  Switch	deleteOrphanedParticles_; //Vora:

  const myMeshSearch *myMSPtr_;

  // contiguous type for mpi transfer of particle values
  struct pTransValues
  {
    char   pIdStr[_N_DIGITS_PARTICLE_ID_];
    scalar  valueList[_N_PARTICLE_PARAMETERS_];
  };

  struct pTransForce
  {
    char          pIdStr[_N_DIGITS_PARTICLE_ID_];
    const char*  forceBuffer;
  };

  // A list with one element for each processor:
  // pDistValuesPlan[procNo] is a DLList containing an entry for each
  // particle to be sent to processor number procNo.
  // pDistValuesPlan[procNo][n] contains a pTransValues holding the parameters
  // needed to define the n-th particle to be sent to procNo
  typedef LList< DLListBase, pTransValues > pTransValuesDLList;
  typedef List< pTransValues >              pTransValuesList;
  typedef List< pTransValuesDLList >        pDistValuesPlan;
  // Contiguous distribution plan; ready for send/recv via buffer
  typedef List< pTransValuesList >          pDistValuesPlanCont;

  // Analogously to transfer of values
  typedef LList< DLListBase, pTransForce > pTransForceDLList;
  typedef List< pTransForce >              pTransForceList;
  typedef List< pTransForceDLList >        pDistForcePlan;
  typedef List< pTransForceList >          pDistForcePlanCont;

  // statistic information
  mutable List<label>   nFree_;   // number of free particles
  mutable List<label>   nMaster_; // number master particles
  mutable List<label>   nSlave_;  // number of slave particles



};

} // namespace Foam

#endif  // headerguard
