//
//
//

#include "density.h"

#include <lpmd/util.h>

#include <iostream>
#include <lpmd/session.h>

using namespace lpmd;

DensityModifier::DensityModifier(std::string args): Plugin("density", "1.0")
{
 ParamList & params = (*this);
 DefineKeyword("type", "hydro");
 DefineKeyword("density");
 // 
 ProcessArguments(args);
 type = params["type"];
 density = double(params["density"]);
}

DensityModifier::~DensityModifier() { }

void DensityModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = density                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to change the density of an original structure.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      type          : Type of cell axis modification. The availables are:      \n";
 std::cout << "                      info  - Only show information about the density.         \n";
 std::cout << "                      hydro - Hydrostatic modification (all axis).             \n";
 std::cout << "                      x,y,z - modify only one axis of the cell.                \n";
 std::cout << "                      xy,yz,xz - modify the two axis only.                     \n";
 std::cout << "      density       : A double number with the objective density.              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " prepare density type=hydro density=6.4                                      \n\n";
 std::cout << "      The plugin is used to change the density of the structure to 6.4 gr/cm^3 \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void DensityModifier::Apply(Simulation & conf)
{
 const double df = double(GlobalSession["ua3togrcm3"]);
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 if(density<=0 && type!="info") throw PluginError("density","Error in density value, must be positive and non-zero.");

 double mass = 0.0e0;
 for(int i=0;i<atoms.Size();++i)
 {
  mass += atoms[i].Mass();
 }
 double dens = df*mass/cell.Volume();
 if(type == "info")
 {
  std::cout << "\n\nOriginal density = " << dens << "gr/cm^3" << '\n';
  std::cout << "Number of atoms = " << atoms.Size() << '\n';
 }
 //Change the density of the cell.
 else if (type == "hydro")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    for (int j=0;j<3;++j) cell[j] = cell[j] + cell[j]*0.01;
   }
   else if(density > dens)
   {
    for (int j=0;j<3;++j) cell[j] = cell[j] - cell[j]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "x" || type == "X")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    cell[0] = cell[0] + cell[0]*0.01;
   }
   else if(density > dens)
   {
    cell[0] = cell[0] - cell[0]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "y" || type == "Y")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    cell[0] = cell[0] + cell[0]*0.01;
   }
   else if(density > dens)
   {
    cell[0] = cell[0] - cell[0]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "z" || type == "Z")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    cell[0] = cell[0] + cell[0]*0.01;
   }
   else if(density > dens)
   {
    cell[0] = cell[0] - cell[0]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "xy" || type == "XY")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    for (int j=0;j<2;++j) cell[j] = cell[j] + cell[j]*0.01;
   }
   else if(density > dens)
   {
    for (int j=0;j<2;++j) cell[j] = cell[j] - cell[j]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "yz" || type == "YZ")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    for (int j=1;j<3;++j) cell[j] = cell[j] + cell[j]*0.01;
   }
   else if(density > dens)
   {
    for (int j=1;j<3;++j) cell[j] = cell[j] - cell[j]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else if (type == "xz" || type == "XZ")
 {
  while(fabs(density-dens)>1E-2)
  {
   if(density < dens)
   {
    cell[0] = cell[0] + cell[0]*0.01;
    cell[2] = cell[2] + cell[2]*0.01;
   }
   else if(density > dens)
   {
    cell[0] = cell[0] - cell[0]*0.01;
    cell[2] = cell[2] - cell[2]*0.01;
   }
   dens = df*mass/cell.Volume();
  }
 }
 else
 {
  throw PluginError("density","Bad definition of the type of change.");
 }
 std::cout << "Final density = " << dens << '\n';
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new DensityModifier(args); }
void destroy(Plugin * m) { delete m; }

