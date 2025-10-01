//
//
//

#include "vacf.h"
#include <lpmd/properties.h>

using namespace lpmd;

Vacf::Vacf(std::string args): Plugin("vacf", "2.1")
{
 ParamList & param = (*this);
 DefineKeyword("dt","1");
 //
 ProcessArguments(args);
 dt = param["dt"];
}

Vacf::~Vacf() { }

void Vacf::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = vacf                                                     \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to evaluate velocity autocorrelation function for    \n";
 std::cout << "      every configuration as function of time (see also 'rvcorr' plugin).      \n";
 std::cout << "      This is a temporal property.                                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      dt            : Set the time step in femto-seconds in case that the      \n";
 std::cout << "                      velocity are not specified in the input file.            \n";
 std::cout << "      output        : Set the output file-name.                                \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use vacf                                                                      \n";
 std::cout << "     output vacf.dat                                                           \n";
 std::cout << "     dt 10.0                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " property vacf start=1 each=10 end=100                                       \n\n";
 std::cout << "      The plugin is used to calculate the velocity autocorrelation function    \n";
 std::cout << "      of the atomic configuration every 10 steps of the first 100 steps. The   \n";
 std::cout << "      data is written in the file vacf.dat                                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}


void Vacf::Evaluate(lpmd::ConfigurationSet & hist, Potential & pot)
{
 vacf(hist,pot,dt,CurrentValue());
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Vacf(args); }
void destroy(Plugin * m) { delete m; }
