//
//
//

#ifndef __SUTTON_H__
#define __SUTTON_H__

#include <lpmd/metalpotential.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class SuttonChen: public lpmd::MetalPotential , public lpmd::Plugin
{
 public:
  //Metodos Generales
  SuttonChen(std::string args);
  ~SuttonChen(){};
  void ShowHelp() const;

  //Metodos Propios de modulo suttonchen
  double pairEnergy(const double &r) const;
  double rhoij(const double &r) const;
  double F(const double &rhoi) const;
  lpmd::Vector PairForce(const lpmd::Vector &normrij, const double & mod) const;
  lpmd::Vector ManyBodies(const lpmd::Vector &normrij, const double &rhoi, const double &rhoj, const double & mod) const;
  lpmd::Vector UpdateCorrections(const double & rho, const int & N, const double & sinv) const;

 private:
  double e,a,n,m,c,rcut;
  double an,am;
  bool corrections;
};

#endif
