//
//
//

#ifndef __BERENDSEN_SM_H__
#define __BERENDSEN_SM_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/plugin.h>

class BerendsenModifier: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
   //Metodos Generales
   BerendsenModifier(std::string args);
   ~BerendsenModifier();
   void ShowHelp() const;

   //Metodos Propios del Modulo Berendsen
   void Apply(lpmd::Simulation & sim);

  private:
    long stop_thermostat, old_step;
    double fromtemp, totemp, tau;
};

#endif



