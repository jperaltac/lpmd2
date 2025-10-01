//Finnis-Sinclair Extended Potential
//Dai, X. D., Kong, Y., Li, J. H., and Liu, B. X., 2006, J. Phys.: Condens. Matter, 18, 45274542. 30
//

#include "finnissinclair-ext.h"
#include <iostream>

using namespace lpmd;

FinnisSinclairExt::FinnisSinclairExt(std::string args): Plugin("finnissinclair-ext", "2.1")
{
 ParamList & params = (*this);
 DefineKeyword("c0");
 DefineKeyword("c1");
 DefineKeyword("c2");
 DefineKeyword("c3");
 DefineKeyword("c4");
 DefineKeyword("A");
 DefineKeyword("B");
 DefineKeyword("c");
 DefineKeyword("d");
 ProcessArguments(args); 
 c0 = params["c0"];
 c1 = params["c1"];
 c2 = params["c2"];
 c3 = params["c3"];
 c4 = params["c4"];
 A = params["A"];
 B = params["B"];
 c = params["c"];
 d = params["d"];
 if(d>=c) SetCutoff(d);
 else SetCutoff(c);
}

void FinnisSinclairExt::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = finnissinclair-ext                                       \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to implement an extended version of the Finnis-Sinclair\n";
 std::cout << "      potential, with two aditional terms in the pairs interaction term and a  \n";
 std::cout << "      modified charge density term, rho(r).                                    \n";
 std::cout << "                       V(r) = (r-c)^2 * (c0+c1*r+c2*r^2+c3*r^3+c4*r^4)         \n";
 std::cout << "                     rho(r) = (r-d)^2 + B^2 *(r-d)^4                           \n";
 std::cout << "                     F(rho) = -A*sqrt(rho)                                     \n";
 std::cout << "      See: J. Phys. Condens. Matter 2006, 18, 45274542. 30.                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      c0            : Sets the value of c0 in the potential (in eV/angstrom^2).\n";
 std::cout << "      c1            : Sets the value of c1 in the potential (in eV/angstrom^3).\n";
 std::cout << "      c2            : Sets the value of c2 in the potential (in eV/angstrom^4).\n";
 std::cout << "      c3            : Sets the value of c3 in the potential (in eV/angstrom^5).\n";
 std::cout << "      c4            : Sets the value of c4 in the potential (in eV/angstrom^6).\n";
 std::cout << "      A             : Sets the value of  A in the potential (in eV/angstrom).  \n";
 std::cout << "      B             : Sets the value of  B in the potential (in angstrom^(-2)).\n";
 std::cout << "      c             : Sets the value of the cutoff for the pairs interaction   \n";
 std::cout << "                      part of the potential, V(r) (angstrom).                  \n";
 std::cout << "      d             : Sets the value of the cutoff for the embedding function  \n";
 std::cout << "                      part of the potential, F(rho(r)) (angstrom).             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin                                                           \n";
 std::cout << " use finnissinclair-ext as FS                                                  \n";
 std::cout << "     c0  47.98066                                                              \n";
 std::cout << "     c1 −34.09924                                                              \n";
 std::cout << "     c2  5.832293                                                              \n";
 std::cout << "     c3  0.017494                                                              \n";
 std::cout << "     c4  0.020393                                                              \n";
 std::cout << "     A   1.848648                                                              \n";
 std::cout << "     B   0.000000                                                              \n";
 std::cout << "     c   3.257200                                                              \n";
 std::cout << "     d   4.147200                                                              \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential FS Mo Mo                                                          \n\n";
 std::cout << "      The plugin implements a the Finnis-Sinclair potential between molibdenum \n";
 std::cout << "      (Mo) atoms.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double FinnisSinclairExt::pairEnergy(const double &r) const
{
 if(r<=c) 
 {
  double r2 = r*r;
  return (r-c)*(r-c)*(c0+c1*r+c2*r2+c3*r2*r+c4*r2*r2);
 }
 else return 0.0e0;
}

double FinnisSinclairExt::rhoij(const double &r) const
{
 if(r<=d)
 {
  double rd=r-d; 
  return (rd*rd)*(1.0e0+B*B*rd*rd);
 }
 else return 0.0e0;
}

double FinnisSinclairExt::F(const double &rhoi) const
{
 return -A*sqrt(rhoi);
}

Vector FinnisSinclairExt::PairForce(const Vector &normrij, const double &r) const
{
 if(r<=c)
 {
  double r2=r*r;
  double rc=r-c;
  double t1=2.0e0*rc*(c0+c1*r+c2*r2+c3*r2*r+c4*r2*r2);
  double t2=(c1+2.0e0*c2*r+3.0e0*c3*r2+4.0e0*c4*r2*r)*rc*rc;
  return (t1+t2)*normrij;
  }
  else return Vector(0.0e0,0.0e0,0.0e0);
}

Vector FinnisSinclairExt::ManyBodies(const Vector &normrij, const double &rhoi, const double &rhoj, const double &r) const
{
 if(r<=d)
 {
  double rd=r-d;
  double t1=A*0.5e0*((1.0e0/sqrt(rhoi))+(1.0e0/sqrt(rhoj)));
  double t2=2.0e0*rd + 4.0e0*B*B*rd*rd*rd;
  return -t1*t2*normrij;
 }
 else return Vector(0.0e0,0.0e0,0.0e0);
}

Vector FinnisSinclairExt::UpdateCorrections(const double &rho, const int &N, const double &sinv) const
{
 return lpmd::Vector(0,0,0);
}

// Esto se incluye para que el modulo pueda ser cargado dinámicamente
Plugin * create(std::string args) {return new FinnisSinclairExt(args);}
void destroy(Plugin * m) { delete m; }

