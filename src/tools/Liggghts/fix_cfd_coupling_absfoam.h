/* ----------------------------------------------------------------------
    This is the

    ██╗     ██╗ ██████╗  ██████╗  ██████╗ ██╗  ██╗████████╗███████╗
    ██║     ██║██╔════╝ ██╔════╝ ██╔════╝ ██║  ██║╚══██╔══╝██╔════╝
    ██║     ██║██║  ███╗██║  ███╗██║  ███╗███████║   ██║   ███████╗
    ██║     ██║██║   ██║██║   ██║██║   ██║██╔══██║   ██║   ╚════██║
    ███████╗██║╚██████╔╝╚██████╔╝╚██████╔╝██║  ██║   ██║   ███████║
    ╚══════╝╚═╝ ╚═════╝  ╚═════╝  ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚══════╝®

    DEM simulation engine, released by
    DCS Computing Gmbh, Linz, Austria
    http://www.dcs-computing.com, office@dcs-computing.com

    LIGGGHTS® is part of CFDEM®project:
    http://www.liggghts.com | http://www.cfdem.com

    Core developer and main author:
    Christoph Kloss, christoph.kloss@dcs-computing.com

    LIGGGHTS® is open-source, distributed under the terms of the GNU Public
    License, version 2 or later. It is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. You should have
    received a copy of the GNU General Public License along with LIGGGHTS®.
    If not, see http://www.gnu.org/licenses . See also top-level README
    and LICENSE files.

    LIGGGHTS® and CFDEM® are registered trade marks of DCS Computing GmbH,
    the producer of the LIGGGHTS® software and the CFDEM®coupling software
    See http://www.cfdem.com/terms-trademark-policy for details.

-------------------------------------------------------------------------
    Contributing author and copyright for this file:
    This file is from LAMMPS
    LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
    http://lammps.sandia.gov, Sandia National Laboratories
    Steve Plimpton, sjplimp@sandia.gov

    Copyright (2003) Sandia Corporation.  Under the terms of Contract
    DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
    certain rights in this software.  This software is distributed under
    the GNU General Public License.
------------------------------------------------------------------------- 
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

Author
	Kamil Braschke
	Chair of Fluid Mechanics
	braschke@uni-wuppertal.de

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------------------- */

#ifdef FIX_CLASS

FixStyle(couple/cfd/absfoam, FixCfdCouplingABSFoam)

#else

#ifndef LMP_FIX_CFD_COUPLING_ABSFOAM_H
#define LMP_FIX_CFD_COUPLING_ABSFOAM_H

#include "fix_cfd_coupling.h"

namespace LAMMPS_NS {

class FixCfdCouplingABSFoam : public Fix  {
 public:
  FixCfdCouplingABSFoam(class LAMMPS *, int, char **);
  ~FixCfdCouplingABSFoam();
  void post_create();
  void pre_delete(bool unfixflag);

  int setmask();
  virtual void init();
  virtual void setup(int);
  virtual void post_force(int);
  double compute_vector(int n);

 protected:

  int iarg;

  void dont_use_force()
  { use_force_ = false; }

  double dragforce_total[3];
  double hdtorque_total[3];
  class FixCfdCoupling* fix_coupling_;
  class FixPropertyAtom* fix_dragforce_;
  class FixPropertyAtom* fix_hdtorque_; // hdtorque = hydrodynamic torque

  class FixPropertyAtom* fix_dispersionTime_;
  class FixPropertyAtom* fix_dispersionVel_;

  class FixPropertyAtom* fix_UrelOld_;

  bool use_force_, use_torque_, use_dens_, use_type_;
  bool use_stochastic_;
  bool use_virtualMass_;
  bool use_superquadric_;
  bool use_id_;

 private:
  bool use_property_;
  char property_name[200];
  char property_type[200];

  bool use_fiber_topo_;
  class FixPropertyAtom* fix_fiber_axis_;
  class FixPropertyAtom* fix_fiber_ends_;

};

}

#endif
#endif
