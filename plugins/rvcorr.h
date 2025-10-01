//
//
//

#ifndef __RVCORR_H__
#define __RVCORR_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class RVCorr: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  RVCorr(std::string args);
  ~RVCorr();

  void ShowHelp() const;

  void Evaluate(lpmd::Configuration & conf, lpmd::Potential & pot);

 private:
    double rcut;
    int nb;
};

#endif

