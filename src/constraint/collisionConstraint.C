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

    $Date: 2018-07-30$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "collisionConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"

#include "RAPID.H" // For collision checks


namespace Foam
{
defineTypeNameAndDebug(collisionConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           collisionConstraint,
                           dictionary
                          );

collisionConstraint::collisionConstraint(
                                              const dictionary     &dict,
                                              const objectRegistry &obr,
                                              const myMeshSearch   *myMSPtr
                                            ):
Constraint(dict, obr, myMSPtr)
{
    cType_   = force;
    nameStr_ = "collisionConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    infoStr_ = aux;
}

void collisionConstraint::constrain(volumetricParticle& particle) const
{
 // do stuff
}


} // namespace Foam

