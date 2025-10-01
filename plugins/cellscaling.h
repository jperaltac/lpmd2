//
//
//

#ifndef __CELLSCALING_H__
#define __CELLSCALING_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class CellScalingModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  CellScalingModifier(std::string args);
  ~CellScalingModifier();
  void Show(std::ostream & os) const;
  void ShowHelp() const;

  //Metodos Propios de modulo cellscaling
  void Apply(lpmd::Simulation & sim);

 private:
  int axis;
  double percent;
  lpmd::Vector s[3];
  bool constant,first;
};

#endif



