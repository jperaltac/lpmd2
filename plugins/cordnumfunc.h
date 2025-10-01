//
//
//

#ifndef __CORDNUMFUNC_H__
#define __CORDNUMFUNC_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class CordNumFunc: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  //Metodos Generales
  CordNumFunc(std::string args);
  ~CordNumFunc();

  void ShowHelp() const;

  void Evaluate(lpmd::Configuration & con, lpmd::Potential & pot);

 private:
  int nb;
  double cut;
};

#endif

