#include "write.H"
#include "OFstream.H"
#include "makros.H"

namespace Foam
{

void  toolWriteMesh(
                 const triSurface& triSurf,
                 const fileName& path
               )
{
  {
    OFstream pointsFile(path + "/points");
    pointsFile << _HEADER_POLYMESH_POINTS_;
    pointsFile << triSurf.points();
  }
  {
    OFstream facesFile(path + "/faces");
    facesFile << _HEADER_POLYMESH_FACES_;
    facesFile << triSurf.size() << "(" << nl;
    forAll(triSurf, labelF)
    {
      facesFile << "  " << static_cast<triFace>(triSurf[labelF]) << nl;
    }
    facesFile << ")" << nl;
    }
  {
    OFstream cellsFile(path + "/cells");
    cellsFile << _HEADER_POLYMESH_CELLS_;
    cellsFile << "1" << nl << "(" << nl;
    cellsFile << "  " << triSurf.size() << nl << "  (" << nl;
    forAll(triSurf, labelF)
    {
      cellsFile << "    " << labelF << nl;
    }
    cellsFile << "  )" << nl;
    cellsFile << ")" << nl;
  }
  {
    OFstream boundaryFile(path + "/boundary");
    boundaryFile << _HEADER_POLYMESH_BOUNDARY_;
    boundaryFile << "1" << nl << "(" << nl;
    boundaryFile << "  surface" << nl << "  {" << nl;
    boundaryFile << "    type       patch;" << nl;
    boundaryFile << "    nFaces     " << triSurf.size() << ";" << nl;
    boundaryFile << "    startFace  0;" << nl;
    boundaryFile << "  }" << nl;
    boundaryFile << ")" << nl;
  }
}

} // namespace Foam
