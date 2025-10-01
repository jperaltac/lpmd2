//
//
//

#include "undopbc.h"

#include <lpmd/simulation.h>

#include <iostream>
#include <iomanip>

using namespace lpmd;

UndoPBCModifier::UndoPBCModifier(std::string args): Plugin("undopbc", "2.0"), oldpositions(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 first_apply = true;
}

UndoPBCModifier::~UndoPBCModifier() { delete [] oldpositions; }

void UndoPBCModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = undopbc                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to remove periodic boundary conditions in a set      \n";
 std::cout << "      of multiple configurations.                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Load the plugin  :                                                           \n";
 std::cout << " use undopbc                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply undopbc start=0 each=1 end=100                                        \n\n";
 std::cout << "      The plugin is used to turn off the periodic boundary conditions for the  \n";
 std::cout << "      first 100 time steps.                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void UndoPBCModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicCell & cell = sim.Cell();
 if (first_apply)
 {
  oldpositions = new Vector[atoms.Size()];
  for (long int i=0;i<atoms.Size();++i) oldpositions[i] = atoms[i].Position(); 
  first_apply = false;
 }
 else
 {
  for (long int i=0;i<atoms.Size();++i)
  {
   atoms[i].Position() = oldpositions[i] + cell.Displacement(oldpositions[i], atoms[i].Position());
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new UndoPBCModifier(args); }
void destroy(Plugin * m) { delete m; }

