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

    $Date: 2013-11-19 13:12:28 +0100 (Di, 19 Nov 2013) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef constraint_H
#define constraint_H

#include "messageStream.H"
#include "dictionary.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "Time.H"
#include "Random.H"
//#include "cachedRandom.H"

#include "makros.H"

#include "myMeshSearch.H"

namespace Foam
{
class volumetricParticle;
///////////////////////////////////////////////////////////
//
// Constraint base class
//

class Constraint
{
public:

declareRunTimeSelectionTable
(
    autoPtr,
    Constraint,
    dictionary,
    (
        const dictionary     &dict,
        const objectRegistry &obr,
        const myMeshSearch   *myMSPtr
    ),
    (dict, obr, myMSPtr)
);

enum constraintType
{
  none,
  force,
  acceleration,
  velocity,
  position
};

//- Runtime type information
TypeName("Constraint");

static autoPtr<Constraint> New(
                                const dictionary     &dict,
                                const objectRegistry &obr,
                                const myMeshSearch   *myMSPtr
                              );
Constraint(
            const dictionary     &dict,
            const objectRegistry &obr,
            const myMeshSearch   *myMSPtr
          );

virtual ~Constraint() {}

virtual string getInfoStr() const { return (nameStr_ + infoStr_); }

virtual void info() const
  {
    Info << nl << nameStr_ << ": " << infoStr_ << nl << endl;
  }

bool modifiesForce()        const;
bool modifiesAcceleration() const;
bool modifiesVelocity()     const;
bool modifiesPosition()     const;


virtual void constrain(volumetricParticle& particle) const
  {
    WarningIn("void Constraint::restrict(volumetricParticle& particle)")
       << "Not implemented!" << endl;
  }

protected:


string nameStr_;
string infoStr_;
constraintType cType_;

const  dictionary     &dict_;
const  objectRegistry &obr_;
const  myMeshSearch   *myMSPtr_;


}; // class Constraint


} // namespace Foam

#endif // constraint_H
