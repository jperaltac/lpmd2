//
//
//

#ifndef __VELDIST_H__
#define __VELDIST_H__

#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/storedvalue.h>

class VelDist : public lpmd::StoredValue<lpmd::Matrix>,
                public lpmd::InstantProperty,
                public lpmd::Plugin {
public:
  VelDist(std::string args);
  ~VelDist();
  void ShowHelp() const;

  void Evaluate(lpmd::Configuration& conf, lpmd::Potential& pot);

private:
  int bins;
};

#endif
