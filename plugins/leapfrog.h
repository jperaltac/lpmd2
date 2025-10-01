//
//
//

#ifndef __LEAPFROG_H__
#define __LEAPFROG_H__

#include <lpmd/onestepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class Leapfrog: public OneStepIntegrator, public Plugin
{
 public:
  //Metodos Generales
  Leapfrog(std::string args);
  ~Leapfrog();
  void ShowHelp() const;

  //Metodos propios modulo verlet
  void Initialize(Simulation & sim, Potential & p);
  void Advance(Simulation & sim, long i);
};


#endif


