//
//
//

#ifndef __NULLMETAL_H__
#define __NULLMETAL_H__

#include <lpmd/metalpotential.h>
#include <lpmd/plugin.h>
#include <lpmd/vector.h>

class NullMetalPotential: public lpmd::MetalPotential , public lpmd::Plugin
{
 public:
  //Metodos Generales
  NullMetalPotential(std::string args);
  ~NullMetalPotential(){};
  void ShowHelp() const;

  double pairEnergy(const double &r) const;
  double rhoij(const double &r) const;
  double F(const double &rhoi) const;
  lpmd::Vector PairForce(const lpmd::Vector &normrij, const double &mod) const;
  lpmd::Vector ManyBodies(const lpmd::Vector &normrij, const double &invrhoi, const double &invrhoj, const double &mod) const;
  double deltarhoi(const double & rhobar) const;
  double deltaU1(const double & rhobar, const int & N) const;
  double deltaU2(const double & rhobar, const int & N, const double & rhoi) const;
  double VirialCorrection(const double &rhobar, const int &N, const double &rhoi) const;
  lpmd::Vector UpdateCorrections(const double &rho, const int &N, const double &sinv) const;
};

#endif

