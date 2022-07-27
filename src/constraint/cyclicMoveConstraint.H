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

#ifndef cyclicMovementConstraint_H
#define cyclicMovementConstraint_H

#include "constraint.H"

#include "Vector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class cyclicMovementConstraint: public Constraint
{
public:

//- Runtime type information
TypeName("cyclicMovementConstraint");

cyclicMovementConstraint(
                          const dictionary     &dict,
                          const objectRegistry &obr,
                          const myMeshSearch   *myMSPtr
                        );

protected:

      Vector<scalar> origin_;   // origin of system
      Vector<scalar> axisRot_;  // axis of rotation
      Vector<scalar> axisSpan_; // 2nd axis spanning mirroring plane
      Vector<scalar> axisPerp_; // 3rd axis perpendicular to both
const scalar         sectorAngleDeg_;
      scalar         sectorAngle_;

virtual void constrain(volumetricParticle& particle) const;


}; // class cyclicMovementConstraint

} // namespace Foam

#endif // cyclicMovementConstraint_H
