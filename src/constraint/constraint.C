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

#include "constraint.H"
#include "volumetricParticle.H"

namespace Foam
{
defineTypeNameAndDebug(Constraint, 0);
defineRunTimeSelectionTable(Constraint, dictionary);


autoPtr<Constraint> Constraint::New(
                                     const dictionary     &dict,
                                     const objectRegistry &obr,
                                     const myMeshSearch   *myMSPtr
                                   )
{

  Istream& nameStream = dict.lookup("type");
  const word constraintName( nameStream );

  dictionaryConstructorTable::iterator cstrIter =
      dictionaryConstructorTablePtr_->find(constraintName);

  if (cstrIter == dictionaryConstructorTablePtr_->end())
  {
      FatalErrorIn
      (
          "Constraint::New(const dictionary &dict)"
      )   << "Unknown constraint " << constraintName << nl << nl
          << "Valid constraint names are :" << endl
          << dictionaryConstructorTablePtr_->sortedToc()
          << exit(FatalIOError);
  }

  return cstrIter()(dict, obr, myMSPtr);
}

bool Constraint::modifiesForce() const
{
  return (cType_ == force);
}

bool Constraint::modifiesAcceleration() const
{
  return (cType_ == acceleration);
}

bool Constraint::modifiesVelocity() const
{
  return (cType_ == velocity);
}

bool Constraint::modifiesPosition() const
{
  return (cType_ == position);
}



Constraint::Constraint(
                        const dictionary     &dict,
                        const objectRegistry &obr,
                        const myMeshSearch   *myMSPtr
                      ):
  nameStr_("Constraint base class not implemented!"),
  infoStr_(),
  cType_(none),
  dict_(dict),
  obr_(obr),
  myMSPtr_(myMSPtr)
{

}


} // namespace Foam

