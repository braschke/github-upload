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

    $Date: 2013-10-28 08:53:30 +0100 (Mo, 28 Okt 2013) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "injector.H"


namespace Foam
{
defineTypeNameAndDebug(Injector, 0);
defineRunTimeSelectionTable(Injector, dictionary);


autoPtr<Injector> Injector::New(const dictionary &dict)
{

  Istream& nameStream = dict.lookup("type");
  const word injectorName( nameStream );

  dictionaryConstructorTable::iterator cstrIter =
      dictionaryConstructorTablePtr_->find(injectorName);

  if (cstrIter == dictionaryConstructorTablePtr_->end())
  {
      FatalErrorIn
      (
          "Injector::New(const dictionary &dict)"
      )   << "Unknown injector " << injectorName << nl << nl
          << "Valid ijector names are :" << endl
          << dictionaryConstructorTablePtr_->sortedToc()
          << exit(FatalIOError);
  }

  return cstrIter()(dict);
}

Injector::Injector(const dictionary &dict):
nameStr_("Injector base class not implemented!"),
infoStr_(),
dict_(dict),
nextToi_(VGREAT),
seed_(dict.lookupOrDefault<label>("seed", 0)),
rndCount_(dict.lookupOrDefault<label>("rndCache", 0)),
rnd_(seed_)//, rndCount_) CHANGED
{

}


} // namespace Foam

