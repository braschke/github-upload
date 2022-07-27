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

    $Date: 2012-07-11 11:05:29 +0200 (Wed, 11 Jul 2012) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef agglomerationConstraint_H
#define agglomerationConstraint_H

#include "constraint.H"

#include "Vector.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
//
//

class agglomerationConstraint: public Constraint
{
public:

	mutable scalar totalMass_;
	const Time &time_;
	mutable vector commonCg_;
	mutable vector velo_;
	mutable vector acc_;
	mutable vector omega_;
	mutable vector torque_;
	mutable vector omegaAcc_;
	mutable symmTensor J_;

//- Runtime type information
TypeName("agglomerationConstraint");

agglomerationConstraint(
                       const dictionary     &dict,
                       const objectRegistry &obr,
                       const myMeshSearch   *myMSPtr
                     );

virtual void constrain(volumetricParticle& particle) const;

void constrainPartners(volumetricParticle& particle) const;

void calcCommonCgAndMass(volumetricParticle& particle) const;
void calculateDispl(volumetricParticle& particle) const;
vector calcForce(volumetricParticle& particle, vector cg) const;
void calcTorque(volumetricParticle& particle, vector cg) const;
void calcJ(volumetricParticle& particle, vector cg) const;
void calcVelo(volumetricParticle& particle) const;
void calcOmega(volumetricParticle& particle) const;


}; // class staticForceConstraint

} // namespace Foam

#endif // staticForceConstraint_H
