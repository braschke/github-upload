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

#ifndef dumpParticlesInjector_H
#define dumpParticlesInjector_H

#include "injector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// londonPotential
//

class dumpParticlesInjector: public Injector
{
public:

//- Runtime type information
TypeName("dumpParticlesInjector");

dumpParticlesInjector(const dictionary &dict);


protected:

virtual void inject(const Time& t, List<List<scalar> >& injected);

// name of file to read from
const fileName fName_;
// single point in time of injection
const scalar   toi_;
// how often to inject the particles during one injection
const int iterations_;


}; // class dumpParticlesInjector

} // namespace Foam

#endif // dumpParticlesInjector_H
