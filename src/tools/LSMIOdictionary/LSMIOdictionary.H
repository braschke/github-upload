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

#ifndef LSMIOdictionary_H
#define LSMIOdictionary_H

#include "dictionary.H"
#include "regIOobject.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class IOdictionary Declaration
\*---------------------------------------------------------------------------*/

class LSMIOdictionary
:
    public regIOobject,
    public dictionary
{
    // Private data

        static bool writeDictionaries;


    // Private Member Functions

        //- Read dictionary from file
        void readFile(const bool);


public:

    TypeName("dictionary");


    // Constructors

        //- Construct given an IOobject
        LSMIOdictionary(const IOobject&);

        //- Construct given an IOobject and dictionary
        LSMIOdictionary(const IOobject&, const dictionary&);

        //- Construct given an IOobject and Istream
        LSMIOdictionary(const IOobject&, Istream&);


    //- Destructor
    virtual ~LSMIOdictionary();


    // Member functions

        //- Name function is needed to disambiguate those inherited
        //  from regIOobject and dictionary
        const word& name() const;

        //- ReadData function required for regIOobject read operation
        bool readData(Istream&);

        //- WriteData function required for regIOobject write operation
        bool writeData(Ostream&) const;


    // Member operators

        //- Assignment of other IOdictionary's entries to this IOdictionary
        void operator=(const LSMIOdictionary&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
