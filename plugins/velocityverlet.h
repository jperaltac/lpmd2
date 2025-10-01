//
//
//

#ifndef __VELOCITYVERLET_H__
#define __VELOCITYVERLET_H__

#include <lpmd/twostepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

class VelocityVerlet: public lpmd::TwoStepIntegrator, public lpmd::Plugin
{
 public:
  //Metodos Generales
  VelocityVerlet(std::string args);
  ~VelocityVerlet();
  void ShowHelp() const;

  //Metodos Propios modulo velocityverlet
  void Initialize(lpmd::Simulation & sim, lpmd::Potential & p);
  void AdvancePosition(lpmd::Simulation & sim, long i);
  void AdvanceVelocity(lpmd::Simulation & sim, long i);
};

#endif
