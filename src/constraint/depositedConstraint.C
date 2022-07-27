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

    Kamil Braschke
    Chair of Fluid Mechanics
    braschke@uni-wuppertal.de

    $Date: 2012-08-20 09:40:52 +0200 (Mon, 20 Aug 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "depositedConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"

#include "volumetricParticle.H"

#include "mpi.h"



namespace Foam
{
defineTypeNameAndDebug(depositedConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           depositedConstraint,
                           dictionary
                          );

depositedConstraint::depositedConstraint(
                                          const dictionary     &dict,
                                          const objectRegistry &obr,
                                          const myMeshSearch   *myMSPtr
                                        ):
Constraint(dict, obr, myMSPtr)
{
    cType_   = velocity;
    nameStr_ = "depositedConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "deposition constrain activated"
           );
    infoStr_ = aux;
}

void depositedConstraint::constrain(volumetricParticle& particle) const
{
	vector velo = particle.getVelocity();
	vector veloAvg = particle.getAverageVelocity();


  if (particle.deposited_) {
	  //_PDBO_("Running deposited constraint")
	  /*particle.getAcceleration() = 1*( - particle.getVelocity() ) / obr_.time().deltaT().value();
	  particle.getOmegaAcc() = 1*( - particle.getOmega() ) / obr_.time().deltaT().value();*/

	  //vector n_vec = particle.contactNormal_;

	 // scalar cosAlpha = (velo & n_vec) / (mag(velo) + VSMALL);

	  // Only reduce velocity by the component in the direction of nearest surface
	  /*if(cosAlpha > 0) {
	  particle.getVelocity() -= (velo & n_vec) * n_vec;
	  }*/

	  //Completely stop the particle
	  particle.getVelocity() -= velo;
	  particle.getAverageVelocity() -= veloAvg;

  }
  else { //deposited is set to false, because nano particles don't need the flag
	  //_PDBO_("Running nano deposited constraint")
	  const pointField& faceCtrs = particle.triSurf().faceCentres();
	  const fvMesh& mesh = refCast<const fvMesh>(obr_);
	  vector wallVector = vector::zero;

	  forAll(faceCtrs, faceI) {
		  label cellI = myMSPtr_->findCell(faceCtrs[faceI]);
		  if( cellI < 0 ) continue;
		  const cell& cFaces = mesh.cells()[cellI];

		  forAll(cFaces, faceC) {
			  label facePatchId = mesh.boundaryMesh().whichPatch(cFaces[faceC]);
			  if(facePatchId < 0) continue;
			  const polyPatch& pp = mesh.boundaryMesh()[facePatchId];
			  if(pp.type() != "wall") continue; //TODO compare with a list of patches on which deposition is possible
			  const face& fc = mesh.faces()[cFaces[faceC]];
			  //if(mesh.isInternalFace(cFaces[faceC])) continue;

			  wallVector = fc.normal(mesh.points());
			  if(wallVector == vector::zero) continue;
			  wallVector /= mag(wallVector);

			  scalar cosAlpha = ( veloAvg & wallVector ) / (mag(veloAvg) + VSMALL);
			  if(cosAlpha > 0) {
				  //_PDBO_("wallVec = " << wallVector << "\tcosAlpha = " << cosAlpha << "\ttempVelo = " << velo <<"\n")
				  if ((veloAvg & wallVector) == 0)
				  {
					  veloAvg = vector::zero;
					  velo = vector::zero;
				  }
				  else
				  {
					  velo -=  getVelocityReduction(velo, wallVector);
					  veloAvg -=  getVelocityReduction(veloAvg, wallVector);
				  }
				  //_PDBO_("Acceleration is: " << particle.getAcceleration() << "\twith wallVec = " << wallVector << "\tand velo = " << velo)
				  //particle.getAcceleration() -= 1*( - (velo & wallVector) * wallVector ) / obr_.time().deltaT().value();
			  }
		  }
	  }

	  particle.getAverageVelocity() = veloAvg;
	  particle.getVelocity() = velo;
	  //particle.getVelocity() -= particle.getVelocity() - velo;
	  //particle.getVelocity() -= velo;
	  //_DBO_("FINAL velocity = " << particle.getVelocity())
	  //_DBO_("FINAL averageVelocity = " << particle.getAverageVelocity())
  }
}

vector depositedConstraint::getVelocityReduction(vector v_vec, vector n_vec) const
{
	//_DBO_("entering: reducing velocity for n_vec = " << n_vec << "\tv_vec = " << v_vec)
		vector reduction = vector::zero;

		reduction = ( v_vec & n_vec ) * n_vec;
		return reduction;

		// Transformation in das Koordinatensystem der Oberfläche
		// Zunächst ONB bestimmen
		scalar n_comp = v_vec & n_vec;
		vector t_vec = v_vec - n_comp * n_vec;
		scalar t_comp = mag(t_vec);
		t_vec /= (t_comp + VSMALL);
		vector m_vec = t_vec ^ n_vec;
		scalar m_comp = mag(m_vec);
		m_vec /= (m_comp + VSMALL);

		// Transformationsmatrix berechnen
		symmTensor b_mat = symmTensor(
				1, 0, 0,
				1, 0,
				1
				);
		tensor b2_mat = tensor(
				t_vec[0], n_vec[0], m_vec[0],
				t_vec[1], n_vec[1], m_vec[1],
				t_vec[2], n_vec[2], m_vec[2]
				);

		tensor tbb2 = inv(b2_mat) & b_mat;
		tensor tb2b = inv(tbb2);

		// Geschwindigkeiten in die Basis der Oberfläche transformieren
		v_vec = transform(tbb2, v_vec);
		n_vec = transform(tbb2, n_vec);

		// Verringerung der Geschwindigkeit in Wandrichtung berechnen und zurück in die alte Basis transformieren
		reduction = (v_vec & n_vec) * n_vec;
		reduction = transform(tb2b, reduction);

		return reduction;

}


} // namespace Foam
