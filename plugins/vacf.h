//
//
//

#ifndef __VACF_H__
#define __VACF_H__

#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/storedvalue.h>

class Vacf : public lpmd::StoredValue<lpmd::Matrix>,
             public lpmd::TemporalProperty,
             public lpmd::Plugin {
public:
  // Metodos Generales
  Vacf(std::string args);
  ~Vacf();
  void ShowHelp() const;

  // Metodos propios de modulo vacf
  void Evaluate(lpmd::ConfigurationSet& hist, lpmd::Potential& pot);

private:
  double dt;
};

#endif
