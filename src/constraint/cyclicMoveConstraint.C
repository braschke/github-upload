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

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "cyclicMoveConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"

#define _2PI_ constant::mathematical::twoPi


namespace Foam
{
defineTypeNameAndDebug(cyclicMovementConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           cyclicMovementConstraint,
                           dictionary
                          );

cyclicMovementConstraint::cyclicMovementConstraint(
                                                    const dictionary     &dict,
                                                    const objectRegistry &obr,
                                                    const myMeshSearch   *myMSPtr
                                                  ):
Constraint(dict, obr, myMSPtr),
origin_( dict.lookup("origin") ),
axisRot_( dict.lookup("axisRot") ),
axisSpan_( dict.lookup("axisSpan") ),
axisPerp_( axisRot_ ^ axisSpan_ ),
sectorAngleDeg_( readScalar(dict.lookup("sectorAngleDeg")) )
{
    cType_   = position;
    nameStr_ = "cyclicMovementConstraint class";

    // normalize axes
    axisRot_  /= mag(axisRot_);
    axisSpan_ /= mag(axisSpan_);
    axisPerp_ /= mag(axisPerp_);

    sectorAngle_ = sectorAngleDeg_ * _2PI_ / 360;

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "axisRot = (%g, %g, %g)\naxisSpan = (%g, %g, %g)\nsectorAngleDeg = %g",
            axisRot_.x(), axisRot_.y(), axisRot_.z(),
            axisSpan_.x(), axisSpan_.y(), axisSpan_.z(),
            sectorAngleDeg_
           );
    infoStr_ = aux;
}

void cyclicMovementConstraint::constrain(volumetricParticle& particle) const
{
  // where will it go?
  vector cg = particle.cg() + particle.getDisplNext() - origin_;

// calc deviation phi from mirroring plane:
// 1) project into (axisSpan_,axisPerp_)-plane
  vector cgProj = cg - (cg & axisRot_)*axisRot_;
// 2) angle to axis span: <a,b> = ||a|| * ||b|| * cos phi
  scalar phi = std::acos( (cgProj & axisSpan_) / (mag(cgProj)*mag(axisSpan_)) );

  if( abs(phi) < sectorAngle_ )
    return;

  // now we have to do some rotations by -phi about axisRot, i.e. -phi*axisRot_
  // so, construct a rotation matrix
  scalar  theta = -phi;  // angle

  tensor  rot   = I;       // Init rotation matrix with identity

  if(mag(theta) > SMALL)
  {
    // eliminate redundant rotations -> euler angle
    theta = std::fmod(theta, _2PI_);
    // Calculate rotation matrix from euler axis/angle
    quaternion q(axisRot_, theta);
    rot = q.R();
  }

  // rotate velocities (only for initial values of next time step)
  particle.getVelocity()        = rot & particle.getVelocity();
  particle.getAverageVelocity() = rot & particle.getAverageVelocity();
  particle.getOmega()           = rot & particle.getOmega();
  particle.getAverageOmega()    = rot & particle.getAverageOmega();
  //rotate cg, note: cg is relative to origin_
  cg = rot & cg;
  // make cg absolute; to here it must go
  cg += origin_;
  // so next displ is
  particle.getDisplNext() = cg - particle.cg();
  // next rot is
  particle.getRotNext() = rot & particle.getRotNext();

  _DBI_

}


} // namespace Foam

#undef _2PI_


