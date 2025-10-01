//
//
//

#include "temperature.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

TemperatureModifier::TemperatureModifier(std::string args): Plugin("temperature", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("t", "300.0");
 DefineKeyword("filterby", "none");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 temp = double(params["t"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

TemperatureModifier::~TemperatureModifier() { }

void TemperatureModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = temperature                                              \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to set the temperature of the system using a uniform \n";
 std::cout << "      distribution of velocities compatible with the temperature required.     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      t             : Temperature goal of the system (in kelvin).              \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " prepare temperature t=600.0                                                 \n\n";
 std::cout << "      The plugin is used to set the initial temperature of the sample to 600K. \n";
 std::cout << "      To thermalize a system at a certain temperature, the temperature         \n";
 std::cout << "      must be applied for a reasonable period of time, each certain steps to   \n";
 std::cout << "      let the system relax,  using the 'apply' statement.                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void TemperatureModifier::Apply(Simulation & sim)
{
 DebugStream() << "-> Rescaling temperature to T = " << temp << '\n';  
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 sim.SetTemperature(temp,(atoms.HaveAny(Tag("fixedvel"))||atoms.HaveAny(Tag("fixedpos"))));
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TemperatureModifier(args); }
void destroy(Plugin * m) { delete m; }


