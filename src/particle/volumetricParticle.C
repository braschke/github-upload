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

#include "volumetricParticle.H"
#include <stdio.h>
#include "quaternion.H"

#include "makros.H"
#include "tools/other/write.H"
#include "mpi.h"

#include <ctime>
#include <chrono>

#include "../pManager/population/population.H"

#include <unistd.h> // Needed for fast generation of symbolic links


namespace Foam
{

  volumetricParticle::volumetricParticle(
                                            const word     &idStr,
                                            const fileName &pathToMesh,
                                                  label     popId,
                                                 scalar     rho,
                                            const Time     &runTime,
                                            const bgGrid&	bg,
													label	bgSearchRadius_,
                                                 stateType  state,
                                            const word     &surfacePatchName,
                                                  bool      writeForceField,
                                                  bool      writePressureForceField,
                                                  bool      writePressureForceDensityField,
                                                  bool      writeStressForceField,
                                                  bool      writeStressForceDensityField,
											      Population* myPop
                                             ):
  idStr_(idStr),
  populationId_(popId),
  genTime_(runTime.value()),
  state_(state),
  time_(runTime),
  meshPath_(pathToMesh),
  stlPtr_(0),
  sfPtr_(0),
  validSf_(false),
  validNormals_(false),
  topoChanged_(false),
  fluidForcePtr_(0),
  solidForcePtr_(0),
  thermoForcePtr_(0),
  electromagForcePtr_(0),
  contactForcePtr_(0),
  savedFluidForcePtr_(0),
  savedSolidForcePtr_(0),
  savedThermoForcePtr_(0),
  savedElectromagForcePtr_(0),
  stressPtr_(0),
  pressurePtr_(0),
  thermoPtr_(0),
  electromagPtr_(0),
  sigmaPtr_(0),
  shareVectorPtr_(0),
  shareScalarPtr_(0),
  polarPtr_(0),
  surfacePatchName_(surfacePatchName),
  writeForceField_(writeForceField),
  writePressureForceField_(writePressureForceField),
  writePressureForceDensityField_(writePressureForceDensityField),
  writeStressForceField_(writeStressForceField),
  writeStressForceDensityField_(writeStressForceDensityField),
  triSurfaceSearchPtr_(),
  bg_(&bg),
  scale_(1),
  massRatio_(1),
  sTemp_(0),
  sArea_(0),
  rho_(rho),
  mass_(0.0),
  dk_(0.0),
  contactNormal_(vector::zero),
  cg_(point::zero),
  commonCg_(vector(GREAT, GREAT, GREAT)),
  commonForce_(vector(GREAT, GREAT, GREAT)),
  commonTorque_(vector(GREAT, GREAT, GREAT)),
  commonVelo_(vector(GREAT, GREAT, GREAT)),
  commonOmega_(vector(GREAT, GREAT, GREAT)),
  deposited_(false),
  contactPoint_(vector::zero),
  displ_(vector::zero),
  J_(I),
  displNext_(vector::zero),
  rotNext_(vector::zero),
  velo_(vector::zero),
  veloAvg_(vector::zero),
  omega_(vector::zero),
  omegaAvg_(vector::zero),
  acc_(vector::zero),
  omegaAcc_(vector::zero),
  totalForce_(vector::zero),
  totalTorque_(vector::zero),
  orientation_(I),
  sc_cg0_(point::zero),
  sc_displ0_(point::zero),
  sc_J0_(I),
  sc_velo0_(vector::zero),
  sc_omega0_(vector::zero),
  sc_orientation0_(I),
  savedSubcyclingPointsPtr_(0),
  ic_cg0_(point::zero),
  ic_displ0_(point::zero),
  ic_J0_(I),
  ic_velo0_(vector::zero),
  ic_omega0_(vector::zero),
  ic_orientation0_(I),
  savedIterativeCouplingPointsPtr_(0),
  aggloJ_(symmTensor::zero),
  myPop_(myPop)
  {
    if(myPop_->generatePostprocFiles()) prepareFiles();

    stlPtr_.reset(
//                   new triSurface(time_.path()/idStr_ + ".stl")
                   new triSurface(time_.path()/meshPath_.name())
                 );

}

  volumetricParticle::~volumetricParticle()
  {
    if(Pstream::parRun())
    {
//      MPI_Comm_free(&myProcs);
    }
  }

/*void volumetricParticle::prepareFiles() const
{
  string cdCmd = "cd "+ time_.path() + "/";

  string noOutput = " &> /dev/null";
  string lnCmd = " ln -sf `basename " + meshPath_ + " ` " + idStr_ + ".stl";
  //_PDBO_("lnCmd = " << lnCmd)
  system(cdCmd + noOutput  + " ; " + lnCmd + noOutput);
//_PDBO_("\n\n command " << (cdCmd + noOutput  + " ; " + lnCmd + noOutput) << "\n\n")

  if(time_.processorCase())
	  cdCmd += "../";

  lnCmd = " ln -sf " + idStr_.substr(0, idStr_.find_first_of("-")) + " " + idStr_;
  system(cdCmd + "constant; " + lnCmd + noOutput);
}*/

void volumetricParticle::prepareFiles() const
{
  // Generate links in case directory
  string linkTo = time_.path() + "/" + meshPath_;
  string linkName = time_.path() + "/" + idStr_ + ".stl";
  char linkTo_char[linkTo.length() + 1];
  strcpy(linkTo_char, linkTo.c_str());
  char linkName_char[linkName.length() + 1];
  strcpy(linkName_char, linkName.c_str());
  symlink(linkTo_char, linkName_char);

  // Generate links in constant directory
  linkTo = time_.path() + "/constant/" + idStr_.substr(0, idStr_.find_first_of("-"));
  linkName = time_.path() + "/constant/" + idStr_;
  if(Pstream::parRun())
  {
	linkTo = time_.path() + "/../constant/" + idStr_.substr(0, idStr_.find_first_of("-"));
	linkName = time_.path() + "/../constant/" + idStr_;
  }
  char linkToConst_char[linkTo.length() + 1];
  strcpy(linkToConst_char, linkTo.c_str());
  char linkNameConst_char[linkName.length() + 1];
  strcpy(linkNameConst_char, linkName.c_str());
  symlink(linkToConst_char, linkNameConst_char);
}


triSurfaceSearch&  volumetricParticle::triSurfSearch() const
{
  if( !triSurfaceSearchPtr_.valid() || topoChanged_ )
  {
      triSurfaceSearchPtr_.reset(
                               new triSurfaceSearch( triSurf() )
                             );
  }

  return triSurfaceSearchPtr_();
}

void  volumetricParticle::discardTriSurfSearch() const
{
  triSurfaceSearchPtr_.clear();
}

void  volumetricParticle::discardFields()
{
	_PDBO_("discardingFields")
  fluidForcePtr_.clear();
  solidForcePtr_.clear();
  thermoForcePtr_.clear();
  electromagForcePtr_.clear();
  contactForcePtr_.clear();
  stressPtr_.clear();
  pressurePtr_.clear();
  thermoPtr_.clear();
  electromagPtr_.clear();
  sigmaPtr_.clear();
  shareVectorPtr_.clear();
  shareScalarPtr_.clear();
  polarPtr_.clear();
//  sfPtr_.clear();
//  normalsPtr_.clear();
}

void volumetricParticle::setFree()
{
  state_ = free;
  /*idStr_[0] = 'f';*/
}

void volumetricParticle::setMaster()
{
  state_ = master;
  /* idStr_[0] = 'm';*/
}

void volumetricParticle::setSlave()
{
  state_ = slave;
  /*idStr_[0] = 's';*/
  discardTriSurfSearch();
}

void volumetricParticle::valuesToList(List<scalar>& list) const
{
  list.resize(_N_PARTICLE_PARAMETERS_);

  vector  eulerAxis;

  orientationToEulerAxis(eulerAxis);

  label i = 0;

  list[i++] = scale_;
  list[i++] = displ_.x();
  list[i++] = displ_.y();
  list[i++] = displ_.z();
  list[i++] = velo_.x();
  list[i++] = velo_.y();
  list[i++] = velo_.z();
  list[i++] = omega_.x();
  list[i++] = omega_.y();
  list[i++] = omega_.z();
  list[i++] = eulerAxis.x();
  list[i++] = eulerAxis.y();
  list[i++] = eulerAxis.z();
  list[i++] = getAge();
}

void volumetricParticle::valuesToList(scalar* list) const
{
  // list must be preallocated with _N_PARTICLE_PARAMETERS_ elements

  vector  eulerAxis;

  orientationToEulerAxis(eulerAxis);

  label i = 0;

  list[i++] = scale_;
  list[i++] = displ_.x();
  list[i++] = displ_.y();
  list[i++] = displ_.z();
  list[i++] = velo_.x();
  list[i++] = velo_.y();
  list[i++] = velo_.z();
  list[i++] = omega_.x();
  list[i++] = omega_.y();
  list[i++] = omega_.z();
  list[i++] = eulerAxis.x();
  list[i++] = eulerAxis.y();
  list[i++] = eulerAxis.z();
  list[i++] = getAge();
}

void volumetricParticle::reloadSTL()
{
    stlPtr_.reset(
                   new triSurface(time_.path()/meshPath_.name())
                 );

    topoChanged_ = true;
    _DBO_(" set topoChanged_ to " << topoChanged_<< " for particle " << idStr_ )
    // topoChanged_ otherwise seems to be ignored for WHATEVER reason
    // Therefore call normals and other geom. now so it's available later
    normals();
    Sf();
    Cf();
    points();

    scaleMesh();
    triSurfSearch();
    discardFields();
    topoChanged_ = false;
    /*calcMassAndCG();
    calcJ();*/
}

void volumetricParticle::processSkiJump()
{
  // set parameters relative to new stl-geometry which is in absolute coords
  scale_        = 1;
  displ_        = vector::zero;
//  velo_         = newVelo;
//  omega_        = vector::zero;
  orientation_  = I;

  stlPtr_.reset(
                 new triSurface(time_.path()/idStr_ + ".stl")
               );

  topoChanged_ = true;

  scaleMesh();

//  calcMassAndCG();
//  calcJ();
_DBO_("Q'n'D hack")
  writeGeometry();
  mass_ = 90;
  J_ = symmTensor(1, 0, 0, 1, 0, 1);
}

void volumetricParticle::listToValues(const List<scalar>& list)
{
  vector  displ;
  vector  eulerAxis;

  label i = 0;

  scale_        = list[i++];
  displ.x()     = list[i++];
  displ.y()     = list[i++];
  displ.z()     = list[i++];
  velo_.x()     = list[i++];
  velo_.y()     = list[i++];
  velo_.z()     = list[i++];
  omega_.x()    = list[i++];
  omega_.y()    = list[i++];
  omega_.z()    = list[i++];
  eulerAxis.x() = list[i++];
  eulerAxis.y() = list[i++];
  eulerAxis.z() = list[i++];
  genTime_      = time_.value() - list[i++];

  scaleMesh();

  calcMassAndCG();
  calcJ();

  rotNext_   = eulerAxis;
  displNext_ = displ;
  kinetic();
}

void volumetricParticle::listToValues(const scalar* list)
{
  vector  displ;
  vector  eulerAxis;

  label i = 0;

  scale_        = list[i++];
  displ.x()     = list[i++];
  displ.y()     = list[i++];
  displ.z()     = list[i++];
  velo_.x()     = list[i++];
  velo_.y()     = list[i++];
  velo_.z()     = list[i++];
  omega_.x()    = list[i++];
  omega_.y()    = list[i++];
  omega_.z()    = list[i++];
  eulerAxis.x() = list[i++];
  eulerAxis.y() = list[i++];
  eulerAxis.z() = list[i++];
  genTime_      = time_.value() - list[i++];

  _DBO_("now mesh should be scaled")
  scaleMesh();

  calcMassAndCG();
  calcJ();

  rotNext_   = eulerAxis;
  displNext_ = displ;
  kinetic();
}

    void volumetricParticle::defineValues(
                                                  scalar  initialScale,
                                            const vector &initialDisplacement,
                                            const vector &velocity,
                                            const vector &angularVelocity,
                                            const vector &initialRotation
                                          )
{
  scale_       = initialScale;
  displ_       = initialDisplacement;
  velo_        = velocity;
  omega_       = angularVelocity;
  orientation_ = *initialRotation; // Hodge dual

  scaleMesh();
  calcMassAndCG();
  calcJ();
  rotNext_   = initialRotation;
  displNext_ = displ_;
  kinetic();
}

void volumetricParticle::scaleMesh()
{
    pointField p = points();
    p *= scale_;
    stlPtr_().movePoints(p);

    _DBO_("scaling Mesh by " << scale_)
    validSf_      = false;
    validNormals_ = false;
}

// For soot burn
void volumetricParticle::scaleSTL(scalar scaleFactor, scalar densityFactor)
{
    scale_ *= scaleFactor;
    pointField p = points();
    p -= cg_;
    p *= scaleFactor;
    p += cg_;
    stlPtr_().movePoints(p);
    rho_ *= densityFactor;
    _DBO_("scaling STL with factor " << scaleFactor)
  	validSf_      = false;
    validNormals_ = false;
    calculatedAgglo_ = false;

    calcMassAndCG();
    calcJ();
}

void volumetricParticle::calcMassAndCG()
{
  /*
   * The volume is \int_V 1 dV
   * f := \begin{pmatrix} x \\ 0 \\ 0 \end{pmatrix}
   * => \nabla\cdot f = 1
   * => \int_V 1 dV = \int_V \nabla\cdot f dV = \int_A f dA
   */
  const vectorField &c = Cf();
  const vectorField &a = Sf();
  //const scalarField &test = magSf();

  mass_ = 0.;
  radEVS_ = 0.;
  scalar area = 0.;
  _DBO_("CG DEBUG L 458")
  forAll(c, faceI)
  {
    //addup volume of all faces respective to their orientation
    mass_ += c[faceI].x() * a[faceI].x();
  }
  if(mass_<0){
    _PDBO_("WARNING: your mass is negative, we ")
    mass_ *=-1;
  }
  radEVS_ = pow((3.*mass_/(4.*constant::mathematical::pi)), (1.*1/3));
  // _PDBO_("Volume is: " << mass_)
  // Now, mass_ contains the total volume
  /*
   * Calculate volume weighted coordinates:
   * \int_V \begin{pmatrix} x \\ y \\ z \end{pmatrix} dV =
   * \begin{pmatrix} \int_V x dV \\ \int_V y dV \\ \int_V z dV \end{pmatrix}
   * \int_V x dV = \int_V \nabla\cdot \begin{pmatrix} 0\\ xy\\ 0 \end{pmatrix} dV
   * = \int_A \begin{pmatrix} 0\\ xy\\ 0 \end{pmatrix} dA
   * Analogously for \int_V y dV and \int_V z dV
   */
  cg_ = vector::zero;
  forAll(c, faceI)
  {
      cg_.x() += c[faceI].x() * c[faceI].y() * a[faceI].y();
      cg_.y() += c[faceI].y() * c[faceI].z() * a[faceI].z();
      cg_.z() += c[faceI].z() * c[faceI].x() * a[faceI].x();
  }
  // Divide volume weighted coordinates by total volume:
  cg_ /= mass_;
  //_DBO_("CG is  " << cg_)
  // Now, mass_ contains total mass
  mass_ *= rho_;
  // Change sign <= inverted cell normals of stl
//  mass_ *= -1;
//  _DBO_("Mass = " << mass_ << ", Density = " << rho_ << " and volume = " << mass_/rho_)

}



void volumetricParticle::calcJ()
{
  const vectorField &c = Cf();
  const vectorField &a = Sf();
  /*
   * Calculate moment of inertia tensor using above technique:
   *
   * J_.xy() = - \int_A (0   0   xyz) dA
   * J_.xz() = - \int_A (0   xzy 0  ) dA
   * J_.yz() = - \int_A (yzx 0   0  ) dA
   * J_.xx() =   \int_A (zzx 0   yyz) dA
   * J_.yy() =   \int_A (zzx xxy 0  ) dA
   * J._zz() =   \int_A (0   xxy yyz) dA
   */

  J_ = symmTensor::zero;

  forAll(c, faceI)
  {
    vector cf  = c[faceI] - cg_; // make relative to cg
    scalar xyz = cf.x() * cf.y() * cf.z();
    scalar zzx = cf.z() * cf.z() * cf.x();
    scalar xxy = cf.x() * cf.x() * cf.y();
    scalar yyz = cf.y() * cf.y() * cf.z();

    J_.xy() += xyz * a[faceI].z();
    J_.xz() += xyz * a[faceI].y();
    J_.yz() += xyz * a[faceI].x();
    J_.xx() += zzx * a[faceI].x() +                      yyz * a[faceI].z();
    J_.yy() += zzx * a[faceI].x() + xxy * a[faceI].y()                     ;
    J_.zz() +=                      xxy * a[faceI].y() + yyz * a[faceI].z();
  }
  J_ *= rho_;
  // Change sign <= inverted cell normals of stl
//  J_ *= -1;
//_DBO_("Moment of inertia tensor = " << J_)
}

bool volumetricParticle::isPointParticle()
{
	return myPop_->isPointParticle();
}

void volumetricParticle::move(scalar relax, const List<autoPtr<Constraint> >& constraintList, int subiteration)
{
	if(!isPointParticle()) moveVolumetricParticle(relax, constraintList);
	else movePointParticle(relax, constraintList, subiteration);
}

void volumetricParticle::movePointParticle(scalar relax, const List<autoPtr<Constraint> >& constraintList, int subiteration)
{
	calcTotalLoadSubCellSize(); // Does nothing currently

	omegaAvg_ = vector::zero;
	omega_    = vector::zero;
	veloAvg_  = velocityAtCgSubCellSize_;
	velo_     = veloAvg_;

	scalar    dt    = time_.deltaT().value();
	rotNext_   = relax * dt * omegaAvg_;  // rotation axis times angle
	displNext_ = relax * dt * veloAvg_;   // translational displacement

	kinetic();

	// apply position constraints here:
	forAll(constraintList, I)
	{
	  const Constraint &c = constraintList[I]();
	  if( ! c.modifiesPosition() )
	    continue;

	  c.constrain(*this);
	}

}

void volumetricParticle::moveVolumetricParticle(scalar relax, const List<autoPtr<Constraint> >& constraintList)
{
  calcTotalLoadNoAdhesion();

  // apply force constraints here:
  forAll(constraintList, I)
  {
    const Constraint &c = constraintList[I]();
    if( ! c.modifiesForce() )
      continue;

    c.constrain(*this);
  }

  calcAcceleration();
  forAll(constraintList, I)
  {
    const Constraint &c = constraintList[I]();
    if( ! c.modifiesAcceleration() )
      continue;

    c.constrain(*this);
  }

  calcVelocity(relax);
  forAll(constraintList, I)
  {
    const Constraint &c = constraintList[I]();
    if( ! c.modifiesVelocity() )
      continue;

    // Ugly but necessary for parallel implementations of
  	// mesh dependent velocity constraints
  	vector tempV = velo_;
  	vector tempVAvg = veloAvg_;
  	vector tempOmega = omega_;
  	vector tempOmegaAvg = omegaAvg_;

    c.constrain(*this);

  }

  // Reduce jitter for particles that neglect adhesion
  if(!myPop_->H()) reduceJitter();

  scalar    dt    = time_.deltaT().value();
  rotNext_   = relax * dt * omegaAvg_;  // rotation axis times angle
  displNext_ = relax * dt * veloAvg_;   // translational displacement

#if 0
  _DBO_("\n**********" <<
  		"\n" << idStr() <<
		"\nrelax = " << relax <<
  		"\nactual rot = " << rotNext_ <<
  		"\nactual displ = " << displNext_ <<
  		"\n**********")
#endif

// apply position constraints here:
  forAll(constraintList, I)
  {
    const Constraint &c = constraintList[I]();
    if( ! c.modifiesPosition() )
      continue;

    c.constrain(*this);
  }
#if 0
    _PDBO_("move before " << idStr_ <<
        "\nrotNext_ = "<< rotNext_<<
        "\ndisplNext_ = "<< displNext_<<
        "\nori_ = "<< orientation_<<
        "\ncg_ = "<< cg_<<
        "\ntotalForce_ = "<< totalForce_<<
        "\ntotalTorque_ = "<< totalTorque_ <<
        "\nTotal Acc = " << acc_<<
        "\nTotal velo = " << velo_<<
        "\nTotal OmegaAcc = " << omegaAcc_ <<
        "\nTotal Omega = " << omega_)
#endif

  kinetic();

#if 0
    _PDBO_("move after " << idStr_ <<
        "\nrotNext_ = "<< rotNext_<<
        "\ndisplNext_ = "<< displNext_<<
        "\nori_ = "<< orientation_<<
        "\ncg_ = "<< cg_<<
        "\ntotalForce_ = "<< totalForce_<<
        "\ntotalTorque_ = "<< totalTorque_ <<
        "\nTotal Acc = " << acc_<<
        "\nTotal velo = " << velo_<<
        "\nTotal OmegaAcc = " << omegaAcc_ <<
        "\nTotal Omega = " << omega_)
#endif


  discardTriSurfSearch();
}


// Calculation of the kinetic energy of a whole agglomerate.
// Method is needed for the collision force resolution in
// pManager::checkForCollisions(). Most parts are taken from
// the method moveWithContactPartners(). Returns a kinetic energy
// vector analogous to method getKineticEnergy().
vector volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)
{
	vector aggloKineticEnergy;

	//_PDBOP_("idstr is " << idStr(), 3)

	// Needs a temporary save of partner list due to later addition
	// of contact partner's partners, which should be reversed at the end
	// of this method
	List<volumetricParticle*> originalContactPartners = contactPartners_;

	// Save intermediate kinetics as they should be restored at the end
	// in order to prevent repeated movement of objects
	//TESTsaveIntermediateState();

	addContactPartnersPartners();

	// Do NOT consider adhesive forces between partners
	// for the determination of the kinetic energy
	if(justCollisionLoad) calcCollisionLoad();
	else calcTotalLoadNoAdhesion();
	vector force  = getTotalForce();

	scalar mass   = getMass();
	vector cg     = getCg() * mass;
	vector velo   = getVelocity() * mass;
	if(myPop_->isPointParticle())  velo = velocityAtCgSubCellSize_ * mass_;
	if(myPop_->isPointParticle())  force = vector::zero;

    if( !(mass > SMALL) )
    {
      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
       << "Mass is zero."
       << exit(FatalError);
    }

	try
	{
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		if( (*partner)->myPop_->isStructure() ) continue;

		if(justCollisionLoad) (*partner)->calcCollisionLoad();
		else (*partner)->calcTotalLoadNoAdhesion();
		if (!(*partner)->myPop_->isPointParticle()) force  = force + (*partner)->getTotalForce();
		mass  += (*partner)->getMass();
		cg    += (*partner)->getCg() * (*partner)->getMass();
		if (!(*partner)->myPop_->isPointParticle()) velo += (*partner)->getVelocity() * (*partner)->getMass();
		else velo += (*partner)->velocityAtCgSubCellSize_ * (*partner)->getMass();
	}
	} catch (...)
	{
	      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
	       << "Error during momentum calculation of partners."
	       << exit(FatalError);
	}


	symmTensor J, Jinv;
	try
	{
	J = getAggloJ();
	//_PDBOP_("J = " << J, 3);
	Jinv = inv(J);
	//_PDBOP_("Jing = " << Jinv, 3);
	} catch (...)
	{
	      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
	       << "Error in inertia tensor calculation. Probably zero determinant."
	       << exit(FatalError);
	}
	cg               /= mass;
	velo             /= mass;


	// Fast adjustment of torque around new cg of agglomerate
	vector torque = getTotalTorque() - (  (cg_ - cg) ^ getTotalForce() );
	if(myPop_->isPointParticle()) torque = vector::zero; //NEWFVV
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		if( (*partner)->myPop_->isStructure() ) continue;
		if( (*partner)->myPop_->isPointParticle() ) continue; //NEWFVV

		torque += (*partner)->getTotalTorque() - ( ((*partner)->getCg() - cg) ^ (*partner)->getTotalForce() );
	}

	// Calculate new velocities of the agglomerate now.
	// As no new agglomerate formation is described here, NO conservation
	// of momenta needs to be calculated.
	vector acc, omegaAcc, veloAvg, omegaAvg, omega;
	scalar dt    = time_.deltaT().value();
	acc			 = force / mass;
	omega        = getOmega(); // All components of agglomerate have the same omega
	if(myPop_->isPointParticle()) omega = vector::zero; //NEWFVV
	omegaAcc	 = Jinv & (torque - ( omega ^ (J & omega) ) );
	if(myPop_->isPointParticle()) omegaAcc     = vector::zero; //NEWFVV
	veloAvg  	 = velo + 0.5 * dt * relax * acc;
	omegaAvg 	 = omega + 0.5 * dt * relax * omegaAcc;
	if(myPop_->isPointParticle()) omegaAvg     = vector::zero; //NEWFVV
	velo    	+= dt * relax * acc;
	omega   	+= dt * relax * omegaAcc;
	try
	{
	if(mag(omega) >= 50000) omega /= 100; //TODO hard-coded for subcellsize particles
	} catch (...)
	{
	      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
	       << "Rotational velocity too high."
	       << exit(FatalError);
	}


	// Constrain movement in accordance with structure contacts (see ::moveWithContactPartners()) -------------------------
	List<structureContact> contactPoints;
	try
	{
	forAllIter(List<structureContact>, structureContacts_, sContact)
	{
		contactPoints.append((*sContact));
	}
	} catch (...)
	{
	      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
	       << "Lost contact point information."
	       << exit(FatalError);
	}

	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		forAllIter(List<structureContact>, (*partner)->structureContacts_, sContact)
		{
			contactPoints.append((*sContact));
		}
	}

	vector constraintAxis;
	switch (contactPoints.size())
	{
		case 1:
			constraintAxis	= (contactPoints[0]).avgNormal;
			cg				= (contactPoints[0]).contactPoint;
			break;
		case 2:
			constraintAxis  = (contactPoints[0]).contactPoint - (contactPoints[1]).contactPoint;
		    if( (contactPoints[0]).contactPoint == (contactPoints[1]).contactPoint)
		    {
		      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
		       << "Two contact points are identical which leads to zero division."
		       << exit(FatalError);
		    }
			try
			{
			constraintAxis /= mag(constraintAxis);
			} catch (...)
			{
			      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
			       << "Zero length constraint axis."
			       << exit(FatalError);
			}
			cg				= ( (contactPoints[0]).contactPoint + (contactPoints[1]).contactPoint ) / 2;
			break;
		case 3:
			constraintAxis	= vector::zero;
			break;
		default:
			break;
	}

	switch (contactPoints.size())
	{
		case 0:
			break;

		case 1:
			omega		= omega - ( omega & constraintAxis ) * constraintAxis;
			omegaAvg    = omega;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;

		case 2:
			omega		= ( omega & constraintAxis ) * constraintAxis;
			omegaAvg    = omega;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;

		default:
			omega 		= vector::zero;
			omegaAvg	= vector::zero;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;
	}
	// End of movement constraint ----------------------------------------------------


	vector relPos	= (cg_ - cg);
	omega_		    = omega;
	omegaAvg_		= omegaAvg;
	rotNext_		= dt * omegaAvg_ * relax;
	vector axis	    = rotNext_;
	scalar theta;
	try
	{
	theta	= mag(axis);
	} catch (...)
	{
	      FatalErrorIn("Foam::volumetricParticle::getKineticEnergyOfAgglomerate(scalar relax, bool justCollisionLoad)")
	       << "Could not determine magnitude of Euler vector. Very large rotational acceleration?"
	       << exit(FatalError);
	}
	tensor rot	    = I;
	if(theta > SMALL)
	{
		  axis	   /= theta;
		  theta     = std::fmod(theta, constant::mathematical::twoPi);
		  quaternion q(axis, theta);
		  rot       = q.R();
	}
	relPos		    = (rot & relPos);
	velo_	 		= velo;
	veloAvg_		= veloAvg;

	/*_DBO_(idStr() <<
			"\n velo = " << velo_ <<
			"\n veloAvg = " << veloAvg_ <<
			"\n veloAGGLO = " << velo <<
			"\n veloAvgAGGLO = " << veloAvg <<
			"\n omega = " << omega_ <<
			"\n omegaAvg = " << omegaAvg_ <<
			"\n omegaAGGLO = " << omega <<
			"\n omegaAvgAGGLO = " << omegaAvg
			)*/

	aggloKineticEnergy = getKineticEnergy();

	//_DBO_("Kinetic energy main:" << aggloKineticEnergy)


	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
		  {
			    if((*partner)->myPop_->isStructure()) continue;

			    relPos								= ((*partner)->getCg() - cg);
				(*partner)->getOmega()				= omega;
				(*partner)->getAverageOmega()		= omegaAvg;
				relPos								= (rot & relPos);
				(*partner)->getVelocity()	 		= velo;
				(*partner)->getAverageVelocity()	= veloAvg;
				aggloKineticEnergy                 += (*partner)->getKineticEnergy();
				//_DBO_("Kinetic energy partner:" << (*partner)->getKineticEnergy())
		  }


	// Reset kinetics
	//subCyclingRestoreState();
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		(*partner)->subCyclingRestoreState();
	}

	// Reset list of contact partners
	contactPartners_ = originalContactPartners;
	//_DBO_("Kinetic energy TOTAL:" << aggloKineticEnergy)

	return aggloKineticEnergy;
}




// Calculates the inertia tensor of an agglomerate
void volumetricParticle::calcAggloJ()
{

	//_PDBOP_("Calculating J of agglomerate", 3)

	//List<volumetricParticle*> originalContactPartners = contactPartners_;

	// Loop over HashTable of all partner's partners and add
	// them to my partners list
	//addContactPartnersPartners();


	/*if (myPop_->isStructure())
	{
		return;
	}*/

	symmTensor 	J, oldJ; // oldJ is used in cases where an agglomerate collides with new particles.
	 	 	 	 	 	 // In that case oldJ defines the inertia tensor of the already existing agglomerate.
	vector		cg 		= cg_ * mass_;
	scalar		mass	= mass_;
	vectorField r 		= Cf();

	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
		{
			cg		+= (*partner)->getCg() * (*partner)->getMass();
			mass	+= (*partner)->getMass();
		}
	commonMass_ = mass;
	cg		/= mass;

	// ------------------ Calculate constrained cg for contacts WITH STRUCTURES --------------------------
	// ------------------ and if necessary calculate rotational constraints ------------------------------
	//
	List<structureContact> contactPoints;
	forAllIter(List<structureContact>, structureContacts_, sContact)
	{
		contactPoints.append((*sContact));
	}
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		forAllIter(List<structureContact>, (*partner)->structureContacts_, sContact)
		{
			contactPoints.append((*sContact));
		}
	}

	vector constraintAxis;
	//_DBO_("Amount of structureContacts for agglomerate of " << idStr() << " is " << contactPoints.size())
	switch (contactPoints.size())
	{
		case 1:
			// contactNormal of the object in contact with structure
			constraintAxis	= (contactPoints[0]).avgNormal;
			cg				= (contactPoints[0]).contactPoint;
			break;
		case 2:
			//  vector pointing from one center of a contactFace to the other
			constraintAxis  = (contactPoints[0]).contactPoint - (contactPoints[1]).contactPoint;
			constraintAxis /= mag(constraintAxis);
			cg				= ( (contactPoints[0]).contactPoint + (contactPoints[1]).contactPoint ) / 2;
			break;
		case 3:
			// too many contact points -> agglomerate can't rotate anymore
			constraintAxis	= vector::zero;
			break;
		default:
			break;
	}
	//
	//------------------- End of cg constrain for structure contacts -----------------------


	vector angularMomRefPoint = cg; // Reference point for angular ground momentum

	//------------------ Determine inertia tensor of the agglomerate ------------------
	//
	  const vectorField &c = Cf();
	  const vectorField &a = Sf();
	  J		= symmTensor::zero;

	  forAll(c, faceI)
	  {
	    vector cf  = c[faceI] - cg;
	    scalar xyz = cf.x() * cf.y() * cf.z();
	    scalar zzx = cf.z() * cf.z() * cf.x();
	    scalar xxy = cf.x() * cf.x() * cf.y();
	    scalar yyz = cf.y() * cf.y() * cf.z();

	    J.xy() += xyz * a[faceI].z();
	    J.xz() += xyz * a[faceI].y();
	    J.yz() += xyz * a[faceI].x();
	    J.xx() += zzx * a[faceI].x() +                      yyz * a[faceI].z();
	    J.yy() += zzx * a[faceI].x() + xxy * a[faceI].y()                     ;
	    J.zz() +=                      xxy * a[faceI].y() + yyz * a[faceI].z();
	  }
	  J *= rho_;

	  forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	  {
		  if ( (*partner)->myPop_->isStructure() ) continue; // NEW
		  const vectorField &c2 = (*partner)->Cf();
		  const vectorField &a2 = (*partner)->Sf();
		  symmTensor J2 = symmTensor::zero;

		  forAll(c2, faceI)
		  {
			vector cf  = c2[faceI] - cg;
			scalar xyz = cf.x() * cf.y() * cf.z();
			scalar zzx = cf.z() * cf.z() * cf.x();
			scalar xxy = cf.x() * cf.x() * cf.y();
			scalar yyz = cf.y() * cf.y() * cf.z();

			J2.xy() += (xyz * a2[faceI].z() ) * (*partner)->getRho();
			J2.xz() += (xyz * a2[faceI].y() ) * (*partner)->getRho();
			J2.yz() += (xyz * a2[faceI].x() ) * (*partner)->getRho();
			J2.xx() += (zzx * a2[faceI].x() +                      yyz * a2[faceI].z() ) * (*partner)->getRho();
			J2.yy() += (zzx * a2[faceI].x() + xxy * a2[faceI].y()                      ) * (*partner)->getRho();
			J2.zz() += (                     xxy * a2[faceI].y() + yyz * a2[faceI].z() ) * (*partner)->getRho();
		  }

		  J += J2;
	  }
	  //
	  //-------------------------- End of inertia tensor calculation -------------------------

	  aggloJ_ = J;
	  forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	  {
		  (*partner)->getAggloJ() = J;
	  }

	  //contactPartners_ = originalContactPartners;
}

// Returns true if a particle is in the contactPartners_ list else false
bool volumetricParticle::isPartner(volumetricParticle* partner)
{
	forAllIter(List<volumetricParticle*>, contactPartners_, partnerPrt)
	{
		if(partner->idStr() == (*partnerPrt)->idStr()) return true;
	}

	return false;
}


// Deletes a list of particles from a particle's contactPartners_ list
// as well as corresponding contact vectors and normals
void volumetricParticle::deleteParticlesFromList(List<volumetricParticle*>& deleteList)
{
	List<volumetricParticle*> keepList;
	List<vector>              vectorList;
	List<vector>              normalList;
	List<volumetricParticle::facePair> facePairList;
	bool toDelete;

	forAllIter(List<volumetricParticle*>, contactPartners_, partnerPrt)
	{
		toDelete = false;
		int listPosition = 0;

		forAllIter(List<volumetricParticle*>, deleteList, deletePrt)
		{
			if((*deletePrt)->idStr() == (*partnerPrt)->idStr())
			{
				toDelete = true;
				if((*deletePrt)->myPop_->isStructure()) structureContacts_.clear();
				unassignedPartners_.append(*deletePrt);
				break;
			}
		}

		if(!toDelete)
		{
			keepList.append(*partnerPrt);
			vectorList.append(contactVectors_[listPosition]);
			normalList.append(contactNormals_[listPosition]);
			facePairList.append(contactFaces_[listPosition]);
		}

		listPosition++;
	}
	contactPartners_ = keepList;
	contactVectors_  = vectorList;
	contactNormals_  = normalList;
	contactFaces_    = facePairList;
}


void volumetricParticle::moveWithContactPartners(scalar relax)
{

	// Check if agglomerate has already been treated
	//_DBO_("Moving with contactPartners for " << idStr())
	if(movedWithContactPartners_)
	{
		//_DBO_("Skipping moveWithContactPartners for " << idStr())
		return;
	}
	movedWithContactPartners_ = true;

	// Loop over HashTable of all partner's partners and add
	// them to my partners list
	List<volumetricParticle*> originalContactPartners = contactPartners_;
	addContactPartnersPartners();

	// Check if is a structure yielding a velocity of zero
	if (myPop_->isStructure())
	{
		velo_ = vector::zero;
		omega_ = vector::zero;
		return;
	}

	// Common kinetic parameters with contact partners
	calcTotalLoadNoAdhesion();
#if 0
	 _DBO_("=== Entering partner-move-method the kinematics for particle " << idStr() << " are: ===" <<
			"\n Cg = " << cg_ <<
			"\n mass = " << mass_ <<
			"\n force = " << totalForce_ <<
			"\n torque = " << totalTorque_ <<
	  		"\n velo = " << getVelocity() <<
	  		"\n veloAvg = " << getAverageVelocity() <<
	  		"\n omega = " <<  getOmega() <<
	  		"\n omegaAVG = " << getAverageOmega() <<
	  		"\n displNext = " <<  getDisplNext() <<
	  		"\n rotNext = " << getRotNext() <<
	  		"\n==============================================")
#endif
	symmTensor 	J, oldJ; // oldJ is used in cases where an agglomerate collides with new particles.
	 	 	 	 	 	 // In that case oldJ defines the inertia tensor of the already existing agglomerate.
	vector		cg 		= cg_ * mass_;
	scalar		mass	= mass_;
	vector		force	= totalForce_;
	vectorField r 		= Cf();
	vector		torque, velo, acc, omega, omegaSingle, omegaAcc, veloAvg, omegaAvg;

	// Individual forceFields used for each particle
	const vectorField &fF = fluidForceField();
	const vectorField &sF = solidForceField();
	const vectorField &tF = thermoForceField();
	const vectorField &emF = electromagForceField();

	// Get common cg, mass, force and velocity
	velo = velo_ * mass_;
	if(myPop_->isPointParticle())  velo = velocityAtCgSubCellSize_ * mass_; //NEWFVV
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
		{
			if ( (*partner)->myPop_->isStructure() ) continue;
			(*partner)->calcTotalLoadNoAdhesion();
			//(*partner)->subtractContactsFromTotalLoad();
			cg		+= (*partner)->getCg() * (*partner)->getMass();
			mass	+= (*partner)->getMass();
			if (!(*partner)->myPop_->isPointParticle()) velo += (*partner)->getVelocity() * (*partner)->getMass();
			else velo += (*partner)->velocityAtCgSubCellSize_ * (*partner)->getMass(); //NEWFVV
			force	+= (*partner)->getTotalForce();
		}
	commonMass_ = mass;
	cg		/= mass;
	velo	/= mass; // Momentum conservation



	// ------------------ Calculate constrained cg for contacts WITH STRUCTURES --------------------------
	// ------------------ and if necessary calculate rotational constraints ------------------------------
	//
	List<structureContact> contactPoints;
	forAllIter(List<structureContact>, structureContacts_, sContact)
	{
		contactPoints.append((*sContact));
	}
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		forAllIter(List<structureContact>, (*partner)->structureContacts_, sContact)
		{
			contactPoints.append((*sContact));
		}
	}

	vector constraintAxis;
	//_PDBOP_("Amount of structureContacts for agglomerate of " << idStr() << " is " << contactPoints.size(), 0)
	switch (contactPoints.size())
	{
		case 1:
			// contactNormal of the object in contact with structure
			constraintAxis	= (contactPoints[0]).avgNormal;
			cg				= (contactPoints[0]).contactPoint;
			break;
		case 2:
			//  vector pointing from one center of a contactFace to the other
			constraintAxis  = (contactPoints[0]).contactPoint - (contactPoints[1]).contactPoint;
			constraintAxis /= mag(constraintAxis);
			cg				= ( (contactPoints[0]).contactPoint + (contactPoints[1]).contactPoint ) / 2;
			break;
		case 3:
			// too many contact points -> agglomerate can't rotate anymore
		default:
			constraintAxis	= vector::zero;
			break;
	}
	/*_PDBOP_("contactPoints.size() = " << contactPoints.size()
			<< "\nconstraintAxis = " << constraintAxis
			<< "\ncg = " << cg, 0)*/
	//
	//------------------- End of cg constrain for structure contacts -----------------------


	vector angularMomRefPoint = cg; // Reference point for angular ground momentum

	//------------------ Determine inertia tensor of the agglomerate ------------------
	//
	  const vectorField &c = Cf();
	  const vectorField &a = Sf();
	  J		= symmTensor::zero;
	  oldJ	= symmTensor::zero;

	  forAll(c, faceI)
	  {
	    vector cf  = c[faceI] - cg;
	    scalar xyz = cf.x() * cf.y() * cf.z();
	    scalar zzx = cf.z() * cf.z() * cf.x();
	    scalar xxy = cf.x() * cf.x() * cf.y();
	    scalar yyz = cf.y() * cf.y() * cf.z();

	    J.xy() += xyz * a[faceI].z();
	    J.xz() += xyz * a[faceI].y();
	    J.yz() += xyz * a[faceI].x();
	    J.xx() += zzx * a[faceI].x() +                      yyz * a[faceI].z();
	    J.yy() += zzx * a[faceI].x() + xxy * a[faceI].y()                     ;
	    J.zz() +=                      xxy * a[faceI].y() + yyz * a[faceI].z();
	  }
	  J *= rho_;
	  oldJ = J;

	  forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	  {
		  if ( (*partner)->myPop_->isStructure() ) continue; // NEW
		  const vectorField &c2 = (*partner)->Cf();
		  const vectorField &a2 = (*partner)->Sf();
		  symmTensor J2 = symmTensor::zero;

		  forAll(c2, faceI)
		  {
			vector cf  = c2[faceI] - cg;
			scalar xyz = cf.x() * cf.y() * cf.z();
			scalar zzx = cf.z() * cf.z() * cf.x();
			scalar xxy = cf.x() * cf.x() * cf.y();
			scalar yyz = cf.y() * cf.y() * cf.z();

			J2.xy() += (xyz * a2[faceI].z() ) * (*partner)->getRho();
			J2.xz() += (xyz * a2[faceI].y() ) * (*partner)->getRho();
			J2.yz() += (xyz * a2[faceI].x() ) * (*partner)->getRho();
			J2.xx() += (zzx * a2[faceI].x() +                      yyz * a2[faceI].z() ) * (*partner)->getRho();
			J2.yy() += (zzx * a2[faceI].x() + xxy * a2[faceI].y()                      ) * (*partner)->getRho();
			J2.zz() += (                     xxy * a2[faceI].y() + yyz * a2[faceI].z() ) * (*partner)->getRho();
		  }

		  if(omega_ == (*partner)->getOmega())
			  oldJ += J2;
		  J += J2;
	  }
	  //
	  //-------------------------- End of inertia tensor calculation -------------------------



	// Get common inertia tensor J and torque
	r 		-= cg;
	torque	= sum( r ^ fF ) + sum( r ^ sF ) + sum( r ^ tF ) + sum( r ^ emF ) + externalTorque_; // Neglect contact force here aswell
	if(myPop_->isPointParticle()) torque = vector::zero; //NEWFVV

	omega = vector::zero;
	if(omega_ != contactPartners_[0]->getOmega())
	{
		omega	= ((cg_ - angularMomRefPoint) ^ (velo_ * mass_)) + (J_ & omega_);  // bahn + eigen
	}

#if	0
	_DBO_("================= MY " << idStr() << " PARTNERS ARE: =========================== ")
	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	{
		_DBO_((*partner)->idStr())
	}
#endif

	forAllIter(List<volumetricParticle*>, contactPartners_, partner)
		{
			(*partner)->commonMass_ = mass; //new
			(*partner)->movedWithContactPartners_ = true;

			if(!myPop_->isPointParticle()) //FVVNEW
			{
			scalar	massP	=  (*partner)->getMass();
			vectorField &pfF = (*partner)->fluidForceField();
			vectorField &psF = (*partner)->solidForceField();
			vectorField &ptF = (*partner)->thermoForceField();
			vectorField &pemF= (*partner)->electromagForceField();
			vectorField pr	 = (*partner)->Cf();

			pr				-= cg;
			if(! (*partner)->myPop_->isStructure() )
			{
				torque			+= sum( pr ^ pfF ) + sum( pr ^ psF ) + sum( pr ^ ptF ) + sum( pr ^ pemF ) +  (*partner)->externalTorque_;
				if(omega_ != (*partner)->getOmega())
				{
					omega			+= (((*partner)->getCg() - angularMomRefPoint) ^ ((*partner)->getVelocity() * massP))
									+ ((*partner)->getJ() & (*partner)->getOmega());
				}
				else
				{
					omega = oldJ & omega_; // No new collision for now, ground momentum omega such that (Jinv & omega) will lead to original ang.vel. omega
				}
			}
			}
		}
	symmTensor	Jinv(inv(J));
	omega =		Jinv & ( omega - ((cg - angularMomRefPoint) ^ (velo * mass)) ); // Transform total ground angular momentum into angular velocity


	// Calculate common average velocity and average omega
	scalar dt    = time_.deltaT().value();
	acc			 = force / mass;
	omegaAcc	 = Jinv & (torque - ( omega ^ (J & omega) ) );
	veloAvg  	 = velo + 0.5 * dt * relax * acc;
	omegaAvg 	 = omega + 0.5 * dt * relax * omegaAcc;
	velo    	+= dt * relax * acc;
	omega   	+= dt * relax * omegaAcc;


	if(mag(omega) >= 100) omega /= mag(omega) * 1e-2; //TODO hard-coded for subcellsize particles

	// Limit the possible rotations in accordance with
	// the contactPoints with structures
	switch (contactPoints.size())
	{
		case 0:
			break;

		case 1:
			omega		= omega - ( omega & constraintAxis ) * constraintAxis;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;

		case 2:
			omega		= ( omega & constraintAxis ) * constraintAxis;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;

		default:
			// More than 2 contactPoints
			omega 		= vector::zero;
			omegaAvg	= vector::zero;
			velo 		= vector::zero;
			veloAvg		= vector::zero;
			break;
	}



	  // Set displacements and rotations for all contact partners and execute kinetics.
	  // NOTE: 	omega/omegaAvg of the agglomerate and the singular
	  //		particles are of course identical, because when the
	  //		agglomerate turns by e.g. 180° the singular
	  //		particles have to, aswell.
      vector relPos	= (cg_ - cg);
	  omega_		= omega;
	  omegaAvg_		= omegaAvg;
	  rotNext_		= dt * omegaAvg_ * relax;
	  vector axis	= rotNext_;
	  scalar theta	= mag(axis);
	  tensor rot	= I;
	  if(theta > SMALL)
	  {
		  axis	   /= theta;
		  theta = std::fmod(theta, constant::mathematical::twoPi);
		  quaternion q(axis, theta);
		  rot = q.R();
	  }

	  vector oldVelo1, oldVelo2; //DISS
	  oldVelo1 = velo_; // DISS

	  displNext_	= dt * veloAvg * relax - relPos;
	  relPos		= (rot & relPos);
	  displNext_   += relPos;
	  velo_	 		= velo + ( omega ^ relPos );
	  veloAvg_		= veloAvg + ( omegaAvg ^ relPos );

#if 0
	  _DBO_("\n°°°°°°°°°° AGGLOMERATE VALUES  " << idStr() << " °°°°°°°°°°" <<
			"\n cg_agl = " << cg <<
			"\n mass_agl = " << mass <<
			"\n force_agl = " << force <<
	  		"\n velo_agl = " << velo <<
	  		"\n veloAvg_agl = " << veloAvg <<
			"\n acc_agl = " << acc <<
	  		"\n omega_agl = " <<  omega <<
	  		"\n omegaAVG_agl = " << omegaAvg <<
			"\n omegaAcc_agl = " << omegaAcc <<
			"\n transl. kin.En. = " << (0.5 * mass * velo & velo) <<
			"\n angul. kin. En. = " << ( 0.5 * omega & (J & omega)) <<
			"\n total kin.En. = " << ((0.5 * mass * velo & velo) + ( 0.5 * omega & (J & omega))) <<
	  		"\n°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°")

	  _DBO_("\n=== moveWithContactPartners for particle " << idStr() << " ===" <<
			"\n relativeCg = " << mag(cg_ - cg) <<
			"\n Cg = " << cg_ <<
			"\n mass = " << mass_ <<
	  		"\n velo = " << getVelocity() <<
	  		"\n veloAvg = " << getAverageVelocity() <<
	  		"\n omega = " <<  getOmega() <<
	  		"\n omegaAVG = " << getAverageOmega() <<
	  		"\n displNext = " <<  getDisplNext() <<
	  		"\n rotNext = " << getRotNext() <<
	  		"\n==============================================")
#endif

	  // Execute kinetic for first particle of the agglomerate
	  // which entered this method
	  kinetic();

	  forAllIter(List<volumetricParticle*>, contactPartners_, partner)
	  {
		    if((*partner)->myPop_->isStructure()) continue;
		    oldVelo2 = (*partner)->getVelocity(); //DISS

		    relPos								= ((*partner)->getCg() - cg);
			(*partner)->getOmega()				= omega;
			(*partner)->getAverageOmega()		= omegaAvg;
			(*partner)->getRotNext()			= dt * omegaAvg * relax;
			(*partner)->getDisplNext()			= dt * veloAvg * relax - relPos;
			relPos								= (rot & relPos);
			(*partner)->getDisplNext()		   += relPos;
			(*partner)->getVelocity()	 		= velo + ( omega ^ relPos );
			(*partner)->getAverageVelocity()	= veloAvg + ( omegaAvg ^ relPos );

#if 0
			  _DBO_("°°°°°°°°°°°°°°°°°°otherPrt°°°°°°°°°°°°°°°°°°"
					  <<"\nvelo = " << (*partner)->getAverageVelocity()
					  <<"\nveloFromOmega = " <<  ( omegaAvg ^ relPos ))


			_DBO_("\n\t--- contactPartner loop for movement for particle " << (*partner)->idStr() << " ---" <<
					"\n\t MASS DIFFERENCE = " << mass_ - (*partner)->getMass() <<
					"\n\t relativeCg = " << mag((*partner)->getCg() - cg) <<
					"\n\t Cg = " << (*partner)->getCg() <<
					"\n\t mass = " << (*partner)->getMass() <<
					"\n\t velo = " << (*partner)->getVelocity() <<
					"\n\t veloAvg = " << (*partner)->getAverageVelocity() <<
			  		"\n\t omega = " <<  (*partner)->getOmega() <<
			  		"\n\t omegaAVG = " << (*partner)->getAverageOmega() <<
					"\n\t displNext = " <<  (*partner)->getDisplNext() <<
					"\n\t rotNext = " << (*partner)->getRotNext() <<
					"\n\t--------------------------------------")
#endif
			//_PDBO_("enter partner-kinetic")
			// Execute the kinetic for partner particle of the agglomerate
			(*partner)->kinetic();
			(*partner)->discardTriSurfSearch();

			//_DBO_("Energyloss = " << (0.5 * mass_ * ( (getVelocity() & getVelocity()) - (oldVelo1 & oldVelo1) + ((*partner)->getVelocity() & (*partner)->getVelocity()) - (oldVelo2 & oldVelo2)) ) ) //DISS
	  }


		// Save current inertia moment tensor of agglomerate for possible
		// faster application in other methods
		getAggloJ() = symm(rot & J & rot.T());;
		forAllIter(List<volumetricParticle*>, contactPartners_, partner)
		{
			(*partner)->getAggloJ() = getAggloJ();
		}

	contactPartners_ = originalContactPartners;

	discardTriSurfSearch();
}

// Prevents jitter for particles with resting contacts
void volumetricParticle::reduceJitter()
{
	return;

	 vector kinEn = getKineticEnergy();

	 if(kinEn.x() < 1e-2) // translational kinEn
		 {
		 getVelocity() = vector::zero;
		 getAverageVelocity() = vector::zero;
		 }
	 if(kinEn.y() < 1e-1) // rotational kinEn
		 {
		 getOmega()	= vector::zero;
		 getAverageOmega() = vector::zero;
		 }
}


// Loops over contact partner's contact partners and adds them to current
// particle's list. This way the full agglomerate will be known to the
// current particle which will be used for the calculation of common
// cg, mass, translation and so on...
void volumetricParticle::addContactPartnersPartners()
{
	HashTable<volumetricParticle*> allPartners;
	List<volumetricParticle*> originalPartners = contactPartners_;

	forAllIter(List<volumetricParticle*>, contactPartners_, partnerI)
	{
		recursivePartners(allPartners);
	}

	// At this point all partners are saved in allPartners,
	// however the list still contains double entries.
	// Those will be deleted next.

	forAllIter(HashTable<volumetricParticle*>, allPartners, partnerI)
	{
		if((*partnerI)->idStr() != idStr())
		{
			bool foundDublicate = false;

			forAllIter(List<volumetricParticle*>, originalPartners, orig)
				{
					if((*partnerI)->idStr() == (*orig)->idStr())
					{
						foundDublicate = true;
						break;
					}
				}
			if (foundDublicate) continue;
			// Finally fill list with all partners that are
			// directly in contacted or linked through
			// other partner to the contactPartners_ list
			contactPartners_.append((*partnerI));
		}
	}
}

// Recursive part for the volumetricParticle::addContactPartnersPartners() method from above
void volumetricParticle::recursivePartners(HashTable<volumetricParticle*> &allPartners)
{
	forAllIter(List<volumetricParticle*>, contactPartners_, iter)
		{
			//if(allPartners.found((*iter)->idStr()) || (*iter)->myPop_->isStructure()) continue; // Do NOT connect objects through common structures
			if(allPartners.found((*iter)->idStr())) continue;
			allPartners.insert((*iter)->idStr(), (*iter));
			if((*iter)->myPop_->isStructure()) continue; // Do NOT connect objects through common structures
			(*iter)->recursivePartners(allPartners);
		}
}


// pManager calls this method in the case of a collision with a structure.
// The faceList is the reduced List of all current collision faces.
// In the end the volumetricParticle class attribute structureContacts_
// contains all contact points with structures that are far enough away ( > maxDist )
// from each other to be considered individual contact points.
void volumetricParticle::setContactPointsWithStructure( List<int>& faceList, vector avgNormal, vector avgCenter )
{
	scalar initMaxDist	= scale_ * myPop_->contactPointDist();

	// The average faceCenter is always chosen as the first contactPoint
	structureContact avgContact;
	avgContact.contactPoint	= avgCenter;
	avgContact.avgNormal		= avgNormal;
	if(structureContacts_.size() == 0) structureContacts_.append(avgContact);
	/*_DBO_("Appended average for " << idStr()
						<<"\navgCenter = " << avgCenter << "\tavgNormal = " << avgNormal)*/

	// Now look for any further contact points with the condition
	// that all of them are at least initMaxDist away from
	// already detected contactPoints
	for(int i = 0; i < faceList.size(); i++)
	{
		bool foundNewPoint 		= true;
		vector contactPoint = Cf()[faceList[i]];

		for(int j = 0; j < structureContacts_.size(); j++)
		{
			if ( mag(contactPoint - structureContacts_[j].contactPoint) < initMaxDist )
				{
					foundNewPoint = false;
					structureContacts_[j].contactPoint = contactPoint;
					structureContacts_[j].avgNormal = avgNormal;
					break;
				}
		}

		if(foundNewPoint)
		{
			structureContact contact;
			contact.contactPoint	= contactPoint;
			contact.avgNormal		= avgNormal;
			structureContacts_.append(contact);
			/*_DBO_("Appended additional for " << idStr()
									<<"\ncenter = " << contactPoint << "\tavgNormal = " << avgNormal)*/
		}
	}
}


// If for any reason the kinetic parameters of injected objects
// slightly differ between processors and no solution can be found
// then this method can be used as a dirty circumvention by
// distributing averaged values between all processors.
void volumetricParticle::parallelAdjustmentConstraints( vector tempV, vector tempVAvg, vector tempOmega, vector tempOmegaAvg)
{
	//_PDBO_("BEFORE distr. : velo = " << velo_ << "\tveloAvg = " << veloAvg_)
	//_PDBO_("BEFORE distr. : TEMPvelo = " << tempV << "\tTEMPveloAvg = " << tempVAvg)

	shareVectorField()[0] = velo_;
	shareVectorField()[1] = veloAvg_;
	shareVectorField()[2] = omega_;
	shareVectorField()[3] = omegaAvg_;

	int n = Pstream::nProcs();

	//myPop_->distributeForces(*bg_, &volumetricParticle::shareVectorField);

	//_DBO_("(shareVectorField()[0]) = " << (shareVectorField()[0]))

	velo_[0] = (shareVectorField()[0])[0] - (n * tempV[0]);
	velo_[1] = (shareVectorField()[0])[1] - (n * tempV[1]);
	velo_[2] = (shareVectorField()[0])[2] - (n * tempV[2]);

	veloAvg_[0] = (shareVectorField()[1])[0] - (n * tempVAvg[0]);
	veloAvg_[1] = (shareVectorField()[1])[1] - (n * tempVAvg[1]);
	veloAvg_[2] = (shareVectorField()[1])[2] - (n * tempVAvg[2]);

	omega_[0] = (shareVectorField()[2])[0] - (n * tempOmega[0]);
	omega_[1] = (shareVectorField()[2])[1] - (n * tempOmega[1]);
	omega_[2] = (shareVectorField()[2])[2] - (n * tempOmega[2]);

	omegaAvg_[0] = (shareVectorField()[3])[0] - (n * tempOmegaAvg[0]);
	omegaAvg_[1] = (shareVectorField()[3])[1] - (n * tempOmegaAvg[1]);
	omegaAvg_[2] = (shareVectorField()[3])[2] - (n * tempOmegaAvg[2]);

	velo_		+= tempV;
	veloAvg_	+= tempVAvg;
	omega_		+= tempOmega;
	omegaAvg_	+= tempOmegaAvg;

	//_PDBO_("AFTER distr. : velo = " << velo_ << "\tveloAvg = " << veloAvg_)
}

template
void volumetricParticle::distributeValue(scalar value);

template
void volumetricParticle::distributeValue(vector value);

template<typename Type>
void volumetricParticle::distributeValue(Type value)
{
return;
}

void volumetricParticle::bounce( vector n_vec, scalar k_pl, scalar mu ) {

	scalar dp, ez, ex;
	dp = 2 * radEVS_;
	vector v_vec = getVelocity();
	vector w_vec = getOmega();

	// Transformation in das Koordinatensystem der Oberfläche
	// Zunächst ONB bestimmen
	scalar n_comp = v_vec & n_vec;
	vector t_vec = v_vec - n_comp * n_vec;
	scalar t_comp = mag(t_vec);
	t_vec /= (t_comp + VSMALL);
	vector m_vec = t_vec ^ n_vec;
	scalar m_comp = mag(m_vec);
	m_vec /= (m_comp + VSMALL);

	// Transformationsmatrix berechnen
	symmTensor b_mat = symmTensor(
			1, 0, 0,
			1, 0,
			1
			);
	tensor b2_mat = tensor(
			t_vec[0], n_vec[0], m_vec[0],
			t_vec[1], n_vec[1], m_vec[1],
			t_vec[2], n_vec[2], m_vec[2]
			);

	tensor tbb2 = inv(b2_mat) & b_mat;
	tensor tb2b = inv(tbb2);

	// Geschwindigkeiten in die Basis der Oberfläche transformieren
	/*v_vec = tbb2 & v_vec;
	w_vec = tbb2 & w_vec;*/

	v_vec = transform(tbb2, v_vec);
	w_vec = transform(tbb2, w_vec);


	// Stoß berechnen
	scalar vrel = sqrt(pow(v_vec[0] + w_vec[2] * dp/2 , 2) + pow(v_vec[2] - w_vec[0] * dp/2, 2));
	ex = (v_vec[0] + dp/2 * w_vec[2]) / (vrel + VSMALL);
	ez = (v_vec[2] - dp/2 * w_vec[0]) / (vrel + VSMALL);

	w_vec[0] = w_vec[0] - 5/dp * ez * (k_pl+1) * mu * v_vec[1];
	w_vec[2] = w_vec[2] + 5/dp * ex * (k_pl+1) * mu * v_vec[1];

	v_vec[0] = v_vec[0] + ex * (k_pl+1) * mu * v_vec[1];
	v_vec[2] = v_vec[2] + ez * (k_pl+1) * mu * v_vec[1];
	v_vec[1] = -k_pl * v_vec[1];

	// Geschwindigkeiten zurück ins globale Koord.sys. transformieren
	/*v_vec = tb2b & v_vec;
	w_vec = tb2b & w_vec;*/
	v_vec = transform(tb2b, v_vec);
	w_vec = transform(tb2b, w_vec);

	getVelocity() = v_vec;
	getOmega() = w_vec;
	//_PDBO_("\n NEW v_vec: " << particle.getVelocity())
	//_PDBO_("\n NEW w_vec: " << particle.getOmega())

	//_PDBO_("\n particleBounce !!!")
	getAverageVelocity() = v_vec;
	getAverageOmega() = w_vec;
	//_PDBO_("\n average v_vec: " << particle.getAverageVelocity())
	//_PDBO_("\n average w_vec: " << particle.getAverageOmega())
}

void volumetricParticle::moveSlave(const vector &e, const vector & displ, scalar relax)
{
  //kinetic();
_DBO_("\nmooeep\n")
  discardTriSurfSearch();
}

// Rotates direction dependent contact information
// for a particle according to kintetic()'s rot tensor
void volumetricParticle::rotateContactComponents(tensor& rot)
{
	if(contactVectors_.size() == 0) return;

	forAll(contactVectors_, partner)
	{
		contactVectors_[partner] = rot & contactVectors_[partner];
		contactNormals_[partner] = rot & contactNormals_[partner];
	}
}


// Rotate
void volumetricParticle::kinetic()
{

//_PDBO_("NEXT ROT IS " << rotNext_ << " AND NEXT DISPLACEMENT FOR " << idStr() << " IS " << displNext_);

  vector  axis  = rotNext_;   // Euler axis times angle

  scalar  theta = mag(axis);  // angle

  tensor  rot   = I;       // Init rotation matrix with identity

  if(theta > SMALL)
  {
    axis /= theta;  // normalize axis -> euler axis

    // eliminate redundant rotations -> euler angle
    theta = std::fmod(theta, constant::mathematical::twoPi);
    // Calculate rotation matrix from euler axis/angle
    quaternion q(axis, theta);
    rot = q.R();//I*cos(theta) + (1-cos(theta))*(axis*axis) + *axis*sin(theta);
  }


  pointField r  = points();         // r contains copy of absolute coordinates
  r      -= cg_;                    // make relative to cg
  r       = rot & r;                // rotate coordinates
  r      += cg_ + displNext_;       // make absolute again and translate
  cg_    += displNext_;             // translate cg
  displ_ += displNext_;             // translate displacement

  orientation_ = rot & orientation_; // rotate orientation


  rotateContactComponents(rot); // rotate contact information


  stlPtr_().movePoints(r);
  // Rotate moments of inertia tensor by
  // J -> rot * J rot^t
  J_ = symm(rot & J_ & rot.T());

  validSf_      = false;
  validNormals_ = false;

  calculatedAgglo_ = false;
}


void volumetricParticle::orientationToEulerAxis(vector& e) const
{
  quaternion q(orientation_);

  scalar     w = q.w();
  vector     v = q.v();
  // for better output: keep angle positive -> switch axis if neccessary
  label   sign = (w >= 0.) ? 1 : -1;


  scalar theta = 2*std::atan2(mag(v), sign*w);
  // eliminate redundant rotations -> euler angle
  scalar dummy = theta;
  theta = std::fmod(theta, constant::mathematical::twoPi);

  e = (theta == 0) ? (vector::zero) : (v / std::sin(0.5*theta));
  e *= sign;


  scalar normE = mag(e);

  if(normE > SMALL)
    e /= normE;

  e *= theta;

#if 0
{
tensor ori(I);

  scalar  theta = mag(e);  // angle

  tensor  rot   = I;       // Init rotation matrix with identity

  if(theta > SMALL)
  {
    e /= theta;  // normalize axis -> euler axis

    // eliminate redundant rotations -> euler angle
    theta = std::fmod(theta, constant::mathematical::twoPi);
    // Calculate rotation matrix from euler axis/angle
    quaternion q(e, theta);
    rot = q.R();//I*cos(theta) + (1-cos(theta))*(axis*axis) + *axis*sin(theta);
  }

  ori = rot & ori;

_PDBO_("Ori to Euler:" << nl <<
       "ori   = " << orientation_ << nl <<
       "euler = " << e << nl <<
       "ori  =  " << ori << endl
      )
}
#endif
}


void volumetricParticle::writeGeometry() const
{
  if( isSlave() )
    return;

  fileName fName(time_.timeName() + "/" + idStr_ + "/polyMesh");
  string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
  system(mkDirCmd);

  OFstream pointsFile(fName + "/points");


  pointsFile << _HEADER_POLYMESH_POINTS_;
  pointsFile << triSurf().points();

  if( topoChanged_ )
    toolWriteMesh(triSurf(), fName);
}

void  volumetricParticle::writeProperties()
{
  //if(state_ == slave)
   // return;

  calcTotalLoad();


#if 0
  LSMIOdictionary& dict = particleProperties();

  vector  eulerAxis;

  orientationToeulerAxis(eulerAxis);

  dict.add<scalar>("scaleFactor", scale_, true);
  dict.add<scalar>("rho", rho_, true);
  dict.add<vector>("centerOfGravity", cg_, true);
  dict.add<vector>("orientation", eulerAxis, true);
  dict.add<vector>("velocity", velo_, true);
  dict.add<vector>("acceleration", acc_, true);
  dict.add<vector>("angularVelocity", omega_, true);
  dict.add<vector>("angularAcceleration", omegaAcc_, true);
  dict.add<vector>("force", totalForce_, true);
  dict.add<vector>("torque", totalTorque_, true);
#endif

{
  fileName fName(time_.timeName() + "/" + idStr_ + "/");
  string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
  system(mkDirCmd);
  fName += "particleProperties";

  OFstream propFile(fName);

  if(!propFile.opened())
  {
    WarningIn("void volumetricParticle::writeProperties()")
                << "Could not open file '" << fName << "'" << nl;
  }

  vector  eulerAxis;

  orientationToEulerAxis(eulerAxis);

  propFile << "age                 " << getAge() << nl;
  propFile << "scaleFactor         " << scale_ << nl;
  propFile << "rho                 " << rho_ << nl;
  propFile << "centerOfGravity     " << cg_ << nl;
  propFile << "orientation         " << eulerAxis << nl;
  propFile << "velocity            " << velo_ << nl;
  propFile << "acceleration        " << acc_ << nl;
  propFile << "angularVelocity     " << omega_ << nl;
  propFile << "angularAcceleration " << omegaAcc_ << nl;
  propFile << "force               " << totalForce_ << nl;
  propFile << "pressure force      " << sum(pressureField()) << nl;
  propFile << "stress force        " << sum(stressField()) << nl;
  propFile << "torque              " << totalTorque_ << nl;

}

  if(writeForceField_ && fluidForcePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/");
    string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
    system(mkDirCmd);
    fName += "fluidForce";

    const vectorField& force  = fluidForceField();
    vector total = sum(force);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_FLUIDFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << force;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writePressureForceField_ && pressurePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/");
    string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
    system(mkDirCmd);
    fName += "fluidPressureForce";

    const scalarField&       p = pressureField();
    const vectorField&      Sn = Sf();
    const vectorField   pForce = - (Sn*p);
    vector               total = sum(pForce);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_FLUIDPRESSUREFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << pForce;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writePressureForceDensityField_ && pressurePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/");
    string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
    system(mkDirCmd);
    fName += "fluidPressureForceDensity";

    const scalarField&       p = pressureField();
    const vectorField&       n = normals();
    const vectorField   pForce = - (n*p);
    vector               total = sum(pForce);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_FLUIDPRESSUREFORCE_DENSITY_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCEDENSITY_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << pForce;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writeStressForceField_ && stressPtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/");
    string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
    system(mkDirCmd);
    fName += "fluidStressForce";

    const symmTensorField& stress = stressField();
    const vectorField&         Sn = Sf();
    const vectorField      sForce = - (Sn & stress); // neg. sign, see calcFluidForces()
    vector                  total = sum(sForce);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_FLUIDSTRESSFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << sForce;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writeStressForceDensityField_ && stressPtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/");
    string mkDirCmd = "mkdir -p "; mkDirCmd += fName;
    system(mkDirCmd);
    fName += "fluidStressForceDensity";

    const symmTensorField& stress = stressField();
    const vectorField&          n = normals();
    const vectorField      sForce = - (n & stress); // neg. sign, see calcFluidForces()
    vector                  total = sum(sForce);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_FLUIDSTRESSFORCE_DENSITY_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCEDENSITY_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << sForce;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writeForceField_ && solidForcePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/solidForce");

    const vectorField& force  = solidForceField();
    vector total = sum(force);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_SOLIDFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << force;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writeForceField_ &&  thermoForcePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/thermoForce");

    const vectorField& force  = thermoForceField();
    vector total = sum(force);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_THERMOFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << force;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }

  if(writeForceField_ &&  electromagForcePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/electromagForce");

    const vectorField& force  = electromagForceField();
    vector total = sum(force);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_ELECTROMAGFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << force;
    forceFile << _WRITE_FIELD_POSTAMBLE_;
  }



if(writeForceField_ &&  contactForcePtr_.valid())
  {
    fileName fName(time_.timeName() + "/" + idStr_ + "/contactForce");

    const vectorField& force  = contactForceField();
    vector total = sum(force);

    OFstream forceFile(fName);
    if(!forceFile.opened())
    {
      WarningIn("void volumetricParticle::writeGeometry()")
                << "Could not open file '" << fName << "'" << nl;
    }
    forceFile << _HEADER_CONTACTFORCE_FIELD_;
    forceFile << _WRITE_DIMENSIONS_FORCE_;
    forceFile << _WRITE_FIELD_PREAMBLE_;
    forceFile << force;
    forceFile << ";" << nl;
    forceFile << "  }" << nl;
    forceFile << "}" << nl;
  }
}

void volumetricParticle::printParticleData()
{
	_DBO_(
			"======= " << idStr() << " ======="
			<<"\ncg           = " << cg_
			<<"\nscale        = " << scale_
			<<"\nmass         = " << mass_
			<<"\nJ            = " << J_
			<<"\norientation  = " << orientation_
			<<"\ntotal force  = " << totalForce_
			<<"\next. force   = " << externalForce_
			<<"\ntotal torque = " << totalTorque_
			<<"\next. torque  = " << externalTorque_
			<<"\naccel.       = " << acc_
			<<"\next. accel.  = " << externalAcc_
			<<"\nomega accel. = " << omegaAcc_
			<<"\next. om.Acc  = " << externalAcc_
			<<"\nvelo         = " << velo_
			<<"\nveloAvg      = " << veloAvg_
			<<"\nomega        = " << omega_
			<<"\nomegaAvg     = " << omegaAvg_
			<<"\ndispl        = " << displ_
			<<"\ndisplNext_   = " << displNext_
			<<"\nrotNext_     = " << rotNext_
			<<"\n=============================="
		)

}

void  volumetricParticle::endOfExecution()
{
  if( topoChanged_ )
    discardFields();

//  topoChanged_ = false;  // clear for next time step
//  _DBO_("Set topoChanged_ to " << topoChanged_)
}

vectorField& volumetricParticle::fluidForceField()
{
  if( !fluidForcePtr_.valid() || topoChanged_ )
  {
      fluidForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }

  return fluidForcePtr_();
}

vectorField& volumetricParticle::savedFluidForceField()
{
  if( !savedFluidForcePtr_.valid() || topoChanged_  )
  {
      savedFluidForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }

  return savedFluidForcePtr_();
}

vectorField& volumetricParticle::solidForceField()
{
  if( !solidForcePtr_.valid() || topoChanged_  )
  {
      solidForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }

  return solidForcePtr_();
}

vectorField& volumetricParticle::thermoForceField()
{
  if( !thermoForcePtr_.valid() || topoChanged_  )
  {
      thermoForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }

  return thermoForcePtr_();
}

vectorField& volumetricParticle::savedThermoForceField()
{
  if( !savedThermoForcePtr_.valid() || topoChanged_  )
  {
      savedThermoForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }

  return savedThermoForcePtr_();
}

vectorField& volumetricParticle::electromagForceField()
{
  if( !electromagForcePtr_.valid() || topoChanged_  )
  {
      electromagForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }
  return electromagForcePtr_();
}

vectorField&     volumetricParticle::contactForceField()
{
    if( !contactForcePtr_.valid() || topoChanged_  )
    {
        contactForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
    }

    return contactForcePtr_();
}

vectorField& volumetricParticle::savedElectromagForceField()
{
  if( !savedElectromagForcePtr_.valid() || topoChanged_  )
  {
      savedElectromagForcePtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
  }
  return savedElectromagForcePtr_();
}

symmTensorField& volumetricParticle::stressField()
{
    if( !stressPtr_.valid() || topoChanged_  )
    {
      stressPtr_.reset( new symmTensorField
                        (
                          triSurf().size(), symmTensor::zero
                        )
                      );
    }

    return stressPtr_();
}

scalarField&     volumetricParticle::pressureField()
{
    if( !pressurePtr_.valid() || topoChanged_  )
    {
        pressurePtr_.reset( new scalarField
                            (
                              triSurf().size(), 0.
                            )
                          );
    }

    return pressurePtr_();
}

scalarField&     volumetricParticle::thermoField()
{
    if( !thermoPtr_.valid() || topoChanged_  )
    {
        thermoPtr_.reset( new scalarField
                            (
                              triSurf().size(), 0.
                            )
                          );
    }

    return thermoPtr_();
}

vectorField&     volumetricParticle::electromagField()
{
    if( !electromagPtr_.valid() || topoChanged_  )
    {
        electromagPtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
    }

    return electromagPtr_();
}

scalarField&     volumetricParticle::sigmaField()
{
    if( !sigmaPtr_.valid() || topoChanged_  )
    {
        sigmaPtr_.reset( new scalarField
                            (
                              triSurf().size(), 0.
                            )
                          );
    }

    return sigmaPtr_();
}

vectorField&     volumetricParticle::shareVectorField()
{
    if( !shareVectorPtr_.valid() )
    {
        shareVectorPtr_.reset( new vectorField
                            (
                            	_MAX_N_PROCESSES_, vector::zero
                            )
                          );
    }

    return shareVectorPtr_();
}

scalarField&     volumetricParticle::shareScalarField()
{
    if( !shareScalarPtr_.valid() )
    {
    	shareScalarPtr_.reset( new scalarField
                            (
                              _MAX_N_PROCESSES_, 0.
                            )
                          );
    }

    return shareScalarPtr_();
}

vectorField&     volumetricParticle::polarField()
{
    if( !polarPtr_.valid() || topoChanged_  )
    {
        polarPtr_.reset( new vectorField
                            (
                              triSurf().size(), vector::zero
                            )
                          );
    }

    return polarPtr_();
}

pointField& volumetricParticle::savedSubcyclingPoints()
{
    if( !savedSubcyclingPointsPtr_.valid() || topoChanged_  )
    {
        savedSubcyclingPointsPtr_.reset( new pointField
                               (
                                 triSurf().nPoints(), vector::zero
                               )
                             );
    }

    return savedSubcyclingPointsPtr_();
}

pointField& volumetricParticle::savedPreCollPoints()
{
    if( !savedPreCollPointsPtr_.valid() || topoChanged_  )
    {
        savedPreCollPointsPtr_.reset( new pointField
                               (
                                 triSurf().nPoints(), vector::zero
                               )
                             );
    }

    return savedPreCollPointsPtr_();
}

pointField& volumetricParticle::savedIntermediatePoints()
{
    if( !savedIntermediatePointsPtr_.valid() || topoChanged_  )
    {
        savedIntermediatePointsPtr_.reset( new pointField
                               (
                                 triSurf().nPoints(), vector::zero
                               )
                             );
    }

    return savedIntermediatePointsPtr_();
}

pointField& volumetricParticle::savedIterativeCouplingPoints()
{
    if( !savedIterativeCouplingPointsPtr_.valid() || topoChanged_  )
    {
        savedIterativeCouplingPointsPtr_.reset( new pointField
                               (
                                 triSurf().nPoints(), vector::zero
                               )
                             );
    }

    return savedIterativeCouplingPointsPtr_();
}

void volumetricParticle::subCyclingSaveState()
{
  const pointField& current = points();
        pointField& storage = savedSubcyclingPoints();

  sc_cg0_          = cg_;
  sc_displ0_       = displ_;
  sc_J0_           = J_;
  sc_velo0_        = velo_;
  sc_veloAvg0_	   = veloAvg_;
  sc_omega0_       = omega_;
  sc_omegaAvg0_    = omegaAvg_;
  sc_orientation0_ = orientation_;
  sc_totalForce0_  = totalForce_;
  sc_totalTorque0_ = totalTorque_;

  storage = current;
}

void volumetricParticle::saveIntermediateState()
{
  const pointField& current = points();
        pointField& storage = savedIntermediatePoints();

  it_cg0_          = cg_;
  it_displ0_       = displ_;
  it_J0_           = J_;
  it_velo0_        = velo_;
  it_veloAvg0_	   = veloAvg_;
  it_omega0_       = omega_;
  it_omegaAvg0_    = omegaAvg_;
  it_orientation0_ = orientation_;
  it_totalForce0_  = totalForce_;
  it_totalTorque0_ = totalTorque_;

  storage = current;
}

void volumetricParticle::subCyclingSavePreCollision()
{
	  const pointField& current = points();
	        pointField& storage = savedPreCollPoints();

	  preColl_cg0_          = cg_;
	  preColl_displ0_       = displ_;
	  preColl_J0_           = J_;
	  preColl_orientation0_ = orientation_;

	  storage = current;
}

void volumetricParticle::subCyclingRestorePreCollision()
{
  const pointField& storage = savedPreCollPoints();

  cg_          = preColl_cg0_;
  displ_       = preColl_displ0_;
  J_           = preColl_J0_;
  orientation_ = preColl_orientation0_;

  stlPtr_().movePoints(storage);

  validSf_      = false;
  validNormals_ = false;
}

void volumetricParticle::subCyclingSavePosition()
{
	  const pointField& current = points();
	        pointField& storage = savedSubcyclingPoints();

	  storage = current;

	  _DBO_("SAVED WITH CG = " << cg_)
}

void volumetricParticle::subCyclingSaveVelocities()
{
	sc_velo0_        = velo_;
	sc_veloAvg0_	 = veloAvg_;
	sc_omega0_       = omega_;
	sc_omegaAvg0_    = omegaAvg_;
}

void volumetricParticle::subCyclingRestoreState()
{
  const pointField& storage = savedSubcyclingPoints();

  cg_          = sc_cg0_;
  displ_       = sc_displ0_;
  J_           = sc_J0_;
  velo_        = sc_velo0_;
  veloAvg_     = sc_veloAvg0_;
  omega_       = sc_omega0_;
  omegaAvg_    = sc_omegaAvg0_;
  orientation_ = sc_orientation0_;
  totalForce_  = sc_totalForce0_;
  totalTorque_ = sc_totalTorque0_;

  stlPtr_().movePoints(storage);

#if 0
  _DBO_("Restoring state with:"
		  << "\ncg_          = " << sc_cg0_
		  << "\ndispl_       = " << sc_displ0_
		  << "\nJ_           = " << sc_J0_
		  << "\nvelo_        = " << sc_velo0_
		  << "\nomega_       = " << sc_omega0_
		  << "\norientation_ = " << sc_orientation0_
  )
#endif

  validSf_      = false;
  validNormals_ = false;
}

void volumetricParticle::restoreIntermediateState()
{
  const pointField& storage = savedIntermediatePoints();

  cg_          = it_cg0_;
  displ_       = it_displ0_;
  displNext_   = it_displNext0_;
  rotNext_     = it_rotNext0_;
  J_           = it_J0_;
  velo_        = it_velo0_;
  veloAvg_     = it_veloAvg0_;
  omega_       = it_omega0_;
  omegaAvg_    = it_omegaAvg0_;
  orientation_ = it_orientation0_;
  totalForce_  = it_totalForce0_;
  totalTorque_ = it_totalTorque0_;

  stlPtr_().movePoints(storage);

  validSf_      = false;
  validNormals_ = false;
}

void volumetricParticle::iterativeCouplingSavePoints()
{
  const pointField& currentPoints = points();
        pointField& storagePoints = savedIterativeCouplingPoints();

  ic_cg0_          = cg_;
  ic_displ0_       = displ_;
  ic_J0_           = J_;
  ic_orientation0_ = orientation_;

  storagePoints = currentPoints;

}

void volumetricParticle::iterativeCouplingSaveForces()
{

  ic_velo0_        = velo_;
  ic_omega0_       = omega_;


  vectorField&  f  = fluidForceField();
  vectorField&  f0 = savedFluidForceField();

  f0 = f;
}

void volumetricParticle::iterativeCouplingRestorePoints()
{
  const pointField& storage = savedIterativeCouplingPoints();

  cg_          = ic_cg0_;
  displ_       = ic_displ0_;
  J_           = ic_J0_;
  orientation_ = ic_orientation0_;

  stlPtr_().movePoints(storage);

  validSf_      = false;
  validNormals_ = false;
}

void volumetricParticle::iterativeCouplingRestoreForces()
{
  velo_        = ic_velo0_;
  omega_       = ic_omega0_;


  vectorField&  f  = fluidForceField();
  vectorField&  f0 = savedFluidForceField();

  f = f0;

  validSf_      = false;
  validNormals_ = false;
}

void volumetricParticle::iterativeCouplingRelaxForces(scalar relax)
{
  cg_          = relax*cg_          + (1-relax)*ic_cg0_;
  displ_       = relax*displ_       + (1-relax)*ic_displ0_;
  J_           = relax*J_           + (1-relax)*ic_J0_;
  velo_        = relax*velo_        + (1-relax)*ic_velo0_;
  omega_       = relax*omega_       + (1-relax)*ic_omega0_;
  orientation_ = relax*orientation_ + (1-relax)*ic_orientation0_;

  vectorField&  f  = fluidForceField();
  vectorField&  f0 = savedFluidForceField();

  f *= relax;
  f += (1-relax)*f0;
}

void volumetricParticle::calcFluidForces(scalar pRef, scalar rho)
{
  symmTensorField&  stress = stressField();
  scalarField&           p = pressureField();
  vectorField           Sn = Sf(); // copy of area vectors
  vectorField&           f = fluidForceField();

  p *= rho;

  p -= pRef;
//_DBO_(f)
  f = f - (Sn*p);
  f = f - (Sn & stress);  // subtract, because:
                          //   1) normals point into fluid
                          //   2) stress (as in forcesFunctionObject)
                          //      has neg. sign to usually calc forces
                          //      on patches with outward pointing normals
}

void volumetricParticle::calcThermoForces(scalar thermophoreticFactor)
{
  scalarField&           t = thermoField();
  vectorField           Sn = Sf(); // copy of area vectors
  vectorField&           f = thermoForceField();

  f = f + (Sn*t)*thermophoreticFactor;
}


void volumetricParticle::calcSurfaceCharge()  // Calculates induced surface charges from the polarization field
{
  vectorField&		pol = polarField();
  vectorField		Sn = Sf();
  scalarField&		sigma = sigmaField();

   sigma = pol & Sn/mag(Sn);
}

void volumetricParticle::calcElectroMagForces()
{
  _DBO_("\nRunning with surface charges and induction")

  vectorField&          em = electromagField();
  vectorField           Sn = Sf();
  vectorField&           f = electromagForceField();

  calcSurfaceCharge();
  scalarField&	sigma = sigmaField();
  f = f + sigma * em * mag(Sn);
}

void volumetricParticle::calcElectroMagForces(const scalar objectCharge)
{
  _DBO_("\nRunning with point charge: " << objectCharge << " As ")
  vectorField&          em = electromagField();
  vectorField           Sn = Sf();
  vectorField&           f = electromagForceField();
  f = f + em * objectCharge * mag(Sn)/sum(mag(Sn));

  scalarField&		sigma = sigmaField();
  sigma = objectCharge * mag(Sn)/sum(mag(Sn));
}

void volumetricParticle::resetForces()
{
  if(fluidForcePtr_.valid())
    fluidForceField() = vector::zero;
  if(solidForcePtr_.valid())
    solidForceField() = vector::zero;
  if(thermoForcePtr_.valid())
    thermoForceField() = vector::zero;
  if(electromagForcePtr_.valid())
    electromagForceField() = vector::zero;
  if(contactForcePtr_.valid())
    contactForceField() = vector::zero;

/*
  ic_cg0_          = sc_cg0_          = cg_           = vector::zero;
  ic_displ0_       = sc_displ0_       = displ_        = vector::zero;
  ic_velo0_        = sc_velo0_        = velo_         = vector::zero;
  ic_omega0_       = sc_omega0_       = omega_        = vector::zero;
*/
}

void volumetricParticle::resetSolidForces()
{
  if(solidForcePtr_.valid())
  {
    solidForceField() = vector::zero;
  }
}


const vectorField& volumetricParticle::Cf() const
{
  return stlPtr_().faceCentres();
}

const vectorField& volumetricParticle::Sf() const
{
  if( topoChanged_ || !sfPtr_.valid() )
  {
    sfPtr_.reset(
                  new vectorField(triSurf().size(), vector::zero)
                );

    validSf_ = false;
  }

  vectorField& sf = sfPtr_();

  if( !validSf_ )
  {
    const pointField& p = points();
    _DBO_("field was not valid")
    forAll(sf, faceI)
    {
      sf[faceI] = triSurf()[faceI].area(p);//normal(p); since OpenFoam8 its now unit normals!!
      //_DBO_("mag of Normal is set to: " << mag(sf[faceI]) << " for " << idStr())
      //_DBO_("points are " << triSurf()[faceI].points(p))
      //_DBO_("areaNormals would be " << triSurf()[faceI].area(p))
    }
    validSf_ = true;
  }

  return sf;
}

const vectorField& volumetricParticle::normals() const
{
  if( topoChanged_ || !normalsPtr_.valid() )
  {
    normalsPtr_.reset(
                       new vectorField(triSurf().size(), vector::zero)
                     );
    validNormals_ = false;
  }

  vectorField& n = normalsPtr_();

  if( !validNormals_ )
  {
//    _DBO_("resetting normals for " << idStr())
    const pointField& p = points();
    forAll(n, faceI)
    {
      n[faceI] = triSurf()[faceI].normal(p);
      n[faceI] /= mag(n[faceI]);
    }
    validNormals_ = true;
  }

  return n;
}

const pointField&  volumetricParticle::points() const
{
  return stlPtr_().points();
}

triSurface&  volumetricParticle::triSurf() const
{
  return stlPtr_();
}

// return absolute velocity of center of face idx
vector  volumetricParticle::getFaceVelocity(label idx) const
{
  // omega x r  + v
  return (omega_ ^ (Cf()[idx] - cg_)) + velo_;
}

vector  volumetricParticle::getPointVelocity(const point& p) const
{
  // omega x r  + v
  return (omega_ ^ (p - cg_)) + velo_;
}

void  volumetricParticle::isInside(
                                    const pointField& samples,
                                    const boundBox& bb,
                                    List<bool>& inside
                                  ) const
{
  inside.resize(samples.size());

  // find distance of a point lying outside;
  // e.g. 2 times of the span of the mesh's bounding box
  const vector lineDist = 2 * bb.span();

  pointField start(1);
  pointField end(1);

  forAll(samples, pointI)
  {
    start[0] = samples[pointI];
    end[0]   = start[0] + lineDist;

    List< List<pointIndexHit> > lineHits;

    triSurfSearch().findLineAll(start, end, lineHits);

    inside[pointI] = ( lineHits[0].size() % 2 ) ? true : false;
  }
}

// return normal and tangential components (with respect to
// face idx) of:
// relative velocity of: center of face idx
// and center of face salveIdx of the other particle slave
volumetricParticle::vecCmpt
     volumetricParticle::getRelVelocityComponents(
                                                   label idx,
												   const volumetricParticle *slave,
                                                   label slaveIdx
                                                 ) const
{
  vector  relVelo  = slave->getFaceVelocity(slaveIdx);
  relVelo         -=       getFaceVelocity(idx);

  vector  n = Sf()[idx];
  n /= mag(n);

  scalar  normalVelo     = relVelo & n;
  vector  t              = relVelo - normalVelo*n;
  scalar  tangentialVelo = mag(t);

  if( tangentialVelo < VSMALL )
    t = vector::zero;
  else
    t /= tangentialVelo;

  vecCmpt pair;
  pair.n          = n;
  pair.t          = t;
  pair.normal     = normalVelo;
  pair.tangential = tangentialVelo;
  pair.mag        = mag(relVelo);

  return pair;
}

// return normal and tangential components (with respect to
// face idx) of:
// relative velocity of: center of face idx
// and the point pos
volumetricParticle::vecCmpt
     volumetricParticle::getRelVelocityComponents(
                                                   label idx,
                                             const vector &pos
                                                 ) const
{
  vector  relVelo  = - getFaceVelocity(idx);

  vector  n = Sf()[idx];
  n /= mag(n);

  scalar  normalVelo     = relVelo & n;
  vector  t              = relVelo - normalVelo*n;
  scalar  tangentialVelo = mag(t);

  if( tangentialVelo < VSMALL )
    t = vector::zero;
  else
    t /= tangentialVelo;

  vecCmpt pair;
  pair.n          = n;
  pair.t          = t;
  pair.normal     = normalVelo;
  pair.tangential = tangentialVelo;
  pair.mag        = mag(relVelo);

  return pair;
}


// return normal and tangential components (with respect to
// face idx) of:
// distance of: center of face idx
// and center of face salveIdx of the other particle slave
volumetricParticle::vecCmpt
     volumetricParticle::getDistanceComponents   (
                                                   label idx,
                                             const volumetricParticle *slave,
                                                   label slaveIdx
                                                 ) const
{
  vector  dist  = slave->Cf()[slaveIdx];
  dist         -= Cf()[idx];

  vector  n = Sf()[idx];
  n /= mag(n);

  scalar  normalDist     = dist & n;
  vector  t              = dist - normalDist*n;
  scalar  tangentialDist = mag(t);

  if( tangentialDist < VSMALL )
    t = vector::zero;
  else
    t /= tangentialDist;

  vecCmpt pair;
  pair.n          = n;
  pair.t          = t;
  pair.normal     = normalDist;
  pair.tangential = tangentialDist;
  pair.mag        = mag(dist);

  return pair;
}

// return normal and tangential components (with respect to
// face idx) of:
// distance of: center of face idx
// and the point pos
volumetricParticle::vecCmpt
     volumetricParticle::getDistanceComponents   (
                                                   label idx,
                                             const vector &pos
                                                 ) const
{
  vector  dist  = pos - Cf()[idx];

  vector  n = Sf()[idx];
  n /= mag(n);

  scalar  normalDist     = dist & n;
  vector  t              = dist - normalDist*n;
  scalar  tangentialDist = mag(t);

  if( tangentialDist < VSMALL )
    t = vector::zero;
  else
    t /= tangentialDist;

  vecCmpt pair;
  pair.n          = n;
  pair.t          = t;
  pair.normal     = normalDist;
  pair.tangential = tangentialDist;
  pair.mag        = mag(dist);

  return pair;

}

void volumetricParticle::defineContact           (
                                                   label idx,
                                             const volumetricParticle *slave,
                                                   label slaveIdx,
                                                   contactKinetic  &cK,
                                                   contactMechanic &cM
                                                 ) const
{
  vecCmpt dist = getDistanceComponents(idx, slave, slaveIdx);
  vecCmpt velo = getRelVelocityComponents(idx, slave, slaveIdx);

  contactKinetic auxK(
                       &dist.n.x(),     // unit normal of to distance
                       &dist.t.x(),     // unit tangent of to distance
                       dist.normal,     // normal cmpt. of distance
                       dist.tangential, // tang. cmpt. of dist.
                       dist.mag,        // absolute distance
                       &velo.n.x(),     // unit normal of to velocity
                       &velo.t.x(),     // unit tangent of to velocity
                       velo.normal,     // normal cmpt. of velocity
                       velo.tangential, // tang. cmpt. of velocity
                       velo.mag         // absolute velocity
                     );
  cK = auxK;

  scalar area = ( mag( Sf()[idx] ) + mag( slave->Sf()[slaveIdx] ) ) * 0.5;

  contactMechanic auxM(area);
  cM = auxM;
}

void volumetricParticle::defineContact           (
                                                   label idx,
                                                   const vector &pos,
                                                   contactKinetic  &cK,
                                                   contactMechanic &cM
                                                 ) const
{
  vecCmpt dist = getDistanceComponents(idx, pos);
  vecCmpt velo = getRelVelocityComponents(idx, pos);

//  _PDBO_(velo.t << " " << velo.n << " " << velo.tangential << " " << velo.normal)

  contactKinetic auxK(
                     &dist.n.x(),     // unit normal of to distance
                     &dist.t.x(),     // unit tangent of to distance
                     dist.normal,     // normal cmpt. of distance
                     dist.tangential, // tang. cmpt. of dist.
                     dist.mag,        // absolute distance
                     &velo.n.x(),     // unit normal of to velocity
                     &velo.t.x(),     // unit tangent of to velocity
                     velo.normal,     // normal cmpt. of velocity
                     velo.tangential, // tang. cmpt. of velocity
                     velo.mag         // absolute velocity
                     );

  cK = auxK;

  scalar area = mag( Sf()[idx] );

  contactMechanic auxM(area);

  cM = auxM;
}

vector& volumetricParticle::getCg()
{
  return cg_;
}

tensor& volumetricParticle::getOrientation()
{
  return orientation_;
}

vector& volumetricParticle::getTotalForce()
{
	if(myPop_->isPointParticle()) totalForce_ = vector::zero;
	return totalForce_;
}

vector& volumetricParticle::getTotalTorque()
{
  return totalTorque_;
}

vector& volumetricParticle::getAverageVelocity()
{
  return veloAvg_;
}

vector& volumetricParticle::getVelocity()
{
  return velo_;
}

vector volumetricParticle::getKineticEnergy()
{
	vector velo 		= getVelocity();
	vector omega 		= getOmega();

	if(myPop_->isPointParticle()) omega = vector::zero;

	scalar kinEnTrans	= 0.5 * mass_ * velo & velo; // translational component
	scalar kinEnRot 	= 0.5 * omega & (J_ & omega); // rotational component

	return vector(kinEnTrans, kinEnRot, (kinEnTrans + kinEnRot));
}

vector& volumetricParticle::getAverageOmega()
{
  return omegaAvg_;
}

vector& volumetricParticle::getOmega()
{
  return omega_;
}

scalar& volumetricParticle::getRho()
{
  return rho_;
}

scalar& volumetricParticle::getMass()
{
  return mass_;
}

symmTensor& volumetricParticle::getJ()
{
  return J_;
}

symmTensor& volumetricParticle::getAggloJ()
{
  // If a new component joins the agglomerate its aggloJ is 0
  // Only then recalculate aggloJ to save time
  if(aggloJ_ == symmTensor::zero) calcAggloJ();
  return aggloJ_;
}

vector& volumetricParticle::getAcceleration()
{
  return acc_;
}

vector& volumetricParticle::getOmegaAcc()
{
  return omegaAcc_;
}

vector& volumetricParticle::getDispl()
{
  return displ_;
}

vector& volumetricParticle::getDisplNext()
{
 return displNext_;
}

vector& volumetricParticle::getRotNext()
{
 return rotNext_;
}

void volumetricParticle::setExternals(vector exAcc, vector exOAcc, vector exF, vector exT)
{
	externalAcc_ 		= exAcc;
	externalOmegaAcc_ 	= exOAcc;
	externalForce_ 		= exF;
	externalTorque_ 	= exT;
}

// Currently only the drag force is implemented
void volumetricParticle::calcTotalLoadSubCellSize()
{
	return; // Currently don't calculate force, just set velocity equal to fluid velocity

	totalForce_ = vector::zero;

	scalar dt  = time_.deltaT().value() / 15;

	scalar rho_f = 1.2;
	scalar eta_f = 1.81e-5;
	scalar d_p   = 80e-9;

	scalar re_p  = rho_f * mag(velocityAtCgSubCellSize_ - velo_) * d_p / eta_f;

	scalar cd    = (1 + pow(re_p, (2/3)) / 6) * 24 / re_p;

	scalar responseTime = rho_ * d_p * d_p / (18 * eta_f);
	scalar timeFactor   = responseTime / dt;



	totalForce_  = cd * constant::mathematical::pi * d_p * d_p * rho_f
			* mag(velocityAtCgSubCellSize_ - velo_) * (velocityAtCgSubCellSize_ - velo_) / 8
			* timeFactor;

}

void volumetricParticle::calcTotalLoad()
{
  word calcType = myPop_->calcTotalLoadType();

  if(myPop_->isPointParticle())
  {
	  calcTotalLoadSubCellSize();
	  return;
  }

  if(calcType == "general")
  {
	  calcTotalLoadGeneral();
  }
  else if(calcType == "conditioned")
  {
	  calcTotalLoadConditioned();
  }
  else
  {
		FatalErrorIn
			(
			    "volumetricParticle::calcTotalLoad()"
			)   << "Unknown type for population's calcTotalLoadType " << calcType << nl
			    << "Valid types are :" << nl << "general" << nl << "conditioned" << endl
			    << exit(FatalIOError);
  }
}

void volumetricParticle::calcTotalLoadGeneral()
{

  const vectorField &fF = fluidForceField();
  const vectorField &sF = solidForceField();
  const vectorField &tF = thermoForceField();
  const vectorField &emF = electromagForceField();
  const vectorField &cF = contactForceField();
//  vector Fg = mass_*gravity_; //Vora

  totalForce_ = sum( fF ) + sum( sF ) + sum( tF ) + sum ( emF ) + sum ( cF ) + externalForce_;

  //_PDBO_("fluid = " << fF << "\nsolid = " << sF << "\nthermo = " << tF << "\nelmag = " << emF << "\ncontact = " << cF)

  vectorField r = Cf(); // copy
  r -= cg_;

  totalTorque_ = sum( r ^ fF ) + sum( r ^ sF ) + sum( r ^ tF ) + sum( r ^ emF ) + sum( r ^ cF ) + externalTorque_;
//  _DBO_("cg = " << cg_ << nl << "Total Force  = " << totalForce_ << nl << "Total Torque = " << totalTorque_)
}

void volumetricParticle::calcTotalLoadConditioned()
{
	vectorField r = Cf();
	r -= cg_;

	const vectorField &fF = fluidForceField();
	const vectorField &sF = solidForceField();
	const vectorField &cF = contactForceField();

	totalForce_  = sum( fF ) + sum( sF ) + sum( cF ) + externalForce_;
	totalTorque_ = sum( r ^ fF ) + sum( r ^ sF ) + sum( r ^ cF ) + externalTorque_;

	if(myPop_->withElectroMagnetic())
	{
		const vectorField &emF = electromagForceField();
		totalForce_  += sum( emF );
		totalTorque_ += sum( r ^ emF );
	}

	if(myPop_->withThermoPhoresis())
	{
		const vectorField &tF = thermoForceField();
		totalForce_  += sum( tF );
		totalTorque_ += sum( r ^ tF );
	}
}

void volumetricParticle::calcTotalLoadNoAdhesion()
{

  if(myPop_->isPointParticle())
  {
	  calcTotalLoadSubCellSize();
	  return;
  }

  const vectorField &fF = fluidForceField();
  const vectorField &sF = solidForceField();

  totalForce_ = sum( fF ) + sum( sF ) + externalForce_;

  vectorField r = Cf();
  r -= cg_;

  totalTorque_ = sum( r ^ fF ) + sum( r ^ sF ) + externalTorque_;

	if(myPop_->withElectroMagnetic())
	{
		const vectorField &emF = electromagForceField();
		totalForce_  += sum( emF );
		totalTorque_ += sum( r ^ emF );
	}

	if(myPop_->withThermoPhoresis())
	{
		const vectorField &tF = thermoForceField();
		totalForce_  += sum( tF );
		totalTorque_ += sum( r ^ tF );
	}
}

// Calculates force and torque purely from impact force dI/dt = F
// Is used during kin.energy loop in pManager::checkForCollisions
void volumetricParticle::calcCollisionLoad()
{
	const vectorField &sF = solidForceField();

	totalForce_ = sum(sF);

	vectorField r = Cf(); // copy
	r -= cg_;

	totalTorque_ = sum(r ^ sF);
}

// This function is necessary for the momentum conservation during collisions
void volumetricParticle::subtractContactsFromTotalLoad()
{
  const vectorField &cF = contactForceField();

  totalForce_ -= sum ( cF );

  vectorField r = Cf(); // copy
  r -= cg_;

  totalTorque_ -= sum( r ^ cF );
}


void volumetricParticle::calcAcceleration()
{
	//_PDBO_("Calcing Acceleration for " << idStr() << " with J = " << J_ << "\tmass = " << mass_)
  // Calculate inverse of moments of inertia tensor
  symmTensor Jinv(inv(J_));

  // acc of center of gravity
  acc_ = externalAcc_ + totalForce_  / mass_;

  // angular acc: J omeagAcc + omega x J omega = torque
  omegaAcc_ = externalOmegaAcc_ + ( Jinv & (totalTorque_ - (omega_ ^ (J_ & omega_))) );
}

void volumetricParticle::calcVelocity(scalar relax = 1)
{
  scalar dt  = time_.deltaT().value() * relax; // <--- relax is here included

//  // assume velocities to be linear on dt
//  //             ^
//  //             |                       +
//  //             |                  +
//  // veloAvg  -> |             +
//  //             |        +
//  // velo     -> |   +
//  //             |
//  //             |---|-------    ---------|---->
//  //                 \_________  ________/
//  //                           \/
//  //                           dt

  veloAvg_  = velo_ + 0.5*dt*acc_;        // avg. used for time integration
  omegaAvg_ = omega_ + 0.5*dt*omegaAcc_;  // avg. used for time integration

  velo_     += dt*acc_;                   // new velo
  omega_    += dt*omegaAcc_;              // new omega

#if 0
  _DBO_("\nMass = " << mass_ << "; cg = " << cg_ << "\nJ = " << J_ <<
		  "\nveloAvg_ = " << veloAvg_<<"\tomegaAvg_ = " << omegaAvg_ <<"\tvelo_ = " << velo_ << "\tomega_" << omega_ <<
		  "\nacc_ = " << acc_ <<"\tomegaAcc = " << omegaAcc_)
#endif
}

// Takes care of the momentum conservation
// after two particles have agglomerated together
// >>>>> Was used after STLs were merged <<<<<
void volumetricParticle::resolveAdhesion(volumetricParticle& partner)
{
	vector oldCg = cg_;
	// New centre of gravity of the agglomeration
	cg_ = ( cg_ * mass_ + partner.getCg() * partner.getMass() )
			/ ( mass_ + partner.getMass() + VSMALL );
	_DBO_("cg1 = " << oldCg << "\tcg = " << partner.getCg() << "\tcgFinal = " << cg_)


	symmTensor oldJ = J_;
	calcJ();
	_DBO_("J1 = " << oldJ << "\tJ2 = " << partner.getJ() << "\tJFinal = " << J_)
	//J_ += partner.getJ();

	////// Conservation of angular momentum
	// Angular momentum due to particle
	omega_ = ( (oldCg - cg_) ^ (velo_ * mass_) )
			+ (oldJ & omega_);

	// Angular momentum due to partner
	omega_ += ((partner.getCg() - cg_) ^ (partner.getVelocity() * partner.getMass()))
			+ (partner.getJ() & partner.getOmega());

	omega_ = inv(J_) & omega_;
	////// ----- end

	////// Conservation of momentum
	velo_ = ( velo_ * mass_ + partner.getVelocity() * partner.getMass() )
			/ ( mass_ + partner.getMass() + VSMALL );
	////// ----- end

	_DBO_("myMass = " << mass_ << "\tpartnerMass = " << partner.getMass())
}


// Finds closest faces for this particle and partner
// Faces have to be moving towards each other
// writes corresponding labels into myFace and prtFace
void volumetricParticle::findClosestFaces(volumetricParticle& partner, label& myFace, label& prtFace)
{
	const pointField myFaceCenters = Cf();
	const pointField prtFaceCenters = partner.Cf();
	const vectorField myFaceNormals = normals();
	const vectorField prtFaceNormals = partner.normals();

	double minDist = GREAT;
	label minFace1, minFace2;


	// Find facePair minFace1 & minFace2 with minimal distance minDist
	forAll(myFaceCenters, face1I)
	{
		vector myFace = myFaceCenters[face1I];

		forAll(prtFaceCenters, face2I)
		{
			// Make sure faces are not moving apart
			int sign1 = (getFaceVelocity(face1I) & myFaceNormals[face1I]);
			int sign2 = (partner.getFaceVelocity(face2I) & prtFaceNormals[face2I]);
			if (sign1 < 0 && sign2 < 0) continue;

			vector prtFace = prtFaceCenters[face2I];
			vector distVec = prtFace - myFace;
			scalar dist = mag(distVec) * sign((distVec & myFaceNormals[face1I]));
			//distVec /= dist + VSMALL;


			if(dist < minDist) {
				minFace1 = face1I;
				minFace2 = face2I;
				minDist = dist;
			}
		}
	}

	myFace = minFace1;
	prtFace = minFace2;
}


} // namespace Foam
