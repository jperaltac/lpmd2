//
//
//

#include "mcy.h"

#include <iostream>

using namespace lpmd;

MCY::MCY(std::string args): Plugin("mcy", "1.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("A");
 DefineKeyword("B");
 DefineKeyword("C");
 DefineKeyword("D");
 DefineKeyword("cutoff");
 ProcessArguments(args);
 cutoff = params["cutoff"];
 SetCutoff(cutoff);
 A = params["A"];
 B = params["B"];
 C = params["C"];
 D = params["D"];
}

void MCY::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = mcy                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the Matsuoka, Clementi, and Yoshimine (MCY)        \n";
 std::cout << "      potential for pairs interaction, which has the form                      \n";
 std::cout << "                   V(r) = A*exp(-B*r)-C*exp(-D*r)                              \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      cutoff        : Cutoff for the potential.                                \n";
 std::cout << "      A             : Sets the value of  A in the potential (in eV).           \n";
 std::cout << "      B             : Sets the value of  B in the potential (in 1/angstrom).   \n";
 std::cout << "      C             : Sets the value of  C in the potential (in eV).           \n";
 std::cout << "      D             : Sets the value of  D in the potential (in 1/angstrom).   \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use mcy                                                                       \n";
 std::cout << "     A 0.001                                                                   \n";
 std::cout << "     B 2.5                                                                     \n";
 std::cout << "     C 0.002                                                                   \n";
 std::cout << "     D 2.7                                                                     \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential mcy H O                                                           \n\n";
 std::cout << "      The plugin implements the MCY potential between hydrogen (H) and         \n";
 std::cout << "      oxigen (O) atoms.                                                        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double MCY::pairEnergy(const double & r) const
{
 return (A*exp(-B*r)-C*exp(-D*r));
}

Vector MCY::pairForce(const Vector & r) const
{
 double rr = r.Module();
 double ff = (1.0/rr)*(C*D*exp(-D*rr)-A*B*exp(-B*rr));
 Vector fv = r*ff;
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new MCY(args); }
void destroy(Plugin * m) { delete m; }

