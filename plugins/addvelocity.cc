//
//
//

#include "addvelocity.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

AddVelocityModifier::AddVelocityModifier(std::string args): Plugin("addvelocity", "1.0")
{ 
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("velocity", "<0.0,0.0,0.0>");
 DefineKeyword("debug","none");
 ProcessArguments(args); 
 start = int((*this)["start"]);
 end = int((*this)["end"]);
 each = int((*this)["each"]);
 velocity = Vector((*this)["velocity"].c_str());
}

AddVelocityModifier::~AddVelocityModifier() { }

void AddVelocityModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = addvelocity                                              \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to add a specific velocity to a set of atoms that    \n";
 std::cout << "      have the tag 'addvelocity' with value=true (see 'tag' plugin).           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      velocity      : Velocity vector that's going to be added to each atom.   \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use addvelocity                                                               \n";
 std::cout << "     velocity <0.002,0.001,0.005>                                              \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply addvelocity start=0 each=10 end=100                                   \n\n";
 std::cout << "      The plugin is used to apply an extra-velocity to all the atoms, between  \n";
 std::cout << "      steps 0 and 100, each 10 steps.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void AddVelocityModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 DebugStream() << "-> Applying addvelocity with velocity ";
 FormattedWrite(DebugStream(), velocity);
 DebugStream() << '\n';
 if (!atoms.HaveAny(Tag("addvelocity"))) return;
 for (int i=0;i<atoms.Size();++i)
 {
  Atom at = atoms[i];
  if (atoms.Have(atoms[i], Tag("addvelocity")) && (bool(Parameter(atoms.GetTag(atoms[i], Tag("addvelocity")))))) 
     atoms[i].Velocity() += velocity;
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AddVelocityModifier(args); }
void destroy(Plugin * m) { delete m; }

