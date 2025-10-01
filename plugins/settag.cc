//
//
//

#include "settag.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

SetTagModifier::SetTagModifier(std::string args): Plugin("settag", "2.1")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("tag");
 DefineKeyword("value", "true");
 DefineKeyword("filterby", "none");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 tag = params["tag"];
 value = params["value"];
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

SetTagModifier::~SetTagModifier() { }

void SetTagModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = settag                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to assign a specific tag to a set of atoms.          \n";
 std::cout << "      The specified tag can be any word. However, a couple of special words are\n";
 std::cout << "      available in the code are fixedpos and fixedvel, which not only assign   \n";
 std::cout << "      a tag to the atoms, they modify them:                                    \n";
 std::cout << "             fixedpos      : If an atom has this tag, then the atom will have a\n";
 std::cout << "                             fixed position during the simulation.             \n";
 std::cout << "             fixedvel      : If an atom has this tag, then the atom will have a\n";
 std::cout << "                             the same velocity for the rest of the simulation. \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      value         : Sets the state of the tag (true / false). This allows the\n";
 std::cout << "                      user to disable the tag at any time.                     \n";  
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use settag as floor                                                           \n";
 std::cout << "     tag floor_atoms                                                           \n";
 std::cout << "     value true                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n"; 
 std::cout << " apply floor over box x=0-37 y=0-37 z=27.0-37.0                              \n\n";
 std::cout << " filter sphere radius=4.5 center=<12.0,12.0,12.0> except=floor_atoms           \n";
 std::cout << "      The plugin is used to set the tag 'floor_atoms' (apply line) to all atoms\n";
 std::cout << "      in the box specified by the ranges in x, y and z. Once the atoms have    \n";
 std::cout << "      this tag, they are filtered (filter line), which means that all atoms    \n";
 std::cout << "      that don't belong to the sphere of radius 4.5, with center in (12,12,12) \n";
 std::cout << "      are eliminated, except those ones having the 'floor_atoms' tag.          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void SetTagModifier::Apply(Simulation & sim)
{
 BasicParticleSet & atoms = sim.Atoms();
 if (tag == "") throw PluginError("settag", "The tag to be set was not specified.");
 DebugStream() << "-> Applying tag " << tag << " with value " << value << " to " << atoms.Size() << " atoms\n";  
 BasicParticleSet & orig_atoms = sim.OriginalAtoms();
 for (long int i=0;i<atoms.Size();++i)
 {
  const BasicAtom & at = atoms[i];
  orig_atoms.SetTag(at, Tag(tag), value);
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new SetTagModifier(args); }
void destroy(Plugin * m) { delete m; }

