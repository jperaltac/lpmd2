//
//
//

#ifndef __CORDNUM_H__
#define __CORDNUM_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

#include <map>

class CordNum: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  //Metodos Generales
  CordNum(std::string args);
  ~CordNum();
  void SetParameter(std::string name);
  void Show(std::ostream & os) const;
  void ShowHelp() const;

  //Metodos Propios del Modulo cordnum
  void Evaluate(lpmd::Configuration & con, lpmd::Potential & pot);

 private:
  int nb;
  std::map<std::string, double> rcut;
  int na;
  std::vector<std::string> satoms; //atomic symbols.
  double trcut;
};

#endif

