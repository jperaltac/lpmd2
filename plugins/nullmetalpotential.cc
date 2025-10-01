//
//
//

#include "nullmetalpotential.h"

#include <iostream>

using namespace lpmd;

NullMetalPotential::NullMetalPotential(std::string args): Plugin("nullmetalpotential", "2.0")
{ 
 //
 ProcessArguments(args); 
}

void NullMetalPotential::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nullmetalpotential                                       \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements a 'phantom' emmbeded-atom-like potential. The     \n";
 std::cout << "      potential is zero everywhere, so the atoms feel no force at all.         \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      None.                                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nullmetalpotential                                                        \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential nullmetalpotential Cu Cu                                          \n\n";
 std::cout << "      The plugin implements a null potential between copper (Cu) atoms.        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double NullMetalPotential::pairEnergy(const double &r) const {assert(&r != 0); return 0.0; }//icc 869

double NullMetalPotential::rhoij(const double &r) const {assert(&r != 0); return 0.0; }//icc 869

double NullMetalPotential::F(const double &rhoi) const {assert(&rhoi != 0); return 0.0; }//icc 869

Vector NullMetalPotential::PairForce(const Vector &normrij, const double & mod) const 
{
 assert(&normrij != 0);//icc 869
 assert(&mod != 0); //icc869
 return Vector(0.0, 0.0, 0.0); 
}

Vector NullMetalPotential::ManyBodies(const Vector &normrij, const double &invrhoi, const double &invrhoj, const double & mod) const 
{
 assert(&normrij != 0); //icc 869
 assert(&invrhoi != 0); //icc 869
 assert(&invrhoj != 0); //icc 869
 assert(&mod != 0); //icc 869 
 return Vector(0.0, 0.0, 0.0); 
}

double NullMetalPotential::deltarhoi(const double &rhobar) const { assert(&rhobar != 0); return 0.0; }//icc 869

double NullMetalPotential::deltaU1(const double &rhobar, const int &N) const 
{ 
 assert(&rhobar != 0); //icc869
 assert(&N != 0);//icc 869 
 return 0.0; 
}

double NullMetalPotential::deltaU2(const double &rhobar, const int &N, const double &rhoi) const 
{ 
 assert(&rhobar != 0); //icc869
 assert(&N != 0);//icc 869
 assert(&rhoi !=0);//icc 869 
 return 0.0; 
}

double NullMetalPotential::VirialCorrection(const double &rhobar, const int &N, const double &rhoi) const 
{
 assert(&rhobar != 0); //icc869
 assert(&N != 0);//icc 869
 assert(&rhoi !=0);//icc 869 
 return 0.0; 
}

Vector NullMetalPotential::UpdateCorrections(const double &rho, const int &N, const double &sinv) const
{
 return lpmd::Vector(0,0,0);
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) {return new NullMetalPotential(args);}
void destroy(Plugin * m) { delete m; }

