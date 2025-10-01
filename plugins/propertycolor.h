//
//
//

#ifndef __PROPERTYCOLOR_SM_H__
#define __PROPERTYCOLOR_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/colorhandler.h>
#include <lpmd/plugin.h>

class PropertyColorModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  PropertyColorModifier(std::string args);
  ~PropertyColorModifier();

  void ShowHelp() const;

  void Apply(lpmd::Simulation & sim);

 private:
  double vmin, vmax, cutoff;
  int column, extheader;
  std::string property; 
  std::ifstream * extfile;
};

#endif

