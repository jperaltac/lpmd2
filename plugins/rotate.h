//
//
//

#ifndef __ROTATE_SM_H__
#define __ROTATE_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/vector.h>
#include <lpmd/plugin.h>
#include <lpmd/configuration.h>

using namespace lpmd;

class RotateModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  RotateModifier(std::string args);
  ~RotateModifier();

  void ShowHelp() const;

  //Metodos Propios
  void Apply(Configuration & conf);
  void Apply(Simulation & md);
 
 private:
  lpmd::Vector axis;
  double rotmat[3][3];                      // arbitrary rotation matrix
  double angle;
};

#endif

