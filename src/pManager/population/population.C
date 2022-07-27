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

#include "population.H"

#include "fvMesh.H"
#include "triSurface.H"
#include "indexedOctree.H"
#include "IFstream.H"
#include "Random.H"
#include <mpi.h>

#include "makros.H"
#include "tools/other/write.H"
#include <chrono>


namespace Foam
{


Population::Population():
               obr_(NULL),
               dict_(NULL),
               name_("N.N."),
               surfacePatchName_("surface"),
			   calcTotalLoadType_("general"),
               id_(-1),
               meshPath_(""),
               outputPath_(""),
               polyMeshPath_(""),
               nextParticleId_(0),
               bgSearchRadius_(0),
               rho_(0.),
               thermophoresis_(0),
		       electromag_(0),
               thermophoreticFactor_(1),
		       objectCharge_(0),
		       epsilonr_(0),
		       a0_(4e-10),
		       H_(0),// Hamaker-Konstante, 5e-19 für PSL, 45.3e-20 gold ; 6.5e-20 Glas Isrealachvili
			   adhesionReductionFactor_(1.0),
		       isStructure_(false),
			   contactCheck_(false),
			   nearWallEffects_(false),
			   oxidation_(false),
			   isPointParticle_(false),
               mapPointParticleMomentum_(false),
			   collidesWithOwnPopulation_(true),
			   distributeToAll_(true),
		       kpl_(0.93),// Restitutionskoeff.; laut Hiller 0.4 - 0.6 für Stoffe wie beispw. Quarz
		       ppl_(3e7),//scalar p_pl = 3*sigma_p; Fließspannung sigma_p // Plastischer Fließdruck nach Hersey und Rees, 1971; Paronen und Ilkka, 1996
			   elasticity_(1e10), // Magnitude of aluminium's elasticity
			   collForceConservation_(1),
		       mu_(0),
			   collDist_(0),
			   mappingDist_(0),
			   contactPointDist_(-1),
			   externalAcc_(vector::zero),
			   externalOmegaAcc_(vector::zero),
			   externalForce_(vector::zero),
			   externalTorque_(vector::zero),
			   minVecBB_(vector::zero),
			   maxVecBB_(vector::zero),
               container_(),
               bg_(0),
               injector_(0),
               constraintList_(0),
               nBornParticles_(0),
			   writeProperties_(true),
               writeForceField_(false),
               writePressureForceField_(false),
               writePressureForceDensityField_(false),
               writeStressForceField_(false),
               writeStressForceDensityField_(false),
			   generatePostprocFiles_(true),
               myMSPtr_(NULL),
               nFree_(0),
               nMaster_(0),
               nSlave_(0),
			   deleteOrphanedParticles_(true),
			   af_(0),//Pre-exponential factor or frequency factor (Af) for oxidation [1/s]
			   rTemp_(0),
			   aEnergy_(0),//Activation energy for oxidation (Ea) in [J/mol]
			   rGasConst_(8.314459848) //molar Gas constant - 8.3144598(48) [J/(K*mol)]
{}

Population::~Population()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    delete *iter;
  }
}

void  Population::read(
                          const objectRegistry &obr,
                          const bgGrid& bg,
                          const myMeshSearch *myMSPtr,
                          const dictionary &dict,
                          const word &popName,
                          label popId
                        )
{
  obr_     = &obr;
  bg_      = &bg;
  myMSPtr_ = myMSPtr;
  dict_    = &dict; // dictionary of pManager
  name_    = popName;
  id_      = popId;
  const dictionary *subDict = dict_->subDictPtr(name_);
  if( !subDict )
  {
    FatalErrorIn("Population::read(const dictionary& dict, const word &popName)")
                      << "Cannot find subdict of population '"
                      << name_ << "'!"
                      << exit(FatalError);
  }
  if( !subDict->readIfPresent<fileName>("meshPath", meshPath_) )
  {
    FatalErrorIn("Population::read(const dictionary& dict)")
              << "No mesh path for population '"
              << name_ << "' specified!"
              << nl << "Use 'meshPath  <filePath>;' to define."
              << exit(FatalError);
  }
  if( !subDict->readIfPresent<scalar>("rho", rho_) )
  {
    FatalErrorIn("Population::read(const dictionary& dict)")
              << "No density for population '"
              << name_ << "' specified!"
              << nl << "Use 'rho  <scalar>;' to define."
              << exit(FatalError);
  }
  subDict->readIfPresent<word>("surfacePatchName", surfacePatchName_);
  subDict->readIfPresent<Switch>("thermophoresis", thermophoresis_);
  subDict->readIfPresent<scalar>("thermophoreticFactor", thermophoreticFactor_);
  subDict->readIfPresent<Switch>("electromag", electromag_);
  subDict->readIfPresent<scalar>("objectCharge", objectCharge_);
  subDict->readIfPresent<scalar>("epsilonr", epsilonr_);
  subDict->readIfPresent<scalar>("a0", a0_);
  subDict->readIfPresent<scalar>("rTemp", rTemp_);
  subDict->readIfPresent<scalar>("hamaker", H_);
  subDict->readIfPresent<Switch>("isDeposited", isStructure_);
  subDict->readIfPresent<Switch>("isStructure", isStructure_);
  subDict->readIfPresent<Switch>("contactCheck", contactCheck_);
  subDict->readIfPresent<Switch>("nearWallEffects", nearWallEffects_);
  subDict->readIfPresent<scalar>("kpl", kpl_);
  subDict->readIfPresent<scalar>("ppl", ppl_);
  subDict->readIfPresent<scalar>("elasticity", elasticity_);
  subDict->readIfPresent<scalar>("energyConservColl",  collForceConservation_); // To make sure no problem with old cases due to namechange
  subDict->readIfPresent<scalar>("collForceConservation",  collForceConservation_);
  subDict->readIfPresent<scalar>("mu", mu_);
  subDict->readIfPresent<scalar>("collisionDistance", collDist_);
  subDict->readIfPresent<scalar>("collDist", collDist_);
  subDict->readIfPresent<scalar>("contactPointDistance", contactPointDist_);
  subDict->readIfPresent<vector>("externalAcceleration", externalAcc_);
  subDict->readIfPresent<vector>("externalOmegaAcceleration", externalOmegaAcc_);
  subDict->readIfPresent<vector>("externalForce", externalForce_);
  subDict->readIfPresent<vector>("externalTorque", externalTorque_);
  subDict->readIfPresent<vector>("minVecBB", minVecBB_);
  subDict->readIfPresent<vector>("maxVecBB", maxVecBB_);
  subDict->readIfPresent<Switch>("writeProperties", writeProperties_);
  subDict->readIfPresent<Switch>("writeForceField", writeForceField_);
  subDict->readIfPresent<Switch>("writePressureForceField", writePressureForceField_);
  subDict->readIfPresent<Switch>("writePressureForceDensityField", writePressureForceDensityField_);
  subDict->readIfPresent<Switch>("writeStressForceField", writeStressForceField_);
  subDict->readIfPresent<Switch>("writeStressForceDensityField", writeStressForceDensityField_);
  subDict->readIfPresent<Switch>("collidesWithOwnPopulation", collidesWithOwnPopulation_);
  subDict->readIfPresent<Switch>("hasSubCellSize", isPointParticle_); // old name in controlDict, LEGACY
  subDict->readIfPresent<Switch>("isPointParticle", isPointParticle_);
  subDict->readIfPresent<Switch>("mapPointParticleMomentum", mapPointParticleMomentum_);
  subDict->readIfPresent<Switch>("distributeToAll", distributeToAll_);
  subDict->readIfPresent<double>("adhesionReductionFactor", adhesionReductionFactor_);
  subDict->readIfPresent<word>("calcTotalLoadType", calcTotalLoadType_);
  subDict->readIfPresent<Switch>("generatePostprocFiles", generatePostprocFiles_);


  // If collision distance is not set, set it according to voidFraction mapping distance
  if(collDist_ == 0)
  {
	   // Set distance used for mapping of voidFraction with help of bounding box
#if 1
	  triSurface* popTriSurf = new triSurface(obr_->time().path()/meshPath_.name());
#else
	  triSurface* popTriSurf;
	  if(!Pstream::parRun()) popTriSurf = new triSurface(obr_->time().path()/meshPath_.name());
	  else popTriSurf = new triSurface(obr_->time().path()+"/../"+meshPath_.name());
#endif
	   boundBox bb = boundBox::invertedBox;

	   forAll(popTriSurf->points(), pointi)
	   {
	       bb.min() = ::Foam::min(bb.min(), popTriSurf->points()[pointi]);
	       bb.max() = ::Foam::max(bb.max(), popTriSurf->points()[pointi]);
	   }

	   mappingDist_ = 0.8 * mag( bb.min() - bb.max() ); // could be 0.5 to save more time, but a bit risky depending on how concave the shape is

	   collDist_ = mappingDist_ * 1.1;

	  _PDBO_("'collisionDistance' not set for population '" << id() <<"'. \n"
	  << "Setting through bounding box with additional 10% to collDist_ = " << collDist_)

	  delete popTriSurf;
  }
  else
  {
	  mappingDist_ = collDist_;
  }
  subDict->readIfPresent<scalar>("mappingDistance", mappingDist_);

  subDict->readIfPresent<Switch>("deleteOrphanedParticles", deleteOrphanedParticles_);

  if(distributeToAll_ && minVecBB_ == maxVecBB_)
  {

	  _DBO_("WARNING! WARNING! WARNING!"
			  << "\ndistributeToAll set to true, but bounding box for particle deletion not set"
			  <<"\nthrough vectors minVecBB and maxVecBB in population's " << name_ << " dictionary!!!"
			  <<"\n\tSet automatically to: "
			  << "\n\tminVecBB = vector(-VGREAT, -VGREAT , -VGREAT)"
			  << "\n\tmaxVecBB = vector(VGREAT, VGREAT , VGREAT)\n")

	    minVecBB_ = vector(-VGREAT, -VGREAT , -VGREAT);
	    maxVecBB_ = vector(VGREAT, VGREAT , VGREAT);
  }


  if( !subDict->readIfPresent<label>("bgSearchRadius", bgSearchRadius_) )
  {
    FatalErrorIn("Population::read(const dictionary& dict)")
              << "No background grid search radius  for population '"
              << name_ << "' specified!"
              << nl << "Use 'bgSearchRadius  <label>;' to define."
              << exit(FatalError);
  }

  if(subDict->readIfPresent<Switch>("oxidation", oxidation_) && (!subDict->readIfPresent<scalar>("af", af_) || !subDict->readIfPresent<scalar>("aEnergy", aEnergy_)) )
  {
    FatalErrorIn("Population::read(const dictionary& dict)")
              << "Soot oxidation is activated for population '"
              << name_ << "', But the values of either frequency factor or activation Energy are not defined."
              << nl << "Please, Use 'af  <scalar>;' to define frequency factor and 'aEnergy <scalar>;' to define activation energy."
              << exit(FatalError);
  }


  word     journalName = "particleJournal-" + name_;

  if( subDict->found("injector") )
  {
    const word& injectorName = subDict->lookupOrDefault<word>("injector", "none");
    if(injectorName != "none")
    {
      injector_ = Injector::New(dict_->subDict(injectorName));
    }
  }

  if( subDict->found("constraints") )
  {
    const List<word> constraintNameList(subDict->lookup("constraints"));

    if(constraintNameList.size() != 0)
    {

      Info << "Apply constraints to population '" << name_ << "':" << endl;
      constraintList_.setSize(constraintNameList.size());

      forAll(constraintNameList, i)
      {
        constraintList_[i] =
                  Constraint::New(
                                   dict_->subDict(constraintNameList[i]),
                                   *obr_,
                                   myMSPtr_
                                 );

        constraintList_[i]().info();
      }
    }
  }

  Info << "Type for integrating surface forces (calcTotalLoadType in populationDict) is set to " << calcTotalLoadType_ << endl;

  outputPath_   = "particles"/name_;
  polyMeshPath_ = "constant"/name_/"polyMesh";

  string noOutput = " &> /dev/null";

  if(Pstream::nProcs() > 1)
  {
    string cpCmd = "cp ";
    cpCmd += meshPath_ + " " + obr_->time().path();
    system(cpCmd + noOutput);
  }

  if(Pstream::master())
  {
    string mkdir    = "mkdir -p ";
    string mkdirCmd = mkdir + polyMeshPath_;
    system(mkdirCmd + noOutput);

    string ln       = "cd system; ln -sf ";
    string lnCmd    = ln + ". " + name_;
    system(lnCmd + noOutput);

    Info << nl << "Preparing " << meshPath_ << " for 'polyMesh' output format... ";

    triSurface triSurf(meshPath_);

    toolWriteMesh(triSurf, polyMeshPath_);

    Info << " done!" << nl;
  }



  if(contactPointDist_ < 0 && !isStructure())
  {
	  triSurface* popTriSurf = new triSurface(obr_->time().path()/meshPath_.name());
	  boundBox bb = boundBox::invertedBox;
	  forAll(popTriSurf->points(), pointi)
	  {
	      bb.min() = ::Foam::min(bb.min(), popTriSurf->points()[pointi]);
	      bb.max() = ::Foam::max(bb.max(), popTriSurf->points()[pointi]);
	  }

	   contactPointDist_ = mag( bb.min() - bb.max() ) * 0.01;

	  _PDBO_("'contactPointDist' not set for population '" << id() <<"'. \n"
	  	  << "Setting it to 1% of the bounding box to contactPointDist_ = " << contactPointDist_)

	   delete popTriSurf;
  }
}


label Population::restoreFromJournal()
{
  List<scalar>  valueList;

  word     journalName = "particleJournal-" + name_;

  if(obr_->time().processorCase())
  {
    journalName += "-" + _ITOS_(Pstream::myProcNo(), _N_DIGITS_PROCNUM_);
  }

  fileName caseDir = "./";
  caseDir.toAbsolute(); // the only way to get rid of the "processor#"
  fileName timeDir = caseDir / obr_->time().timeName();
  fileName fName   = timeDir/journalName;

  IFstream journalFile(fName);

  if( !journalFile )
  {
    return 0;
  }

  if(!journalFile.opened())
  {
    WarningIn("void volumetricParticle::writeJournal()")
                << "Could not open file '" << fName << "'" << nl;
  }

  List<journalEntry> journal(journalFile);

  forAll(journal, i)
  {
    journalEntry& entry = journal[i];
//    _DBO_("gonna instantiate with " << entry.second())
    instantiateParticle(entry.first(), entry.second());
  }

  return journal.size();
}

void Population::distributeParticles(
                                      const bgGrid& bg
                                    )
{
  if(distributeToAll_) return; // distribute to all = do not distribute particles at all, but distribute fields to all

  pDistValuesPlan     planSend    (Pstream::nProcs());
  pDistValuesPlanCont planSendCont(Pstream::nProcs());
  pDistValuesPlanCont planRecvCont(Pstream::nProcs());

  const fvMesh& mesh= refCast<const fvMesh>(*obr_);

  // Auxiliary container to circumvent renaming problem:
  // In some situations the keys of some entries must be changed.
  // This could only be done via deleting and
  // inserting under new key. This procedure must not be
  // done within iterator loop.
  HashTable<volumetricParticle*> auxNewName;

  forAllIter( HashTable<volumetricParticle*>, container_, iter)
  {

	volumetricParticle *pPtr = *iter;

	// In serial runs particles are not delete according to the
	// background grid (further below), but simply with the help of the mesh search
	if(!Pstream::parRun())
	{
		//if( ( myMSPtr_->findCell( pPtr->getCg() ) == -1 || pPtr->getCg().x() >= 0.00011 ) && deleteOrphanedParticles_ )
		if( ( myMSPtr_->findCell( pPtr->getCg() ) == -1 ) && deleteOrphanedParticles_ )
		{
			_PDBO_("Deleting orphaned particle " << iter.key() <<  " with cg = " << pPtr->getCg())
			container_.erase(iter);
		}
	      continue;
	}

    word pIdStr = iter.key();
    List<label> pIdx(3);
    volumetricParticle::stateType state = pPtr->getState();

    point pCg   = pPtr->cg();
    label pProc = bg.getProcessor(pCg); // maybe obsolete
    bg.getAllNeighbourProcessorsCompact(pIdx, pPtr->particleProcs_, bgSearchRadius_);
    bg.getIndex(pCg, pIdx);

    // Delete orphaned particle (whose CG is out of the domain): Default case
    // But it will not be deleted if "deleteOrphandedParticles" has been specified as "false/no".
    if( pProc == -1)
    {
    	if( deleteOrphanedParticles_ )
    	{
    		_PDBO_("Deleting orphaned particle " << pIdStr <<  " with cg = " << pCg)
    		container_.erase(iter);
    		continue;
    	}
    }


    if(Pstream::parRun())
    {
    switch( state )
    {

    case volumetricParticle::slave :

      // Has particle entered own mesh?
      if( bg.isMine(pCg) )
      {
    	  List<label> neighbourProcs;
    	          pTransValues pTrans;

    	          bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);
    	          // Write particle's id string and value list into transfer structure
    	          strcpy(pTrans.pIdStr, pIdStr.c_str());
    	          pPtr->valuesToList(pTrans.valueList);


    	          // send particle to all neighbouring processors
    	          forAll(neighbourProcs, neighbourI)
    	          {
    	            // send to processor number procNo
    	            label toProcNo = neighbourProcs[neighbourI];

    	            if( (toProcNo == -1) || (toProcNo == Pstream::myProcNo()))
    	              continue;


    	            // Add transfer structure to plan
    	            planSend[toProcNo].insert(pTrans);
    	          }


        pPtr->setMaster();
        //_PDBO_("slave to master: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'm';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);
        break;
      }
      // Has particle left the processor boarder (e.g. moved further into
      // foreign area)? Then delete particle.
      if( !bg.isNeighbourOfProc(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        _PDBO_("Deleting escaped slave particle " << pIdStr << " with cg = " << pCg)
        container_.erase(iter);
      }

      break;

    case volumetricParticle::master :

      // Has particle left own mesh?
//      if( myMSPtr_->findCell( pCg ) == -1 )
//      if( !bg.isMine(pCg) ) //Original
      if( !bg.isMine(pCg) && pProc != -1)  //Vora: in case of "deleteOrphanedParticles" sets to "No", particle could have only slave states after some time steps.
      {
        pPtr->setSlave();
        //_PDBO_("master to slave: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 's';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);

        break;
      }
      // Has particle left the processor boarder (e.g. moved further into
      // own area)? Then release from slaves.
      if( !bg.nearProcessorBoarder(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        pPtr->setFree();
        // Rename particle: erase from container and mark for new insertion
        //_PDBO_("master to free: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'f';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);

//        break;
      }

//      break;
        // no break here: distribute anyway, because master particle can reach
        // new borders without getting free


    case volumetricParticle::free :


      // Has particle approached boarder? Then turn state into master and
      // send to neighbouring procs
      if( bg.nearProcessorBoarder(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        List<label> neighbourProcs;
        pTransValues pTrans;

        bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);
        // Write particle's id string and value list into transfer structure
        strcpy(pTrans.pIdStr, pIdStr.c_str());
        pPtr->valuesToList(pTrans.valueList);


        // send particle to all neighbouring processors
        forAll(neighbourProcs, neighbourI)
        {
          // send to processor number procNo
          label toProcNo = neighbourProcs[neighbourI];

          if( (toProcNo == -1) || (toProcNo == Pstream::myProcNo()))
            continue;


          // Add transfer structure to plan
          planSend[toProcNo].insert(pTrans);
        }

        pPtr->setMaster();
        // Rename particle: erase from container and mark for new insertion
        //_PDBO_("free to master: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'm';
        auxNewName.insert(pIdStr, pPtr);

        container_.erase(iter);
      }

      break;

    default :

      FatalErrorIn("Population::distributeParticles(...)")
                          << "Undefined particle state '"
                          << state << "'!"
                          << exit(FatalError);

    } // end switch
    } // end if(parRun)


  } // end forAllIter

  if(!Pstream::parRun())
      return;


  // Insert particles marked for insertion under new name
  forAllConstIter( HashTable<volumetricParticle*>, auxNewName, iter)
  {
	//_PDBO_("inserting particle " << iter.key() << "from auxNewName to container_")
    container_.insert(iter.key(), *iter);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Make "plan" contiguous; copy planSend to planSendCont
    label nToRecv[_MAX_N_PROCESSES_];
    label nToSend[_MAX_N_PROCESSES_];

    forAll(planSend, procI)
    {
      nToSend[procI] = planSend[procI].size();
      planSendCont[procI].resize(nToSend[procI]);

      // For each pTransValues in the DLList
      label pTransI = 0;
      forAllConstIter(pTransValuesDLList , planSend[procI], iter)
      {
        // copy pTransValues into contiguous list
        planSendCont[procI][pTransI++] = *iter;
      }
    }
    // Ask for number of particles to receive from others and
    // tell other processors how many particles to receive from me
    for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
    {
        if( procNo == Pstream::myProcNo() )
          continue;

        UIPstream::read(
                          UOPstream::commsTypes::nonBlocking,
                          procNo,
                          reinterpret_cast<char*>(nToRecv + procNo),
                          sizeof(label),
                          id_
                        );

        UOPstream::write(
                          UOPstream::commsTypes::nonBlocking,
                          procNo,
                          reinterpret_cast<char*>(nToSend + procNo),
                          sizeof(label),
                          id_
                        );
    }
    UPstream::waitRequests(0);

    // Send and receive particle data
    for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
    {
      if( procNo == Pstream::myProcNo() )
        continue;

      planRecvCont[procNo].resize(nToRecv[procNo]);

      UIPstream::read(
                        UOPstream::commsTypes::nonBlocking,
                        procNo,
                        reinterpret_cast<char*>( planRecvCont[procNo].data() ),
                        nToRecv[procNo] * sizeof(pTransValues),
                        id_
                      );


      UOPstream::write(
                        UOPstream::commsTypes::nonBlocking,
                        procNo,
                        reinterpret_cast<const char*>( planSendCont[procNo].cdata() ),
                        nToSend[procNo] * sizeof(pTransValues),
                        id_
                      );

    }
    UPstream::waitRequests(0);

    // Instantiate received particles as slave particles
    forAll(planRecvCont, procI)
    {
      forAll(planRecvCont[procI], pTransI)
      {
        pTransValues& pT = planRecvCont[procI][pTransI];
        pT.pIdStr[0] = 's';

        if( !container_.found(pT.pIdStr) )
        {
          instantiateParticle(pT.pIdStr, pT.valueList);
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
}



void Population::distributeParticlesMPI(
                                      const bgGrid& bg
                                    )
{
  if(distributeToAll_) return;

  pDistValuesPlan     planSend    (Pstream::nProcs());
  pDistValuesPlanCont planSendCont(Pstream::nProcs());
  pDistValuesPlanCont planRecvCont(Pstream::nProcs());

  const fvMesh& mesh= refCast<const fvMesh>(*obr_);

  // Auxiliary container to circumvent renaming problem:
  // In some situations the keys of some entries must be changed.
  // This could only be done via deleting and
  // inserting under new key. This procedure must not be
  // done within iterator loop.
  HashTable<volumetricParticle*> auxNewName;

  forAllIter( HashTable<volumetricParticle*>, container_, iter)
  {

	volumetricParticle *pPtr = *iter;

	// In serial runs particles are not delete according to the
	// background grid (further below), but simply with the help of the mesh search
	if(!Pstream::parRun())
	{
		//if( ( myMSPtr_->findCell( pPtr->getCg() ) == -1 || pPtr->getCg().x() >= 0.00011 ) && deleteOrphanedParticles_ )
		if( ( myMSPtr_->findCell( pPtr->getCg() ) == -1 ) && deleteOrphanedParticles_ )
		{
			_PDBO_("Deleting orphaned particle " << iter.key() <<  " with cg = " << pPtr->getCg())
			container_.erase(iter);
		}
	      continue;
	}

    word pIdStr = iter.key();
    List<label> pIdx(3);
    volumetricParticle::stateType state = pPtr->getState();

    point pCg   = pPtr->cg();
    label pProc = bg.getProcessor(pCg); // maybe obsolete
    bg.getAllNeighbourProcessorsCompact(pIdx, pPtr->particleProcs_, bgSearchRadius_);
    bg.getIndex(pCg, pIdx);

    // Delete orphaned particle (whose CG is out of the domain): Default case
    // But it will not be deleted if "deletOrphandedParticles" has been specified as "false/no".
    if( pProc == -1)
    {
    	if( deleteOrphanedParticles_ )
    	{
    		_PDBO_("Deleting orphaned particle " << pIdStr <<  " with cg = " << pCg)
    		container_.erase(iter);
    		continue;
    	}
    }


    if(Pstream::parRun())
    {
    switch( state )
    {

    case volumetricParticle::slave :

      // Has particle entered own mesh?
      if( bg.isMine(pCg) )
      {
    	  List<label> neighbourProcs;
    	          pTransValues pTrans;

    	          bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);
    	          // Write particle's id string and value list into transfer structure
    	          strcpy(pTrans.pIdStr, pIdStr.c_str());
    	          pPtr->valuesToList(pTrans.valueList);


    	          // send particle to all neighbouring processors
    	          forAll(neighbourProcs, neighbourI)
    	          {
    	            // send to processor number procNo
    	            label toProcNo = neighbourProcs[neighbourI];

    	            if( (toProcNo == -1) || (toProcNo == Pstream::myProcNo()))
    	              continue;


    	            // Add transfer structure to plan
    	            planSend[toProcNo].insert(pTrans);
    	          }


        pPtr->setMaster();
        //_PDBO_("slave to master: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'm';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);
        break;
      }
      // Has particle left the processor boarder (e.g. moved further into
      // foreign area)? Then delete particle.
      if( !bg.isNeighbourOfProc(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        _PDBO_("Deleting escaped slave particle " << pIdStr << " with cg = " << pCg)
        container_.erase(iter);
      }

      break;

    case volumetricParticle::master :

      // Has particle left own mesh?
//      if( myMSPtr_->findCell( pCg ) == -1 )
//      if( !bg.isMine(pCg) ) //Original
      if( !bg.isMine(pCg) && pProc != -1)  //Vora: in case of "deleteOrphanedParticles" sets to "No", particle could have only slave states after some time steps.
      {
        pPtr->setSlave();
        //_PDBO_("master to slave: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 's';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);

        break;
      }
      // Has particle left the processor boarder (e.g. moved further into
      // own area)? Then release from slaves.
      if( !bg.nearProcessorBoarder(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        pPtr->setFree();
        // Rename particle: erase from container and mark for new insertion
        //_PDBO_("master to free: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'f';
        auxNewName.insert(pIdStr, pPtr);
        container_.erase(iter);

//        break;
      }

//      break;
        // no break here: distribute anyway, because master particle can reach
        // new borders without getting free


    case volumetricParticle::free :


      // Has particle approached boarder? Then turn state into master and
      // send to neighbouring procs
      if( bg.nearProcessorBoarder(pIdx, Pstream::myProcNo(), bgSearchRadius_) )
      {
        List<label> neighbourProcs;
        pTransValues pTrans;

        bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);
        // Write particle's id string and value list into transfer structure
        strcpy(pTrans.pIdStr, pIdStr.c_str());
        pPtr->valuesToList(pTrans.valueList);


        // send particle to all neighbouring processors
        forAll(neighbourProcs, neighbourI)
        {
          // send to processor number procNo
          label toProcNo = neighbourProcs[neighbourI];

          if( (toProcNo == -1) || (toProcNo == Pstream::myProcNo()))
            continue;


          // Add transfer structure to plan
          planSend[toProcNo].insert(pTrans);
        }

        pPtr->setMaster();
        // Rename particle: erase from container and mark for new insertion
        //_PDBO_("free to master: " << pIdStr << " with new status " << pPtr->getState())
        pIdStr[0] = 'm';
        auxNewName.insert(pIdStr, pPtr);

        container_.erase(iter);
      }

      break;

    default :

      FatalErrorIn("Population::distributeParticles(...)")
                          << "Undefined particle state '"
                          << state << "'!"
                          << exit(FatalError);

    } // end switch
    } // end if(parRun)


  } // end forAllIter

  if(!Pstream::parRun())
      return;


  // Insert particles marked for insertion under new name
  forAllConstIter( HashTable<volumetricParticle*>, auxNewName, iter)
  {
	//_PDBO_("inserting particle " << iter.key() << "from auxNewName to container_")
    container_.insert(iter.key(), *iter);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Make "plan" contiguous; copy planSend to planSendCont
  label nToRecv[_MAX_N_PROCESSES_];
  label nToSend[_MAX_N_PROCESSES_];

  forAll(planSend, procI)
  {
    nToSend[procI] = planSend[procI].size();
    planSendCont[procI].resize(nToSend[procI]);

    // For each pTransValues in the DLList
    label pTransI = 0;
    forAllConstIter(pTransValuesDLList , planSend[procI], iter)
    {
      // copy pTransValues into contiguous list
      planSendCont[procI][pTransI++] = *iter;
    }
  }

  MPI_Status status; //Vora_stdMPI
  MPI_Request send_request; //Vora_ stdMPI
  MPI_Request recv_request; //Vora_stdMPI

  // Ask for number of particles to receive from others and
  // tell other processors how many particles to receive from me
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
      if( procNo == Pstream::myProcNo() )
        continue;

            MPI_Irecv(reinterpret_cast<char*>(nToRecv + procNo), sizeof(label), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &recv_request); //Vora: MPI
            MPI_Isend(reinterpret_cast<char*>(nToSend + procNo), sizeof(label), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &send_request);	//Vora: MPI

            MPI_Wait(&send_request, &status); //Vora: MPI
            MPI_Wait(&recv_request, &status); //Vora: MPI
  }

  // Send and receive particle data
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;

    planRecvCont[procNo].resize(nToRecv[procNo]);

    MPI_Irecv(reinterpret_cast<char*>( planRecvCont[procNo].data() ), nToRecv[procNo] * sizeof(pTransValues), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &recv_request); //Vora
    MPI_Isend(reinterpret_cast<const char*>( planSendCont[procNo].cdata() ), nToSend[procNo] * sizeof(pTransValues), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &send_request);	//Vora

    MPI_Wait(&send_request, &status); //Vora
    MPI_Wait(&recv_request, &status); //Vora
  }


  // Instantiate received particles as slave particles
  forAll(planRecvCont, procI)
  {
    forAll(planRecvCont[procI], pTransI)
    {
      pTransValues& pT = planRecvCont[procI][pTransI];
      pT.pIdStr[0] = 's';

      if( !container_.found(pT.pIdStr) )
      {
        instantiateParticle(pT.pIdStr, pT.valueList);
      }
    }
  }
}


// Distributes the velocity of ALL point particles SIMULTANOUSLY
// to all processors.
void Population::distributePointVelocity()
{
	if(!Pstream::parRun()) return;

	int pCount = container_.size();

	double* veloArray = new double[pCount*3]; // contains the three velocity components of all point particles
	double* veloArrayReduced = new double[pCount*3]; // necessary for the root process during the MPI broadcast operation

	// Fill array with velocities
	int i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		veloArray[i] = container_[iter.key()]->velocityAtCgSubCellSize_.x();
		veloArray[i+1] = container_[iter.key()]->velocityAtCgSubCellSize_.y();
		veloArray[i+2] = container_[iter.key()]->velocityAtCgSubCellSize_.z();
		veloArrayReduced[i] = veloArrayReduced[i+1] = veloArrayReduced[i+2] = 0;
		i=i+3;
	}

	List<label> neighbourProcs;
	neighbourProcs.resize(Pstream::nProcs());
	for(int i = 0; i < Pstream::nProcs(); i++)
	{
		neighbourProcs[i] = i;
	}

	// Collect velocities from all processors in
	// root processors (proc 0) velocity array
	// Particles that are outside of a processor's domain
	// have a velocity equal to zero
	MPI_Reduce(veloArray, veloArrayReduced, (pCount*3), MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // Now the root processor distributes the reduced
    // velocity array back to all other processors
    MPI_Bcast(veloArrayReduced, (pCount*3), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

	// Update particle velocities
	i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		container_[iter.key()]->velocityAtCgSubCellSize_.x() = veloArrayReduced[i];
		container_[iter.key()]->velocityAtCgSubCellSize_.y() = veloArrayReduced[i+1];
		container_[iter.key()]->velocityAtCgSubCellSize_.z() = veloArrayReduced[i+2];
		i=i+3;
	}

	//////// ------ Solve cluster mpi trash ....
	i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		veloArray[i] = container_[iter.key()]->getCg().x();
		veloArray[i+1] = container_[iter.key()]->getCg().y();
		veloArray[i+2] = container_[iter.key()]->getCg().z();
		veloArrayReduced[i] = veloArrayReduced[i+1] = veloArrayReduced[i+2] = 0;
		i=i+3;
	}

	MPI_Reduce(veloArray, veloArrayReduced, (pCount*3), MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Bcast(veloArrayReduced, (pCount*3), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

	i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		container_[iter.key()]->getCg().x() = veloArrayReduced[i];
		container_[iter.key()]->getCg().y() = veloArrayReduced[i+1];
		container_[iter.key()]->getCg().z() = veloArrayReduced[i+2];
		i=i+3;
	}

	delete veloArray;
	delete veloArrayReduced;
}



#if 1
// General method for distributing fields of all types
// between all processors in one single large communication.
// Assumes each object of the same population has identical
// field sizes.
template <typename Type>
void Population::distributeForcesNew    (
                                       Field<Type>& (volumetricParticle::* fieldGetter) ()
                                     )
{
	if(!Pstream::parRun() || container_.size() < 1) return;

	// Dirty way to get populations field size, probably something way smarter available...
	int fCount;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
		{
			fCount = (container_[iter.key()]->*fieldGetter)().size();
			//const Field<Type>& pForceField = (container_[iter.key()]->*fieldGetter)();
			break;
		}

	int nProcs = Pstream::nProcs();
	// Fill array with field data of type Type
	// Buffers for MPI will point to them and communicate them as char-data
	Field<Type> allForceFields(fCount*container_.size());
	Field<Type> allForceFieldsCommunicate(fCount*container_.size());

	int pCount = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		for(int i = 0; i < fCount; i++)
		{
			allForceFields[pCount*fCount+i] = (container_[iter.key()]->*fieldGetter)()[i];
		}
		pCount++;
    }

	// Flatten buffer to char through reinterpret_cast so it can
	// be distributed through basic MPI routines
	std::size_t totalBufSize = allForceFields.size() * sizeof(Type);
	char* buffer = reinterpret_cast<char*>(allForceFields.data());
	char* bufferCommunicate = reinterpret_cast<char*>(allForceFieldsCommunicate.data());
	allForceFieldsCommunicate = allForceFields - allForceFields;

	// Collect and sum all field data on processer 0
    for(int pr = 1; pr < Pstream::nProcs(); pr++)
    {
    	if(Pstream::myProcNo() == pr) MPI_Send(buffer, totalBufSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    	else if(Pstream::myProcNo() == 0)
    	{
    		MPI_Recv(bufferCommunicate, totalBufSize, MPI_CHAR, pr, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		allForceFields = allForceFields + allForceFieldsCommunicate;
    	}
    	MPI_Barrier(MPI_COMM_WORLD); // better safe than sorry
    }

	// Distribute buffer data
    MPI_Bcast(buffer, totalBufSize, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

	// Adjust local fields with distributed data
	pCount = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		(container_[iter.key()]->*fieldGetter)() = SubField<Type>(allForceFields, fCount, pCount*fCount);
		pCount++;
	}

	distributeParticleValues();
 }
#endif

void Population::distributeParticleValues()
{
	if(!Pstream::parRun()) return;

	int pCount = container_.size();

	double* dataArray = new double[pCount*9];

	int i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		dataArray[i]   = container_[iter.key()]->getCg().x();
		dataArray[i+1] = container_[iter.key()]->getCg().y();
		dataArray[i+2] = container_[iter.key()]->getCg().z();
		dataArray[i+3] = container_[iter.key()]->getVelocity().x();
		dataArray[i+4] = container_[iter.key()]->getVelocity().y();
		dataArray[i+5] = container_[iter.key()]->getVelocity().z();
		dataArray[i+6] = container_[iter.key()]->getOmega().x();
		dataArray[i+7] = container_[iter.key()]->getOmega().y();
		dataArray[i+8] = container_[iter.key()]->getOmega().z();
		i=i+9;
	}


    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(dataArray, (pCount*9), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

	i = 0;
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		container_[iter.key()]->velocityAtCgSubCellSize_.x() = dataArray[i];
		container_[iter.key()]->velocityAtCgSubCellSize_.y() = dataArray[i+1];
		container_[iter.key()]->velocityAtCgSubCellSize_.z() = dataArray[i+2];
		container_[iter.key()]->getVelocity().x()            = dataArray[i+3];
		container_[iter.key()]->getVelocity().y()            = dataArray[i+4];
		container_[iter.key()]->getVelocity().z()            = dataArray[i+5];
		container_[iter.key()]->getOmega().x()               = dataArray[i+6];
		container_[iter.key()]->getOmega().y()               = dataArray[i+7];
		container_[iter.key()]->getOmega().z()               = dataArray[i+8];
		i=i+9;
	}

	delete dataArray;
}


#if 0
// Old general method for distributing fields of all types
// between neighbouring processors with individual processor
// to processor communications
template <typename Type>
void Population::distributeForces    (
                                       const bgGrid& bg,
                                       Field<Type>& (volumetricParticle::* fieldGetter) ()
                                     )
{
//  if(!Pstream::parRun() || Pstream::myProcNo() == 0)
	if(!Pstream::parRun())
    return;

  pDistForcePlan        planRecv    (Pstream::nProcs());
  pDistForcePlanCont    planRecvCont(Pstream::nProcs());
  pDistForcePlanCont    planSendCont(Pstream::nProcs());

  // Collect all receives for any master particles
  forAllIter( HashTable<volumetricParticle*>, container_, iter)
  {
    word pIdStr = iter.key();
    volumetricParticle *pPtr = *iter;
    volumetricParticle::stateType state = pPtr->getState();

    // Nothing to do for free and slave particles
    if(
        (   state == volumetricParticle::free
        || state == volumetricParticle::slave )
      )
      continue;

    List<label> pIdx(3);
    point pCg    = pPtr->cg();
    bg.getIndex(pCg, pIdx);

    List<label> neighbourProcs;
    pTransForce pTrans;

    if (distributeToAll_)
    {
    	neighbourProcs.resize(Pstream::nProcs());

    	for(int i = 0; i < Pstream::nProcs(); i++)
    	{
    		neighbourProcs[i] = i;
    	}

    	//_PDBO_("neighbourProcs of " << pIdStr << " : " << neighbourProcs)
    }
    else bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);

     // Write particle's id string and value list into transfer structure
    strcpy(pTrans.pIdStr, pIdStr.c_str());

    // Do not store directly into particle's force field
    pTrans.forceBuffer = 0;

    // plan to receive particle forces from all neighbouring processors
    forAll(neighbourProcs, neighbourI)
    {
      // recv from processor number fromProcNo
      label fromProcNo = neighbourProcs[neighbourI];

      if(
             fromProcNo == -1
          || fromProcNo == Pstream::myProcNo()
        )
        continue;

      // Add transfer structure to plan
      planRecv[fromProcNo].insert(pTrans);
    }
  } // end forAllIter

  // Execute planned transfers
  label nToRecv[_MAX_N_PROCESSES_];		// at location n, nToRecv contains the number of particles that processor n receives from me
  label nToSend[_MAX_N_PROCESSES_];

  // Count particle forces that I want to receive from each processor
  forAll(planRecv, procI)
  {
    nToRecv[procI] = planRecv[procI].size();
    planRecvCont[procI].resize(nToRecv[procI]);

    // For each pTransForce in the DLList
    label pTransI = 0;
    forAllConstIter(pTransForceDLList, planRecv[procI], iter)
    {
      // copy pTransForce into contiguous list
      planRecvCont[procI][pTransI++] = *iter;
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Status status;	// Vora_stdMPI
  MPI_Request send_request; // Vora_stdMPI
  MPI_Request recv_request; // Vora_stdMPI

  // Tell other processors how many particles they have to send to me
  // and ask for the number of particles that I have to send to them
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
	  if( procNo == Pstream::myProcNo())
        continue;

            MPI_Isend(&nToRecv[procNo], sizeof(int), MPI_INT, procNo, id_, MPI_COMM_WORLD, &send_request);
            MPI_Irecv(&nToSend[procNo], sizeof(int), MPI_INT, procNo, id_, MPI_COMM_WORLD, &recv_request);

            MPI_Wait(&send_request, &status); //Vora: stdMPI
            MPI_Wait(&recv_request, &status); //Vora: stdMPI

  }

  // Tell others which particles I want to receive
  // and ask which particles to send
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;

    planSendCont[procNo].resize(nToSend[procNo]);

        MPI_Isend(reinterpret_cast<const char*>( planRecvCont[procNo].cdata() ), nToRecv[procNo] * sizeof(pTransForce), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &send_request); //Vora: stdMPI
        MPI_Irecv(reinterpret_cast<char*>( planSendCont[procNo].data() ), nToSend[procNo] * sizeof(pTransForce), MPI_CHAR, procNo, id_, MPI_COMM_WORLD, &recv_request); //Vora: stdMPI

        MPI_Wait(&send_request, &status); //Vora: stdMPI
        MPI_Wait(&recv_request, &status); //Vora: stdMPI

  }
  // At first fire up all non blocking send requests
  //
  // for each processor
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;

    // for each slave particle
    forAll(planSendCont[procNo], pTransI)
    {
      // fetch transfer info
      const pTransForce* pTrans = &(planSendCont[procNo][pTransI]);
      word pIdStr(pTrans->pIdStr);
      if(!distributeToAll_) pIdStr[0] = 's';
      //_PDBO_("hashtabling for processor " << procNo)

      // fetch particle and its force field to be transferred
      volumetricParticle* pPtr = container_[pIdStr];
      const Field<Type>& pForceField = (pPtr->*fieldGetter)();

      // fetch buffer info
      label  bufSize = pForceField.size() * sizeof(Type);
      const char* buffer = reinterpret_cast<const char*>(
                                                            pForceField.cdata()
                                                            );
            MPI_Isend(buffer, bufSize, MPI_CHAR, procNo, pTransI, MPI_COMM_WORLD, &send_request); //Vora: MPI
    }
  }

  // Secondly fire up corresponding non blocking recv requests
  // but process received data immediately to save buffer memory.
  // (There are possibly several slaves belonging to one master. This results
  // in several singular received force fields that have to be added
  // to one force field of the master particle.)
  //
  Field<Type> forceBuf;
  // for each processor
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;
    // for each received slave particle
    forAll(planRecvCont[procNo], pTransI)
    {
      // fetch transfer info
      const pTransForce* pTrans = &(planRecvCont[procNo][pTransI]);
      word pIdStr(pTrans->pIdStr);
      // fetch particle and its force field to which the received forces
      // have to be added
      //_PDBO_("hashtabling for processor " << procNo)
      volumetricParticle* pPtr = container_[pIdStr];
      Field<Type>& pForceField = (pPtr->*fieldGetter)();

      // prepare buffer
      forceBuf.resize( pForceField.size() );
      label  bufSize = pForceField.size() * sizeof(Type);
      char* buffer = reinterpret_cast<char*>(
                                                 forceBuf.data()
                                               );
      MPI_Irecv(buffer, bufSize, MPI_CHAR, procNo, pTransI, MPI_COMM_WORLD, &recv_request); //Vora: MPI

      MPI_Wait(&recv_request, &status); //Vora: MPI

      pForceField += forceBuf;
    }
  }


//  UPstream::waitRequests(0); // superfluous
// Order of MPI execution has been changed: The Third fire up all non blocking recv request loop is shifted after the non blocking send request loop

  // Fourth fire up corresponding non blocking send requests of summed forces
  //
  // for each processor
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;
    // for each received slave particle
    forAll(planRecvCont[procNo], pTransI)
    {
      // fetch transfer info
      const pTransForce* pTrans = &(planRecvCont[procNo][pTransI]);
      word pIdStr(pTrans->pIdStr);

      // fetch particle and its force field to which the received forces
      // have to be added
      //_PDBO_("hashtabling for processor " << procNo)
      volumetricParticle* pPtr = container_[pIdStr];
      const Field<Type>& pForceField = (pPtr->*fieldGetter)();

      // prepare buffer
      label  bufSize = pForceField.size() * sizeof(Type);
      const char* buffer = reinterpret_cast<const char*>(
                                                        pForceField.cdata()
                                                        );
      MPI_Isend(buffer, bufSize, MPI_CHAR, procNo, pTransI, MPI_COMM_WORLD, &send_request); //Vora: stdMPI
    }
  }

  // Third fire up all non blocking recv requests of summed forces
  //
  // for each processor
  for(label procNo = 0; procNo < Pstream::nProcs(); ++procNo)
  {
    if( procNo == Pstream::myProcNo() )
      continue;
    // for each slave particle
    forAll(planSendCont[procNo], pTransI)
    {
      // fetch transfer info
      const pTransForce* pTrans = &(planSendCont[procNo][pTransI]);
      word pIdStr(pTrans->pIdStr);
      if(!distributeToAll_) pIdStr[0] = 's';

      // fetch particle and its force field to store summed forces
      //_PDBO_("hashtabling for processor " << procNo)
      volumetricParticle* pPtr = container_[pIdStr];
      Field<Type>& pForceField = (pPtr->*fieldGetter)();

      // fetch buffer info
      label  bufSize = pForceField.size() * sizeof(Type);
      char* buffer = reinterpret_cast<char*>(
                                              pForceField.data()
                                            );
      MPI_Irecv(buffer, bufSize, MPI_CHAR, procNo, pTransI, MPI_COMM_WORLD, &recv_request); //Vora: MPI

      MPI_Wait(&recv_request, &status); //Vora: stdMPI
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
}
#endif

void Population::writeParticlePropertiesAndGeometries(const bgGrid& bg)
{
  // timePath nonsense?

  if(writePressureForceField_ || writePressureForceDensityField_)
    distributeForcesNew(&volumetricParticle::pressureField );

  if( writeStressForceField_ || writeStressForceDensityField_ )
    distributeForcesNew(&volumetricParticle::stressField );

  if(!writeProperties_) return;

  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    volumetricParticle* pPtr = *iter;

    pPtr->writeProperties();
    pPtr->writeGeometry();
  }
}

void Population::writeJournal()
{
  if( container_.size() == 0 )
  {
    return;
  }

  List<scalar> valueList(_N_PARTICLE_PARAMETERS_);

  word     journalName = "particleJournal-" + name_;

  if(obr_->time().processorCase())
  {
    journalName += "-" + _ITOS_(Pstream::myProcNo(), _N_DIGITS_PROCNUM_);
  }

  fileName caseDir = "./";
  caseDir.toAbsolute(); // the only way to get rid of the "processor#"
  fileName timeDir = caseDir / obr_->time().timeName();
  fileName fName   = timeDir/journalName;

  //_PDBO_("file test " << timeDir << " name= " << journalName)

  string mkDirCmd = "mkdir -p "; mkDirCmd += timeDir;
  string rmCmd    = "rm -rf ";   rmCmd += fName;
  system(rmCmd);     // delete existent, if any
  system(mkDirCmd);  // create path, if not existent

  OFstream journalFile(fName);

  if(!journalFile.opened())
  {
    WarningIn("void volumetricParticle::writeJournal()")
                << "Could not open file '" << fName << "'" << nl;
  }


  List<journalEntry> journal(container_.size());
  label count = 0;

  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    word pIdStr = iter.key();
    const volumetricParticle *pPtr = *iter;

    if(distributeToAll_)
    {
      if(pPtr->isFree())
    	  pIdStr.replace(0, name_.size(), "f");
      else if(pPtr->isMaster())
    	  pIdStr.replace(0, name_.size(), "m");
      else if(pPtr->isSlave())
    	  pIdStr.replace(0, name_.size(), "s");
      else
        FatalErrorIn("Population::writeJournal(...)")
                                        << "Undefined particle state!"
                                        << exit(FatalError);
    }
    else
    {
      if(pPtr->isFree())
        pIdStr[0] = 'f';
      else if(pPtr->isMaster())
        pIdStr[0] = 'm';
      else if(pPtr->isSlave())
        pIdStr[0] = 's';
      else
        FatalErrorIn("Population::writeJournal(...)")
                                        << "Undefined particle state!"
                                        << exit(FatalError);
    }

    pPtr->valuesToList(valueList);

    journalEntry entry(pIdStr, valueList);
    journal[count++] = entry;

  }
  journalFile << journal;
}

volumetricParticle* Population::instantiateParticle(
                                                     const List<scalar>& valueList,
                                                     bool inMeshOnly
                                                   )
{
  word keyName;
  if (distributeToAll_ && Pstream::myProcNo() == 0) keyName = "m-";
  else if (distributeToAll_ ) keyName = "s-";
  else  keyName = "f-";
  if (distributeToAll_) keyName += _ITOS_(0, _N_DIGITS_PROCNUM_); // Processor 0 gets all
  else keyName += _ITOS_(Pstream::myProcNo(), _N_DIGITS_PROCNUM_);
  keyName += "-";
  keyName += _ITOS_(id_, _N_DIGITS_POPNUM_);
  keyName += "-";
  keyName += _ITOS_(getParticleId(), _N_DIGITS_PARTICLENUM_);
  return instantiateParticle(keyName, valueList, inMeshOnly);
}

volumetricParticle* Population::instantiateParticle(
                                                     const word& keyName,
                                                     const scalar* valueList,
                                                     bool inMeshOnly
                                                   )
{
  List<scalar> list(_N_PARTICLE_PARAMETERS_);

  forAll(list, listI)
  {
    list[listI] = valueList[listI];
  }
  return instantiateParticle(keyName, list, inMeshOnly);
}

volumetricParticle* Population::instantiateParticle(
                                                     const word& keyName,
                                                     const List<scalar>& valueList,
                                                     bool inMeshOnly
                                                   )
{
  volumetricParticle::stateType state = volumetricParticle::free;
  word pIdStr = keyName;

  //_PDBO_("instantiated " << pIdStr)

  if(pIdStr[0] == 'f')
  {
    state = volumetricParticle::free;
    nBornParticles_++;
  }
  else if(pIdStr[0] == 'm')
    state = volumetricParticle::master;
  else if(pIdStr[0] == 's')
    state = volumetricParticle::slave;
  else
     FatalErrorIn("Population::instantiateParticle(...)")
                                << "Undefined particle state '"
                                << pIdStr[0] << "'!"
                                << exit(FatalError);

  pIdStr.replace(0, 1, name_);
  volumetricParticle *pPtr = new volumetricParticle(
                                                     pIdStr,
                                                     meshPath_,
                                                     id_,
                                                     rho_,
                                                     obr_->time(),
                                                     *bg_,
													 bgSearchRadius_,
                                                     state,
                                                     surfacePatchName_,
                                                     writeForceField_,
                                                     writePressureForceField_,
                                                     writePressureForceDensityField_,
                                                     writeStressForceField_,
                                                     writeStressForceDensityField_,
													 this
                                                   );

  pPtr->listToValues(valueList);
  // Now, the geomtry, kinetics and inertia are setup

  if( inMeshOnly && ( myMSPtr_->findCell( pPtr->cg() ) == -1 ) && !distributeToAll_ ) //Vora: Particle, whose CG is out of domain, will not be deleted if..
  {																 // Vora: ...users have defined "no" for "deleteOrphanedParticles".
	if( deleteOrphanedParticles_ || (Pstream::myProcNo() != 0))
	 {
		_PDBO_("Deleting particle " << pPtr->idStr() << " at cg = " << pPtr->cg())
       delete pPtr;
       return 0;
     }
	else
	{
		// _PDBO_("Particle with cg = " << pPtr->cg() << " is out of domain, but still it is injected out of domain at defined position.")
		Info << endl << "Particle with cg = " << pPtr->cg() << " is out of domain,"
				<< "but still it is injected out of domain at defined position "
				<< "and it will be considered by processor 0." << endl;
	}
  }

  if(distributeToAll_) container_.insert(pIdStr, pPtr);
  else container_.insert(keyName, pPtr);

  generateParticleId();

  pPtr->setExternals(externalAcc_, externalOmegaAcc_, externalForce_, externalTorque_);

#if 0
{
    vector  eulerAxis;

  pPtr->orientationToEulerAxis(eulerAxis);

  _PDBO_("instantiated " << pPtr->idStr() << nl
          << "centerOfGravity     " << pPtr->getCg() << nl
          << "orientation         " << eulerAxis << nl
          << "velocity            " << pPtr->getVelocity() << nl
          << "acceleration        " << pPtr->getAcceleration() << nl
          << "angularVelocity     " << pPtr->getOmega() << nl
          << "angularAcceleration " << pPtr->getOmegaAcc() << nl
        )

}
#endif
  return pPtr;
}

void Population::renameParticle(const word& oldName, const word& newName)
{
  HashTable<volumetricParticle*>::iterator iter = container_.find(oldName);

  if (iter == container_.cend())
  {
    FatalErrorIn("Population::renameParticle(const word& oldName, const word& newName)")
    << oldName << " not found in table."
    << exit(FatalError);
  }

  volumetricParticle* const pPtr = *iter;

  container_.erase(iter);
  container_.insert(newName, pPtr);
}

label Population::generateParticleId()
{
  return nextParticleId_++;
}

label Population::degenerateParticleId()
{
  return nextParticleId_--;
}

label Population::getParticleId()
{
  return nextParticleId_;
}

void Population::nParticlesProc() const
{
  label nProcs   = Pstream::nProcs();
  label myProcNo = Pstream::myProcNo();

  nFree_.resize(nProcs);   nFree_   = 0;
  nMaster_.resize(nProcs); nMaster_ = 0;
  nSlave_.resize(nProcs);  nSlave_  = 0;

  label countFree   = 0;
  label countMaster = 0;
  label countSlave  = 0;

  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    if( (*iter)->isFree() )
    {
      ++countFree;
      continue;
    }
    if( (*iter)->isMaster() )
    {
      ++countMaster;
      continue;
    }
    if( (*iter)->isSlave() )
    {
      ++countSlave;
    }
  }

  nFree_  [myProcNo] = countFree;
  nMaster_[myProcNo] = countMaster;
  nSlave_ [myProcNo] = countSlave;

  Pstream::listCombineGather(nFree_  , maxEqOp<label>() );
  Pstream::listCombineGather(nMaster_, maxEqOp<label>() );
  Pstream::listCombineGather(nSlave_ , maxEqOp<label>() );
}

void Population::printStats() const
{
  nParticlesProc(); // count and scatter

  if( !Pstream::master() )
    return;

  Info << endl << "Population id = " << id_ << ", name = '" << name_ << "'" << nl << "proc#\t";

  for(label proc = 0; proc < Pstream::nProcs(); ++proc)
  {
    Info << _ITOS_(proc, 3) << "\t";
  }

  Info << endl << "free\t";

  forAll(nFree_, proc)
  {
    Info << _ITOS_(nFree_[proc], 3) << "\t";
  }

  Info << endl << "master\t";

  forAll(nMaster_, proc)
  {
    Info << _ITOS_(nMaster_[proc], 3) << "\t";
  }

  Info << endl << "slave\t";

  forAll(nSlave_, proc)
  {
    Info << _ITOS_(nSlave_[proc], 3) << "\t";
  }

  Info << endl;
}


void Population::move(scalar relax, int subiteration)
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
	// Ignore particles that have been deactivated through deposition
	if( (*iter)->deposited_ || isStructure())
		{
			(*iter)->getVelocity() = vector::zero;
			(*iter)->getAverageVelocity() = vector::zero;
			(*iter)->getOmega() = vector::zero;
			(*iter)->getAverageOmega() = vector::zero;
			continue;
		}

    if ((*iter)->contactPartners_.size() == 0)
    	{
    	//	_DBO_((*iter)->idStr() << " is moving alone")
    		(*iter)->move(relax, constraintList_, subiteration);
    	}
    else
    	{
    	// _DBO_((*iter)->idStr() << " is moving with partners")
    		(*iter)->moveWithContactPartners(relax);
    	}
  }
}


void Population::checkNearWallEffects() {

	// If population is just a structure for deposition
	// it is not necessary to check for contacts
	if( isStructure_ )
	{
		forAllConstIter(HashTable<volumetricParticle*>, container_, iter)
		{
			(*iter)->deposited_ = true;
		}
		return;
	}
	//distributeParticles(*bg_); // Particles have moved before so their new positions need to be known by the processors

	const fvMesh& mesh = refCast<const fvMesh>(*obr_);
	const volScalarField y = obr_->lookupObject<volScalarField>("wallDistance");
	const volVectorField wallN = obr_->lookupObject<volVectorField>("wallN");
	const scalar pi = constant::mathematical::pi;

	const scalar hamaker = H_;

	forAllConstIter(HashTable<volumetricParticle*>, container_, iter)
	{
		if(container_.size() < 1) break;
		vectorField& contactForceField = (*iter)->contactForceField();
		if( (*iter)->deposited_ ) continue; // For the case that checkNearWallEffects is called from CheckForContacts

		const pointField& faceCtrs = (*iter)->triSurf().faceCentres();
		//const pointField& faceN = (*iter)->triSurf().faceNormals(); //magnitude isn't equal to face area!!!
		const pointField& faceN = (*iter)->Sf();
		forAll(faceCtrs, faceI)
		{

			//Berechne distanz zur triSurf
			label cellI = myMSPtr_->findCell(faceCtrs[faceI]);
			if (cellI < 0)
				continue;
			vector triToCc = faceCtrs[faceI] - mesh.C().internalField()[cellI]; // Distance STL face centre to mesh cell centre
			vector wallVec = wallN[cellI] / ( mag(wallN[cellI]) + VSMALL ); //Normierter Walldist vector der Zelle
			vector faceVec = faceN[faceI]; //Face Normale der triSurf
			scalar dist = y[cellI] - (triToCc & wallVec);

			// Contact force according to plate-plate
			contactForceField[faceI] = (-1.) * hamaker * faceVec
					* ((faceVec / (mag(faceVec) + VSMALL)) & ((-1.) * wallVec))
					/ (pi * 6 * pow(dist, 3) + VSMALL);

			//Coulombsche Reibung
			const cell& cFaces = mesh.cells()[cellI];
			forAll(cFaces, cFaceI) {
				const face& fc = mesh.faces()[cFaces[cFaceI]];
				if(mesh.isInternalFace(cFaces[cFaceI])) continue;

				vector velo = (*iter)->getVelocity();
				if(velo != vector::zero) {
					velo -= (wallVec & velo) * wallVec;
					velo /= mag(velo) + VSMALL;
					vector normalForce = contactForceField[faceI];
					contactForceField[faceI] = normalForce - velo * mu_ * mag(normalForce);
				}
			}
		}
	}
}

void Population::checkForContacts(List<volumetricParticle*> allParticles) {

	// If population is just a structure for deposition
	// it is not necessary to check for contacts
	if( isStructure_ )
	{
		forAllConstIter(HashTable<volumetricParticle*>, container_, iter)
		{
			(*iter)->deposited_ = true;
		}
		return;
	}

	const fvMesh& mesh = refCast<const fvMesh>(*obr_);
	const pointField &midPoints = mesh.C().internalField();
	const volScalarField& depo = obr_->lookupObject<volScalarField>("deposit");
	const volScalarField& voidFrac = obr_->lookupObject<volScalarField>(
			"voidFrac");
	const volScalarField y = obr_->lookupObject<volScalarField>("wallDistance");
	const volVectorField wallN = obr_->lookupObject<volVectorField>("wallN");
	const scalar pi = constant::mathematical::pi;

	//distributeParticles(*bg_); // Particles have moved before so their new positions need to be known by the processors

	forAllConstIter(HashTable<volumetricParticle*>, container_, iter)
	{
		if( (*iter)->deposited_ ) continue; // No need to check for already deposited particles

		((*iter)->shareVectorField())[0] = vector::zero;
		bool foundContactFace = false;
		bool bounced = false;
		const pointField& faceCtrs = (*iter)->triSurf().faceCentres();
		const pointField& faceN = (*iter)->triSurf().faceNormals();
		scalar kinEn = ((*iter)->getKineticEnergy()).z();
		label cg_label = myMSPtr_->findCell((*iter)->cg());
		scalar dp = 2 * (*iter)->radEVS_;
		vector v_vec = (*iter)->getVelocity();
		//_PDBO_("velocity: " << v_vec)
		//scalar sigma_p = 62.5e6; // SiO2 600kp/mm² ; Fließspannung
		//scalar p_pl = 3*sigma_p; // Plastischer Fließdruck nach Hersey und Rees, 1971; Paronen und Ilkka, 1996
		scalar d_k; // Durchmesser der Kontaktfläche - vllt über Abflachungsfaktor analog zu Alexander Haarmann

		scalar volFac = 2; // Faktor volFac absolut arbiträr gewählt
		vector n_vec(vector::max);
		label minFace1;
		label minFace2;
		scalar minDist = VGREAT;
		volumetricParticle::vecCmpt pair;
		pair.mag = VGREAT;
		pointField contactPoint(1);

		// Check for collision with other deposited particle
		//if(!(*iter)->deposited_ && allParticles.size() > 1) {
		if (allParticles.size() > 1) {
			forAll(depo, cellI)
			{ // ------- forAll -------
				if (depo[cellI] != 0) {
					// Check if depo belongs to current particle
					List<bool> inside(1);
					pointField tempContactPoint(1);
					tempContactPoint[0] = mesh.C().internalField()[cellI];
					(*iter)->isInside(tempContactPoint, *bg_, inside);
					if (!inside[0])
						continue;

						forAllConstIter(List<volumetricParticle*>, allParticles,iter2)
						{
							if ((*iter)->cg() == (*iter2)->cg())
								continue;
							const pointField& faceCtrs2 =
									(*iter2)->triSurf().faceCentres();

							forAll(faceCtrs, faceI)
							{ // ------- forAll -------
								forAll(faceCtrs2, faceI2)
								{ // ------- forAll -------
									volumetricParticle::vecCmpt pairTemp;
									pairTemp = (*iter)->getDistanceComponents(
											faceI, (*iter2), faceI2);
									if (pairTemp.mag < pair.mag) {
										pair = pairTemp;
										minFace1 = faceI;
										minFace2 = faceI2;
										n_vec = (*iter2)->normals()[faceI2]
												* (-1.0); // surface normals of STL point into the fluid
										contactPoint[0] = faceCtrs2[faceI2];
										foundContactFace = true;
										/*_PDBO_(
												"Found contact with other particle at face centre "
														<< contactPoint[0]
														<< " with face normal "
														<< n_vec)*/
									}
								}
							}
						}
					}	// end of if(depo[cellI] ...
				}
			} // END of other deposited particle collision check

		if (!foundContactFace) {
			forAll(faceCtrs, faceI)
			{ // ------- forAll -------
				label cellI = myMSPtr_->findCell(faceCtrs[faceI]);
				scalar cellDist = volFac
						* pow((1. * mesh.V()[cellI]), (1. * 1 / 3));
				if (y[cellI] > cellDist || cellI < 0)
					continue;

				//_PDBO_("cellI = " << cellI << "\ty[cellI] = " << y[cellI] << "\tcellDist = " << cellDist)
				const cell& cFaces = mesh.cells()[cellI];
				scalar cosAlpha = -1;

				//_PDBO_("---------- start faceLoop ----------\n" << wallN[cellI])
				forAll(cFaces, cFaceI)
				{
					const face& fc = mesh.faces()[cFaces[cFaceI]];
					if (mesh.isInternalFace(cFaces[cFaceI]))
						continue;

					fc.points(mesh.points());
					const vector fNormal = fc.normal(mesh.points());

					cosAlpha = (fNormal & v_vec)
							/ (mag(fNormal) * mag(v_vec) + VSMALL);
					if ((cosAlpha >= 0) || kinEn <= VSMALL) { // Make sure particle is moving TOWARDS wall
						double pairTemp = mag(
								(faceCtrs[faceI] - fc.centre(mesh.points()))
										& fNormal / (mag(fNormal) + VSMALL));

						if (pairTemp <= pair.mag) {
							pair.mag = pairTemp;
							contactPoint[0] = faceCtrs[faceI];
							foundContactFace = true;
							n_vec = fNormal;
							/*_PDBO_(
									"contactMesh: " << fc.centre(mesh.points())
											<< "\t contactTriSurf: "
											<< faceCtrs[faceI]
											<< "\t distance: " << pairTemp)*/
							//break;
						}
					}

				}
				//_PDBO_("---------- end faceLoop ----------")
			}
		} // END of wall collision check

		// Process contact if it happened
		if (foundContactFace) {
			//n_vec = pair.n + pair.t;
			n_vec = n_vec / ( mag(n_vec) + VSMALL );

			//adh. and def. energy are calculated
			double h_pl = dp * (v_vec & n_vec) * sqrt(1 - kpl_ * kpl_)
					* sqrt((*iter)->getRho() / (6 * ppl_ + VSMALL));
			if (h_pl < 0)
				h_pl *= -1.0;
			d_k = sqrt(h_pl * dp);
			scalar defEn = 0.5 * ppl_ * pi * dp * h_pl * h_pl;
			double adhEn = 0; // adhEn in D.Maugis, Contact, Adhesion and Rupture of Elastic Solids
			adhEn = H_ * dp * h_pl / (12 * a0_ * a0_ + VSMALL);

			// Use energy as criterion for deposition or bounce
			//if (kinEn)
			//	(*iter)->bounce(n_vec, kpl_, mu_); // Get new velocity components for energy balance
			(*iter)->contactNormal_ = n_vec;

			/*_PDBO_("\n kin. Energy is: " << kinEn)
			_PDBO_("\n def. Energy is: " << defEn)
			_PDBO_("\n adh. Energy is: " << adhEn)*/

			if (kinEn - (defEn + adhEn) <= 0 || true) { // SET TRUE TO ALWAYS DEPOSIT
				(*iter)->contactPoint_ = contactPoint[0];
				(*iter)->dk_ = d_k;
				(*iter)->deposited_ = true;
				_PDBO_("\n particleDepo for pop " << name_)
			} else if (!(*iter)->deposited_) {
				//v_vec *= 1 - ( adhEn / ( kinEn + VSMALL ) ) ;
				bounced = true;
				_PDBO_(
						"\n particleBounce for pop " << name_
								<< " with final v_vec: "
								<< (*iter)->getVelocity())
			}

			/*_PDBO_("WATCH OUT")
			 _PDBO_("WARNING!!!!! IMPLEMENTED WITH ABORT DUE TO DEPOSITION OR BOUNCE!!!!")
			 _PDBO_("WATCH OUT")
			 FatalError
			 << "Exit due to deposition or bounce" << nl
			 << exit(FatalError);*/

		} // END of processing contacts
		else
		{
			(*iter)->deposited_ = false;
		}

		// Put collision data into sharing fields
		if (bounced) {
			((*iter)->shareVectorField())[0] = (*iter)->contactPoint_;
			((*iter)->shareScalarField())[1] = 1;
			((*iter)->shareVectorField())[1] = (*iter)->getVelocity();
			((*iter)->shareVectorField())[2] = (*iter)->getOmega();
		} else {
			((*iter)->shareVectorField())[0] = vector::zero;
			((*iter)->shareScalarField())[1] = 0;
			//((*iter)->shareVectorField())[1] = vector::zero;
			//((*iter)->shareVectorField())[2] = vector::zero;
		}
		((*iter)->shareScalarField())[0] = ((*iter)->deposited_) ? 1 : 0;
		((*iter)->shareScalarField())[2] = ((*iter)->deposited_) ? d_k : 0;
		((*iter)->shareVectorField())[3] =
				((*iter)->deposited_) ? n_vec : vector::zero;

	} // end forAllIter

// ============================ Contacts finished here. Now only distribute relevant information to other processors

	distributeForcesNew(&volumetricParticle::shareVectorField);
	distributeForcesNew(&volumetricParticle::shareScalarField);
	forAllConstIter(HashTable<volumetricParticle*>, container_, iter)
	{
		if (((*iter)->shareScalarField())[1] != 0) { // bounced
			(*iter)->getVelocity() = ((*iter)->shareVectorField())[1];
			(*iter)->getOmega() = ((*iter)->shareVectorField())[2];
			(*iter)->contactPoint_ = vector::zero;
			(*iter)->deposited_ = false;
		}
		if (((*iter)->shareScalarField())[0]) { //deposited
			(*iter)->deposited_ = true;
			(*iter)->dk_ = ((*iter)->shareScalarField())[2];
			if (((*iter)->contactNormal_ == vector::zero))
				(*iter)->contactNormal_ = ((*iter)->shareVectorField())[3];
			if (((*iter)->contactPoint_ == vector::zero))
				(*iter)->contactPoint_ = ((*iter)->shareVectorField())[0];
		} else
			(*iter)->deposited_ = false;
	}

}


void Population::subCyclingSaveState()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->subCyclingSaveState();
  }
}

void Population::subCyclingRestoreState()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->subCyclingRestoreState();
  }
}

void Population::iterativeCouplingSavePoints()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->iterativeCouplingSavePoints();
  }
}

void Population::iterativeCouplingSaveForces()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->iterativeCouplingSaveForces();
  }
}

void Population::iterativeCouplingRestorePoints()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->iterativeCouplingRestorePoints();
  }
}

void Population::iterativeCouplingRestoreForces()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->iterativeCouplingRestoreForces();
  }
}

void Population::iterativeCouplingRelaxForces(scalar relax)
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    (*iter)->iterativeCouplingRelaxForces(relax);
  }
}

void Population::inject()
{
  if( !injector_.valid() )
    return;

  List<List<scalar> > toInject;
  injector_().inject(obr_->time(), toInject);

  label totalToInject = toInject.size();
  char aux[16];

  forAll(toInject, labelI)
  {
    List<scalar>& particleValues = toInject[labelI];
    instantiateParticle(particleValues, true);
    if( !(labelI%10) )
    {
      scalar donePercent = (labelI+1.0)/totalToInject * 100;
      sprintf(aux, "%3.2f", donePercent);
      Info << "\r\tInjection " << aux << "\% done.";
    }
  }
  Info << "\r\tInjection " << 100 << "\% done.         "<< endl;
}

void Population::endOfExecution()
{
  forAllConstIter( HashTable<volumetricParticle*>, container_, iter)
  {
    volumetricParticle* pPtr = *iter;

    pPtr->endOfExecution();
  }
}

void Population::deleteParticle(word &key)
{
	  const char prefixList[] = {'f', 'm', 's'};
	  char prefix[] = "x";

	  for(label i = 0; i < 3 ; ++i)
	  {
	    word  aux = key;
	    prefix[0] = prefixList[i];

	    if(!distributeToAll_) aux.replace(0, name_.size(), prefix);

	    if( container_.found(aux) )
	      container_.erase(aux);
	  }
	  distributeParticles(*bg_);
}

void Population::deleteParticlesBB()
{
	if(!distributeToAll_) return;
	/*scalar minX = -1e3;
	scalar maxX = 11e-5;
	scalar minY = -8e-5;
	scalar maxY = 8e-5;
	scalar minZ = -3e-4;
	scalar maxZ = 3e-4;*/

	scalar minX = minVecBB_.x();
	scalar maxX = maxVecBB_.x();
	scalar minY = minVecBB_.y();
	scalar maxY = maxVecBB_.y();
	scalar minZ = minVecBB_.z();
	scalar maxZ = maxVecBB_.z();

	List<volumetricParticle*> partnersToDelete;

	// Check if particle fulfils delete conditions
	forAllIter(HashTable<volumetricParticle*>, container_, iter)
	{
		if(((*iter)->getCg().x() > maxX || (*iter)->getCg().x() < minX
			||	(*iter)->getCg().y() > maxY || (*iter)->getCg().y() < minY
			||	(*iter)->getCg().z() > maxZ || (*iter)->getCg().z() < minZ)
				 && !(*iter)->myPop_->isStructure())
		{
			// Make sure to also delete its partners
			// as otherwise rotational components might
			// go haywire
			(*iter)->addContactPartnersPartners();
			partnersToDelete.append((*iter)->contactPartners_);

		    // Delete particle
			_PDBO_("Deleting " << (*iter)->idStr() << "at cg = " << (*iter)->getCg() << "\nand with velo = " << (*iter)->getVelocity())
			container_.erase(iter);
		}
	}

	// Now delete corresponding partners
	for(int i = 0; i < partnersToDelete.size(); i++)
	{
		if(partnersToDelete[i] != NULL)
		{
			_PDBO_("Partner deleting " << partnersToDelete[i]->idStr())

		    // Master
			word aux = partnersToDelete[i]->idStr();
			//aux.replace(0, name_.size(), "m");
			container_.erase(aux);

			// Student
			aux = partnersToDelete[i]->idStr();
			//aux.replace(0, name_.size(), "s");
			container_.erase(aux);
		}
	}
}

volumetricParticle* Population::findParticleByIdStr(const word& idStr)
{
  const char prefixList[] = {'f', 'm', 's'};
  char prefix[] = "x";

  for(label i = 0; i < 3 ; ++i)
  {
    word  aux = idStr;
    prefix[0] = prefixList[i];

    if(!distributeToAll_) aux.replace(0, name_.size(), prefix);

    if( container_.found(aux) )
      return *(container_.find(aux));
  }

  return 0;
}


void Population::pOxidation(const bgGrid& bg)
{
	// pOxidation calculate the new scale for the particle size depending on the burn off rate
	// Burn off rate calculated based on the Arrhenius' equation. The value of Temperature is taken as
	// surface area weighted average. Values for face centers are considered same as cell center.
	// In case of parallel, the communicators are created based on the number of master particles and
	// communicate with the correspondig slave particles.

	const volScalarField& rTemp = obr_->lookupObject<volScalarField>("T"); //accessing the Temperature
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		volumetricParticle *sootParPtr = *iter;
		point pCenGrav = sootParPtr->cg();
		sootParPtr->sArea_ = 0;
		sootParPtr->sTemp_ = 0; //sum over all faces: Area weighted sum

		const pointField& fCntrs = sootParPtr->triSurf().faceCentres();
		const vectorField& fAreaVectors = sootParPtr->Sf();
		int i = 0;
		forAll(fCntrs, faceI) //calculate the sum of T*surfaceFaceArea (sTemp) and sum of sufaceFaceArea (sArea)
		{
			label cellI = myMSPtr_->findCell(fCntrs[faceI]); //Find the corresponding cell for particle face center
			if(cellI < 0)
				continue;
			sootParPtr->sArea_ += mag(fAreaVectors[faceI]);
			sootParPtr->sTemp_ += rTemp.internalField()[cellI]*mag(fAreaVectors[faceI]); //sum over all faces: Area weighted sum
			i++;
		}
//		_PDBO_("I am "<< iter.key() << ". Total number of faces are " << i <<". Before communication, sArea and sTemp are : " << sootParPtr->sArea_ << " and "<< sootParPtr->sTemp_)
	}

	if(Pstream::parRun())  // creating communicators and communications only if the case is in parallel.
	{
		nParticlesProc();
		Pstream::listCombineScatter(nFree_);
		Pstream::listCombineScatter(nMaster_);
		Pstream::listCombineScatter(nSlave_);

		label nMasterParSum = 0; //total number of master particles
		List<label> nMasterConsecutiveProc = nMaster_;

		forAll(nMaster_, n)
		{
			nMasterParSum += nMaster_[n];
			nMasterConsecutiveProc[n] = nMasterParSum; //sum of nMaster_ on consecutive processor number
		}

		List<label> nProcInCommunications(nMasterParSum,0);
		List<List<label>>  commProcessorsList(nMasterParSum);
		List<word> pIdList(nMasterParSum);

		label posIndex	= nMasterConsecutiveProc[Pstream::myProcNo()]-1;

		forAll(commProcessorsList, listI)
		{
			commProcessorsList[listI].resize(0);
		}

		forAllIter( HashTable<volumetricParticle*>, container_, iter)
		{
				word pIdStr = iter.key();
				volumetricParticle *sootParPtr = *iter;
				volumetricParticle::stateType state = sootParPtr->getState();

				// Nothing to do for free and slave particles
				if( state == volumetricParticle::free || state == volumetricParticle::slave )
				  continue;
				List<label> pIdx(3);
				point pCg    = sootParPtr->cg();
				bg.getIndex(pCg, pIdx);

				List<label> neighbourProcs;

				bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);

				label nProInComm = 0;
				forAll(neighbourProcs, neighbourI)
				{
					if( neighbourProcs[neighbourI] == -1)
						continue;
					nProInComm++;
				}

				commProcessorsList[posIndex].resize(nProInComm);
				nProcInCommunications[posIndex] = nProInComm;
				pIdList[posIndex] = sootParPtr->idStr();
				nProInComm = 0;

				forAll(neighbourProcs, neighbourI)
				{
					if( neighbourProcs[neighbourI] == -1)
						continue;
					commProcessorsList[posIndex][nProInComm] = neighbourProcs[neighbourI];
					nProInComm++;
				}
				posIndex--;
		}

		Pstream::listCombineGather(nProcInCommunications, maxEqOp<label>() );
		Pstream::listCombineScatter(nProcInCommunications); // Each processor knows about number of processors in each communicator.

		Pstream::listCombineGather(pIdList, maxEqOp<word>() );
		Pstream::listCombineScatter(pIdList); //Now each processor knows about master particles Ids

		forAll(commProcessorsList, listI)
		{
			if(!commProcessorsList[listI].size())
			{
				commProcessorsList[listI].resize(nProcInCommunications[listI],-1);
			}
		}

		forAll(commProcessorsList, listI)
		{
				Pstream::listCombineGather(commProcessorsList[listI], maxEqOp<label>() );
				Pstream::listCombineScatter(commProcessorsList[listI]); // each processor knows about who are in each communicator.
		}


		// Now each processor knows three things: how many procs in comms, who are in comms; and each comm is for which particle's Id.
		// Time to create communicators and communicate.
		forAll(commProcessorsList, cplistI)
		{
			// Get the rank and size in the original communicator
			int world_rank, world_size;
			MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
			MPI_Comm_size(MPI_COMM_WORLD, &world_size);

			// Get the group of processes in MPI_COMM_WORLD
			MPI_Group world_group;
			MPI_Comm_group(MPI_COMM_WORLD, &world_group);

			int nProcComm = commProcessorsList[cplistI].size(); // number of processors in the communicators
			int ranks[nProcComm]; 		// Assign the rank of processors
			forAll(commProcessorsList[cplistI],indexI)
			{
				ranks[indexI]=commProcessorsList[cplistI][indexI];
			}

			// construct the group containing all of the sub ranks in world_group
			MPI_Group sub_group;
			MPI_Group_incl(world_group, nProcComm, ranks, &sub_group);

			// Create a new communicator based on the group
			MPI_Comm sub_comm;
			MPI_Comm_create_group(MPI_COMM_WORLD, sub_group, 0, &sub_comm);

			forAllIter( HashTable<volumetricParticle*>, container_, iter)
			{
				word pIdStr = iter.key();
				volumetricParticle *sootParPtr = *iter;
				volumetricParticle::stateType state = sootParPtr->getState();

				// Nothing to do for free particles
				if( state == volumetricParticle::free )
					continue;

				if( pIdList[cplistI] == sootParPtr->idStr())
					{
						MPI_Allreduce(&sootParPtr->sTemp_, &sootParPtr->sTemp_, 1, MPI_DOUBLE, MPI_SUM, sub_comm);
						MPI_Allreduce(&sootParPtr->sArea_, &sootParPtr->sArea_, 1, MPI_DOUBLE, MPI_SUM, sub_comm);
//						_PDBO_("I am "<< iter.key() << ". After communication, sArea and sTemp are : " << sootParPtr->sArea_ << " and "<< sootParPtr->sTemp_)
					}
			}

			if(!MPI_COMM_NULL) // frees the groups and deallocates the communicator.
			{
				MPI_Group_free(&world_group); // frees a group.
				MPI_Group_free(&sub_group); //
				MPI_Comm_free(&sub_comm); //marks the communicator object for deallocation.
			}
		}
	} // End of if loop

	// Now the master and corresponding slave objects have identical sTemp and sArea variables, which are
	// necessary parameters to calculate the Arrhenius rate constant. So, scaling the STL as:
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		volumetricParticle *sootParPtr = *iter;
		point pCenGrav = sootParPtr->cg();
		scalar pDiaFactor = 1; //scaling factor for diameter
		scalar pDenFactor = 1; //scaling factor for particle density

		// still sTemp is sum of T*surfaceArea and sArea is total surface Area. Need to calculate area weighted temperature
		sootParPtr->sTemp_ /= (sootParPtr->sArea_ + VSMALL) ;
//		_PDBO_("I am "<< iter.key() << ". After average, sArea and sTemp are : " << sootParPtr->sArea_ << " and "<< sootParPtr->sTemp_)

		//calculate the Arrhenius rate constant (k=A*exp(-Ea/RT))
		const scalar k_arrh = this->af_*Foam::exp(-this->aEnergy_/(this->rGasConst_*(sootParPtr->sTemp_+VSMALL)));

		//calculate the fraction of oxidative mass removal(m/m0) = exp(-k*deltaT)
		scalar oxiMassRate = Foam::exp(-k_arrh*obr_->time().deltaT().value());
		sootParPtr->massRatio_ *=oxiMassRate; // Update the mass ratio (fraction of mass left up to now)

		if(sootParPtr->massRatio_ > 0.6) //First 40% Burn-off only via surface oxidation
		{
			pDiaFactor = Foam::pow(oxiMassRate, 1./3); //reduction of diameter
		}
		else //after 40% Burn off via surface and volume burning
		{
			pDiaFactor = Foam::pow(oxiMassRate, 1./6); //diameter redcution
			pDenFactor = Foam::pow(oxiMassRate,1./2); //Density reduction
		}
		sootParPtr->scaleSTL(pDiaFactor, pDenFactor);//Scale down the STL and update mass, volume and density.
	}
}	// End pOxidation


void Population::pOxidation()
{
	forAllIter( HashTable<volumetricParticle*>, container_, iter)
	{
		volumetricParticle *sootParPtr = *iter;
		point pCenGrav = sootParPtr->cg();
		scalar pDiaFactor = 1; //scaling factor for diameter
		scalar pDenFactor = 1; //scaling factor for particle density

		// still sTemp is sum of T*surfaceArea and sArea is total surface Area. Need to calculate area weighted temperature
		scalar temperature = rTemp_;

		//calculate the Arrhenius rate constant (k=A*exp(-Ea/RT))
		const scalar k_arrh = this->af_*Foam::exp(-this->aEnergy_/(this->rGasConst_*(temperature+VSMALL)));

		//calculate the fraction of oxidative mass removal(m/m0) = exp(-k*deltaT)
		scalar oxiMassRate = Foam::exp(-k_arrh*obr_->time().deltaT().value());
		sootParPtr->massRatio_ *=oxiMassRate; // Update the mass ratio (fraction of mass left up to now)

		if(sootParPtr->massRatio_ > 0.6) //First 40% Burn-off only via surface oxidation
		{
			pDiaFactor = Foam::pow(oxiMassRate, 1./3); //reduction of diameter
		}
		else //after 40% Burn off via surface and volume burning
		{
			pDiaFactor = Foam::pow(oxiMassRate, 1./6); //diameter redcution
			pDenFactor = Foam::pow(oxiMassRate,1./2); //Density reduction
		}
		sootParPtr->scaleSTL(pDiaFactor, pDenFactor);//Scale down the STL and update mass, volume and density.
	}
}

template <typename Type>
void Population::dynComm4FieldDist    (
                                       const bgGrid& bg,
                                       Field<Type>& (volumetricParticle::* fieldGetter) ()
                                     )
{
	if(!Pstream::parRun())
    return;

		nParticlesProc();
		Pstream::listCombineScatter(nFree_);
		Pstream::listCombineScatter(nMaster_);
		Pstream::listCombineScatter(nSlave_);

		label nMasterParSum = 0; //total number of master particles
		List<label> nMasterConsecutiveProc = nMaster_;

		forAll(nMaster_, n)
		{
			nMasterParSum += nMaster_[n];
			nMasterConsecutiveProc[n] = nMasterParSum; //sum of nMaster_ on consecutive processor number
		}

		List<label> nProcInCommunications(nMasterParSum,0);
		List<List<label>>  commProcessorsList(nMasterParSum);
		List<word> pIdList(nMasterParSum);

		label posIndex	= nMasterConsecutiveProc[Pstream::myProcNo()]-1;

		forAll(commProcessorsList, listI)
		{
			commProcessorsList[listI].resize(0);
		}

		forAllIter( HashTable<volumetricParticle*>, container_, iter)
		{
				word pIdStr = iter.key();
				volumetricParticle *parPtr = *iter;
				volumetricParticle::stateType state = parPtr->getState();

				// Nothing to do for free and slave particles
				if( state == volumetricParticle::free || state == volumetricParticle::slave )
				  continue;
				List<label> pIdx(3);
				point pCg    = parPtr->cg();
				bg.getIndex(pCg, pIdx);

				List<label> neighbourProcs;

				bg.getAllNeighbourProcessorsCompact(pIdx, neighbourProcs, bgSearchRadius_);

				label nProInComm = 0;
				forAll(neighbourProcs, neighbourI)
				{
					if( neighbourProcs[neighbourI] == -1)
						continue;
					nProInComm++;
				}

				commProcessorsList[posIndex].resize(nProInComm);
				nProcInCommunications[posIndex] = nProInComm;
				pIdList[posIndex] = parPtr->idStr();
				nProInComm = 0;

				forAll(neighbourProcs, neighbourI)
				{
					if( neighbourProcs[neighbourI] == -1)
						continue;
					commProcessorsList[posIndex][nProInComm] = neighbourProcs[neighbourI];
					nProInComm++;
				}
				posIndex--;
		}

		Pstream::listCombineGather(nProcInCommunications, maxEqOp<label>() );
		Pstream::listCombineScatter(nProcInCommunications); // Each processor knows about number of processors in each communicator.

		Pstream::listCombineGather(pIdList, maxEqOp<word>() );
		Pstream::listCombineScatter(pIdList); //Now each processor knows about master particles Ids

		forAll(commProcessorsList, listI)
		{
			if(!commProcessorsList[listI].size())
			{
				commProcessorsList[listI].resize(nProcInCommunications[listI],-1);
			}
		}

		forAll(commProcessorsList, listI)
		{
				Pstream::listCombineGather(commProcessorsList[listI], maxEqOp<label>() );
				Pstream::listCombineScatter(commProcessorsList[listI]); // each processor knows about who are in each communicator.
		}


		// Now each processor knows three things: how many procs in comms, who are in comms; and each comm is for which particle's Id.
		// Time to create communicators and communicate.
		forAll(commProcessorsList, cplistI)
		{
			// Get the rank and size in the original communicator
			int world_rank, world_size;
			MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
			MPI_Comm_size(MPI_COMM_WORLD, &world_size);

			// Get the group of processes in MPI_COMM_WORLD
			MPI_Group world_group;
			MPI_Comm_group(MPI_COMM_WORLD, &world_group);

			int nProcComm = commProcessorsList[cplistI].size(); // number of processors in the communicators
			int ranks[nProcComm]; 		// Assign the rank of processors
			forAll(commProcessorsList[cplistI],indexI)
			{
				ranks[indexI]=commProcessorsList[cplistI][indexI];
			}

			// construct the group containing all of the sub ranks in world_group
			MPI_Group sub_group;
			MPI_Group_incl(world_group, nProcComm, ranks, &sub_group);

			// Create a new communicator based on the group
			MPI_Comm sub_comm;
			MPI_Comm_create_group(MPI_COMM_WORLD, sub_group, 0, &sub_comm);

			forAllIter( HashTable<volumetricParticle*>, container_, iter)
			{
				word pIdStr = iter.key();
				volumetricParticle *parPtr = *iter;
				volumetricParticle::stateType state = parPtr->getState();

				// Nothing to do for free particles
				if( state == volumetricParticle::free )
					continue;

				if( pIdList[cplistI] == parPtr->idStr())
					{
						Field<Type> fieldBuf;
						Field<Type>& pFieldDist = (parPtr->*fieldGetter)();
						fieldBuf.resize(pFieldDist.size());
						label  bufSize = (parPtr->*fieldGetter)().size() * sizeof(Type);

						const char* readData = reinterpret_cast<const char*>(pFieldDist.cdata());
						char* receiveData = reinterpret_cast<char*>(fieldBuf.data());

						MPI_Allreduce(readData, receiveData, bufSize, MPI_CHAR, MPI_SUM, sub_comm);

						pFieldDist = fieldBuf;
					}
			}

			if(!MPI_COMM_NULL) // frees the groups and deallocates the communicator.
			{
				MPI_Group_free(&world_group); // frees a group.
				MPI_Group_free(&sub_group); //
				MPI_Comm_free(&sub_comm); //marks the communicator object for deallocation.
			}
		}
}

} 	// namespace Foam
