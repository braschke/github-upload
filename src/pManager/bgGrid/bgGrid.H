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


#ifndef bgGrid_H
#define bgGrid_H

#include "boundBox.H"
#include "polyMesh.H"
#include "treeDataCell.H"



#define _multiIndex_(i__, j__, k__, iDim__, jDim__, kDim__) \
         (k__ * iDim__ * jDim__   +   j__ * iDim__   +   i__)

namespace Foam
{

class bgGrid
      : public boundBox
{
public:

  enum    {
             hasXNeighbour    =  1,
             hasxNeighbour    =  2,
             hasYNeighbour    =  4,
             hasyNeighbour    =  8,
             hasZNeighbour    = 16,
             haszNeighbour    = 32,
             hasAllNeighbours = 63
          };

             bgGrid (const polyMesh &, const scalar&);

             void printInfo()
             {
               Info << endl <<
                "BackgroundGrid information:" << endl <<
                "\tbound box: min = " << min() << ", max = " << max() << endl <<
                "\tgranularity: " << granularity_ << endl <<
                "\tgrid cells: (" << nx_ << " " << ny_ << " " << nz_ << ")" <<
                endl << endl;
             }


             label getIJKDistance(const List<label>&, const List<label>&) const;
             label getIJKDistance(const point&, const point&) const;

             bool nearDomainBoarder(const point&, label) const;
             bool nearDomainBoarder(const List<label>& idx, label) const;
             bool isNeighbourOfProc(const List<label>& idx, const label& procNo, label) const;
             bool isNeighbourOfProc(const point& p, const label& procNo, label) const;
             bool nearProcessorBoarder(const point&, const label& procNo, label) const;
             bool nearProcessorBoarder(const List<label>& idx, const label& procNo, label) const;

             bool  isMine(const point& p) const;
             label getProcessor(const point&) const;
             label getProcessor(const List<label>&) const;
             label getProcessor(const label&, const label&, const label&) const;
             void  getIndex(const point &, label&, label&, label&) const;
             void  getIndex(const point &, List<label>&) const;
             label getAllNeighbourProcessorsCompact(const List<label>&, List<label>&, label) const;
             label getAllNeighbourProcessors(const List<label>&, List<label>&, label) const;
             label getAllNeighbourProcessors(const label&, const label&, const label&, List<label>&, label) const;

protected:

            void  makeProcessorMap();
  inline    bool  isBoundaryCell(const label& status) const;
  inline    void  setProcessor(const List<label>&, const label&);
  inline    void  setProcessor(const label&, const label&, const label&, const label&);
  inline    point getMidpoint(const label&, const label&, const label&) const;
  inline    point getMidpoint(const List<label>&) const;
  inline    label getIndexStatus(const List<label>&) const;
  inline    bool  isIndexValid(const List<label>&) const;
//  inline    void  makeConnectivity();

    const polyMesh&           mesh_;
           scalar             granularity_;
           label              nx_, ny_, nz_;
           List<label>        belongsToProcessor_;
//           List<List<label> > connectivity_;

};

} // namespace Foam

#endif  // headerguard
