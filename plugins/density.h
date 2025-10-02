//
//
//

#ifndef __DENSITY_H__
#define __DENSITY_H__

#include <lpmd/configuration.h>
#include <lpmd/plugin.h>
#include <lpmd/systemmodifier.h>
#include <lpmd/vector.h>

using namespace lpmd;

class DensityModifier : public lpmd::SystemModifier, public lpmd::Plugin {
public:
  // Metodos Generales
  DensityModifier(std::string args);
  ~DensityModifier();

  void ShowHelp() const;

  // Metodos Propios
  void Apply(Simulation& conf);

private:
  std::string type;
  double density;
};

#endif
