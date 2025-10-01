//
//
//

#include "harmonic.h"

#include <iostream>

using namespace lpmd;

Harmonic::Harmonic(std::string args): Plugin("harmonic", "2.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("k");
 DefineKeyword("a");
 ProcessArguments(args);
 k = params["k"];
 a = params["a"];
}

void Harmonic::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = harmonic                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the harmonic potential for pairs interaction,      \n";
 std::cout << "      which has the form                                                       \n";
 std::cout << "                             V(r) = (1/2) k*(r-a).                             \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      k             : Determines de value of k (spring constant) for the       \n";
 std::cout << "                      potential (in eV/angstrom).                              \n";
 std::cout << "      a             : Determines de value of a (equilibrium position) for      \n";
 std::cout << "                      the potential (in angstrom).                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use harmonic as HP                                                            \n";
 std::cout << "     k 3.4                                                                     \n";
 std::cout << "     a 3                                                                       \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential HP Ar Ar                                                          \n\n";
 std::cout << "      The plugin implements a the harmonic potential between argon (Ar) atoms  \n";
 std::cout << "      with the values of k and a indicated above.                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double Harmonic::pairEnergy(const double & r) const
{
 return 0.5e0*k*(fabs(r)-a)*(fabs(r)-a);
}

Vector Harmonic::pairForce(const Vector & r) const
{
 double rr = r.Module();
 double ff = k*(rr-a)/rr;
 Vector fv = r*ff;
// fv.Scale(ff);
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Harmonic(args); }
void destroy(Plugin * m) { delete m; }
