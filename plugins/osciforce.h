//
//
//

#ifndef __OSCILATORY_FORCE_POTENTIAL_H__
#define __OSCILATORY_FORCE_POTENTIAL_H__

#include <lpmd/potential.h>
#include <lpmd/plugin.h>

class OsciForcePotential: public lpmd::Potential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  OsciForcePotential(std::string args);
  ~OsciForcePotential();

  void ShowHelp() const;

  double energy(lpmd::Configuration & con);
  double AtomEnergy(lpmd::Configuration & con, long i);
  void UpdateForces(lpmd::Configuration & con);

 private:
  lpmd::Vector force;
  double phase;
  int n;
  int counter;
};


#endif

