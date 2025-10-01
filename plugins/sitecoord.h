//
//
//

#ifndef __SITECOORD_H__
#define __SITECOORD_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class SiteCoord: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  SiteCoord(std::string args);
  ~SiteCoord();
  void ShowHelp() const;

  void Evaluate(lpmd::Configuration & conf, lpmd::Potential & pot);

 private:
    double rcut;
};

#endif

