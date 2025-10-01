//
//
//

#ifndef __LJM_POTENTIAL_H__
#define __LJM_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class LennardJonesMod: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  LennardJonesMod(std::string args); 
  ~LennardJonesMod() { };
  void ShowHelp() const;

  //Metodos Propios de modulo lennardjones modificado
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double sigma, epsilon, m, n, cohesive, cutoff;
};


#endif

