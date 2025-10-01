//
//
//

#include "veldist.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/configuration.h>

#include <sstream>

using namespace lpmd;

VelDist::VelDist(std::string args): Plugin("veldist", "2.1")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 DefineKeyword("bins", "300");
 ProcessArguments(args);
 bins = int(params["bins"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

VelDist::~VelDist() { }

void VelDist::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = veldist                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to evaluate a velocities histogram.                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      bins          : Set the number of bins fo the histogram.                 \n";
 std::cout << "      output        : Set the output file name.                                \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use veldist                                                                   \n";
 std::cout << "     output veldist.dat                                                        \n";
 std::cout << "     bins 500                                                                  \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";  
 std::cout << " property veldist start=1 each=10 end=100                                    \n\n";
 std::cout << "      The plugin is used to make a velocities histogram for the first 100      \n";
 std::cout << "      steps, each 10 steps, using a 500 different velocities. The data is      \n";
 std::cout << "      written in the file veldist.dat.                                         \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void VelDist::Evaluate(Configuration & conf, Potential & pot)
{
 assert(&pot != 0);
 //
 // Primero determina los valores extremos
 // 
 BasicParticleSet & atoms = conf.Atoms();
 long int n = atoms.Size();
 Vector vmin, vmax;
 double vmodmin, vmodmax, vrmin, vrmax;
 vmin = vmax = atoms[0].Velocity();
 vmodmin = vmodmax = atoms[0].Velocity().Module();
 vrmin = vmodmin;
 vrmax = vmodmax;
 for (long int i=0;i<n;++i)
 {
  const Vector & v = atoms[i].Velocity();
  for (int q=0;q<3;++q)
  {
   if (v[q] < vmin[q]) vmin[q] = v[q]; 
   if (v[q] > vmax[q]) vmax[q] = v[q]; 
   if (vmin[q] < vrmin) vrmin = vmin[q];
   if (vmax[q] > vrmax) vrmax = vmax[q];
  }
  if (v.Module() < vmodmin) vmodmin = v.Module();
  if (v.Module() > vmodmax) vmodmax = v.Module();
  if (vmodmin < vrmin) vrmin = vmodmin;
  if (vmodmax > vrmax) vrmax = vmodmax;
 }
 // 
 // Ahora llena el histograma 
 //
 Matrix & m = CurrentValue() = Matrix(5, bins);
 
 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "v");
 m.SetLabel(1, "vx dist");
 m.SetLabel(2, "vy dist");
 m.SetLabel(3, "vz dist");
 m.SetLabel(4, "|v| dist");
 for (int z=0;z<bins;++z)
 {
  m.Set(0, z, vrmin+z*(vrmax-vrmin)/double(bins-1));
  for (int q=0;q<3;++q) m.Set(q+1, z, 0.0);
  m.Set(4, z, 0.0);  
 }
 for (long int i=0;i<n;++i)
 {
  const Vector & v = atoms[i].Velocity();
  for (int q=0;q<3;++q)
  {
   int k = int(floor((bins-1)*((v[q]-vrmin)/(vrmax-vrmin))));
   m.Set(q+1, k, m.Get(q+1, k)+1);
  }
  int k = int(floor((bins-1)*((v.Module()-vrmin)/(vrmax-vrmin))));
  m.Set(4, k, m.Get(4, k)+1);
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new VelDist(args); }
void destroy(Plugin * m) { delete m; }


