//
//
//

#ifndef __NULLPOTENTIAL_H__
#define __NULLPOTENTIAL_H__

#include <lpmd/potential.h>
#include <lpmd/plugin.h>
#include <lpmd/configuration.h>

using namespace lpmd;

class NullPotential: public lpmd::Potential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  NullPotential(std::string args); 
  ~NullPotential();
  void ShowHelp() const;

  //Metodos Proopios de modulo nullpotential
  double energy(Configuration & conf);
  double AtomEnergy(Configuration & conf, long i);
  void UpdateForces(Configuration & conf);
};


#endif

