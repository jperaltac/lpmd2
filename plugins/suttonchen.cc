//Sutton-Chen PseudoPotential plugin.
//Sutton, A. P., and Chen, J., 1990, Philos. Mag. Lett., 61, 139. 3, 30
//Rafii-Tabar, H., and Sutton, A. P., 1991, Philos. Mag. Lett., 63, 217. 3, 30, 36, 37
//Todd, B. D., and Lynden-Bell, R. M., 1993, Surf. Science, 281, 191. 3, 30

#include "suttonchen.h"
#include <iostream>

using namespace lpmd;

SuttonChen::SuttonChen(std::string args): Plugin("suttonchen", "2.1")
{
 ParamList & params = (*this);
 //
 DefineKeyword("e");
 DefineKeyword("a");
 DefineKeyword("n");
 DefineKeyword("m");
 DefineKeyword("c");
 DefineKeyword("cutoff");
 DefineKeyword("corrections","false");
 ProcessArguments(args); 
 e = double(params["e"]);
 a = double(params["a"]);
 n = double(params["n"]);
 m = double(params["m"]);
 c = double(params["c"]);
 rcut = double(params["cutoff"]);
 corrections = bool(params["corrections"]);
 an = pow(a,n);
 am = pow(a,m);
 SetCutoff(rcut);
}

void SuttonChen::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = suttonchen                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin incorporate the SuttonChen potencial, used frequently         \n";
 std::cout << "      for metallic atomic interaction. Based in embedded atom model.           \n";
 std::cout << "          V(r) = e*(a/r)^n ; F(rho) = -c*e*sqrt(rho) ; rho(r) = (a/r)^m        \n";
 std::cout << "      See: Philosophical Mag. Lett. 1991, Vol 63, No 4, 217-224                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      e             : Sets the value of epsilon in the potential (in eV).      \n";
 std::cout << "      a             : Sets the value of a in the potencial (in angstrom).      \n";
 std::cout << "      n             : Value of n in the potential (dimensionless).             \n";
 std::cout << "      m             : Value of m in the potential (dimensionless).             \n";
 std::cout << "      c             : Value of c in the potential (dimensionless).             \n";
 std::cout << "      cutoff        : Sets the cutoff for the potential (in angstrom).         \n";
 std::cout << "      corrections   : Sets if corrections to the metallic potential should be   \n";
 std::cout << "                      considered (true / false). Recommended for homogeneous systems.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use suttonchen as sc                                                          \n";
 std::cout << "     e 0.015713                                                                \n";
 std::cout << "     a 3.610000                                                                \n";
 std::cout << "     n 9.0                                                                     \n";
 std::cout << "     m 6.0                                                                     \n";
 std::cout << "     c 39.75500                                                                \n";
 std::cout << "     cutoff 6.0                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Using the loaded plugin :                                                    \n";
 std::cout << " potential sc Cu Cu                                                          \n\n";
 std::cout << "      The plugin implements a the Sutton-Chen potential between copper (Cu)    \n";
 std::cout << "      atoms.                                                                   \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double SuttonChen::pairEnergy(const double &r) const { double ir = 1/r; return e*an*pow(ir,n); }

double SuttonChen::rhoij(const double &r) const { double ir = 1/r; return am*pow(ir,m); }

double SuttonChen::F(const double &rhoi) const { return -c*e*sqrt(rhoi); }

Vector SuttonChen::PairForce(const Vector &normrij, const double &rmod) const
{
 double ir = 1.0e0/rmod;
 return -n*e*an*pow(ir,n+1)*normrij;
}

Vector SuttonChen::ManyBodies(const Vector &normrij, const double &rhoi, const double &rhoj, const double &rmod) const
{
 double ir = 1.0e0/rmod;
 double tmp=0.5*m*c*e*((1/sqrt(rhoi))+1/(sqrt(rhoj)))*am*pow(ir,m+1);
 return tmp*normrij;
}

Vector SuttonChen::UpdateCorrections(const double &rho, const int &N, const double &sinv) const
{
 double drho=0.0e0,du=0.0e0,dvir=0.0e0;
 double fac=-c*e*0.5e0*sinv;
 drho = (4*M_PI*rho*rcut/(m-1))*pow((a/(rcut)),m);
 int pot=n-3;
 double aor = a/rcut;
 du = (2*M_PI*N*rho*e/(n-3))*a*a*a*pow(aor,pot);
 std::cerr << "rho = " << rho << '\n';
 std::cerr << "N   = " << N   << '\n';
 std::cerr << "e   = " << e   << '\n';
 std::cerr << "a   = " << a   << '\n';
 std::cerr << "n   = " << n   << '\n';
 std::cerr << "aor = " << aor << '\n';
 std::cerr << "rcut= " << rcut<< '\n';
 std::cerr << "pot = " << pot << '\n';
 std::cerr << "pow = " << pow(aor,pot) << '\n';
 double v1 = -2.0e0*M_PI*rho*N*e*n*a*a*a*pow(aor,pot)/(n-3);
 pot = m-3;
 std::cerr << "pot = " << pot << '\n';
 double v2 = -4.0e0*M_PI*fac*rho*m*a*a*a*pow(aor,pot)/(m-3);
 std::cerr << " V1 = " << v1 << '\n';
 std::cerr << " V2 = " << v2 << '\n';
 dvir=v1+v2;
 return Vector(drho,du,dvir);
}

// Esto se inlcuye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) {return new SuttonChen(args);}
void destroy(Plugin * m) { delete m; }

