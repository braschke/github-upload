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

#ifndef injector_H
#define injector_H

#include "messageStream.H"
#include "dictionary.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "Time.H"
//#include "cachedRandom.H" CHANGED
#include "Random.H"

#include "makros.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// Injector base class
//

class Injector
{
public:

declareRunTimeSelectionTable
(
    autoPtr,
    Injector,
    dictionary,
    (
        const dictionary &dict
    ),
    (dict)
);

//- Runtime type information
TypeName("Injector");

static autoPtr<Injector> New(const dictionary &dict);

Injector(const dictionary &dict);

virtual ~Injector() {}

virtual string getInfoStr() { return (nameStr_ + infoStr_); }

virtual void info() const
  {
    Info << nl << nameStr_ << infoStr_ << nl;
  }



virtual void inject(const Time& t, List<List<scalar> >& injected)
  {
    WarningIn("void Injector::inject(Time& t, List<scalar>& injected)")
       << "Not implemented!" << endl;
  }

protected:

string nameStr_;
string infoStr_;

const  dictionary  &dict_;

// next point in time of injection
scalar nextToi_;

// random generator
const label  seed_;
const label  rndCount_;
Random rnd_;//cachedRandom rnd_; CHANGED

}; // class Injector


} // namespace Foam

#endif // injector_H
