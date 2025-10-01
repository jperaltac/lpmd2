//
//
//

#ifndef __CRYSTAL2D_H__
#define __CRYSTAL2D_H__

#include <lpmd/cellgenerator.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class Crystal2DGenerator: public lpmd::CellGenerator, public lpmd::Plugin
{
 public:
   
  //Metodos Generales
  Crystal2DGenerator(std::string args);
  virtual ~Crystal2DGenerator();
  void ShowHelp() const;

  //Metodos Propios del module scgnerator
  void Generate(Configuration & conf) const;

 private:
   int spc;
   long nx, ny, nz;
   double a, b, gamma;
};

#endif

