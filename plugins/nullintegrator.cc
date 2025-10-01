//
//
//

#include "nullintegrator.h"

#include <lpmd/potential.h>
#include <lpmd/simulation.h>

#include <iostream>

using namespace lpmd;

NullIntegrator::NullIntegrator(std::string args): Plugin("nullintegrator", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
}

NullIntegrator::~NullIntegrator() { }

void NullIntegrator::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nullintegrator                                           \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements a 'phantom' integrator. The positions and         \n";
 std::cout << "      velocities remain unchanged.                                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nullintegrator                                                            \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator nullintegrator start=1000                                        \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void NullIntegrator::Advance(Simulation & sim, Potential & p) { p.UpdateForces(sim); }

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new NullIntegrator(args); }
void destroy(Plugin * m) { delete m; }


