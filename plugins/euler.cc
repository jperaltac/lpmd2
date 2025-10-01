//
//
//

#include "euler.h"
#include <lpmd/simulation.h>
#include <lpmd/atom.h>

#include <iostream>

#ifdef __ICC
#pragma warning (disable:869)
#endif

using namespace lpmd;

Euler::Euler(std::string args): Plugin("euler", "2.0")
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

Euler::~Euler() { }

void Euler::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = euler                                                    \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the equations of movement using the Euler\n";
 std::cout << "      method. This method is not recommended for MD simulation, but it is      \n";
 std::cout << "      available for comparisons (instructive for programmers).                 \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      dt            : Set the time-step for the integrator (in femto-seconds). \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use euler                                                                     \n";
 std::cout << "     dt 10.0                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator euler start=1000                                                 \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Euler::Advance(Simulation & sim, long i)
{
 BasicParticleSet & atoms = sim.Atoms();
 BasicCell & cell = sim.Cell();
 const Atom & now = atoms[i];
 atoms[i].Position() = cell.FittedInside(now.Position() + now.Velocity()*dt);
 atoms[i].Velocity() += now.Acceleration()*dt;
}

#ifdef __ICC
#pragma warning (default:869)
#endif

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Euler(args); }
void destroy(Plugin * m) { delete m; }

