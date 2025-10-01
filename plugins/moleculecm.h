//
//
//

#ifndef __MOLECULE_CM_H__
#define __MOLECULE_CM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class MoleculeCMModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  MoleculeCMModifier(std::string args);
  ~MoleculeCMModifier();
  void ShowHelp() const;

  //Metodos Propios de modulo cellscaling
  void Apply(lpmd::Simulation & con);

 private:
  double radius;
};

#endif



