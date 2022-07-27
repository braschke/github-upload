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

    Mehdi Baba Mehdi
    Chair of Fluid Mechanics

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "randomDistInjector.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "random.H"



namespace Foam
{
defineTypeNameAndDebug(randomDistInjector, 0);
addToRunTimeSelectionTable(
                           Injector,
                           randomDistInjector,
                           dictionary
                          );

randomDistInjector::randomDistInjector(const dictionary &dict):
Injector(dict),
fName_(dict.lookup("file")),
toi_(dict.lookupOrDefault<scalar>("toi", 0.0)),
period_(dict.lookupOrDefault<scalar>("period", 0.0)),
variance_(dict.lookupOrDefault<scalar>("variance", 0.0)),
numberOfObject_(dict.lookupOrDefault<scalar>("numberOfObjects", 1.0)),
distributionFunction_(dict.lookupOrDefault<string>("distributionFunction", "guassian")),
minDist_(dict.lookupOrDefault<scalar>("minDist", 0.0)),
normalOfPlane_(dict.lookupOrDefault<string>("normalOfPlane", "x")),
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
omegaX_(dict.lookupOrDefault<scalar>("omegaX", 0.0)),
omegaY_(dict.lookupOrDefault<scalar>("omegaY", 0.0)),
omegaZ_(dict.lookupOrDefault<scalar>("omega", 0.0))
{
    nameStr_ = "randomDistInjector class";
    infoStr_ = " (reading from file '" + fName_
             + "', injecting at time= " + _ITOS_(toi_,1) + ")";

    // plan one-time-injection
    nextToi_ = toi_;
}

void randomDistInjector::inject(const Time& t, List<List<scalar> >& injected)
{
    injected.setSize(0); 
        
    int factor = ( t.value() - toi_ ) / period_;
    toi_ += factor * period_;

    if(
           (t.value()                    <  toi_)
        || (t.value()-t.deltaT().value() >= toi_)
      )
      return;

    IFstream file(fName_);

    if( !file.opened() )
      FatalErrorIn("void randomDistInjector::inject()")
                    << "Could not open file '" << fName_ << "'" << nl
                    << exit(FatalError);

    // temporary list to store given values in file, used as mean value to generate random numbers
    List<List<scalar>> tmpList;
    file >> tmpList;
     
    int particleInEachDir = ceil(sqrt(numberOfObject_));

    int numObject ((scalar)numberOfObject_);    
    List<scalar>* rndParticle = new List<scalar>[numObject];
    
    scalar* randomValue = new scalar[numObject];
    randomDistHandler(distributionFunction_, randomValue, (tmpList[0])[0], variance_, numObject);

    // point to the first element of array pointers
    auto firstElementParticle = rndParticle;
    auto firstElementValue = randomValue;
    
    // used to increase the distance between particles when collision happen
    scalar factorOfSafty = 1.1;
    int count = 1;
    
    _DBO_(" Try " << count++ << "\n")
    while(true)
    {
        for(int i = 0; i < numberOfObject_; i++)
        {
            rndParticle->append(*(randomValue++));
            if(normalOfPlane_ == "x" || normalOfPlane_ == "X")
            {
               rndParticle->append((tmpList[0])[1]);
               rndParticle->append((tmpList[0])[2] - particleInEachDir * minDist_ * factorOfSafty / 2 + floor(i / particleInEachDir) * minDist_ * factorOfSafty);        
               rndParticle->append((tmpList[0])[3] - particleInEachDir * minDist_ * factorOfSafty / 2 + (i%particleInEachDir) * minDist_ * factorOfSafty);
            } else if ( normalOfPlane_ == "y" || normalOfPlane_ == "Y")
            {
               rndParticle->append((tmpList[0])[1] - particleInEachDir * minDist_ * factorOfSafty / 2 + floor(i / particleInEachDir) * minDist_ * factorOfSafty);        
               rndParticle->append((tmpList[0])[2]);
               rndParticle->append((tmpList[0])[3] - particleInEachDir * minDist_ * factorOfSafty / 2 + (i%particleInEachDir) * minDist_ * factorOfSafty);
            } else if (normalOfPlane_ == "z" || normalOfPlane_ == "Z")
            {
               rndParticle->append((tmpList[0])[1] - particleInEachDir * minDist_ * factorOfSafty / 2 + floor(i / particleInEachDir) * minDist_ * factorOfSafty);        
               rndParticle->append((tmpList[0])[2] - particleInEachDir * minDist_ * factorOfSafty / 2 + (i%particleInEachDir) * minDist_ * factorOfSafty);
               rndParticle->append((tmpList[0])[3]);
            }
            rndParticle->append((tmpList[0])[4]);
            rndParticle->append((tmpList[0])[5]);
            rndParticle->append((tmpList[0])[6]);
            rndParticle->append((tmpList[0])[7]);
            rndParticle->append((tmpList[0])[8]);
            rndParticle->append((tmpList[0])[9]);
            rndParticle->append((tmpList[0])[10]);
            rndParticle->append((tmpList[0])[11]);
            rndParticle->append((tmpList[0])[12]);
            injected.append(*rndParticle);
            if(i<numberOfObject_ -1)   ++rndParticle;
        }
    
        if (overlappingCheck(injected))
        {
            break;
        }
        if (count == 1000)
        {
          FatalErrorIn("void randomDistInjector::inject()")
                    << "Particles collide after 1000 tries! '" << nl
                    << exit(FatalError);
        }
        rndParticle = firstElementParticle;
        randomValue = firstElementValue;

        _DBO_("Particles collide!  try " << count++ << "\n")

        factorOfSafty += factorOfSafty * 0.5;        
    }
   
     _DBO_("Injection " << numberOfObject_ << " object with " << distributionFunction_
          << " distribution" << "\n")

     rndParticle = firstElementParticle;
     injected.clear();

     for(int i = 0; i < numberOfObject_; i++)
     {

        injected.append(*(rndParticle++));
        vector velocity = vector((injected[i])[4], (injected[i])[5], (injected[i])[6]);
        scalar velMag = mag(velocity);

        _DBO_("Object " << i+1 << ": " << (injected[i])[0] << ", " << 
              (injected[i])[1] << ", " << (injected[i])[2] << ", " << (injected[i])[3] <<
      ", " << (injected[i])[4] << ", " << (injected[i])[5] << ", " << (injected[i])[6] <<
      ", " << (injected[i])[7] << ", " << (injected[i])[8] << ", " << (injected[i])[9] <<
      ", " << (injected[i])[10] << ", " << (injected[i])[11] << ", " << (injected[i])[12] << "\n")
     }

  
    Info << endl << nameStr_ << ": " << injected.size() << " particles to inject." << endl;
}

// Check if generated object collide while injected in the fluid flow
bool randomDistInjector::overlappingCheck(List<List<scalar>>& injected)
{
    scalar distance;
    for(int i = 0; i < numberOfObject_ -1; ++i)
    {
        for(int j = i+1; j < numberOfObject_; ++j)
        {
            distance = sqrt(pow((injected[i])[1] - (injected[j])[1],2) +
                            pow((injected[i])[2] - (injected[j])[2],2) +
                            pow((injected[i])[3] - (injected[j])[3],2));
            if (distance < ((injected[i])[0] + (injected[j])[0]) * 1.1)
            {
                return false;
            }
        }
    }
    return true;
}


}   
