//
//
//

#ifndef __TABULATED_PAIR_POTENTIAL_H__
#define __TABULATED_PAIR_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class TabulatedPair: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  TabulatedPair(std::string args); 
  ~TabulatedPair();
  void ShowHelp() const;

  //Metodos propios del modulo fastlj
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

  void ReadTable();

 private:
  std::string file;
  long bins;
  double *etable, *ftable, cutoff;
};


#endif

