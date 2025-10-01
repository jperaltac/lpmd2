//
//
//

#include "replicate.h"

#include <lpmd/simulation.h>

#include <iostream>

using namespace lpmd;

ReplicateModifier::ReplicateModifier(std::string args): Plugin("replicate", "2.0")
{
 ParamList & param = (*this);
 //
 DefineKeyword("nx", "1");
 DefineKeyword("ny", "1");
 DefineKeyword("nz", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 nx = int(param["nx"]);
 ny = int(param["ny"]);
 nz = int(param["nz"]);
}

ReplicateModifier::~ReplicateModifier() { }

void ReplicateModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = replicate                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to replicate a group of atoms. It is useful to create\n";
 std::cout << "      crystals, where you have a motive that can be replicated.                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      nx            : Sets the number of replications of the set of atoms in   \n";
 std::cout << "                      the X  direction.                                        \n";
 std::cout << "      ny            : Sets the number of replications of the set of atoms in the\n";
 std::cout << "                      the Y  direction.                                        \n";
 std::cout << "      nz            : Sets the number of replications of the set of atoms in the\n";
 std::cout << "                      the Z  direction.                                        \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input module=xyz file=example.xyz                                             \n";
 std::cout << " prepare replicate nx=3 ny=3 nz=3                                            \n\n";
 std::cout << "      The plugin is used to replicate the sample of atoms in the simulation cell.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void ReplicateModifier::Apply(Simulation & con)
{
 lpmd::BasicParticleSet & atoms = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 Cell original_cell(cell);
 Array<Vector> base_positions;
 Array<int> base_z;
 for (long int i=0;i<atoms.Size();++i) 
 {
  base_positions.Append(cell.Fractional(atoms[i].Position()));
  base_z.Append(atoms[i].Z());
 }
 atoms.Clear();
 for (int k=0;k<nz;++k)
  for (int j=0;j<ny;++j)
   for (int i=0;i<nx;++i)
   {
    for (long int p=0;p<base_positions.Size();++p)
    {
     Vector scaled_base(base_positions[p][0]/double(nx), base_positions[p][1]/double(ny), base_positions[p][2]/double(nz));
     Vector position = (scaled_base+Vector(i/double(nx), j/double(ny), k/double(nz)));
     atoms.Append(Atom(ElemSym[base_z[p]], original_cell.Cartesian(position)));
    }
   }
 for (long int i=0;i<atoms.Size();++i) atoms[i].Velocity() = Vector(0.0, 0.0, 0.0);
 for (long int i=0;i<atoms.Size();++i) atoms[i].Acceleration() = Vector(0.0, 0.0, 0.0);
 cell[0] *= nx;
 cell[1] *= ny;
 cell[2] *= nz;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ReplicateModifier(args); }
void destroy(Plugin * m) { delete m; }


