//
//
//

#include "propertycolor.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>
#include <lpmd/error.h>
#include <lpmd/session.h>

#include <iostream>
#include <fstream>

using namespace lpmd;

PropertyColorModifier::PropertyColorModifier(std::string args): Plugin("propertycolor", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("property","temperature");
 DefineKeyword("min","0");
 DefineKeyword("max","1");
 DefineKeyword("cutoff","10");
 DefineKeyword("extfile","");
 DefineKeyword("extcolumn", "2");
 DefineKeyword("extheader", "1");
 DefineKeyword("filterby", "none");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 property = params["property"];
 vmin = double(params["min"]);
 vmax = double(params["max"]);
 cutoff = double(params["cutoff"]);
 if (property == "external")
 {
  extfile = new std::ifstream(params["extfile"].c_str());
  column = int(params["extcolumn"]);
  extheader = int(params["extheader"]);
 }
 else extfile = NULL;
}

PropertyColorModifier::~PropertyColorModifier() 
{ 
 if (extfile != NULL) delete extfile;
}

void PropertyColorModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = propertycolor                                            \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to color the atoms by some specific property.        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      property      : Sets the property to be evaluated for coloring           \n";
 std::cout << "                      (exernal / temperature / velocity / acceleration /       \n";
 std::cout << "                      neighbors / random).                                     \n";
 std::cout << "      min           : Minimum value of the property (corresponding to blue).   \n";
 std::cout << "      max           : Maximum value of the property (corresponding to red).    \n";
 std::cout << "      cutoff        : Sets the cutoff radius for the neighbors counting in the \n";
 std::cout << "                      case of choosing that property.                          \n";
 std::cout << "      extfile       : Specify the file where the property is stored in case of \n";
 std::cout << "                      choosing an external property.                           \n";
 std::cout << "      extcolumn     : Choose the column of extfile that has the data to check  \n";
 std::cout << "                      for filter in the case of choosing an external property. \n";
 std::cout << "      extheader     : HELP NOT AVAILABLE YET                                   \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use propertycolor                                                             \n";
 std::cout << "     property temperature                                                      \n";
 std::cout << "     min 0                                                                     \n";
 std::cout << "     max 300                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply propertycolor start=0 each=1 end=1000                                 \n\n";
 std::cout << "      The plugin is used to color the atoms by temperature during the first    \n";
 std::cout << "      1000 steps. Red atoms will represent a temperature of 300 K or higher,   \n";
 std::cout << "      while lower temperaures are represented by a color gradient that goes from\n";
 std::cout << "      red to blue (0 K).                                                       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PropertyColorModifier::Apply(Simulation & sim)
{
 const double kin2ev = double(GlobalSession["kin2ev"]);
 const double kboltzmann = double(GlobalSession["kboltzmann"]);
 BasicParticleSet & atoms = sim.Atoms();
 DebugStream() << "-> Applying color according to " << property << '\n';
 // Manage header line(s) for external mode
 std::string line;
 if (property == "external")
    for (int k=0;k<extheader;k++) getline(*extfile, line);
 //
 for (long int i=0;i<atoms.Size();++i)
 {
  double v = 0.0;
  // 
  // Por ahora seleccionamos aqui de donde sale el escalar para asignar el color
  // Esto hasta tener un sistema mas dinamico donde no esten en el codigo todas las posibilidades
  //
  if (property == "temperature") v = (1.0/3.0)*(kin2ev/kboltzmann)*atoms[i].Mass()*atoms[i].Velocity().SquareModule();
  else if (property == "velocity") v = atoms[i].Velocity().SquareModule();
  else if (property == "acceleration") v = atoms[i].Acceleration().SquareModule();
  else if (property == "neighbors")
  {
   lpmd::NeighborList & nlist = sim.Neighbors(i,true,cutoff);
   for(long int k=0;k<nlist.Size();++k) v++;
  }
  else if (property == "random") v = vmin + (vmax-vmin)*drand48();
  else if (property == "external")
  {
   getline(*extfile, line);
   Array<std::string> lspl = StringSplit(line);
   v = strtod(lspl[column-1].c_str(), 0);
  }
  else throw PluginError("propertycolor", "Cannot color atoms by property \""+property+"\"");
  double vnorm = (v-vmin)/(vmax-vmin);
  if (vnorm < 0.0) vnorm = 0.0;
  if (vnorm > 1.0) vnorm = 1.0;
  const Color c = lpmd::ColorFromScalar(vnorm);
  const BasicAtom & at = atoms[i]; 
  ColorHandler::ColorOfAtom(at) = c;
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new PropertyColorModifier(args); }
void destroy(Plugin * m) { delete m; }

