//
//
//

#ifndef __SHEAR_SM_H__
#define __SHEAR_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/vector.h>
#include <lpmd/plugin.h>

class ShearModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  ShearModifier(std::string args);
  ~ShearModifier();

  void ShowHelp() const;

  //Metodos Propios
  void Apply(lpmd::Simulation & sim);
 
 private:
   int shear_axis, perp_axis;
   double strain;
};

#endif

