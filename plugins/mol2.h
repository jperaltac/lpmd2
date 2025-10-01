//
//
//

#ifndef __MOL2MODULE_H__
#define __MOL2MODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>

class Mol2Format: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
   //Metodos Generales
   Mol2Format(std::string args);
   virtual ~Mol2Format();
   void ShowHelp() const;

   //Metodos Propios de modulo mol2
   void WriteHeader(std::ostream & os, lpmd::SimulationHistory * sh) const;
   void ReadHeader(std::istream & is) const;
   void WriteCell(std::ostream & os, lpmd::Configuration & con) const;
   bool ReadCell(std::istream & is, lpmd::Configuration & con) const;
   
   long int GetInterval() const {return interval;}

 private:
   long int interval;
   bool rcell;
};

#endif

