//
//
//

#ifndef __EULER_H__
#define __EULER_H__

#include <lpmd/onestepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class Euler: public lpmd::OneStepIntegrator, public lpmd::Plugin
{
 public:
  //Metodos Generales
  Euler(std::string args);
  ~Euler();
  void ShowHelp() const;

  //Metodos Propios del modulo euler
  void Advance(Simulation & sim, long i);
};

#endif


