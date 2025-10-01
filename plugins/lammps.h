//
//
//

#ifndef __LAMMPSMODULE_H__
#define __LAMMPSMODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>
#include <lpmd/simulation.h>
#include <lpmd/array.h>
#include <lpmd/indirectatom.h>
#include <string>

class LAMMPSFormat: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
   //Metodos Generales
   LAMMPSFormat(std::string args);
   virtual ~LAMMPSFormat();
   void ShowHelp() const;

   //Metodos Propios de modulo LAMMPS
   void ReadHeader(std::istream & is) const;
   void ReadHeaderData(std::istream & is) const;
   bool ReadCell(std::istream & is, lpmd::Configuration & con) const;
   bool SkipCell(std::istream & is) const;
   void WriteHeader(std::ostream & os, lpmd::SimulationHistory *) const;
   void WriteCell(std::ostream & os, lpmd::Configuration & con) const;
   
   long int GetInterval() const {return interval;}

 private:
   long int * linecounter;
   long int interval;
   mutable long int level;
   std::string inside, displace;
   std::string speclist;
   mutable lpmd::Array<std::string> species; // atomic symbols
   bool rcell;
   mutable lpmd::Array<std::string> hdr;
   mutable std::string ftype, datatype;
   /////////////////////////////////////////
   //private variables for data file-types///
   /////////////////////////////////////////
   mutable lpmd::Vector CellVect[3];
   mutable long int natoms;
   lpmd::Atom noc;
};

#endif

