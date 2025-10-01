//
//
//

#include "pairdistances.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/atompair.h>
#include <lpmd/configuration.h>

#include <sstream>

using namespace lpmd;

PairDistances::PairDistances(std::string args): Plugin("pairdistances", "2.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("rcut");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 ProcessArguments(args);
 rcut = params["rcut"];
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

void PairDistances::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = pairdistances                                            \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to calculate the distance between all pair of atoms  \n";
 std::cout << "      of the simulation cell.                                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      rcut          : Set the cutoff radius for the function. Pairs at a       \n";
 std::cout << "                      distance r > rcut will not be considered.                \n";
 std::cout << "      output        : Output file.                                             \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use pairdistances                                                             \n";
 std::cout << "     rcut 10.5                                                                 \n";
 std::cout << "     output pairs.dat                                                          \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " property pairdistances start=0 each=1 end=100                               \n\n";
 std::cout << "      The plugin is used to find all pairs separated at a distace r < rcut in  \n";
 std::cout << "      every one of the first 100 steps. The data is written in the file pairs.dat\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PairDistances::Evaluate(Configuration & conf, Potential & pot)
{
 assert(&pot != 0);
 BasicParticleSet & atoms = conf.Atoms();
 const long int n = atoms.Size();
 NeighborList total_list;
 for (long int i=0;i<n;++i)
 {
  const NeighborList & nlist = conf.Neighbors(i, false, rcut);
  for (long int k=0;k<nlist.Size();++k) total_list.Append(nlist[k]);
 } 
 //
 // Output 
 //
 Matrix & m = CurrentValue();
 m = Matrix(3, total_list.Size());
 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "r");
 m.SetLabel(1, "i");
 m.SetLabel(2, "j");
 // 
 for (long int i=0;i<total_list.Size();++i)
 {
  const AtomPair & nn = total_list[i];
  m.Set(0, i, sqrt(nn.r2));
  m.Set(1, i, (double)nn.i_index);
  m.Set(2, i, (double)nn.j_index);
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new PairDistances(args); }
void destroy(Plugin * m) { delete m; }


