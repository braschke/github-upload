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


#include "staticVelocityConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(staticVelocityConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           staticVelocityConstraint,
                           dictionary
                          );

staticVelocityConstraint::staticVelocityConstraint(
                                                    const dictionary     &dict,
                                                    const objectRegistry &obr,
                                                    const myMeshSearch   *myMSPtr
                                                  ):
Constraint(dict, obr, myMSPtr),
staticVelocity_( dict.lookup("staticVelocity") )
{
    cType_   = acceleration;
    nameStr_ = "staticVelocityConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "staticVelocity = (%g, %g, %g)",
            staticVelocity_.x(), staticVelocity_.y(), staticVelocity_.z()
           );
    infoStr_ = aux;
}

void staticVelocityConstraint::constrain(volumetricParticle& particle) const
{
//  Vector<scalar>  fixedAxis(1,0,0);
//  Vector<scalar>  curOmegaAcc = particle.getOmegaAcc();
//  Vector<scalar>  newOmegaAcc = (curOmegaAcc & fixedAxis) * fixedAxis; // project to fixedAxis
//_DBO_("CODE CHANGE: curOmegaAcc = " << curOmegaAcc << " newOmegaAcc = " << newOmegaAcc)

//  particle.getOmegaAcc() = newOmegaAcc;
       // ( newOmega - curOmega ) / obr_.time().deltaT().value();



  particle.getAcceleration() =
        ( staticVelocity_ - particle.getVelocity() ) / obr_.time().deltaT().value();
}


} // namespace Foam

