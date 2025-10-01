//
//
//

#include "displace.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

DisplaceModifier::DisplaceModifier(std::string args): Plugin("displace", "2.0")
{
 ParamList & params = (*this);
 DefineKeyword("x", "1.0");
 DefineKeyword("y", "0.0");
 DefineKeyword("z", "0.0");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 // 
 ProcessArguments(args);
 offset = Vector(double(params["x"]), double(params["y"]), double(params["z"]));
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

DisplaceModifier::~DisplaceModifier() { }

void DisplaceModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = displace                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to displace (move) a group of atoms. A constant      \n";
 std::cout << "      displacement vector is added to the position of each atom.               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      x             : X coordinate of the displacement vector.                 \n";
 std::cout << "      y             : Y coordinate of the displacement vector.                 \n";
 std::cout << "      z             : Z coordinate of the displacement vector.                 \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example                                                                       \n";
 std::cout << " #Loading plugin :                                                             \n";
 std::cout << " prepare displace x=1.0 y=0.0 z=0.0 start=0 each=1 end=1                     \n\n";
 std::cout << "      The plugin is used to displace all the atoms in <1,0,0> of its original  \n";
 std::cout << "      position.                                                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void DisplaceModifier::Apply(lpmd::Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicCell & cell = sim.Cell();
 for (long int i=0;i<atoms.Size();++i)
 {
  atoms[i].Position() = cell.FittedInside(atoms[i].Position()+offset);
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new DisplaceModifier(args); }
void destroy(Plugin * m) { delete m; }

