//
//
//

#include "pinatom.h"

#include <lpmd/simulation.h>

#include <iostream>
#include <iomanip>

using namespace lpmd;

PinAtomModifier::PinAtomModifier(std::string args): Plugin("pinatom", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("index", "0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 index = int(params["index"]);
 initial_pos = NULL;
}

PinAtomModifier::~PinAtomModifier() { delete initial_pos; }

void PinAtomModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = pinatom                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to fix the position of a particular atom.            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      index         : Sets the index of the atom, in the atoms list, that is   \n";
 std::cout << "                      wanted to be considered. The index of an atom corresponds\n";
 std::cout << "                      to the place in which the atom appears in the input file \n";
 std::cout << "                      (the first lines will corespond to the atoms 1,2,3,4,etc.).\n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use pinatom                                                                   \n";
 std::cout << "     index 42                                                                  \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply pinatom start=100 each=1 end=1000                                     \n\n";
 std::cout << "      The plugin is used to keep fixed the atom number 42 from the step number \n";
 std::cout << "      100 to the step number 1000.                                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PinAtomModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 if (initial_pos == NULL) 
 {
  initial_pos = new Vector(atoms[index].Position());
 }
 Vector correction = (*initial_pos) - atoms[index].Position();
 for (long i=0;i<atoms.Size();++i)
 {
  atoms[i].Position() = atoms[i].Position() + correction;  
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new PinAtomModifier(args); }
void destroy(Plugin * m) { delete m; }

