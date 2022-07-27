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

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "londonPotential.H"
#include "addToRunTimeSelectionTable.H"

namespace Foam
{
defineTypeNameAndDebug(londonPotential, 0);
addToRunTimeSelectionTable(
                           Potential,
                           londonPotential,
                           dictionary
                          );

londonPotential::londonPotential(const dictionary &d):
Potential(d),
rrC(3e-21)
{
  nameStr_ = "londonPotential class";

  scalar rho_1 = readScalar( dict_.lookup("rho_1") );
  scalar rho_2 = readScalar( dict_.lookup("rho_2") );
  scalar     C = readScalar( dict_.lookup("C") );

  rrC = rho_1 * rho_2 * C;

}

// Implementation of v(s) = \rho_1 \rho_2 * 1/s^3 * \int_s^\infty \phi(r)r^2\,dr
// With \phi(r) := -C/r^6
// v(s) = -C * rho_1 * rho_2 /3 *1/s^6
scalar londonPotential::v(scalar s) const
  {
    s = (s < VSMALL) ? VSMALL : s;

    scalar  s3 = s*s*s;
    scalar  s6 = s3*s3;

    if( s>=minimalDistance_ )
    {
      return (-rrC/3 / s6);
    }
    else // limit potential to constant \phi(m) for r\in ]0,m]
    {
      scalar  m3  = minimalDistance_ * minimalDistance_ * minimalDistance_;
      scalar  m6 = m3*m3;

      return -rrC/3*(2/m3/s3 - 1/m6);
    }
  }


} // namespace Foam


