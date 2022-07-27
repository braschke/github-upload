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
    Kamil Braschke
    Chair of Fluid Mechanics
    markus.buerger@uni-wuppertal.de

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "pManager.H"

#include "integration.H"

#include "volFields.H"
#include "dictionary.H"
#include "Time.H"
#include "wordReList.H"
#include "fvcGrad.H"

#include "kinematicMomentumTransportModel.H"
#include "incompressibleMomentumTransportModel.H"//#include "turbulentTransportModel.H"
#include "fluidThermoMomentumTransportModel.H"//#include "turbulentFluidThermoModel.H"


#include "incompressibleTwoPhaseMixture.H"
#include "multiphaseMixture.H"

#include "dynamicFvMesh.H"
#include "treeBoundBox.H"
#include "pointFields.H"
#include "meshToMesh.H"

#include "indexedOctree.H"

#include "splash.H"
#include "volumetricParticle.H"

#include "wallDist.H"

#include "addToRunTimeSelectionTable.H"
#include "fvMesh.H"
#include <ctime>
#include <chrono>


#include "RAPID.H" // For collision checks

#include "Random.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //
namespace Foam
{
namespace functionObjects
{
  defineTypeNameAndDebug(pManager, 0);

  addToRunTimeSelectionTable(functionObject, pManager, dictionary);
}
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::pManager::pManager
(
    const word& name,
    const Time& runTime,
    const dictionary& dict,
    const bool loadFromFiles
)
:
//     fvMeshFunctionObject(name, runTime, dict),
//     logFiles(obr_, name),
    regionFunctionObject(name, runTime, dict),
    name_(name),
    dict_(dict),
//     obr_(obr),
     obr_
     (
         runTime.lookupObject<objectRegistry>
         (
             dict.lookupOrDefault("region", polyMesh::defaultRegion)
         )
  ),
    active_(true),
    log_(false),
    writeLocal_(false),
    patchSet_(),
    pName_(word::null),
    UName_(word::null),
    TName_(word::null),
    EMName_(word::null),
    PolName_(word::null),
    sigmaName_(word::null),
    rhoName_(word::null),
    fDName_(""),
	adhesionIntegrationType_("general"),
	collisionRegionForStructures_("spherical"),
    rhoRef_(VGREAT),
    pRef_(0),
    fsi_(false),
    em_(false),
	thermoForces_(false),
    moveParticles_(true),
    ppCollisions_(true),
	breakAgglomerates_(false),
	breakAgglomeratesIterations_(0),
	printKinetic_(0),
    voidFracPtr_(NULL),
    particleVeloPtr_(NULL),
    wallDistPtr_(NULL),
    wallNPtr_(NULL),
    depositPtr_(NULL),
    writeDevRhoReff_(0),
    devRhoReffPtr_(NULL),
    validDevRhoReff_(false),
    activeFacecentres_(0),
    activeFacevectors_(0),
    particleList_(0),
    contactHash_(),
    contactRadiusFactor_(0),
    moveParticlesRelax_(1),
    moveParticlesSubcycles_(1),
	kinEnPrecision_(0.001),
	kinEnLoopsMax_(50),
    iterativeCouplingInitialRelax_(0.1),
    iterativeCouplingCurrentRelax_(0.1),
    iterativeCouplingSubcycles_(0),
    iterativeCouplingIsSubCycle_(false),
    iterativeCouplingCurrSubcycle_(0),
    iterativeCouplingSavedTime_(),
    popList_(0),
    nPopulations_(0),
    nParticles_(0),
    bgGridPtr_(NULL),
    bgGranularity_(1),
    meshGeomChanged_(false),
    myMS_(runTime, dict),
	stresstensorInterpolationMethod_("firstOutside"),
	stresstensorNeighbourWeighting_("equal"),
	pressureNeighbourWeighting_("equal"),
	cLciWnlci_weighting_("inverseDistance"),
	stresstensorFirstSecondWeight_(0.5),
	pressureInterpolationMethod_("firstOutside"),
	pressureFirstSecondWeight_(0.5)
 {
    _PRINT_ABSFOAM_

    for(label i= 0; i < MAX_POPULATIONS; i++)
      for(label j= 0; j < MAX_POPULATIONS; j++)
      {
        potTable[i][j].reset(0);
        contactModelTable[i][j].reset(0);
      }

    read(dict);

    backGroundGrid();

    if( Pstream::parRun() )
    {
      MPI_Barrier(MPI_COMM_WORLD);
    }
    start();
 }

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::pManager::~pManager()
{
    clearOut();
}

// Delete all storage
void Foam::functionObjects::pManager::clearOut()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::Potential*  Foam::functionObjects::pManager::potTableGet(label i, label j)
{
  Potential *ptr = 0;
  if(i>j)
  {
    if(potTable[j][i].valid())
      ptr = potTable[j][i].operator->();
  }
  else
  {
    if(potTable[i][j].valid())
      ptr = potTable[i][j].operator->();
  }

  return ptr;
}

void    Foam::functionObjects::pManager::potTableSet(label i, label j, autoPtr<Potential> pot)
{
  if(i>j)
    potTable[j][i] = pot;
  else
    potTable[i][j] = pot;
}

Foam::contactModel*  Foam::functionObjects::pManager::cMTableGet(label i, label j)
{
  contactModel *ptr = 0;

  if(i>j)
  {
    if(contactModelTable[j][i].valid())
      ptr = contactModelTable[j][i].operator->();
  }
  else
  {
    if(contactModelTable[i][j].valid())
      ptr = contactModelTable[i][j].operator->();
  }

  return ptr;
}

void        Foam::functionObjects::pManager::cMTableSet(label i, label j, autoPtr<contactModel> pot)
{
  if(i>j)
    contactModelTable[j][i] = pot;
  else
    contactModelTable[i][j] = pot;
}

/*
 * Return name3 and name4 of entry (name1 name2 name3 name4) or (name2 name1 name3 name4)
 */
namespace Foam
{
static bool findPair(
                      const List< List<word> > list,
                      const word &name1,
                      const word &name2,
                            word &name3,
                            word &name4
                    )
{
  forAll(list, i)
  {
    const List<word> &entry = list[i];
    const word& n1 = entry[0];
    const word& n2 = entry[1];

    if(
        (n1 == name1 && n2 == name2)   ||
        (n1 == name2 && n2 == name1)
      )
    {
      name3 = entry[2];
      name4 = entry[3];
      return true;
    }
  }
  return false;
}
}

// void Foam::functionObjects::pManager::read(const dictionary& dict)
bool Foam::functionObjects::pManager::read(const dictionary& dict)
{
    if (active_)
    {
        log_ = dict.lookupOrDefault<Switch>("log", false);
        writeLocal_ = dict.lookupOrDefault<Switch>("writeLocal", false);

//        const fvMesh& mesh = mesh_;
       const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");
//	const fvMesh& mesh = time.lookupObject<fvMesh>("mesh");
//	const fvMesh& mesh = time_.db().parent().lookupObject<fvMesh>("mesh");

        patchSet_ = mesh.boundaryMesh().patchSet
        (
            wordReList(dict.lookup("patches"))
        );

        // Optional entries U and p
        pName_ = dict.lookupOrDefault<word>("pName", "p");
        UName_ = dict.lookupOrDefault<word>("UName", "U");
        TName_ = dict.lookupOrDefault<word>("TName", "T");
        EMName_ = dict.lookupOrDefault<word>("EMName", "EM");
        PolName_ = dict.lookupOrDefault<word>("PolName", "PField");
        sigmaName_ = dict.lookupOrDefault<word>("sigmaName", "sigma");
        rhoName_ = dict.lookupOrDefault<word>("rhoName", "rho");
        adhesionIntegrationType_ = dict.lookupOrDefault<word>("adhesionIntegrationType", "general");
        collisionRegionForStructures_ = dict.lookupOrDefault<word>("collisionRegionForStructures", "spherical");
        Info << nl << "adhesionIntegrationType = " << adhesionIntegrationType_ << endl;

		stresstensorInterpolationMethod_ = dict.lookupOrDefault<word>("stresstensorInterpolationMethod", "firstOutside");
		Info << "Define 'stressTensorInterpolationMethod' = " << stresstensorInterpolationMethod_ << endl;
		pressureInterpolationMethod_ = dict.lookupOrDefault<word>("pressureInterpolationMethod", "firstOutside");
		Info << "Define 'pressureInterpolationMethod' = " << pressureInterpolationMethod_ << endl;
		stresstensorFirstSecondWeight_ = dict.lookupOrDefault<scalar>("stresstensorFirstSecondWeight",0.5);
		Info << "Define 'stresstensorFirstSecondWeight' = " << stresstensorFirstSecondWeight_ << endl;

		stresstensorNeighbourWeighting_ = dict.lookupOrDefault<word>("stresstensorNeighbourWeighting","equal");
		Info << "Define 'stresstensorNeighbourWeighting' = " << stresstensorNeighbourWeighting_ << endl;
		pressureNeighbourWeighting_ = dict.lookupOrDefault<word>("pressureNeighbourWeighting","equal");
		Info << "Define 'pressureNeighbourWeighting' = " << pressureNeighbourWeighting_ << endl;

		cLciWnlci_weighting_ = dict.lookupOrDefault<word>("cLciWnlci_weighting_","inverseDistance");
		Info << "Define 'cLciWnlci_weighting' = " << cLciWnlci_weighting_ << endl;

		pressureFirstSecondWeight_ = dict.lookupOrDefault<scalar>("pressureFirstSecondWeight",0.5);
		Info << "Define 'pressureFirstSecondWeight' = " << pressureFirstSecondWeight_ << endl;

        // Check whether UName, pName and rhoName exists,
        // if not deactivate forces
        if
        (
            !obr_.foundObject<volVectorField>(UName_)
         || !obr_.foundObject<volScalarField>(pName_)
         || (
                rhoName_ != "rhoInf"
             && !obr_.foundObject<volScalarField>(rhoName_)
            )
        )
        {
            active_ = false;

            WarningIn("void Foam::functionObjects::pManager::read(const dictionary&)")
                << "Could not find " << UName_ << ", " << pName_;

            if (rhoName_ != "rhoInf")
            {
                Info<< " or " << rhoName_;
            }

            Info<< " in database." << nl
                << "    De-activating forces." << endl;

        }

        // Reference density needed for incompressible calculations
        rhoRef_ = readScalar(dict.lookup("rhoInf"));

        // Reference pressure, 0 by default
        pRef_ = dict.lookupOrDefault<scalar>("pRef", 0.0);

        // Switch on fluid structure interaction
        fsi_  = dict.lookupOrDefault<Switch>("fsi", false);

        // Switch on electromagnetic interaction
        em_  = dict.lookupOrDefault<Switch>("em", false);

        // Switch on force fields due to temperature interaction
        thermoForces_  = dict.lookupOrDefault<Switch>("thermoForces", false);

        //- move particles
        moveParticles_ = dict.lookupOrDefault<Switch>("moveParticles", true);
        Info << nl << "Define 'moveParticles' = " << moveParticles_ << endl;

        //- resolve particle-particle-collision
        ppCollisions_ = dict.lookupOrDefault<Switch>("ppCollisions", true);
        Info << nl << "Define 'ppCollisions' = " << ppCollisions_ << endl;

        breakAgglomerates_ = dict.lookupOrDefault<Switch>("breakAgglomerates", false);
        Info << nl << "Define 'breakAgglomerates' = " << breakAgglomerates_ << endl;

        if(breakAgglomerates_)
        {
        	breakAgglomeratesIterations_ = dict.lookupOrDefault<int>("breakAgglomeratesIterations", 0);
        	Info << nl << "Using " << breakAgglomeratesIterations_ << " partner particle iterations for breakage decision." << endl;
        }

        //- settings for kinetic energy loop for impact resolution
        if(ppCollisions_)
        {
        	kinEnPrecision_ = dict.lookupOrDefault<scalar>("kinEnPrecision", 0.001);
			kinEnLoopsMax_  = dict.lookupOrDefault<int>("kinEnLoopsMax", 50);

			Info << nl << _LSM_MAGENTA << "Settings for kinetic energy loops in collisions are: " << nl << _LSM_WHITE
					<< "kinEnPrecision = " << (kinEnPrecision_*100) << "%" << nl
					<< "kinEnLoopsMax  = " << kinEnLoopsMax_ << nl;
        }

        printKinetic_ = dict.lookupOrDefault<unsigned int>("printKinetic", 0);
        Info << nl << "Define 'printKinetic' = " << printKinetic_ << endl;

        bgGranularity_ = dict.lookupOrDefault<scalar>("bgGranularity", 1.);
        Info << "Define 'bgGranularity' = " << bgGranularity_ << endl;

        List<word> popNames(0);
        dict.readIfPresent<List<word>  >("populations", popNames);
        if(!popNames.size())
        {
          WarningIn("void Foam::functionObjects::pManager::read(const dictionary&)")
                  << "No populations declared." << nl << "Use 'populations'"
                  << nl << "to declare a list of populations!" << endl;
        }
        popList_.setSize(popNames.size());
        forAll(popNames, iName)
        {
          word name = popNames[iName];
          Population &pop = popList_[iName];
          // Read properties from dict "name" and assign id = iName+1
          pop.read(obr_, backGroundGrid(), &myMS_, dict, name, iName+1); // id 0 is reserved for walls
        }

        forAll(popList_,i) //does solver solve for Temperature in case if any population is subjected to Soot Oxidation?
        {
        	if (popList_[i].withOxidation() && !obr_.foundObject<volScalarField>(TName_) && !popList_[i].getRTemp())
        	{
        		FatalErrorIn("Foam::functionObjects::pManager::read(const dictionary& dict)")
                              << "Soot Oxidation is activated for "<< popList_[i].name() <<
							  ", but could not find the "<< TName_ << " (no Temperature solution in the domain)."
                              << nl << "Please, either use correct solver or switched-off the soot oxidation."
                              << exit(FatalError);
        	}
        }

        List< List<word> > interactionList(0);
        dict.readIfPresent<List< List<word> >  >("interactions", interactionList);

        // Fill tableau of potential pointers:
        // list items are of type (name1 name2 name3 name4)
        // where name1 and name2 are population names or patch names
        // name3/name4 is a name of a subDict defining a potential/contactModel
        // Set potTable[iId][jId] = potPtr; and
        // contactModelTable[iId][jId] = cMPtr;
        // with iId = pop id of population name1
        // with jId = pop id of population name2
        // potPtr = ptr to potential defined by subDict name3
        // cMPtr = ptr to contactModel defined by subDict name4

        for(label iId = 1; iId <= popList_.size(); iId++)
        { // Ids are list-index plus 1
          word name1 = popList_[iId-1].name();
          word potName, cMName;
          word name2;
          for(label jId = iId; jId <= popList_.size(); jId++)
          { // Ids are list-index plus 1
            name2   = popList_[jId-1].name();
            // find entry (name1 name2 name3 name4) or (name2 name1 name3 name4)
            // in interactionList
            if( !findPair(interactionList, name1, name2, potName, cMName) )
            {
              Info << endl << "Could not find potential or contactModel"
                           << " for interaction of "
                           << name1 << " <-> " << name2 << " ." << endl;
            }
            else
            {
              if( potName != "none" )
              {
                potTableSet(iId, jId, Potential::New(dict_.subDict(potName)) );
                Info << nl << "Define potential " << potName
                     << " (" << potTableGet(iId,jId)->getInfoStr() << ")" << nl
                     << " for interaction of "
                     << name1 << " <-> " << name2 << " ." << endl;
              }
              if( cMName != "none" )
              {
                autoPtr<contactModel> cMPtr(new contactModel(dict_.subDict(cMName)));
                cMTableSet(iId, jId, cMPtr);
                Info << nl << "Define conatctModel " << cMName
                     << " (" << cMTableGet(iId,jId)->getInfoStr() << ")" << nl
                     << " for interaction of "
                     << name1 << " <-> " << name2 << " ." << endl;
              }
            }
          }
          // Search for interaction of name1 against "wall"
          name2 = "wall";
          if( !findPair(interactionList, name1, name2, potName, cMName) )
          {
            Info << endl << "Could not find potential or contactModel"
                         << "for interaction of "
                         << name1 << " <-> " << name2 << " ." << endl;
          }
          else
          {
            if( potName != "none" )
            {
              potTableSet(iId, 0, Potential::New(dict_.subDict(potName)) );
              Info << nl << "Define potential " << potName
                   << " (" << potTableGet(iId,0)->getInfoStr() << ")" << nl
                   << " for interaction of "
                   << name1 << " <-> " << name2 << " ." << endl;
            }
            if( cMName != "none" )
            {
              autoPtr<contactModel> cMPtr(new contactModel(dict_.subDict(cMName)));
              cMTableSet(iId, 0, cMPtr);
              Info << nl << "Define contactModel " << cMName
                   << " (" << cMTableGet(iId,0)->getInfoStr() << ")" << nl
                   << " for interaction of "
                   << name1 << " <-> " << name2 << " ." << endl;
            }
          }
        }

        contactRadiusFactor_ = dict.lookupOrDefault<scalar>("contactRadiusFactor", 1.5);
        Info << "Define 'contactRadiusFactor' = " << contactRadiusFactor_ << endl;

        moveParticlesRelax_ = dict.lookupOrDefault<scalar>("moveParticlesRelax", 1.0);
        Info << "Define 'moveParticlesRelax' = " << moveParticlesRelax_ << endl;

        moveParticlesSubcycles_ = dict.lookupOrDefault<label>("moveParticlesSubcycles", 1);
        Info << "Define 'moveParticlesSubcycles' = " << moveParticlesSubcycles_ << endl;

        writeDevRhoReff_ = dict.lookupOrDefault<Switch>("writeDevRhoReff", 0);
        Info << "Writing 'writeDevRhoReff field': " << writeDevRhoReff_ << endl;

        if( dict.found("iterativeCoupling") )
        {
          const dictionary subDict = dict.subDict("iterativeCoupling");

          if( !subDict.found("iterativeCouplingInitialRelax") )
          {
            FatalErrorIn("Foam::functionObjects::pManager::read(const dictionary& dict)")
             << "No relaxation factor for iterative coupling found!"
             << nl << "Use 'iterativeCouplingInitialRelax  <scalar>;' to define."
             << exit(FatalError);
          }
          iterativeCouplingInitialRelax_     = subDict.lookupOrDefault<scalar>("iterativeCouplingInitialRelax", 0.1);

          if( !subDict.found("iterativeCouplingSubcycles") )
          {
            FatalErrorIn("Foam::functionObjects::pManager::read(const dictionary& dict)")
             << "No number of subcycles for iterative coupling found!"
             << nl << "Use 'iterativeCouplingSubcycles  <label>;' to define."
             << exit(FatalError);
          }
          iterativeCouplingSubcycles_ = subDict.lookupOrDefault<label>("iterativeCouplingSubcycles", 0);

/*
          if( !subDict.found("fields") )
          {
            FatalErrorIn("Foam::functionObjects::pManager::read(const dictionary& dict)")
             << "No list of fields participating in iterative coupling found!"
             << nl << "Use 'fields  <wordList>;' to define."
             << exit(FatalError);
          }
          // list of fields (e.g. U, p, T, nu, alhpa) which participate in the cycle
          subDict.lookup("fields") >> iterativeCouplingFields_;
*/
        }
    }

    return true;                                            // Vora
}

void Foam::functionObjects::pManager::printStats(const word& title) const
{
  Info << endl <<
  "========== "<< title << " =============================" << endl;
  forAll(popList_, i)
  {
    popList_[i].printStats();
  }

  Info << endl <<
  "============================================================" << endl;
}


const Foam::bgGrid& Foam::functionObjects::pManager::backGroundGrid()
{
    if (!bgGridPtr_.valid())
    {
       const fvMesh& mesh = refCast<const fvMesh>(obr_); //Vora
       // 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");
        bgGridPtr_.reset(
                          new bgGrid(mesh, bgGranularity_)
                        );
    }

    return bgGridPtr_();
}

void Foam::functionObjects::pManager::moveSolids()
{
	subCyclingPreCollisionSaveState();
    if(moveParticles_)
    {
      scalar relaxStep    = moveParticlesRelax_/moveParticlesSubcycles_;
      scalar currentRelax;
      if( moveParticlesSubcycles_ > 1)
      {
        currentRelax = relaxStep;
        subCyclingSaveState();
        Info << nl << "subcycle: ";
      }
      else
      {
        currentRelax = 1;
      }

      std::chrono::steady_clock::time_point start_time, end_time;

      _PDBOP_("Only breaking agglomerates at beginning of time step !!!", 0)
      if(breakAgglomerates_) breakAgglomerates(currentRelax);

      for(label i = 0; i < moveParticlesSubcycles_; ++i)
      {
        resetSolidForces();
        /*calcSolidForces();
        distributeForces( &volumetricParticle::solidForceField );*/

        //calcAdhesiveForcesBetweenContactPartners();

        if (ppCollisions_)
        {
        	forAll(particleList_, i)
        	{
        	  //particleList_[i]->solidForceField() = vector::zero; // Reset collision forces
          	  particleList_[i]->movedWithContactPartners_ = false;
        	}
        	start_time = std::chrono::steady_clock::now();
        	checkForCollisions(currentRelax);
            end_time = std::chrono::steady_clock::now();

            forAll(particleList_, i)
            {
              //particleList_[i]->unassignedPartners_.clear();
              if(mag(particleList_[i]->getOmega()) >= 100) particleList_[i]->getOmega() /= mag(particleList_[i]->getOmega()) * 1e-2;
              if(mag(particleList_[i]->getAverageOmega()) >= 100) particleList_[i]->getAverageOmega() /= mag(particleList_[i]->getAverageOmega()) * 1e-2;
            }
            //_DBO_("checkForCollisions took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
        	//checkAndUndoPenetration();
        }

        //if(breakAgglomerates_) breakAgglomerates(currentRelax);

        // Clear adhesive forces for actual movement
        // if the particle is not in contact with a structure
        if(adhesionIntegrationType_ == "general")
        {
			for(int p = 0; p < particleList_.size(); p++)
					{
						particleList_[p]->contactForceField() = vector::zero;
						//TESTparticleList_[p]->solidForceField() = vector::zero; // Reset collision forces
					}
        }
        subCyclingPreCollisionSaveState();
        start_time = std::chrono::steady_clock::now();
        move(currentRelax, i);
        end_time = std::chrono::steady_clock::now();

        //TEST the forLoop below
		for(int p = 0; p < particleList_.size(); p++)
				{
					particleList_[p]->solidForceField() = vector::zero; // Reset collision forces
				}


        //_DBO_("kinetics (integration + movement) took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
        //checkAndUndoPenetration();

        subCyclingSaveState();
        Info << i << nl;
      }
      Info << nl;

    }
    else
    {
    	Info<<"===== Running with non-moving particles ====="<< endl;
		calcSolidForces();
    }

    // Deletes particles, that are outside
    // of a user defined bounding box
    deleteParticlesBB();

    forAll(particleList_, i)
    {
      particleList_[i]->unassignedPartners_.clear();
      //particleList_[i]->contactPartners_.clear();
      //particleList_[i]->contactVectors_.clear();
      //particleList_[i]->contactNormals_.clear();
      //particleList_[i]->structureContacts_.clear();
    }

    pOxidation();
}

void Foam::functionObjects::pManager::iterativeCouplingCFDSaveFields()
{
    HashTable<const volVectorField*> volVectFieldsHash = (obr_.lookupClass<volVectorField>());
    for(
        HashTable<const volVectorField*>::const_iterator iter = volVectFieldsHash.begin();
        iter != volVectFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "particleVelo"
        )
      {
        continue;
      }
      iter()->storePrevIter();
    }

    HashTable<const volScalarField*> volScalFieldsHash = (obr_.lookupClass<volScalarField>());
    for(
        HashTable<const volScalarField*>::const_iterator iter = volScalFieldsHash.begin();
        iter != volScalFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "voidFrac"
        )
      {
        continue;
      }
      iter()->storePrevIter();
    }
}

void Foam::functionObjects::pManager::iterativeCouplingCFDRestoreFields()
{
    HashTable<const volVectorField*> volVectFieldsHash = (obr_.lookupClass<volVectorField>());
    for(
        HashTable<const volVectorField*>::const_iterator iter = volVectFieldsHash.begin();
        iter != volVectFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "particleVelo"
        )
      {
        continue;
      }
      const_cast<volVectorField*>(iter())->operator==( iter()->prevIter() );
    }

    HashTable<const volScalarField*> volScalFieldsHash = (obr_.lookupClass<volScalarField>());
    for(
        HashTable<const volScalarField*>::const_iterator iter = volScalFieldsHash.begin();
        iter != volScalFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "voidFrac"
        )
      {
        continue;
      }
      const_cast<volScalarField*>(iter())->operator==( iter()->prevIter() );
    }
}

void Foam::functionObjects::pManager::iterativeCouplingCFDRelax()
{
    HashTable<const volVectorField*> volVectFieldsHash = (obr_.lookupClass<volVectorField>());
    for(
        HashTable<const volVectorField*>::iterator iter = volVectFieldsHash.begin();
        iter != volVectFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "particleVelo"
        )
      {
        continue;
      }
      const_cast<volVectorField*>(iter())->relax(iterativeCouplingCurrentRelax_);
    }

    HashTable<const volScalarField*> volScalFieldsHash = (obr_.lookupClass<volScalarField>());
    for(
        HashTable<const volScalarField*>::iterator iter = volScalFieldsHash.begin();
        iter != volScalFieldsHash.end();
        ++iter
       )
    {
      word name(iter()->name());
      if(
             name.find("_0") != std::string::npos
          || name == "voidFrac"
        )
      {
        continue;
      }
      const_cast<volScalarField*>(iter())->relax(iterativeCouplingCurrentRelax_);
    }
}

bool Foam::functionObjects::pManager::isIterativeCoupling()
{
  return ( iterativeCouplingSubcycles_ > 1 );
}

bool Foam::functionObjects::pManager::isFirstSubiteration()
{
  return ( iterativeCouplingCurrSubcycle_ == 0 );
}

bool Foam::functionObjects::pManager::isLastSubiteration()
{
  return ( iterativeCouplingCurrSubcycle_ >=  (iterativeCouplingSubcycles_ -1) );
}

bool Foam::functionObjects::pManager::isSecondOrLaterSubiteration()
{
  return ( iterativeCouplingCurrSubcycle_ > 0 );
}


Foam::scalar Foam::functionObjects::pManager::subiterationProgress()
{
  // progress is in (0,1]
  return ( (iterativeCouplingCurrSubcycle_ + 1.0) / iterativeCouplingSubcycles_ );
}

Foam::label  Foam::functionObjects::pManager::subiterationsLeft()
{
  return ( iterativeCouplingSubcycles_ - iterativeCouplingCurrSubcycle_ -1 );
}

void Foam::functionObjects::pManager::updateGeomProperties()
{
  if( !meshGeomChanged_ )
    return;

  myMS_.setup();
  // fvMesh has not changed. since this update of
  // geom. properties of pManager, obviously!
  // So, reset 'changed' status.
  meshGeomChanged_ = false;
}


bool Foam::functionObjects::pManager::execute()
{
    Info << nl << "pManager: execute()... " << nl;
    int particlesBeforeInjection = particleList_.size();
    _DBO_("particleList_.size() wurde aufgerufen")
    std::chrono::steady_clock::time_point start_time, end_time;

    //  Check if mesh properties need to be updated
   const fvMesh& mesh = refCast<const fvMesh>(obr_);
   _DBO_("does the mesh need changing: " << mesh.changing())
    if( mesh.changing())
    {
      meshGeomChanged_ = true;
    }
    meshGeomChanged_ = true;
    
    updateGeomProperties();

    Time& time = const_cast<Time&>(obr_.time());

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // prepare basic stuff in first iterative step (always true for non-iterative coupling):
    //
    if( isFirstSubiteration() )
    {
      start_time = std::chrono::steady_clock::now();
      injectParticles();

#if 1
      end_time = std::chrono::steady_clock::now();
      _PDBO_("Injecting particles took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif

      distributeParticles();
#if 1
      end_time = std::chrono::steady_clock::now();
      _PDBO_("Distributing particles took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif
      renewFaceLists();
      preLoadParticleFields(); // to be sure all fields exist in the beginning
      if( isIterativeCoupling() )
      {
        // save particle stuff for subiteration
        iterativeCouplingParticleSavePoints();
        iterativeCouplingParticleSaveForces();
      }
      resetForces();
    }
    //
    // finished: preparing basic stuff
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // First time in iterative coupling: save states
    //
    if(
          isIterativeCoupling()
       && isFirstSubiteration()
      )
    {
      Info << nl << "\tIterative coupling \'on\': "
           << iterativeCouplingSubcycles_ << " subcycles with initial relaxation factor "
           << iterativeCouplingInitialRelax_ << " ." << nl;

      // save CFD stuff
      iterativeCouplingCFDSaveFields();
      // save time state
      iterativeCouplingSavedTime_ = obr_.time();

      iterativeCouplingCurrentRelax_ = iterativeCouplingInitialRelax_;
    }
    //
    // finished saving states
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // define reduced timestep and relaxation factor
    //
    if( isIterativeCoupling() )
    {
      if( isLastSubiteration() )
      {
        // make full time step: restore old deltaT
        scalar deltaT = iterativeCouplingSavedTime_.deltaT().value();

        Info << nl << "\tIterative coupling: last subcycle; "
             << "proceed full CFD time step with deltaT = " << deltaT
             << nl;
        time.setDeltaT(deltaT);

        iterativeCouplingCurrentRelax_ = 1;
      }
      else
      {
        scalar p      = subiterationProgress();
        label  toGo   = subiterationsLeft();
        scalar factor = std::pow(100., -toGo);
        _DBO_("toGo = " << toGo << "; factor = " << factor)

        // make reduced time step
        //scalar timeFrac = factor * iterativeCouplingSavedTime_.deltaT().value();
        scalar timeFrac = 1e-8;

       // increase relaxation factor
       //iterativeCouplingCurrentRelax_ = p * (1-iterativeCouplingInitialRelax_);
       iterativeCouplingCurrentRelax_ = 0.5;

#if 0
       // subcycling progress p \in (0,1]
       scalar p = subiterationProgress();
       // make reduced time step
       scalar timeFrac = p * iterativeCouplingSavedTime_.deltaT().value();

       // increase relaxation factor
       iterativeCouplingCurrentRelax_ += p * (1-iterativeCouplingInitialRelax_);
#endif

       Info << nl << "\tIterative coupling: proceed this subcycle with "
             << "deltaT = " << timeFrac
             << nl;
       time.setDeltaT(timeFrac);
      }
    }
    //
    // finished: defining reduced timestep and relaxation factor
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // from 2nd subcycle on: relax recently calculated CFD fields with stored ones
    //
    if( isSecondOrLaterSubiteration() )
    {
      Info << nl << "\tIterative coupling: subcycle "
           << iterativeCouplingCurrSubcycle_ << " of "
           << iterativeCouplingSubcycles_ << " with relaxation factor "
           << iterativeCouplingCurrentRelax_ << nl;

      iterativeCouplingCFDRelax();
    }
    //
    // finished: relax CFD solution
    ///////////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // map fluid forces to particles
    //
    Info << nl << "pManager: Map fluid forces to particles\' surface."
         << nl;

    start_time = std::chrono::steady_clock::now();
    mapFluidForcesToParticles();
#if 1
      end_time = std::chrono::steady_clock::now();
//      _PDBO_("MapFluidForcesToParticles took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif
    start_time = std::chrono::steady_clock::now();
    distributeForces( &volumetricParticle::fluidForceField );

    distributePointVelocity(); // Only for point particles
#if 1
      end_time = std::chrono::steady_clock::now();
      _PDBO_("DistributeFluidForceField took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif
    start_time = std::chrono::steady_clock::now();
    mapThermophoreticForcesToParticles();
    if(thermoForces_) distributeForces( &volumetricParticle::thermoForceField );
    mapElectroMagneticForcesToParticles();
    if(em_) distributeForces( &volumetricParticle::electromagForceField );
#if 1
      end_time = std::chrono::steady_clock::now();
 //     _PDBO_("Other mapping & fieldDistributing took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif

    //
    // finished: mapping fluid forces to particles
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////
    // from 2nd subcycle on: relax recently interpolated force fields on particles with stored ones
    if( isSecondOrLaterSubiteration() )
    //if( isIterativeCoupling() )
    {
      Info << nl << "\tIterative coupling: relaxing interpolated fluid forces on particles\' surface."
           << nl;

      iterativeCouplingParticleRelaxForces();
      // remember this as prediction of counter force
      iterativeCouplingParticleSaveForces();
    }
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // move particles
    //
    Info << nl << "pManager: Move particles."
         << nl;

    /*Info << nl <<  "pManager: Entering checkForContacts." << nl;
    start_time = std::chrono::steady_clock::now();
    checkForContacts();*/
    Info << nl <<  "pManager: Distributing forces." << nl;
    distributeForces( &volumetricParticle::contactForceField );
    Info << nl <<  "pManager: Moving solids." << nl;
//    start_time = std::chrono::steady_clock::now();
    moveSolids();
//    end_time = std::chrono::steady_clock::now();
//    _PDBO_("Moving solids " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")

    // prepare next CFD step: modify matrices of linear equation
    Info << nl << "pManager: Map particles\' momentum to voidFrac and particleVelo."
         << nl;

    start_time = std::chrono::steady_clock::now();
    if(particlesBeforeInjection < particleList_.size()) mapParticleMomentumToFluid();
    else mapParticleMomentumToFluidOverNeighbours();
    end_time = std::chrono::steady_clock::now();
    _PDBO_("Mapping particle momentum took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")

    _PDBI_

//    start_time = std::chrono::steady_clock::now();
    mapParticlePermittivityToFluid();
    _PDBI_
    mapParticleSigmaToFluid();
    _PDBI_
//    end_time = std::chrono::steady_clock::now();
//    _PDBO_("Mapping electromagnetics took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
    //mapParticleDepositToFluid();

    if(     isIterativeCoupling()
        && !isLastSubiteration()
      )
    {
      Info << nl << "\tIterative coupling: restore old particle positions."
           << nl;
      iterativeCouplingParticleRestorePoints();
      // remember: particles momentum is still mapped to fluid
    }
    //
    // finished: moving particles
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // prepare for CFD calculation
    //
    if( isIterativeCoupling() )
    {
      // turn back time
      Info << nl << "\tIterative coupling: turn back time."
           << nl;
      time.setTime(
                    iterativeCouplingSavedTime_.value(),
                    iterativeCouplingSavedTime_.timeIndex()
                  );

      // from 2nd subcycle on
      if( isSecondOrLaterSubiteration() )
      {
        // turn back old CFD and particle state
        Info << nl << "\tIterative coupling: restore old CFD states."
             << nl;
        iterativeCouplingCFDRestoreFields();
      }

      // count subcycles
      if( isLastSubiteration() )
      { // reached end?
        iterativeCouplingCurrSubcycle_ = 0;
      }
      else
      {
        ++iterativeCouplingCurrSubcycle_;
      }
    }
    //
    // finished: preparing for CFD calculation
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    //start_time = std::chrono::steady_clock::now();
    //printStats("pManager statistics");
    //end_time = std::chrono::steady_clock::now();
    //_PDBO_("printing stats took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")

    start_time = std::chrono::steady_clock::now();
    _PDBI_
    endOfExecution();
#if 0
      end_time = std::chrono::steady_clock::now();
      _PDBO_("endOfExecution took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
#endif
    Info<< "Foam::functionObjects::pManager:: end execute()";


      _PDBI_
    if(printKinetic_) printKineticEnergy();

    return true;                                            // Vora
}


void Foam::functionObjects::pManager::start()
{
    return;//CHANGED since of8 uses start() and execute() both before first iteration
    updateGeomProperties();

    emergeParticles();
    Info << nl << "Setting up via pManager: start()... " << nl;
    std::chrono::steady_clock::time_point start_time, end_time;

    start_time = std::chrono::steady_clock::now();
    injectParticles();
    end_time = std::chrono::steady_clock::now();
    start_time = std::chrono::steady_clock::now();
    distributeParticles();
    end_time = std::chrono::steady_clock::now();
    start_time = std::chrono::steady_clock::now();
    renewFaceLists();
    end_time = std::chrono::steady_clock::now();
    start_time = std::chrono::steady_clock::now();
    preLoadParticleFields(); // to be sure all fields exist in the beginning
    end_time = std::chrono::steady_clock::now();
    resetForces();


    // map fluid forces to particles
    Info << nl << "pManager: map fluid forces to particles\' surface."
         << nl;

    mapFluidForcesToParticles();
    distributeForces( &volumetricParticle::fluidForceField );
    mapThermophoreticForcesToParticles();
    distributeForces( &volumetricParticle::thermoForceField );
    mapElectroMagneticForcesToParticles();
    distributeForces( &volumetricParticle::electromagForceField );
    //
    // finished: mapping fluid forces to particles
    // map particle to fluid field
    Info << nl << "pManager: Map particles\' momentum to voidFrac and particleVelo."
         << nl;

    distributePointVelocity(); // Only for point particles

    start_time = std::chrono::steady_clock::now();
    mapParticleMomentumToFluid();
    end_time = std::chrono::steady_clock::now();
    _PDBO_("initial MAPPARTICLEMOMENTUM took " << 1.0 * (std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000.0 << " seconds")
    mapParticlePermittivityToFluid();
    mapParticleSigmaToFluid();
    //mapParticleDepositToFluid();

    printStats("Injection Statistics");
    Info << nl << "Finished setup via pManager" << nl;
}

bool Foam::functionObjects::pManager::end()                 // Vora
{
    // Do nothing
    return true;
}

void Foam::functionObjects::pManager::timeSet()
{

}

bool Foam::functionObjects::pManager::write()
{
    if( obr_.time().outputTime() )
    {
      Info << nl << "pManager: writing journals, properties and geometries of particles... ";
      writePopulationJournal();
      writeParticlePropertiesAndGeometries();
      Info << "done!" << nl << endl;
    }

    endOfExecution();

    return true;                                            // Vora
}

void Foam::functionObjects::pManager::writeParticlePropertiesAndGeometries()
{
  forAll(popList_, i)
  {
    popList_[i].writeParticlePropertiesAndGeometries(backGroundGrid());
  }
}

void Foam::functionObjects::pManager::endOfExecution()
{
  forAll(popList_, i)
  {
    popList_[i].endOfExecution();
  }
}


void Foam::functionObjects::pManager::writePopulationJournal()
{
  forAll(popList_, i)
  {
    popList_[i].writeJournal();
  }
}

void Foam::functionObjects::pManager::emergeParticles()
{
  forAll(popList_, i)
  {
      popList_[i].restoreFromJournal();
  }
}

void Foam::functionObjects::pManager::distributeParticles()
{
  forAll(popList_, i)
  {
      popList_[i].distributeParticles(backGroundGrid());
	  //popList_[i].distributeParticlesMPI(backGroundGrid());
  }
}

template <class Type>
void Foam::functionObjects::pManager::distributeForces(
                                 Field<Type>& (volumetricParticle::* fieldGetter) ()
                               )
{
  forAll(popList_, i)
  {
      //if(!popList_[i].isPointParticle()) popList_[i].distributeForces(backGroundGrid(), fieldGetter);
	  if(!popList_[i].isPointParticle()) popList_[i].distributeForcesNew(fieldGetter);
  }
}

void Foam::functionObjects::pManager::distributePointVelocity()
{
 forAll(popList_, i)
 {
     if(popList_[i].isPointParticle()) popList_[i].distributePointVelocity();
 }
}

void Foam::functionObjects::pManager::deleteParticlesBB()
{
  forAll(popList_, i)
  {
    popList_[i].deleteParticlesBB();
  }
}

void Foam::functionObjects::pManager::injectParticles()
{
  forAll(popList_, i)
  {
    //_DBO_ TODO_DBO_("isInsideFirst: " << isInsideFirst)
    _DBO_("in poplist are " << i << "elements.");
    popList_[i].inject();
  }
}

void Foam::functionObjects::pManager::preLoadParticleFields()
{
  if( !fsi_ )
    return;

  // map pressure and stress of main mesh to all particles
  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    volumetricParticle *pPtr = particleList_[i];

//    Population& pop = popList_[pPtr->popId()-1];

    pPtr->fluidForceField();
    pPtr->solidForceField();
    pPtr->contactForceField();

//    if( pop.withThermoPhoresis() )
      pPtr->thermoField();
      pPtr->electromagField();
  }
}

void Foam::functionObjects::pManager::move(scalar relax, int subiteration)
{
  if( !moveParticles_ )
    return;

  forAll(popList_, i)
  {
    popList_[i].move(relax, subiteration);
  }
}

void Foam::functionObjects::pManager::checkForContacts() {

	const fvMesh& mesh = refCast<const fvMesh>(obr_); //Vora
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");


	forAll(popList_, i) {
		if (popList_[i].withContactCheck() || popList_[i].withNearWallEffects())
		{
			wallN() = wallDist(mesh).n();
			wallDistance() = wallDist(mesh).y();
		}
		if (popList_[i].withContactCheck()){
			Info << "Solving for macro-contacts... "<< nl;
			popList_[i].checkForContacts(particleList_);
		}
		if (popList_[i].withNearWallEffects()){
			Info<< "Solving adhesion and friction... "<< nl;
			popList_[i].checkNearWallEffects();
		}
	}
}

//*****************SootBurn-TrialCode****Vora
void Foam::functionObjects::pManager::pOxidation()
{
  forAll(popList_,i)
	{
	  if (popList_[i].withOxidation())
		{
		  Info << "Solving for Oxidation. . ."<< nl;
		  //popList_[i].pOxidation(backGroundGrid());
		  popList_[i].pOxidation();
		}
	}
}

// Moves contact partners slightly apart from each other
// if one STL is penetrating the other.
// This is necessary to assure that agglomerates can break apart again
// as otherwise very deep penetrations / collisions could lead to
// non-seperable objects.
// Most part of the code is identical to the collision checks and setup in checkForCollisions().
void Foam::functionObjects::pManager::checkAndUndoPenetration() {

	for(int i = 0; i < particleList_.size(); i++)
	{
		volumetricParticle* firstPrt = particleList_[i];

		// Skip if object is structure or particle with adhesion that doesn't have any contactPartners
		if (firstPrt->myPop_->isStructure() || (firstPrt->myPop_->H() && firstPrt->contactPartners_.size() == 0) ) continue;

		// Prepare collision modell for particle 1
		RAPID_model *m1 = new RAPID_model;
		m1->BeginModel();

		forAll(firstPrt->triSurf() , tri)
			{
				double p0[3] = { ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[0])]).x(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[0])]).y(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[0])]).z() };

				double p1[3] = { ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[1])]).x(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[1])]).y(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[1])]).z() };

				double p2[3] = { ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[2])]).x(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[2])]).y(),
								 ((firstPrt->triSurf().points())[((firstPrt->triSurf()[tri])[2])]).z() };

				m1->AddTri( p0, p1, p2, tri );
			}
		m1->EndModel();

		forAllIter(List<volumetricParticle*>, firstPrt->contactPartners_, otherPrt)
		{
			// Prepare collision modell for particle 2
			RAPID_model *m2 = new RAPID_model;
			m2->BeginModel();

			forAll((*otherPrt)->triSurf() , tri)
			{
				double p0[3] = { (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[0])]).x(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[0])]).y(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[0])]).z() };

				double p1[3] = { (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[1])]).x(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[1])]).y(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[1])]).z() };

				double p2[3] = { (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[2])]).x(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[2])]).y(),
								 (((*otherPrt)->triSurf().points())[(((*otherPrt)->triSurf()[tri])[2])]).z() };

				m2->AddTri( p0, p1, p2, tri );
			}
			m2->EndModel();

			// Check for collisions
			double R1[3][3], R2[3][3], T1[3], T2[3];
			R1[0][0] = R1[1][1] = R1[2][2] = 1.0;
			R1[0][1] = R1[1][0] = R1[2][0] = 0.0;
			R1[0][2] = R1[1][2] = R1[2][1] = 0.0;
			R2[0][0] = R2[1][1] = R2[2][2] = 1.0;
			R2[0][1] = R2[1][0] = R2[2][0] = 0.0;
			R2[0][2] = R2[1][2] = R2[2][1] = 0.0;
			T1[0] = 0.0;  T1[1] = 0.0; T1[2] = 0.0;
			T2[0] = 0.0;  T2[1] = 0.0; T2[2] = 0.0;
			RAPID_Collide(R1, T1, m1, R2, T2, m2, RAPID_ALL_CONTACTS);


			delete m2; // Important for memory issues

			if (!RAPID_num_contacts)
				{
				continue;
				}

			vector dist, contactVector;
			int elem_id1, elem_id2;
			scalar distMag = 0;

			// Solution for faces with DEEPEST penetration - IN DIRECTION OF FACE1
			for(int i = 0; i < RAPID_num_contacts; i++)
				{
					dist = ( firstPrt->triSurf().faceCentres()[RAPID_contact[i].id1] - (*otherPrt)->triSurf().faceCentres()[RAPID_contact[i].id2] );
					if(distMag < mag(dist & firstPrt->normals()[RAPID_contact[i].id1]))
						{
						distMag = mag(dist);
						elem_id1 = RAPID_contact[i].id1;
						elem_id2 = RAPID_contact[i].id2;
						}
				}

			// ================ HERE BEGINGS THE UNIQUE CODE OF THIS METHOD COMPARED TO checkForCollisions() ================

			// Get the corner points of the triangular elements
			// taking part in the collision
			pointField tri1Points(3), tri2Points(3);
			tri1Points[0] = firstPrt->triSurf().points()[((firstPrt->triSurf()[elem_id1])[0])];
			tri1Points[1] = firstPrt->triSurf().points()[((firstPrt->triSurf()[elem_id1])[1])];
			tri1Points[2] = firstPrt->triSurf().points()[((firstPrt->triSurf()[elem_id1])[2])];
			tri2Points[0] = (*otherPrt)->triSurf().points()[(((*otherPrt)->triSurf()[elem_id2])[0])];
			tri2Points[1] = (*otherPrt)->triSurf().points()[(((*otherPrt)->triSurf()[elem_id2])[1])];
			tri2Points[2] = (*otherPrt)->triSurf().points()[(((*otherPrt)->triSurf()[elem_id2])[2])];

			int point1, point2;
			distMag = 0;
			vector veloDirection = firstPrt->getVelocity() / (mag(firstPrt->getVelocity()) + VSMALL);

			// Select points of particle 1 that are in particle 2 and vice versa.
			// Then select points with shortest distance in velocity direction.
			List<bool> isInsideFirst(3), isInsideOther(3);
			firstPrt->isInside(tri2Points, backGroundGrid(), isInsideFirst);
			(*otherPrt)->isInside(tri1Points, backGroundGrid(), isInsideOther);

			/*_DBO_("isInsideFirst: " << isInsideFirst)
			_DBO_("isInsideOther: " << isInsideOther)*/

			if(firstPrt->getVelocity() == vector::zero)
			{
				//veloDirection = firstPrt->getAcceleration() / (mag(firstPrt->getAcceleration()) + VSMALL);
				veloDirection = (*otherPrt)->normals()[elem_id2];
			}

			for(int i = 0; i < 3; i++)
			{
				if(!isInsideOther[i]) continue;
				for(int j = 0; j < 3; j++)
				{
					//if(!isInsideFirst[j]) continue;
					if(  mag( ( tri1Points[i] - tri2Points[j] ) & veloDirection ) > distMag)
					{
						distMag = mag( ( tri1Points[i] - tri2Points[j] ) & veloDirection );
						point1 = i;
						point2 = j;
					}
				}
			}

			_DBO_("Undoing penetration with distMag = " << distMag << " and veloDirection = " << veloDirection)
			int directionSign = -sign(veloDirection & (firstPrt->Sf()[elem_id1]));
			pointField r 	= firstPrt->points();
			distMag *= 0.8;
			r				+= directionSign * distMag * veloDirection;
			firstPrt->triSurf().movePoints(r);
		}

		delete m1; // Important for memory issues
	}
}

void Foam::functionObjects::pManager::undoPenetration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, int elem_id1, int elem_id2) {

	// Get the corner points of the triangular elements
	// taking part in the collision
	pointField tri1Points(4), tri2Points(4);
	tri1Points[0] = firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[0])];
	tri1Points[1] = firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[1])];
	tri1Points[2] = firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[2])];
	tri1Points[3] = ( tri1Points[0] + tri1Points[1] + tri1Points[2] ) / 3;
	tri2Points[0] = otherPrt.triSurf().points()[((otherPrt.triSurf()[elem_id2])[0])];
	tri2Points[1] = otherPrt.triSurf().points()[((otherPrt.triSurf()[elem_id2])[1])];
	tri2Points[2] = otherPrt.triSurf().points()[((otherPrt.triSurf()[elem_id2])[2])];
	tri2Points[3] = ( tri2Points[0] + tri2Points[1] + tri2Points[2] ) / 3;

	int point1, point2;
	scalar distMag = 0;
	vector veloDirection = firstPrt.getVelocity() / (mag(firstPrt.getVelocity()) + VSMALL);

	// Select points of particle 1 that are in particle 2 and vice versa.
	// Then select points with biggest distance in velocity direction.
	List<bool> isInsideFirst(4), isInsideOther(4);
	firstPrt.isInside(tri2Points, backGroundGrid(), isInsideFirst);
	otherPrt.isInside(tri1Points, backGroundGrid(), isInsideOther);

	if(firstPrt.getVelocity() == vector::zero)
	{
		//veloDirection = firstPrt->getAcceleration() / (mag(firstPrt->getAcceleration()) + VSMALL);
		veloDirection = otherPrt.normals()[elem_id2];
	}
	veloDirection = otherPrt.normals()[elem_id2];

	for(int i = 0; i < 4; i++)
	{
		if(!isInsideOther[i]) continue;
		for(int j = 0; j < 4; j++)
		{
			//if(!isInsideFirst[j]) continue;
			//if(  mag( ( tri1Points[i] - tri2Points[j] ) & veloDirection ) > distMag)
			if(  mag(tri1Points[i] - tri2Points[j]) > distMag)
			{
				//distMag = mag( ( tri1Points[i] - tri2Points[j] ) & veloDirection );
				distMag = mag( tri1Points[i] - tri2Points[j] );
				veloDirection = ( tri1Points[i] - tri2Points[j] ) / distMag;
				point1 = i;
				point2 = j;
			}
		}
	}

	// If STL is too rough and none of the colliding points
	// are within the other body use middlepoints of triElements
	if (distMag <= SMALL)
	{
		distMag = mag(tri1Points[4] - tri2Points[4]);
		veloDirection = (tri1Points[4] - tri2Points[4]) / distMag;
	}

	_DBO_("Undoing penetration with distMag = " << distMag << " and veloDirection = " << veloDirection)
	int directionSign = -sign(veloDirection & (firstPrt.Sf()[elem_id1]));
	pointField r 	= firstPrt.points();
	distMag *= 0.8;
	r				+= directionSign * distMag * veloDirection;
	firstPrt.triSurf().movePoints(r);
}

// Move two particles away from each other
// according to the vector distVec
void Foam::functionObjects::pManager::undoPenetration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector distVec) {

	scalar distMag 	= mag(distVec);
	_DBO_("Undoing penetration with distMag = " << distMag << " and distVec = " << distVec)
	pointField r 	= firstPrt.points();
	//distVec 	   *= -1.0 / ( 1.1 * moveParticlesSubcycles_ + VSMALL );
	distVec 	   *= -1.0;
	r			   += distVec;
	firstPrt.triSurf().movePoints(r);

	// Same for other particle, except if it is a structure
	if(!otherPrt.myPop_->isStructure())
	{
		r 				= otherPrt.points();
		r			   -= distVec;
		otherPrt.triSurf().movePoints(r);
	}
}

// Version for objects that don't adhere and are in
// contact with a structure
void Foam::functionObjects::pManager::undoPenetrationWithStructure(volumetricParticle& firstPrt, volumetricParticle& otherPrt, List<int> pFaces, vector structNormal, vector structCf) {

	pointField r;
	pointField triPoints(3 * pFaces.size());
	List<bool> isInsideStructure(3 * pFaces.size());
	double maxPenetration 	= 0;
	int deepestPoint		= 0;

	// Fill the list with all points of the faces colliding with the structure
	for(int i = 0; i < pFaces.size(); i++)
	{
		int elem_id1	= pFaces[i];

		triPoints[i*3] 		= firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[0])];
		triPoints[i*3 + 1] 	= firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[1])];
		triPoints[i*3 + 2] 	= firstPrt.triSurf().points()[((firstPrt.triSurf()[elem_id1])[2])];
	}

	// Check which points are inside the structure
	otherPrt.isInside(triPoints, backGroundGrid(), isInsideStructure);

	// Find deepest point inside the structure
	for(int i = 0; i < triPoints.size(); i++)
	{
		if(isInsideStructure[i] && (mag((structCf - triPoints[i]) & structNormal) > maxPenetration))
		{
			maxPenetration 	= mag((structCf - triPoints[i]) & structNormal);
			deepestPoint 	= i;
		}
	}

	// Now move the STL out according to the deepest point
	r 				= firstPrt.points();
	r			   -= 0.9 * structNormal * maxPenetration;
	firstPrt.triSurf().movePoints(r);

	_DBO_("Moving away from structure with the vector: " << (-structNormal * maxPenetration) << "\n AND TIMES REDUCED FACTOR 0.9 FOR MOVING")

}

// Reduce a List<volumetricParticle*> so that
// it contains no duplicate entries
void Foam::functionObjects::pManager::reduceParticleList(List<volumetricParticle*>& prtList)
{
	List<volumetricParticle*> reducedList;

	if (prtList.size() <= 1) return;

	// Append only the last appearance
	// of any entry in prtList
	for(int i = 0; i < prtList.size(); i++)
	{
		for(int j = i+1; j < prtList.size(); j++)
		{
			if(prtList[i]->idStr() == prtList[j]->idStr()) break;
			else if(j == (prtList.size() - 1)) reducedList.append(prtList[i]);
		}
	}
	reducedList.append(prtList[prtList.size() - 1]); // Last one can always be appended, as any possible clones appeared before
	prtList = reducedList;
}


// Check for separation of particles:
// iteration = 0: just check for single particle detachment
// iteration = 1: do iteration = 0, then check for detachment of agglomerate consisting of particle and neighbours with distance up to 1
// iteration = n: do iteration = 0, 1, ..., n-1, then check for detachment of agglomerate consisting of particle and neighbours with distance up to n
// Distance 1 are direct neighbours
// Distance 2 are neighbours' neighbours
// Distance n are (n-1) * "neighbours'" + neighbours
void Foam::functionObjects::pManager::reassignContactPartners(volumetricParticle& particle, int iterations)
{
	if(particle.myPop_->isStructure()|| !breakAgglomerates_ || particle.contactPartners_.size() == 0) return;

	List<volumetricParticle*> kernelList;    // contains particles constructing kernel of current subiteration
	List<volumetricParticle*> borderList;    // contains particles at the border from which detachment might occure
	List<volumetricParticle*> oldBorderList; // contains particles at the border from which detachment might occure (from previous iteration)
	List<volumetricParticle*> deleteList;    // contains particles that should be deleted from the kernel particle's contactPartners_ list
	List<volumetricParticle*> keepList;      // contains particles that should be kept as the new border
	List<volumetricParticle*> helperList;
	List<vector> adhesionList;               // Speed-up: Used to save adhesive forces, thus do not have to recalculate them again.

	// Compute breakage for increasing sizes of kernel
	for(int i = 0; i <= iterations; i++)
	{

		if(i == 0)
		{
			// Set fresh new kernel and borders
			kernelList.append(&particle);
			borderList.append(&particle);
			oldBorderList.append(&particle);
		}
		else
		{
			// New kernel includes former border particles
			oldBorderList = borderList;
			kernelList.append(borderList);
		}


		// Expand border particles by neighbours of
		// former border particles
		helperList = borderList;
		borderList.clear();
		forAllIter(List<volumetricParticle*>, helperList, borderPrt)
		{
			borderList.append((*borderPrt)->contactPartners_);
		}

		// Exclude particles from borderList if they are
		// already part of the kernel
		helperList = borderList;
		borderList.clear();
		for(int b = 0; b < helperList.size(); b++)
		{
			for(int k = 0; k < kernelList.size(); k++)
			{
				if(helperList[b]->idStr() == kernelList[k]->idStr()) break;
				else if(k == (kernelList.size() - 1)) borderList.append(helperList[b]);
			}
		}
		reduceParticleList(borderList);
		if(borderList.size() == 0) return; // No more adjacent particles to check adhesion with
		helperList.clear();

		////////////
		// Now both kernelList and boarderList are prepared for
		// separation checks !!!
		////////////


		// Calculate total force on kernel
		vector kernelForce = vector::zero;
		forAllIter(List<volumetricParticle*>, kernelList, kernelPrt)
		{
			kernelForce += (*kernelPrt)->getTotalForce();
		}

		// Calculate total adhesion force
		// on kernel particles (with border particles)
		vector kernelAdhesion = vector::zero;
		forAllIter(List<volumetricParticle*>, oldBorderList, oldPrt)
		{
			vector adhesionVector;

			forAllIter(List<volumetricParticle*>, borderList, borderPrt)
			{
				if( !((*oldPrt)->isPartner(*borderPrt)) ) continue;
				adhesionVector  = testAdhesion( **oldPrt, **borderPrt);
				adhesionList.append(adhesionVector);
			}
		}


		// Perform separation checks between particles from
		// old border and new border
		int adhesionIterator = 0;
		forAllIter(List<volumetricParticle*>, oldBorderList, oldPrt)
		{
			deleteList.clear();

			// Put on deletion list if total kernel force
			// is greater than total adhesion force in direction
			// of adhesion with border particle.
			// Direction is evaluated component wise.
			forAllIter(List<volumetricParticle*>, borderList, borderPrt)
			{
				if( !((*oldPrt)->isPartner(*borderPrt)) ) continue;

				vector adhesionVector = adhesionList[adhesionIterator];
				vector direction      = adhesionVector / (mag(adhesionVector) + VSMALL);
				adhesionIterator++;

				kernelAdhesion = vector::zero;
				forAllIter(List<vector>, adhesionList, adh)
				{
					if((*adh).x() * direction.x() > 0) kernelAdhesion.x() += (*adh).x();
					if((*adh).y() * direction.y() > 0) kernelAdhesion.y() += (*adh).y();
					if((*adh).z() * direction.z() > 0) kernelAdhesion.z() += (*adh).z();
				}

				vector directedAdhesion = (kernelAdhesion & direction) * direction;

				if(((kernelForce + directedAdhesion) & directedAdhesion) <= 0)
				{
#if 1
					_PDBOP_("Separating particles " << (*oldPrt)->idStr() << " and " << (*borderPrt)->idStr()
							<< "\n totalForce = " << kernelForce
							<< "\n adhesionVector = " << adhesionVector
							<< "\n ((kernelForce + adhesionVector) & adhesionVector) = " << ((kernelForce + adhesionVector) & adhesionVector)
							<< "\n (*oldPrt)->getTotalForce() = " << (*oldPrt)->getTotalForce()
							<< "\n sum((*oldPrt)->fluidForceField()) = " << sum((*oldPrt)->fluidForceField())
							<< "\n sum((*oldPrt)->solidForceField()) = " << sum((*oldPrt)->solidForceField())
							<< "\n sum((*oldPrt)->contactForceField()) = " << sum((*oldPrt)->contactForceField())
							<< "\n (*borderPrt)->getTotalForce() = " << (*borderPrt)->getTotalForce()
							, 0)
					_PDBOP_("Fluidforcefield = " << (*oldPrt)->fluidForceField(), 0)
#endif
					deleteList.append(*borderPrt);
					if((*borderPrt)->myPop_->isStructure())
					{
						(*oldPrt)->structureContacts_.clear();
					}
				}
				else if(!(*borderPrt)->myPop_->isStructure()) // Don't add structures, because they would later become part of the kernel
				{
					keepList.append(*borderPrt);
				}
			}

			// Now delete all deleteList particles from the current border particle...
			(*oldPrt)->deleteParticlesFromList(deleteList);
			(*oldPrt)->unassignedPartners_.append(deleteList);

			// ... and delete old border particles from deleteList's particles contactPartners
			helperList.append(*oldPrt);
			forAllIter(List<volumetricParticle*>, deleteList, deletePrt)
			{
				(*deletePrt)->deleteParticlesFromList(helperList);
				(*deletePrt)->unassignedPartners_.append(helperList);
			}
			helperList.clear();
		}

		borderList = keepList;
		if(borderList.size() == 0) return; // No more adjacent particles to check adhesion with
		reduceParticleList(borderList);
		keepList.clear();
		adhesionList.clear();
	}
}

Foam::vector Foam::functionObjects::pManager::testAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt)
{
	// Find the right contact vector
	vector contactVector;
	for(int i = 0; i < firstPrt.contactPartners_.size(); i++)
	{
		if(firstPrt.contactPartners_[i]->idStr() == otherPrt.idStr())
		{
			contactVector = firstPrt.contactVectors_[i];
			break;
		}
	}


	double hamaker, force, force1, force2, forceReductionFactor1, forceReductionFactor2;
	hamaker = sqrt( firstPrt.myPop_->H() * otherPrt.myPop_->H() );

	forceReductionFactor1 = firstPrt.myPop_->adhesionReductionFactor();
	forceReductionFactor2 = otherPrt.myPop_->adhesionReductionFactor();


	double contactDist = mag(contactVector);
	contactVector /= contactDist + VSMALL; // normalized vector pointing in contact direction
	contactDist += firstPrt.myPop_->a0(); // depth of contact + literature contact distance (often 4 Angström)
	contactVector *= contactDist;
	pointField r = firstPrt.triSurf().points();
	r += contactVector;
	firstPrt.triSurf().movePoints(r);

	const vectorField firstCf = firstPrt.Cf();
	const vectorField firstSf = firstPrt.Sf();
	const vectorField firstNormals = firstPrt.normals();
	vectorField firstForceField = firstPrt.contactForceField();
	firstForceField = vector::zero;

	const vectorField otherCf = otherPrt.Cf();
	const vectorField otherSf = otherPrt.Sf();
	const vectorField otherNormals = otherPrt.normals();

	vector avgNormals = vector::zero;
	vector avgNormalsWeighted = vector::zero;
	double distance, factor;
	vector distanceVec;

	forAll(firstCf, firstIter)
	{
		force  = 0;
		force1 = 0;
		force2 = 0;

		forAll(otherCf, otherIter)
		{
			distance = mag(firstCf[firstIter] - otherCf[otherIter]);
			if(distance > 10e-6) continue; // Hard coded cut-off distance of 1e-7m; complexity increases in O(n²)

			factor   = 0.5 * hamaker / ( distance * distance * distance * 3 * constant::mathematical::twoPi + VSMALL );
			force = factor * ( mag(firstSf[firstIter]) +  mag(otherSf[otherIter]) ) / 2.0;
			force1 = factor * mag(firstSf[firstIter]);
			firstForceField[firstIter] += force1 * firstNormals[firstIter] / forceReductionFactor1;
		}
	}

	r -= contactVector;
	firstPrt.triSurf().movePoints(r);

//	_PDBOP_("adh = " << sum(firstForceField), 0)

	return sum(firstForceField);
}



// Look through all contactPartners_ of particle
// and check if they are still in contact.
// adjust the list particle.contactPartners_ accordingly
void Foam::functionObjects::pManager::reassignContactPartners(volumetricParticle& particle)
{
	// Prepare collision data for first object

	RAPID_model *m1 = new RAPID_model;
	m1->BeginModel();
	triSurface *triSurface1Ptr		 =  new triSurface(particle.triSurf());
	triSurface triSurf1  = *triSurface1Ptr;
	delete triSurface1Ptr;
	forAll(triSurf1 , tri)
	{
		double p0[3] = { ((triSurf1.points())[((triSurf1[tri])[0])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[0])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[0])]).z() };

		double p1[3] = { ((triSurf1.points())[((triSurf1[tri])[1])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[1])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[1])]).z() };

		double p2[3] = { ((triSurf1.points())[((triSurf1[tri])[2])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[2])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[2])]).z() };

		m1->AddTri( p0, p1, p2, tri );
	}
	m1->EndModel();

	// Reassigned lists with contact partners
	List<volumetricParticle*> 			reassignedPartners, unassignedPartners;
	List<volumetricParticle::facePair>	reassignedFaces;
	List<vector>						reassignedVectors;
	List<vector>						reassignedNormals;
	volumetricParticle*					partner;
	vector								contactDirection;
	vector								contactNormal;

	// Check for all contact partners if they are still in contact
	// with particle. If yes then write into reassigned lists.
	if(particle.contactPartners_.size() != particle.contactVectors_.size())
	{
		forAll(particle.contactPartners_, iter)
		{
			_DBO_(particle.contactPartners_[iter]->idStr())
		}
	}
	for(int i = 0; i < particle.contactPartners_.size(); i++)
	{
		partner          = particle.contactPartners_[i];

		collisionCheckVer1(*m1, *partner);

		// Prefer contactVectors defined by surface normals of structures if possible
		if(partner->myPop_->isStructure() && RAPID_num_contacts) contactDirection = -1 * partner->normals()[RAPID_contact[0].id2];
		//else contactDirection =  particle.contactVectors_[i] / ( mag(particle.contactVectors_[i]) + VSMALL );
		/*if(partner->myPop_->isStructure())
		{
			for(int j = 0; j < partner->contactPartners_.size(); j++)
			{
				if( partner->contactPartners_[j]->idStr() == particle.idStr())
				{
					contactDirection  = -1 * partner->contactVectors_[j];
					contactDirection /= mag(contactDirection) + VSMALL;
					break;
				}
			}
		}*/
		else contactDirection =  particle.contactVectors_[i] / ( mag(particle.contactVectors_[i]) + VSMALL );

		contactNormal = particle.contactNormals_[i];

		_DBO_("Data for brake apart decision:"
				<< "\nDirection of contact: " << contactDirection
				<< "\nNormal of contact: " << contactNormal
				<< "\nBreaking velocity: " << particle.getVelocity()
				<< "\nFluid force:" << sum(particle.fluidForceField())
				<< "\nAdhesion force:" << sum(particle.contactForceField())
				<< "\nTotal force:" << particle.getTotalForce()
				<< "\n force X normal = " << (particle.getTotalForce() & contactNormal)
				)

        vector forceResult = particle.getTotalForce();
        //vector adhResult   = sum(particle.contactForceField());
		contactDirection = sum(particle.contactForceField());



		//if ( (!RAPID_num_contacts) ||
		//if( ( particle.myPop_->isStructure() ||  partner->myPop_->isStructure() ) && ( (particle.getVelocity() & contactDirection) < 0 && ( particle.getVelocity() & partner->getVelocity() ) <= 0 ))
		//if( ( particle.myPop_->isStructure() ||  partner->myPop_->isStructure() ) && (!RAPID_num_contacts) )
        //if( ( particle.myPop_->isStructure() ||  partner->myPop_->isStructure() ) && (!RAPID_num_contacts || ((forceResult & adhResult) < 0) ) )
        if( ( particle.myPop_->isStructure() ||  partner->myPop_->isStructure() ) && ((forceResult & contactDirection) < 0) )
		{
			//_DBO_("REMOVING PARTICLE " << partner->idStr() << " FROM PARTNER LIST OF " << particle.idStr())
			unassignedPartners.append(partner);
			particle.unassignedPartners_.append(partner);
			particle.structureContacts_.clear();
			partner->structureContacts_.clear();
		}
		//else if( !RAPID_num_contacts )
		//else if( ( (particle.getVelocity() & contactDirection) < 0 && ( particle.getVelocity() & partner->getVelocity() ) <= 0 ) || !RAPID_num_contacts )
        //else if( !RAPID_num_contacts || ((forceResult & adhResult) < 0) )
        else if( (forceResult & contactNormal) < 0 )
		{
		//_DBO_("REMOVING PARTICLE " << partner->idStr() << " FROM PARTNER LIST OF " << particle.idStr())
			unassignedPartners.append(partner);
			particle.unassignedPartners_.append(partner);
		}
		else
		{
			// Collision, so write into reassigned list
			//_DBO_("Rapid contacts with old partner are: " << RAPID_num_contacts)
			reassignedPartners.append(partner);
			reassignedVectors.append(particle.contactVectors_[i]);
			reassignedNormals.append(particle.contactNormals_[i]);
			reassignedFaces.append(particle.contactFaces_[i]);
		}
	}

	// Set partner lists to adjusted lists
	if(reassignedPartners.size())
	{
		particle.contactPartners_	= reassignedPartners;
		particle.contactFaces_		= reassignedFaces;
		particle.contactVectors_	= reassignedVectors;
		particle.contactNormals_	= reassignedNormals;
	}

	delete m1;


	// Now delete the current particle from
	// the lists of the newly unassigned partners
	for(int i = 0; i < unassignedPartners.size(); i++)
	{
		//_DBO_("Unassigning for " << unassignedPartners[i]->idStr() <<"\n Its partners size is " << unassignedPartners[i]->contactPartners_.size())
		reassignedPartners.clear();
		reassignedVectors.clear();
		reassignedNormals.clear();

		for(int j = 0; j < unassignedPartners[i]->contactPartners_.size(); j++)
		{
			if(unassignedPartners[i]->contactPartners_[j]->idStr() != particle.idStr())
			{
				//_DBO_("got something: " << particle.idStr())
				reassignedPartners.append(unassignedPartners[i]->contactPartners_[j]);
				reassignedVectors.append(unassignedPartners[i]->contactVectors_[j]);
				reassignedNormals.append(unassignedPartners[i]->contactNormals_[j]);
			}
		}


		unassignedPartners[i]->contactPartners_ = reassignedPartners;
		unassignedPartners[i]->contactVectors_ 	= reassignedVectors;
		unassignedPartners[i]->contactNormals_ 	= reassignedNormals;
		unassignedPartners[i]->unassignedPartners_.append(&particle);
		//_DBO_("Now the partners size changed to " << unassignedPartners[i]->contactPartners_.size())
	}

}


// Check if object firstPrt and object otherPrt
// have any intersecting surface elements.
// Amount of contacts is written into the
// global variable RAPID_num_contacts and the colliding
// faces are stored in RAPID_contact[i].id1 and RAPID_contact[i].id2
void Foam::functionObjects::pManager::collisionCheckVer2(volumetricParticle& firstPrt, volumetricParticle& otherPrt)
{
	// Prepare collision data for first object
	RAPID_model *m1 = new RAPID_model;
	m1->BeginModel();
	triSurface *triSurface1Ptr		 =  new triSurface(firstPrt.triSurf());
	triSurface triSurf1  = *triSurface1Ptr;
	delete triSurface1Ptr;
	forAll(triSurf1 , tri)
	{
		double p0[3] = { ((triSurf1.points())[((triSurf1[tri])[0])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[0])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[0])]).z() };

		double p1[3] = { ((triSurf1.points())[((triSurf1[tri])[1])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[1])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[1])]).z() };

		double p2[3] = { ((triSurf1.points())[((triSurf1[tri])[2])]).x(),
						 ((triSurf1.points())[((triSurf1[tri])[2])]).y(),
						 ((triSurf1.points())[((triSurf1[tri])[2])]).z() };

		m1->AddTri( p0, p1, p2, tri );
	}
	m1->EndModel();


	// Pipe into singular object version of method
	// and do the actual collision check
	collisionCheckVer1(*m1, otherPrt);

	delete m1;
}


// Same as collisionCheckVer2, but with
// one passed RAPID_model m1 to save time
// otherwise needed for constructing m1
void Foam::functionObjects::pManager::collisionCheckVer1(RAPID_model& m1, volumetricParticle& otherPrt)
{
	// Prepare collision data for second object
	RAPID_model *m2 = new RAPID_model;
	m2->BeginModel();
	triSurface *triSurface2Ptr		 =  new triSurface(otherPrt.triSurf());
	triSurface triSurf2  = *triSurface2Ptr;
	delete triSurface2Ptr;

	forAll(triSurf2 , tri)
	{
		double p0[3] = { ((triSurf2.points())[((triSurf2[tri])[0])]).x(),
						 ((triSurf2.points())[((triSurf2[tri])[0])]).y(),
						 ((triSurf2.points())[((triSurf2[tri])[0])]).z() };

		double p1[3] = { ((triSurf2.points())[((triSurf2[tri])[1])]).x(),
						 ((triSurf2.points())[((triSurf2[tri])[1])]).y(),
						 ((triSurf2.points())[((triSurf2[tri])[1])]).z() };

		double p2[3] = { ((triSurf2.points())[((triSurf2[tri])[2])]).x(),
						 ((triSurf2.points())[((triSurf2[tri])[2])]).y(),
						 ((triSurf2.points())[((triSurf2[tri])[2])]).z() };

		m2->AddTri( p0, p1, p2, tri );
	}
	//_DBO_("Points are:\n" << triSurf2.points() )
	m2->EndModel();


	// Matrices needed for collision detection; mainly identities
	double R1[3][3], R2[3][3], T1[3], T2[3];
	R1[0][0] = R1[1][1] = R1[2][2] = 1.0;
	R1[0][1] = R1[1][0] = R1[2][0] = 0.0;
	R1[0][2] = R1[1][2] = R1[2][1] = 0.0;
	R2[0][0] = R2[1][1] = R2[2][2] = 1.0;
	R2[0][1] = R2[1][0] = R2[2][0] = 0.0;
	R2[0][2] = R2[1][2] = R2[2][1] = 0.0;
	T1[0] = 0.0;  T1[1] = 0.0; T1[2] = 0.0;
	T2[0] = 0.0;  T2[1] = 0.0; T2[2] = 0.0;

	// Actual collision check
	RAPID_Collide(R1, T1, &m1, R2, T2, m2, RAPID_ALL_CONTACTS);

	delete m2;
}

bool Foam::functionObjects::pManager::areParticlesFarApart(volumetricParticle* firstPrt, volumetricParticle* otherPrt)
{

	if (!firstPrt->myPop_->isStructure() || collisionRegionForStructures_ == "spherical")
	{
		return (mag( firstPrt->getCg() - otherPrt->getCg() ) >=
					( firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_ ));
	}
	else if(collisionRegionForStructures_ == "cylindricalZ")
	{
		vector distVec  = firstPrt->getCg() - otherPrt->getCg();
		scalar distRad  = distVec.x() * distVec.x() + distVec.y() * distVec.y();
		scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
		distCrit       *= distCrit;
		if(distRad >= distCrit)
			return true;
		else
			return false;

	}
	else if(collisionRegionForStructures_ == "cylindricalY")
	{
		vector distVec  = firstPrt->getCg() - otherPrt->getCg();
		scalar distRad  = distVec.x() * distVec.x() + distVec.z() * distVec.z();
		scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
		distCrit       *= distCrit;
		if(distRad >= distCrit)
			return true;
		else
			return false;

	}
	else if(collisionRegionForStructures_ == "cylindricalX")
	{
		vector distVec  = firstPrt->getCg() - otherPrt->getCg();
		scalar distRad  = distVec.z() * distVec.z() + distVec.y() * distVec.y();
		scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
		distCrit       *= distCrit;
		if(distRad >= distCrit)
			return true;
		else
			return false;

	}
	else if(collisionRegionForStructures_ == "planeX")
		{
			vector distVec  = firstPrt->getCg() - otherPrt->getCg();
			scalar distMag  = distVec.x() * distVec.x();
			scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
			distCrit       *= distCrit;
			if(distMag >= distCrit)
				return true;
			else
				return false;

		}
	else if(collisionRegionForStructures_ == "planeY")
		{
			vector distVec  = firstPrt->getCg() - otherPrt->getCg();
			scalar distMag  = distVec.y() * distVec.y();
			scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
			distCrit       *= distCrit;
			if(distMag >= distCrit)
				return true;
			else
				return false;

		}
	else if(collisionRegionForStructures_ == "planeZ")
		{
			vector distVec  = firstPrt->getCg() - otherPrt->getCg();
			scalar distMag  = distVec.z() * distVec.z();
			scalar distCrit = firstPrt->myPop_->collDist() * firstPrt->scale_ + otherPrt->myPop_->collDist() * otherPrt->scale_;
			distCrit       *= distCrit;
			if(distMag >= distCrit)
				return true;
			else
				return false;

		}
	else
	{
	    FatalErrorIn("pManager::areParticlesFarApart(volumetricParticle* firstPrt, volumetricParticle* otherPrt)")
	              << "'"<< collisionRegionForStructures_ << "' is not a valid type for collisionRegionForStructures!" << nl
	              << "Valid types are: " << nl
	              << "'spherical'" << nl
				  << "cylindricalX" << nl
				  << "cylindricalY" << nl
				  << "cylindricalZ" << nl
	              << exit(FatalError);
	}
}


// Checks for collisions between two STL-objects
// Differentiates between a newly occuring collision
// and an already existing one like in the case of two
// adhering particles.
void Foam::functionObjects::pManager::checkForCollisions(scalar currentRelax) {

	for(int i = 0; i < particleList_.size(); i++)
	{
	volumetricParticle* firstPrt = particleList_[i];

	// Small particles don't check for collisions at first.
	// After they have been deposited on a structure however
	// they will look for further collisions.
	if(firstPrt->myPop_->isPointParticle() && firstPrt->contactPartners_.size() == 0) continue;


	// Information on pre-existing contact partners
	List<volumetricParticle*> contactPartnersTemp = firstPrt->contactPartners_;
	double otherPrtIsPartner;

	// Used to check weather the collision model for the first object
	// has already been created to save significant time
	bool m1Created = false;

	//RAPID_model *m1 = new RAPID_model;
	RAPID_model *m1;
	triSurface triSurf1;

	for(int j = 0; j < particleList_.size(); j++)
	{
		// Reduce possible pair-wise combinations except for structures
		// as they are always at the end of the list
		if(i > j && !(firstPrt->myPop_->isStructure()) && firstPrt->contactPartners_.size() == 0 ) continue;
		if(i==j) continue;

		volumetricParticle* otherPrt = particleList_[j];

		// IMPORTANT:
		// This means that in the whole adhesion method, a structure can
		// only be the firstPrt and NEVER the otherPrt !!!
		if(otherPrt->myPop_->isStructure()) continue;

		// Ignore collisions with own population if set like this in controlDict
		if(!(firstPrt->myPop_->collidesWithOwnPopulation()) && otherPrt->myPop_ == firstPrt->myPop_ ) continue;


		// Check if other particle is close enough according to
		// dictionary entry "collisionDistance" (in units for the non-scaled geometry!!!)
		// for the population in controlDict.
		if( areParticlesFarApart(firstPrt, otherPrt) )
		{
			continue;
		}


		// Particles that are already considered to be partners
		// should not perform any further collision checks
		// between each other
		otherPrtIsPartner = false;
		for (int i = 0; i < contactPartnersTemp.size(); i++)
		{
			if (contactPartnersTemp[i]->idStr() == otherPrt->idStr())
			{
				otherPrtIsPartner = true;
				break;
			}
		}
		if(otherPrtIsPartner && !(firstPrt->myPop_->isStructure()) )
		{
			continue;
		}


		// Make sure that a particle which got recently unassigned from the partner list
		// isn't directly reassigned. This is necessary as the unassignment is done via
		// a prediction of the movement, i.e. at this moment right now the unassigned partners
		// can still be touching due to a subCyclingRestoreState() although the previous loop
		// calculated that they will go apart.
		bool gotUnassigned = false;
		for(int u = 0; u < firstPrt->unassignedPartners_.size(); u++)
		{
			if(firstPrt->unassignedPartners_[u]->idStr() == otherPrt->idStr())
			{
				gotUnassigned = true;
				break;
			}
		}
		// Prevent recombination
		// of just unassigned particles
		if (gotUnassigned) continue;


		triSurface *triSurface2Ptr		 =  new triSurface(otherPrt->triSurf());
		triSurface triSurf2  = *triSurface2Ptr;
		delete triSurface2Ptr;


		////Prepare collision model for particle 1
		//
		if(!m1Created)
		{
			m1 = new RAPID_model;
			m1->BeginModel();
			triSurface *triSurface1Ptr		 =  new triSurface(firstPrt->triSurf());
			triSurf1  = *triSurface1Ptr;
			delete triSurface1Ptr;

			forAll(triSurf1 , tri)
			{
				double p0[3] = { ((triSurf1.points())[((triSurf1[tri])[0])]).x(),
								 ((triSurf1.points())[((triSurf1[tri])[0])]).y(),
								 ((triSurf1.points())[((triSurf1[tri])[0])]).z() };

				double p1[3] = { ((triSurf1.points())[((triSurf1[tri])[1])]).x(),
								 ((triSurf1.points())[((triSurf1[tri])[1])]).y(),
								 ((triSurf1.points())[((triSurf1[tri])[1])]).z() };

				double p2[3] = { ((triSurf1.points())[((triSurf1[tri])[2])]).x(),
								 ((triSurf1.points())[((triSurf1[tri])[2])]).y(),
								 ((triSurf1.points())[((triSurf1[tri])[2])]).z() };

				m1->AddTri( p0, p1, p2, tri );
			}
			m1->EndModel();
			m1Created = true;
		}
		//
		////Finished preparing model for particle 1


		collisionCheckVer1(*m1, *otherPrt); // Return collision data

#if 0
		_DBO_("firstPrt = " << firstPrt->idStr() << "\totherPrt = " << otherPrt->idStr() <<
				"\nnumber of contacts: " << RAPID_num_contacts <<
			  "\nenum: " << RAPID_ALL_CONTACTS )
#endif


		vector dist, contactVector;
		int elem_id1, elem_id2;
		scalar distMag = 0;

		if(!RAPID_num_contacts) continue;

		//_DBO_("FirstCollider+" << firstPrt->idStr() <<"\tSecondCollider+" <<otherPrt->idStr())

		int originalCollidingFacesN = RAPID_num_contacts; // might be nice to keep, e.g. for reentrainment reasons

		// Make a reduced list with no double entries
		// for faces with multiple registered collisions
		List<int> reducedList1, reducedList2;
		reducedList1.append(RAPID_contact[0].id1);
		reducedList2.append(RAPID_contact[0].id2);
		for(int i = 1; i < RAPID_num_contacts; i++)
		{
			int iter = 0;
			while(iter < reducedList1.size())
			{
				if(reducedList1[iter] == RAPID_contact[i].id1)
				{
					iter = -1;
					break;
				}
				iter++;
			}
			if (iter != -1) reducedList1.append(RAPID_contact[i].id1); // Arrived at end of list without double entries

			iter = 0;
			while(iter < reducedList2.size())
			{
				if(reducedList2[iter] == RAPID_contact[i].id2)
				{
					iter = -1;
					break;
				}
				iter++;
			}
			if (iter != -1) reducedList2.append(RAPID_contact[i].id2);
		}

		// Check if the force needs to be spread over several STL elements.
		// It will prevent that a body goes off into a completely wrong direction
		// after a point-edge collision for example.
		// Generally a face whose faceNormal has a positive angle with the faceVelocity
		// should be taken into consideration to prevent the application of forces on the wrong side
		// move into the same general direction. Therefore (and to safe computational time) we
		// go over all colliding faces.
		// IMPORTANT: The "averages" are weighted with the face area to reduce the influence
		// of tiny fluctuating surface elements!!!
		double			collisionArea1 = VSMALL;
		double			collisionArea2 = VSMALL;
		vector			avgFaceVelo1(vector::zero);
		vector			avgFaceVelo2(vector::zero);
		vector			avgFaceNormal1(vector::zero);
		vector			avgFaceNormal2(vector::zero);
		vector			avgFaceCenter1(vector::zero);
		vector			avgFaceCenter2(vector::zero);
		int				maxListIter = max(reducedList1.size(), reducedList2.size());

		for(int i = 0; i < maxListIter; i++)
		{
			if(i < reducedList1.size())
			{
				elem_id1 = reducedList1[i];

				collisionArea1	+= mag(firstPrt->Sf()[elem_id1]);
				avgFaceVelo1	+= firstPrt->getFaceVelocity(elem_id1) * mag(firstPrt->Sf()[elem_id1]);
				avgFaceNormal1	+= firstPrt->normals()[elem_id1] * mag(firstPrt->Sf()[elem_id1]);
				avgFaceCenter1	+= firstPrt->Cf()[elem_id1] * mag(firstPrt->Sf()[elem_id1]);
			}
			if(i < reducedList2.size())
			{
				elem_id2 = reducedList2[i];

				collisionArea2	+= mag(otherPrt->Sf()[elem_id2]);
				avgFaceVelo2	+= otherPrt->getFaceVelocity(elem_id2) * mag(otherPrt->Sf()[elem_id2]);
				avgFaceNormal2	+= otherPrt->normals()[elem_id2] * mag(otherPrt->Sf()[elem_id2]);
				avgFaceCenter2	+= otherPrt->Cf()[elem_id2] * mag(otherPrt->Sf()[elem_id2]);
			}
		}

#if 0
		_DBO_("Dump of average values in the collision BEFORE NORMALIZATION:"
		<< "\nParticle1 = " << firstPrt->idStr()
		<< "\nParticle2 = " << otherPrt->idStr()
		<< "\nCg1 = " << firstPrt->getCg()
		<< "\nCg2 = " <<  otherPrt->getCg()
		<< "\ncollisionArea1 = " <<  collisionArea1
		<< "\ncollisionArea2 = " <<  collisionArea2
		<< "\navgFaceCenter1 = " << avgFaceCenter1
		<< "\navgFaceCenter2 = " << avgFaceCenter2
		<< "\navgFaceNormal1 = " <<  avgFaceNormal1
		<< "\navgFaceNormal2 = " <<  avgFaceNormal2
		<< "\nmag(avgFaceNormal1) = " <<  mag(avgFaceNormal1)
		<< "\nmag(avgFaceNormal2) = " <<  mag(avgFaceNormal2)
		<< "\navgFaceVelo1 = " << avgFaceVelo1
		<< "\navgFaceVelo2 = " << avgFaceVelo2
		<< "\nreducedList1 = " << reducedList1
		<< "\nreducedList2 = " << reducedList2
		)
#endif

		avgFaceNormal1	/= collisionArea1;
		avgFaceNormal2	/= collisionArea2;
		avgFaceNormal1	/= mag(avgFaceNormal1) + VSMALL;
		avgFaceNormal2	/= mag(avgFaceNormal2) + VSMALL;
		if(avgFaceNormal2 == vector::zero) avgFaceNormal2 = -avgFaceNormal1;
		if(avgFaceNormal1 == vector::zero) avgFaceNormal1 = -avgFaceNormal2;
		avgFaceVelo1	/= collisionArea1;
		avgFaceVelo2	/= collisionArea2;
		avgFaceCenter1	/= collisionArea1;
		avgFaceCenter2	/= collisionArea2;
		avgFaceVelo1	= (((avgFaceNormal1 & avgFaceVelo1) * avgFaceNormal1) ); //& avgFaceNormal2) * avgFaceNormal2;
		avgFaceVelo2	= (((avgFaceNormal2 & avgFaceVelo2) * avgFaceNormal2) ); //& avgFaceNormal1) * avgFaceNormal1;


#if 0
		_DBO_("Dump of average values in the collision:"
		<< "\nCg1 = " << firstPrt->getCg()
		<< "\nCg2 = " <<  otherPrt->getCg()
		<< "\ncollisionArea1 = " <<  collisionArea1
		<< "\ncollisionArea2 = " <<  collisionArea2
		<< "\navgFaceCenter1 = " << avgFaceCenter1
		<< "\navgFaceCenter2 = " << avgFaceCenter2
		<< "\navgFaceNormal1 = " <<  avgFaceNormal1
		<< "\navgFaceNormal2 = " <<  avgFaceNormal2
		<< "\nmag(avgFaceNormal1) = " <<  mag(avgFaceNormal1)
		<< "\nmag(avgFaceNormal2) = " <<  mag(avgFaceNormal2)
		<< "\navgFaceVelo1 = " << avgFaceVelo1
		<< "\navgFaceVelo2 = " << avgFaceVelo2
		<< "\nreducedList1 = " << reducedList1
		<< "\nreducedList2 = " << reducedList2
		)
#endif

		// Solution for faces with MOST SHALLOW penetration - IN DIRECTION OF FACE1 & FACE2
		distMag = VGREAT;
		for(int i = 0; i < RAPID_num_contacts; i++)
				{
				dist = ( triSurf1.faceCentres()[RAPID_contact[i].id1] - triSurf2.faceCentres()[RAPID_contact[i].id2] );
				//if(distMag < mag((((dist & avgFaceNormal1) * avgFaceNormal1)) & avgFaceNormal2) )
				//if(distMag > mag((((dist & avgFaceNormal1) * avgFaceNormal1)) & avgFaceNormal2) )
				if(distMag > mag(dist) && (dist & avgFaceNormal1) > 0)
					{
					if(firstPrt->myPop_->isStructure()) distMag = dist & avgFaceNormal1;
					else distMag = mag((((dist & avgFaceNormal1) * avgFaceNormal1)) & avgFaceNormal2);
					elem_id1 = RAPID_contact[i].id1;
					elem_id2 = RAPID_contact[i].id2;
					}
				}


		// More precise solution for penetrating faces if firstPrt is a structure
		if(firstPrt->myPop_->isStructure())
		{
			distMag = 0;

			const vectorField* otherPoints = &(otherPrt->triSurf().points());
			const Foam::List<Foam::labelledTri>* otherLocalFaces = &(otherPrt->triSurf());

			vector pointOnStructure = triSurf1.faceCentres()[RAPID_contact[0].id1];

			// Adapative activation of loop over all faces
			// to get closest colliding edge point for triangle integration of adhesive forces.
			// Might significantly increase calculation time and can yield wrong
			// edges if a concave body envolopes another in the collision.
			bool triangleIntegration = (adhesionIntegrationType_ == "triangleIntegration") ? true : false;
			int maxListSize = triangleIntegration ? otherLocalFaces->size() : reducedList2.size();

			for(int i = 0; i < maxListSize; i++)
			{
				for(int j = 0; j < 3; j++)
				{
					vector edgePoint = triangleIntegration ? (*otherPoints)[(*otherLocalFaces)[i] [j]] : (*otherPoints)[(*otherLocalFaces)[reducedList2[i]] [j]];

					// Fast check if otherPrt's point is inside structure
					// Might fail for plate-like geometries
					scalar distance = (edgePoint - pointOnStructure) & avgFaceNormal1;
					if( distance < 0)
					{
						if(distMag < mag(distance))
						{
							distMag  = mag(distance);
							elem_id2 = triangleIntegration ? i : reducedList2[i];
							contactVector = -distMag * avgFaceNormal1;
						}
					}
				}
			}
		}
		else
		{
			distMag = mag(
					((( firstPrt->Cf()[elem_id1] - otherPrt->Cf()[elem_id2] )
							& avgFaceNormal2 ) * avgFaceNormal2 ) & avgFaceNormal1
							);
			// contactVector is later used in the calculation of the adhesive forces
			contactVector = otherPrt->Cf()[elem_id2] - firstPrt->Cf()[elem_id1];
		}

		//_DBO_("structure->Cf = " << triSurf1.faceCentres()[RAPID_contact[i].id1] << "\t elem_id2 = " << elem_id2 << "\t distMag = " << distMag << "\t contactVector = " << contactVector)


		// If other particle was already a partner of current structure
		// run method to adjust amount of contact points and
		// skip to next particle
		if(otherPrtIsPartner && firstPrt->myPop_->isStructure())
		{
			if(firstPrt->myPop_->isStructure()) otherPrt->setContactPointsWithStructure( reducedList2, avgFaceNormal2, avgFaceCenter2 );
			continue;
		}


		 // Particles with no adhesion are directly moved outside of the colliding
		 // body to prevent further penetration
		 if(!otherPrt->myPop_->H() && firstPrt->myPop_->isStructure())
		 {
			 contactVector = - avgFaceNormal1 * distMag;
			 contactVector /= mag(contactVector) + VSMALL;
			 undoPenetrationWithStructure(*otherPrt, *firstPrt, reducedList2, contactVector, firstPrt->Cf()[elem_id1]);
			 otherPrt->subCyclingSavePosition();
		 }


	 	/*
		Now that we know that the two objects are colliding right at this very moment
		calculate and apply collision forces and later check if the objects are going
		to remain together even after the collision.
		*/

		// Calculate first estimation for collision force at the colliding faces.
		// Value is based on a centric fully elastic collision.
		scalar commonMass;
		scalar sonicspeed = 5088.01; // calculate as v_s = sqrt(YoungsModulus/density); not used right now
		scalar youngsModulus = 1.55e5;
		scalar impactTime = 2 * firstPrt->myPop_->collDist() * firstPrt->scale_ / sonicspeed;
		if(firstPrt->myPop_->isStructure())
		{
			// Neglect other mass, because the movement of the structure is abmissal and as such is its change of momentum
			commonMass = otherPrt->getMass();
		}
		else
		{
			commonMass = otherPrt->getMass() + otherPrt->getMass();
		}


		vector forceDirection	= ( avgFaceNormal2 - avgFaceNormal1 );
		forceDirection /= mag(forceDirection) + VSMALL;
		scalar collForceTrans	= mag( ( avgFaceVelo2 - avgFaceVelo1 ) * commonMass ) / ( impactTime + VSMALL );

		scalar collForceRot;
		vector rc				= (avgFaceCenter1 + avgFaceCenter2) * 0.5;
		if(firstPrt->myPop_->isStructure())
			{
			rc = avgFaceCenter2;
			collForceRot = mag(( otherPrt->getJ() &  otherPrt->getOmega() )
															/ ( mag( ( (otherPrt->getCg() - rc) ^ forceDirection ) * impactTime )  + VSMALL ) );
			}
		else
			{
			collForceRot	= mag(( (- firstPrt->getJ() + otherPrt->getJ() ) & firstPrt->getOmega()
															- (otherPrt->getJ() & otherPrt->getOmega()) )
															/ ( mag( ( (firstPrt->getCg() + otherPrt->getCg() - 2 * rc) ^ forceDirection ) * impactTime )  + VSMALL ) );
			}

		if(firstPrt->myPop_->isStructure()) collForceTrans *= 2.0;
		scalar collForceTot		= collForceRot + collForceTrans;
		collForceTot *= impactTime / ( time_.deltaT().value() ) ; // Integrate over impactTime & make independent of timeStep

# if 0
		_DBO_(
			"CollisionForce between " << firstPrt->idStr() << " and " << otherPrt->idStr() << "is " << collForceTot
		<< "\nfaceVelo1 = " << firstPrt->getFaceVelocity(elem_id1) <<"\t mass1 = " << firstPrt->getMass()
		<< "\nfaceVelo2 = " << otherPrt->getFaceVelocity(elem_id2) <<"\t mass2 = " << otherPrt->getMass()
		<< "\nimpactTime = " << impactTime <<"\t deltaT = " << time_.deltaT().value()
		<< "\nDenominator of collForceRot = " << ( (firstPrt->getCg() - rc) ^ forceDirection )
			)
#endif

		// Add collisionForces to the solidForceField
		scalar maxPenetration = 1e-3;
		scalar minPenetration = 1e-12;
		scalar collisionForcePen = (distMag < maxPenetration) ? (distMag * youngsModulus) : (maxPenetration * youngsModulus);

		vector veloInPlane;
		//_DBO_("collForceTrans = " << collForceTrans << "\tcollForceRot = " << collForceRot << "\tcollForceTot = " << collForceTot)

		// Particles smaller than a cell are supposed to always adhere
		if(otherPrt->myPop_->isPointParticle()) collForceTot = 0;

		// Integrates adhesive forces if firstPrt has non-zero hamaker value
		// and integration didn't already happen (as it does for existing contact partners)
		if(firstPrt->myPop_->H() && !otherPrtIsPartner) integrateAdhesion(*firstPrt, *otherPrt, contactVector, elem_id1, elem_id2);

		volumetricParticle::facePair firstCollidingFaces, otherCollidingFaces;
		firstCollidingFaces.elem_id1 = elem_id1;
		firstCollidingFaces.elem_id2 = elem_id2;
		otherCollidingFaces.elem_id1 = elem_id1;
		otherCollidingFaces.elem_id2 = elem_id2;

			if(collForceTot != 0)
			{
#if 0
			_DBO_( "############# Data before integrating collision forces (infos do not consider agglomerate state) #####################"
					"\n"<< firstPrt->idStr() << " AND " << otherPrt->idStr()
								<< "\nveloFirstPrt = " << firstPrt->getVelocity()
								<< "\nomegaFirstPrt = " << firstPrt->getOmega()
								<< "\nkinEn firstPrt = " << firstPrt->getKineticEnergy()
								<< "\nveloOtherPrt = " << otherPrt->getVelocity()
								<< "\nveloOmegaPrt = " << otherPrt->getOmega()
								<< "\nkinEn otherPrt = " << otherPrt->getKineticEnergy()
								<< "\ncollForceTot = " << collForceTot )
#endif
		//////////////////////////////////////////////////////
		// Adjust collision forces
		// with kinetic energy conservation.
		// Compare kin. energy of collision system (body1 & body2)
		// before and after collision.
		//
		//

			vector firstKinEn, otherKinEn;
			vector firstPrtPreCollVelo = firstPrt->getVelocity();
			vector otherPrtPreCollVelo = otherPrt->getVelocity();
			vector firstPrtIterVelo, otherPrtIterVelo;
			bool directionSwitched;
			// Differentiate between colliding agglomerates and colliding singular particles
			if(firstPrt->myPop_->isStructure()) firstKinEn = vector::zero;
			else if(!firstPrt->contactPartners_.size())
			{
				firstPrt->calcTotalLoadNoAdhesion();
				firstPrt->calcAcceleration();
				firstPrt->calcVelocity(currentRelax);
				firstKinEn = firstPrt->getKineticEnergy();
				firstPrt->subCyclingRestoreState();
			}
			else
			{
				firstKinEn = firstPrt->getKineticEnergyOfAgglomerate(currentRelax);
				firstPrt->subCyclingRestoreState();
			}

			if(otherPrt->myPop_->isStructure()) otherKinEn = vector::zero;
			else if(!otherPrt->contactPartners_.size())
			{
				otherPrt->calcTotalLoadNoAdhesion();
				otherPrt->calcAcceleration();
				otherPrt->calcVelocity(currentRelax);
				otherKinEn = otherPrt->getKineticEnergy();
				otherPrt->subCyclingRestoreState();
			}
			else
			{
				otherKinEn = otherPrt->getKineticEnergyOfAgglomerate(currentRelax);
				otherPrt->subCyclingRestoreState();
			}

			// Current kinetic energy after considering dissipation
			scalar oldKinEn = firstPrt->myPop_->collForceConservation() * firstKinEn.z() + otherPrt->myPop_->collForceConservation() * otherKinEn.z();

			if(firstPrt->myPop_->isStructure()) firstKinEn = vector::zero;
			else if(!firstPrt->contactPartners_.size())
			{
				firstPrt->calcCollisionLoad();
				firstPrt->calcAcceleration();
				firstPrt->calcVelocity(currentRelax);
				firstKinEn = firstPrt->getKineticEnergy();
				firstPrt->subCyclingRestoreState();
			}
			else
			{
				firstKinEn = firstPrt->getKineticEnergyOfAgglomerate(currentRelax, true); // true = only calculate collision load of agglomerate, false is the default value
				firstPrt->subCyclingRestoreState();
			}

			if(otherPrt->myPop_->isStructure()) otherKinEn = vector::zero;
			else if(!otherPrt->contactPartners_.size())
			{
				otherPrt->calcCollisionLoad();
				otherPrt->calcAcceleration();
				otherPrt->calcVelocity(currentRelax);
				otherKinEn = otherPrt->getKineticEnergy();
				otherPrt->subCyclingRestoreState();
			}
			else
			{
				otherKinEn = otherPrt->getKineticEnergyOfAgglomerate(currentRelax, true);
				otherPrt->subCyclingRestoreState();
			}

			// New kinetic energy after the collision
			scalar newKinEn = firstKinEn.z() + otherKinEn.z();

			scalar ratioKinEn = oldKinEn / ( newKinEn + VSMALL ); // energyloss * (should / is)
			scalar ratioKinEnLast = ratioKinEn;
			scalar lowestFactor = 0;
			scalar highestFactor = 1000;
			scalar lastIterFactor = (lowestFactor + highestFactor) / 2.0;
			scalar newFactor = (lowestFactor + highestFactor) / 2.0;
			int breakCounter = 1;

			Vector2D<scalar> bestRatioSet, secondBestRatioSet; // Safes collision data with best kin.en. conservation
			bestRatioSet.x() = VGREAT;
			secondBestRatioSet.x() = VGREAT;

			// Adjust the precision of the kinetic enrgy loop
			// according to kinEnPrecision entry in dictionary
			scalar kinEnPrecLimitHigh = 1 + kinEnPrecision_;
			scalar kinEnPrecLimitLow  = 1 - kinEnPrecision_;
			// Main loop for force adjustment based on kin. energy
			do
			{
#if 0
			_DBO_("Kinetic energy stats for collision:"
			<< "\n Particles' kinEn before: " << oldKinEn
			<< "\n newKinEn:" << newKinEn
			<< "\n ratioKinEn with conservation factor: " << ratioKinEn
			<< "\n ratioKinEnLast = " << ratioKinEnLast
			<< "\n lastIterFactor = " << lastIterFactor
			<< "\n massFirst = " << firstPrt->getMass()
			<< "\n accelFirst = " << firstPrt->getAcceleration()
			<< "\n forceFirst = " << firstPrt->getTotalForce()
			<< "\nkinEn firstPrt = " << firstKinEn
			<< "\niterVeloFirstPrt = " << firstPrtIterVelo
			//<< "\niterOmegaFirstPrt = " << firstPrt->getOmega()
			<< "\n massOther = " << otherPrt->getMass()
			<< "\n accelOther = " << otherPrt->getAcceleration()
			<< "\n forceOther = " << otherPrt->getTotalForce()
			<< "\nkinEn otherPrt = " << otherKinEn
			<< "\niterVeloOtherPrt = " << otherPrtIterVelo
			//<< "\niterOmegaOtherPrt = " << otherPrt->getOmega()
			<< "\nnewKinEn - oldKinEn * conserv = " << (newKinEn - oldKinEn)
			//<< "\n lowestFactor = " << lowestFactor
			//<< "\n highestFactor = " << highestFactor

			)
#endif

				// kinEnLoopsMax_ can be defined in the dictionary
			    if(breakCounter > kinEnLoopsMax_) break;
				if(breakCounter == kinEnLoopsMax_ && ratioKinEnLast <= 0.5) newFactor = 0; // Smooths resting contacts

				// Correct collision forces
				for(int i = 0; i < maxListIter; i++)
				{
					if(i < reducedList1.size() && !(firstPrt->myPop_->isStructure()))
					{
						int elem_id1 = reducedList1[i];
						firstPrt->solidForceField()[elem_id1] = (-1) * newFactor * collForceTot * mag(firstPrt->Sf()[elem_id1]) * avgFaceNormal1 / (collisionArea1 + VSMALL);
					}
					if(i < reducedList2.size())
					{
						int elem_id2 = reducedList2[i];
						otherPrt->solidForceField()[elem_id2] = (-1) * newFactor * collForceTot * mag(otherPrt->Sf()[elem_id2]) * (-1 * avgFaceNormal1) / (collisionArea2 + VSMALL);
					}
				}
				// Calculate new kinetic energy of collision system
				// with adjusted correction factor
				scalar lastIterFactorTemp = newFactor;

				if(firstPrt->myPop_->isStructure()) firstKinEn = vector::zero;
				else if(!firstPrt->contactPartners_.size())
				{
					firstPrt->calcCollisionLoad();
					firstPrt->calcAcceleration();
					firstPrt->calcVelocity(currentRelax);
					firstPrtIterVelo = firstPrt->getVelocity();
					firstKinEn = firstPrt->getKineticEnergy();
					firstPrt->subCyclingRestoreState();
				}
				else
				{
					firstKinEn = firstPrt->getKineticEnergyOfAgglomerate(currentRelax, true);
					firstPrtIterVelo = firstPrt->getVelocity();
					firstPrt->subCyclingRestoreState();
				}

				if(otherPrt->myPop_->isStructure()) otherKinEn = vector::zero;
				else if(!otherPrt->contactPartners_.size())
				{
					otherPrt->calcCollisionLoad();
					otherPrt->calcAcceleration();
					otherPrt->calcVelocity(currentRelax);
					otherPrtIterVelo = otherPrt->getVelocity();
					otherKinEn = otherPrt->getKineticEnergy();
					otherPrt->subCyclingRestoreState();
				}
				else
				{
					otherKinEn = otherPrt->getKineticEnergyOfAgglomerate(currentRelax, true);
					otherPrtIterVelo = otherPrt->getVelocity();
					otherPrt->subCyclingRestoreState();
				}

				newKinEn = firstKinEn.z() + otherKinEn.z();

				ratioKinEn = oldKinEn / ( newKinEn + VSMALL );

				if( mag(1 - ratioKinEn) <= mag(1 - bestRatioSet.x()) )
				{
					secondBestRatioSet.x() = bestRatioSet.x();
					secondBestRatioSet.y() = bestRatioSet.y();
					bestRatioSet.x() = ratioKinEn;
					bestRatioSet.y() = newFactor;
				}
				else if( mag(1 - ratioKinEn) <= mag(1 - secondBestRatioSet.x()) )
				{
					secondBestRatioSet.x() = ratioKinEn;
					secondBestRatioSet.y() = newFactor;
				}

				// Adjust the correction factor based on the
				// ratio and change of ratio compared to the
				// last step of the iterative process
				lastIterFactor = newFactor;
				if(oldKinEn - newKinEn < 0)
				{
					highestFactor = newFactor;
					newFactor = (lowestFactor + newFactor) / 2.0;
				}
				else if(oldKinEn - newKinEn > 0)
				{
					lowestFactor = newFactor;
					newFactor = (highestFactor + newFactor) / 2.0;
				}
				ratioKinEnLast = ratioKinEn;
				breakCounter++;
			} while( !(kinEnPrecLimitLow < ratioKinEnLast && ratioKinEnLast < kinEnPrecLimitHigh));
#if	0
			if(breakCounter >= kinEnLoopsMax_ || true)
				{	_DBO_("WARNING! WARNING! WARNING!\nLoop for kinetic energy conservation during collision did NOT converge after kinEnLoopsMax = " << breakCounter << " loops!!!"
					<< "\nfirstPrt = " << firstPrt->idStr()
					<< "\notherPrt = " << otherPrt->idStr()
					<< "\nfinalVeloFirstPrt = " << firstPrtIterVelo
					<< "\nfinalOmegaFirstPrt = " << firstPrt->getOmega()
					<< "\nkinEn firstPrt = " << firstKinEn
					<< "\nfinalVeloOtherPrt = " << otherPrtIterVelo
					<< "\nfinalOmegaOtherPrt = " << otherPrt->getOmega()
					<< "\nkinEn otherPrt = " << otherKinEn
					<< "\nfinal kinEnRatio = " << ratioKinEn << " for " << otherPrt->idStr()
					<< "\nnewFactor = " << newFactor
					<< "\nsum(firstPrt->solidForceField()) = " << sum(firstPrt->solidForceField())
					<< "\nsum(otherPrt->solidForceField()) = " << sum(otherPrt->solidForceField())
					<< "\nsum(otherPrt->fluidForceField()) = " << sum(otherPrt->fluidForceField()) )
				}
#endif
			// Kamil: Habe kurz Output für die Diss gebraucht
			/*vector e_1(1, 0, 0);
			vector refPoint  = vector::zero;
			vector groundAng = firstPrt->getCg() ^ (firstPrt->getVelocity() * firstPrt->getMass());
			groundAng       += otherPrt->getCg() ^ (otherPrt->getVelocity() * otherPrt->getMass());
			vector ownAng    = firstPrt->getJ() & firstPrt->getOmega();
			ownAng          += otherPrt->getJ() & otherPrt->getOmega();
			vector momVec    = firstPrt->getMass() * firstPrt->getVelocity() + otherPrt->getMass() * otherPrt->getVelocity();
			scalar momVecMag = mag(momVec);
			scalar alpha     = acos((e_1 & momVec) / (momVecMag+VSMALL));
			_DBO_("alpha is " << alpha << "\nmomVecMag is " << momVecMag << "\ngroundAng is " << mag(groundAng) << "\nownAng is " << mag(ownAng))*/
		//
		//
		// End of kin.En. adjustment
		//////////////////////////////////////////////////////

			// Integrate current forces to predict
			// if particles will stay together or separate
			scalar    dt    = time_.deltaT().value();
			double moveRelax;
			if(firstPrt->myPop_->collForceConservation() > 0.5) moveRelax = 2 * currentRelax * (1 / firstPrt->myPop_->collForceConservation());
			else moveRelax = 2 * currentRelax;
			if(!firstPrt->myPop_->isStructure())
			{
				firstPrt->subCyclingRestoreState();
				if(!firstPrt->contactPartners_.size())
				{
					firstPrt->calcTotalLoad();
					firstPrt->calcAcceleration();
					firstPrt->calcVelocity(currentRelax);
					//TESTfirstPrt->subCyclingSaveVelocities();
					firstPrt->getRotNext()   = dt * moveRelax * firstPrt->getOmega();
					firstPrt->getDisplNext() = dt * moveRelax * firstPrt->getVelocity();
					firstPrt->kinetic();
				}
				else
				{
					firstPrt->moveWithContactPartners(currentRelax);
					firstPrt->movedWithContactPartners_ = false;
					forAllIter(List<volumetricParticle*>, firstPrt->contactPartners_, partner)
					{
						(*partner)->movedWithContactPartners_ = false;
						//TEST(*partner)->subCyclingSaveVelocities();
						(*partner)->subCyclingRestoreState();
					}
				}
			}

			otherPrt->subCyclingRestoreState();
			if(!otherPrt->contactPartners_.size())
			{
				otherPrt->calcTotalLoad();
				otherPrt->calcAcceleration();
				otherPrt->calcVelocity(currentRelax);
				//TESTotherPrt->subCyclingSaveVelocities();
				otherPrt->getRotNext()   = dt * moveRelax * otherPrt->getOmega();
				otherPrt->getDisplNext() = dt * moveRelax * otherPrt->getVelocity();
				otherPrt->kinetic();
			}
			else
			{
				otherPrt->moveWithContactPartners(currentRelax);
				otherPrt->movedWithContactPartners_ = false;
				forAllIter(List<volumetricParticle*>, otherPrt->contactPartners_, partner)
				{
					(*partner)->movedWithContactPartners_ = false;
					//TEST(*partner)->subCyclingSaveVelocities();
					(*partner)->subCyclingRestoreState();
				}
			}

			// Re-entrainment due to collision
			//firstPrt->subtractContactsFromTotalLoad();
			reassignContactPartners(*firstPrt, breakAgglomeratesIterations_);
			//otherPrt->subtractContactsFromTotalLoad();
			reassignContactPartners(*otherPrt, breakAgglomeratesIterations_);
#if 0
				_PDBOP_("Data after integrating all forces post collision:\n" << firstPrt->idStr() << " is at " << firstPrt->getCg()
						<< "\n" << otherPrt->idStr() << "is at" << otherPrt->getCg()
						, 0)
#endif
			collisionCheckVer2(*firstPrt, *otherPrt);
		} // ---------------------------------------------------- end of collForceTot != 0 if-clause


			// If a rebound happens, particles should loose some velocity due to
			// adhesive forces acting during the collision. In that case superpose the adhesive force
			// on the collision force and integrate for new velocity. Adhesive forces are deleted later
			// inside pManager::moveSolids() (which also calls this method)
			vector reboundVelocity1 = firstPrt->getVelocity();
			vector reboundVelocity2 = otherPrt->getVelocity();
			scalar    dt    = time_.deltaT().value();
			firstPrt->subCyclingRestoreState();
			otherPrt->subCyclingRestoreState();
#if 0
			_DBO_("reboundVelocity1 = " << reboundVelocity1 << "\n"
					"reboundVelocity2) = " << reboundVelocity2  << "\n"
					"avgFaceNormal1 = " << avgFaceNormal1 << "\n"
					"avgFaceNormal2 = " << avgFaceNormal2)
#endif
			//if((avgFaceNormal1 & reboundVelocity2) > 0)
			////////////////////////////////////////////
			// Start of energy loss from adhesive forces
			//
/*
			if(!RAPID_num_contacts)
			{
				for(int i = 0; i < maxListIter; i++)
				{
					if(i < reducedList1.size() && !(firstPrt->myPop_->isStructure()))
					{
						int elem_id1 = reducedList1[i];
						firstPrt->solidForceField()[elem_id1] += firstPrt->contactForceField()[elem_id1];
					}
					if(i < reducedList2.size())
					{
						int elem_id2 = reducedList2[i];
						otherPrt->solidForceField()[elem_id2] += otherPrt->contactForceField()[elem_id2];
					}
				}
				// Calculate the velocities from collision forces (impact + adhesion)
				// for both particles
				vector collForce  = sum(otherPrt->solidForceField());

				scalar reducedMass = otherPrt->getMass();
				scalar reducedR   = 3.445e-06;//1.29e-6;
				scalar nu1        = 0.31;
				scalar nu2        = 0.3;//0.33;
				scalar E1         = 1.2e9;
				scalar E2         = 329;//2.1e9;
				scalar reducedE   = 1.0 / ( (1.0 - nu1*nu1)/E1 + (1.0 - nu2*nu2)/E2 );
				double expTerm    = std::pow( (reducedMass * reducedMass)/(reducedE * reducedE * reducedR * mag(otherPrt->getVelocity())), 1./5.);
				scalar collTime   = 2.868 * expTerm;
				scalar etaSolid   = 1; // Kork
				scalar corrTerm   = 1.0 ;
				//_DBO_("KTerm = " << KTerm << "\ncolltime = " << (collTime * corrTerm) << "\ndt = " << dt << "\tcurrentRelax = " << currentRelax << "\treducedE = " << reducedE << "\texpTerm = " << expTerm << "\tmass = " << reducedMass)
				vector adhForce   = sum(otherPrt->contactForceField()) * (corrTerm * collTime/ (dt * currentRelax));

				vector collTorque = sum( (otherPrt->Cf() - otherPrt->getCg()) ^ otherPrt->solidForceField());
				vector collAcc    = (collForce + adhForce) / otherPrt->getMass();
				symmTensor Jinv(inv(otherPrt->getJ()));
				vector collOmegaA =  Jinv & ( collTorque - ( otherPrt->getOmega() ^ ( otherPrt->getJ() & otherPrt->getOmega() ) ) ) ;

				otherPrt->getVelocity() += dt * currentRelax * collAcc;
				otherPrt->getOmega()    += dt * currentRelax * collOmegaA;
				otherPrt->subCyclingSaveVelocities();

				if(!firstPrt->myPop_->isStructure())
				{
					collForce  = sum(firstPrt->solidForceField());
					collTorque = sum( (firstPrt->Cf() - firstPrt->getCg()) ^ firstPrt->solidForceField());
					collAcc    = collForce / firstPrt->getMass();
					Jinv = inv(firstPrt->getJ());
					collOmegaA =  Jinv & ( collTorque - ( firstPrt->getOmega() ^ ( firstPrt->getJ() & firstPrt->getOmega() ) ) ) ;

					firstPrt->getVelocity() += dt * currentRelax * collAcc;
					firstPrt->getOmega()    += dt * currentRelax * collOmegaA;
					firstPrt->subCyclingSaveVelocities();
				}

				// Clear corresponding position in fields to to prevent double integration
				for(int i = 0; i < maxListIter; i++)
				{
					if(i < reducedList1.size() && !(firstPrt->myPop_->isStructure()))
					{
						int elem_id1 = reducedList1[i];
						firstPrt->solidForceField()[elem_id1] = vector::zero;
					}
					if(i < reducedList2.size())
					{
						int elem_id2 = reducedList2[i];
						otherPrt->solidForceField()[elem_id2] = vector::zero;
					}
				}
			}

			// If rebounding, move to position slightly before collision
			//if((avgFaceNormal1 & reboundVelocity2) > 0)
			if(!RAPID_num_contacts)
			{
				otherPrt->subCyclingRestorePreCollision();
				if(!firstPrt->myPop_->isStructure()) firstPrt->subCyclingRestorePreCollision();
			}
			//
			// End of energy loss from adhesive forces
			//////////////////////////////////////////

			// The following part is only for maintaining agglomeration and
			// re-entrainement, so if the particle doesn't consider adhesion
			// end the method here.
 */
			if(!otherPrt->myPop_->H() || !firstPrt->myPop_->H()) continue;

			double particlesAdhered = false;

			// previously !RAPID_num_contacts || (firstPrt->myPop_->isStructure() && !(otherPrt->myPop_->H()) &&  (avgFaceNormal1 & reboundVelocity2) > 0)
			if(!RAPID_num_contacts && (firstPrt->myPop_->collForceConservation() != 0) && (otherPrt->myPop_->collForceConservation() != 0))/* ||
					(firstPrt->myPop_->isStructure() &&  (avgFaceNormal1 & reboundVelocity2) > 0)
					) // Objects don't collide anymore*/
			{
#if 0
				_PDBOP_("RAPID_NUM_CONTACTS = " << RAPID_num_contacts << " ==> NO COLLISION"
						<< "\n" << firstPrt->idStr() << " is at " << firstPrt->getCg()
						<< "\n" << otherPrt->idStr() << "is at" << otherPrt->getCg()
						, 0)
#endif
			}
			else // Objects still collide and should be considered an agglomerate
			{
				if(firstPrt->myPop_->isStructure()) otherPrt->setContactPointsWithStructure( reducedList2, avgFaceNormal2, avgFaceCenter2 );

				for(int i = 0; i <= otherPrt->contactPartners_.size(); i++)
				{
					if(otherPrt->contactPartners_.size() == 0) // First partner
					{
						/*_PDBOP_("Appending first partner for particle at " << otherPrt->getCg()
								<< "with the ID " << otherPrt->idStr(), 0)*/
						particlesAdhered = true;
						otherPrt->contactPartners_.append(firstPrt);
						otherPrt->contactNormals_.append(avgFaceNormal2);
						otherPrt->contactVectors_.append(-1*contactVector);
						otherPrt->contactFaces_.append(otherCollidingFaces);
						firstPrt->contactPartners_.append(otherPrt);
						firstPrt->contactVectors_.append(contactVector);
						firstPrt->contactNormals_.append(avgFaceNormal1);
						firstPrt->contactFaces_.append(firstCollidingFaces);
						break;
					}
					else if(otherPrt->contactPartners_[i]->getCg() == firstPrt->getCg()) // Already partners
					{
						break;
					}
					else if(i == (otherPrt->contactPartners_.size() - 1))
					{
						particlesAdhered = true;
						otherPrt->contactPartners_.append(firstPrt);
						otherPrt->contactVectors_.append(-1*contactVector);
						otherPrt->contactNormals_.append(avgFaceNormal2);
						otherPrt->contactFaces_.append(otherCollidingFaces);
						firstPrt->contactPartners_.append(otherPrt);
						firstPrt->contactVectors_.append(contactVector);
						firstPrt->contactNormals_.append(avgFaceNormal1);
						firstPrt->contactFaces_.append(firstCollidingFaces);
						break;
					}
				}
			}


			// Clear collision forces if objects adhere to each other (otherwise no momentum conservation)
			// and apply restitution forces. Default value for restitutionForce is zero!
			if(particlesAdhered)
			{
				_DBO_("FirstAdherer+" << firstPrt->idStr() <<"\tSecondAdherer+" <<otherPrt->idStr())

				firstPrt->calcAggloJ(); // Calculate and save new moment of inertia tensor of new agglomerate

				for(int i = 0; i < RAPID_num_contacts; i++)
				{
					if(i < reducedList1.size() && !(firstPrt->myPop_->isStructure()))
					{
						int elem_id1 = reducedList1[i];
						firstPrt->solidForceField()[elem_id1] = vector::zero;
					}
					if(i < reducedList2.size())
					{
						int elem_id2 = reducedList2[i];
						otherPrt->solidForceField()[elem_id2] = vector::zero;
					}
				}
			}


#if 0
		_DBO_("\n --- Writing into partner list ---"
				<< "\nmy ID: " << firstPrt->idStr()
				<<"\npartner ID: " << otherPrt->idStr()
				<<"\ncontactFaces: " << firstPrt->contactFaces_
				<<"\ncontactVectors: " << firstPrt->contactVectors_
				<<"\ncollidingFaces: " << RAPID_num_contacts
				<<"\n\t--- Lists ---"
				<<"\n\t contactPartners: " << firstPrt->contactPartners_
				<<"\n\t contactFaces: " << firstPrt->contactFaces_
				<<"\n\t contactVectors: " << firstPrt->contactVectors_
				<<"\n\t-------------"
				<<"\n\t--- Kinetics ---"
				<<"\n\t firstPrt->getVelocity() = " << firstPrt->getVelocity()
				<<"\n\t otherPrt->getVelocity() = " << otherPrt->getVelocity()
				<<"\n\t-------------"
				<<"\n ---------------------------------")
#endif
	}
	if(m1Created) delete m1;
	}
}

void Foam::functionObjects::pManager::integrateForcesDuringCollision(volumetricParticle* firstPrt, volumetricParticle* otherPrt, List<int> reducedList1, List<int> reducedList2)
{
	return;
}

// Moves elements of an agglomerate individually to check
// whether some parts will break free due to present forces
void Foam::functionObjects::pManager::breakAgglomerates(scalar currentRelax)
{
	scalar    dt    = time_.deltaT().value();


	forAllIter(List<volumetricParticle*>, particleList_, prt)
	{
		if( !(*prt)->contactPartners_.size() || (*prt)->myPop_->isStructure()) continue;
		(*prt)->saveIntermediateState();
		(*prt)->calcTotalLoad();
		(*prt)->subtractContactsFromTotalLoad();
	}

	forAllIter(List<volumetricParticle*>, particleList_, prt)
	{
		if( !(*prt)->contactPartners_.size() || (*prt)->myPop_->isStructure()) continue;
		reassignContactPartners(**prt, breakAgglomeratesIterations_);
		//reassignContactPartners(**prt);
	}

	forAllIter(List<volumetricParticle*>, particleList_, prt)
	{
		if( !(*prt)->contactPartners_.size() || (*prt)->myPop_->isStructure()) continue;
		(*prt)->restoreIntermediateState();
	}

	/*for(int i = 0; i < particleList_.size(); i++)
	{
		volumetricParticle* pPtr = particleList_[i];


		pPtr->saveIntermediateState();

		pPtr->calcTotalLoad();
		pPtr->calcAcceleration();
		pPtr->calcVelocity(currentRelax);
		pPtr->getRotNext()   = dt * currentRelax * pPtr->getOmega();
		pPtr->getDisplNext() = dt * currentRelax * pPtr->getVelocity();
#if 0
		_DBO_("Particle: " << pPtr->idStr() << " Prediction Running with integration over full timestep!!!!!"
				<<"\nTotal Force = " << pPtr->getTotalForce()
				<<"\nAdhesion Force = " << sum(pPtr->contactForceField())
				<<"\Fluid Force = " << sum(pPtr->fluidForceField()) )
#endif
		pPtr->kinetic();

		reassignContactPartners(*pPtr);
		pPtr->restoreIntermediateState();
	}*/

	return;
}


// Loops over all contacts of all particles and adds adhesive forces applying a
// translation along the contact normal.
void Foam::functionObjects::pManager::calcAdhesiveForcesBetweenContactPartners()
{
	for(int i = 0; i < particleList_.size(); i++)
	{
		volumetricParticle* firstPrt = particleList_[i];

		// Reduce contactPartners_ to actual partners as the list might contain
		// non-direct partners from the agglomerate used in the last time step
		firstPrt->contactPartners_.setSize(firstPrt->contactVectors_.size());
		// Does particle have no partner?
		if(firstPrt->contactPartners_.size() == 0) continue;

		// Calculate adhesive forces over list of all partners
		forAll(firstPrt->contactPartners_, otherIter)
		{
			volumetricParticle* otherPrt = firstPrt->contactPartners_[otherIter];
			if(firstPrt->idStr() == otherPrt->idStr()) continue;
			vector contactVector = firstPrt->contactVectors_[otherIter];
			volumetricParticle::facePair collidingFaces = firstPrt->contactFaces_[otherIter];

			integrateAdhesion(*firstPrt, *otherPrt, contactVector, collidingFaces.elem_id1, collidingFaces.elem_id2);
		}
	}
}

// General function selector for the adhesion integration
void Foam::functionObjects::pManager::integrateAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector, int elem_id1 = -1, int elem_id2 = -1 )
{

	if(adhesionIntegrationType_ == "general") integrateAdhesionGeneral(firstPrt, otherPrt, contactVector);
	else if(adhesionIntegrationType_ == "flatContact") integrateAdhesionForFlatContact(firstPrt, otherPrt, contactVector);
	else if(adhesionIntegrationType_ == "triangleIntegration") integrateAdhesionTriangleIntegration(firstPrt, otherPrt, contactVector, elem_id1, elem_id2);
	else
	{
		FatalErrorIn
			(
			    "pManager::integrateAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector )"
			)   << "Unknown integration type " << adhesionIntegrationType_ << nl
			    << "Valid integration types are :" << nl << "general" << nl << "flatContact" << endl
			    << exit(FatalIOError);
	}
}

void Foam::functionObjects::pManager::integrateAdhesionTriangleIntegration(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector, int elem_id1, int elem_id2 )
{
	vector cTest = contactVector;
	vector newContactVector = vector::zero;
	//vector newContactVector = vector::zero;

	scalar contactDist = mag(contactVector);

	//get wall coordinates
	const vectorField firstPoints = firstPrt.triSurf().points();//.points();	//maybe we dont need a full copy here
	const vectorField firstCf = firstPrt.Cf();//facecenter
	const vectorField firstSf = firstPrt.Sf();
	const vectorField firstNormals = firstPrt.normals();

	const Foam::List<Foam::labelledTri> firstLocalFaces = firstPrt.triSurf();//.localFaces();
	vectorField &firstForceField = firstPrt.contactForceField();

	//get Particle data
	const vectorField otherPoints = otherPrt.triSurf().points();
	const vectorField otherCf = otherPrt.Cf();
	const vectorField otherSf = otherPrt.Sf();
	const vectorField otherNormals = otherPrt.normals();

	const Foam::List<Foam::labelledTri> otherLocalFaces = otherPrt.triSurf();//.localFaces();
	vectorField &otherForceField = otherPrt.contactForceField();

	// Find edge point deepest behind contact plane
	vector contactEdgePoint = vector::zero;
	for(int i = 0 ; i < 3 ; i++ ){
		vector edgePoint = otherPoints[(otherLocalFaces[elem_id2]) [i]];
		if( ( (edgePoint - firstCf[elem_id1]) & firstNormals[elem_id1] ) <  0
				&& mag( (edgePoint - firstCf[elem_id1]) & firstNormals[elem_id1] ) > mag(newContactVector))
		{
			newContactVector = ((edgePoint - firstCf[elem_id1]) & firstNormals[elem_id1]) * firstNormals[elem_id1];
			contactDist = mag(newContactVector);
			contactEdgePoint = edgePoint;
		}
	}

	scalar a0 = firstPrt.myPop_->a0();
	newContactVector = newContactVector - firstNormals[elem_id1] * a0;


	scalar integrateForce = 0;
	scalar ha, hb, hc, Aproj, hamaker, lenOfNormal, force;
	hamaker = sqrt( firstPrt.myPop_->H() * otherPrt.myPop_->H() );

	vector vectorA = vector::zero;
	vector vectorB = vector::zero;
	vector vectorC = vector::zero;
	vector projectedA = vector::zero;
	vector projectedB = vector::zero;
	vector projectedC = vector::zero;
	vector myForce = vector::zero;
	vector normalOfHalfSpace = vector::zero;
	vector pointOnPlane = vector::zero;

	/*
	 * get wall normal
	 * nearest element combination is elem_1 elem_2
	 * so elem_1 is nearest wall element
	 * that is interpreted as the surface of infinite half space
	 */


	normalOfHalfSpace = firstNormals[elem_id1];
	//"moving" plane so it is in contactDistance a0
	//pointOnPlane = firstCf[elem_id1] - newContactVector;
	pointOnPlane = firstCf[elem_id1];


	forAll(otherCf, faceNumber){
		vectorA = otherPoints[(otherLocalFaces[faceNumber]) [0]];
		vectorB = otherPoints[(otherLocalFaces[faceNumber]) [1]];
		vectorC = otherPoints[(otherLocalFaces[faceNumber]) [2]];
		//Aproj = std::fabs((normalOfHalfSpace & otherSf[faceNumber]));
		Aproj = mag((normalOfHalfSpace & otherSf[faceNumber]));

		//get height of trianglepoints with scalarprodukt
		//ha = (vectorA - pointOnPlane - (normalOfHalfSpace * a0)) & normalOfHalfSpace;// / mag(normalOfHalfSpace);
		//hb = (vectorB - pointOnPlane - (normalOfHalfSpace * a0)) & normalOfHalfSpace;// / mag(normalOfHalfSpace);
		//hc = (vectorC - pointOnPlane - (normalOfHalfSpace * a0)) & normalOfHalfSpace;// / mag(normalOfHalfSpace);

		ha = mag((vectorA - contactEdgePoint) & normalOfHalfSpace) + a0;
		hb = mag((vectorB - contactEdgePoint) & normalOfHalfSpace) + a0;
		hc = mag((vectorC - contactEdgePoint) & normalOfHalfSpace) + a0;


		//if(ha < 0 || hb < 0 || hc < 0) _DBO_(" A = " << vectorA << " B = " << vectorB << " C = " << vectorC)

		force           = hamaker / (3.0 * constant::mathematical::twoPi) / (ha * hb * hc) * Aproj;
		//force           = (hamaker / (3.0 * constant::mathematical::twoPi)) * (Aproj / (ha * hb * hc));
		integrateForce += force;

		otherForceField[faceNumber] += force * otherNormals[faceNumber];
	}

	_DBO_("COLLISION HAPPENED with pointOnPlane = " << pointOnPlane << " and adhesive force = " << integrateForce);
    //FatalErrorIn("EXITING") << exit(FatalError);

}


void Foam::functionObjects::pManager::integrateAdhesionGeneral(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector )
{

	//if(!firstPrt.myPop_->isStructure()) return;
	double hamaker, force, force1, force2, forceReductionFactor1, forceReductionFactor2;
	hamaker = sqrt( firstPrt.myPop_->H() * otherPrt.myPop_->H() );

	forceReductionFactor1 = firstPrt.myPop_->adhesionReductionFactor();
	forceReductionFactor2 = otherPrt.myPop_->adhesionReductionFactor();


	double contactDist = mag(contactVector);
	contactVector /= contactDist + VSMALL; // normalized vector pointing in contact direction
	contactDist += firstPrt.myPop_->a0(); // depth of contact + literature contact distance (often 4 Angström)
	contactVector *= contactDist;
	pointField r = firstPrt.triSurf().points();
	r += contactVector;
	firstPrt.triSurf().movePoints(r);

	const vectorField firstCf = firstPrt.Cf();
	const vectorField firstSf = firstPrt.Sf();
	const vectorField firstNormals = firstPrt.normals();
	vectorField &firstForceField = firstPrt.contactForceField();

	const vectorField otherCf = otherPrt.Cf();
	const vectorField otherSf = otherPrt.Sf();
	const vectorField otherNormals = otherPrt.normals();
	vectorField &otherForceField = otherPrt.contactForceField();

	//_DBO_("Calculating adhesive forces for " << firstPrt.idStr() << " and " << otherPrt.idStr() << " with contactVector " << contactVector)
	//firstPrt.printParticleData();
	//otherPrt.printParticleData();

	vector avgNormals = vector::zero;
	vector avgNormalsWeighted = vector::zero;
	double distance, factor, angle;
	vector distanceVec;

	forAll(firstCf, firstIter)
	{
		force  = 0;
		force1 = 0;
		force2 = 0;

		forAll(otherCf, otherIter)
		{
			distance = mag(firstCf[firstIter] - otherCf[otherIter]);
			if(distance > 10e-6) continue; // Hard coded cut-off distance of 1e-7m; complexity increases in O(n²)

			angle    = (firstNormals[firstIter] & otherNormals[otherIter]) / (mag(firstNormals[firstIter]) * mag(otherNormals[otherIter])) ;

			//if(!((angle < -0.5) && (angle > -1.1))) continue; // angle has to be between between 110° and 250°

			factor   = 0.5 * hamaker / ( distance * distance * distance * 3 * constant::mathematical::twoPi + VSMALL ); // Factor of 0.5 because every particle enters this function twice ...
			force = factor * ( mag(firstSf[firstIter]) +  mag(otherSf[otherIter]) ) / 2.0;
			force1 = factor * mag(firstSf[firstIter]);
			force2 = factor * mag(otherSf[otherIter]);
			firstForceField[firstIter] += force1 * firstNormals[firstIter] / forceReductionFactor1;
			otherForceField[otherIter] += force2 * otherNormals[otherIter] / forceReductionFactor2;
		}
	}

	_PDBOP_("COLLISION HAPPENED!!! Adhesive force on " << firstPrt.idStr() << " in contact with " << otherPrt.idStr() << " with contactVector " << contactVector << " is " << sum(firstForceField)
			<<"\n and the other way around the force is " << sum(otherForceField) << " with mag = " << mag(sum(otherForceField)), 0)

	FatalErrorIn("EXITING") << exit(FatalError);


	r -= contactVector;
	firstPrt.triSurf().movePoints(r);
	/*firstPrt.getDisplNext() *= -1;
	firstPrt.getRotNext() *= -1;
	firstPrt.kinetic();*/
}


void Foam::functionObjects::pManager::integrateAdhesionForFlatContact(volumetricParticle& firstPrt, volumetricParticle& otherPrt, vector contactVector )
{

	double hamaker, force;
	hamaker = sqrt( firstPrt.myPop_->H() * otherPrt.myPop_->H() );
	pointField r = firstPrt.triSurf().points();

	scalar forceReductionFactor1 = firstPrt.myPop_->adhesionReductionFactor();
	scalar forceReductionFactor2 = otherPrt.myPop_->adhesionReductionFactor();

	//--------------------------------------------------------------------
	_DBO_("Watch out! This is currently hard-coded!")
	if(!firstPrt.myPop_->isStructure()) return;
	contactVector = vector(0, 0, 1);
	if(firstPrt.myPop_->isStructure()) contactVector *= -1;
	int lowestFace, highestFace;
	double lowestZ = VGREAT;
	double highestZ = -1 * VGREAT;
	forAll(otherPrt.Cf(), otherIter)
	{
		if((otherPrt.Cf()[otherIter]).z() > highestZ)
		{
			highestZ    = (otherPrt.Cf()[otherIter]).z();
			highestFace = otherIter;
		}
	}
	forAll(firstPrt.Cf(), firstIter)
	{
		if((firstPrt.Cf()[firstIter]).z() < lowestZ)
		{
			lowestZ    = (firstPrt.Cf()[firstIter]).z();
			lowestFace = firstIter;
		}
	}
	_DBO_("highestFaceCf = " << otherPrt.Cf()[highestFace] << "\tlowestFaceCf = " << firstPrt.Cf()[lowestFace])
	_DBO_("highestZ - lowestZ = " << (highestZ - lowestZ))
	_DBO_("a0 = " << firstPrt.myPop_->a0())
	contactVector *= (highestZ - lowestZ) - firstPrt.myPop_->a0();
	_DBO_("contactVector = " << contactVector)
	if((highestZ - lowestZ) > 0) contactVector *= -1;
	r += contactVector;
	firstPrt.triSurf().movePoints(r);

	_DBO_("highestFace - lowestFace = " << (otherPrt.Cf()[highestFace] - firstPrt.Cf()[lowestFace]))
	//--------------------------------------------------------------------

	const vectorField firstCf = firstPrt.Cf();
	const vectorField firstSf = firstPrt.Sf();
	const vectorField firstNormals = firstPrt.normals();
	vectorField &firstForceField = firstPrt.contactForceField();
	firstForceField = vector::zero;

	const vectorField otherCf = otherPrt.Cf();
	const vectorField otherSf = otherPrt.Sf();
	const vectorField otherNormals = otherPrt.normals();
	vectorField &otherForceField = otherPrt.contactForceField();
	otherForceField = vector::zero;

	vector avgNormals = vector::zero;
	vector avgNormalsWeighted = vector::zero;
	double distance, factor, angle;
	vector distanceVec;



	forAll(firstCf, firstIter)
	{
		force = 0;

		forAll(otherCf, otherIter)
		{
			distance = mag((firstCf[firstIter] - otherCf[otherIter]) & vector(0, 0, 1));
			if(distance > 10e-6) continue; // Hard coded cut-off distance of 1e-7m; complexity increases in O(n²)
			distanceVec = otherCf[otherIter] - firstCf[firstIter]; //test
			distanceVec = distanceVec - (distanceVec & vector(0, 0, -1)) * vector(0, 0, -1); //test
			if(mag(distanceVec) > (0.5 * sqrt(2 * mag(firstSf[firstIter])) ) ) continue; //test

			//_DBO_("firstNormal = " << firstNormals[firstIter] << "\notherNormal = " << otherNormals[otherIter])

			angle    = (firstNormals[firstIter] & otherNormals[otherIter]) / (mag(firstNormals[firstIter]) * mag(otherNormals[otherIter])) ;

			//if(!((angle < -0.5) && (angle > -1.1))) continue; // angle has to be between between 110° and 250°
			//distance = firstPrt.myPop_->a0();

			factor   = 1 * hamaker / ( distance * distance * distance * 3 * constant::mathematical::twoPi + VSMALL ); // Factor of 0.5 because every particle enters this function twice ...
			force = factor * ( mag(firstSf[firstIter]) +  mag(otherSf[otherIter]) ) / 2.0;
			//_DBO_("at least i do something with angle = " << angle << " and dist = " << distance << "\nyielding force = " << force)
			firstForceField[firstIter] += force * firstNormals[firstIter] / (2 * forceReductionFactor1);
			otherForceField[otherIter] += force * otherNormals[otherIter] / (2 * forceReductionFactor2);
		}
	}

	_DBO_("Adhesive force on " << firstPrt.idStr() << " in contact with " << otherPrt.idStr() << " with contactVector " << contactVector << " is " << sum(firstForceField)
			<<"\n and the other way around the force is " << sum(otherForceField) << " with mag = " << mag(sum(otherForceField)))


	r -= contactVector;
	firstPrt.triSurf().movePoints(r);
}



// Integrates the Adhesive force over the STLs of both objects firstPrt and otherPrt.
// HOWEVER, forceResults are only written into the forceFields of firstPrt !!!
void Foam::functionObjects::pManager::integrateAdhesion(volumetricParticle& firstPrt, volumetricParticle& otherPrt )
{

	double hamaker, force;
	hamaker = firstPrt.myPop_->H();

	const vectorField firstCf = firstPrt.Cf();
	const vectorField firstSf = firstPrt.Sf();
	const vectorField firstNormals = firstPrt.normals();
	const vectorField otherCf = otherPrt.Cf();
	vectorField &firstForceField = firstPrt.contactForceField();

	_DBO_("ADHESIVE CALCULATION")


	forAll(firstCf, firstIter)
	{
		force = 0;

		forAll(otherCf, otherIter)
		{
			double distance = mag(firstCf[firstIter] - otherCf[otherIter]);
			if(distance > 1e-7) continue; // Hard coded cut-off distance of 1e-7m; complexity increases in O(n²)

			force = hamaker * mag(firstSf[firstIter]) / (3 * constant::mathematical::twoPi * distance * distance * distance + VSMALL) ;
			_DBO_("Adhesive force = " << force)
			firstForceField[firstIter] += force * firstNormals[firstIter];
		}
	}
}

// Takes the STLs of the volumetricParticles prt1 and prt2
// and merges them into one new STL for prt1 while deleting prt2.
// The new STL gets name of prt1's STL plus the suffix "m" for "modified".
// This is necessary as other particles of prt1's population still
// link to the original STL.
void Foam::functionObjects::pManager::mergeSTL(volumetricParticle *prt1, volumetricParticle *prt2) {

	triSurface tempSurf(prt1->triSurf());
	triSurface tempSurfPartner(prt2->triSurf());
	tempSurf.scalePoints(1 / prt1->scale_);
	tempSurfPartner.scalePoints(1 / prt2->scale_);

	/////// triSurf consists of points and list of labelledTri
	// The point fields can simply be merged together
	vectorField pointsField(tempSurf.points());
	int pointsFieldSize = pointsField.size();
	pointsField.append(tempSurfPartner.points());

	List<labelledTri> tempLabels  = static_cast<List<labelledTri>&> (tempSurfPartner);

	// Here the labelledTri get adjusted to reflect the positions
	// of the new points in the list
	forAll(tempLabels, i)
	   {
	    forAll(tempLabels[i], j)
	    {
	    	(tempLabels[i])[j] += pointsFieldSize;
	    }
	   }

	static_cast<List<labelledTri>&> (tempSurfPartner) = tempLabels;

	// Append the newly labelled List of labelledTri
	// and update to the new points
	_PDBO_("cg 1 : " << prt1->getCg())
	_PDBO_("cg 2 : " << prt2->getCg())
	tempSurf.append(tempSurfPartner);
	tempSurf.movePoints(pointsField);

	(prt1->meshPath_).replace(".stl", "m.stl");
	tempSurf.write(obr_.time().path()/prt1->meshPath_.name());
	/////// Here the merging of the two STLs is finished

	prt1->reloadSTL();
	_PDBO_("Resolving adhesion.")
	prt1->resolveAdhesion( *prt2 );

	_PDBO_("Deleting partner.")
	vector outside(-1000, -1000, -1000);
	prt2->getCg() = outside;

	//distributeParticles();
}

// Merges the STLs of particles which are in contact
// Needs a list of contact partners. Kept method for flexibility...
void Foam::functionObjects::pManager::mergeStl() {

	HashTable<volumetricParticle*> mergedParticles; // Contains particles that have already been merged

	forAll(particleList_, particleI)
	{
		volumetricParticle* particle = particleList_[particleI];
		if (particle->contactPartners_.size() < 1 || mergedParticles.found(particle->idStr())) continue;

		forAll(particle->contactPartners_, partnerI)
		{
			volumetricParticle* partner = particle->contactPartners_[partnerI];
			_PDBO_("--------------- appending")
			triSurface tempSurf(particle->triSurf());
			triSurface tempSurfPartner(partner->triSurf());
			tempSurf.scalePoints(1 /particle->scale_);
			//_PDBO_(">>>>>>>>> tempSurf: \n" << tempSurf)
			tempSurfPartner.scalePoints(1 / partner->scale_);
			//_PDBO_(">>>>>>>>> tempSurfPartner: \n" << tempSurfPartner)

			/////// triSurf consists of points and list of labelledTri
			// The point fields can simply be merged together
			vectorField pointsField(tempSurf.points());
			int pointsFieldSize = pointsField.size();
		    pointsField.append(tempSurfPartner.points());

		    List<labelledTri> tempLabels  = static_cast<List<labelledTri>&> (tempSurfPartner);

		    // Here the labelledTri get adjusted to reflect the positions
		    // of the new points in the list
		    forAll(tempLabels, i)
		    {
			    forAll(tempLabels[i], j)
			    {
			    	(tempLabels[i])[j] += pointsFieldSize;
			    }
		    }

		    static_cast<List<labelledTri>&> (tempSurfPartner) = tempLabels;

		    // Append the newly labelled List of labelledTri
		    // and update to the new points
		    tempSurf.append(tempSurfPartner);
		    tempSurf.movePoints(pointsField);
			tempSurf.write(obr_.time().path()/particle->meshPath_.name());
			/////// Here the merging of the two STLs is finished

			mergedParticles.insert(partner->idStr(), partner);
			particle->reloadSTL(); // Update the auto_pointers
			_PDBO_("---------------- done appending - now resolving Collision")
			particle->resolveAdhesion( (*partner) ); // Adjustments for momentum conservation
			_PDBO_("---------------- resolved collision")
		}

		particle->contactPartners_.clear();
	}

	_PDBO_("Going to delete partner particles")
	forAll(popList_, popI)
	{
		forAll(mergedParticles.toc(), particleI)
		{
			if (popList_[popI].findParticleByIdStr( mergedParticles.toc()[particleI] ))
			{
				//popList_[popI].deleteParticle(mergedParticles.toc()[particleI]);
				vector outside(-10, -10, -10);
				popList_[popI].findParticleByIdStr( mergedParticles.toc()[particleI] )->getCg() = outside;
			}
		}
	}
	_PDBO_("Done deleting partner particles")
	distributeParticles();
	_PDBO_("redistributed")
}


void Foam::functionObjects::pManager::renewFaceLists()
{
  //////////////////////////
  // Collect all facelists
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");


  // walls
  activeFacecentres_.setSize(patchSet_.size());
  activeFacevectors_.setSize(patchSet_.size());
  label wallCount = 0;
  forAllConstIter(labelHashSet, patchSet_, iter)
  {
    label patchI = iter.key();

    activeFacecentres_[wallCount] = &(mesh.boundary()[patchI].Cf());
    activeFacevectors_[wallCount] = &(mesh.boundary()[patchI].Sf());
    wallCount++;
  }

  nParticles_ = 0;
  forAll(popList_, i)
  {
      nParticles_ += popList_[i].size();
  }

  // particles
  particleList_.setSize(nParticles_);
  label count = 0;
  forAll(popList_, popI) // for all populations
  {
    const Population &pop = popList_[popI];
    forAllConstIter( HashTable<volumetricParticle*>, pop.container(), iter )
    {
      volumetricParticle *particle = *iter;
      particleList_[count++] = particle;
    }
  }
}

void Foam::functionObjects::pManager::subCyclingSaveState()
{
  forAll(popList_, i)
  {
    popList_[i].subCyclingSaveState();
  }
}

void Foam::functionObjects::pManager::subCyclingPreCollisionSaveState()
{
  forAll(particleList_, i)
  {
    particleList_[i]->subCyclingSavePreCollision();
  }
}

void Foam::functionObjects::pManager::subCyclingRestoreState()
{
  forAll(popList_, i)
  {
    popList_[i].subCyclingRestoreState();
  }
}

void Foam::functionObjects::pManager::iterativeCouplingParticleSavePoints()
{
  forAll(popList_, i)
  {
    popList_[i].iterativeCouplingSavePoints();
  }
}

void Foam::functionObjects::pManager::iterativeCouplingParticleSaveForces()
{
  forAll(popList_, i)
  {
    popList_[i].iterativeCouplingSaveForces();
  }
}

void Foam::functionObjects::pManager::iterativeCouplingParticleRestorePoints()
{
  forAll(popList_, i)
  {
    popList_[i].iterativeCouplingRestorePoints();
  }
}

void Foam::functionObjects::pManager::iterativeCouplingParticleRestoreForces()
{
  forAll(popList_, i)
  {
    popList_[i].iterativeCouplingRestoreForces();
  }
}

void Foam::functionObjects::pManager::iterativeCouplingParticleRelaxForces()
{
  forAll(popList_, i)
  {
    popList_[i].iterativeCouplingRelaxForces(iterativeCouplingCurrentRelax_);
  }
}

void Foam::functionObjects::pManager::calcSolidForces()
{
  /////////////////////////////////////////////////////////////////////////////
  // for all particles
  //
	//distributeParticles();
  for(label i = 0; i < nParticles_; i++)
  {
    volumetricParticle* pPtr1 = particleList_[i];

    if( pPtr1->isSlave() )
      continue;

    vectorField &f          =  pPtr1->solidForceField();
    const pointField   &pCf =  pPtr1->Cf();
    const vectorField  &pSf =  pPtr1->Sf();
    const word     &pIdStr1 =  pPtr1->idStr();
    label popIdI            =  pPtr1->popId();

    Potential    *pot = 0;
    contactModel *cM  = 0;

    ///////////////////////////////////////////////////////////////////////////
    // for any other particle
    //
    for(label j = 0; j < nParticles_; j++)
    {
      if(j==i)
        continue;
      volumetricParticle* pPtr2 = particleList_[j];
      if(
              backGroundGrid().getIJKDistance(
                                               pPtr1->cg(),
                                               pPtr2->cg()
                                             )
           >= 4
        )
        continue;

      const pointField   &pCfo =  pPtr2->Cf();
      const vectorField  &pSfo =  pPtr2->Sf();
      const word      &pIdStr2 =  pPtr2->idStr();
      label popIdJ             =  pPtr2->popId();

      pot = potTableGet(popIdI, popIdJ);
      cM  = cMTableGet(popIdI, popIdJ);  // just check if cM is available

      if( !pot && !cM )
        goto sweep_walls;

      surfaceIntegration(
                          &f,
                          pCf, pSf,
                          pCfo, pSfo,
                          pot,
                          cM,
                          pIdStr1,
                          pIdStr2,
                          popIdI,
                          popIdJ,
                          &contactHash_,
                          contactRadiusFactor_,
                          -1.0
                        );
    }


sweep_walls:

    if( !(backGroundGrid().nearDomainBoarder(pPtr1->cg(), 2)) )
      continue;

    pot = potTableGet(popIdI, 0);    // potential for popIdI against any wall
    cM  = cMTableGet(popIdI, 0);// contactModel for popIdI against any wall

    if(  !pot && !cM  )
      continue;

    ///////////////////////////////////////////////////////////////////////////
    // for all active walls
    //
    const word  wallNameDummy("wall");
    label wallCount = 0;
    forAllConstIter(labelHashSet, patchSet_, iter)
    {
      const pointField   &aCf = *(activeFacecentres_[wallCount]);
      const vectorField  &aSf = *(activeFacevectors_[wallCount]);

      // Surface integration algorithm
      surfaceIntegration(
                          &f,
                          pCf, pSf,
                          aCf, aSf,
                          pot,
                          cM,
                          pIdStr1,
                          wallNameDummy,
                          popIdI,
                          0, // popId 0 means wall
                          &contactHash_,
                          contactRadiusFactor_,
                          -1.0
                        );
      wallCount++;
    }
  }
  volumetricParticle* pPtr1 = particleList_[0];
}

void Foam::functionObjects::pManager::resetForces()
{
  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    particleList_[i]->resetForces();
  }

  validDevRhoReff_ = false;
}

void Foam::functionObjects::pManager::resetSolidForces()
{
  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    particleList_[i]->resetSolidForces();
  }
}

void Foam::functionObjects::pManager::processContacts()
{
  contactIter_t cIter = contactHash_.begin();

  // iteration over all contact candidates
  while( cIter != contactHash_.end() )
  {
    facePair        &fP = const_cast<contactState&>(*cIter);
    contactKinetic  &cK = const_cast<contactState&>(*cIter);
    contactMechanic &cM = const_cast<contactState&>(*cIter);

    const label               mF = fP._fMaster; // face index master
    const label               sF = fP._fSlave;  // face index slave
    const label           mPopId = fP._popIdMaster;
    const label           sPopId = fP._popIdSlave;
    const word           &mIdStr = fP._pIdStrMaster; // particle id string
    const word           &sIdStr = fP._pIdStrSlave;


    const volumetricParticle *mP = popList_[mPopId-1].findParticleByIdStr(mIdStr);
    const volumetricParticle *sP = (sPopId > 0)    ?
                                   popList_[sPopId-1].findParticleByIdStr(sIdStr) :
                                   0;

    // if counterpart is another particle
    if(sP)
    {
      mP->defineContact(mF, sP, sF, cK, cM); // set contactKinetic
    }
    else // counterpart is a wall with index -fP._pIdxSlave
    {
      const vector pos = (*activeFacecentres_[-sPopId])[sF];
      mP->defineContact(mF, pos, cK, cM);    // set contactKinetic
//      _PDBO_(pos)
    }

    // lost candidate status?
    if( cK._distance > contactRadiusFactor_*sqrt(cM._A) )
    {
      cIter = contactHash_.erase(cIter);
      continue;
    }
    if(sPopId == 0)
    {
//       _PDBO_(cK._nDist << " " << cK._tDist << " " << 1.6*sqrt(cM._A))
    }
    if(cK._nDist < 0 && mag(cK._tDist) < 1.6*sqrt(cM._A))
    {
        if(sPopId == 0)
        {
//         _PDBI_
        }

      fP._history = facePair::established;
      // q 'n' d hack
      vectorField &force = const_cast<volumetricParticle*>(mP)->solidForceField();
      const contactModel* contact = cMTableGet(mPopId, sPopId);

      force[mF] += contact->getForce(cK, cM);
    }
    else
      fP._history = facePair::candidate;

    ++cIter;
  }
}

Foam::volScalarField& Foam::functionObjects::pManager::voidFrac()
{
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");


  if(! mesh.foundObject<volScalarField>("voidFrac") )
  {
    // create field for void fraction in main mesh
    voidFracPtr_.reset( new volScalarField
               (
                 IOobject
                 (
                     "voidFrac",
                     obr_.time().timeName(),
                     mesh,
                     IOobject::READ_IF_PRESENT,
                     IOobject::AUTO_WRITE
                 ),
                 mesh,
                 dimensioned<scalar>("fluid", dimensionSet(0, 0, 0, 0, 0), 0.0),
                 zeroGradientFvPatchField<scalar>::typeName
               )
                 );
  }

  volScalarField& vF = const_cast<volScalarField&>(
                            mesh.lookupObject<volScalarField>("voidFrac")
                                                  );
  return vF;
}

Foam::volScalarField& Foam::functionObjects::pManager::deposit()
{
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");

  if(! mesh.foundObject<volScalarField>("deposit") )
  {
    // create field for deposit in main mesh
    depositPtr_.reset( new volScalarField
               (
                 IOobject
                 (
                     "deposit",
                     obr_.time().timeName(),
                     mesh,
                     IOobject::READ_IF_PRESENT,
                     IOobject::AUTO_WRITE
                 ),
                 mesh,
                 dimensioned<scalar>("fluid", dimensionSet(0, 0, 0, 0, 0), 0.0),
                 zeroGradientFvPatchField<scalar>::typeName
               )
                 );
  }

  volScalarField& depo = const_cast<volScalarField&>(
                            mesh.lookupObject<volScalarField>("deposit")
                                                  );
  return depo;
}

Foam::volVectorField& Foam::functionObjects::pManager::particleVelo()
{
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");

  if(! mesh.foundObject<volVectorField>("particleVelo") )
  {
    // create field for void fraction in main mesh
    particleVeloPtr_.reset( new volVectorField
               (
                 IOobject
                 (
                     "particleVelo",
                     obr_.time().timeName(),
                     mesh,
                     IOobject::READ_IF_PRESENT,
                     IOobject::AUTO_WRITE
                 ),
                 mesh,
                 dimensioned<vector>("fluid", dimensionSet(0, 1, -1, 0, 0), vector(0., 0., 0.)),
                 zeroGradientFvPatchField<vector>::typeName
               )
                 );
  }

  volVectorField& pV = const_cast<volVectorField&>(
                            mesh.lookupObject<volVectorField>("particleVelo")
                                                  );

  return pV;
}

Foam::volScalarField& Foam::functionObjects::pManager::wallDistance()
{
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");

  if(! mesh.foundObject<volScalarField>("wallDistance") )
  {
    // create field for wallDistance in main mesh
    wallDistPtr_.reset( new volScalarField
            (
              IOobject
              (
                  "wallDistance",
                  obr_.time().timeName(),
                  mesh,
                  IOobject::READ_IF_PRESENT,
                  IOobject::AUTO_WRITE
              ),
              mesh,
              dimensioned<scalar>("fluid", dimensionSet(0, 1, 0, 0, 0), VGREAT),
              zeroGradientFvPatchField<scalar>::typeName
            )
              );
  }

  volScalarField& yW = const_cast<volScalarField&>(
                            mesh.lookupObject<volScalarField>("wallDistance")
                                                  );

  return yW;
}


Foam::volVectorField& Foam::functionObjects::pManager::wallN()
{
 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");


  if(! mesh.foundObject<volVectorField>("wallN") )
  {
    // create field for wallN in main mesh
    wallNPtr_.reset( new volVectorField
            (
              IOobject
              (
                  "wallN",
                  obr_.time().timeName(),
                  mesh,
                  IOobject::READ_IF_PRESENT,
                  IOobject::AUTO_WRITE
              ),
              mesh,
              dimensioned<vector>("fluid", dimensionSet(0, 0, 0, 0, 0), vector(VGREAT, VGREAT, VGREAT)),
              zeroGradientFvPatchField<vector>::typeName
            )
              );
  }

  volVectorField& nW = const_cast<volVectorField&>(
                            mesh.lookupObject<volVectorField>("wallN")
                                                  );

  return nW;
}

void Foam::functionObjects::pManager::mapParticleMomentumToFluidOverNeighbours()
{
  if( !fsi_ )
    return;

 const fvMesh&     mesh      = refCast<const fvMesh>(obr_);
 const pointField &midPoints = mesh.C().internalField();


 // Make sure neighbour pointer exists
 if(!mesh.hasCellCells()) mesh.cellCells();

  vectorField      &pV        = particleVelo().ref();
  scalarField      &vF        = voidFrac().ref();
  volScalarField   vFOld	  = voidFrac().oldTime();

  List<bool> inside(midPoints.size());

  // reset particle velocities
  pV = vector::zero;
  // reset voidFrac_ to zero (fluid everywhere)
  vF = 0.0;

  const vector lineDist = 2 * backGroundGrid().span();

  pointField start(1);
  pointField end(1);

  List<scalar> mappingDists(nParticles_);

  // Precalc cut-off distance for increased
  // calculation speed in following forAll-loop
  for(label i = 0; i < nParticles_ ; i++)
  {
	  volumetricParticle *pPtr = particleList_[i];
	  mappingDists[i] = popList_[pPtr->popId()-1].mappingDist() * pPtr->scale_;
  }

  forAll(midPoints, midPoint)
  {
	  // Check if a neighbour of the cell has a voidFraction = 1
	  // If not than this cell will also not need to be set to voidFraction = 1
	  labelList neighbourCells = mesh.cellCells(midPoint);

	  bool hasVoidFracNeighbour = false;
	  forAll(neighbourCells, nCell)
	  {
		  if(vFOld[neighbourCells[nCell]] == 1)
		  {
			  hasVoidFracNeighbour = true;
			  break;
		  }
	  }
	  if(!hasVoidFracNeighbour) continue;



	  for(label i = 0; i < nParticles_; i++)
	  {
		  volumetricParticle *pPtr = particleList_[i];

		  if(pPtr->myPop_->isPointParticle())
		  {
			  // If there is an idea or model for how to treat the coupling of such
			  // small particles to the fluid then the corresponding method,
			  // e.g. pManager::subCellFluidCoupling, should be implemented right here.
			  // The following "continue" has to remain in any case.
			  if(!pPtr->myPop_->mapPointParticleMomentum()) continue;
			  label cellI = myMS_.findCell( pPtr->getCg() );
			  vF[midPoint] += 1.0;
			  pV[midPoint] = pPtr->velocityAtCgSubCellSize_;
			  continue;
		  }

		  // Is cell close enough to be inside the STL?
		  if(mag(midPoints[midPoint] - pPtr->getCg()) > mappingDists[i]) continue;

		  start[0] = midPoints[midPoint];
		  end[0]   = start[0] + lineDist;
		  List< List<pointIndexHit> > lineHits;

		  // Raycast based search
		  //_DBO_("start = " << start << " end = " << end << " lineDist = " << lineDist)
		  pPtr->triSurfSearch().findLineAll(start, end, lineHits);

		  // Apply voidFrac and particle velocity to fluid
		  if (lineHits[0].size() % 2)
		  {
			  vF[midPoint] += 1.0;
			  pV[midPoint] = pPtr->getPointVelocity(midPoints[midPoint]);

			  break; // This midpoint found its particle, so skip to next midpoint
		  }
	  }
  }


  // In parallel run: Always do a check at processor boundary
  // cells, because voidFrac boundary cells are not communicated
  // during solver execution time.
  if(Pstream::parRun())
  {
	  List<label> bFaceCells;
	  forAll(mesh.boundaryMesh(), iPatch)
		{
		  // Find all processor boundary cells
		  if(mesh.boundaryMesh().types()[iPatch] == "processor")
		  {
			  bFaceCells.append(mesh.boundaryMesh()[iPatch].faceCells());
		  }
		}

	  // Almost identical to above forAll(midPoints, ...) loop
	  forAll(bFaceCells, iCell)
	  {
		  for(label i = 0; i < nParticles_; i++)
	      {
	          volumetricParticle *pPtr = particleList_[i];

	          if(pPtr->myPop_->isPointParticle())
	          {
	      	    // If there is an idea or model for how ...
	      	    continue;
	          }

	          if(mag(midPoints[bFaceCells[iCell]] - pPtr->getCg()) > mappingDists[i]) continue;

	          start[0] = midPoints[bFaceCells[iCell]];
	          end[0]   = start[0] + lineDist;
	          List< List<pointIndexHit> > lineHits;

	          pPtr->triSurfSearch().findLineAll(start, end, lineHits);

	          if (lineHits[0].size() % 2)
	          {
	      	    vF[bFaceCells[iCell]] += 1.0;
	      	    pV[bFaceCells[iCell]] = pPtr->getPointVelocity(midPoints[bFaceCells[iCell]]);

	      	    break;
	          }
	      }
	  }
  }

}

void Foam::functionObjects::pManager::mapParticleMomentumToFluid()
{
  if( !fsi_ )
    return;

  //_DBO_("Currently hard-coded exit for subcell particles PLEASE CHANGE THIS")
  //return;

 const fvMesh&     mesh      = refCast<const fvMesh>(obr_);
 const pointField &midPoints = mesh.C().internalField();

  vectorField      &pV        = particleVelo().ref();
  scalarField      &vF        = voidFrac().ref();

  List<bool> inside(midPoints.size());

  // reset particle velocities
  pV = vector::zero;
  // reset voidFrac_ to zero (fluid everywhere)
  vF = 0.0;

  const vector lineDist = 2 * backGroundGrid().span();

  pointField start(1);
  pointField end(1);

  List<scalar> mappingDists(nParticles_);

  // Precalc cut-off distance for increased
  // calculation speed in following forAll-loop
  for(label i = 0; i < nParticles_ ; i++)
  {
	  volumetricParticle *pPtr = particleList_[i];
	  mappingDists[i] = popList_[pPtr->popId()-1].mappingDist() * pPtr->scale_;
  }

  forAll(midPoints, midPoint)
  {
	  for(label i = 0; i < nParticles_; i++)
	  {
		  volumetricParticle *pPtr = particleList_[i];

		  if(pPtr->myPop_->isPointParticle())
		  {
			  // If there is an idea or model for how to treat the coupling of such
			  // small particles to the fluid then the corresponding method,
			  // e.g. pManager::subCellFluidCoupling, should be implemented right here.
			  // The following "continue" has to remain in any case.
			  if(!pPtr->myPop_->mapPointParticleMomentum()) continue;
			  label cellI = myMS_.findCell( pPtr->getCg() );
			  vF[midPoint] += 1.0;
			  pV[midPoint] = pPtr->velocityAtCgSubCellSize_;
			  continue;
		  }

		  // Is cell close enough to be inside the STL?
		  if(mag(midPoints[midPoint] - pPtr->getCg()) > mappingDists[i]) continue;

		  start[0] = midPoints[midPoint];
		  end[0]   = start[0] + lineDist;
		  List< List<pointIndexHit> > lineHits;

		  // Raycast based search
		  //_DBO_("start = " << start << " end = " << end << " lineDist = " << lineDist)
		  pPtr->triSurfSearch().findLineAll(start, end, lineHits);

		  // Apply voidFrac and particle velocity to fluid
		  if (lineHits[0].size() % 2)
		  {
			  vF[midPoint] += 1.0;
			  pV[midPoint] = pPtr->getPointVelocity(midPoints[midPoint]);

			  break; // This midpoint found its particle, so skip to next midpoint
		  }
	  }
  }


#if 0
_DBO_("Experimental...")
  const labelList& owner     = mesh.owner();
  const labelList& neighbour = mesh.neighbour();

  forAll(owner, faceI)
  {
    label oI = owner[faceI];
    label nI = neighbour[faceI];

    scalar diff = vF[oI] - vF[nI];

    if( magSqr(diff) < VSMALL )
      continue; // face is inside or outside

    // now faceI belongs to boundary of ib
    if( vF[oI] > VSMALL)  // owner is inside voidFrac
    {
      vF[oI] = 0.9;
    }
    else  // neighbour is inside voidFrac
    {
      vF[nI] = 0.9;
    }
  }
  forAll(owner, faceI)
  {
    label oI = owner[faceI];
    label nI = neighbour[faceI];

    scalar diff = vF[oI] - vF[nI];

    if( magSqr(diff) < VSMALL )
      continue; // face is inside or outside

    // now faceI belongs to boundary of ib
    if( vF[oI] > 0.1/*VSMALL*/)  // owner is inside voidFrac
    {
      vF[nI] = 0.8;
    }
    else  // neighbour is inside voidFrac
    {
      vF[oI] = 0.8;
    }
  }
  forAll(owner, faceI)
  {
    label oI = owner[faceI];
    label nI = neighbour[faceI];

    scalar diff = vF[oI] - vF[nI];

    if( magSqr(diff) < VSMALL )
      continue; // face is inside or outside

    // now faceI belongs to boundary of ib
    if( vF[oI] > 0.1/*VSMALL*/)  // owner is inside voidFrac
    {
      vF[nI] = 0.7;
    }
    else  // neighbour is inside voidFrac
    {
      vF[oI] = 0.7;
    }
  }
#endif
}


void Foam::functionObjects::pManager::mapFluidForcesToParticles()
{
  if( !fsi_ )
    return;

  // Retrieve stress tensor field
  const symmTensorField       &stress = devRhoReff().internalField();//devRhoReff()();//Vora
  // Retrieve pressure field
  const volScalarField& pVol = obr_.lookupObject<volScalarField>(pName_);
  const scalarField& p = pVol.internalField();//pVol();//Vora
  

 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");
  const pointField &midPoints = mesh.C().internalField();
  const volScalarField& vF       = voidFrac();
  const scalarField&    meshVols = mesh.V();

  // map pressure and stress of main mesh to all particles
  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    volumetricParticle *pPtr = particleList_[i];

    // Check and special treatment for particles smaller than the common cell size
	if(pPtr->myPop_->isPointParticle())
	{
	  // Other methods or variants are possible!
	  mapFluidForcesToSubCellParticle(pPtr);
	  //pPtr->calcFluidForces(pRef_, rho(pVol));
	  continue;
	}
	
    const pointField&  faceCtrs  = pPtr->triSurf().faceCentres();
    symmTensorField&   pStress   = pPtr->stressField();
    scalarField&       pP        = pPtr->pressureField();
    const vectorField& n         = pPtr->normals();
	//tensorinterpolation
	//maybe interpolate preassure too later!!
	//scalarField& pPFOS = pPtr->pressureField();
	//scalarField& pPSOS = pPtr->pressureField();
	scalar pFOS, pSOS, numberOfFuildNeighbours, pInterpol, distance;
	point CellCenterPoint, pointHelper1, pointHelper2;
	vector eigenValueTest;
	symmTensor stressFOS, stressSOS;
	symmTensor interpolatedTensorField = symmTensor::zero;
	/*symmTensorField compareFOS(numberOfFaces,symmTensor::zero);
	symmTensorField compareSOS(numberOfFaces,symmTensor::zero);
	vectorField           Sn = pPtr->Sf();*/
	vectorField			  forceFOS,forceSOS,forceInterpolated;

	std::chrono::steady_clock::time_point start_time_all, end_time_all;

	start_time_all = std::chrono::steady_clock::now();
    forAll(faceCtrs, faceI)
    {
      point faceMid = faceCtrs[faceI];
	  point faceFOs,faceSOs;

      label cellI = myMS_.findCell( faceMid );

      pStress[faceI] = symmTensor::zero;
      pP[faceI]      = 0.0;
      if( cellI == -1 )
      {
        continue;
      }

#if 0
// testing
        pStress[faceI] = 1000./998*I;
        pP[faceI]      = 1000./998;
        continue;

#endif
	  label firstFound   = cellI;
      label firstOutside = cellI;
	  label secondOutside = firstOutside;

	  if( ((stresstensorInterpolationMethod_ ) == "firstOutside") 
	  || (pressureInterpolationMethod_ == "firstOutside")
	  || ((stresstensorInterpolationMethod_ ) == "secondOutside")
	  || (pressureInterpolationMethod_== "secondOutside")
	  || ((stresstensorInterpolationMethod_ )== "lci")
	  || (pressureInterpolationMethod_== "lci")
	  || ((stresstensorInterpolationMethod_ ) == "wlci")
	  || (pressureInterpolationMethod_ == "wlci")
	  || ((stresstensorInterpolationMethod_ ) == "lii")
	  || (pressureInterpolationMethod_ == "lii" )
	  || (pressureInterpolationMethod_ == "cLciWnlci")){

	  

      
#if 1
      scalar dx   = std::cbrt(meshVols[firstFound]);
      scalar diag = dx*_LSMSQRTOFTHREE;

      const label  nSteps = 2;
      const scalar step   = dx; //diag/ nSteps;

      // If face centre lies inside voidFrac, move outside along face normal
      // for a distance of factor*cellSize
      if( vF[firstFound] > 0. )
      {

        for(label currStep = 0; (currStep < nSteps) && (firstOutside != -1) && (vF[firstOutside] > 0.); ++currStep )
        {
          faceMid += step*n[faceI];
		  faceFOs = faceMid;
          firstOutside = myMS_.findCell( faceMid );
        }
        if( (firstOutside == -1) || (vF[firstOutside] > 0.) )
        {
          pStress[faceI] = symmTensor::zero;
          pP[faceI]      = 0.0;
          continue;
        }
      }
#endif

	  //pStressFOS = stress[firstOutside];
	  //_DBO_("this is firstOutside before " << firstOutside);
      
#if 1
	  if(((stresstensorInterpolationMethod_ ) == "secondOutside")
	  || (pressureInterpolationMethod_== "secondOutside")
	  || ((stresstensorInterpolationMethod_ )== "lci")
	  || (pressureInterpolationMethod_== "lci")
	  || ((stresstensorInterpolationMethod_ ) == "wlci")
	  || (pressureInterpolationMethod_ == "wlci")
	  || ((stresstensorInterpolationMethod_ ) == "lii")
	  || (pressureInterpolationMethod_ == "lii" )
	  || (stresstensorInterpolationMethod_ == "cLciWnlci")
	  || (pressureInterpolationMethod_ == "cLciWnlci")){
      for(label currStep = 0; (currStep < nSteps) && (secondOutside != -1) && (secondOutside == firstOutside); ++currStep )
      {
        faceMid += step*n[faceI];
		faceSOs = faceMid;
        secondOutside = myMS_.findCell( faceMid );
      }

      if( secondOutside == -1 )
      {
        pStress[faceI] = symmTensor::zero;
        pP[faceI]      = 0.0;
        continue;
      }
	  }
#endif
	  //TODO -->here goes interpolation of tensors--> we have to check what kind of tensor this is and how to iterate on it!!!
	  //first Idea is linear interpolation.. maybe we implement another function here:
	  //choose firstOutside, secondOutside, linearInterpolation--> best if we grep information from controlDict
	  //options name could be: shearTrnsorInterpolationmethod
	  
	  //pStressFOS = stress[firstOutside];
	  //pStressSOS = stress[secondOutside];
	  stressFOS = stress[firstOutside];
	  stressSOS = stress[secondOutside];
	  pFOS = p[firstOutside];
	  pSOS = p[secondOutside];

	}
	  
	  //pPFOS[faceI] = p[firstOutside];
	  //pPSOS = p[secondOutside];
	  /*testDruckFOS = p[firstOutside];
	  testDruckSOS = p[secondOutside];*/
	  //_DBO_("druck firstOutside " << testDruckFOS << " aus Zelle " << firstOutside);
	  //_DBO_("druck secondOutside " << testDruckSOS << " aus Zelle " << secondOutside);
	  //_DBO_("firstOutside Tesnor " << stressTestFOS << " und secondOutside Tensor " << stressTestSOS)


	  /*_DBO_("this is firstOutside " << firstOutside << " and this 2nd " << secondOutside )
	  _DBO_("0 first Outside Tensor "<<pStressFOS[0] << " second Outside Tensor " <<pStressSOS[0]);
	  _DBO_("1 first Outside Tensor "<<pStressFOS[1] << " second Outside Tensor " <<pStressSOS[1]);
	  _DBO_("2 first Outside Tensor "<<pStressFOS[2] << " second Outside Tensor " <<pStressSOS[2]);
	  _DBO_("2 first Outside Tensor "<<pStressFOS[3] << " second Outside Tensor " <<pStressSOS[3]);
	  _DBO_("2 first Outside Tensor "<<pStressFOS[4] << " second Outside Tensor " <<pStressSOS[4]);
	  _DBO_("2 first Outside Tensor "<<pStressFOS[5] << " second Outside Tensor " <<pStressSOS[5]);*/
	  
	  /*
	  _DBO_("0 interpolated Tensor " << interpolatedTensorField[0]);
	  _DBO_("1 interpolated Tensor " << interpolatedTensorField[1]);
	  _DBO_("2 interpolated Tensor " << interpolatedTensorField[2]);
	  _DBO_("3 interpolated Tensor " << interpolatedTensorField[3]);
	  _DBO_("4 interpolated Tensor " << interpolatedTensorField[4]);
	  _DBO_("2 interpolated Tensor " << interpolatedTensorField[2]);*/


	  /*
      pStress[faceI] = stress[secondOutside];
      pP[faceI]      = p[secondOutside];*/
	  //labelList testList = mesh.cellCells(cellI);
	  //_DBO_("Nachbarliste von " << cellI <<" ist: " << testList);
	
	  if(stresstensorInterpolationMethod_ == "firstOutside"){
		  pStress[faceI] = stress[firstOutside];
	  }
	  else if(stresstensorInterpolationMethod_ == "secondOutside"){
		  pStress[faceI] = stress[secondOutside];

	  }
	  else if(stresstensorInterpolationMethod_ == "lci"){
		  for(int iter = 0; iter <=5; iter++){
		  interpolatedTensorField[iter] = -(stressSOS[iter]-stressFOS[iter]) +stressFOS[iter];
		  //_DBO_("firstOutside " << stressFOS[iter] << " secondOutside " << stressSOS[iter] << " interpolated " << interpolatedTensorField[iter]);
		  }
		  pStress[faceI] = interpolatedTensorField;
	  }
	  else if(stresstensorInterpolationMethod_ == "wlci"){
		  for(int iter = 0; iter <=5; iter++){
		  interpolatedTensorField[iter] = stressFOS[iter] * stresstensorFirstSecondWeight_+ (1- stresstensorFirstSecondWeight_) * stressSOS[iter];
		  //_DBO_("firstOutside " << stressFOS[iter] << " secondOutside " << stressSOS[iter] << " interpolated " << interpolatedTensorField[iter]);
		  } 
		  pStress[faceI] = interpolatedTensorField;
	  }
	  else if(stresstensorInterpolationMethod_ == "cLciWnlci"){
		  //faceFOs //fos
		  //faceSOs //sos
		  //midPoints
		  labelList neighbourCellsFOs = mesh.cellCells(firstOutside);
		  labelList neighbourCellsSOs = mesh.cellCells(secondOutside);
		  scalar neighbourWeightsFOs = 0;
		  scalar neighbourWeightsSOs = 0;
		  //get weighting
		  if( cLciWnlci_weighting_ == "inverseDistance"){
			  numberOfFuildNeighbours = 0;
			  scalar weight;
			  interpolatedTensorField = symmTensor::zero;
			  symmTensor interpolatedTensorFieldFOs = symmTensor::zero;
			  symmTensor interpolatedTensorFieldSOs = symmTensor::zero;
			  if(midPoints[firstOutside] == faceFOs){
				  interpolatedTensorFieldFOs = stressFOS;
				  neighbourWeightsFOs = 1;
			  }
			  else{
				forAll(neighbourCellsFOs,cell){
					if(vF[neighbourCellsFOs[cell]] < 1){
						pointHelper1 = midPoints[neighbourCellsFOs[cell]];
						pointHelper2 = faceFOs;
						weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
						neighbourWeightsFOs += weight;
						numberOfFuildNeighbours +=1;
						interpolatedTensorFieldFOs += weight * stress[neighbourCellsFOs[cell]];
						//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " stress " << stress[cell]);
					}
					//+fos
				}
				pointHelper1 = midPoints[firstOutside];
				pointHelper2 = faceFOs;
				weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
				neighbourWeightsFOs += weight;
				interpolatedTensorFieldFOs += weight * stressFOS;
			  }
			  if(midPoints[secondOutside] == faceSOs){
				  interpolatedTensorFieldSOs = stressSOS;
				  neighbourWeightsSOs = 1;
			  }
			  else{
				forAll(neighbourCellsSOs,cell){
					if(vF[neighbourCellsSOs[cell]] < 1){
						pointHelper1 = midPoints[neighbourCellsSOs[cell]];
						pointHelper2 = faceSOs;
						weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
						neighbourWeightsSOs += weight;
						numberOfFuildNeighbours +=1;
						interpolatedTensorFieldSOs += weight * stress[neighbourCellsSOs[cell]];
						//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " stress " << stress[cell]);
					}
					//+fos
				}
				pointHelper1 = midPoints[secondOutside];
				pointHelper2 = faceSOs;
				weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
				neighbourWeightsSOs += weight;
				interpolatedTensorFieldSOs += weight * stressSOS;
			  }
			  interpolatedTensorFieldFOs = 1 / neighbourWeightsFOs * interpolatedTensorFieldFOs;
			  interpolatedTensorFieldSOs = 1 / neighbourWeightsSOs * interpolatedTensorFieldSOs;
			  
			  interpolatedTensorField = 2 * interpolatedTensorFieldFOs - interpolatedTensorFieldSOs;
		  }
		  pStress[faceI] = interpolatedTensorField;
		  //FOS

	  }
	  else if(stresstensorInterpolationMethod_ == "wnlci"){ //weightedNeighbour
		labelList neighbourCells = mesh.cellCells(cellI);
		if (stresstensorNeighbourWeighting_ == "equal"){
			numberOfFuildNeighbours = 0;
			interpolatedTensorField = symmTensor::zero;
			forAll (neighbourCells,cell){
				//_DBO_("voidFraq is " << vF[neighbourCells[cell]]); 				  
				if(vF[neighbourCells[cell]] < 1){
					numberOfFuildNeighbours +=1;
					interpolatedTensorField += stress[neighbourCells[cell]];
					//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " stress " << stress[cell]);
				}
			}
			//_DBO_("sum is " << interpolatedTensorField << " and number of neighbours " << numberOfFuildNeighbours);
			interpolatedTensorField  /= numberOfFuildNeighbours;
			pStress[faceI] = interpolatedTensorField;
			  
		  	//_DBO_("cell stress is " << pStress[faceI]);
	  	}
		else if (pressureNeighbourWeighting_ == "distance"){
			//faceMid
			interpolatedTensorField = symmTensor::zero;
			numberOfFuildNeighbours = 0;
			scalar singleWeight;
			forAll (neighbourCells,cell){
				 if(vF[neighbourCells[cell]] < 1){
					//numberOfFuildNeighbours += mag(midPoints[neighbourCells[cell]] - faceMid[faceI]);//TODO
					//singleWeight = mag(midPoints[neighbourCells[cell]] - faceMid[faceI]);
					pointHelper1 = faceCtrs[faceI];
					pointHelper2 = midPoints[neighbourCells[cell]];
					singleWeight = mag(pointHelper1 - pointHelper2);//mag(midPoints[neighbourCells[cell]]);//1;

					numberOfFuildNeighbours += singleWeight;
					//CellCenterPoint = midPoints[neighbourCells[cell]];
					interpolatedTensorField += stress[neighbourCells[cell]]*singleWeight;
					//_DBO_("singleWeight = " << singleWeight);
					//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " pressure " << p[cell]);
				  }
			  }
			  //_DBO_("sum is " << pInterpol << " and number of neighbours " << numberOfFuildNeighbours);
			interpolatedTensorField  /= numberOfFuildNeighbours;
			pStress[faceI] = interpolatedTensorField;
		}

	  }
	  else if (stresstensorInterpolationMethod_ == "lii"){
		  scalar firstI_FOs, secondI_FOs, thirdI_FOs;
		  scalar firstI_SOs, secondI_SOs, thirdI_SOs;
		  scalar interpolated_firstI, interpolated_secondI, interpolated_thirdI;
		  vector lambda_vector = vector::zero;
		  symmTensor D = symmTensor::zero;
		  interpolatedTensorField = symmTensor::zero;
		  tensor interpolatedTransformationTensor, tensorHelper1, tensorHelper2, tensorHelper3;

		  /*
		  firstI_FOs 	= invariantI(stressFOS);
		  secondI_FOs 	= invariantII(stressFOS);
		  thirdI_FOs 	= invariantIII(stressFOS);

		  firstI_SOs 	= invariantI(stressSOS);
		  secondI_SOs 	= invariantII(stressSOS);
		  thirdI_SOs 	= invariantIII(stressSOS);

		  */
		  firstI_FOs = tr(stressFOS);
		  firstI_SOs = tr(stressSOS);

		  if(mag(dev(stressFOS)) == 0){
			  secondI_FOs = 0;
			  thirdI_FOs = 0;
		  }
		  else{
			  secondI_FOs = sqrt(3./2.)*mag(stressFOS)/mag(dev(stressFOS));
			  thirdI_FOs = 3*sqrt(6.)*det(dev(stressFOS)/mag(dev(stressFOS)));
		  }
		  if(mag(dev(stressSOS)) == 0){
			  secondI_SOs = 0;
			  thirdI_SOs = 0;
		  }
		  else{
			  secondI_SOs = sqrt(3./2.)*mag(stressSOS)/mag(dev(stressSOS));
			  thirdI_SOs = 3*sqrt(6.)*det(dev(stressSOS)/mag(dev(stressSOS)));
		  }

		  
		  interpolated_firstI = stresstensorFirstSecondWeight_*firstI_FOs + (1.-stresstensorFirstSecondWeight_) * firstI_SOs;
		  interpolated_secondI = stresstensorFirstSecondWeight_*secondI_FOs + (1.-stresstensorFirstSecondWeight_) * secondI_SOs;
		  interpolated_thirdI = stresstensorFirstSecondWeight_*thirdI_FOs + (1.-stresstensorFirstSecondWeight_) * thirdI_SOs;
		  /*
		  _DBO_("I1_fos " << firstI_FOs << " and I1_sos " << firstI_SOs);
		  _DBO_("interpolated I1" << interpolated_firstI);*/
		  //scalar test = 0;
		  //lambda_vector[0] = getEigenValuesFromInvariants(interpolated_firstI, interpolated_secondI, interpolated_thirdI,test);
		  scalar I1 = interpolated_firstI;
		  scalar I2 = interpolated_secondI;
		  scalar I3 = interpolated_thirdI;

		  scalar test1,test2, test3;
		  pStress[faceI] = symmTensor::zero;//VSMALL
		  /*
		  test1 = tr(stressFOS);
		  _DBO_("test" << mag(dev(stressFOS)));
		  if(mag(dev(stressFOS)) == 0){
			  test2 = 0;
			  test3 = 0;
		  }
		  else{
			  test2 = sqrt(3./2.)*mag(stressFOS)/mag(dev(stressFOS));
			  test3 = 3*sqrt(6.)*det(dev(stressFOS)/mag(dev(stressFOS)));
		  }
		  
		  _DBO_("trace is " << test1 << " and first I " << firstI_FOs);
		  _DBO_("paper says " << test2 << " and second I " << secondI_FOs << " where mag " << mag(dev(stressFOS)));
		  _DBO_("paper says " << test3 << " and thrid I " << thirdI_FOs);
		  */
		  scalar z;
		  
		  for (int i = 0; i <= 2; i++){
			  //_DBO_("thrid I " << I3);
			  //_DBO_(i << (acos(I3)+(-2.0+i*2.0)*constant::mathematical::pi)/3);
			  if(i == 0){
				  z = 0;
			  }
			  if(i ==1){
				  z = -2;
			  }
			  else{
				  z = 2;
			  }
			  /*_DBO_("I1 " << I1 << " I2 " << I2 << " I3 " << I3);
			  _DBO_("cos ( " << (acos(I3)+(z)*constant::mathematical::pi)/3.);
			  _DBO_("sqrt(" << 3.-2.*I2*I2)
			  _DBO_("small" << VSMALL)*/
			  if( 3.-2.*I2*I2 < 0){
				  lambda_vector[i] = I1/3.+2.*I1*I2/(3.*1E-16)*cos((acos(I3)+(-2.0+i*2.0)*constant::mathematical::pi)/3.);
			  }
			  else{
				  lambda_vector[i] = I1/3.+2.*I1*I2/(3.*sqrt(3.-2.*I2*I2+1E-15))*cos((acos(I3)+(-2.0+i*2.0)*constant::mathematical::pi)/3.);
			  }
			  //lambda_vector[i] = I1/3.+2.*I1*I2/(3.*sqrt(3.-2.*I2*I2+1E-15))*cos((acos(I3)+(-2.0+i*2.0)*constant::mathematical::pi)/3.);
		  }
		  //tensorHelper1 = eigenVectors(stressFOS);
		  //tensorHelper2 = eigenVectors(stressSOS);
		  tensorHelper1 = stresstensorFirstSecondWeight_ * stressFOS+ (1.-stresstensorFirstSecondWeight_)*stressSOS;


		  interpolatedTransformationTensor = eigenVectors(tensorHelper1);//tensorHelper1*stresstensorFirstSecondWeight_+tensorHelper2*(1.-stresstensorFirstSecondWeight_);
		  tensorHelper2 = interpolatedTransformationTensor.T();
		  //Roots< 3 > testRoot;
		  //const Foam::Roots<3> testRoot(1.,2.,3.)
		  //_DBO_(Roots< 3 > (1.,2.,3.)const);
		  //Tensor <complex> testVector;
		  //testVector = eigenVectors(stressFOS);
		  //_DBO_(eigenVectors(stressFOS));
		  tensor testTensor = eigenVectors(stressFOS);
		  tensor Transposte = testTensor.T();
		  //_DBO_(testTensor);
		  //_DBO_(Transposte);

		  //Roots< 3 > roots(cubicEqn(1, I1, I2, I3).roots()) const;
		  //Foam::Roots<3> Foam::cubicEqn::roots() Roots<3> roots(Foam::cubicEqn(1, I1, I2, I3).roots());
		  //_DBO_("roots " << roots);
		  D[0] = lambda_vector[0];
		  D[3] = lambda_vector[1];
		  D[5] = lambda_vector[2];
		  
		  tensorHelper3 = interpolatedTransformationTensor & D & tensorHelper2;
		  for(int i = 0; i <= 2 ; i++){
			  interpolatedTensorField[i] = tensorHelper3[i];
		  }
		  for(int i = 1; i <= 2 ; i++){
			  interpolatedTensorField[2+i] = tensorHelper3[3+i];
		  }
		  interpolatedTensorField[5] = tensorHelper3[8];
		  pStress[faceI] = interpolatedTensorField;
		  
		  //_DBO_("finalTensor: " << Tensorhelper3);
		  //_DBO_("diagonalMatrix" << D);
		  
		  
		  /*
		  for (int pNo = 0; pNo<=2;pNo++){
			  lambda_vector[pNO] = 1;

		  }*/
		  



	  }
	  else{
		  pStress[faceI] = symmTensor::zero;
	  }



	  if(pressureInterpolationMethod_ == "firstOutside"){
		  pP[faceI]		 = p[firstOutside];
	  }
	  else if (pressureInterpolationMethod_ == "firstOutside"){
		  pP[faceI]		 = p[secondOutside];
	  }
	  else if (pressureInterpolationMethod_ == "lci"){
		  pP[faceI]		 = 2*pFOS-pSOS;
	  }
	  else if (pressureInterpolationMethod_ == "wlci"){
		  pP[faceI]		 = pFOS * stresstensorFirstSecondWeight_ + pSOS*(1- stresstensorFirstSecondWeight_);
	  }

	  else if(pressureInterpolationMethod_ == "cLciWnlci"){
		  //faceFOs //fos
		  //faceSOs //sos
		  //midPoints
		  labelList neighbourCellsFOs = mesh.cellCells(firstOutside);
		  labelList neighbourCellsSOs = mesh.cellCells(secondOutside);
		  scalar neighbourWeightsFOs = 0;
		  scalar neighbourWeightsSOs = 0;
		  scalar interpolatedP = 0;
		  scalar interpolateFOs = 0;
		  scalar interpolateSOs = 0;
		  //get weighting
		  if( cLciWnlci_weighting_ == "inverseDistance"){
			  numberOfFuildNeighbours = 0;
			  scalar weight;
			  
			  if(midPoints[firstOutside] == faceFOs){
				  interpolateFOs = pFOS;
				  neighbourWeightsFOs = 1;
			  }
			  else{
				forAll(neighbourCellsFOs,cell){
					if(vF[neighbourCellsFOs[cell]] < 1){
						pointHelper1 = midPoints[neighbourCellsFOs[cell]];
						pointHelper2 = faceFOs;
						weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
						neighbourWeightsFOs += weight;
						numberOfFuildNeighbours +=1;
						interpolateFOs += weight * p[neighbourCellsFOs[cell]];
						//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " stress " << stress[cell]);
					}
					//+fos
				}
				pointHelper1 = midPoints[firstOutside];
				pointHelper2 = faceFOs;
				weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
				neighbourWeightsFOs += weight;
				interpolateFOs += weight * pFOS;
			  }
			  if(midPoints[secondOutside] == faceSOs){
				  interpolateSOs = pSOS;
				  neighbourWeightsSOs = 1;
			  }
			  else{
				forAll(neighbourCellsSOs,cell){
					if(vF[neighbourCellsSOs[cell]] < 1){
						pointHelper1 = midPoints[neighbourCellsSOs[cell]];
						pointHelper2 = faceSOs;
						weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
						neighbourWeightsSOs += weight;
						numberOfFuildNeighbours +=1;
						interpolateSOs += weight * p[neighbourCellsSOs[cell]];
						//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " stress " << stress[cell]);
					}
					//+fos
				}
				pointHelper1 = midPoints[secondOutside];
				pointHelper2 = faceSOs;
				weight = 1/(mag(pointHelper2 - pointHelper1) * mag(pointHelper2 - pointHelper1));
				neighbourWeightsSOs += weight;
				interpolateSOs += weight * pSOS;
			  }
			  interpolateSOs = 1 / neighbourWeightsFOs * interpolateSOs;
			  interpolateFOs = 1 / neighbourWeightsSOs * interpolateSOs;
			  
			  interpolatedP = 2 * interpolateFOs - interpolateSOs;
		  }
		  pStress[faceI] = interpolatedP;
		  //FOS

	  }
	

	  else if (pressureInterpolationMethod_ == "wnlci"){
		  labelList neighbourCells = mesh.cellCells(cellI);
		  if (pressureNeighbourWeighting_ == "equal"){
			numberOfFuildNeighbours = 0;
			pInterpol = 0;
			forAll (neighbourCells,cell){
				 if(vF[neighbourCells[cell]] < 1){
					numberOfFuildNeighbours +=1;
					pInterpol += p[neighbourCells[cell]];
					//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " pressure " << p[cell]);
				  }
			  }
			  //_DBO_("sum is " << pInterpol << " and number of neighbours " << numberOfFuildNeighbours);
			  pInterpol  /= numberOfFuildNeighbours;
			  pP[faceI] = pInterpol;
		  }
		  else if (pressureNeighbourWeighting_ == "distance"){
			//faceMid
			pInterpol = 0;
			numberOfFuildNeighbours = 0;
			scalar singleWeight;
			scalar testWeight = 0;
			forAll (neighbourCells,cell){
				 if(vF[neighbourCells[cell]] < 1){
					//numberOfFuildNeighbours += mag(midPoints[neighbourCells[cell]] - faceMid[faceI]);//TODO
					//singleWeight = mag(midPoints[neighbourCells[cell]] - faceMid[faceI]);
					pointHelper1 = faceCtrs[faceI];
					pointHelper2 = midPoints[neighbourCells[cell]];
					singleWeight = mag(pointHelper1 - pointHelper2);//mag(midPoints[neighbourCells[cell]]);//1;
					numberOfFuildNeighbours += singleWeight;
					//CellCenterPoint = midPoints[neighbourCells[cell]];
					pInterpol += p[neighbourCells[cell]]*singleWeight;
					//_DBO_("singleWeight = " << singleWeight);
					//_DBO_("cell " << cell <<" hat Wert " << numberOfFuildNeighbours << " pressure " << p[cell]);
				  }
			  }
			  //_DBO_("all weights Together = " << numberOfFuildNeighbours);
			  //_DBO_("sum is " << pInterpol << " and number of neighbours " << numberOfFuildNeighbours);
			  pInterpol  /= numberOfFuildNeighbours;
			  pP[faceI] = pInterpol;
		  }

	  }
	  else{
		  pP[faceI] 	 = 0;
	  }

	  //eigenValueTest = eigenValues(pStress[faceI]);
	  //scalar testInvariant = invariantIII(pStress[faceI]);
	  //_DBO_("eigenValues are " << eigenValueTest << "3rd Invariant is: " << testInvariant);

	  //compareFOS[faceI] = stress[firstOutside];
	  //compareSOS[faceI] = stress[secondOutside];
	  //pStress[faceI] = interpolatedTensorField;
	  

//      if( secondOutside == firstOutside )
//      { _DBI_ }

    }
	//we compare forces here!!
	/*
	forceFOS = Sn & compareFOS;
	forceSOS = Sn & compareSOS;
	forceInterpolated = Sn & pStress;
	scalar comFOS,comSOS,comInter = 0;
	forAll(forceFOS,faceI){
		scalar FOS,SOS,interpolated = 0;
		for(int axis = 0; axis <=2; axis++){
			FOS += forceFOS[faceI][axis]*forceFOS[faceI][axis];
			SOS += forceSOS[faceI][axis]*forceSOS[faceI][axis];
			interpolated += forceInterpolated[faceI][axis]*forceInterpolated[faceI][axis];

		}
		comFOS += sqrt(FOS);
		comSOS += sqrt(SOS);
		comInter += sqrt(interpolated);

	}
	_DBO_("here we compare all 3 methods:");
	_DBO_("firstOutside " << comFOS);
	_DBO_("secondOutside " << comSOS);
	_DBO_(stresstensorInterpolationMethod_ << comInter);
	*/


	end_time_all = std::chrono::steady_clock::now();
	
	point testPoint = midPoints[0];
	//_DBO_("test point " << testPoint);
	//(std::chrono::duration_cast<std::chrono::microseconds>(end_time_all - start_time_all))

	_DBO_("für Partikel "<< pPtr->idStr() << " war die benötigte Zeit (" << ((std::chrono::duration_cast<std::chrono::microseconds>(end_time_all - start_time_all)).count()) / 1000000.0 << ") seconds" );
    pPtr->calcFluidForces(pRef_, rho(pVol));
  }
}


// This method is one possible version of how to treat the fluid force mapping for
// particles which are smaller than the usual cell size.
// Here only the fluid velocity at the particle's CG is checked and then directly
// mapped to the first surface element, thus saving computational.
// The fluidForceField()[0] is later in the code distributed in parallel to other processors.
//void Foam::functionObjects::pManager::mapFluidForcesToSubCellParticle(volumetricParticle* pPtr, const symmTensorField& stress, const scalarField& p)
void Foam::functionObjects::pManager::mapFluidForcesToSubCellParticle(volumetricParticle* pPtr)
{
	vector cg = pPtr->getCg();
	label cellI = myMS_.findCell( cg );

	const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

	if(cellI >= 0) pPtr->velocityAtCgSubCellSize_ = U[cellI];
	else pPtr->velocityAtCgSubCellSize_ = vector::zero;

	/*forAll(pPtr->fluidForceField(), fIter)
	{
		pPtr->fluidForceField()[fIter] = pPtr->velocityAtCgSubCellSize_;
	}
	pPtr->fluidForceField()[0] = pPtr->velocityAtCgSubCellSize_;*/

	return;
}


void Foam::functionObjects::pManager::mapThermophoreticForcesToParticles()
{
  if( !fsi_ )
    return;

  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    volumetricParticle *pPtr = particleList_[i];

    Population& pop = popList_[pPtr->popId()-1];

    if( ! pop.withThermoPhoresis() )
      continue;

    // Retrieve temperature field
    const volScalarField& TVol = obr_.lookupObject<volScalarField>(TName_);
    const scalarField& T = TVol.internalField();//TVol();//Vora

    const pointField& faceCtrs = pPtr->triSurf().faceCentres();
    scalarField&     pT        = pPtr->thermoField();

    forAll(faceCtrs, faceI)
    {
      label cellI = myMS_.findCell( faceCtrs[faceI] );

      if( cellI == -1 )
      {
        pT[faceI]      = 0.0;
        continue;
      }

      pT[faceI]      = T[cellI];
    }
    pPtr->calcThermoForces(pop.thermophoreticFactor());
  }
}


void Foam::functionObjects::pManager::mapElectroMagneticForcesToParticles()
{
  if( !em_ )
    return;

  //_PDBO_("started mapping emagForces")

  // map EField, Polarization and surface Charge of main mesh to all particles
  for(label i = 0; i < nParticles_; i++)
  {
    volumetricParticle *pPtr = particleList_[i];

    Population& pop = popList_[pPtr->popId()-1];

    if( ! pop.withElectroMagnetic() )
      continue;

    // Retrieve electromagnetic field
    const volVectorField& EMVol = obr_.lookupObject<volVectorField>(EMName_);
    const volVectorField& PolVol = obr_.lookupObject<volVectorField>(PolName_);
    const volScalarField& sigmaVol = obr_.lookupObject<volScalarField>(sigmaName_);
    const vectorField& EM = EMVol.internalField(); //EMVol()//Vora
    const vectorField& Pol = PolVol.internalField(); //PolVol();//Vora
    const scalarField& sigma = sigmaVol.internalField(); //sigmaVol();//Vora

    const pointField& faceCtrs = pPtr->triSurf().faceCentres();
    vectorField&     pEM        = pPtr->electromagField(); // References to triSurf of particle
    vectorField&     pPol        = pPtr->polarField();
    scalarField&     pSigma       = pPtr->sigmaField();

    forAll(faceCtrs, faceI)
    {
      label cellI = myMS_.findCell( faceCtrs[faceI] );

      if( cellI == -1 )
      {
        pEM[faceI]      = vector::zero;
        pPol[faceI]	= vector::zero;
        pSigma[faceI]	= 0;
        continue;
      }

      pEM[faceI]      = EM[cellI]; // Set values of ref. triSurf
      pPol[faceI]      = Pol[cellI];
      pSigma[faceI]      = sigma[cellI];
    }
    if(pop.objectCharge()) pPtr->calcElectroMagForces(pop.objectCharge());
    else pPtr->calcElectroMagForces();
  }

  //_PDBO_("finished mapping emagForces")
}

void Foam::functionObjects::pManager::mapParticlePermittivityToFluid() //changes the rel. permittivity where the particle is
{
  if( !em_ )
    return;

  //_PDBO_("started mapping permittivity")

 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");

  volScalarField& realEpsilon = const_cast<volScalarField&>(
                            mesh.lookupObject<volScalarField>("epsilon")
                                                  );
  const pointField &midPoints = mesh.C().internalField();
  List<bool> inside(midPoints.size());


// map permittivity of all particles to main mesh
  for(label i = 0; i < nParticles_; i++) // for all particles
  {
    volumetricParticle *pPtr = particleList_[i];
    Population& pop = popList_[pPtr->popId()-1];
    if( ! pop.withElectroMagnetic() )
      continue;
    const scalar particlePermittivity = pop.epsilonr();
    //const scalar fluidPermittivity =
    // Use octree based search for all fluid cells lying inside particle
    pPtr->isInside(midPoints, backGroundGrid(), inside);

    forAll(inside, cellI)
    {

      if(!inside[cellI]) {
    	  realEpsilon[cellI] = 1.0004;
    	  continue;
      }

      realEpsilon[cellI] = particlePermittivity;
    }
  }

  //_PDBO_("finished mapping permittivity")
}

void Foam::functionObjects::pManager::mapParticleSigmaToFluid() //Maps the particles sigma field (induced surface charges) to the fluids sigma field
{
  if( !em_ )
    return;

  return;
  //_PDBO_("started mapping sigma")

 const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");

  volScalarField& realSigma = const_cast<volScalarField&>(
                            mesh.lookupObject<volScalarField>("sigma")
                                                  );
  const pointField &midPoints = mesh.C().internalField();
  List<bool> inside(midPoints.size());

  for(label i = 0; i < nParticles_; i++){
    //Only apply to particles for which electroMags are on
    volumetricParticle *pPtr = particleList_[i];
    Population& pop = popList_[pPtr->popId()-1];
    if( ! pop.withElectroMagnetic() )
      continue;

    pPtr->isInside(midPoints, backGroundGrid(), inside);

    const pointField& faceCtrs = pPtr->triSurf().faceCentres();
    scalarField&     pSigma       = pPtr->sigmaField();

    //Reset sigmaField to delete induced charges from last time-step
    const scalarField& vF = voidFrac();
    forAll(vF, labelI) {
	if(!vF[labelI] > 0) realSigma[labelI] = 0; //SOLLTE NOCH ZU sigma_fluid o.ä. geändert werden!!!!!!! TODO
    }


    //Write new sigma values calculated in volumetricParticle.C to the field
    forAll(faceCtrs, faceI)
    {
      label cellI = myMS_.findCell( faceCtrs[faceI] );
      if(cellI != -1) realSigma[cellI] = pSigma[faceI];
    }


  }

  //_PDBO_("finished mapping sigma")
}

void Foam::functionObjects::pManager::mapParticleDepositToFluid() // set cells where the particle has been deposited
{
	const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");
	const pointField &midPoints = mesh.C().internalField();

//Vora: OF-3
//	scalarField      &realDeposit        = deposit().internalField();

//Vora: OF-5.x
// one needs to replace instances of "boundaryField()" and "internalField()" with "boundaryFieldRef()" and "internalFieldRef()" if one wants to modify them in the code. The non-"Ref" calls are const now.
// internalField and internalFieldRef have been replaced by direct calls to the field variable. Eg: U.internalField() becomes U() for a const reference to the internal field, or U.ref() for non-const access.

	scalarField      &realDeposit        = deposit().ref();
	List<bool> inside(midPoints.size());

	// Reset the deposition field
	realDeposit = 0.0;

	for(label i = 0; i < nParticles_; i++)
	{
		volumetricParticle *pPtr = particleList_[i];
		//Population& pop = popList_[pPtr->popId()-1]; // TODO continue if particle should not have any deposit mechanic, add some switch or bool...
		 pPtr->isInside(midPoints, backGroundGrid(), inside);

		if (mag(pPtr->deposited_))
		{
		    forAll(inside, cellI)
		    {
		      if(!inside[cellI])
		        continue;
		      realDeposit[cellI] = 1.0;
		    }

		}
	}
}

void Foam::functionObjects::pManager::printKineticEnergy() const
{
	_DBO_("===== Kinetic Energy =====")
	scalar kinEn = 0;
	forAll(particleList_, prt)
		{
		kinEn += (particleList_[prt]->getKineticEnergy()).z();
		if(printKinetic_ == 2) _DBO_( particleList_[prt]->idStr()
				<< " :\ntransEn = " << (particleList_[prt]->getKineticEnergy()).x()
				<< " :\trotEn = " << (particleList_[prt]->getKineticEnergy()).y()
				<< " :\ttotEn = " << (particleList_[prt]->getKineticEnergy()).z() )
		}
	_DBO_("Total kin.en. :\t" << kinEn)
	_DBO_("===============")
}



const Foam::volSymmTensorField& Foam::functionObjects::pManager::devRhoReff() const
{
  typedef compressible::momentumTransportModel cmpTurbModel;//turbulenceModel cmpTurbModel; CHANGED
  typedef incompressible::momentumTransportModel icoTurbModel;//turbulenceModel icoTurbModel; CHANGED


  tmp<volSymmTensorField> tmpField( static_cast<volSymmTensorField*>(0) );


  if( !validDevRhoReff_ || !obr_.foundObject<volSymmTensorField>("devRhoReff") )
  {


    if
    (
        obr_.foundObject<cmpTurbModel>(momentumTransportModel::typeName)//cmpTurbModel::propertiesName)
    )
    {
        const cmpTurbModel& turb =
            obr_.lookupObject<cmpTurbModel>(momentumTransportModel::typeName);//(cmpTurbModel::propertiesName);

        tmpField = turb.devTau();//.devRhoReff();
    }
    else if
    (
        obr_.foundObject<icoTurbModel>(momentumTransportModel::typeName)
    )
    {
        const incompressible::momentumTransportModel& turb =//turbulenceModel& turb =
            obr_.lookupObject<icoTurbModel>(momentumTransportModel::typeName);//(icoTurbModel::propertiesName);

        tmpField = rho()*turb.devSigma();//.devReff();
    }
    else if
    (
        obr_.foundObject<fluidThermo>(fluidThermo::typeName)
    )
    {
        const fluidThermo& thermo =
            obr_.lookupObject<fluidThermo>(fluidThermo::typeName);

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        tmpField = -thermo.mu()*dev(twoSymm(fvc::grad(U)));
    }
    else if
    (
        obr_.foundObject<transportModel>("transportProperties")
    )
    {
        const transportModel& laminarT =
            obr_.lookupObject<transportModel>("transportProperties");

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        tmpField = -rho()*laminarT.nu()*dev(twoSymm(fvc::grad(U)));
    }
    else if
    (
        obr_.foundObject<twoPhaseMixture>("transportProperties")
    )
    {
        const incompressibleTwoPhaseMixture& laminarT =
                dynamic_cast<const incompressibleTwoPhaseMixture&>(
                                            obr_.lookupObject<transportModel>
                                            ("transportProperties")
                                                                    );

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        tmpField = -laminarT.mu()*dev(twoSymm(fvc::grad(U)));
    }
    else if
    (
        obr_.foundObject<multiphaseMixture>("transportProperties")
    )
    {_DBO_("Found multiphaseMixture")
        const multiphaseMixture& laminarT =
                dynamic_cast<const multiphaseMixture&>(
                                            obr_.lookupObject<transportModel>
                                            ("transportProperties")
                                                                    );

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        tmpField = -laminarT.mu()*dev(twoSymm(fvc::grad(U)));
    }
    else if
    (
        obr_.foundObject<dictionary>("transportProperties")
    )
    {
        const dictionary& transportProperties =
             obr_.lookupObject<dictionary>("transportProperties");

        dimensionedScalar nu(transportProperties.lookup("nu"));

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        tmpField = -rho()*nu*dev(twoSymm(fvc::grad(U)));
    }
    else
    {
        FatalErrorIn("Foam::functionObjects::pManager::devRhoReff()")
            << "No valid model for viscous stress calculation."
            << exit(FatalError);

        return volSymmTensorField::null();
    }
  } // end if


  if( !obr_.foundObject<volSymmTensorField>("devRhoReff") )
  {
    devRhoReffPtr_.reset(
                            new volSymmTensorField(
                                                   "devRhoReff",
                                                   tmpField
                                                  )
                        );

    if( writeDevRhoReff_ )
      devRhoReffPtr_->writeOpt() = IOobject::AUTO_WRITE;
    else
      devRhoReffPtr_->writeOpt() = IOobject::NO_WRITE;
  }
  else if( !validDevRhoReff_ )
  {
    devRhoReffPtr_() = tmpField();
  }

//  volSymmTensorField& devRhoReff = const_cast<volSymmTensorField&>(
//                             obr_.lookupObject<volSymmTensorField>("devRhoReff")
//                                                                 );

  validDevRhoReff_ = true;


  return devRhoReffPtr_();
}


Foam::tmp<Foam::volScalarField> Foam::functionObjects::pManager::rho() const
{
    if (rhoName_ == "rhoInf")
    {
       const fvMesh& mesh = refCast<const fvMesh>(obr_);
// 	const fvMesh& mesh = obr_.lookupObject<fvMesh>("data");


        return tmp<volScalarField>
        (
            new volScalarField
            (
                IOobject
                (
                    "rho",
                    mesh.time().timeName(),
                    mesh
                ),
                mesh,
                dimensionedScalar("rho", dimDensity, rhoRef_)
            )
        );
    }
    else
    {
        return(obr_.lookupObject<volScalarField>(rhoName_));
    }
}


Foam::scalar Foam::functionObjects::pManager::rho(const volScalarField& p) const
{
    if (p.dimensions() == dimPressure)
    {
        return 1.0;
    }
    else
    {
        if (rhoName_ != "rhoInf")
        {
            FatalErrorIn("pManager::rho(const volScalarField& p)")
                << "Dynamic pressure is expected but kinematic is provided."
                << exit(FatalError);
        }

        return rhoRef_;
    }
}



//} //End namespace functionObjects //Vora
//} // namespace Foam
// ************************************************************************* //
