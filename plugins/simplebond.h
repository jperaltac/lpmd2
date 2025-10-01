//
//
//

#ifndef __SIMPLEBOND_POTENTIAL_H__
#define __SIMPLEBOND_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class SimpleBond: public lpmd::PairPotential, public lpmd::Plugin
{
 public: 
  //Metodos Generales
  SimpleBond(std::string args); 
  ~SimpleBond() { };
  void ShowHelp() const;

  //Metodos Propios de modulo simplebond
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double epsilon, r0, sigma;
};

#endif
