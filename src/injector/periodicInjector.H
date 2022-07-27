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

#ifndef periodicInjector_H
#define periodicInjector_H

#include "injector.H"
#include <random>

namespace Foam
{

///////////////////////////////////////////////////////////
//
// londonPotential
//

class periodicInjector: public Injector
{
public:

//- Runtime type information
TypeName("periodicInjector");

periodicInjector(const dictionary &dict);


protected:

virtual void inject(const Time& t, List<List<scalar> >& injected);

double rng();

// name of file to read from
const fileName fName_;
// first point in time of injection
scalar   toi_;
// period of injection
const scalar   period_;
// Use random variation for injection parameters?
const bool		randomize_;
// What should be the minimum distance between randomly injected particles?
const scalar minDistance_;
unsigned	seed_;
// Maximum values for random position added on actual position and orientation
// (absolute value, so posX = 0.1 is plus/minus 0.1m)
const scalar posX_, posY_, posZ_, orX_, orY_, orZ_;
// Maximum values for random scale and position added on actual position
// (fraction of actual value, e.g. velX = 0.1 = 10% with mag(vel) = 10 means between 10.1 and 9.9 )
const scalar scale_, velX_, velY_, velZ_;

std::linear_congruential_engine<unsigned int, 48271, 0, 2147483647> rng_; // Corresponds to "Minimum standard", recommended by Park, Miller, and Stockmeyer in 1993
double rngRange_;
unsigned int rngCalls_;
unsigned int rngStartAfterNCalls_;


}; // class periodicInjector

} // namespace Foam

#endif // periodicInjector_H
