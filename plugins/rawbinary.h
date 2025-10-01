//
//
//

#ifndef __RAWBINMODULE_H__
#define __RAWBINMODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>
#include <lpmd/simulation.h>
#include <lpmd/array.h>
#include <string.h>

class RawBinFormat: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
  //Metodos Generales
  RawBinFormat(std::string args);
  virtual ~RawBinFormat();
  void ShowHelp() const;

  void ReadHeader(std::istream & is) const;
  bool ReadCell(std::istream & is, lpmd::Configuration & con) const;
  void WriteHeader(std::ostream & os, lpmd::SimulationHistory *) const;
  void WriteCell(std::ostream & os, lpmd::Configuration & con) const;
  long int GetInterval() const { return interval; }

 private:
  long int interval;
  long int level;
  bool rcell;
};

#endif

