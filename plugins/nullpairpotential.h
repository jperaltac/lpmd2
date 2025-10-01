//
//
//

#ifndef __NULL_PAIRPOTENTIAL_H__
#define __NULL_PAIRPOTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class NullPairPotential: public lpmd::PairPotential, public lpmd::Plugin
{
 public: 
  //Metodos Generales
  NullPairPotential(std::string args); 
  ~NullPairPotential() { };
  void ShowHelp() const;

  //Metodos Propios modulo nullpairpotential
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;


};


#endif

