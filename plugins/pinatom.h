//
//
//

#ifndef __PINATOM_H__
#define __PINATOM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class PinAtomModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  PinAtomModifier(std::string args);
  ~PinAtomModifier();
  void ShowHelp() const;

  void Apply(lpmd::Simulation & sim);

 private:
  long index;
  lpmd::Vector * initial_pos;
};

#endif

