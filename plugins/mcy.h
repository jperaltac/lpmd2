//
//
//

#ifndef __MCY_POTENTIAL_H__
#define __MCY_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class MCY: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  MCY(std::string args); 
  ~MCY() { };
  void ShowHelp() const;

  //Metodos propios modulo mcy
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
   double A, B, C, D, cutoff;
};


#endif

