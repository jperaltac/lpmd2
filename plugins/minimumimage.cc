//
//
//

#include "minimumimage.h"
#include <lpmd/configuration.h>

#include <cmath>

using namespace lpmd;

MinimumImageCellManager::MinimumImageCellManager(std::string args): Plugin("minimumimage", "2.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("cutoff", "0.0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 rcut = params["cutoff"];
}

MinimumImageCellManager::~MinimumImageCellManager() { }

void MinimumImageCellManager::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = minimumimage                                             \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the minimum image method for making neighbors     \n";
 std::cout << "      lists.It is one of the available cell managers.                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      cutoff        : Sets the cutoff radius for the evaluation.               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use minimumimage                                                              \n";
 std::cout << "     cutoff 8.5                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " cellmanager minimumimage                                                    \n\n";
 std::cout << "      The plugin is used to select the minimumimage method for making the lists\n";
 std::cout << "      of atoms' neighbors.                                                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void MinimumImageCellManager::Show(std::ostream & os) const
{
 Module::Show(os);
 if (fabs(rcut) < 1E-10) os << "   No cutoff was defined." << '\n';
}

void MinimumImageCellManager::Reset() { }

void MinimumImageCellManager::UpdateCell(Configuration & conf) 
{ 
 BasicCell & cell = conf.Cell();
 if (fabs(rcut) < 1E-10) 
 {
  for (int q=0;q<3;++q)
   if (0.5*cell[q].Module() > rcut) rcut = 0.5*cell[q].Module();
 }
}

void MinimumImageCellManager::UpdateAtom(Configuration & conf, long i)
{
 UpdateCell(conf);
}

double MinimumImageCellManager::Cutoff() const { return rcut; }

void MinimumImageCellManager::BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double rcu)
{
 if (rcu < 1.0E-10) EndWithError("MinimumImage cutoff equals zero...");
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const long int n = atoms.Size();
 nlist.Clear();
 AtomPair nn;
 nn.i = &atoms[i];
 nn.i_index = i;
 const Vector & ipos = atoms[i].Position(); 
 if (full)
 {
  for (long int j=0;j<n;++j)
   if (i != j)
   {
    nn.j = &(atoms[j]);
    nn.rij = cell.Displacement(ipos, nn.j->Position());
    nn.r2 = nn.rij.SquareModule();
    nn.j_index = j;
    if (nn.r2 < rcu*rcu) nlist.Append(nn);
   }
 }
 else
 {
  for (long int j=i+1;j<n;++j)
  {
   nn.j = &(atoms[j]);
   nn.rij = cell.Displacement(ipos, nn.j->Position());
   nn.r2 = nn.rij.SquareModule();
   nn.j_index = j;
   if (nn.r2 < rcu*rcu) nlist.Append(nn);
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new MinimumImageCellManager(args); }
void destroy(Plugin * m) { delete m; }


