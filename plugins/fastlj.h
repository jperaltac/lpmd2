//
//
//

#ifndef __FASTLJ_POTENTIAL_H__
#define __FASTLJ_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class FastLJ: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  FastLJ(std::string args); 
  ~FastLJ();
  void ShowHelp() const;

  //Metodos propios del modulo fastlj
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:  
  long bins;
  double sigma, epsilon, cutoff;
  double *etable, *fftable;  
  void Tabulate();
};


#endif

