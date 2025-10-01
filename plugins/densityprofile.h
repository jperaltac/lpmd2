//
//
//

#ifndef __DENSITYPROFILE_H__
#define __DENSITYPROFILE_H__

#include <lpmd/storedvalue.h>
#include <lpmd/matrix.h>
#include <lpmd/property.h>
#include <lpmd/plugin.h>
#include <lpmd/util.h>

class DensityProfile: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  //Metodos Generales
  DensityProfile(std::string args);
  ~DensityProfile();
  void SetParameter(std::string name);
  void Show(std::ostream & os) const;
  void ShowHelp() const;

  //Metodos Propios de modulo gdr
  void Evaluate(lpmd::Configuration & con, lpmd::Potential & pot);

 private:
    int bins;
    int axis;
    double range[3][2];
    long int counter;
};

#endif

