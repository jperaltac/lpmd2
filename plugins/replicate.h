//
//
//

#ifndef __REPLICATE_H__
#define __REPLICATE_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class ReplicateModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  ReplicateModifier(std::string args);
  ~ReplicateModifier();
  void ShowHelp() const;

  //Metodos Propios de modulo cellscaling
  void Apply(lpmd::Simulation & con);

 private:
  int nx, ny, nz;
};

#endif

