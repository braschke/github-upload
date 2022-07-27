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

#include "integration.H"

#include "makros.H"



namespace Foam
{

void surfaceIntegration(
                         vectorField     *f1,
                         const pointField       &faceCenters1,
                         const vectorField      &faceVectors1,
                         const pointField       &faceCenters2,
                         const vectorField      &faceVectors2,
                         const Potential        *pot,
                         const contactModel     *cM,
                         const word&             pIdStr1,
                         const word&             pIdStr2,
                         label                   popId1,
                         label                   popId2,
                         contactHash_t          *hash,
                         scalar                  contactDistFactor,
                         scalar                  scale
                       )
{
  scalar facSqr = contactDistFactor*contactDistFactor;

  cout << "Integrating potential for " << pIdStr1 << "\n";
  forAll(faceCenters1, face1I)
  {
    vector dS1 = faceVectors1[face1I];
    scalar  S1 = mag(dS1);
    dS1 /= S1; // surface normal


    forAll(faceCenters2, face2I)
    {
      vector   dx   = (faceCenters2[face2I] - faceCenters1[face1I]);
      scalar   s   = mag(dx);
      if (s > 2e-8) continue; //TODO Set maximum distance in dict
      scalar dxdS1 = (dx & dS1);

      if( pot )
      {
        vector dS2   = faceVectors2[face2I];
        scalar  S2   = mag(dS2);
        dS2 /= S2; // surface normal

        s = (s < SMALL) ? SMALL : s;

        //vector fPart = S1*S2 * pot->v(s) * dxdS1 * dS2;
        // kurze Anpassung für mein vdW-Potential - später ändern
        // TODO KAMIL
        vector fPart = pot->v(s) * S1 * dS2;
        //_PDBO_("fPart = " << fPart << "\tdist = " << dist << "\nface1 = " << face1I << "\tface2 = " << face2I )

        //fPart *= scale;

        f1->operator[](face1I)  += fPart;
        //f1->operator[](face1I)  = vector(face1I, face1I, face1I);
      }

      if( cM )
      {
        // Face pair is contact candidate
        // if distance is smaller than face size times factor;
        // face size := sqrt(area)
        // |s| <= contactDistFactor * sqrt(area)
        // <=>  s*s <= contactDistFactor^2 * area
        if(s*s <= facSqr*S1 && hash)
        {
          hash->insert(contactState(facePair(face1I, face2I, popId1, popId2, pIdStr1, pIdStr2)));
        }
      }

    } // forAll faceCenters2
  }  // forAll faceCenters1
  cout << "\n"<< endl;

}


}  // Foam
