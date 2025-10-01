//
//
//

#include "verletlist.h"
#include <lpmd/configuration.h>

#include <cmath>

using namespace lpmd;

VerletListCellManager::VerletListCellManager(std::string args): Plugin("verletlist", "1.0")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("cutoff", "0.0");
 DefineKeyword("extcutoff", "0.0");
 DefineKeyword("maxneighbors", "80");
 DefineKeyword("each", "5");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 rcut = params["cutoff"];
 maxneighbors = int(params["maxneighbors"]);
 each = int(params["each"]);
 if (fabs(extcut) < 1E-10) extcut = 1.08*rcut;
 head = NULL;
 vlist = NULL;
 nv = NULL;
}

VerletListCellManager::~VerletListCellManager() { }

void VerletListCellManager::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = verletlist                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the verlet-list method for making neighbors lists.\n";
 std::cout << "      It is one of the available cell managers.                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      cutoff        : Sets the inner cutoff radius for the list (in angstrom). \n";
 std::cout << "      extcutoff     : Sets the outter cutoff radius for the list. Its default  \n";
 std::cout << "                      value is (cutoff + .08*cutoff).                          \n"; 
 std::cout << "      maxneighbors  : Set the maximum number of neighbors that have a atom in  \n";
 std::cout << "                      the list.                                                \n";
 std::cout << "      each          : Set the frequency to 'rebuild' the atomic neighbors list.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use verletlist                                                                \n";
 std::cout << "     cutoff 5.0                                                                \n";
 std::cout << "     maxneighbors 15                                                           \n";
 std::cout << "     each 10                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " cellmanager verletlist                                                      \n\n";
 std::cout << "      The plugin is used to select the verlet-list method for making the lists \n";
 std::cout << "      of atoms' neighbors. In this case, the maximum number of neighbors allowed\n";
 std::cout << "      is 15, and the list is updated each 10 steps.                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

}


void VerletListCellManager::Reset() { }

void VerletListCellManager::UpdateVerletList(Configuration & conf) 
{ 
 if (fabs(rcut) < 1E-10) throw PluginError("verletlist", "No cutoff was defined");
 BasicCell & cell = conf.Cell();
 BasicParticleSet & atoms = conf.Atoms();
 const long int n = atoms.Size();
 if (n != old_size) 
 { 
  delete [] head;
  head = NULL;
  delete [] nv;
  nv = NULL;
  for (long i=0;i<old_size;++i) delete vlist[i];
  delete [] vlist;
  vlist = NULL;
 }
 if (head == NULL) 
 {
  head = new double[n];
  nv = new long[n];
  vlist = new double*[n];
  for (long i=0;i<n;++i) vlist[i] = new double[maxneighbors];
 }
 for (long i=0;i<n;++i) nv[i] = 0;
 for (long i=0;i<n-1;++i)
   for (long j=i+1;j<n;++j)
   {
    const Vector rij = cell.Displacement(atoms[i].Position(), atoms[j].Position());
    if (rij.SquareModule() < extcut*extcut)
    {
     if ((nv[j] == maxneighbors-1) || (nv[i] == maxneighbors-1)) 
      throw PluginError("verletlist", "maxneighbors is too small!");
     // Add i to verlet list of j
     vlist[j][nv[j]++] = i;
     // Add j to verlet list of i
     vlist[i][nv[i]++] = j;
    }
   }
}

void VerletListCellManager::UpdateAtom(Configuration & conf, long i)
{
 nv[i] = 0;
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const long int n = atoms.Size();
 for (long j=0;j<n;++j)
 {
  if (j != i)
  {
   const Vector rij = cell.Displacement(atoms[i].Position(), atoms[j].Position());
   if (rij.SquareModule() < extcut*extcut)
   {
    if (nv[i] == maxneighbors-1) throw PluginError("verletlist", "maxneighbors is too small!");
    // Add j to verlet list of i
    vlist[i][nv[i]++] = j;
   }
  }
 }
}

void VerletListCellManager::UpdateCell(Configuration & conf) 
{
 BasicParticleSet & atoms = conf.Atoms();
 bool mustupdate = (((step_count % each) == 0) || (atoms.Size() != old_size));
 if (mustupdate) UpdateVerletList(conf);
 step_count++;
 old_size = atoms.Size();
}

double VerletListCellManager::Cutoff() const { return rcut; }

void VerletListCellManager::BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double rcu)
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 nlist.Clear();
 AtomPair nn;
 nn.i = &atoms[i];
 nn.i_index = i;
 const Vector & ipos = atoms[i].Position(); 
 if (full)
 {
  for (long jp=0;jp<nv[i];++jp) 
  {
   long j = vlist[i][jp];
   nn.j = &(atoms[j]);
   nn.rij = cell.Displacement(ipos, nn.j->Position());
   nn.r2 = nn.rij.SquareModule();
   nn.j_index = j;
   if (nn.r2 < rcu*rcu) nlist.Append(nn);
  }
 }
 else
 {
  for (long jp=0;jp<nv[i];++jp) 
  {
   long j = vlist[i][jp];
   if (j < i) continue;
   nn.j = &(atoms[j]);
   nn.rij = cell.Displacement(ipos, nn.j->Position());
   nn.r2 = nn.rij.SquareModule();
   nn.j_index = j;
   if (nn.r2 < rcu*rcu) nlist.Append(nn);
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new VerletListCellManager(args); }
void destroy(Plugin * m) { delete m; }


