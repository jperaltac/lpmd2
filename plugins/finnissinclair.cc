//Finnis-Sinclair Potential
//Finnis, M. W., and Sinclair, J. E., 1984, Philos. Mag. A, 50, 45. 3, 29, 30
//

#include "finnissinclair.h"
#include <iostream>

using namespace lpmd;

FinnisSinclair::FinnisSinclair(std::string args): Plugin("finnissinclair", "2.1")
{
 ParamList & params = (*this);
 DefineKeyword("c0");
 DefineKeyword("c1");
 DefineKeyword("c2");
 DefineKeyword("A");
 DefineKeyword("B");
 DefineKeyword("c");
 DefineKeyword("d");
 ProcessArguments(args); 
 c0 = params["c0"];
 c1 = params["c1"];
 c2 = params["c2"];
 A = params["A"];
 B = params["B"];
 c = params["c"];
 d = params["d"];
 if(d>=c) SetCutoff(d);
 else SetCutoff(c);
}

void FinnisSinclair::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = finnissinclair                                           \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to implement the Finnis-Sinclair potential, used for  \n";
 std::cout << "      BCC metals. It is based in the embedded atom model, with                 \n";
 std::cout << "                       V(r) = (r-c)^2 * (c0+c1*r+c2*r^2)                       \n";
 std::cout << "                     rho(r) = (r-d)^2 + (B/d)*(r-d)^3                          \n";
 std::cout << "                     F(rho) = -A*sqrt(rho)                                     \n";
 std::cout << "      See: Philosophical Magazine 2009, Vol 89: No 34-36, 3311—3332.           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      c0            : Sets the value of c0 in the potential (in eV/angstrom^2).\n";
 std::cout << "      c1            : Sets the value of c1 in the potential (in eV/angstrom^3).\n";
 std::cout << "      c2            : Sets the value of c2 in the potential (in eV/angstrom^4).\n";
 std::cout << "      A             : Sets the value of  A in the potential (in eV/angstrom).  \n";
 std::cout << "      B             : Sets the value of  B in the potential (dimensionless).   \n";
 std::cout << "      c             : Sets the value of the cutoff for the pairs interaction   \n";
 std::cout << "                      part of the potential, V(r) (angstrom).                  \n";
 std::cout << "      d             : Sets the value of the cutoff for the embedding function  \n";
 std::cout << "                      part of the potential, F(rho(r)) (angstrom).             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin                                                           \n";
 std::cout << " use finnissinclair as FS                                                      \n";
 std::cout << "     c0  1.2371147                                                             \n";
 std::cout << "     c1 -0.3592185                                                             \n";
 std::cout << "     c2 -0.0385607                                                             \n";
 std::cout << "     A   1.8289050                                                             \n";
 std::cout << "     B   1.8000000                                                             \n";
 std::cout << "     c   3.4000000                                                             \n";
 std::cout << "     d   3.5697450                                                             \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential FS Fe Fe                                                          \n\n";
 std::cout << "      The plugin implements a the Finnis-Sinclair potential between iron (Fe)  \n";
 std::cout << "      atoms.                                                                   \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double FinnisSinclair::pairEnergy(const double &r) const
{
 if(r<=c) return (r-c)*(r-c)*(c0+c1*r+c2*r*r);
 else return 0.0e0;
}

double FinnisSinclair::rhoij(const double &r) const
{
 if(r<=d) return ((r-d)*(r-d))*(1.0e0+B*(r-d)/d);
 else return 0.0e0;
}

double FinnisSinclair::F(const double &rhoi) const
{
 return -A*sqrt(rhoi);
}

Vector FinnisSinclair::PairForce(const Vector &normrij, const double &r) const
{
 if(r<=c)
 {
  double t1=2.0e0*(r-c)*(c0+c1*r+c2*r*r);
  double t2=(r-c)*(r-c)*(c1+2.0e0*c2*r);
  return (t1+t2)*normrij;
 }
 else return Vector(0.0,0.0,0.0);
}

Vector FinnisSinclair::ManyBodies(const Vector &normrij, const double &rhoi, const double &rhoj, const double &r) const
{
 if(r<=d)
 {
  double t1=((1.0e0/sqrt(rhoi))+(1.0e0/sqrt(rhoj)))*A/2.0e0;
  double t2=2.0e0*(r-d)+3.0e0*B*(r-d)*(r-d)/d;
  return -t1*t2*normrij;
 }
 else return Vector(0.0e0,0.0e0,0.0e0);
}

Vector FinnisSinclair::UpdateCorrections(const double &rho, const int &N, const double &sinv) const
{
 return lpmd::Vector(0,0,0);
}

// Esto se incluye para que el modulo pueda ser cargado dinámicamente
Plugin * create(std::string args) {return new FinnisSinclair(args);}
void destroy(Plugin * m) { delete m; }

