/*---------------------------------------------------------------------------*\
.      _________________________________________________________
.     /                                                        /|
.    /                                                        / |
.   |--------------------------------------------------------|  |
.   |        _    ____ ____  _____                           |  |
.   |       / \  | __ ) ___||  ___|__   __ _ _ __ ___        |  |
.   |      / _ \ |  _ \___ \| |_ / _ \ / _` | '_ ` _ \       |  |
.   |     / ___ \| |_) |__) |  _| (_) | (_| | | | | | |      |  |
.   |    /_/   \_\____/____/|_|  \___/ \__,_|_| |_| |_|      |  |
.   |                                                        |  |
.   |    Arbitrary  Body  Simulation    for    OpenFOAM      | /
.   |________________________________________________________|/
.
-------------------------------------------------------------------------------

Author

    Markus Buerger
    Chair of Fluid Mechanics
    markus.buerger@uni-wuppertal.de

    $Date: 2016-10-11 13:35:48 +0200 (Di, 11 Okt 2016) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef MAKROS_H
#define MAKROS_H

#include <iomanip>

#ifndef _OCTBOX_SUPPORT_
  #define _OCTBOX_SUPPORT_    0
#endif

// Number of scalar values (size, position, velocity, ...) describing particle
#define _N_PARTICLE_PARAMETERS_ 14

#define _MAX_N_PROCESSES_      256
#define _N_DIGITS_PROCNUM_     4
#define _N_DIGITS_POPNUM_      4
#define _N_DIGITS_PARTICLENUM_ 4
#define _N_DIGITS_PARTICLE_ID_ (_N_DIGITS_PROCNUM_ + _N_DIGITS_PROCNUM_ + _N_DIGITS_PROCNUM_ + 23)

#define _LSM_RESET   "\033[0m"
#define _LSM_BLACK   "\033[30m"      /* Black */
#define _LSM_RED     "\033[31m"      /* Red */
#define _LSM_GREEN   "\033[32m"      /* Green */
#define _LSM_YELLOW  "\033[33m"      /* Yellow */
#define _LSM_BLUE    "\033[34m"      /* Blue */
#define _LSM_MAGENTA "\033[35m"      /* Magenta */
#define _LSM_CYAN    "\033[36m"      /* Cyan */
#define _LSM_WHITE   "\033[37m"      /* White */
#define _LSM_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define _LSM_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define _LSM_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define _LSM_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define _LSM_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define _LSM_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define _LSM_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define _LSM_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define _ITOS_(i__, w__)                                                       \
            dynamic_cast< std::ostringstream & >                             \
               (                                                               \
                 (                                                             \
                   std::ostringstream() <<                                     \
                           std::dec << std::setw(w__) << std::setfill('0') <<  \
                           i__                                                 \
                 )                                                             \
               ).str()                                                         \



#define InfoLog(__s__) if(log_) Info << __s__ << endl;

#define _DBI_MESSAGE  _LSM_RED                                                 \
                << endl << "DEBUG-code in line " << __LINE__                   \
                << " of file \'" << __FILE__ << "\'." << endl << endl          \
                << _LSM_RESET ;

#define _DBO_MESSAGE(msg__)  _LSM_RED                                          \
                    << endl << "DEBUG-output: " << _LSM_GREEN                  \
                    << msg__ << endl << _LSM_RED                               \
                    << "line " << __LINE__ <<                                  \
                    " of file \'" << __FILE__ << "\'."                         \
                    << endl << endl                                            \
                    << _LSM_RESET ;

#define _DBI_   Info << _DBI_MESSAGE
#define _PDBI_  Pout << _DBI_MESSAGE

#define _DBO_(msg__)   Info << _DBO_MESSAGE(msg__)
#define _PDBO_(msg__)  Pout << _DBO_MESSAGE(msg__)
#define _PDBOP_(msg__, i__) if(Pstream::myProcNo()==i__) _PDBO_(msg__) else if(!Pstream::parRun()) _DBO_(msg__)


#define _LSMMAG_(x_, y_, z_) sqrt(x_*x_ + y_*y_ + z_*z_)
#define _LSMMAGV_(vec) _LSMMAG_(vec[0], vec[1], vec[2])
#define _LSMINNER_(x1_, x2_, x3_, y1_, y2_, y3_) (x1_*y1_ + x2_*y2_ + x3_*y3_)

#define _LSMSQRTOFTHREE std::sqrt(3)


#define _WRITE_DIMENSIONS_FORCE_ \
"dimensions  [1 1 -2 0 0 0 0];" << nl << nl

#define _WRITE_DIMENSIONS_FORCEDENSITY_ \
"dimensions  [1 -1 -2 0 0 0 0];" << nl << nl

#define _WRITE_FIELD_PREAMBLE_ \
"internalField uniform " << total << ";" << nl << nl << \
"boundaryField" << nl << "{" << nl <<                   \
"  surface" << nl << "  {" << nl <<                     \
"    type   fixedValue;" << nl <<                       \
"    value  nonuniform List<vector>" << nl;

#define _WRITE_FIELD_POSTAMBLE_ \
";" << nl <<   \
"  }" << nl << \
"}" << endl;

#define _HEADER_POLYMESH_POINTS_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       vectorField;           \n" \
"    location    \"constant/polyMesh\"; \n" \
"    object      points;                \n" \
"}                                      \n\n" \

#define _HEADER_POLYMESH_FACES_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       faceList;              \n" \
"    location    \"constant/polyMesh\"; \n" \
"    object      faces ;                \n" \
"}                                      \n\n" \

#define _HEADER_POLYMESH_CELLS_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       cellList;              \n" \
"    location    \"constant/polyMesh\"; \n" \
"    object      cells ;                \n" \
"}                                      \n\n" \

#define _HEADER_POLYMESH_BOUNDARY_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       polyBoundaryMesh;      \n" \
"    location    \"constant/polyMesh\"; \n" \
"    object      boundary;              \n" \
"}                                      \n\n" \

#define _HEADER_FLUIDFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      fluidForce;            \n" \
"}                                      \n\n" \

#define _HEADER_FLUIDPRESSUREFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      fluidPressureForce;    \n" \
"}                                      \n\n" \

#define _HEADER_FLUIDPRESSUREFORCE_DENSITY_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      fluidPressureForceDensity;    \n" \
"}                                      \n\n" \

#define _HEADER_FLUIDSTRESSFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      fluidStressForce;      \n" \
"}                                      \n\n" \

#define _HEADER_FLUIDSTRESSFORCE_DENSITY_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      fluidStressForceDensity;      \n" \
"}                                      \n\n" \

#define _HEADER_SOLIDFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      solidForce;            \n" \
"}                                      \n\n" \

#define _HEADER_THERMOFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      thermoForce;           \n" \
"}                                      \n\n" \

#define _HEADER_ELECTROMAGFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      electromagForce;       \n" \
"}										\n\n" \

#define _HEADER_CONTACTFORCE_FIELD_ \
"FoamFile                               \n" \
"{                                      \n" \
"    version     2.2;                   \n" \
"    format      ascii;                 \n" \
"    class       volVectorField;        \n" \
"    object      contactForce;          \n" \
"}										\n\n" \

#endif // MAKROS_H
