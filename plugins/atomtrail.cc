/*
 *
 *
 *
 */

#include "atomtrail.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/atompair.h>
#include <lpmd/configuration.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace lpmd;

AtomTrail::AtomTrail(std::string args): Plugin("atomtrail", "1.0")
{
 ParamList & params = (*this);

 DefineKeyword("nx", "50");
 DefineKeyword("ny", "50");
 DefineKeyword("nz", "50");
 DefineKeyword("mode", "2D");
 DefineKeyword("plane", "XY");
 DefineKeyword("species", "all");
 DefineKeyword("debug", "none");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 
 ProcessArguments(args);

 nx = int(params["nx"]);
 ny = int(params["ny"]);
 nz = int(params["nz"]);
 mode = params["mode"];
 plane = params["plane"];
 species = params["species"];
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 OutputFile() = params["output"];
}

AtomTrail::~AtomTrail() { }

void AtomTrail::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = atomtrail                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin determines the atom-trail of the atoms in the simulation.    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      nx            : Number of subdivisions in x axis.                        \n";
 std::cout << "      ny            : Number of subdivisions in y axis.                        \n";
 std::cout << "      nz            : Number of subdivisions in z axis.                        \n";
 std::cout << "      mode          : Sets the the dimensions you want to consider (2D / 3D).  \n";
 std::cout << "      plane         : If mode is 2D then you have to write in what plane you   \n";
 std::cout << "                      want to analyze (xy/yz/xz).                              \n";
 std::cout << "      species       : You can choose a particular species.                     \n";
 std::cout << "      output        : Name of the ouput file.                                  \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use atomtrail as at                                                           \n";
 std::cout << "     output trail2d.dat                                                        \n";
 std::cout << "     nx 100                                                                    \n";
 std::cout << "     ny 100                                                                    \n";
 std::cout << "     plane XY                                                                  \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Using the loaded plugin :                                                    \n";
 std::cout << " property at start=0 end=-1 each=1                                           \n\n";
 std::cout << "      The plugin is used to find the atom's trail in the XY plane. The data is \n";
 std::cout << "      written in the file trail2d.dat.                                         \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void AtomTrail::Evaluate(Configuration & conf, Potential & pot)
{
 assert(&pot != 0);//icc 869
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 int pl1 = -1, pl2 = -1;
 if (mode == "2D")
 {
  if ((plane == "XY") || (plane == "xy")) { pl1 = 0; pl2 = 1; }
  else if ((plane == "XZ") || (plane == "xz")) { pl1 = 0; pl2 = 2; }
  else if ((plane == "YZ") || (plane == "yz")) { pl1 = 1; pl2 = 2; }
  else throw PluginError("atomtrail", "plane must be one of: XY, XZ, YZ");
 }

 double * trail = 0;
 if (mode == "2D")
 { 
  trail = new double[nx*ny]; 
  for (int i=0;i<ny;++i)
    for (int j=0;j<nx;++j) trail[i*nx+j] = 0.0;
 }
 else if (mode == "3D") 
 {
  trail = new double[nx*ny*nz]; 
  for (int i=0;i<nz;++i)
    for (int j=0;j<ny;++j)
      for (int k=0;k<nx;++k) trail[i*ny*nx+j*nx+k] = 0.0;
 }
 else throw PluginError("atomtrail", "Unknown mode, "+mode);

 long int cnt = 0;
 for (long int a=0;a<atoms.Size();++a)
 {
  if ((species == "all") || (species == atoms[a].Symbol()))
  {
   Vector fpos = cell.Fractional(atoms[a].Position());
   if (mode == "2D")
   {
    int i = int(floor((ny-1)*fpos[pl2]));
    int j = int(floor((nx-1)*fpos[pl1]));
    trail[i*nx+j] += 1.0;
   }
   else
   {
    int i = int(floor((nz-1)*fpos[2]));
    int j = int(floor((ny-1)*fpos[1]));
    int k = int(floor((nx-1)*fpos[0]));
    trail[i*ny*nx+j*nx+k] += 1.0;
   }
   cnt++;
  }
 }

 //
 // Output 
 //
 Matrix & m = CurrentValue();
 if (mode == "2D") m = Matrix(3, nx*ny);
 else m = Matrix(4, nx*ny*nz);

 m.SetLabel(0, "bin_x");
 m.SetLabel(1, "bin_y");
 if (mode == "2D") m.SetLabel(2, "count");
 else { m.SetLabel(2, "bin_z"); m.SetLabel(3, "count"); }

 long int n = 0;
 if (mode == "2D")
 {
  for (int i=0;i<ny;++i)
    for (int j=0;j<nx;++j)
    {
     m.Set(0, n, j);
     m.Set(1, n, i);
     m.Set(2, n, trail[i*nx+j]/double(cnt));
     n++;
    }
 }
 else
 {
  for (int i=0;i<nz;++i)
    for (int j=0;j<ny;++j)
      for (int k=0;k<nx;++k)
      {
       m.Set(0, n, k);
       m.Set(1, n, j);
       m.Set(2, n, i);
       m.Set(3, n, trail[i*ny*nx+j*nx+k]/double(cnt));
       n++;
      }
 }
 delete [] trail;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AtomTrail(args); }
void destroy(Plugin * m) { delete m; }

