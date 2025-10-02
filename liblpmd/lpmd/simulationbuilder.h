/*
 *
 *
 *
 */

#ifndef __LPMD_SIMULATION_BUILDER_H__
#define __LPMD_SIMULATION_BUILDER_H__

#include <runtime/runtime_context.h>
#include <lpmd/simulation.h>
#include <lpmd/basicparticleset.h>

namespace lpmd
{

 class SimulationBuilder
 {
  public:
    SimulationBuilder();
    ~SimulationBuilder();

    static Simulation & CreateFixedOrthogonal(RuntimeContext & context, long int atoms, const BasicAtom & at);
    static Simulation & CreateGeneric(RuntimeContext & context, long int atoms, const BasicAtom & at);
    static Simulation & CreateGeneric(RuntimeContext & context);
    static Simulation & CloneOptimized(RuntimeContext & context, const Simulation & sim);
  
  private:
   static class SimBuildImpl impl;
 };

}  // lpmd

#endif

