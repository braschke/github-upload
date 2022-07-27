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

    $Date: 2017-07-24 10:47:46 +0200 (Mo, 24 Jul 2017) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef myMeshSearch_H
#define myMeshSearch_H

#include "objectRegistry.H"
#include "point.H"
#include "volMesh.H"
#include "fvMesh.H"
#include "meshSearch.H" // OF meshSearch
#include "regionFunctionObject.H"

#include "makros.H"

//#if _OCTBOX_SUPPORT_
#if _OCTBOX_SUPPORT_

  #define DOUBLE_PRECISION 1
  #define INCLUDE_STATIC 0

    #include "octbox.h"

  #undef DOUBLE_PRECISION
  #undef INCLUDE_STATIC

#endif



namespace Foam
{

class myMeshSearch
{
public:
             myMeshSearch(
                           const objectRegistry& obr,
                           const dictionary& dict //  parent dictionary, e.g. from funcObj
                         );

            ~myMeshSearch();

             void printInfo() const
             {
//               Info << endl <<
//                "myMeshSearch information:" << endl <<
//                "\tbound box: min = " << min() << ", max = " << max() << endl <<
//                "\tgranularity: " << granularity_ << endl <<
//                "\tgrid cells: (" << nx_ << " " << ny_ << " " << nz_ << ")" <<
//                endl << endl;
             }

void         setup();

label        findCell(const point& p) const;

enum method {
              OF, octbox, hash
            };

private:
        const objectRegistry &obr_;

        const dictionary     *dict_;

        const fvMesh         &mesh_;

        vector		bbMax_;
        vector		bbMin_;

              method          searchMethod_;

              polyMesh::cellDecomposition  cellDecomp_;

              List<List<List<label>>> hashMap_;
              double	deltaX_, deltaY_, deltaZ_;

        //- Data bounding box
        mutable autoPtr<treeBoundBox> overallBbPtr_;

        //- Demand driven octrees
        mutable autoPtr<indexedOctree<treeDataFace>> boundaryTreePtr_;
        mutable autoPtr<indexedOctree<treeDataCell>> cellTreePtr_;

        //- Pointer to hashMap
        mutable List<List<List<label*>*>*>* hashMapPtr_;
        mutable double countX_, countY_, countZ_;

#if _OCTBOX_SUPPORT_
              OCTBOX          octbox_;
              scalar          minDist_;
#endif

              const indexedOctree<Foam::treeDataCell>&
              cellTree() const;

              const List<List<List<label*>*>*>* hashMap() const;

              void  clearOut();


};

} // namespace Foam

#endif  // headerguard
