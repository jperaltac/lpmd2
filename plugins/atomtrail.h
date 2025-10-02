/*
 *
 *
 *
 *
 */

#ifndef __ATOMTRAIL_PLUGIN_H__
#define __ATOMTRAIL_PLUGIN_H__

#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/storedvalue.h>

class AtomTrail : public lpmd::StoredValue<lpmd::Matrix>,
                  public lpmd::InstantProperty,
                  public lpmd::Plugin {
public:
  // Metodos Generales
  AtomTrail(std::string args);
  ~AtomTrail();
  void ShowHelp() const;
  void Evaluate(lpmd::Configuration& config, lpmd::Potential& pot);

private:
  int nx, ny, nz;
  std::string plane, species, mode;
};

#endif
