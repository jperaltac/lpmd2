#ifndef LPMD_FORCEACCUMULATOR_H
#define LPMD_FORCEACCUMULATOR_H

#include <lpmd/configuration.h>
#include <algorithm>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace lpmd {
namespace detail {

inline int ForceThreadCount(long atoms) {
#ifdef _OPENMP
  return static_cast<int>(std::min<long>(omp_get_max_threads(), std::max<long>(atoms, 1)));
#else
  (void)atoms;
  return 1;
#endif
}

inline int ForceThreadIndex() {
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

// A thread owns its entire buffer, including both endpoints of every pair.
// This avoids locks inside the neighbor loop and races on the stress tensor.
struct ForceAccumulator {
  explicit ForceAccumulator(long atoms) : acceleration(atoms), energy(0.0), virial(0.0), stress{} {}

  void AddPair(long i, long j, const Vector& rij, const Vector& force,
               double factor, double imass, double jmass) {
    acceleration[i] += force * (factor / imass);
    acceleration[j] -= force * (factor / jmass);
    virial -= Dot(rij, force);
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        stress[row][col] -= rij[row] * force[col];
  }

  std::vector<Vector> acceleration;
  double energy;
  double virial;
  double stress[3][3];
};

inline void MergeForces(Configuration& conf, const std::vector<ForceAccumulator>& partial,
                        double& energy) {
  BasicParticleSet& atoms = conf.Atoms();
  const long n = atoms.Size();
  const int thread_count = static_cast<int>(partial.size());
#ifdef _OPENMP
#pragma omp parallel for num_threads(thread_count) if(thread_count > 1) schedule(static)
#endif
  for (long i = 0; i < n; ++i) {
    Vector acceleration;
    for (const auto& thread : partial)
      acceleration += thread.acceleration[i];
    atoms[i].Acceleration() += acceleration;
  }

  for (const auto& thread : partial) {
    energy += thread.energy;
    conf.Virial() += thread.virial;
    Matrix& stress = conf.StressTensor();
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        stress.Set(row, col, stress.Get(row, col) + thread.stress[row][col]);
  }
}

} // namespace detail
} // namespace lpmd

#endif
