//
//
//

#include <lpmd/atompair.h>
#include <lpmd/matrix.h>
#include <lpmd/pairpotential.h>
#include <runtime/runtime_context.h>
#include "forceaccumulator.h"

using namespace lpmd;

PairPotential::PairPotential() {}

PairPotential::~PairPotential() {}

double PairPotential::energy(Configuration& conf) {
  assert(&conf != 0);
  return energycache;
} // icc 869

double PairPotential::AtomEnergy(Configuration& conf, long i) {
  double etmp = 0.0;
  NeighborList nlist;
  const double cutoff = GetCutoff();
  BasicParticleSet& atoms = conf.Atoms();
  conf.GetCellManager().BuildNeighborList(conf, i, nlist, true, cutoff);
  for (long k = 0; k < nlist.Size(); ++k) {
    AtomPair nn = nlist[k];
    if (AppliesTo(atoms[i].Z(), nn.j->Z()) && nn.r2 < cutoff * cutoff)
      etmp += pairEnergy(sqrt(nn.r2));
  }
  return etmp;
}

void PairPotential::UpdateForces(Configuration& conf) {
  const double forcefactor = conf.Context().session()["forcefactor"];
  BasicParticleSet& atoms = conf.Atoms();
  const long int n = atoms.Size();
  energycache = 0.0;
  const double cutoff = GetCutoff();
  // Some cells lazily cache Cartesian/fractional transforms. Initialize the
  // cache before several neighbor queries access it concurrently.
  conf.Cell().Fractional(Vector());
  const int thread_count = detail::ForceThreadCount(n);
  std::vector<detail::ForceAccumulator> partial;
  partial.reserve(thread_count);
  for (int thread = 0; thread < thread_count; ++thread)
    partial.emplace_back(n);

#ifdef _OPENMP
#pragma omp parallel num_threads(thread_count) if(thread_count > 1)
#endif
  {
    RuntimeContextScope context(conf.Context());
    auto& local = partial[detail::ForceThreadIndex()];
#ifdef _OPENMP
#pragma omp for schedule(static)
#endif
    for (long i = 0; i < n; ++i) {
      NeighborList nlist;
      conf.GetCellManager().BuildNeighborList(conf, i, nlist, false, cutoff);
      for (long k = 0; k < nlist.Size(); ++k) {
        const AtomPair& nn = nlist[k];
        if (AppliesTo(atoms[i].Z(), nn.j->Z()) && nn.r2 < cutoff * cutoff) {
          local.energy += pairEnergy(sqrt(nn.r2));
          local.AddPair(i, nn.j_index, nn.rij, pairForce(nn.rij), forcefactor,
                        atoms[i].Mass(), nn.j->Mass());
        }
      }
    }
  }
  detail::MergeForces(conf, partial, energycache);
}
