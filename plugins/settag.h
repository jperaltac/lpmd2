//
//
//

#ifndef __SETTAG_SM_H__
#define __SETTAG_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class SetTagModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  SetTagModifier(std::string args);
  ~SetTagModifier();

  void ShowHelp() const;

  //Metodos Propios
  void Apply(lpmd::Simulation & sim);

 private:
  std::string tag;
  std::string value;
};

#endif

