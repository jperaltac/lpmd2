//
//
//

#include "cellscaling.h"

#include <lpmd/simulation.h>

#include <iostream>
#include <iomanip>

using namespace lpmd;

CellScalingModifier::CellScalingModifier(std::string args): Plugin("cellscaling", "2.0")
{
 ParamList & params = (*this);
 //
 axis = -1;
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("percent", "0.0");
 DefineKeyword("axis", "all");
 AssignParameter("constant", "true");
 AssignParameter("first", "true");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 percent = double(params["percent"]);
 constant = bool(params["constant"]);
 first = bool(params["first"]);
 std::string ax = params["axis"];
 if ((ax == "all") || (ax == "ALL")) axis = -1;
 if ((ax == "x") || (ax == "X")) axis = 0;
 if ((ax == "y") || (ax == "Y")) axis = 1;
 if ((ax == "z") || (ax == "Z")) axis = 2;
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
}

CellScalingModifier::~CellScalingModifier() { }

void CellScalingModifier::Show(std::ostream & os) const
{
 if (axis == -1) os << "   " << std::setw(10) << "axis" << " = " << "all\n";
 if (axis == 0) os << "   " << std::setw(10) << "axis" << " = " << "X\n";
 if (axis == 1) os << "   " << std::setw(10) << "axis" << " = " << "Y\n";
 if (axis == 2) os << "   " << std::setw(10) << "axis" << " = " << "Z\n";
 Module::Show(os);
}

void CellScalingModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = cellscaling                                              \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to scale the size of the cell. It can be used to     \n";
 std::cout << "      increase/decrease the size of just one cell axis, or all of them         \n";
 std::cout << "      (hydrostatic compression/expansion).                                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      percent       : Sets the percentage in which the cell axis (or axes) will\n";
 std::cout << "                      be scaled. It can be positive (expansion) or negative    \n";
 std::cout << "                      (compression).                                           \n";
 std::cout << "      axis          : Sets what axis (or axes) that will be scaled (X/Y/Z/ALL).\n";
 std::cout << "      constant      : Determines if the percentual scaling is cumulative or    \n";
 std::cout << "                      not (true / false):                                      \n";
 std::cout << "                      true / false <-> non cumulative / cumulative.            \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use cellscaling                                                               \n";
 std::cout << "     percent 20                                                                \n";
 std::cout << "     axis X                                                                    \n";
 std::cout << "     constant true                                                             \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply cellscaling start=0 each=300 end=1000                                 \n\n";
 std::cout << "      The plugin is used to scale the X axis of the simulation cell in 20% of  \n";
 std::cout << "      the original size of the cell, each 300 steps, ending in the step 1000.  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void CellScalingModifier::Apply(Simulation & sim)
{
 lpmd::BasicCell & cell = sim.Cell();
 if (constant == false)
 {
  if (axis == -1)
  {
   DebugStream() << "-> Rescaling cell by " << percent << "%\n";
   //sc.RescalePercent(percent);
   for (int i=0;i<3;++i) cell[i] = cell[i] + cell[i] * percent/100.0e0;
  }
  else
  {
   DebugStream() << "-> Rescaling axis " << axis << " in a " << percent << "%\n";
   //sc.RescalePercent(percent, axis);
   cell[axis] = cell[axis] + cell[axis]*percent/100.0e0;
  }
 }
 else if (constant == true)
 {
  if (first == true)
  {
   for(int i=0;i<3;++i) s[i] = cell[i]*percent/100;
   first=false;
  }
  if (axis == -1)
  {
   DebugStream() << "-> Rescaling constant by " << percent << "%\n";
   //sc.RescaleVector(s[0],s[1],s[2]);
   for (int i=0;i<3;++i) cell[i] = cell[i] + s[i];
  }
  else
  {
   DebugStream() << "-> Rescaling axis " << axis <<" constant by " << percent << "%\n";
   //sc.RescaleVector(s[axis],axis);
   cell[axis] = cell[axis] + s[axis];
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new CellScalingModifier(args); }
void destroy(Plugin * m) { delete m; }


