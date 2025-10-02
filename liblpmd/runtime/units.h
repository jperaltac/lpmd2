#ifndef __LPMD_RUNTIME_UNITS_H__
#define __LPMD_RUNTIME_UNITS_H__

namespace lpmd
{
 struct UnitSystem
 {
  double forcefactor;
  double kboltzmann;
  double kin2ev;
  double pressfactor;
  double ua3togrcm3;
  double q2a2force;
  double q2a2ev;
 };

 const UnitSystem & DefaultUnitSystem();
}

#endif
