//
//
//

#ifndef __ADDVELOCITY_H__
#define __ADDVELOCITY_H__

#include <lpmd/plugin.h>
#include <lpmd/systemmodifier.h>

class AddVelocityModifier : public lpmd::SystemModifier, public lpmd::Plugin {
public:
  // Metodos Generales
  AddVelocityModifier(std::string args);
  ~AddVelocityModifier();
  void ShowHelp() const;

  void Apply(lpmd::Simulation& sim);

private:
  lpmd::Vector velocity;
};

#endif
