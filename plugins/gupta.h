//
//
//

#ifndef __GUPTA_H__
#define __GUPTA_H__

#include <lpmd/metalpotential.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class Gupta: public lpmd::MetalPotential , public lpmd::Plugin
{
 public:
  //Metodos Generales
  Gupta(std::string args);
  ~Gupta(){};
  void ShowHelp() const;

  //Metodos Propios de modulo gupta
  double pairEnergy(const double &r) const;
  double rhoij(const double &r) const;
  double F(const double &rhoi) const;
  lpmd::Vector PairForce(const lpmd::Vector &normrij, const double &mod) const;
  lpmd::Vector ManyBodies(const lpmd::Vector &normrij, const double &invrhoi, const double &invrhoj, const double &mod) const;
  lpmd::Vector UpdateCorrections(const double &rho, const int &N, const double &sinv) const;

 private:
  double A,r0,p,B,q,rcut;
};

#endif
