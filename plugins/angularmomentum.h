//
//
//

#ifndef __ANGMOM_H__
#define __ANGMOM_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>

class AngularMomentum: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  AngularMomentum(std::string args);
  ~AngularMomentum();
  void ShowHelp() const;

  void Evaluate(lpmd::Configuration & conf, lpmd::Potential & pot);
 private:
  lpmd::Vector center;
};

#endif

