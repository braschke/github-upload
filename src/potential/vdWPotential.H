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

#ifndef vdWPotential_H
#define vdWPotential_H

#include "potential.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// van der Waals force for plate close to half-space
//

class vdWPotential: public Potential
{
public:

//- Runtime type information
TypeName("vdWPotential");

vdWPotential(const dictionary &dict);


protected:

  scalar v(scalar s) const;

  scalar       hamaker; // rho_1 * rho_2 * C

}; // class vdWPotential

} // namespace Foam

#endif // vdWPotential_H
