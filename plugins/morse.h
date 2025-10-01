//
//
//

#ifndef __MORSE_POTENTIAL_H__
#define __MORSE_POTENTIAL_H__

#include <lpmd/pairpotential.h>
#include <lpmd/plugin.h>

class Morse: public lpmd::PairPotential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  Morse(std::string args); 
  ~Morse() { };
  void ShowHelp() const;

  //Metodos propios modulo morse
  double pairEnergy(const double & r) const;
  lpmd::Vector pairForce(const lpmd::Vector & r) const;

 private:
  double depth, a, re;

};


#endif

