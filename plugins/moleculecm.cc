//
//
//

#include "moleculecm.h"

#include <lpmd/simulation.h>

#include <iostream>

using namespace lpmd;

MoleculeCMModifier::MoleculeCMModifier(std::string args): Plugin("moleculecm", "2.0")
{
 //
 DefineKeyword("radius", "0.0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 radius = double((*this)["radius"]);
}

MoleculeCMModifier::~MoleculeCMModifier() { }

void MoleculeCMModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = moleculecm                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin replaces all the atoms of each molecule (for example, H2) for\n";
 std::cout << "      a single particle, located at the center of mass of the molecule.        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      radius        :                                                          \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use moleculecm                                                                \n";
 std::cout << "     radius 1.1                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply moleculecm                                                            \n\n";
 std::cout << "      The plugin is used to...                                                 \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void MoleculeCMModifier::Apply(Simulation & con)
{
 lpmd::BasicParticleSet & atoms = con.Atoms();

 // Construct an "index table" so we don't have to depend on Atom::Index()
 std::map<BasicAtom *, long int> indices;
 for (long int i=0;i<atoms.Size();++i) indices[&atoms[i]] = i;

 std::list<lpmd::Atom> tmplist;
 int * used = new int[atoms.Size()];
 for (long int i=0;i<atoms.Size();++i) used[i] = 0;
 for (long int i=0;i<atoms.Size();++i) 
 {
  //nlist.clear();
  Vector cmpos; 
  double m = 0.0;
  if (used[i] == 0)
  {
   cmpos = cmpos + (atoms[i].Mass()*atoms[i].Position());
   m += atoms[i].Mass();
   lpmd::NeighborList & nlist = con.Neighbors(i,true,radius);
   lpmd::AtomPair * closest = NULL;
   for (long int k=0;k<nlist.Size();++k)
   {
    lpmd::AtomPair & nn = nlist[k];
    if (nn.r2 < radius*radius)
    {
     if (used[indices[nn.j]] == 0)
     {
      if (closest == NULL) closest = &nn;
      else if (nn.r2 < closest->r2) closest = &nn;
     }
    }
   }
   if (closest != NULL)
   {
    cmpos = cmpos + ((closest->j)->Mass()*(atoms[i].Position()+closest->rij));
    m += (closest->j)->Mass();
    cmpos = (1.0/m)*cmpos;
    tmplist.push_back(Atom(atoms[i].Symbol(), cmpos));
    used[i] = 1;
    used[indices[closest->j]] = 1;
   }
  }
 }
 atoms.Clear(); 
 for (std::list<Atom>::const_iterator it=tmplist.begin();it!=tmplist.end();++it) atoms.Append(Atom(*it));
 delete [] used;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new MoleculeCMModifier(args); }
void destroy(Plugin * m) { delete m; }


