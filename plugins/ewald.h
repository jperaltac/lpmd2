//
//
//

#ifndef __EWALDPOTENTIAL_H__
#define __EWALDPOTENTIAL_H__

#include <lpmd/potential.h>
#include <lpmd/plugin.h>

#include <lpmd/vector.h>

class Ewald: public lpmd::Potential, public lpmd::Plugin
{
 public:
  //Metodos Generales
  Ewald(std::string args); 
  ~Ewald();
  void ShowHelp() const;

  //Metodos Proopios de modulo ewald
  double energy(lpmd::Configuration & conf);
  double AtomEnergy(lpmd::Configuration & conf, long i);
  void UpdateForces(lpmd::Configuration & conf);

 private:
   int kmax;
   bool surfdip;
   double alpha, rcut, kcut, ecorr;
   std::vector<lpmd::Vector> * kpoints;
   double * kfac;
   //
   void BuildKPointMesh(lpmd::Configuration & conf);
   void RealSpace(lpmd::Configuration & conf, double & e);
   void ReciprocalSpace(lpmd::Configuration & conf, double & e);
   void SurfaceDipole(lpmd::Configuration & conf, lpmd::Vector * forces, double & e);
   double EnergyConstantCorrection(lpmd::Configuration & conf);
};


#endif

