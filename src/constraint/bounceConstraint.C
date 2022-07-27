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


#include "bounceConstraint.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "Matrix.H"

#include "volumetricParticle.H"

#include "mpi.h"



namespace Foam
{
defineTypeNameAndDebug(bounceConstraint, 0);
addToRunTimeSelectionTable(
                           Constraint,
                           bounceConstraint,
                           dictionary
                          );

bounceConstraint::bounceConstraint(
                                    const dictionary     &dict,
                                    const objectRegistry &obr,
                                    const myMeshSearch   *myMSPtr
                                  ):
Constraint(dict, obr, myMSPtr)
{
    cType_   = velocity;
    nameStr_ = "bounceConstraint class";

    // nasty hack
    char aux[1024]; aux[1023] = '\0';
    sprintf(
            aux,
            "bounce constrain activated"
           );
    infoStr_ = aux;
}

void bounceConstraint::constrain(volumetricParticle& particle) const
{

	volumetricParticle::stateType state = particle.getState();
	//if state == volumetricParticle::master

	//overall number of processors
	label n = Pstream::nProcs();
	//List<int> stickedList(n, 0);
	int send_stickedVal[n];
	int rec_stickedVal[n];
	label procNo = Pstream::myProcNo();

	List<label> pIdx(3);


	//_PDBO_("Entered deposit constraint");
	const fvMesh& mesh = refCast<const fvMesh>(obr_);
	const pointField &midPoints = mesh.C().internalField();
	const volScalarField& depo = obr_.lookupObject<volScalarField>("deposit");
	const volScalarField& voidFrac = obr_.lookupObject<volScalarField>("voidFrac");
	const volScalarField y = obr_.lookupObject<volScalarField>("wallDistance");
	const volVectorField wallN = obr_.lookupObject<volVectorField>("wallN");
	const scalar pi = constant::mathematical::pi;

	label cg_label = myMSPtr_->findCell( particle.cg() );

	scalar cg_dist = y[cg_label];
	_PDBO_("midpoint of cg  is: "<< midPoints[cg_label])



	scalar k_pl, mu, ez, ex, dp;
	dp = 2*particle.radEVS_;
	vector v_vec = particle.getVelocity();
	vector w_vec = particle.getOmega();
	scalar kinEn = particle.getKineticEnergy();

	//TODO Die folgenden Parameter sollten eigentlich Eigenschaften vom Partikel sein
	scalar sigma_p = 62.5e6; // SiO2 600kp/mm² ; Fließspannung
	scalar p_pl = 3*sigma_p; // Plastischer Fließdruck nach Hersey und Rees, 1971; Paronen und Ilkka, 1996
	p_pl = 3e7;
	mu = 18.5e-6; // Gleitreibungskoeffzient - Sollte später Teil der Population sein, hängt aber von beiden Materialien ab
	scalar d_k; // Durchmesser der Kontaktfläche - vllt über Abflachungsfaktor analog zu Alexander Haarmann
	scalar a_0 = 4e-10; // Kontaktabstand - vllt keine Partikeleigenschaft, sollte aber besser extern eingelesen werden
	scalar A_h = 2e-18; // Hamaker-Konstante, 5e-19 für PSL, 45.3e-20 gold ; 6.5e-20 Glas Isrealachvili
	k_pl = 0.93; // Restitutionskoeff.; laut Hiller 0.4 - 0.6 für Stoffe wie beispw. Quarz
	scalar atom_r = 174e-12; // Radius des Atoms; 174pm berechnet für Gold


	scalar volFac = 1.2; // Faktor volFac absolut arbiträr gewählt
	 if(!particle.sticked_) {
		  forAll(voidFrac, cellI) {
			  if(voidFrac[cellI] != 0) {
				  scalar cellDist = volFac*pow((1.*mesh.V()[cellI]), (1.*1/3));
				  vector n_vec = wallN[cellI];
				  n_vec /= mag(n_vec);
				  scalar cosAlpha = (v_vec & n_vec)/(mag(v_vec) * mag(n_vec));
				  if((depo[cellI] != 0 || y[cellI] < cellDist) && cosAlpha > 0 && cosAlpha < 1) {

						// Kontaktradius bestimmen über (3.18) Lehmann - Beladungskinetik
					    //_PDBO_("v_vec: " << v_vec << "      n_vec: " << n_vec )
						//double h_pl = dp * dp * pow((v_vec & n_vec), 2) * (1 - k_pl * k_pl) * particle.getRho() / (6 * p_pl);
						double h_pl = dp * (v_vec & n_vec) * sqrt(1 - k_pl * k_pl) * sqrt(particle.getRho() / (6 * p_pl));
					    if (h_pl < 0) h_pl *= -1.0;
						//_PDBO_("h_pl: " << h_pl)
						d_k = sqrt(h_pl * dp);
						//_PDBO_("\n d_k: " << d_k)
						//_PDBO_("\n dp:" << dp)
						// Energieverlust durch plastische Verformung
						//scalar defEn = 0.5 * p_pl * pi * pow(d_k, 4) / dp;
						scalar defEn = 0.5 * p_pl * pi * dp * h_pl * h_pl;
						//scalar rho_atom = 2425 * (6.02214085774e23 / 0.0601); // Atomare Teichendichte des Wandmaterials (1/m³)
						// scalar adhEn = A_h / (constant::mathematical::pi * 6 * pow(a_0, 3) * rho_atom); // adhEn eines Atoms
						double adhEn = 0; // adhEn in D.Maugis, Contact, Adhesion and Rupture of Elastic Solids
						adhEn = A_h * dp * h_pl / (12 * a_0 * a_0);
						// Restitutionskoeffizient über Energien bestimmen (dafür müsste d_k über Über Oberflächenergie nach DMT oder JKR bestimmt werden)
						//k_pl = 1 - sqrt(defEn / kinEn);

						// Berechnung mit approx. Atomschichten
						/*scalar max_dist = 40e-9; // Maximale Distanz in der noch vdW-Kräfte berechnet werden sollen
						int atomlayers = floor(0.5 * (max_dist - a_0) / atom_r);
						_PDBO_("\n atomlayers: " << atomlayers)
						scalar pack_density = 0.74;
						scalar layervolume = (2 * atom_r * pi * d_k * d_k / 4);
						_PDBO_("\n layervolume: " << layervolume)
						double atom_r_cube = pow(atom_r, 3);
						_PDBO_("\n atom_r_cube: " << atom_r_cube)
						double atomsPerLayer = floor(pack_density * layervolume / (4/3 * pi * atom_r_cube / 8));
						_PDBO_("\n atomsPerLayer: " << atomsPerLayer)

						for(int i = 0; i < atomlayers; i++) {
							 adhEn += (A_h / (pi * 6 * pow((a_0 + 2 * i * atom_r), 3) * rho_atom)) * atomsPerLayer;
						}*/


					  /*_PDBO_("\n cosAlpha: " << cosAlpha)
					  _PDBO_("\n OLD v_vec: " << v_vec)
					  _PDBO_("\n OLD w_vec: " << w_vec)*/

					  _PDBO_("\n kin. Energy is: " << kinEn)
					  _PDBO_("\n def. Energy is: " << defEn)
					  _PDBO_("\n adh. Energy is: " << adhEn)
					  //_PDBO_("\n Diff. kinEn to defEn+adhEn is: " << (kinEn - (defEn + adhEn)))

						if(kinEn - (defEn + adhEn) <= 0) {
							_PDBO_("particleInfo: Adhesion")
						}
						else {
							_PDBO_("particleInfo: Bounce")
						}

					  // Transformation in das Koordinatensystem der Oberfläche
					  // Zunächst ONB bestimmen
					  scalar n_comp = v_vec & n_vec;
					  vector t_vec = v_vec - n_comp * n_vec;
					  scalar t_comp = mag(t_vec);
					  t_vec /= t_comp;
					  vector m_vec = t_vec ^ n_vec;
					  scalar m_comp = mag(m_vec);
					  m_vec /= m_comp;

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
					  v_vec = tbb2 & v_vec;
					  w_vec = tbb2 & w_vec;


					  // Stoß berechnen
					  scalar vrel = sqrt(pow(v_vec[0] + w_vec[2] * dp/2 , 2) + pow(v_vec[2] - w_vec[0] * dp/2, 2));
					  ex = (v_vec[0] + dp/2 * w_vec[2]) / vrel;
					  ez = (v_vec[2] - dp/2 * w_vec[0]) / vrel;

					  v_vec[0] = v_vec[0] + ex * (k_pl+1) * mu * v_vec[1];
					  v_vec[2] = v_vec[2] + ez * (k_pl+1) * mu * v_vec[1];
					  v_vec[1] = -k_pl * v_vec[1];

					  w_vec[0] = w_vec[0] - 5/dp * ez * (k_pl+1) * mu * w_vec[1];
					  w_vec[2] = w_vec[2] + 5/dp * ex * (k_pl+1) * mu * w_vec[1];

					  // Geschwindigkeiten zurück ins globale Koord.sys. transformieren
					  v_vec = tb2b & v_vec;
					  w_vec = tb2b & w_vec;

					  particle.getVelocity() = v_vec;
					  particle.getOmega() = w_vec;
					  //_PDBO_("\n NEW v_vec: " << particle.getVelocity())
					  //_PDBO_("\n NEW w_vec: " << particle.getOmega())

					  //particle.sticked_ = true;
					  //_PDBO_("\n particleBounce !!!")
					  particle.getAverageVelocity() = v_vec;
					  particle.getAverageOmega() = w_vec;
					  //_PDBO_("\n average v_vec: " << particle.getAverageVelocity())
					  //_PDBO_("\n average w_vec: " << particle.getAverageOmega())
					  break;
				  }
			  }
		  }
	  }
}


} // namespace Foam
