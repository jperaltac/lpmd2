//
//
//

#include "fastlj.h"

#include <iostream>

using namespace lpmd;

FastLJ::FastLJ(std::string args): Plugin("fastlj", "2.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("sigma");
 DefineKeyword("epsilon");
 DefineKeyword("cutoff");
 DefineKeyword("bins", "500");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 sigma = params["sigma"];
 epsilon = params["epsilon"];
 cutoff = params["cutoff"];
 SetCutoff(cutoff);
 bins = int(params["bins"]);
 Tabulate();
}

FastLJ::~FastLJ() 
{ 
 delete [] etable;
 delete [] fftable;
}

void FastLJ::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = fastlj                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to implement the fast Lennard-Jones potential for the \n";
 std::cout << "      atoms pairs interaction. The potential uses a table of values to have a  \n";
 std::cout << "      quick access to the potential.                                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      sigma         : Sets the value of sigma in the potential (in angstrom).  \n";
 std::cout << "      epsilon       : Sets the value of epsilon in the potential (in eV).      \n";
 std::cout << "      cutoff        : Sets the cutoff radius for the potential (in angstrom).  \n";
 std::cout << "      bins          : Sets the number of cells for the table.                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading plugin :                                                             \n";
 std::cout << " use fastlj                                                                    \n";
 std::cout << "     sigma 3.4                                                                 \n";
 std::cout << "     epsilon 0.5                                                               \n";
 std::cout << "     bins 500                                                                  \n";
 std::cout << "     cutoff 1.90                                                               \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential fastlj Ar Ar                                                      \n\n";
 std::cout << "      The plugin implements the Lennard-Jones potential between argon (Ar)     \n";
 std::cout << "      atoms using a table of values of 500 cells.                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void FastLJ::Tabulate()
{
 double r, e, ff, r6, r12;
 etable = new double[bins];
 fftable = new double[bins];
 DebugStream() << "-> Building table for Fast LJ potential, " << bins << " bins" << '\n';
 for (long i=0;i<bins;++i)
 {
  r = cutoff*double(i)/double(bins);
  r6 = pow(sigma / r, 6.0e0);
  r12 = r6*r6;
  e = 4.0e0*epsilon*(r12 - r6); 
  ff = -48.0e0*(epsilon/(r*r))*(r12 - 0.50e0*r6);
  etable[i] = e;
  fftable[i] = ff;
 }
 DebugStream() << "-> Table finished." << '\n';
}

double FastLJ::pairEnergy(const double & r) const
{
 // Interpolate the value of the energy from the table
 long k = long(floor(bins*r/cutoff));
 return etable[k];
}

Vector FastLJ::pairForce(const Vector & r) const
{
 double rr2 = r.SquareModule();
 double rr = sqrt(rr2);
 Vector fv = r;
 long k = long(floor(bins*rr/cutoff));
 fv = fv*fftable[k];
// fv.Scale(fftable[k]);
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new FastLJ(args); }
void destroy(Plugin * m) { delete m; }
