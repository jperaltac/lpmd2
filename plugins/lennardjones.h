//
//
//

#ifndef __LJ_POTENTIAL_H__
#define __LJ_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class LennardJones: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  LennardJones(std::string args); 
  ~LennardJones() { };
  void ShowHelp() const;

  //Metodos Propios de modulo lennardjones
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double sigma, epsilon, cutoff;
};


#endif

