//
//
//

#ifndef __HARMONIC_POTENTIAL_H__
#define __HARMONIC_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class Harmonic: public lpmd::PairPotential, public lpmd::Plugin
{
 public: 
  //Metodos Generales
  Harmonic(std::string args); 
  ~Harmonic() { };
  void ShowHelp() const;

  //Metodos Propios de modulo harmonic
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double k,a;
};

#endif
