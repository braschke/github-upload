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


#include "lennardJonesPotential.H"
#include "addToRunTimeSelectionTable.H"

namespace Foam
{
defineTypeNameAndDebug(lennardJonesPotential, 0);
addToRunTimeSelectionTable(
                           Potential,
                           lennardJonesPotential,
                           dictionary
                          );

lennardJonesPotential::lennardJonesPotential(const dictionary &d):
Potential(d),
rrCa(3e-21),
rrCr(1.5e-24)
{
  nameStr_ = "lennardJonesPotential class";

  scalar rho_1  = readScalar( dict_.lookup("rho_1") );
  scalar rho_2  = readScalar( dict_.lookup("rho_2") );
  // \Phi(r) = eps*( (rm/r)^12 - 2*(rm/r)^6 )
  // \Phi has in r := rm its minimum:
  // \Phi(r_m) = \epsilon
  scalar    eps = readScalar( dict_.lookup("epsilon") );
  scalar    rm  = readScalar( dict_.lookup("rMin") );
  // Coefficients for attraction (a) and repulsion (r)
  scalar  rm6 = rm*rm*rm;
  rm6   = rm6*rm6;
  scalar    Ca  = 2*eps*rm6;     // C_a := 2\eps r_m^6
  scalar    Cr  =   eps*rm6*rm6; // C_r :=  \eps r_m^{12}
_DBO_("\n\nCa = " << Ca << "; Cr = " << Cr)
  rrCa = rho_1 * rho_2 * Ca;
  rrCr = rho_1 * rho_2 * Cr;

}

// Implementation of v(s) = \rho_1\rho_2* 1/s^3 * \int_s^\infty \phi(r)r^2\,dr
// With \phi(r) := +Cr/r^12 -Ca/r^6
// v(s) = 1/s^6 * [ rrCr/9/s^6 - rrCa/3 ]
scalar lennardJonesPotential::v(scalar s) const
  {
    s = (s < VSMALL) ? VSMALL : s;

    scalar  s3 = s*s*s;
    scalar  s6 = s3*s3;
    scalar rs6 = 1/s6;

    if( s>=minimalDistance_ )
    {
      return rs6 * ( rrCr/9*rs6 -rrCa/3 );
    }
    else // limit potential to constant \phi(m) for r\in ]0,m]
    {
      scalar  m3  = minimalDistance_ * minimalDistance_ * minimalDistance_;
      scalar  m6  = m3*m3;
      scalar  m9  = m6*m3;
      scalar  m12 = m6*m6;

      return ( rrCr*( 4/(3*s3*m9) - 1/m12 )  - rrCa*( 2/(s3*m3) - 1/m6 ) ) / 3;
    }

  }


} // namespace Foam

