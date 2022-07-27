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


#include "fixedAxisForceConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(fixedAxisForceConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           fixedAxisForceConstraint,
                           dictionary
                          );

fixedAxisForceConstraint::fixedAxisForceConstraint(
                                                    const dictionary     &dict,
                                                    const objectRegistry &obr,
                                                    const myMeshSearch   *myMSPtr
                                                  ):
Constraint(dict, obr, myMSPtr),
fixedAxis_( dict.lookup("fixedAxis") )
{
    cType_   = force;
    nameStr_ = "fixedAxisForceConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "fixedAxis = (%g, %g, %g)",
            fixedAxis_.x(), fixedAxis_.y(), fixedAxis_.z()
           );
    infoStr_ = aux;
_DBI_
    scalar length = mag(fixedAxis_);
_DBI_
    if(length < SMALL)
      WarningIn("fixedAxisForceConstraint::fixedAxisForceConstraint()")
                << "fixedAxis = " << fixedAxis_ << " is singular!"  << nl;
_DBI_
    // normalize axis
    fixedAxis_ /= length;
_DBI_
}

void fixedAxisForceConstraint::constrain(volumetricParticle& particle) const
{
  Vector<scalar>  curForce = particle.getTotalForce();
  Vector<scalar>  newForce = (curForce & fixedAxis_) * fixedAxis_; // project to fixedAxis


  particle.getTotalForce() = newForce;
}


} // namespace Foam

