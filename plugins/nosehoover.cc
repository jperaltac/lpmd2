//
//
//

#include "nosehoover.h"

#include <lpmd/simulation.h>
#include <lpmd/potential.h>
#include <lpmd/session.h>
#include <lpmd/properties.h>

using namespace lpmd;

NoseHoover::NoseHoover(std::string args): Plugin("nosehoover", "2.0")
{
 //
 DefineKeyword("fmass");
 DefineKeyword("dt", "1.0");
 DefineKeyword("start", "1");
 DefineKeyword("t", "300.0");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 dt = double((*this)["dt"]);
 start = int((*this)["start"]);
 q = double((*this)["fmass"]);
 temp = double((*this)["t"]);
}

NoseHoover::~NoseHoover() { }

void NoseHoover::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nosehoover                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements the Nose-Hoover thermostat method to integrate    \n";
 std::cout << "      the movement equations. This allows the user to make simulations in the  \n";
 std::cout << "      NVT ensemble.                                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      dt            : Sets the time-step in femto-seconds for the integration  \n";
 std::cout << "                      step.                                                    \n";
 std::cout << "      fmass         : Ficticius mass Q.                                        \n";
 std::cout << "      t             : Temperature of the reservoir.                            \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nosehoover                                                                \n";
 std::cout << "     dt 10.0                                                                   \n";
 std::cout << "     fmass 1.0                                                                 \n";
 std::cout << "     t 600.0                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator nosehoover start=1000                                            \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void NoseHoover::Initialize(Simulation & sim, Potential & p)
{
 for (long int i=0;i<sim.Atoms().Size();++i) auxlist.push_back(Vector());
 UseOldConfig(sim);
 Configuration & oldsc = OldConfig();
 p.UpdateForces(oldsc);
 friction = 0.0;
}

void NoseHoover::AdvancePosition(Simulation & sim, long i)
{
 const double kboltzmann = double(GlobalSession["kboltzmann"]);
 
 Configuration & oldsc = OldConfig();
 BasicCell & cell = sim.Cell();
 Atom now = sim.Atoms()[i];
 Atom old = oldsc.Atoms()[i];
 auxlist[i] = old.Acceleration();
 friction += ((3.0*sim.Atoms().Size()/q)*kboltzmann*(Temperature(sim.Atoms())-temp)*dt);
 const Vector newacc = now.Acceleration() - now.Velocity()*friction;
 sim.Atoms()[i].Acceleration() = newacc;
 Vector newpos = now.Position() + now.Velocity()*dt + (2.0/3.0)*newacc*dt*dt - (1.0/6.0)*old.Acceleration()*dt*dt;
 oldsc.Atoms()[i].Acceleration() =  newacc;
 sim.Atoms()[i].Position() = cell.FittedInside(newpos);
}

void NoseHoover::AdvanceVelocity(Simulation & sim, long i)
{
 Configuration & oldsc = OldConfig();
 Atom now = sim.Atoms()[i];
 Atom old = oldsc.Atoms()[i];
 sim.Atoms()[i].Velocity() = now.Velocity() + (1.0/3.0)*now.Acceleration()*dt + (5.0/6.0)*old.Acceleration()*dt - (1.0/6.0)*auxlist[i]*dt;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new NoseHoover(args); }
void destroy(Plugin * m) { delete m; }
