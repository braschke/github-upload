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

 $Date: 2017-08-03 11:34:47 +0200 (Do, 03 Aug 2017) $

 License

 This file is contaminated by GNU General Public Licence.
 You should have received a copy of the GNU General Public License
 along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

 \*---------------------------------------------------------------------------*/

#include "myMeshSearch.H"
#include "volFieldsFwd.H"
#include "GeometricField.H"

#include "indexedOctree.H"
#include "DynamicList.H"
#include "demandDrivenData.H"
#include "treeDataCell.H"
#include "treeDataFace.H"
#include "fvMesh.H"

#include <cmath>

#include <ctime>
#include <chrono>

namespace Foam {
myMeshSearch::myMeshSearch(const objectRegistry& obr, const dictionary& dict //  parent dictionary, e.g. from funcObj
		) :
		obr_(obr), dict_(dict.subDictPtr("myMeshSearchDict")), mesh_(
				obr_.lookupObject<const fvMesh>("region0")),
//  			      mesh_(refCast<const fvMesh>(obr)),
		searchMethod_(octbox), cellDecomp_(polyMesh::CELL_TETS), hashMapPtr_(
				NULL)
#if _OCTBOX_SUPPORT_
, minDist_(0.)                 //,
#endif
{
#if  _OCTBOX_SUPPORT_
	word smEntry("octbox");
#else
	word smEntry("OF");
#endif

	word decEntry("cellTets");

	if (dict_) {
		smEntry = dict_->lookupOrDefault < word > ("searchMethod", "octbox");
		decEntry = dict_->lookupOrDefault < word > ("cellDecomp", "cellTets");
	}

	// set searchMethod
	if (smEntry == "OF") {
		searchMethod_ = OF;
		Info << nl << "myMeshSearch::myMeshSearch():" << nl
				<< "Using search method: 'OF'" << endl;
	} else if (smEntry == "hash") {
		searchMethod_ = hash;
		deltaX_ = dict_->lookupOrDefault<double>("deltaX", 1);
		deltaY_ = dict_->lookupOrDefault<double>("deltaY", 1);
		deltaZ_ = dict_->lookupOrDefault<double>("deltaZ", 1);

		vector minBb(dict_->lookup("minBb"));
		vector maxBb(dict_->lookup("maxBb"));
		Foam::vector lengthVec = maxBb - minBb;

		// Length of corresponding lists
		countX_ = lengthVec.x() / deltaX_;
		countY_ = lengthVec.y() / deltaY_;
		countZ_ = lengthVec.z() / deltaZ_;

		Info << nl << "myMeshSearch::myMeshSearch():" << nl
				<< "Using search method: 'hash'" << nl
				<< "Please make sure the hexmesh is structured" << nl
				<< "and that deltaX, deltaY, deltaZ, minBB and maxBb" << nl
				<< "are set in the controlDict!" << endl;
	} else if (smEntry == "octbox") {
		searchMethod_ = octbox;
		Info << nl << "myMeshSearch::myMeshSearch():" << nl
				<< "Using search method: 'octbox'" << endl;

#if ! _OCTBOX_SUPPORT_
		WarningIn("myMeshSearch::myMeshSearch()")
				<< "Entry 'searchMethod' set to 'octbox'," << nl
				<< "but 'myMeshSearch' was not built with 'octbox' support!"
				<< nl << "Now, using default 'OF'." << endl;
		searchMethod_ = OF;
#endif
	} else {
#if  _OCTBOX_SUPPORT_
		WarningIn("myMeshSearch::myMeshSearch()")
		<< "Entry 'searchMethod' ill defined!" << nl
		<< "Proper values are 'OF', 'hash' and 'octbox'." << nl
		<< "Now, using default 'octbox'." << endl;
		searchMethod_ = octbox;
#else
		WarningIn("myMeshSearch::myMeshSearch()")
				<< "Entry 'searchMethod' ill defined!" << nl
				<< "Proper values are 'OF' and 'hash'." << nl
				<< "Now, using default 'OF'." << endl;
		searchMethod_ = OF;
#endif
	}

	if (searchMethod_ == OF) {
		// set decomposition mode in case of search method 'OF'

		if (decEntry == "cellTets") {
			cellDecomp_ = polyMesh::CELL_TETS;
		} else if (decEntry == "faceDiagTris") {
			cellDecomp_ = polyMesh::FACE_DIAG_TRIS;
		} else if (decEntry == "faceCentreTris") {
			cellDecomp_ = polyMesh::FACE_CENTRE_TRIS;
		} else if (decEntry == "facePlanes") {
			cellDecomp_ = polyMesh::FACE_PLANES;
		} else {
			WarningIn("myMeshSearch::myMeshSearch()")
					<< "Entry 'cellDecomp' ill defined!" << nl
					<< "Proper values are 'cellTets', 'faceDiagTris', 'faceCentreTris' and 'facePlanes'."
					<< nl << "Now, using default 'cellTets'." << endl;
			cellDecomp_ = polyMesh::CELL_TETS;
			decEntry = "cellTets";
		}

		Info << nl << "myMeshSearch::myMeshSearch():" << nl
				<< "Using cell decomposition method '" << decEntry << "'." << nl
				<< endl;
	}

#if _OCTBOX_SUPPORT_
	memset(&octbox_, 0, sizeof(OCTBOX));
#endif

        // Prepare individual bounding boxes
        bbMin_ = treeBoundBox(mesh_.points()).min();
        bbMax_ = treeBoundBox(mesh_.points()).max();

        setup();
}

myMeshSearch::~myMeshSearch() {
	clearOut();
#if _OCTBOX_SUPPORT_

	OCTBOX_cleanup(&octbox_);

#endif
}

void Foam::myMeshSearch::clearOut() {
	boundaryTreePtr_.clear();
	cellTreePtr_.clear();
	overallBbPtr_.clear();
}

void myMeshSearch::setup() // construct new
{
	std::chrono::steady_clock::time_point start_time, end_time;

	Info << nl << "Setting up 'myMeshSearch'!" << nl << endl;
	clearOut();


#if _OCTBOX_SUPPORT_
	if( searchMethod_ != octbox )
	return;

	word infoStr("ABSFoam, processor# ");
	infoStr.append( _ITOS_(Pstream::myProcNo(), _N_DIGITS_PROCNUM_) );

	const pointField &cc = mesh_.C().internalField();

	minDist_ = std::cbrt( min( mesh_.V() ).value() ) * _LSMSQRTOFTHREE;

	OCTBOX *octboxPtr = &octbox_;

	OCTBOX_setup_b
	(
			octboxPtr,
			infoStr.c_str(),
			cc.size(),
			minDist_,
			minDist_ * 10, // currently ignored
			0
			|OCTBOX_TYPE_GEOM
			|OCTBOX_FLAG_ECENT,
			bbMin_.x(),
			bbMin_.y(),
			bbMin_.z(),
			bbMax_.x(),
			bbMax_.y(),
			bbMax_.z()
	);

	forAll(cc, labelI)
	{
		point p = cc[labelI];
		scalar x = p.x();
		scalar y = p.y();
		scalar z = p.z();

		const OLEAF *leaf = OCTBOX_insert(
				octboxPtr,
				x, y, z,
				labelI,
				0
		);

		if (!leaf)
		{
			FatalErrorIn("myMeshSearch::setup()")
			<< "Failed to insert vertex " << labelI
			<< "(" << x << ", " << y << ", " << z << ") = "
			<< "(" << OCTBOX_UX(octboxPtr,x) << ", " << OCTBOX_UX(octboxPtr,x) << ", " << OCTBOX_UX(octboxPtr,x) << ")"
			<< "into the octal tree." << nl << exit(FatalError);
		}

		if( leaf->l_idx != labelI )
		{
			WarningIn("myMeshSearch::setup()")
			<< "Inserted vertex " << labelI
			<< "(" << x << ", " << y << ", " << z << ") = "
			<< "(" << OCTBOX_UX(octboxPtr,x) << ", " << OCTBOX_UX(octboxPtr,x) << ", " << OCTBOX_UX(octboxPtr,x) << ")"
			<< "is a duplicate node." << nl;
		}

	}
	OCTBOX_close(octboxPtr, 0);
#endif
}


label myMeshSearch::findCell(const point& p) const
  {
	// Check if point is outside of current bounding box
	if(
			bbMin_.x() > p.x() || bbMin_.y() > p.y() || bbMin_.z() > p.z()
			|| bbMax_.x() < p.x() || bbMax_.y() < p.y() || bbMax_.z() < p.z()
	  )
	{ return -1; }

    switch(searchMethod_)
    {
      case OF:
      #if 0
        return mesh_.findCell(p, cellDecomp_);
      #else
        return cellTree().findInside(p);
      #endif

      case hash:
    	  /*_PDBO_("For point " << p)
    	  _PDBO_("floor(p.x()/deltaX_) = " << floor(p.x()/deltaX_))
    	  _PDBO_("floor(p.y()/deltaY_) = " << floor(p.y()/deltaY_))
    	  _PDBO_("floor(p.z()/deltaZ_) = " << floor(p.z()/deltaZ_))*/
    	  int posX, posY, posZ;
    	  posX = floor(p.x()/deltaX_);
    	  posY = floor(p.y()/deltaY_);
    	  posZ = floor(p.z()/deltaZ_);

    	  /*if(countX_ < posX || countY_ < posY || countZ_ < posZ ||
    			  posX < 0 || posY < 0 || posZ < 0)
    		  return -1;*/

		//_PDBO_("hash returns: " << *(*(*(*hashMap())[floor(p.x()/deltaX_)])[floor(p.y()/deltaY_)])[floor(p.z()/deltaZ_)])
		return *(*(*(*hashMap())[posX])[posY])[posZ];

#if _OCTBOX_SUPPORT_
		case octbox:
		const OCTBOX* octboxPtr = &octbox_;

		//const OLEAF *leaf = OCTBOX_find(octboxPtr, p.x(), p.y(), p.z());
		const OLEAF *leaf = OCTBOX_search(octboxPtr, p.x(), p.y(), p.z(), 0);

		if (!leaf)
		{
#if 0
			FatalErrorIn("myMeshSearch::findCell()")
			<< "Failed to find vertex "
			<< "(" << p.x() << ", " << p.y() << ", " << p.z() << ") "
			<< "in mesh." << nl << exit(FatalError);
#endif

			return -1; // no return: error exit above
		}

        // perform check
        label  foundIdx     = leaf->l_idx;
        /*point  foundPoint(leaf->l_rx, leaf->l_ry, leaf->l_rz );
        scalar deviation    = mag(foundPoint - p);

		 if( deviation > 1.1 * minDist_ )
		 {
		 // _PDBO_("\nNearest cell too far away! Dev = " << deviation << "; 1.1 * minDist_ = " << (1.1*minDist_))

           foundIdx = -1;
        }*/

		return foundIdx;

#endif
	}
	return 0;
}

const Foam::List<Foam::List<Foam::List<Foam::label*>*>*>*
myMeshSearch::hashMap() const {
	if (hashMapPtr_ == NULL) {
		_PDBO_("\nBuilding hashMap\n")
		_PDBO_(
				"countX = " << ceil(countX_) << "\tcountY = " << ceil(countY_)
						<< "\tcountZ = " << ceil(countZ_))

		List<label*>* listZ = new List<label*>(ceil(countZ_ + 1));
		List<List<label*>*>* listY = new List<List<label*>*>(ceil(countY_ + 1));
		List<List<List<label*>*>*>* listX = new List<List<List<label*>*>*>(
				ceil(countX_ + 1));

		for (int x = 0; x < ceil(countX_ + 1); x++) {
			for (int y = 0; y < ceil(countY_ + 1); y++) {
				for (int z = 0; z < ceil(countZ_ + 1); z++) {
					(*listZ)[z] = new (std::nothrow) label(-1);
					if (!(*listZ)[z])
						std::cout << "MALLOC PROBLEM!!!";
				}

				(*listY)[y] = new (std::nothrow) List<label*>(*listZ);
				if (!(*listY)[y])
					std::cout << "MALLOC PROBLEM!!!";
			}

			(*listX)[x] = new (std::nothrow) List<List<label*>*>(*listY);
			if (!(*listX)[x])
				std::cout << "MALLOC PROBLEM!!!";
		}
		_PDBO_("Finished allocating memory for hashMap.")

		forAll(mesh_.C(), cellC)
		{
			//_PDBO_("cellC = " << cellC << " with centre = " << mesh_.C()[cellC])
			//_PDBO_("Accesing element: " << floor((mesh_.C()[cellC]).x()/deltaX_) << " " << floor((mesh_.C()[cellC]).y()/deltaY_) << " " << floor((mesh_.C()[cellC]).z()/deltaZ_))
			*((*(*(*listX)[floor((mesh_.C()[cellC]).x() / deltaX_)])[floor(
					(mesh_.C()[cellC]).y() / deltaY_)])[floor(
					(mesh_.C()[cellC]).z() / deltaZ_)]) = cellC;
		}
		_PDBO_("Finished building hashMap.")

		//_PDBO_("============= FINAL listX with cellI set IS =============\n" << *listX)
		//hashMapPtr_.reset(&listX);
		hashMapPtr_ = listX;
		//cout << reinterpret_cast<void*>(listX) << endl;
	}
	return hashMapPtr_;
}

const Foam::indexedOctree<Foam::treeDataCell>&
myMeshSearch::cellTree() const {
	if (!cellTreePtr_.valid()) {
		Info<<"\nmyMeshSearch: constructing cellTree\n";
		//
		// Construct tree
		//

		if (!overallBbPtr_.valid()) {
			Random rndGen(261782);
			overallBbPtr_.reset(new treeBoundBox(mesh_.points()));

			treeBoundBox& overallBb = overallBbPtr_();
			// Extend slightly and make 3D
			overallBb = overallBb.extend( 1e-4); //overallBb.extend(rndGen, 1e-4);
			overallBb.min() -= point(ROOTVSMALL, ROOTVSMALL, ROOTVSMALL);
			overallBb.max() += point(ROOTVSMALL, ROOTVSMALL, ROOTVSMALL);
		}

		cellTreePtr_.reset(new indexedOctree<treeDataCell>(treeDataCell(false, // not cache bb
				mesh_, cellDecomp_ // cell decomposition mode for inside tests
				), overallBbPtr_(), 8,              // maxLevel
				10,             // leafsize
				6.0             // duplicity
				));
	}

	return cellTreePtr_();
}

} // namespace Foam
