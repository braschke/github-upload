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

#ifndef staticFileInjector_H
#define staticFileInjector_H

#include "injector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// londonPotential
//

class staticFileInjector: public Injector
{
public:

//- Runtime type information
TypeName("staticFileInjector");

staticFileInjector(const dictionary &dict);


protected:

virtual void inject(const Time& t, List<List<scalar> >& injected);

// name of file to read from
const fileName fName_;
// single point in time of injection
const scalar   toi_;


}; // class staticFileInjector

} // namespace Foam

#endif // staticFileInjector_H
