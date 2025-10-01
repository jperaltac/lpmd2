//
//
//

#include "morse.h"

#include <iostream>

using namespace lpmd;

Morse::Morse(std::string args): Plugin("morse", "2.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("depth");
 DefineKeyword("a");
 DefineKeyword("re");
 ProcessArguments(args);
 depth = params["depth"];
 a = params["a"];
 re = params["re"];
}

void Morse::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = morse                                                    \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the Morse potential for pairs interaction, with    \n";
 std::cout << "      the form                                                                 \n";
 std::cout << "            V(r) = depth*(1.0-exp(-a*(r - re))) * (1.0-exp(-a*(r - re)))       \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      depth         : Sets de value of depth for the potential (in eV).        \n";
 std::cout << "      a             : Sets de value of a (width) for the potential (in angstrom).\n";
 std::cout << "      re            : Sets de value of re (equilibrium position) for the       \n";
 std::cout << "                      potential (in angstrom).                                 \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use morse as MO                                                               \n";
 std::cout << "     depth 3.4                                                                 \n";
 std::cout << "     a 3.0                                                                     \n";
 std::cout << "     re 1.4                                                                    \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential MO Ar Ar                                                          \n\n";
 std::cout << "      The plugin implements the Morse potential between argon (Ar) atoms.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double Morse::pairEnergy(const double & r) const
{
 return depth*(1.0-exp(-a*(r - re)))*(1.0-exp(-a*(r - re)));
}

Vector Morse::pairForce(const Vector & r) const
{
 double rr = r.Module();
 double ff = 2.0*a*(depth/rr)*(1.0-exp(-a*(rr-re)))*exp(-a*(rr-re));
 Vector fv = r * ff;
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Morse(args); }
void destroy(Plugin * m) { delete m; }
