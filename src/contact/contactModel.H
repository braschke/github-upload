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

    $Date: 2013-10-28 08:53:30 +0100 (Mo, 28 Okt 2013) $

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef contactModel_H
#define contactModel_H

#include "messageStream.H"
#include "dictionary.H"
#include "vector.H"

#include "contact.H"

namespace Foam
{

///////////////////////////////////////////////////////////
//
// contact model base class
//

class contactModel
{
public:

    contactModel(const dictionary &d) :
    E_(1.0),
    mu_(0.0),
    e_(1.0),
    m_(1.0),
    D_(0.0),
    infoStr_("Standard contact model")
    {
      scalar E1(readScalar(d.lookup("E1")));
      scalar E2(readScalar(d.lookup("E2")));
      scalar nu1(readScalar(d.lookup("nu1")));
      scalar nu2(readScalar(d.lookup("nu2")));
      scalar m1(readScalar(d.lookup("m1")));
      scalar m2 = d.lookupOrDefault("m2", VGREAT);
      scalar R1(readScalar(d.lookup("R1")));
      scalar R2 = d.lookupOrDefault("R2", VGREAT);

      mu_ = d.lookupOrDefault("mu", 0.0);
      e_  = d.lookupOrDefault("e", 1.0);
      D_  = d.lookupOrDefault("D", 0.0);

      m_  = 1 / (1/m1 + 1/m2);
      R_  = 1 / (1/R1 + 1/R2);
      E_ = 1 / ( (1-nu1*nu1)/E1 + (1-nu2*nu2)/E2 );
    }

    ~contactModel() {}


    vector getForce(contactKinetic  &cK, contactMechanic &cM) const
    {
      // spring constant
      //scalar k    = cM._A*E_;
      // maximum deviation (neglecting damping)
      //scalar y_hat = cK._nDist + sqrt(m_/k)*cK._nVel;
      // ToDo
      // ...
      vector normalDist(cK._normalDist[0], cK._normalDist[1], cK._normalDist[2]);
      vector tangentVel(cK._tangentVel[0], cK._tangentVel[1], cK._tangentVel[2]);
      //vector forceN = cK._nDist * k * normalDist + sqrt(m_ * k) * normalVel;
      //vector forceN = -4/3 * E_ * sqrt(R_ * pow(mag(cK._nDist), 3)) * normalDist;
      vector forceN = cK._nDist * E_ * cM._A * normalDist;

      _DBO_("\nnormal dist = " << normalDist << endl)

      scalar D = 0.;

      if( cK._nVel > 0 )
        D = D_;

      vector forceD = cK._nVel * D * cM._A * normalDist;

      vector forceT = -mag(forceN) * mu_ * tangentVel;

//      _PDBO_(forceN << " " << forceD << " " << D)
//      _PDBO_(forceT)

//      _PDBO_(normalDist << " " << " " << tangentVel << " " << forceN << " " << cK._nDist << " " << cK._tDist)

      return  (forceN + forceT + forceD);
    }


    const string& getInfoStr() const
    {
      return infoStr_;
    }

    void info() const
    {
      Info << infoStr_ << endl;
    }


protected:

// youngs modulus (effective)
scalar E_;
// Coulomb friction coefficient
scalar mu_;
// coefficient of restitution
scalar e_;
// particle mass (effective)
scalar m_;
// particle Radius (effective)
scalar R_;
// damping factor
scalar D_;

string infoStr_;

}; // class contactModel


} // namespace Foam

#endif // contactModel_H
