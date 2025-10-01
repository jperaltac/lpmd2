//
//
//

#include "angularmomentum.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/session.h>
#include <lpmd/configuration.h>

#include <sstream>

using namespace lpmd;

AngularMomentum::AngularMomentum(std::string args): Plugin("angularmomentum", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 DefineKeyword("center");
 DefineKeyword("debug", "none");
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
 center = Vector((*this)["center"].c_str());
}

AngularMomentum::~AngularMomentum() { }

void AngularMomentum::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = angularmomentum                                          \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      Evaluate the angular momentum of a system respect to an origin (center). \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      center        : Vector that specify the center.                          \n";
 std::cout << "      output        : Output File.                                             \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the Plugin :                                                         \n";
 std::cout << " use angularmomentum                                                           \n";
 std::cout << "     center <5.0,5.0,5.0>                                                      \n";
 std::cout << "     output angmom.dat                                                         \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Apply the plugin :                                                           \n";  
 std::cout << " property angularmomentum start=1 each=10 end=100                            \n\n";
 std::cout << "      The plugin is used to calculate angular momentum of the system respect   \n";
 std::cout << "      to the point (5,5,5).                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void AngularMomentum::Evaluate(Configuration & conf, Potential & pot)
{
 assert(&pot != 0);
 const double kin2ev = double(GlobalSession["kin2ev"]);
 const double evfs2h = 0.24179895;          // eV*fs to Planck's h 
 BasicParticleSet & atoms = conf.Atoms();
 long int n = atoms.Size();
 Vector lsum(0.0, 0.0, 0.0);
 for (long int i=0;i<n;++i)
 {
  const Vector & v = atoms[i].Velocity();
  const Vector & r = atoms[i].Position();
  double m = atoms[i].Mass();
  lsum = lsum + m*Cross(r-center, v);
 }
 Matrix & m = CurrentValue() = Matrix(4, 1);
 
 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "Lx");
 m.SetLabel(1, "Ly");
 m.SetLabel(2, "Lz");
 m.SetLabel(3, "|L|");
 for (int q=0;q<3;++q) m.Set(q, 0, kin2ev*evfs2h*lsum[q]); // L in units of Planck's h
 m.Set(3, 0, sqrt(lsum[0]*lsum[0]+lsum[1]*lsum[1]+lsum[2]*lsum[2])*kin2ev*evfs2h);
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AngularMomentum(args); }
void destroy(Plugin * m) { delete m; }

