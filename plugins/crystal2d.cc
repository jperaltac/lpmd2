//
//
//

#include "crystal2d.h"

#include <lpmd/atom.h>
#include <lpmd/configuration.h>

using namespace lpmd;

Crystal2DGenerator::Crystal2DGenerator(std::string args): Plugin("crystal2d", "2.0")
{
 ParamList & params = (*this);
 DefineKeyword("a", "1");
 DefineKeyword("b", "1");
 DefineKeyword("gamma", "90");
 DefineKeyword("nx", "1");
 DefineKeyword("ny", "1");
 DefineKeyword("symbol", "H");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 a = params["a"];
 b = params["b"];
 gamma = M_PI*double(params["gamma"])/180.0e0;
 spc = ElemNum(params["symbol"]);
 nx = int(params["nx"]);
 ny = int(params["ny"]);
 nz = int(params["nz"]);
}

Crystal2DGenerator::~Crystal2DGenerator() { }

void Crystal2DGenerator::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = crystal2d                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to generate two-dimensional lattices.                \n";
 std::cout << "      The total number of atoms in the cell corresponds to nx*ny (see below).  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      a             : Sets the length of the first basis vector: (a,0,0).      \n";
 std::cout << "      b             : Sets the length of the second vector.                    \n";
 std::cout << "      gamma         : Sets the angle (in degrees) between a and b.             \n";
 std::cout << "      symbol        : Symbol of the atomic species.                            \n";
 std::cout << "      nx            : Sets the number of replications in the X direction.      \n";
 std::cout << "      ny            : Sets the number of replications in the Y direction.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input crystal2d a=1.0 b=1.0 gamma=45.0 symbol=Ar nx=2 ny=2                  \n\n";
 std::cout << "      The plugin is used to generate a two-dimensional lattice of argon atoms. \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Crystal2DGenerator::Generate(Configuration & conf) const
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 assert(atoms.Size() == 0);
 double ax = 1.0/double(nx);
 double ay = 1.0/double(ny);
 Vector baseA(ax,0,0);
 Vector baseB(ay*cos(gamma),ay*sin(gamma),0);
 DebugStream() << "-> Using vector a = ";
 FormattedWrite(DebugStream(), baseA);
 DebugStream() << '\n';
 DebugStream() << "-> Using vector b = ";
 FormattedWrite(DebugStream(), baseB);
 DebugStream() << '\n';
 long int cc = 0;
 for (long j=0;j<ny;++j)
  for (long i=0;i<nx;++i)
  {
   atoms.Append(Atom(spc));
   atoms[cc++].Position() = cell.FittedInside(cell.Cartesian(j*baseA + i*baseB));
  }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Crystal2DGenerator(args); }
void destroy(Plugin * m) { delete m; }

