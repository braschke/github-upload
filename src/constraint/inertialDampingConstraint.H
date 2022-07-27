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

    $Date: 2012-07-11 11:05:29 +0200 (Wed, 11 Jul 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef inertialDampingConstraint_H
#define inertialDampingConstraint_H

#include "constraint.H"

#include "Vector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class inertialDampingConstraint: public Constraint
{
public:

//- Runtime type information
TypeName("inertialDampingConstraint");

inertialDampingConstraint(
                                const dictionary     &dict,
                                const objectRegistry &obr,
                                const myMeshSearch   *myMSPtr
                              );
protected:

const scalar      dampingFactor_;
// scale dampingFactor (per particle) linearly to 1.0:
// actual_factor(age = 0)         := dampingFactor_
// actual_factor(age = rampTime_) := 1.0
// rampTime <= 0 means constant factor forever
const scalar      rampTime_;

      scalar      slope_; // aux value

virtual void constrain(volumetricParticle& particle) const;


}; // class inertialDampingConstraint

} // namespace Foam

#endif // inertialDampingConstraint_H
