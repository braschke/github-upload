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

    Kamil Braschke
    Chair of Fluid Mechanics
    braschke@uni-wuppertal.de

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "dumpParticlesInjector.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

namespace Foam
{
defineTypeNameAndDebug(dumpParticlesInjector, 0);
addToRunTimeSelectionTable(
                           Injector,
                           dumpParticlesInjector,
                           dictionary
                          );

dumpParticlesInjector::dumpParticlesInjector(const dictionary &dict):
Injector(dict),
fName_(dict.lookup("file")),
toi_(dict.lookupOrDefault<scalar>("toi", 0.0)),
iterations_(dict.lookupOrDefault<int>("iterations", 100))
{
    nameStr_ = "staticFileInjector class";
    infoStr_ = " (reading from file '" + fName_
             + "', injecting at time= " + _ITOS_(toi_,1) + ")";

    // plan one-time-injection
    nextToi_ = toi_;
}

void dumpParticlesInjector::inject(const Time& t, List<List<scalar> >& injected)
  {
    injected.setSize(0);

    if(
           (t.value()                    <  toi_)
        || (t.value()-t.deltaT().value() >= toi_)
      )
      return;

    IFstream file(fName_);

    if( !file.opened() )
      FatalErrorIn("void staticFileInjector::inject()")
                    << "Could not open file '" << fName_ << "'" << nl
                    << exit(FatalError);


    List<List<scalar>> tempInjected;
    file >> tempInjected;
    for(int i = 0; i < iterations_; i++)
    {
    	injected.append(tempInjected);
    }

    Info << endl << nameStr_ << ": " << injected.size() << " particles to inject." << endl;
  }


} // namespace Foam

