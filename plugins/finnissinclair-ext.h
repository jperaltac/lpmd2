//
//
//

#ifndef __FINNISEXT_H__
#define __FINNISEXT_H__

#include <lpmd/metalpotential.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class FinnisSinclairExt: public lpmd::MetalPotential , public lpmd::Plugin
{
 public:
  //Metodos Generales
  FinnisSinclairExt(std::string args);
  ~FinnisSinclairExt(){};
  void ShowHelp() const;

  //Metodos Propios de modulo FinnisSinclairExt
  double pairEnergy(const double &r) const;
  double rhoij(const double &r) const;
  double F(const double &rhoi) const;
  lpmd::Vector PairForce(const lpmd::Vector &modrij, const double &mod) const;
  lpmd::Vector ManyBodies(const lpmd::Vector &modrij, const double &invrhoi, const double &invrhoj, const double & mod) const;
  lpmd::Vector UpdateCorrections(const double &rho, const int &N, const double &sinv) const;

 private:
  double c0,c1,c2,c3,c4,A,B,c,d;	// Parameters "c" and "d" are cutoffs
};

#endif
