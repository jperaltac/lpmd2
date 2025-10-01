//
//
//

#ifndef __DISPLACE_SM_H__
#define __DISPLACE_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/vector.h>
#include <lpmd/plugin.h>

class DisplaceModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  DisplaceModifier(std::string args);
  ~DisplaceModifier();

  void ShowHelp() const;

  //Metodos Propios
  void Apply(lpmd::Simulation & sim);
 
 private:
  lpmd::Vector offset;
};

#endif

