//
//
//

#ifndef __CENTROSYM_H__
#define __CENTROSYM_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class CentroSymmetry: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  CentroSymmetry(std::string args);
  ~CentroSymmetry();
  void ShowHelp() const;
  void Evaluate(lpmd::Configuration & conf, lpmd::Potential & pot);

 private:
    double rcut, a;
};

#endif

