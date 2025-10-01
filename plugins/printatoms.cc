//
//
//

#include "printatoms.h"

#include <lpmd/simulation.h>
#include <lpmd/colorhandler.h>

#include <iostream>

using namespace lpmd;

PrintAtomsVisualizer::PrintAtomsVisualizer(std::string args): Plugin("printatoms", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("print", "index,x,y,z");
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 tags = StringSplit(params["print"], ',');
}

PrintAtomsVisualizer::~PrintAtomsVisualizer() { }

void PrintAtomsVisualizer::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = printatoms                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to show the basic characteristics of an atom: position,\n";
 std::cout << "      velocity, acceleration and color. The output is written in the standard  \n";
 std::cout << "      output, so you can redirect it to a file if you want to save it.         \n";
 std::cout << "      The tags to do this are: x,y,z,vx,vy,vz,ax,ay,az,r,g,b. The first 3 are  \n";
 std::cout << "      the coordinates of the position, the second 3, the velocity ones, and    \n";
 std::cout << "      the third threesome are the coordinates of the acceleration vector. The  \n";
 std::cout << "      last 3 corresponds to the color in RGB (red-green-blue) format.          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      print         : Sets the tags that are going to be printed:              \n";
 std::cout << "                      (x,y,z,vx,vy,vz,ax,ay,az,r,g,b).                         \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use printatoms                                                                \n";
 std::cout << "     print x,y,z,vx,vy,vz                                                      \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " visualize printatoms start=1 end=1000 each=50 over index index=10-10        \n\n";
 std::cout << "      The plugin is used to monitor the position and velocity of the atom      \n";
 std::cout << "      10 in the standard output during the first 1000 steps, each 50.          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PrintAtomsVisualizer::Apply(const Simulation & sim) 
{ 
 const BasicParticleSet & atoms = sim.Atoms();
 for (long int i=0;i<atoms.Size();++i) 
 {  
  const Vector & pos = atoms[i].Position();
  const Vector & vel = atoms[i].Velocity();
  const Vector & acc = atoms[i].Acceleration();
  std::cout << "-> ";
  for (int j=0;j<tags.Size();++j)
  {
   const std::string & tag = tags[j];
   std::cout << tag << "=";
   if (tag == "x") std::cout << pos[0];
   else if (tag == "y") std::cout << pos[1];
   else if (tag == "z") std::cout << pos[2];
   else if (tag == "vx") std::cout << vel[0];
   else if (tag == "vy") std::cout << vel[1];
   else if (tag == "vz") std::cout << vel[2];
   else if (tag == "ax") std::cout << acc[0];
   else if (tag == "ay") std::cout << acc[1];
   else if (tag == "az") std::cout << acc[2];
   else if ((tag == "r") && ColorHandler::HaveColor(atoms[i])) std::cout << ColorHandler::ColorOfAtom(atoms[i])[0];
   else if ((tag == "g") && ColorHandler::HaveColor(atoms[i])) std::cout << ColorHandler::ColorOfAtom(atoms[i])[1];
   else if ((tag == "b") && ColorHandler::HaveColor(atoms[i])) std::cout << ColorHandler::ColorOfAtom(atoms[i])[2];
   else if (atoms.Have(atoms[i], Tag(tag))) std::cout << atoms.GetTag(atoms[i], Tag(tag));
   std::cout << " ";
  }
  std::cout << '\n';
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new PrintAtomsVisualizer(args); }
void destroy(Plugin * m) { delete m; }

