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

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/
#ifndef integration_H
#define integration_H

#include "fvPatchFields.H"
#include "pointField.H"

#include  "potential.H"
#include  "contact.H"
#include  "contactModel.H"


namespace Foam
{

        void surfaceIntegration(
                                   vectorField    *f1,
                                   const pointField      &faceCenters1,
                                   const vectorField     &faceVectors1,
                                   const pointField      &faceCenters2,
                                   const vectorField     &faceVectors2,
                                   const Potential       *pot,
                                   const contactModel    *cM,
                                   const word&            pIdStr1,
                                   const word&            pIdStr2,
                                   label                  popId1,
                                   label                  popId2,
                                   contactHash_t         *hash = 0,
                                   scalar                 contactDistFactor = 1,
                                   scalar                 scale = 1.0
                               );

} // Foam

#endif // integration_H
