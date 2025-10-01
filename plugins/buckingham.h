//
//
//

#ifndef __BUCK_POTENTIAL_H__
#define __BUCK_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class Buckingham: public lpmd::PairPotential , public lpmd::Plugin
{
 public:

  //Metodos Generales  
  Buckingham(std::string args); 
  ~Buckingham() { };
  void ShowHelp() const;

  //Metodos Propios de Modulo Buckingham
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double B1, Ro, B2, cutoff;
};


#endif

