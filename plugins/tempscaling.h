//
//
//

#ifndef __TEMPSCALING_SM_H__
#define __TEMPSCALING_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class TempScalingModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  TempScalingModifier(std::string args);
  ~TempScalingModifier();
  void ShowHelp() const;

  //Metodos Propios
  void Apply(lpmd::Simulation & sim);

 private:
  double fromtemp, totemp;
};

#endif



