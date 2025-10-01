//
//
//

#ifndef __NOSEHOOVER_H__
#define __NOSEHOOVER_H__

#include <lpmd/vector.h>
#include <lpmd/twostepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

class NoseHoover: public lpmd::TwoStepIntegrator, public lpmd::Plugin
{
 public:
   //Metodos Generales
   NoseHoover(std::string args);
   ~NoseHoover();
   void ShowHelp() const;

   //Metodos Propios del Modulo NoseHoover
   void Initialize(lpmd::Simulation & sim, lpmd::Potential & p);
   void AdvancePosition(lpmd::Simulation & sim, long i);
   void AdvanceVelocity(lpmd::Simulation & sim, long i);

 private:
      double q;
      double temp;
      double friction;
      std::vector<lpmd::Vector> auxlist;
};

#endif


