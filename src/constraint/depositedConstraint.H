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

    $Date: 2012-07-11 11:05:29 +0200 (Wed, 11 Jul 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef depositedConstraint_H
#define depositedConstraint_H

#include "constraint.H"

#include "Vector.H"

#include "triSurface.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class depositedConstraint: public Constraint
{
public:

//- Runtime type information
TypeName("depositedConstraint");

depositedConstraint(
                     const dictionary     &dict,
                     const objectRegistry &obr,
                     const myMeshSearch   *myMSPtr
                   );

protected:


virtual void constrain(volumetricParticle& particle) const;
vector getVelocityReduction(vector v_vec, vector n_vec) const;


}; // class depositedConstraint

} // namespace Foam

#endif // depositedConstraint_H
