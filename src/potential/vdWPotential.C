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


#include "vdWPotential.H"
#include "addToRunTimeSelectionTable.H"
#include "makros.H"
#include <math.h>


namespace Foam
{
defineTypeNameAndDebug(vdWPotential, 0);
addToRunTimeSelectionTable(
                           Potential,
                           vdWPotential,
                           dictionary
                          );

vdWPotential::vdWPotential(const dictionary &d):
Potential(d)
{
  nameStr_ = "vdWPotential class";

  hamaker = readScalar( dict_.lookup("hamaker") );

}

scalar vdWPotential::v(scalar s) const
  {
	const scalar pi = M_PI;
    s = (s < VSMALL) ? VSMALL : s;

    scalar  s3 = s*s*s;

    if( s>=minimalDistance_ )
    {
      return (- hamaker / (pi * 6 * s3));
    }
    else // limit potential to constant \phi(m) for r\in ]0,m]
    {
      scalar  m3  = minimalDistance_ * minimalDistance_ * minimalDistance_;

      return (- hamaker / (pi * 6 * m3));
    }
  }


} // namespace Foam


