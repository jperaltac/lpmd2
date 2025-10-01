//Gupta Potential
//Cleri, F., and Rosato, V., 1993, Phys. Rev. B, 48, 22. 30
//

#include "gupta.h"
#include <iostream>

using namespace lpmd;

Gupta::Gupta(std::string args): Plugin("gupta", "2.1")
{
 ParamList & params = (*this);
 //
 DefineKeyword("A");
 DefineKeyword("r0");
 DefineKeyword("p");
 DefineKeyword("B");
 DefineKeyword("q");
 DefineKeyword("cutoff");
 ProcessArguments(args); 
 A = params["A"];
 r0 = params["r0"];
 p = params["p"];
 B = params["B"];
 q = params["q"];
 rcut = params["cutoff"];
 SetCutoff(rcut);
}

void Gupta::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = gupta                                                    \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to implement the Gupta potential, used for metals. It \n";
 std::cout << "      is based in the embedded atom model, with                                \n";
 std::cout << "                       V(r) = A*exp(-p*(r-r0)/r0)                              \n";
 std::cout << "                     rho(r) = exp(-2*q*(r-r0)/r0)                              \n";
 std::cout << "                     F(rho) = -B*sqrt(rho)                                     \n";
 std::cout << "      See: Fabrizio Cleri and Vittorio Rosato PRB 48, pag. 22 (1993).          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      A             : Sets the value of  A in the potential (in eV).           \n";
 std::cout << "      B             : Sets the value of  B in the potential (in eV).           \n";
 std::cout << "      r0            : Sets the value of r0 (first neighbors distance) in the   \n";
 std::cout << "                      the potential (in angstrom).                             \n";
 std::cout << "      p             : Sets the value of  p in the potential (dimensionless).   \n";
 std::cout << "      q             : Sets the value of  q in the potential (dimensionless).   \n";
 std::cout << "      cutoff        : Cutoff for the potential (in angstrom).                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin                                                           \n";
 std::cout << " use gupta as Gup                                                              \n";
 std::cout << "     A  0.2061                                                                 \n";
 std::cout << "     r0 2.8840                                                                 \n";
 std::cout << "     p  10.229                                                                 \n";
 std::cout << "     B  1.7900                                                                 \n";
 std::cout << "     q  4.0360                                                                 \n";
 std::cout << "     cutoff 15.0                                                               \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential Gup Au Au                                                         \n\n";
 std::cout << "      The plugin implements a the Gupta potential between gold (Au) atoms.     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

double Gupta::pairEnergy(const double &r) const { return A*exp(-p*(r-r0)/r0); }

double Gupta::rhoij(const double &r) const { return exp(-2.0e0*q*(r-r0)/r0); }

double Gupta::F(const double &rhoi) const { return -B*sqrt(rhoi); }

Vector Gupta::PairForce(const Vector &normrij, const double & rmod) const
{
 return -(A*p/r0)*exp(-p*(rmod-r0)/r0)*normrij;
}

Vector Gupta::ManyBodies(const Vector &normrij, const double &rhoi, const double &rhoj, const double &rmod) const
{
 double tmp=(B*q/r0)*((1.0e0/sqrt(rhoi))+(1.0e0/sqrt(rhoj)))*exp(-2.0e0*q*(rmod-r0)/r0);
 return tmp*normrij;
}

Vector Gupta::UpdateCorrections(const double &rho, const int &N, const double &sinv) const
{
 return lpmd::Vector(0,0,0);
}

// Esto se inlcuye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) {return new Gupta(args);}
void destroy(Plugin * m) { delete m; }

