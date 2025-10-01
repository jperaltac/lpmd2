//
//
//

#include "nullpairpotential.h"

#include <iostream>

using namespace lpmd;

NullPairPotential::NullPairPotential(std::string args): Plugin("nullpairpotential", "2.0")
{ 
 ProcessArguments(args); 
}

void NullPairPotential::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nullpairpotential                                        \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements a 'phantom' pair potential. The potential is zero \n";
 std::cout << "      everywhere, so the atoms feel no force at all.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      None.                                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example                                                                       \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nullpairpotential                                                         \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential nullpairpotential Ar Kr                                           \n\n";
 std::cout << "      The plugin implements a null potential between argon (Ar) and krypton    \n";
 std::cout << "      (Kr) atoms. You can use this potential to make an species 'invisible'    \n";
 std::cout << "      to another.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double NullPairPotential::pairEnergy(const double & r) const {assert(&r != 0); return 0.0; }//icc 869

Vector NullPairPotential::pairForce(const Vector & r) const {assert(&r != 0); return Vector(); }//icc 869

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new NullPairPotential(args); }
void destroy(Plugin * m) { delete m; }


