//
//
//

#include "beeman.h"

#include <lpmd/simulation.h>
#include <lpmd/potential.h>

using namespace lpmd;

Beeman::Beeman(std::string args): Plugin("beeman", "2.0")
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

Beeman::~Beeman() { }

void Beeman::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = beeman                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the movement equations using the beeman  \n";
 std::cout << "      method.                                                                  \n";
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
 std::cout << " use beeman                                                                    \n";
 std::cout << "     dt 10.0                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator beeman start=1000                                                \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Beeman::Initialize(Simulation & sim, Potential & p)
{
 BasicParticleSet & atoms = sim.Atoms();
 for (long int i=0;i<atoms.Size();++i) auxlist.push_back(Vector());
 UseOldConfig(sim);
 p.UpdateForces(OldConfig());
}

void Beeman::AdvancePosition(Simulation & sim, long i)
{
 BasicParticleSet & atoms = sim.Atoms();
 BasicParticleSet & oldatoms = OldConfig().Atoms();
 BasicCell & cell = sim.Cell();
 const Atom & now = atoms[i];
 const Atom & old = oldatoms[i];
 auxlist[i] = old.Acceleration();
 Vector newpos = now.Position() + now.Velocity()*dt + (2.0/3.0)*now.Acceleration()*dt*dt - (1.0/6.0)*old.Acceleration()*dt*dt;
 oldatoms[i].Acceleration() = now.Acceleration();
 atoms[i].Position() = cell.FittedInside(newpos);
}

void Beeman::AdvanceVelocity(Simulation & sim, long i)
{
 BasicParticleSet & atoms = sim.Atoms();
 BasicParticleSet & oldatoms = OldConfig().Atoms();
 const Atom & now = atoms[i];
 const Atom & old = oldatoms[i];
 atoms[i].Velocity() = now.Velocity()+(1.0/3.0)*now.Acceleration()*dt+(5.0/6.0)*old.Acceleration()*dt-(1.0/6.0)*auxlist[i]*dt;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Beeman(args); }
void destroy(Plugin * m) { delete m; }

