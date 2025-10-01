//
//
//

#ifndef __CRYSTAL3D_H__
#define __CRYSTAL3D_H__

#include <lpmd/cellgenerator.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class CrystalGenerator: public lpmd::CellGenerator, public lpmd::Plugin
{
 public:
   //Metodos Generales
   CrystalGenerator(std::string args);
   virtual ~CrystalGenerator();
   void ShowHelp() const;

   //Metodos Propios del modulo fccgenerator
   void Generate(Configuration & config) const;

 private:
   int spc;
   long nx, ny, nz;
   std::string type; // type=bcc,fcc,hcp,sc
};

#endif
