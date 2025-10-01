//
//
//

#include "velocityverlet.h"

#include <lpmd/simulation.h>

#include <iostream>

using namespace lpmd;

VelocityVerlet::VelocityVerlet(std::string args): Plugin("velocityverlet", "2.1")
{
 DefineKeyword("dt", "1.0");
 DefineKeyword("start", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 dt = double((*this)["dt"]);
 start = int((*this)["start"]);
}

VelocityVerlet::~VelocityVerlet() { }

void VelocityVerlet::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = velocityverlet                                           \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the movement equations using the         \n";
 std::cout << "      velocity-verlet method.                                                  \n";
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
 std::cout << " use velocityverlet                                                            \n";
 std::cout << "     dt 10.0                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator velocityverlet start=1000                                        \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void VelocityVerlet::Initialize(Simulation & sim, Potential & p) { assert(&p != 0); UseOldConfig(sim); }

void VelocityVerlet::AdvancePosition(Simulation & sim, long i)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms(); 
 lpmd::BasicCell & cell = sim.Cell();
 lpmd::BasicParticleSet & oldatoms = OldConfig().Atoms(); 
 Vector newpos = atoms[i].Position() + atoms[i].Velocity()*dt + 0.5*atoms[i].Acceleration()*dt*dt;
 oldatoms[i].Acceleration() = atoms[i].Acceleration();
 atoms[i].Position() = cell.FittedInside(newpos);
}

void VelocityVerlet::AdvanceVelocity(Simulation & sim, long i)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicParticleSet & oldatoms = OldConfig().Atoms();
 atoms[i].Velocity() = atoms[i].Velocity() + 0.5*dt*(oldatoms[i].Acceleration() + atoms[i].Acceleration());
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new VelocityVerlet(args); }
void destroy(Plugin * m) { delete m; }



