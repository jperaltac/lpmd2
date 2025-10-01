//
//
//

#ifndef __XYZMODULE_H__
#define __XYZMODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>
#include <lpmd/simulation.h>

#include <iomanip>

class XYZFormat: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
  //Metodos Generales
  XYZFormat(std::string args);
  virtual ~XYZFormat();
  void ShowHelp() const;

  //Metodos propios de modulo xyz
  void ReadHeader(std::istream & is) const;
  bool ReadCell(std::istream & is, lpmd::Configuration & c) const;
  bool SkipCell(std::istream & is) const;
  void WriteHeader(std::ostream & os, lpmd::SimulationHistory *sh) const;
  void WriteCell(std::ostream & os, lpmd::Configuration & c) const;
  long int GetInterval() const { return interval; }

 private:
  long int * linecounter;
  long int interval;
  long int level;
  bool rcell;
  std::string coords, inside;
  std::string external, zerocm;
  long int precout;
};

#endif

