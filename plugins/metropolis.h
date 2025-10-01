//
//
//

#ifndef __METROPOLIS_H__
#define __METROPOLIS_H__

#include <lpmd/onestepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

class Metropolis: public lpmd::OneStepIntegrator, public lpmd::Plugin
{
 public:
  //Metodos Generales
  Metropolis(std::string args);
  ~Metropolis();
  void ShowHelp() const;

  //Metodos propios modulo verlet
  void Initialize(lpmd::Simulation & sim, lpmd::Potential & p);
  void Advance(lpmd::Simulation & sim, long i);
 private:
  double Temp;
  double rand,percent;
};


#endif


