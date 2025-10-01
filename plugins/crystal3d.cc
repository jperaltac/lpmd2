//
//
//

#include "crystal3d.h"

#include <lpmd/atom.h>
#include <lpmd/configuration.h>

using namespace lpmd;

CrystalGenerator::CrystalGenerator(std::string args): Plugin("crystal3d", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("nx", "1");
 DefineKeyword("ny", "1");
 DefineKeyword("nz", "1");
 DefineKeyword("symbol", "H");
 DefineKeyword("type", "fcc");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 spc = ElemNum((*this)["symbol"]);
 type = (*this)["type"];
 nx = int(params["nx"]);
 ny = int(params["ny"]);
 nz = int(params["nz"]);
}

CrystalGenerator::~CrystalGenerator() { }

void CrystalGenerator::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = crystal3d                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to generate three-dimensional lattices. The available\n";
 std::cout << "      lattices are BCC (base-centered cubic), FCC (face-centered cubic), HCP   \n";
 std::cout << "      (Hexagonal close-packed) and SC (simple cubic).                          \n";
 std::cout << "      The total number of atoms in the cell corresponds to nx*ny*nx*NB, (see   \n";
 std::cout << "      below) where NB is the number of atoms in the unit cell. NB=2 for BCC    \n";
 std::cout << "      and HCP lattices, NB=1 for SC lattice and NB=4 for the FCC lattice.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      symbol        : Symbol of the atomic species.                            \n";
 std::cout << "      type          : Sets the type of unit cell (fcc/bcc/hcp/sc).             \n";
 std::cout << "      nx            : Sets the number of replications of the unit cell in the  \n";
 std::cout << "                      X  direction.                                            \n";
 std::cout << "      ny            : Sets the number of replications of the unit cell in the  \n";
 std::cout << "                      Y  direction.                                            \n";
 std::cout << "      nz            : Sets the number of replications of the unit cell in the  \n";
 std::cout << "                      Z  direction.                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input crystal3d symbol=Ar type=fcc nx=3 ny=3 nz=3                           \n\n";
 std::cout << "      The plugin is used to generate a three-dimensional fcc lattice of argon  \n";
 std::cout << "      atoms. The total number of atoms generated this case is 3x3x3x4=108.     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void CrystalGenerator::Generate(Configuration & conf) const
{
 long int cc = 0;
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 double ax = 1.0/double(nx);
 double ay = 1.0/double(ny);
 double az = 1.0/double(nz);
 bool create_atoms = (atoms.Size() == 0);
 
 if (type=="bcc")
 {
  for (long k=0;k<nz;++k)
   for (long j=0;j<ny;++j)
    for (long i=0;i<nx;++i)
    {
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector(double(i)*ax, double(j)*ay, double(k)*az));
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+0.5)*ax, (double(j)+0.5)*ay, (double(k)+0.5)*az));
    }
 }
 else if (type=="fcc")
 {
  for (long k=0;k<nz;++k)
   for (long j=0;j<ny;++j)
    for (long i=0;i<nx;++i)
    {
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+0.51)*ax, (double(j)+0.5)*ay, (double(k)+0.5)*az));
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+0.51)*ax, double(j)*ay, (double(k)+1.0)*az));
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+1.01)*ax, double(j)*ay, (double(k)+0.5)*az));
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+1.01)*ax, (double(j)+0.5)*ay, (double(k)+1.0)*az));
    }
 }
 else if (type=="hcp")
 {
  for (long k=0;k<nz;++k)
   for (long j=0;j<ny;++j)
    for (long i=0;i<nx;++i)
    {
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+(2.0/3.0))*ax, (double(j)+(1.0/3.0))*ay, (double(k)+(3.0/4.0))*az));
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector((double(i)+(1.0/3.0))*ax, (double(j)+(2.0/3.0))*ay, (double(k)+(1.0/4.0))*az));
    }
 }
 else if (type=="sc")
 {
  for (long k=0;k<nz;++k)
   for (long j=0;j<ny;++j)
    for (long i=0;i<nx;++i)
    {
     if (create_atoms) atoms.Append(Atom(spc));
     atoms[cc++].Position() = cell.Cartesian(Vector(double(i)*ax, double(j)*ay, double(k)*az));
    }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new CrystalGenerator(args); }
void destroy(Plugin * m) { delete m; }


