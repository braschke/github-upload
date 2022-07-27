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


#include "inertialDampingConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(inertialDampingConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           inertialDampingConstraint,
                           dictionary
                          );

inertialDampingConstraint::inertialDampingConstraint(
                                                                const dictionary     &dict,
                                                                const objectRegistry &obr,
                                                                const myMeshSearch   *myMSPtr
                                                              ):
Constraint(dict, obr, myMSPtr),
dampingFactor_( readScalar( dict.lookup("dampingFactor") ) ),
rampTime_(      readScalar( dict.lookup("rampTime")      ) ),
slope_(1.)
{
    cType_   = acceleration;
    nameStr_ = "inertialDampingConstraint class";


    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "\ndampingFactor = %g\nrampTime = %g",
            dampingFactor_, rampTime_
           );
    infoStr_ = aux;

    // damping decay per unit time
    if( rampTime_ > 0 )
    {
      slope_ = -(dampingFactor_ - 1);
    }
}

void inertialDampingConstraint::constrain(volumetricParticle& particle) const
{
  scalar current = 1.0;

  scalar age = particle.getAge();

  if( rampTime_ <= 0. ) // no ramping
  {
    current = dampingFactor_;
  }
  else
  {
    scalar relTime = age / rampTime_;

    // clamp to [0,1]
    relTime = max(relTime, 0.);
    relTime = min(relTime, 1.);

    current = relTime*slope_ + dampingFactor_;
  }

  particle.getAcceleration() /= current;
  particle.getOmegaAcc() /= current;
//  _PDBO_("damping " << particle.idStr() << " with factor " << current)
}


} // namespace Foam

