//
//
//

#ifndef __DRBEEMAN_H__
#define __DRBEEMAN_H__

#include <lpmd/vector.h>
#include <lpmd/twostepintegrator.h>
#include <lpmd/stepper.h>
#include <lpmd/plugin.h>

#include <vector>

using namespace lpmd;

class DRBeeman: public lpmd::TwoStepIntegrator, public lpmd::Plugin
{
 public:
   //Metodos Generales
   DRBeeman(std::string args);
   ~DRBeeman();
   void ShowHelp() const;

   //Metodos Propios del Modulo DRBeeman
   void Initialize(Simulation & sim, Potential & p);
   void AdvancePosition(Simulation & sim, long i);
   void AdvanceVelocity(Simulation & sim, long i);

 private:
      std::vector<lpmd::Vector> auxlist;
      double distance;
};

#endif


