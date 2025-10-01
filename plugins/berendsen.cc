//
//
//

#include "berendsen.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>
#include <lpmd/integrator.h>
#include <lpmd/properties.h>

#include <iostream>

using namespace lpmd;

BerendsenModifier::BerendsenModifier(std::string args): Plugin("berendsen", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("from", "300.0");
 DefineKeyword("to", "300.0");
 DefineKeyword("tau", "400.0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 fromtemp = double(params["from"]);
 totemp = double(params["to"]);
 tau = double(params["tau"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 // stop_thermostat no es un parametro sino un flag interno
 stop_thermostat = -1;
}

BerendsenModifier::~BerendsenModifier() { }

void BerendsenModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = berendsen                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to rescale the system temperature using the berendsen\n";
 std::cout << "      thermostat. This is one of the most used methods in velocity rescaling   \n";
 std::cout << "      process because the change in the system temperature is less sharp.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      from          : Initial temperature for the scaling process.             \n";
 std::cout << "      to            : Final temperature for the scaling process.               \n";
 std::cout << "      tau           : Thermostat interval.                                     \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use berendsen                                                                 \n";
 std::cout << "     tau  400.0                                                                \n";
 std::cout << "     from 84.0                                                                 \n";
 std::cout << "     to   10.0                                                                 \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply berendsen start=0 each=10 end=100                                     \n\n";
 std::cout << "      With this, we will apply the plugin between the 0 and 100 timesteps      \n";
 std::cout << "      each 10 timesteps.                                                       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void BerendsenModifier::Apply(lpmd::Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::Integrator & integrator = sim.Integrator();
 double timestep = integrator.Timestep();
 double set_temp = ValueAtStep(sim.CurrentStep(), fromtemp, totemp);
 if (stop_thermostat == -1)
 {
  stop_thermostat = long(double(sim.CurrentStep()) + tau/timestep);
  DebugStream() << "-> Berendsen thermostat activated (stops in step " << stop_thermostat << "), rescaling temperature to T = " << set_temp << '\n';
  old_step = each;
  each = 1;
 }
 else
 {
  if (sim.CurrentStep() <= stop_thermostat) 
  {
   Vector vel(0,0,0);
   double xi, ti=lpmd::Temperature(atoms);
   for (int i=0;i<atoms.Size();++i)
   {
    vel = atoms[i].Velocity();
    xi = sqrt(1.0 + (double(timestep)/tau)*(set_temp/ti - 1.0));
    vel = vel*xi;
    atoms[i].Velocity() = vel;
   }
  }
  else
  {
   stop_thermostat = -1;
   DebugStream() << "-> Berendsen thermostat stopped." << '\n';
   each = old_step;
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new BerendsenModifier(args); }
void destroy(Plugin * m) { delete m; }


