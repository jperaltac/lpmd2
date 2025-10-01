//
//
//

#include "atomenergy.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/atompair.h>
#include <lpmd/configuration.h>
#include <lpmd/pairpotential.h>
#include <lpmd/combinedpotential.h>

#include <sstream>
#include <exception>

using namespace lpmd;

AtomEnergy::AtomEnergy(std::string args): Plugin("atomenergy", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 DefineKeyword("debug", "none");
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

void AtomEnergy::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = atomenergy                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      Plugin designed to evaluate the potential energy per atom.               \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      output        : Output File                                              \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Load the Plugin   :                                                          \n";
 std::cout << " use atomenergy                                                                \n";
 std::cout << "     output energ.dat                                                          \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Apply the Plugin :                                                           \n";  
 std::cout << " property atomenergy start=1 each=10 end=100                                 \n\n";
 std::cout << "      The plugin is used to calculate the potential energy per atom as a       \n";
 std::cout << "      function of time. The data is written in the file energ.dat.             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void AtomEnergy::Evaluate(Configuration & conf, Potential & pot)
{
 assert(&pot != 0);
 BasicParticleSet & atoms = conf.Atoms();
 const long int n = atoms.Size();
 double * pair_energy = new double[n];
 CombinedPotential & comb = dynamic_cast<CombinedPotential &>(pot);
 for (long i=0;i<n;++i) pair_energy[i] = comb.AtomEnergy(conf, i);
 //
 // Output 
 //
 Matrix & m = CurrentValue();
 m = Matrix(2, n);
 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "index");
 m.SetLabel(1, "energy");
 // 
 for (long int i=0;i<n;++i)
 {
  m.Set(0, i, i);
  m.Set(1, i, pair_energy[i]);
 }
 delete [] pair_energy;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AtomEnergy(args); }
void destroy(Plugin * m) { delete m; }

