//
//
//

#ifndef __PD_H__
#define __PD_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class PairDistances: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  //Metodos Generales
  PairDistances(std::string args);
  void ShowHelp() const;

  //Metodos Propios de modulo pairdistances
  void Evaluate(lpmd::Configuration & conf, lpmd::Potential & pot);

 private:
    double rcut;
};

#endif

