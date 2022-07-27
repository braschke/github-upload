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


#include "periodicInjector.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "Random.H"
#include <chrono>
#include "mpi.h"

namespace Foam
{
defineTypeNameAndDebug(periodicInjector, 0);
addToRunTimeSelectionTable(
                           Injector,
                           periodicInjector,
                           dictionary
                          );

periodicInjector::periodicInjector(const dictionary &dict):
Injector(dict),
fName_(dict.lookup("file")),
toi_(dict.lookupOrDefault<scalar>("toi", 0.0)),
period_(dict.lookupOrDefault<scalar>("period", 0.0)),
randomize_(dict.lookupOrDefault<bool>("randomize", false)),
minDistance_(dict.lookupOrDefault<scalar>("minDistance", 0)),
seed_(dict.lookupOrDefault<unsigned>("seed", std::chrono::system_clock::now().time_since_epoch().count())),
rngStartAfterNCalls_(dict.lookupOrDefault<unsigned>("startAfterNCalls", 0)),
scale_(dict.lookupOrDefault<scalar>("scale", 0.0)),
posX_(dict.lookupOrDefault<scalar>("posX", 0.0)),
posY_(dict.lookupOrDefault<scalar>("posY", 0.0)),
posZ_(dict.lookupOrDefault<scalar>("posZ", 0.0)),
orX_(dict.lookupOrDefault<scalar>("orX", 0.0)),
orY_(dict.lookupOrDefault<scalar>("orY", 0.0)),
orZ_(dict.lookupOrDefault<scalar>("orZ", 0.0)),
velX_(dict.lookupOrDefault<scalar>("velX", 0.0)),
velY_(dict.lookupOrDefault<scalar>("velY", 0.0)),
velZ_(dict.lookupOrDefault<scalar>("velZ", 0.0)),
rngCalls_(0)
{
    nameStr_ = "periodicInjector class";
    if(randomize_) nameStr_ += " with randomization";
    infoStr_ = " (reading from file '" + fName_
             + "', injecting at time= " + _ITOS_(toi_,1) + ")";

    // plan one-time-injection
    nextToi_ = toi_;
    //osRandomSeed(seed_);

    rng_.seed(seed_);
    rngRange_ = rng_.max() - rng_.min();

	// Set position in RNG list in case of continued simulation
	if(rngStartAfterNCalls_) rng_.discard(rngStartAfterNCalls_);
}

// Yields a random value and increases the counter for generated random numbers.
// ONLY USE THIS FOR RNG!!!
double periodicInjector::rng()
{
	rngCalls_++;
	return rng_()/rngRange_;
}

void periodicInjector::inject(const Time& t, List<List<scalar> >& injected)
  {
	// ---------------- Random seed experimente
	/*_DBO_("seed = " << seed_)
	for(int i = 0; i <= 100; i++)
	{
		_DBO_("i = " << i << " \t random = " << )
	}*/
	// --------------------------------

    injected.setSize(0);
    Info << "Current toi = " << toi_ << " for file " << fName_ << endl;

    if(
           (t.value()                    <  toi_)
        || (t.value()-t.deltaT().value() >= toi_)
      )
      return;

    toi_ += period_;

    IFstream file(fName_);

    if( !file.opened() )
      FatalErrorIn("void periodicInjector::inject()")
                    << "Could not open file '" << fName_ << "'" << nl
                    << exit(FatalError);

    file >> injected;
    List<List<scalar> > injectedOrig = injected;

    if( randomize_ )
    {
            List<vector> injectedPositions;
    	    for(int entry = 0; entry < injected.size(); entry++)
        	{
        		vector velocity = vector((injected[entry])[4], (injected[entry])[5], (injected[entry])[6]);
        		scalar velMag = mag(velocity);

        		if(!Pstream::parRun())
        		{
					(injected[entry])[0] *= 1 + ( rng() - 0.5 ) * 2 * scale_;
					(injected[entry])[1] += ( rng() - 0.5 ) * 2 * posX_;
					(injected[entry])[2] += ( rng() - 0.5 ) * 2 * posY_;
					(injected[entry])[3] += ( rng() - 0.5 ) * 2 * posZ_;
					(injected[entry])[4] += velMag * ( rng() - 0.5 ) * 2 * velX_;
					(injected[entry])[5] += velMag * ( rng() - 0.5 ) * 2 * velY_;
					(injected[entry])[6] += velMag * ( rng() - 0.5 ) * 2 * velZ_;
					(injected[entry])[10] += ( rng() - 0.5 ) * 2 * orX_;
					(injected[entry])[11] += ( rng() - 0.5 ) * 2 * orY_;
					(injected[entry])[12] += ( rng() - 0.5 ) * 2 * orZ_;

					injectedPositions.append(vector((injected[entry])[1], (injected[entry])[2], (injected[entry])[3]));
        		}
        		else
        		{
        			double randomValue[10];
        			double randomPosIteration[3];
        			vector originalPos = vector((injected[entry])[1], (injected[entry])[2], (injected[entry])[3]);
        			if(Pstream::myProcNo() == 0)
        			{
        				for(int i = 0; i < 10; i++)
        				{
        					randomValue[i]  = rng();
        				}
        			}
        			//MPI_Bcast(void* data, int count, MPI_Datatype datatype, int root, MPI_Comm communicator)
        			MPI_Barrier(MPI_COMM_WORLD);
        			MPI_Bcast(randomValue, 10, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        			MPI_Barrier(MPI_COMM_WORLD);
        			(injected[entry])[0] *= 1 + ( randomValue[0] - 0.5 ) * 2 * scale_;
					(injected[entry])[1] += ( randomValue[1] - 0.5 ) * 2 * posX_;
					(injected[entry])[2] += ( randomValue[2] - 0.5 ) * 2 * posY_;
					(injected[entry])[3] += ( randomValue[3] - 0.5 ) * 2 * posZ_;

					// Very rudimentary prevention of particle collision at injection through minimal distance minDistance_.
					// Might become problematic when a lot of particles are injected at once.
					vector pos = vector((injected[entry])[1], (injected[entry])[2], (injected[entry])[3]);
					int adjustIterations = 0;
					for(int i = 0; i < injectedPositions.size(); i++)
					{
						if(minDistance_ <= 0) break;

						if(mag(pos - injectedPositions[i]) > minDistance_) continue;

						if(Pstream::myProcNo() == 0)
						{
							for(int i = 0; i < 3; i++)
						    {
								randomPosIteration[i]  = rng();
						    }
						}
						MPI_Barrier(MPI_COMM_WORLD);
						MPI_Bcast(randomPosIteration, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
						MPI_Barrier(MPI_COMM_WORLD);

						(injected[entry])[1] = originalPos.x() + ( randomPosIteration[0] - 0.5 ) * 2 * posX_;
						(injected[entry])[2] = originalPos.y() + ( randomPosIteration[1] - 0.5 ) * 2 * posY_;
						(injected[entry])[3] = originalPos.z() + ( randomPosIteration[2] - 0.5 ) * 2 * posZ_;
						pos = vector((injected[entry])[1], (injected[entry])[2], (injected[entry])[3]);
						i = -1;
						adjustIterations++;
						if(adjustIterations > 100) break;
					}
					if(adjustIterations > 100) // Reset in case of bad distribution
					{
						_PDBOP_("Restarting injection, because plane could not be filled without collisions!!!", 0)
						entry = -1;
						injectedPositions.clear();
						injected = injectedOrig;
						continue;
					}

					(injected[entry])[4] += velMag * ( randomValue[4] - 0.5 ) * 2 * velX_;
					(injected[entry])[5] += velMag * ( randomValue[5] - 0.5 ) * 2 * velY_;
					(injected[entry])[6] += velMag * ( randomValue[6] - 0.5 ) * 2 * velZ_;
					(injected[entry])[10] += ( randomValue[7] - 0.5 ) * 2 * orX_;
					(injected[entry])[11] += ( randomValue[8] - 0.5 ) * 2 * orY_;
					(injected[entry])[12] += ( randomValue[9] - 0.5 ) * 2 * orZ_;

					injectedPositions.append(pos);
					_PDBOP_("injected[entry] = " << injected[entry], 0)
        		}
        	}

        }

    Info << endl << nameStr_ << ": " << injected.size() << " particles to inject. Next toi = " << toi_ << " for file " << fName_ << endl;
    Info << "A total of " << rngCalls_ << " random number calls has been executed since the start of the simulation." << endl;

  }


} // namespace Foam
