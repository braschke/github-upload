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


#include "addForceConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(addForceConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           addForceConstraint,
                           dictionary
                          );

addForceConstraint::addForceConstraint(
                                              const dictionary     &dict,
                                              const objectRegistry &obr,
                                              const myMeshSearch   *myMSPtr
                                            ):
Constraint(dict, obr, myMSPtr),
addForce_( dict.lookup("addForce") )
{
    cType_   = force;
    nameStr_ = "addForceConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "addForce = (%g, %g, %g)",
            addForce_.x(), addForce_.y(), addForce_.z()
           );
    infoStr_ = aux;
}

void addForceConstraint::constrain(volumetricParticle& particle) const
{
  particle.getTotalForce() += addForce_;
}


} // namespace Foam

