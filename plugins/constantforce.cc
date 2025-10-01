//
//
//

#include "constantforce.h"

#include <lpmd/simulation.h>
#include <lpmd/session.h>

#include <iostream>

using namespace lpmd;

ConstantForcePotential::ConstantForcePotential(std::string args): Plugin("constantforce", "2.0")
{ 
 //
 DefineKeyword("force");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 force = Vector((*this)["force"].c_str());
}

ConstantForcePotential::~ConstantForcePotential() { }

void ConstantForcePotential::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = constantforce                                            \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to apply an external constant force over a set of    \n";
 std::cout << "      atoms. This force F is added to the force that each atoms feels, i.e.,   \n";
 std::cout << "      the atom's acceleration is augmented (or diminished) by an extra term    \n";
 std::cout << "      F/m, where m is the mass of the atom. This force must be entered in      \n";
 std::cout << "      electrovols over angstrom (eV/A).                                      \n\n";
 std::cout << "      Note: For a given force F, the atoms won't accelarate at the same rate   \n";
 std::cout << "            if they have different masses. Moreover, the force gives an extra  \n";
 std::cout << "            energy to the system, then the energy will not be conserved.       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      forcevector   : Additional force (in eV/angstrom) to be applied over     \n";
 std::cout << "                      each atom.                                               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use constantforce as gravity                                                  \n";
 std::cout << "     force <0.0,0.0,-4.06E-16>                                                 \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential gravity Ar Ar                                                     \n\n";
 std::cout << "      The plugin is used to apply a constant force of -4.06x10^16 eV/angstrom  \n";
 std::cout << "      in the z direction over argon atoms. This force corresponds to the       \n";
 std::cout << "      acceleration of gravity g=9.8 m/s^2 multiplied by the mass of 1 argon    \n";
 std::cout << "      atom (39.948 amu) which, as you can see, is negligible.                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double ConstantForcePotential::energy(Configuration & con) {assert(&con != 0); return 0.0; }//icc 869

double ConstantForcePotential::AtomEnergy(lpmd::Configuration & con, long i) { return 0.0; }

void ConstantForcePotential::UpdateForces(Configuration & con) 
{ 
 lpmd::BasicParticleSet & atoms = con.Atoms();
 const double forcefactor = double(GlobalSession["forcefactor"]);
 long count = 0;
 for (long int i=0;i<atoms.Size();++i)
 {
  if ((atoms.Have(atoms[i], Tag("noconstantforce"))) && (atoms.GetTag(atoms[i], Tag("noconstantforce")) == "true")) continue;
  double mi = atoms[i].Mass();
  atoms[i].Acceleration() = atoms[i].Acceleration() + force*(forcefactor/mi);
  count++;
 }
 DebugStream() << "-> Applied constant force <";
 DebugStream() << force[0] << ", " << force[1] << ", " << force[2];
 DebugStream() << "> to " << count << " atoms" << '\n';
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ConstantForcePotential(args); }
void destroy(Plugin * m) { delete m; }

