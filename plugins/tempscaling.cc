//
//
//

#include "tempscaling.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

TempScalingModifier::TempScalingModifier(std::string args): Plugin("tempscaling", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("from", "300.0");
 DefineKeyword("to", "300.0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 fromtemp = double(params["from"]);
 totemp = double(params["to"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

TempScalingModifier::~TempScalingModifier() { }

void TempScalingModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = tempscaling                                              \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to scale the temperature of the system, using a      \n";
 std::cout << "      velocity rescaling on the atoms of the cell.                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      from          : Initial temperature.                                     \n";
 std::cout << "      to            : Final required temperature.                              \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use tempscaling                                                               \n";
 std::cout << "     from 84.0                                                                 \n";
 std::cout << "     to   10.0                                                                 \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply tempscaling start=0 each=10 end=100                                   \n\n";
 std::cout << "      The plugin is used to reduce the temperature of the system from 84 to 10 K\n";
 std::cout << "      in 100 steps linearly.                                                   \n";
 std::cout << "      To thermalize a system at a certain temperature, the temperature scaling \n";
 std::cout << "      must be applied for a reasonable period of time, each certain steps to   \n";
 std::cout << "      let the system relax.                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void TempScalingModifier::Apply(Simulation & sim)
{
 double set_temp = ValueAtStep(sim.CurrentStep(), fromtemp, totemp);
 DebugStream() << "-> Rescaling temperature to T = " << set_temp << '\n';  
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 sim.SetTemperature(set_temp,(atoms.HaveAny(Tag("fixedvel"))||atoms.HaveAny(Tag("fixedpos"))));
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TempScalingModifier(args); }
void destroy(Plugin * m) { delete m; }

