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
#ifndef contact_H
#define contact_H

#include <string.h>
#include <word.H>

#include  <unordered_set>

#include "makros.H"


#ifdef WM_SP
  typedef float LSMreal;
#else
  typedef double  LSMreal;
#endif

#define _VECDIM3_  3
#define _VECDIM6_  6
#define _SIZEVEC3_ (_VECDIM3_*sizeof(LSMreal))
#define _SIZEVEC6_ (_VECDIM6_*sizeof(LSMreal))
#define _SIZEVEC_  _SIZEVEC3_


///////////////////////////////////////////////////////////////////////////////
//
// struct facePair
//
struct facePair
{
  enum course
  {
    candidate,
    established,
    released
  };

  facePair(
            int    fMaster    = -1,
            int    fSlave     = -1,
            int    popIdMaster = -1,
            int    popIdSlave  = -1,
      Foam::word   pIdStrMaster = "none",
      Foam::word   pIdStrSlave  = "none",
            course history    = candidate
          ) :
          _fMaster(fMaster),
          _fSlave(fSlave),
          _popIdMaster(popIdMaster),
          _popIdSlave(popIdSlave),
          _pIdStrMaster(pIdStrMaster),
          _pIdStrSlave(pIdStrSlave),
          _history(history)
  {}

  facePair(
            const facePair &fP
          ) :
          _fMaster(fP._fMaster),
          _fSlave(fP._fSlave),
          _popIdMaster(fP._popIdMaster),
          _popIdSlave(fP._popIdSlave),
          _pIdStrMaster(fP._pIdStrMaster),
          _pIdStrSlave(fP._pIdStrSlave),
          _history(fP._history)
  {}

  bool operator==(const facePair &right) const
  {
    return (
               _fMaster == right._fMaster
            && _fSlave  == right._fSlave
            && _popIdMaster == right._popIdMaster
            && _popIdSlave  == right._popIdSlave
            && _pIdStrMaster == right._pIdStrMaster
            && _pIdStrSlave  == right._pIdStrSlave
           );
  }

        // face numbers
        int     _fMaster;
        int     _fSlave;
        int     _popIdMaster;
        int     _popIdSlave;
        // global particle ids
  Foam::word    _pIdStrMaster;
  Foam::word    _pIdStrSlave;
        course  _history;
};

///////////////////////////////////////////////////////////////////////////////
//
// struct contactKinetic
//

struct contactKinetic
{
  contactKinetic(
                const LSMreal normalDist[]  = 0,
                const LSMreal tangentDist[] = 0,
                      LSMreal nDist         = 0,
                      LSMreal tDist         = 0,
                      LSMreal distance      = 0,

                const LSMreal normalVel[]  = 0,
                const LSMreal tangentVel[] = 0,
                      LSMreal nVel         = 0,
                      LSMreal tVel         = 0,
                      LSMreal velocity     = 0
                ) :
                _nDist(nDist),
                _tDist(tDist),
                _distance(distance),
                _nVel(nVel),
                _tVel(tVel),
                _velocity(velocity)
  {
      if(normalDist)
        memcpy(_normalDist, normalDist, _SIZEVEC_);
      else
        _normalDist[0] = _normalDist[1] = _normalDist[2] = 0.0;

      if(tangentDist)
        memcpy(_tangentDist, tangentDist, _SIZEVEC_);
      else
        _tangentDist[0] = _tangentDist[1] = _tangentDist[2] = 0.0;

      if(normalVel)
        memcpy(_normalVel, normalVel, _SIZEVEC_);
      else
          _normalVel[0] = _normalVel[1] = _normalVel[2] = 0.0;

      if(tangentVel)
        memcpy(_tangentVel, tangentVel, _SIZEVEC_);
      else
          _tangentVel[0] = _tangentVel[1] = _tangentVel[2] = 0.0;
  }

  contactKinetic(
                  const contactKinetic &cK
                ) :
                _nDist(cK._nDist),
                _tDist(cK._tDist),
                _distance(cK._distance),
                _nVel(cK._nVel),
                _tVel(cK._tVel),
                _velocity(cK._velocity)
  {
    memcpy(_normalDist,  cK._normalDist,  _SIZEVEC_);
    memcpy(_tangentDist, cK._tangentDist, _SIZEVEC_);
    memcpy(_normalVel,   cK._normalVel,   _SIZEVEC_);
    memcpy(_tangentVel,  cK._tangentVel,  _SIZEVEC_);
  }

  contactKinetic& operator=(const contactKinetic &cK)
  {
    if(this == &cK)
      return *this;

    this->_nDist    = cK._nDist;
    this->_tDist    = cK._tDist;
    this->_distance = cK._distance;
    this->_nVel     = cK._nVel;
    this->_tVel     = cK._tVel;
    this->_velocity = cK._velocity;

    memcpy(this->_normalDist,  cK._normalDist,  _SIZEVEC_);
    memcpy(this->_tangentDist, cK._tangentDist, _SIZEVEC_);
    memcpy(this->_normalVel,   cK._normalVel,   _SIZEVEC_);
    memcpy(this->_tangentVel,  cK._tangentVel,  _SIZEVEC_);

    return *this;
  }

  /////////////////////////////////////
  // positional parameters

  // unit normal for distance
  LSMreal  _normalDist[_VECDIM3_];
  // unit tangential for distance
  LSMreal  _tangentDist[_VECDIM3_];
  // normal component for distance
  LSMreal  _nDist;
  // tangential component for distance
  LSMreal  _tDist;
  // absolute distance: | _nDist*_normalDist + _tDist*_tangentDist |
  LSMreal  _distance;



  /////////////////////////////////////
  // separation velocity parameters

  // unit normal for velocity
  LSMreal  _normalVel[_VECDIM3_];
  // unit tangential for velocity
  LSMreal  _tangentVel[_VECDIM3_];
  // normal component for velocity
  LSMreal  _nVel;
  // tangential component for velocity
  LSMreal  _tVel;
  // absolute velocity: | _nVel*_normalVel + _tVel*_tangentVel |
  LSMreal  _velocity;

};

///////////////////////////////////////////////////////////////////////////////
//
// struct contactMechanic
//
struct contactMechanic
{
  contactMechanic(
                   LSMreal A  = 1.0
                 ) :
                 _A(A)
  {}

  contactMechanic(
                   const contactMechanic &cM
                 ) :
                 _A(cM._A)
  {}

  // contact area
  LSMreal  _A;
};

struct contactForce
{
  contactForce(
                LSMreal fn   = 0,
                LSMreal ft   = 0
              ) :
              _fn(fn),
              _ft(ft)
  {}

  contactForce(
          const contactForce &cF
              ) :
              _fn(cF._fn),
              _ft(cF._ft)
  {}

  // normal force
  LSMreal _fn;
  // tangential force
  LSMreal _ft;

};

///////////////////////////////////////////////////////////////////////////////
//
// struct contactState
//
struct contactState : public facePair,
                      public contactKinetic,
                      public contactMechanic,
                      public contactForce
{
  contactState(
          const facePair &fP
              ) : facePair(fP),
                  contactKinetic(),
                  contactMechanic(),
                  contactForce()
  {}

};

///////////////////////////////////////////////////////////////////////////////
//
// single linked list
//

template<typename T>
struct  slimListItem
{

  slimListItem(): _elem(),
                  _next(0)
  {}

  slimListItem(
                const T& elem
              ) : _elem(elem),
                  _next(0)
  {}

  T             _elem;
  slimListItem* _next;
};

template<typename T>
struct slimList
{
  slimList(
          ) : _n(0),
              _root(0),
              _last(0)
  {}

  ~slimList()
  {
    while(_root)
      pop();
  }

  bool empty()
  {
    return (_n == 0);
  }

  unsigned int size()
  {
    return _n;
  }

  // append at end
  void append(const T &elem)
  {
    slimListItem<T> *tmp = new slimListItem<T>(elem);

    if(_n)
      _last->_next = tmp;
    else
      _root = tmp;

    _last = tmp;
    _n++;
  }

  // delete first element
  void pop()
  {
    if(!_root)
      return;

    slimListItem<T> *tmp = _root->_next;
    delete _root;
    _root = tmp;
    _n--;
  }

  unsigned int     _n;
  slimListItem<T> *_root;
  slimListItem<T> *_last;
};


///////////////////////////////////////////////////////////////////////////////


class contactHash
//class hash<contactState>
{
public:
    size_t operator()(const contactState &s) const
    {
      int fMaster    = s._fMaster;
      int fSlave     = s._fSlave;
      int pIdxMaster = Foam::word::hash()(s._pIdStrMaster);
      int pIdxSlave  = Foam::word::hash()(s._pIdStrSlave);

      return fMaster + fSlave + pIdxMaster + pIdxSlave;
    }
};

class contact_equal_to
//class equal_to<contactState>
{
public:
    size_t operator()(const contactState &left, const contactState &right) const
    {
      // equality :<=> (identical facePairs)
      return static_cast<facePair>(left) == static_cast<facePair>(right);
    }
};

typedef std::unordered_set<contactState, contactHash, contact_equal_to>  contactHash_t;

typedef std::unordered_set<contactState, contactHash, contact_equal_to>::iterator  contactIter_t;


#endif // contact_H






