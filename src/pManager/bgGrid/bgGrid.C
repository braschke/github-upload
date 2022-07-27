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


#include "bgGrid.H"
#include "makros.H"

#include "indexedOctree.H"


namespace Foam
{


bgGrid::bgGrid (const polyMesh &p, const scalar& granularity):
        boundBox(p.points(), true),
        mesh_(p),
        granularity_(granularity),
        nx_(0), ny_(0), nz_(0),
        belongsToProcessor_(0)//,
//        connectivity_(Pstream::nProcs())
{
    if(granularity <= 0.)
    {
      FatalErrorIn("bgGrid::bgGrid (const polyMesh &p, const scalar& granularity)")
                << "Negative granularity = " << granularity_
                << exit(FatalError);
    }
    vector ext = vector::one * granularity_ * 2; // *10
    min() -= ext;
    max() += ext;
    vector DX = span();
    nx_ = std::floor(DX.x()/granularity_) + 1;
    ny_ = std::floor(DX.y()/granularity_) + 1;
    nz_ = std::floor(DX.z()/granularity_) + 1;
    belongsToProcessor_.resize(nx_*ny_*nz_, -1);
    makeProcessorMap();
//    makeConnectivity();

    printInfo();
}

bool  bgGrid::isBoundaryCell(const label& status) const
{
  return (
            ((status & hasAllNeighbours) != hasAllNeighbours) ?
            true                                              :
            false
          );
}

void bgGrid::getIndex(const point &p, label& i, label& j, label& k) const
{
  List<label> idx(3);

  getIndex(p, idx);

  i = idx[0];
  j = idx[1];
  k = idx[2];
}

void bgGrid::getIndex(const point &p, List<label>& idx) const
{
  point  d = p - this->min();

  d /= granularity_;
  idx[0] = std::floor(d.x());
  idx[1] = std::floor(d.y());
  idx[2] = std::floor(d.z());
}

label bgGrid::getIJKDistance(const List<label>& a, const List<label>& b) const
{
  return  std::max(
                     std::max(std::abs(a[0]-b[0]), std::abs(a[1]-b[1])),
                     std::abs(a[2]-b[2])
                   );
}

label bgGrid::getIJKDistance(const point& x, const point& y) const
{
  List<label>  idx_x(3), idx_y(3);

  getIndex(x, idx_x);
  getIndex(y, idx_y);

  return getIJKDistance(idx_x, idx_y);
}

bool bgGrid::isMine(const point& p) const
{
  if( Pstream::parRun() )
  {
    return ( getProcessor(p) == Pstream::myProcNo() );
  }

  return true;
}

label bgGrid::getProcessor(const point& p) const
{
  List<label>  idx(3);

  getIndex(p, idx);

  return getProcessor(idx);
}

label bgGrid::getProcessor(const label& i, const label& j, const label& k) const
{
  if(
      0 <= i && i < nx_ &&
      0 <= j && j < ny_ &&
      0 <= k && k < nz_
    )
  {
    return belongsToProcessor_[
                                 _multiIndex_(i, j, k, nx_, ny_, nz_)
                               ];
  }
  else
  {
      WarningIn("label bgGrid::getProcessor(const List<label>& idx)")
             << "Index (i, j, k) = (" << i << ", " << j << ", " << k << ")"
             << " out of range (0-" << nx_-1 << ", 0-" << ny_-1 << ", 0-" << nz_-1
             << ")." << endl;
      return -1;
  }
}

label bgGrid::getProcessor(const List<label>& idx) const
{
  return getProcessor(idx[0], idx[1], idx[2]);
}

void bgGrid::setProcessor(const List<label>& idx, const label& p)
{
  setProcessor(idx[0], idx[1], idx[2], p);
}

void bgGrid::setProcessor(const label& i, const label& j, const label& k, const label& p)
{
  if(
      0 <= i && i < nx_ &&
      0 <= j && j < ny_ &&
      0 <= k && k < nz_
    )
  {
    belongsToProcessor_[
                         _multiIndex_(i, j, k, nx_, ny_, nz_)
                       ] = p;
  }
  else
  {
      WarningIn("void bgGrid::setProcessor(const label& i, const label& j, const label& k, const label& p)")
             << "Index (i, j, k) = (" << i << ", " << j << ", " << k << ")"
             << " out of range (0-" << nx_-1 << ", 0-" << ny_-1 << ", 0-" << nz_-1
             << ")." << endl;
  }
}


point bgGrid::getMidpoint(const label& i, const label& j, const label& k) const
{
  point mid = min() + vector(
                              (i + 0.5)*granularity_,
                              (j + 0.5)*granularity_,
                              (k + 0.5)*granularity_
                            );
  return mid;
}

point bgGrid::getMidpoint(const List<label>& idx) const
{
  return getMidpoint(idx[0], idx[1], idx[2]);
}

label bgGrid::getIndexStatus(const List<label>& idx) const
{
  label i = idx[0];
  label j = idx[1];
  label k = idx[2];

  label status = 0;

  status |= ( i < nx_-1 ) ? hasXNeighbour : 0;
  status |= ( i >=    1 ) ? hasxNeighbour : 0;
  status |= ( j < ny_-1 ) ? hasYNeighbour : 0;
  status |= ( j >=    1 ) ? hasyNeighbour : 0;
  status |= ( k < nz_-1 ) ? hasZNeighbour : 0;
  status |= ( k >=    1 ) ? haszNeighbour : 0;

  return status;
}

bool bgGrid::isIndexValid(const List<label>& idx) const
{
  label i = idx[0];
  label j = idx[1];
  label k = idx[2];

  if(
      i >= 0 && i < nx_ &&
      j >= 0 && j < ny_ &&
      k >= 0 && k < nz_
    )
    return true;


  return false;
}

label bgGrid::getAllNeighbourProcessors(const List<label>& idx, List<label>& procList, label radius) const
{
  label line = 2*radius + 1;
  procList.resize(line*line*line);  // (line^3 -1) potential neighbouring processes
  procList = -1;
  label count = 0;

  List<label> aux(3);
  for(label ix = -radius; ix <= radius; ++ix)
    for(label iy = -radius; iy <= radius; ++iy)
      for(label iz = -radius; iz <= radius; ++iz)
      {
        aux = idx;
        aux[0] += ix;
        aux[1] += iy;
        aux[2] += iz;

        if( !isIndexValid(aux) )
          continue;

        label proc = getProcessor(aux);

        if( proc != -1 )
            procList[count++] = proc;
      }
  // the first count entries of procList contain neighbouring processes
  // entries may be duplicate
  return count;
}

label bgGrid::getAllNeighbourProcessors(
                                             const label& i,
                                             const label& j,
                                             const label& k,
                                             List<label>& procList,
                                             label radius
                                            ) const
{
  List<label> idx(3);

  idx[0] = i;
  idx[1] = j;
  idx[2] = k;

  return getAllNeighbourProcessors(idx, procList, radius);
}

label bgGrid::getAllNeighbourProcessorsCompact(const List<label>& idx, List<label>& procList, label radius) const
{
  label line = 2*radius + 1;
  List<label> fullList(line*line*line);
  List<bool> aux(Pstream::nProcs(), false);

  label countFull = getAllNeighbourProcessors(idx, fullList, radius);

  // Mark occurencies
  for(label i = 0; i < countFull; ++i)
   aux[fullList[i]] = true;

  // count real number of neighbours
  label countCompact = 0;
  for(label i = 0; i < Pstream::nProcs(); ++i)
    if( aux[i] ) countCompact++;

  procList.resize(countCompact);

  label count = 0;
  for(label i = 0; i < Pstream::nProcs(); ++i)
    if( aux[i] ) procList[count++] = i;

  return count;
}

/*
void bgGrid::makeConnectivity()
{
  List<List<bool> > conn(Pstream::nProcs()); // aux. connectivity list

  for(label i = 0; i< Pstream::nProcs(); ++i)
    conn[i].resize(Pstream::nProcs(), false);

  // Fill connectivity matrix
  for(label ix = 0; ix < nx_; ++ix)
    for(label iy = 0; iy < ny_; ++iy)
      for(label iz = 0; iz < nz_; ++iz)
      {
        label proc = getProcessor(ix, iy, iz);

        if( proc == -1 )
          continue;

        List<label> neighboursOfCell(27);
        label nNeighbourProcs;
        nNeighbourProcs = getAllNeighbourProcessors(ix, iy, iz, neighboursOfCell, 1);
        for(label i = 0; i < nNeighbourProcs; ++i)
        {
          conn[proc][neighboursOfCell[i]] = true;
        }
      }
  // Construct connectivity lists
  for(label i = 0; i < Pstream::nProcs(); ++i)
  {
    label count = 0;
    List<bool>& connLine = conn[i];

    // count number of neighbours
    count = 0;
    for(label j = 0; j < Pstream::nProcs(); ++j)
      if(connLine[j])  count++;

    // processor i has count neighbouring processes
    connectivity_[i].resize(count);

    count = 0;
    for(label j = 0; j < Pstream::nProcs(); ++j)
      if(connLine[j]) connectivity_[i][count++] = j;
  }
}
*/

bool bgGrid::nearDomainBoarder(const point& p, label dist) const
{
  List<label> idx(3);
  getIndex(p, idx);

  return nearDomainBoarder(idx, dist);
}

bool bgGrid::nearDomainBoarder(const List<label>& idx, label dist) const
{
  label status = getIndexStatus(idx);

  // If bg cell is a boundary cell, then it is definitely NEAR a wall
  if( isBoundaryCell(status) )
    return true;

  List<label> aux(3);
  // If any of the neighbouring bg cells lies outside the fluid domain
  // assume vicinity of a wall
  for(label ix = -dist; ix <= dist; ++ix)
    for(label iy = -dist; iy <= dist; ++iy)
      for(label iz = -dist; iz <= dist; ++iz)
      {
        aux = idx;
        aux[0] += ix;
        aux[1] += iy;
        aux[2] += iz;

        if( !isIndexValid(aux) )
          continue;

        if(getProcessor(aux) == -1)
          return true;
      }

  return false;
}

bool bgGrid::isNeighbourOfProc(const List<label>& idx, const label& procNo, label radius) const
{
  List<label> aux(3);
  // If one of the neighbouring bg cells belongs to processor procNo, we're
  // a neighbour of processor procNo
  for(label ix = -radius; ix <= radius; ++ix)
    for(label iy = -radius; iy <= radius; ++iy)
      for(label iz = -radius; iz <= radius; ++iz)
      {
        aux = idx;
        aux[0] += ix;
        aux[1] += iy;
        aux[2] += iz;

        if( !isIndexValid(aux) )
          continue;

        if( getProcessor(aux) == procNo )
          return true;
      }

  return false;
}

bool bgGrid::isNeighbourOfProc(const point& p, const label& procNo, label radius) const
{
  List<label> idx(3);
  getIndex(p, idx);

  return isNeighbourOfProc(idx, procNo, radius);
}

bool bgGrid::nearProcessorBoarder(const List<label>& idx, const label& procNo, label radius) const
{
  List<label> aux(3);
  // If one of the neighbouring bg cells belongs to another (other than procNo) processor, we're
  // near the boarder of processor procNo
  for(label ix = -radius; ix <= radius; ++ix)
    for(label iy = -radius; iy <= radius; ++iy)
      for(label iz = -radius; iz <= radius; ++iz)
      {
        aux = idx;
        aux[0] += ix;
        aux[1] += iy;
        aux[2] += iz;

        if( !isIndexValid(aux) )
          continue;

        label auxProc = getProcessor(aux);

        if( auxProc != procNo && auxProc != -1 )
          return true;
      }

  return false;
}

bool bgGrid::nearProcessorBoarder(const point& p, const label& procNo, label radius) const
{
  List<label> idx(3);
  getIndex(p, idx);

  return nearProcessorBoarder(idx, procNo, radius);
}

void bgGrid::makeProcessorMap()
{
  for(label i = 0; i < nx_; ++i)
  {
    for(label j = 0; j < ny_; ++j)
    {
      for(label k = 0; k < nz_; ++k)
      {
        point mid = getMidpoint(i, j, k);

        if( mesh_.findCell(mid) != -1 )
        {
          setProcessor(i, j, k, Pstream::myProcNo());
        }
      }
    }
  }
  Pstream::listCombineGather(belongsToProcessor_, maxEqOp<label>());
  Pstream::listCombineScatter(belongsToProcessor_);
}


} // namespace Foam
