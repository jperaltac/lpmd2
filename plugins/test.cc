/*
 * Regression plugin that exercises common container operations to verify that
 * the Simulation and particle set APIs behave as expected.  The previous
 * version relied on legacy direct/indirect arrays that no longer exist in the
 * modern code base, which caused compilation to fail.  The implementation below
 * sticks to the current `BasicParticleSet` interface instead.
 */

#include "test.h"

#include <lpmd/atom.h>
#include <lpmd/basicparticleset.h>
#include <lpmd/timer.h>
#include <lpmd/util.h>

#include <iostream>
#include <utility>

using lpmd::BasicParticleSet;
using lpmd::Simulation;

Test::Test(std::string args) : Plugin("test", "1.1"), iterations_(10), atoms_per_iteration_(10000) {
  DefineKeyword("iterations", "10");
  DefineKeyword("atoms", "10000");
  ProcessArguments(args);

  iterations_ = int((*this)["iterations"]);
  atoms_per_iteration_ = static_cast<long>(int((*this)["atoms"]));
}

Test::~Test() = default;

void Test::ShowHelp() const {
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Module Name        = test                                                     \n";
  std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Info      >>                                                          \n";
  std::cout << "      Utility plugin that stresses the particle container implementation to    \n";
  std::cout << "      make sure append, clear and iteration routines remain functional.        \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Options   >>                                                          \n";
  std::cout << "      iterations    : Number of warm-up loops executed by PerformTest.         \n";
  std::cout << "      atoms         : Number of atoms appended per iteration.                  \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Test::PerformTest(Simulation& sim) {
  BasicParticleSet& atoms = sim.Atoms();
  atoms.Clear();

  const lpmd::Atom prototype("Cu");
  lpmd::Timer timer;

  std::cout << "Running container stress test with " << atoms_per_iteration_
            << " atoms per iteration and " << iterations_ << " iterations.\n";

  for (int iteration = 0; iteration < iterations_; ++iteration) {
    timer.Start();
    for (long idx = 0; idx < atoms_per_iteration_; ++idx) {
      atoms.Append(prototype);
      atoms[idx].Position() = lpmd::RandomVector(1.0);
    }
    timer.Stop();
    timer.ShowElapsedTimes();
    atoms.Clear();
  }
}

// Factory hooks for the dynamic loader
extern "C" lpmd::Plugin* create(std::string args) { return new Test(std::move(args)); }
extern "C" void destroy(lpmd::Plugin* plugin) { delete plugin; }
