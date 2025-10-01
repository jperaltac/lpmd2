//
//
//

#include "osciforce.h"

#include <lpmd/simulation.h>
#include <lpmd/session.h>

#include <iostream>

using namespace lpmd;

OsciForcePotential::OsciForcePotential(std::string args): Plugin("osciforce", "1.0")
{ 
 //
 DefineKeyword("force");
 DefineKeyword("phase","0.0e0");
 DefineKeyword("n","10");
 DefineKeyword("counter","0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 force = Vector((*this)["force"].c_str());
 phase = double((*this)["phase"]);
 n = int((*this)["n"]);
 counter = int((*this)["counter"]);
}

OsciForcePotential::~OsciForcePotential() { }

void OsciForcePotential::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = osciforce                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to apply an external oscillating force over a set of \n";
 std::cout << "      atoms. This force F is added to the force that each atoms feels, i.e.,   \n";
 std::cout << "      the atom's acceleration is augmented (or diminished) by an extra term    \n";
 std::cout << "      F/m, where m is the mass of the atom. The form of this force is        \n\n";
 std::cout << "                           F = f * sin(2*pi*c/n + phase),                    \n\n";
 std::cout << "      where 'f' is a constant vector. This vector must be entered in           \n";
 std::cout << "      electrovols over angstrom (eV/A). The term 'c' is equal to the amount of \n";
 std::cout << "      times the plugin has been called. Each 'n' calls (steps), the force      \n";
 std::cout << "      completes a period.                                                    \n\n";
 std::cout << "      Note: For a given force F, the atoms won't accelarate at the same rate   \n";
 std::cout << "            if they have different masses. Moreover, the force gives an extra  \n";
 std::cout << "            energy to the system, then the energy will not be conserved.       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      force         : Sets the term 'f' in the force (in eV/angstrom).         \n";
 std::cout << "      phase         : Sets the term 'phase' in the force (dimensionless).      \n";
 std::cout << "      n             : Sets the term 'n' in the force (dimensionless).          \n";
 std::cout << "      counter       : Sets the term 'c' in the force (dimensionless).          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use osciforce as sinforce                                                     \n";
 std::cout << "     force <0.0,0.0,-0.1>                                                      \n";
 std::cout << "     phase 0                                                                   \n";
 std::cout << "     n 10                                                                      \n";
 std::cout << "     c 0                                                                       \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential sinforce Ar Ar                                                    \n\n";
 std::cout << "      The plugin is used to apply an oscillating force in the z-direction with \n";
 std::cout << "      a module of 0.1 eV/angstroms between argon (Ar) atoms. The force         \n";
 std::cout << "      oscillates between (0, 0, 0.1) eV/angstroms and (0, 0, -0.1) eV/angstroms,\n";
 std::cout << "      with a period of 10 steps.                                               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double OsciForcePotential::energy(Configuration & con) {assert(&con != 0); return 0.0; }//icc869

double OsciForcePotential::AtomEnergy(lpmd::Configuration & con, long i) { return 0.0; }

void OsciForcePotential::UpdateForces(Configuration & con) 
{ 
 lpmd::BasicParticleSet & atoms = con.Atoms();
 const double forcefactor = double(GlobalSession["forcefactor"]);
 lpmd::Vector tmpforce = force*sin(2*M_PI*(double(counter/n)) + phase);
 for (long int i=0;i<atoms.Size();++i)
 {
  double mi = atoms[i].Mass();
  atoms[i].Acceleration() = atoms[i].Acceleration() + tmpforce*(forcefactor/mi);
 }
 counter++;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new OsciForcePotential(args); }
void destroy(Plugin * m) { delete m; }

