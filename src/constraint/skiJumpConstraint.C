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


#include "skiJumpConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(skiJumpConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           skiJumpConstraint,
                           dictionary
                          );

skiJumpConstraint::skiJumpConstraint(
                                      const dictionary     &dict,
                                      const objectRegistry &obr,
                                      const myMeshSearch   *myMSPtr
                                    ):
Constraint(dict, obr, myMSPtr),
skiVelocity_        ( vector::zero                                   ),
modelUpdateInterval_( readScalar(dict.lookup("modelUpdateInterval")) ),
nextModelUpdate_    ( obr.time().value() + modelUpdateInterval_      ),
veloFileName_       ( dict.lookup("veloFile")                        ),
veloUpdateCommand_  ( dict.lookup("veloUpdateCommand")               ),
modelUpdateCommand_ ( dict.lookup("modelUpdateCommand")              )
{
    cType_   = acceleration;
    nameStr_ = "skiJumpConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "\nveloFile           = \'%s\'\n"
            "veloUpdateCommand    = \'%s\'\n"
            "modelUpdateCommand   = \'%s\'\n"
            "modelUpdateInterval  = \'%g\'",
            veloFileName_.c_str(),
            veloUpdateCommand_.c_str(),
            modelUpdateCommand_.c_str(),
            modelUpdateInterval_
           );

    infoStr_ = aux;
}

void skiJumpConstraint::constrain(volumetricParticle& particle) const
{
  char  command[1024]; command[1023] = '\0';

  sprintf(
           command,
           "%s %g",
           veloUpdateCommand_.c_str(),
           obr_.time().value()
         );

  Info << nl <<
       "========================================" << nl <<
       "skiJumpConstraint: velocity update" << nl << endl;

  system( command );

  Info << nl <<
       "Done!" << nl <<
       "========================================" << endl;


  IFstream veloFile(veloFileName_);

  skiVelocity_.x()     = readScalar(veloFile);
  skiVelocity_.y()     = readScalar(veloFile);
  skiVelocity_.z()     = readScalar(veloFile);

  scalar time = obr_.time().value();

  // At this moment the total loads are calculated
  // The following topology change will take effect
  // not till next time step
  if( modelUpdateInterval_ >= 0 && time >= nextModelUpdate_ )
  {
    nextModelUpdate_ += modelUpdateInterval_;

    Info << nl <<
         "========================================" << nl <<
         "skiJumpConstraint: updating model" << nl << endl;

    // Only master
    if( Pstream::master() )
      system(modelUpdateCommand_);

    // All processes have to wait until new model is available
    // Do fake communication for synchronization
    int dummy = 0;
    Pstream::scatter(dummy);

    // Now reload stl-geometry
    particle.processSkiJump();

    Info << nl <<
         "Done!" << nl <<
         "========================================" << endl;

  }

  particle.getAcceleration() =
        ( skiVelocity_ - particle.getVelocity() ) / obr_.time().deltaT().value();

  particle.getOmegaAcc() = vector::zero;
}


} // namespace Foam

