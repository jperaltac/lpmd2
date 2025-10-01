//
//
//

#include "shear.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>
#include <lpmd/plugin.h>

#include <iostream>

using namespace lpmd;

ShearModifier::ShearModifier(std::string args): Plugin("shear", "2.1")
{
 ParamList & param = (*this);
 DefineKeyword("axis", "X");
 DefineKeyword("normal", "Y");
 DefineKeyword("strain", "0.01");
 // 
 ProcessArguments(args);
 if ((param["axis"] == "x") || (param["axis"] == "X")) shear_axis = 0;
 else if ((param["axis"] == "y") || (param["axis"] == "Y")) shear_axis = 1;
 else if ((param["axis"] == "z") || (param["axis"] == "Z")) shear_axis = 2;
 else throw PluginError("shear", "Invalid shear axis");
 if ((param["normal"] == "x") || (param["normal"] == "X")) perp_axis = 0;
 else if ((param["normal"] == "y") || (param["normal"] == "Y")) perp_axis = 1;
 else if ((param["normal"] == "z") || (param["normal"] == "Z")) perp_axis = 2;
 else throw PluginError("shear", "Invalid normal axis");
 if (shear_axis == perp_axis) throw PluginError("shear", "Shear axis must be orthogonal to normal axis");
 strain = double(param["strain"]);
 start = int(param["start"]);
 end = int(param["end"]);
 each = int(param["each"]);
}

ShearModifier::~ShearModifier() { }

void ShearModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = shear                                                    \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to apply a shear over the simulation cell.           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      axis          : Sets the axis where the shear is applied.                \n";
 std::cout << "      normal        : Orthogonal axis to the shear.                            \n";
 std::cout << "      strain        : Maximum displacement to be applied \'strain*L\'(normal)  \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " prepare shear axis=X normal=Y strain=0.01                                   \n\n";
 std::cout << "      The plugin is used apply a shear in the X-axis direction, keeping the    \n";
 std::cout << "      Y-axis as vector normal to the plane of shearing.                        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void ShearModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicCell & cell = sim.Cell();
 // primero cambia la forma de la celda...
 Vector deformation(0.0, 0.0, 0.0);
 deformation[shear_axis] = strain*cell[perp_axis].Module(); 
 Vector newaxis = cell[perp_axis]+deformation;
 newaxis = newaxis*double(cell[perp_axis].Module())/double(newaxis.Module());
 cell[perp_axis] = newaxis;
 
 // luego desplaza los atomos
 Vector offset(0.0, 0.0, 0.0);
 for (long int i=0;i<atoms.Size();++i)
 {
  Vector pos = atoms[i].Position(); 
  offset[shear_axis] = pos[perp_axis]*strain; 
  atoms[i].Position() = pos+offset;
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ShearModifier(args); }
void destroy(Plugin * m) { delete m; }

