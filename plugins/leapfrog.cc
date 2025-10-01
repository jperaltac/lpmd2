//
//
//

#include "leapfrog.h"

#include <lpmd/simulation.h>
#include <lpmd/potential.h>

#include <iostream>

using namespace lpmd;

Leapfrog::Leapfrog(std::string args): Plugin("leapfrog", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("dt", "1.0");
 DefineKeyword("start", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 dt = params["dt"];
 start = int(params["start"]);
}

Leapfrog::~Leapfrog() { }

void Leapfrog::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = leapfrog                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the equations of movement using the leap \n";
 std::cout << "      frog method.                                                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      dt            : Sets the time-step in femto-seconds for the integration  \n";
 std::cout << "                      step.                                                    \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use leapfrog                                                                  \n";
 std::cout << "     dt 1.0                                                                    \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator leapfrog start=1000                                              \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Leapfrog::Initialize(Simulation & sim, Potential & p)
{
 UseOldConfig(sim);
 BasicParticleSet & oldatoms = OldConfig().Atoms();
 p.UpdateForces(OldConfig());
 // Necesitamos las velocidades al tiempo -0.5*dt, no -dt
 for (long int i=0;i<oldatoms.Size();++i)
 {
  const Atom & old = oldatoms[i];
  oldatoms[i].Velocity() += old.Acceleration()*0.5*dt;
 } 
}

void Leapfrog::Advance(Simulation & sim, long i)
{
 BasicParticleSet & atoms = sim.Atoms();
 BasicCell & cell = sim.Cell();
 BasicParticleSet & oldatoms = OldConfig().Atoms();
 const Atom & now = atoms[i];                      // was Atom now = sc[i];
 Vector vhalf = oldatoms[i].Velocity() + now.Acceleration()*dt;
 atoms[i].Position() = cell.FittedInside(atoms[i].Position() + vhalf*dt);
 atoms[i].Velocity() = 0.5*(vhalf+oldatoms[i].Velocity());
 oldatoms[i].Velocity() = vhalf;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Leapfrog(args); }
void destroy(Plugin * m) { delete m; }

