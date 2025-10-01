//
//
//

#include "centrosymmetry.h"

#include <lpmd/simulation.h>
#include <lpmd/properties.h>

#include <sstream>

using namespace lpmd;

CentroSymmetry::CentroSymmetry(std::string args): Plugin("centrosymmetry", "1.0")
{
 ParamList & params=(*this);
 //
 DefineKeyword("rcut");
 DefineKeyword("a", "1.0");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 rcut = params["rcut"];
 a = double(params["a"]);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

CentroSymmetry::~CentroSymmetry() { }

void CentroSymmetry::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = centrosymmetry                                           \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to calculate the centrosymmetry parameter (CSP) for  \n";
 std::cout << "      individual atoms. See Kelchner et al, Phys. Rev. B 58, 11085 (1998).     \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      rcut          : Sets the maximum cutoff radius for the CSP calculation.  \n";
 std::cout << "      a             : Sets the lattice constant to normalize data to this constant.\n";
 std::cout << "      output        : Output file.                                             \n";
 std::cout << "      average       : Sets if the the property must be averaged over all       \n";
 std::cout << "                      configurations (true / false)                            \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use centrosymmetry                                                            \n";
 std::cout << "     output csp.dat                                                            \n";
 std::cout << "     rcut 4.0                                                                  \n";
 std::cout << "     a 5.26                                                                    \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " property centrosymmetry start=1 each=10 end=100                             \n\n";
 std::cout << "      The plugin is used to calculate the CSP per site between the steps 1 and \n";
 std::cout << "      100, each 10 steps, with a cutoff of 4 angstroms and a lattice parametrer\n";
 std::cout << "      of a=5.26 angstrom. The data is written in the file csp.dat.             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void CentroSymmetry::Evaluate(Configuration & con, Potential & pot)
{
 assert(&pot != 0);
 BasicParticleSet & atoms = con.Atoms();
 const long int natoms = atoms.Size();
 Matrix & m = CurrentValue();
 m = Matrix(2, natoms);
 m.SetLabel(0, "Atom");
 m.SetLabel(1, "CentroSymmetry");

 NeighborList * neighbormatrix = new NeighborList[natoms];
 DebugStream() << "-> Computing centrosymmetry parameter per site, over " << natoms << " atoms\n";
 DebugStream() << "-> Building neighbor lists\n";
 for (long i=0;i<natoms;++i) neighbormatrix[i] = con.Neighbors(i, true, rcut);

 DebugStream() << "-> Building index table\n";
 // Construct an "index table" so we don't have to depend on Atom::Index()
 std::map<BasicAtom *, long int> indices;
 for (long int i=0;i<atoms.Size();++i) indices[&atoms[i]] = i;
 // 

 for (long int i=0;i<natoms;++i)
 {
  NeighborList & nlist = neighbormatrix[i];
  long int nneigh = nlist.Size();
  bool processed[nneigh];
  double csp = 0.0;
  for (int k=0;k<nneigh;++k) processed[k] = false;
  while (1)
  {
   int first_nonp = -1;
   for (int k=0;k<nneigh;++k)
       if (!processed[k]) { first_nonp = k; break; }
   if (first_nonp == -1) break;
   processed[first_nonp] = true;
   Vector f = nlist[first_nonp].rij, fop;
   int match = -1;
   double dmin = Dot(nlist[0].rij, f);
   for (long int it=0;it<nlist.Size();++it)
   {
    if (it == first_nonp) continue;
    double d = Dot(nlist[it].rij, f);
    if (d < dmin) { dmin = d; match = it; fop = nlist[it].rij; }
   }
   if (match == -1) csp += f.SquareModule();
   else csp += (fop+f).SquareModule();
   processed[match] = true; 
  }
  m.Set(0, i, (double)i);
  m.Set(1, i, csp/(a*a));
 }
 delete [] neighbormatrix;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new CentroSymmetry(args); }
void destroy(Plugin * m) { delete m; }

