//
//
//

#include "gdr.h"

#include <lpmd/simulation.h>
#include <lpmd/properties.h>

#include <sstream>

using namespace lpmd;

Gdr::Gdr(std::string args): Plugin("gdr", "2.0")
{
 ParamList & params=(*this);
 //
 DefineKeyword("rcut");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 DefineKeyword("bins", "200");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 rcut = params["rcut"];
 nb = int(params["bins"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

Gdr::~Gdr() { }

void Gdr::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = gdr                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to calculate the radial distribution function (also  \n";
 std::cout << "      called pair correlation funcion): g(r).                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      bins          : Sets the number of subdivisions of the range [0,rcut].   \n";
 std::cout << "      rcut          : Set the cutoff radius for the function.                  \n";
 std::cout << "      output        : Output file.                                             \n";
 std::cout << "      average       : Sets if the the property must be averaged over all       \n";
 std::cout << "                      configurations (true / false)                            \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use gdr                                                                       \n";
 std::cout << "     bins 200                                                                  \n";
 std::cout << "     output filegdr.dat                                                        \n";
 std::cout << "     rcut 15.0                                                                 \n";
 std::cout << "     average true                                                              \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " property gdr start=1 each=10 end=100                                        \n\n";
 std::cout << "      The plugin is used to calculate the g(r) function of the atomic          \n";
 std::cout << "      configuration every 10 steps of the first 100 steps. The data is written \n";
 std::cout << "      in the file filegdr.dat                                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Gdr::Evaluate(Configuration & con, Potential & pot)
{
 // fabs(rcut) < 1e-05 used to avoid comparing doubles
 if (nb == 0 || fabs(rcut) < 1e-05) throw PluginError("gdr", "Error in calculation: Cutoff or bins have wrong value.");
 gdr(con,pot,nb,rcut,CurrentValue());
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Gdr(args); }
void destroy(Plugin * m) { delete m; }

