//
//
//

#include "lennardjones.h"

#include <iostream>

using namespace lpmd;

LennardJones::LennardJones(std::string args): Plugin("lennardjones", "2.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("sigma");
 DefineKeyword("epsilon");
 DefineKeyword("cutoff");
 ProcessArguments(args);
 sigma = params["sigma"];
 epsilon = params["epsilon"];
 cutoff = params["cutoff"];
 SetCutoff(cutoff);
}

void LennardJones::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = lennardjones                                             \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the Lenard-Jones potential for pairs interaction, \n";
 std::cout << "      which has the form                                                       \n";
 std::cout << "            V(r)=4*epsilon( (sigma/r)^12 - (sigma/r)^6 )                       \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      sigma         : Sets de value of sigma for the potential (in angstrom).  \n";
 std::cout << "      epsilon       : Sets de value of epsilon for the potential (in eV).      \n";
 std::cout << "      cutoff        : Cutoff for the potential (in angstrom).                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use lennardjones as lj                                                        \n";
 std::cout << "     sigma 3.4                                                                 \n";
 std::cout << "     epsilon 2.0                                                               \n";
 std::cout << "     cutoff 1.90                                                               \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential lj Ar Ar                                                          \n\n";
 std::cout << "      The plugin implements the Lennard-Jones potential between argon (Ar) atoms.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double LennardJones::pairEnergy(const double & r) const
{
 double rtmp=sigma/r;
 double r6 = rtmp*rtmp*rtmp*rtmp*rtmp*rtmp;
 double r12 = r6*r6;
 return 4.0e0*epsilon*(r12 - r6); 
}

Vector LennardJones::pairForce(const Vector & r) const
{
 double rr2 = r.SquareModule();
 double r6 = pow(sigma*sigma / rr2, 3.0e0);
 double r12 = r6*r6;
 double ff = -48.0e0*(epsilon/rr2)*(r12 - 0.50e0*r6);
 Vector fv = r*ff;
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new LennardJones(args); }
void destroy(Plugin * m) { delete m; }

