//
//
//

#include "quenchedmd.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

QuenchedMDModifier::QuenchedMDModifier(std::string args): Plugin("quenchedmd", "2.0")
{
 ParamList & params=(*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");

 ProcessArguments(args);
 start = int((*this)["start"]);
 end = int((*this)["end"]);
 each = int((*this)["each"]);
}

QuenchedMDModifier::~QuenchedMDModifier() { }

void QuenchedMDModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = quenchedmd                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin performs a quenched molecular dynamics process on the sample.\n";
 std::cout << "      It consists in evaluating, for every atom,  the dot product between its  \n";
 std::cout << "      acceleration (a) and its velocity (v). If a.v < 0, v is set to zero.     \n";
 std::cout << "      This procedure is frequently used to find the structure of minimum       \n";
 std::cout << "      energy of the system.                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use quenchedmd                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply quenchedmd start=0 each=1 end=1000                                    \n\n";
 std::cout << "      The plugin is used to perform a quenched molecular dynamics in 1000 steps.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void QuenchedMDModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 //
 for (long int i=0;i<atoms.Size();++i)
 {
  if (Dot(atoms[i].Acceleration(), atoms[i].Velocity()) < 0.0) atoms[i].Velocity() = Vector(0.0, 0.0, 0.0);
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new QuenchedMDModifier(args); }
void destroy(Plugin * m) { delete m; }

