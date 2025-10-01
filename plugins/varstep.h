//
//
//

#ifndef __VARIABLESTEP_H__
#define __VARIABLESTEP_H__

#include <lpmd/integrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

class VariableStep: public lpmd::Integrator, public lpmd::Plugin
{
 public:
  VariableStep(std::string args);
  ~VariableStep();
  void ShowHelp() const;

  void Initialize(lpmd::Simulation & sim, lpmd::Potential & p);
  void Advance(lpmd::Simulation & sim, lpmd::Potential & p);

 private:
   lpmd::Vector * vhalf;
   double curr_dt, elapsed_time, scalefactor;
};

#endif
