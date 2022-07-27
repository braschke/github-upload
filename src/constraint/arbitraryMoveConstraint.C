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


#include "arbitraryMoveConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"


namespace Foam
{
defineTypeNameAndDebug(arbitraryMoveConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           arbitraryMoveConstraint,
                           dictionary
                          );

arbitraryMoveConstraint::arbitraryMoveConstraint(
                                      const dictionary     &dict,
                                      const objectRegistry &obr,
                                      const myMeshSearch   *myMSPtr
                                    ):
Constraint(dict, obr, myMSPtr),
objVelocity_        ( vector::zero                                   ),
objOmega_           ( vector::zero                                   ),
moveFileName_       ( dict.lookup("moveFile")                        ),
moveUpdateInterval_ ( readScalar(dict.lookup("moveUpdateInterval"))  ),
nextModelUpdate_    ( obr.time().value() + moveUpdateInterval_       ),
moveUpdateCommand_  ( dict.lookup("moveUpdateCommand")               )
{
    cType_   = acceleration;
    nameStr_ = "arbitraryMoveConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "moveFile           = \'%s\'\n",
            "moveUpdateCommand  = \'%s\'\n"
            "moveUpdateInterval = \'%g\'",
            moveFileName_.c_str(),
            moveUpdateCommand_.c_str(),
            moveUpdateInterval_
           );

    infoStr_ = aux;
}

void arbitraryMoveConstraint::constrain(volumetricParticle& particle) const
{
  char  command[1024]; command[1023] = '\0';

  sprintf(
           command,
           "%s %g",
           moveUpdateCommand_.c_str(),
           obr_.time().value()
         );

  IFstream moveFile(moveFileName_);

  objVelocity_.x()     = readScalar(moveFile);
  objVelocity_.y()     = readScalar(moveFile);
  objVelocity_.z()     = readScalar(moveFile);
  objOmega_.x()        = readScalar(moveFile);
  objOmega_.y()        = readScalar(moveFile);
  objOmega_.z()        = readScalar(moveFile);

  scalar time = obr_.time().value();

  Info << nl <<
       objVelocity_.x() << " " << objVelocity_.y() << " " << objVelocity_.z() << nl <<
       objOmega_.x() << " " << objOmega_.y() << " " << objOmega_.z() << nl << endl;

// Update values

  if( moveUpdateInterval_ >= 0 && time >= nextModelUpdate_ )
  {
    nextModelUpdate_ += moveUpdateInterval_;

    Info << nl <<
         "========================================" << nl <<
         "arbitraryMoveConstraint: velocity update" << nl << endl;

    Info << nl << command << nl << endl;

    system( command );

    Info << nl <<
         "Done!" << nl <<
         "========================================" << endl;
  }

  particle.getAcceleration() =
        ( objVelocity_ - particle.getVelocity() ) / obr_.time().deltaT().value();

  particle.getOmegaAcc() =
        ( objOmega_ - particle.getOmega() ) / obr_.time().deltaT().value();
}


} // namespace Foam

