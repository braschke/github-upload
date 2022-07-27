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

    $Date: 2018-09-24 14:05:00 +0200 (Mon, 24 Sept 2018) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef periodicTranslationConstraint_H
#define periodicTranslationConstraint_H

#include "constraint.H"

#include "Vector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class periodicTranslationConstraint: public Constraint
{
public:

//- Runtime type information
TypeName("periodicTranslationConstraint");

periodicTranslationConstraint(
                          const dictionary     &dict,
                          const objectRegistry &obr,
                          const myMeshSearch   *myMSPtr
                        );

protected:

const Vector<scalar> staticVelocity_;
const scalar period_;

virtual void constrain(volumetricParticle& particle) const;


}; // class periodicTranslationConstraint

} // namespace Foam

#endif // periodicTranslationConstraint_H
