//
//
//

#ifndef __DLPOLYMODULE_H__
#define __DLPOLYMODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>
#include <lpmd/simulation.h>

class DlPolyFormat: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
  //Metodos Generales
  DlPolyFormat(std::string args);
  virtual ~DlPolyFormat();
  void ShowHelp() const;

  //Metodos propios de modulo dlpoly
  void ReadHeader(std::istream & is) const;
  bool ReadCell(std::istream & is, lpmd::Configuration & con) const;
  bool SkipCell(std::istream & is) const;
  void WriteHeader(std::ostream & os, lpmd::SimulationHistory *sh) const;
  void WriteCell(std::ostream & os, lpmd::Configuration & con) const;
  long int GetInterval() const { return interval; }

 private:
  long int interval, *stepcnt;
  int level, pbkey, Nconfigs, Natoms;
  std::string ftype;
  bool rcell;
  double dt;
};

#endif

