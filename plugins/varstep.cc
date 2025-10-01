//
//
//

#include "varstep.h"

#include <lpmd/simulation.h>
#include <lpmd/session.h>

#include <iostream>

using namespace lpmd;

VariableStep::VariableStep(std::string args): Plugin("varstep", "2.1")
{
 DefineKeyword("dt", "1.0");
 DefineKeyword("scale", "0.15");
 DefineKeyword("start", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 dt = double((*this)["dt"]);
 scalefactor = double((*this)["scale"]);
 start = int((*this)["start"]);
 vhalf = NULL;
}

VariableStep::~VariableStep() 
{ 
 delete vhalf; 
 DebugStream() << "-> Total simulated time: " << elapsed_time << " fs\n";
}

void VariableStep::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = varstep                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to integrate the movement equations using the         \n";
 std::cout << "      variable step method.                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      dt            : Sets the time-step in femto-seconds for the integration  \n";
 std::cout << "                      step.                                                    \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use varstep                                                                   \n";
 std::cout << "     dt 1.0                                                                    \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " integrator varstep start=500                                                \n\n";
 std::cout << "      The plugin can be called at the beginning of the simulation (without the \n";
 std::cout << "      start option or setting start=0) or at any other time step (like         \n";
 std::cout << "      start=1000). This allows you to change integration method during the     \n";
 std::cout << "      simulation.                                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void VariableStep::Initialize(Simulation & sim, Potential & p) { assert(&p != 0); UseOldConfig(sim); }

void VariableStep::Advance(Simulation & sim, Potential & p)
{
 p.UpdateForces(sim);
 BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicCell & cell = sim.Cell();
 lpmd::BasicParticleSet & oldatoms = OldConfig().Atoms(); 
 const Vector aczero(0.0, 0.0, 0.0);

 long int i = 0;
 Atom at("H");
 // Setea a cero las aceleraciones de los atomos con fixedvel
 if (atoms.HaveAny(Tag("fixedvel"))) 
 {
  GlobalSession.DebugStream() << "-> Considering the fixedvel flag on some atoms\n";
  for (i=0;i<atoms.Size();++i)
  {
   at = atoms[i];
   if (atoms.Have(at, Tag("fixedvel"))) atoms[i].Acceleration() = aczero;
  }
 }

 // Compute vhalf
 if (vhalf == NULL)
 {
  vhalf = new Vector[atoms.Size()];
  elapsed_time = 0.0;
 }
 for (long int i=0;i<atoms.Size();++i)
     vhalf[i] = atoms[i].Velocity() + 0.5*curr_dt*atoms[i].Acceleration();

 // Find the maximal acceleration change
 double max_acc = -1.0, med_acc = 0.0;
 for (long int i=0;i<atoms.Size();++i)
 {
  double dacc = (atoms[i].Acceleration()-oldatoms[i].Acceleration()).Module();
  if (dacc > max_acc) max_acc = dacc;
  med_acc += atoms[i].Acceleration().Module();
 }
 med_acc /= double(atoms.Size());
 
 // do something with the timestep
 curr_dt = scalefactor*(med_acc/max_acc)*dt;
 if (atoms.HaveAny(Tag("fixedpos")))
 {
  GlobalSession.DebugStream() << "-> Considering the fixedpos flag on some atoms\n";
  for (i=0;i<atoms.Size();++i)
  {
   at = atoms[i];
   if (atoms.Have(at, Tag("fixedpos")) && (atoms.GetTag(at, Tag("fixedpos")) == "true")) continue;
   else 
   {
    Vector newpos = atoms[i].Position() + vhalf[i]*curr_dt;
    oldatoms[i].Acceleration() = atoms[i].Acceleration();
    atoms[i].Position() = cell.FittedInside(newpos);
   }
  }
 }
 else
 {
  for (i=0;i<atoms.Size();++i)
  {
   Vector newpos = atoms[i].Position() + vhalf[i]*curr_dt;
   oldatoms[i].Acceleration() = atoms[i].Acceleration();
   atoms[i].Position() = cell.FittedInside(newpos);
  }
 }

 p.UpdateForces(sim);

 // Setea a cero las aceleraciones de los atomos con fixedvel
 if (atoms.HaveAny(Tag("fixedvel"))) 
 {
  GlobalSession.DebugStream() << "-> Considering the fixedvel flag on some atoms\n";
  for (i=0;i<atoms.Size();++i)
  {
   at = atoms[i];
   if (atoms.Have(at, Tag("fixedvel"))) atoms[i].Acceleration() = aczero;
  }
 }

 if (atoms.HaveAny(Tag("fixedpos")))
 {
  GlobalSession.DebugStream() << "-> Considering the fixedpos flag on some atoms\n";
  for (i=0;i<atoms.Size();++i)
  {
   at = atoms[i];
   if (atoms.Have(at, Tag("fixedpos")) && (atoms.GetTag(at, Tag("fixedpos")) == "true")) continue;
   else
     atoms[i].Velocity() = vhalf[i] + 0.5*curr_dt*atoms[i].Acceleration();
  }
 }
 else
 {
  for (i=0;i<atoms.Size();++i)
     atoms[i].Velocity() = vhalf[i] + 0.5*curr_dt*atoms[i].Acceleration();
 }
 elapsed_time += curr_dt;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new VariableStep(args); }
void destroy(Plugin * m) { delete m; }

