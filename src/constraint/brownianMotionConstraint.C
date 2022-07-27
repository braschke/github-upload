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

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "brownianMotionConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"
#include "Random.H"

#include <ctime>
#include <chrono>
#include <random>

#include "mpi.h"


namespace Foam
{
defineTypeNameAndDebug(brownianMotionConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           brownianMotionConstraint,
                           dictionary
                          );

brownianMotionConstraint::brownianMotionConstraint(
                                                    const dictionary     &dict,
                                                    const objectRegistry &obr,
                                                    const myMeshSearch   *myMSPtr
                                                  ):
Constraint(dict, obr, myMSPtr),
//deltaT_(obr.time().deltaT().value()),
subcycles_( dict_.lookupOrDefault<int>("subcycles", 2) ),
constDisplacement_(dict_.lookupOrDefault<scalar>("constDisplacement", 0)),
diffFactor_(dict_.lookupOrDefault<scalar>("diffFactor", 1))
{
    cType_   = position;
    nameStr_ = "brownianMotionConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "\nBrownian motions constraint will run with %d subcycles."
            "\nMake sure it is the same as moveParticlesSubcycle further below!", subcycles_
           );
    infoStr_ = aux;
}

void brownianMotionConstraint::constrain(volumetricParticle& particle) const
{

  scalar deltaT	= obr_.time().deltaT().value();

  scalar kb		= 1.38064852e-23;
  scalar temp	= 293.12;
  scalar eta	= 1.81e-5;
  scalar pD		= 80e-9;
  scalar Cc		= 1; // Cunningham

  scalar D	= diffFactor_ * kb * temp * Cc / (3 * 3.14 * eta * pD);

  double mean 	= 0.0;
  double var 	= 2 * D * deltaT / subcycles_;
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::normal_distribution<double> distribution(mean, sqrt(var));
  std::default_random_engine generator (seed);


  if(constDisplacement_)
  {
	  scalar factor;

	  factor = distribution(generator) < 0 ? -1 : 1;
	  scalar diffX = factor * constDisplacement_ / subcycles_;

	  factor = distribution(generator) < 0 ? -1 : 1;
	  scalar diffY = factor * constDisplacement_ / subcycles_;

	  factor = distribution(generator) < 0 ? -1 : 1;
	  scalar diffZ = factor * constDisplacement_ / subcycles_;

	  particle.getDisplNext().x() += diffX;
	  particle.getDisplNext().y() += diffY;
	  particle.getDisplNext().z() += diffZ;

	  return;
  }

  scalar diffX	= distribution(generator);
  scalar diffY	= distribution(generator);
  scalar diffZ	= distribution(generator);

  if(Pstream::parRun())
  {
	  double diff[3];
	  if(Pstream::myProcNo() == 0)
	  {
		  diff[0] = diffX;
	  	  diff[1] = diffY;
	  	  diff[2] = diffZ;
	  }
	  MPI_Bcast(diff, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	  diffX = diff[0];
	  diffY = diff[1];
	  diffZ = diff[2];
  }

  // For approach with ctype_ = position
  particle.getDisplNext().x() += diffX;
  particle.getDisplNext().y() += diffY;
  particle.getDisplNext().z() += diffZ;




  /*_DBO_("BrownMotion:\n"
		  << "diffx = \t"<< diffX
		  << "\ndiffy = \t" << diffY
		  << "\ndiffz = \t" << diffZ)*/
}


} // namespace Foam

