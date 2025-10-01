//
//
//

#ifndef __SETCOLOR_SM_H__
#define __SETCOLOR_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/colorhandler.h>
#include <lpmd/plugin.h>

class SetColorModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  SetColorModifier(std::string args);
  ~SetColorModifier();

  void ShowHelp() const;

  //Metodos Propios
  void Apply(lpmd::Simulation & sim);

 private:
  lpmd::Color color;
};

#endif

