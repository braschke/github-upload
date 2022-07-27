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

#include "LSMIOdictionary.H"
#include "Pstream.H"

#include "makros.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Parallel aware reading, using non-virtual type information (typeName instead
// of type()) because of use in constructor.
void Foam::LSMIOdictionary::readFile(const bool masterOnly)
{
    if (debug)
    {
        Pout<< "LSMIOdictionary : Reading " << objectPath()
            << " from file " << endl;
    }

    // Set flag for e.g. codeStream
    bool oldFlag = false;//regIOobject::masterOnlyReading;
    //regIOobject::masterOnlyReading = masterOnly;

    // Read file
    readStream(typeName) >> *this;
    close();

    //regIOobject::masterOnlyReading = oldFlag;

    if (writeDictionaries && Pstream::master())
    {
        Sout<< nl
            << "--- IOdictionary " << name()
            << ' ' << objectPath() << ":" << nl;
        writeHeader(Sout);
        writeData(Sout);
        Sout<< "--- End of IOdictionary " << name() << nl << endl;
    }

}


// * * * * * * * * * * * * * * * Members Functions * * * * * * * * * * * * * //

bool Foam::LSMIOdictionary::readData(Istream& is)
{
    is >> *this;

    if (writeDictionaries && Pstream::master() && !is.bad())
    {
        Sout<< nl
            << "--- IOdictionary " << name()
            << ' ' << objectPath() << ":" << nl;
        writeHeader(Sout);
        writeData(Sout);
        Sout<< "--- End of LSMIOdictionary " << name() << nl << endl;
    }

    return !is.bad();
}


bool Foam::LSMIOdictionary::writeData(Ostream& os) const
{
    dictionary::write(os, false);
    return os.good();
}


// ************************************************************************* //
