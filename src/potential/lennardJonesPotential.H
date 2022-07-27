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

#ifndef lennardJonesPotential_H
#define lennardJonesPotential_H

#include "potential.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// lennardJonesPotential
//

class lennardJonesPotential: public Potential
{
public:

//- Runtime type information
TypeName("lennardJonesPotential");

lennardJonesPotential(const dictionary &dict);



protected:

// Implementation of v(s) = 1/s^3 * \int_s^\infty \phi(r)r^2\,dr
// With \phi(r) := +Cr/r^12 -Ca/r^6
  scalar v(scalar s) const;

  scalar              rrCa; // rho_1 * rho_2 * Ca
  scalar              rrCr; // rho_1 * rho_2 * Cr

}; // class lennardJonesPotential

} // namespace Foam

#endif // lennardJonesPotential_H
