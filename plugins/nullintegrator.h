//
//
//

#ifndef __NULLINTEGRATOR_H__
#define __NULLINTEGRATOR_H__

#include <lpmd/integrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class NullIntegrator: public lpmd::Integrator, public lpmd::Plugin
{
 public: 
  //Metodos Generales
  NullIntegrator(std::string args);
  ~NullIntegrator();
  void ShowHelp() const;

  //Metodos propios de modulo nullintegrator
  void Advance(Simulation & sim, Potential & p);

};


#endif


