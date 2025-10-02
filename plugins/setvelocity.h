//
//
//

#ifndef __SETVELOCITY_SM_H__
#define __SETVELOCITY_SM_H__

#include <lpmd/plugin.h>
#include <lpmd/systemmodifier.h>

class SetVelocityModifier : public lpmd::SystemModifier, public lpmd::Plugin {
public:
  // Metodos Generales
  SetVelocityModifier(std::string args);
  ~SetVelocityModifier();

  void ShowHelp() const;

  // Metodos Propios
  void Apply(lpmd::Simulation& sim);

private:
  lpmd::Vector velocity;
};

#endif
