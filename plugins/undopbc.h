//
//
//

#ifndef __UNDOPBC_H__
#define __UNDOPBC_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class UndoPBCModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  UndoPBCModifier(std::string args);
  ~UndoPBCModifier();
  void ShowHelp() const;

  void Apply(lpmd::Simulation & sim);

 private:
  lpmd::Vector * oldpositions;
  bool first_apply;
};

#endif

