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

#include "potential.H"


namespace Foam
{
defineTypeNameAndDebug(Potential, 0);
defineRunTimeSelectionTable(Potential, dictionary);


autoPtr<Potential> Potential::New(const dictionary &dict)
{

  Istream& nameStream = dict.lookup("type");
  const word potentialName( nameStream );

  dictionaryConstructorTable::iterator cstrIter =
      dictionaryConstructorTablePtr_->find(potentialName);

  if (cstrIter == dictionaryConstructorTablePtr_->end())
  {
      FatalErrorIn
      (
          "Potential::New(const dictionary &dict)"
      )   << "Unknown potential " << potentialName << nl << nl
          << "Valid potential names are :" << endl
          << dictionaryConstructorTablePtr_->sortedToc()
          << exit(FatalIOError);
  }

  return cstrIter()(dict);
}

Potential::Potential(const dictionary &d):
dict_(d),
nameStr_("Potential base class not implemented!"),
infoStr_(),
minimalDistance_(VSMALL)
{
  dict_.readIfPresent<scalar>("minimalDistance", minimalDistance_);

  if(minimalDistance_ > VSMALL)
  {
    infoStr_  = " (cropped at minimal distance of ";
    infoStr_ += _ITOS_(minimalDistance_,0);
    infoStr_ += ")";
  }
}

} // namespace Foam

