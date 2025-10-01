//
//
//

#ifndef __TAGSURFACE_H__
#define __TAGSURFACE_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class TagSurfaceModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
  //Metodos Generales
  TagSurfaceModifier(std::string args);
  ~TagSurfaceModifier();
  void ShowHelp() const;

  void Apply(lpmd::Simulation & sim);

 private:
  std::string symbol,direction;
  double R0;
};

#endif



