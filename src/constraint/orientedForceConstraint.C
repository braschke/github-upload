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


#include "orientedForceConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(orientedForceConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           orientedForceConstraint,
                           dictionary
                          );

orientedForceConstraint::orientedForceConstraint(
                                                  const dictionary     &dict,
                                                  const objectRegistry &obr,
                                                  const myMeshSearch   *myMSPtr
                                                ):
Constraint(dict, obr, myMSPtr),
initialForce_( dict.lookup("initialForce") )
{
    cType_   = force;
    nameStr_ = "orientedForceConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "initialForce = (%g, %g, %g)",
            initialForce_.x(), initialForce_.y(), initialForce_.z()
           );
    infoStr_ = aux;
}

void orientedForceConstraint::constrain(volumetricParticle& particle) const
{
  particle.getTotalForce() = particle.getOrientation() & initialForce_;
}


} // namespace Foam

