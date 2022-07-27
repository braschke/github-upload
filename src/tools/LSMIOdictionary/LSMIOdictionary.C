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
#include "objectRegistry.H"
#include "Pstream.H"

#include "makros.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
defineTypeNameAndDebug(LSMIOdictionary, 0);

bool LSMIOdictionary::writeDictionaries
(
    debug::infoSwitch("writeDictionaries", 0)
);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::LSMIOdictionary::LSMIOdictionary(const IOobject& io)
:
    regIOobject(io)
{
    // Temporary warning
    if (debug && io.readOpt() == IOobject::MUST_READ)
    {
        WarningIn("LSMIOdictionary::LSMIOdictionary(const IOobject&)")
            << "Dictionary " << name()
            << " constructed with IOobject::MUST_READ"
            " instead of IOobject::MUST_READ_IF_MODIFIED." << nl
            << "Use MUST_READ_IF_MODIFIED if you need automatic rereading."
            << endl;
    }

    // Everyone check or just master
    bool masterOnly =
        regIOobject::fileModificationChecking == timeStampMaster
     || regIOobject::fileModificationChecking == inotifyMaster;

    // Check if header is ok for READ_IF_PRESENT
    bool isHeaderOk = false;
    if (io.readOpt() == IOobject::READ_IF_PRESENT)
    {
      isHeaderOk = headerOk();
    }


    if
    (
        (
            io.readOpt() == IOobject::MUST_READ
         || io.readOpt() == IOobject::MUST_READ_IF_MODIFIED
        )
     || isHeaderOk
    )
    {
        readFile(masterOnly);
    }

    dictionary::name() = IOobject::objectPath();
}


Foam::LSMIOdictionary::LSMIOdictionary(const IOobject& io, const dictionary& dict)
:
    regIOobject(io)
{
    // Temporary warning
    if (debug && io.readOpt() == IOobject::MUST_READ)
    {
        WarningIn
        (
            "LSMIOdictionary::LSMIOdictionary(const IOobject& const dictionary&)"
        )   << "Dictionary " << name()
            << " constructed with IOobject::MUST_READ"
            " instead of IOobject::MUST_READ_IF_MODIFIED." << nl
            << "Use MUST_READ_IF_MODIFIED if you need automatic rereading."
            << endl;
    }

    // Everyone check or just master
    bool masterOnly =
        regIOobject::fileModificationChecking == timeStampMaster
     || regIOobject::fileModificationChecking == inotifyMaster;


    // Check if header is ok for READ_IF_PRESENT
    bool isHeaderOk = false;
    if (io.readOpt() == IOobject::READ_IF_PRESENT)
    {
        if (masterOnly)
        {
            if (Pstream::master())
            {
                isHeaderOk = headerOk();
            }
            Pstream::scatter(isHeaderOk);
        }
        else
        {
            isHeaderOk = headerOk();
        }
    }


    if
    (
        (
            io.readOpt() == IOobject::MUST_READ
         || io.readOpt() == IOobject::MUST_READ_IF_MODIFIED
        )
     || isHeaderOk
    )
    {
        readFile(masterOnly);
    }
    else
    {
        dictionary::operator=(dict);
    }

    dictionary::name() = IOobject::objectPath();
}


Foam::LSMIOdictionary::LSMIOdictionary(const IOobject& io, Istream& is)
:
    regIOobject(io)
{
    dictionary::name() = IOobject::objectPath();
    // Note that we do construct the dictionary null and read in afterwards
    // so that if there is some fancy massaging due to a functionEntry in
    // the dictionary at least the type information is already complete.
    is  >> *this;
}


// * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * * //

Foam::LSMIOdictionary::~LSMIOdictionary()
{}


// * * * * * * * * * * * * * * * Members Functions * * * * * * * * * * * * * //

const Foam::word& Foam::LSMIOdictionary::name() const
{
    return regIOobject::name();
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void Foam::LSMIOdictionary::operator=(const LSMIOdictionary& rhs)
{
    dictionary::operator=(rhs);
}


// ************************************************************************* //
