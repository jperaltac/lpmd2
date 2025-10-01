//
//
//

#ifndef __VASPMODULE_H__
#define __VASPMODULE_H__

#include <lpmd/cellformat.h>
#include <lpmd/plugin.h>

class VaspFormat: public lpmd::CellFormat, public lpmd::Plugin
{
 public:
  //Metodos Generales
  VaspFormat(std::string args);
  virtual ~VaspFormat();
  void ShowHelp() const;

  //Metodos propios de modulo vasp
  void ReadHeader(std::istream & is) const;
  bool ReadCell(std::istream & is, lpmd::Configuration & con) const;
  void WriteHeader(std::ostream & os, lpmd::SimulationHistory * sh) const;
  void WriteCell(std::ostream & os, lpmd::Configuration & con) const;
  long int GetInterval() const { return interval; }

 private:
  long int interval;
  int level;
  bool rcell;
  mutable bool first;
  std::string speclist;
  std::string numatoms;
  mutable lpmd::Array<std::string> satoms; // atomic symbols
  mutable lpmd::Array<std::string> numesp; // number of atom per spicies
  std::string tp;  // formato posiciones (Direct/Cartesian)
  std::string ftype; // file type (POSCAR, CONTCAR, XDATCAR)
};

#endif

