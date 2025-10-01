//
//
//

#ifndef __SKEWSTART_H__
#define __SKEWSTART_H__

#include <lpmd/cellgenerator.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class SkewStartGenerator: public lpmd::CellGenerator, public lpmd::Plugin
{
 public:
  //Metodos Generales
  SkewStartGenerator(std::string args); 
  virtual ~SkewStartGenerator();
  void ShowHelp() const;

  //Metodos propios del modulo skewstartgenerator
  void Generate(Configuration & config) const;

 private:
  long n;   // Number of atoms
  int spc;  // which species (atomic number)
};

#endif


