//
//
//

#ifndef __MSD_H__
#define __MSD_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class MSD: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::TemporalProperty, public lpmd::Plugin
{
 public:
  MSD(std::string args);
  void ShowHelp() const;

  void Evaluate(lpmd::ConfigurationSet & hist, lpmd::Potential & pot);
  void ZeroCM(lpmd::Configuration & conf);

 private:
  double rcutmin, rcutmax;
  bool zerocm;
};

#endif

