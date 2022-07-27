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

    Kamil Braschke
    Chair of Fluid Mechanics
    braschke@uni-wuppertal.de

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "agglomerationConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(agglomerationConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           agglomerationConstraint,
                           dictionary
                          );

agglomerationConstraint::agglomerationConstraint(
                                              const dictionary     &dict,
                                              const objectRegistry &obr,
                                              const myMeshSearch   *myMSPtr
                                            ):
Constraint(dict, obr, myMSPtr),
totalMass_(0),
commonCg_(vector::zero),
velo_(vector::zero),
acc_(vector::zero),
omega_(vector::zero),
omegaAcc_(vector::zero),
torque_(vector::zero),
J_(symmTensor::zero),
time_(obr.time())
{
    cType_   = position;
    nameStr_ = "agglomerationConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "Agglomeration activated."
           );
    infoStr_ = aux;
}

void agglomerationConstraint::constrain(volumetricParticle& particle) const
{
	if(particle.contactPartners_.size() < 1 || particle.calculatedAgglo_ == true)
		return;
	_PDBO_(">>>>>>>>>>>>>>> Entered agglomeration constraint! <<<<<<<<<<<<<<<")

	_PDBO_("particle entering with velo = " << particle.getVelocity())

	calcCommonCgAndMass(particle);
	calcVelo(particle);
	calcJ(particle, commonCg_);
	vector force = calcForce(particle, commonCg_);
	calcTorque(particle, commonCg_);
	calcOmega(particle);

	// Calculate acceleration
	symmTensor Jinv(inv(J_));
	acc_ = force / totalMass_;
	omegaAcc_ = Jinv & (torque_ - (omega_ ^ (J_ & omega_)));

	// Calculate average velocities for time integration
	// For clarification check volumetricParticle::calcAcceleration()
	scalar dt = time_.deltaT().value();
	velo_ += 0.5*dt*acc_;
	omega_ += 0.5*dt*omegaAcc_; //vernachlässigen wegen bug mit zu hohen werten ???

	// Calculate position
	calculateDispl(particle);

	/*_PDBO_("Common mass = "<< totalMass_ << "\tCommon Cg = "<< commonCg_)
	_PDBO_("\nTotal force = " << force <<"\nTotal torque = " << torque_)
	_PDBO_("\nJ = " << J_ << "\nJinv = " << Jinv)
	_PDBO_("dt = " << dt << "\tvelo = " << velo_ << "\tomega = " << omega_)*/
	_PDBO_("\ndisplNext = " << particle.getDisplNext() << "\trotNext = " << particle.getRotNext())

	// Final velocity
	// For clarification check volumetricParticle::calcAcceleration()
	// Impose values on particle
	particle.getOmega() = omega_ + 0.5 * dt * omegaAcc_;
	particle.getVelocity() = velo_ + 0.5 * dt * acc_ + ( particle.getOmega() ^ (particle.getCg() - commonCg_) );

	_PDBO_("\nfinal velo = " << particle.getVelocity() << "\tfinal rotOmega = " << particle.getOmega())

	constrainPartners(particle);
	particle.calculatedAgglo_ = true;


}

void agglomerationConstraint::constrainPartners(volumetricParticle& particle) const
{
	scalar dt = time_.deltaT().value();

	forAll(particle.contactPartners_, partnerI)
	{
		volumetricParticle* partner = particle.contactPartners_[partnerI];
		_PDBO_("Partner using velo = " << velo_ << " and dist = " << partner->getCg() - commonCg_)
		//partner->getDisplNext() = dt * velo_ + (( partner->getCg() - commonCg_ ) ^ omega_ * dt);
		//partner->getRotNext() = dt * omega_;
		_PDBO_("\nPARTNERdisplNext = " << partner->getDisplNext() << "\tPARTNERrotNext = " << partner->getRotNext())
		partner->getOmega() = omega_ + 0.5 * dt *omegaAcc_;
		partner->getVelocity() = velo_ + 0.5 * dt * acc_ + ( partner->getOmega() ^ (partner->getCg() - commonCg_) );
		partner->calculatedAgglo_ = true;
	}
}

void agglomerationConstraint::calculateDispl(volumetricParticle& particle) const
{
	List<vector> displacements(particle.contactPartners_.size() + 1);

	// Get vectors pointing to Cgs of all particles from Cg of agglom.
	displacements[0] = particle.getCg() - commonCg_;
	forAll(particle.contactPartners_, partnerI)
	{
		displacements[partnerI + 1] = particle.contactPartners_[partnerI]->getCg() - commonCg_;
	}

	scalar dt = time_.deltaT().value();
	vector rotation = dt * omega_;
	vector displ = dt * velo_;

	vector newCg = commonCg_ + displ;

	// Rotation around commonCg_
	// For more details see voluemtricParticle::kinetic()
	scalar theta = mag(rotation);
	tensor rot = I;
	if(theta > SMALL)
	{
		rotation /= theta;
		theta = std::fmod(theta, constant::mathematical::twoPi);
		quaternion q(rotation, theta);
		rot = q.R();
	}
	// Displacement due to that rotation
	displacements[0] = displ + (rot & displacements[0]) - ( particle.getCg() - commonCg_ );
	forAll(particle.contactPartners_, partnerI)
	{
		displacements[partnerI + 1] = displ + (rot & displacements[partnerI + 1]) - ( particle.getCg() - commonCg_ );
		particle.contactPartners_[partnerI]->getDisplNext() = displacements[partnerI + 1];
		particle.contactPartners_[partnerI]->getRotNext() = rotation;
	}



	// Impose position on particle
	//_PDBO_("Main: using velo = " << velo_ << " and dist = " << particle.getCg() - commonCg_)
	//displ +=  ( particle.getCg() - commonCg_ )  ^ omega_ * dt; // Additional translation due to rotation around agglom. Cg

	particle.getDisplNext() = displacements[0];
	particle.getRotNext() = rotation;
}

void agglomerationConstraint::calcCommonCgAndMass(volumetricParticle& particle) const
{
	vector commonCg = particle.getCg() * particle.getMass();
	scalar totalMass = particle.getMass();


	forAll(particle.contactPartners_, partnerI)
		{
			volumetricParticle* partner = particle.contactPartners_[partnerI];
			commonCg += partner->getCg() * partner->getMass();
			totalMass += partner->getMass();
		}

	commonCg /= totalMass + VSMALL;
	totalMass_ = totalMass;
	commonCg_ = commonCg;
	if(particle.commonCg_ != vector(GREAT, GREAT, GREAT))
		commonCg_ = particle.commonCg_;
	else
	{
		particle.commonCg_ = commonCg_;
		forAll(particle.contactPartners_, partnerI)
		{
			particle.contactPartners_[partnerI]->commonCg_ = commonCg_;
		}
	}
}

vector agglomerationConstraint::calcForce(volumetricParticle& particle, vector cg) const
{
	vector force = particle.getTotalForce();

	forAll(particle.contactPartners_, partnerI) {
		force += particle.contactPartners_[partnerI]->getTotalForce();
	}

	if(particle.commonForce_ != vector(GREAT, GREAT, GREAT))
		force = particle.commonForce_;
	else
	{
		particle.commonForce_ = force;
		forAll(particle.contactPartners_, partnerI)
		{
			particle.contactPartners_[partnerI]->commonForce_ = force;
		}
	}
	return force;
}

void agglomerationConstraint::calcTorque(volumetricParticle& particle, vector cg) const
{
	if(particle.calculatedAgglo_)
	{
		torque_ = particle.commonTorque_;
		return;
	}

	const vectorField &fF = particle.fluidForceField();
	const vectorField &sF = particle.solidForceField();
	const vectorField &tF = particle.thermoForceField();
	const vectorField &emF = particle.electromagForceField();
	const vectorField &cF = particle.contactForceField();

	vectorField r = particle.Cf();
	r -= cg;

	torque_ = sum( r ^ fF ) + sum( r ^ sF ) + sum( r ^ tF ) + sum( r ^ emF ) + sum( r ^ cF );

	forAll(particle.contactPartners_, partnerI)
		{
			volumetricParticle* partner = particle.contactPartners_[partnerI];

			const vectorField &fFP = partner->fluidForceField();
			const vectorField &sFP = partner->solidForceField();
			const vectorField &tFP = partner->thermoForceField();
			const vectorField &emFP = partner->electromagForceField();
			const vectorField &cFP = partner->contactForceField();

			r = partner->Cf();
			r -= cg;
			torque_ += sum( r ^ fFP ) + sum( r ^ sFP ) + sum( r ^ tFP ) + sum( r ^ emFP ) + sum( r ^ cFP );
		}

	particle.commonTorque_ = torque_;
	forAll(particle.contactPartners_, partnerI)
	{
		particle.contactPartners_[partnerI]->commonTorque_ = torque_;
	}
}

// For more Info check volumetricParticle::calcJ
void agglomerationConstraint::calcJ(volumetricParticle& particle, vector cg) const
{


	// Check if commonJ_ has already been calculated
	if(particle.calculatedAgglo_)
	{
		J_ = particle.commonJ_;
		return;
	}

  vectorField c = particle.Cf();
  vectorField a = particle.Sf();
  scalar rho = particle.getRho();

  symmTensor J = symmTensor::zero;

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
  J *= rho;


  forAll(particle.contactPartners_, partnerI)
  {
	  volumetricParticle* partner = particle.contactPartners_[partnerI];
	  vectorField cP = partner->Cf();
	  vectorField aP = partner->Sf();
	  rho = partner->getRho();
	  symmTensor JPartner = symmTensor::zero;

	  forAll(cP, faceI)
	  {
	    vector cf  = cP[faceI] - cg;
	    scalar xyz = cf.x() * cf.y() * cf.z();
	    scalar zzx = cf.z() * cf.z() * cf.x();
	    scalar xxy = cf.x() * cf.x() * cf.y();
	    scalar yyz = cf.y() * cf.y() * cf.z();

	    JPartner.xy() += xyz * aP[faceI].z();
	    JPartner.xz() += xyz * aP[faceI].y();
	    JPartner.yz() += xyz * aP[faceI].x();
	    JPartner.xx() += zzx * aP[faceI].x() +                      yyz * aP[faceI].z();
	    JPartner.yy() += zzx * aP[faceI].x() + xxy * aP[faceI].y()                     ;
	    JPartner.zz() +=                      xxy * aP[faceI].y() + yyz * aP[faceI].z();
	  }

	  JPartner *= rho;
	  J += JPartner;
  }
  J_ = J;

  // Distribute commonJ_
  forAll(particle.contactPartners_, partnerI)
  {
  	particle.contactPartners_[partnerI]->commonJ_ = J_;
  }
}

void agglomerationConstraint::calcVelo(volumetricParticle& particle) const
{
	if(particle.calculatedAgglo_)
	{
		velo_ = particle.commonVelo_;
		return;
	}

	velo_ = particle.getVelocity() * particle.getMass();
	scalar totalMass = particle.getMass();

	forAll(particle.contactPartners_, partnerI)
	{
		volumetricParticle* partner = particle.contactPartners_[partnerI];
		velo_ += partner->getVelocity() * partner->getMass();
		totalMass += partner->getMass();
	}

	velo_ /= totalMass + VSMALL;

	  // Distribute commonVelo_
	  forAll(particle.contactPartners_, partnerI)
	  {
	  	particle.contactPartners_[partnerI]->commonVelo_ = velo_;
	  }
}

void agglomerationConstraint::calcOmega(volumetricParticle& particle) const
{
	if(particle.calculatedAgglo_)
	{
		omega_ = particle.commonOmega_;
		return;
	}

	omega_ = ( (particle.getCg() - commonCg_) ^ (particle.getVelocity() * particle.getMass()) ) + (particle.getJ() & particle.getOmega());

	forAll(particle.contactPartners_, partnerI)
	{
		volumetricParticle* partner = particle.contactPartners_[partnerI];
		omega_ += ((partner->getCg() - commonCg_) ^ (partner->getVelocity() * partner->getMass())) + (partner->getJ() & partner->getOmega());
	}

	omega_ = inv(J_) & omega_;

	  // Distribute commonOmega_
	  forAll(particle.contactPartners_, partnerI)
	  {
	  	particle.contactPartners_[partnerI]->commonOmega_ = omega_;
	  }
}


} // namespace Foam

