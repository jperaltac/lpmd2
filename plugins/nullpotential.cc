//
//
//

#include "nullpotential.h"

#include <iostream>

using namespace lpmd;

NullPotential::NullPotential(std::string args): Plugin("nullpotential", "2.0")
{ 
 ProcessArguments(args); 
}

NullPotential::~NullPotential() { }

void NullPotential::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nullpotential                                            \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements a 'phantom' potential. The potential is zero      \n";
 std::cout << "      everywhere, so the atoms feel no force at all.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      None.                                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example                                                                       \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nullpotential                                                             \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential nullpotential Ar Kr                                               \n\n";
 std::cout << "      The plugin implements a null potential between argon (Ar) and krypton    \n";
 std::cout << "      (Kr) atoms. You can use this potential to make an species 'invisible'    \n";
 std::cout << "      to another.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double NullPotential::energy(Configuration & conf) {assert(&conf != 0); return 0.0; }//icc 869

double NullPotential::AtomEnergy(Configuration & conf, long i) { return 0.0; }

void NullPotential::UpdateForces(Configuration & conf) { assert(&conf != 0); }//icc 869

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new NullPotential(args); }
void destroy(Plugin * m) { delete m; }


