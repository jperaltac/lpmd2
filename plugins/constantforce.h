//
//
//

#ifndef __CONSTANT_FORCE_POTENTIAL_H__
#define __CONSTANT_FORCE_POTENTIAL_H__

#include <lpmd/potential.h>
#include <lpmd/plugin.h>

class ConstantForcePotential: public lpmd::Potential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  ConstantForcePotential(std::string args);
  ~ConstantForcePotential();

  void ShowHelp() const;

  double energy(lpmd::Configuration & con);
  double AtomEnergy(lpmd::Configuration & con, long i);
  void UpdateForces(lpmd::Configuration & con);

 private:
  lpmd::Vector force;
};


#endif

