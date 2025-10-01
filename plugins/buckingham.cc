//
//
//

#include "buckingham.h"

#include <iostream>

using namespace lpmd;

Buckingham::Buckingham(std::string args): Plugin("buckingham", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("B1");
 DefineKeyword("B2");
 DefineKeyword("Ro");
 DefineKeyword("cutoff");
 ProcessArguments(args);
 B1 = params["B1"];
 B2 = params["B2"];
 Ro = params["Ro"];
 cutoff = params["cutoff"];
 SetCutoff(cutoff);
}

void Buckingham::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = buckingham                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin incorparate the buckingham potential, this plugin do not     \n";
 std::cout << "      incorporate the coulombian part. In or der to add the coulombian part,   \n";
 std::cout << "      take a look to the ewald plugin.                                         \n";
 std::cout << "                  V(r) = B1*exp(-r/rho) - B2/(r^6)                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      Ro            : Sets the rho value for the potential (in angstrom).      \n";
 std::cout << "      B1            : Sets the B1 value for the potential (in eV).             \n";
 std::cout << "      B2            : Sets the B2 value for the potential (in eV*angstrom^6).  \n";
 std::cout << "      cutoff        : Cutoff radius set for the potential (in angstrom).       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use buckingham as BK1                                                         \n";
 std::cout << "     Ro 1.0                                                                    \n";
 std::cout << "     B1 2.0                                                                    \n";
 std::cout << "     B2 1.0                                                                    \n";
 std::cout << "     cutoff 1.90                                                               \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential BK1 Ge O                                                          \n\n";
 std::cout << "      The plugin implements the Buckingham potential between the germanium (Ge)\n";
 std::cout << "      and oxigen (O) atoms.                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double Buckingham::pairEnergy(const double & r) const
{
 double T1 = B1*exp(-r/Ro);
 double r6 = r*r*r*r*r*r;
 double T2 = B2/r6;
 return T1-T2; 
}

Vector Buckingham::pairForce(const Vector & r) const 
{
 double rr2= r.SquareModule();
 double rr = sqrt(rr2);
 double r8 = rr*rr*rr*rr*rr*rr*rr*rr;
 double f6 = 6*B2/r8;
 double pe = B1*exp(-rr/Ro)/(Ro*rr);
 double ff = f6 - pe;
 Vector fv = r*ff;
// fv.Scale(ff);
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Buckingham(args); }
void destroy(Plugin * m) { delete m; }
