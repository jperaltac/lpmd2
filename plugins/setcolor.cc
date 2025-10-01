//
//
//

#include "setcolor.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

SetColorModifier::SetColorModifier(std::string args): Plugin("setcolor", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("color");
 DefineKeyword("filterby", "none");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 color = Color(params["color"].c_str());
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

SetColorModifier::~SetColorModifier() { }

void SetColorModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = setcolor                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to give a specific color of a certain group of atoms \n";
 std::cout << "      (see also 'propertycolor' plugin).                                       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      color         : Sets the color, in RGB format, to be given to the atoms. \n";
 std::cout << "                      The color is a vector written in the form <R,G,B>.       \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use setcolor                                                                  \n";
 std::cout << "     color <1.0,0.0,1.0>                                                       \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply setcolor start=0 each=1 end=1 over index index=0-100                  \n\n";
 std::cout << "      The plugin is used to fix a magenta color (100\% red, 0\% green, 100\%   \n";
 std::cout << "      blue) to the first hundred atoms in the atoms list of the simulation cell.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void SetColorModifier::Apply(Simulation & sim)
{
 BasicParticleSet & atoms = sim.Atoms();
 DebugStream() << "-> Applying color ";
 FormattedWrite(DebugStream(), color);
 DebugStream() << " to " << atoms.Size() << " atoms\n";  
 for (long int i=0;i<atoms.Size();++i) ColorHandler::ColorOfAtom(atoms[i]) = color;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new SetColorModifier(args); }
void destroy(Plugin * m) { delete m; }

