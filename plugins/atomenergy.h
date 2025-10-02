//
//
//

#ifndef __ATOMENERGY_H__
#define __ATOMENERGY_H__

#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/storedvalue.h>

class AtomEnergy : public lpmd::StoredValue<lpmd::Matrix>,
                   public lpmd::InstantProperty,
                   public lpmd::Plugin {
public:
  // Metodos Generales
  AtomEnergy(std::string args);
  void ShowHelp() const;

  void Evaluate(lpmd::Configuration& conf, lpmd::Potential& pot);
};

#endif
