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

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef potential_H
#define potential_H

#include "messageStream.H"
#include "dictionary.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"

#include "makros.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// Potential base class
//

class Potential
{
public:

declareRunTimeSelectionTable
(
    autoPtr,
    Potential,
    dictionary,
    (
        const dictionary &dict
    ),
    (dict)
);

//- Runtime type information
TypeName("Potential");

static autoPtr<Potential> New(const dictionary &dict);

Potential(const dictionary &d);

virtual ~Potential() {}

virtual string getInfoStr() { return (nameStr_ + infoStr_); }

virtual void info() const
  {
    Info << nl << nameStr_ << infoStr_ << nl;
  }


// Implementation of v(s) = \rho_1\rho_2* 1/s^3 * \int_s^\infty \phi(r)r^2\,dr
virtual scalar v(scalar s) const
  {
    WarningIn("double Potential::v(scalar)")
       << "Not implemented!" << endl;
    return 0.;
  }

protected:

const  dictionary  &dict_;
string nameStr_;
string infoStr_;

double minimalDistance_;


}; // class Potential


} // namespace Foam

#endif // potential_H
