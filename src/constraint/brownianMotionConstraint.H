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

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef brownianMotionConstraint_H
#define brownianMotionConstraint_H

#include "constraint.H"

#include "Vector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class brownianMotionConstraint: public Constraint
{
public:

//- Runtime type information
TypeName("brownianMotionConstraint");

brownianMotionConstraint(
                          const dictionary     &dict,
                          const objectRegistry &obr,
                          const myMeshSearch   *myMSPtr
                        );

protected:

int subcycles_;
scalar constDisplacement_;
scalar diffFactor_;

virtual void constrain(volumetricParticle& particle) const;


}; // brownianMotionConstraint

} // namespace Foam

#endif // brownianMotionConstraint_H
