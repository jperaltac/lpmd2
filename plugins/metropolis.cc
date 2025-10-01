//
//
//

#include "metropolis.h"

#include <lpmd/simulation.h>
#include <lpmd/properties.h>
#include <lpmd/session.h>

#include <cmath>
#include <iostream>

using namespace lpmd;

Metropolis::Metropolis(std::string args): Plugin("metropolis", "1.0")
{
 DefineKeyword("start", "1");
 DefineKeyword("temp","0.0");
 DefineKeyword("percent","5");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int((*this)["start"]);
 Temp = double((*this)["temp"]);
 percent = double((*this)["percent"]);
}

Metropolis::~Metropolis() { }

void Metropolis::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = metropolis                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the equations of movement using the      \n";
 std::cout << "      metropolis method.                                                       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      temp          : Sets the external temperature.                         . \n";
 std::cout << "      percent       : Sets the percent of randomness for the random vector     \n";
 std::cout << "                      that is going to be added to the atom's position.        \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use metropolis                                                                \n";
 std::cout << "     temp 300.0                                                                \n";
 std::cout << "     percent 10                                                                \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator metropolis start=1000                                            \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Metropolis::Initialize(Simulation & sim, Potential & p) { assert(&p != 0); UseOldConfig(sim); }//icc 869

void Metropolis::Advance(Simulation & sim, long i)
{
 const double kboltzmann = double(GlobalSession["kboltzmann"]);
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::CombinedPotential & pots = sim.Potentials();
 lpmd::BasicCell & cell = sim.Cell();
 double penergy = pots.AtomEnergy(sim, i);
 double min = sim.MinimumPairDistance();
 Vector oldpos = atoms[i].Position();
 Vector random = RandomVector(min*percent/100);
 Vector newpos = cell.FittedInside(atoms[i].Position() + random);
 atoms[i].Position() = newpos;
 sim.GetCellManager().UpdateAtom(sim, i);
 double npe = pots.AtomEnergy(sim, i);
 if (npe < penergy) atoms[i].Position() = newpos;
 double r = exp(-(npe-penergy)/(kboltzmann*Temp));
 if (drand48() < r) atoms[i].Position() = newpos;
 else atoms[i].Position() = oldpos;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Metropolis(args); }
void destroy(Plugin * m) { delete m; }

