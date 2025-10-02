//
//
//

#ifndef __QUENCHEDMD_H__
#define __QUENCHEDMD_H__

#include <lpmd/plugin.h>
#include <lpmd/systemmodifier.h>

class QuenchedMDModifier : public lpmd::SystemModifier, public lpmd::Plugin {
public:
  // Metodos Generales
  QuenchedMDModifier(std::string args);
  ~QuenchedMDModifier();
  void ShowHelp() const;

  // Metodos Propios
  void Apply(lpmd::Simulation& sim);
};

#endif
