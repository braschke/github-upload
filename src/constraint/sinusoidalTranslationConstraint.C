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

    $Date: 2018-09-24 14:05:00 +0200 (Mon, 24 Sept 2018) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "sinusoidalTranslationConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "makros.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(sinusoidalTranslationConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
						   sinusoidalTranslationConstraint,
                           dictionary
                          );

sinusoidalTranslationConstraint::sinusoidalTranslationConstraint(
                                                    const dictionary     &dict,
                                                    const objectRegistry &obr,
                                                    const myMeshSearch   *myMSPtr
                                                  ):
Constraint(dict, obr, myMSPtr),
//staticVelocity_( dict.lookup("staticVelocity") ),
//period_( dict.lookupOrDefault<scalar>("period", 0.0) )
amplitude_( dict.lookup("amplitude") ),
frequency_( dict.lookupOrDefault<scalar>("frequency", 0.0) )
{
    cType_   = acceleration;
    nameStr_ = "sinusoidalTranslationConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "amplitude = (%g, %g, %g)",
            amplitude_.x(), amplitude_.y(), amplitude_.z()
           );
    infoStr_ = aux;
}

void sinusoidalTranslationConstraint::constrain(volumetricParticle& particle) const
{
	scalar t = obr_.time().value();
//	int sig = sign(sin(M_PI * t / period_)) ;
	vector sinVelocity = 2*M_PI*frequency_*amplitude_*cos(2*M_PI*frequency_*t);

  particle.getAcceleration() =
         ( sinVelocity - particle.getVelocity() ) / obr_.time().deltaT().value();

  _PDBO_("time = " << t << "\tsinVelocity = " << sinVelocity << "\t")
}


} // namespace Foam

