//

//
//

#include <lpmd/atompair.h>
#include <lpmd/configuration.h>
#include <lpmd/metalpotential.h>
#include <lpmd/properties.h>
#include <runtime/runtime_context.h>
#include "forceaccumulator.h"

using namespace lpmd;

MetalPotential::MetalPotential() {
  rho = NULL;
  initial = true;
  du = 0.0e0;
  drho = 0.0e0;
  dvir = 0.0e0;
}

MetalPotential::MetalPotential(const MetalPotential& mp) {
  assert(&mp != 0);
  rho = NULL;
  initial = true;
  du = 0.0e0;
  drho = 0.0e0;
  dvir = 0.0e0;
} // icc 869

MetalPotential::~MetalPotential() {
  if (rho != NULL) {
    delete[] rho;
    rho = NULL;
  };
}

void MetalPotential::Initialize(Configuration& conf) {
  assert(&conf != 0); // icc 869
}

double MetalPotential::energy(Configuration& conf) {
  assert(&conf != 0);
  return energycache;
} // icc 869

double MetalPotential::AtomEnergy(Configuration& conf, long i) {
  double etmp = 0.0, rhoi = 0.0;
  NeighborList nlist;
  BasicParticleSet& atoms = conf.Atoms();
  conf.GetCellManager().BuildNeighborList(conf, i, nlist, true, GetCutoff());
  for (long k = 0; k < nlist.Size(); ++k) {
    rhoi += rhoij(sqrt(nlist[k].r2));
    const AtomPair& nn = nlist[k];
    if (AppliesTo(atoms[i].Z(), nn.j->Z()))
      etmp += pairEnergy(sqrt(nn.r2));
  }
  etmp += F(rhoi);
  return etmp;
}

void MetalPotential::UpdateForces(Configuration& conf) {
  const double forcefactor = double(conf.Context().session()["forcefactor"]);
  BasicParticleSet& atoms = conf.Atoms();
  const long n = atoms.Size();

  energycache = 0.0;
  double mpd = 0.0; // mean particle density
  conf.Cell().Fractional(Vector());
  const int thread_count = detail::ForceThreadCount(n);
  std::vector<detail::ForceAccumulator> partial;
  partial.reserve(thread_count);
  for (int thread = 0; thread < thread_count; ++thread)
    partial.emplace_back(n);

  delete[] rho;
  rho = new double[n];
  for (long i = 0; i < n; ++i) {
    rho[i] = 0.0e0;
  }

  // Load Corrections in the begining of the simulation.
  // Correction apply only for suttonchen and gupta poential.
  // and for highly homogeneous systems of this potentials.
  if (initial == true) {
    double sinv = 0.0e0;
    // mpd = atoms.Size()/conf.Cell().Volume();
    for (long i = 0; i < n; ++i) {
      double rhoi = 0.0e0;
      NeighborList nlist;
      conf.GetCellManager().BuildNeighborList(conf, i, nlist, true, GetCutoff());
      for (long k = 0; k < nlist.Size(); ++k) {
        rhoi += rhoij(sqrt(nlist[k].r2));
      }
      sinv += 1.0e0 / (sqrt(rhoi));
    }
    Vector tmp = UpdateCorrections(mpd, n, sinv);
    // du=tmp[1];
    // dvir=tmp[2];
    std::ostream& debug = conf.Context().session().DebugStream();
    debug << '\n';
    debug << "================================================================================"
          << '\n';
    debug << "=====================  METALLIC  CORRECTIONS.  ================================="
          << '\n';
    debug << "================================================================================"
          << '\n';

    debug << "Mean particle density = " << mpd << '\n';
    debug << "Sum 1/sqrt(rhoi)      = " << sinv << '\n';
    debug << "Delta rhoi            = " << tmp[0] << '\n';
    debug << "Delta U1              = " << tmp[1] << '\n';
    debug << "Delta Virial 1 + 2    = " << tmp[2] << '\n';
    initial = false;
  }

#ifdef _OPENMP
#pragma omp parallel num_threads(thread_count) if(thread_count > 1)
#endif
  {
    RuntimeContextScope context(conf.Context());
#ifdef _OPENMP
#pragma omp for schedule(static)
#endif
    for (long i = 0; i < n; ++i) {
      double rhoi = 0.0e0;
      NeighborList nlist;
      conf.GetCellManager().BuildNeighborList(conf, i, nlist, true, GetCutoff());
      for (long k = 0; k < nlist.Size(); ++k)
        rhoi += rhoij(sqrt(nlist[k].r2));
      rho[i] = rhoi + mpd;
    }
    // The implicit barrier above makes every density available to both
    // endpoints before their many-body forces are evaluated.
    auto& local = partial[detail::ForceThreadIndex()];
#ifdef _OPENMP
#pragma omp for schedule(static)
#endif
    for (long i = 0; i < n; ++i) {
      NeighborList nlist;
      conf.GetCellManager().BuildNeighborList(conf, i, nlist, false, GetCutoff());
      for (long k = 0; k < nlist.Size(); ++k) {
        const AtomPair& nn = nlist[k];
        if (AppliesTo(atoms[i].Z(), nn.j->Z())) {
          const double r = sqrt(nn.r2);
          Vector norm = nn.rij;
          norm.Normalize();
          local.energy += pairEnergy(r);
          const Vector force = PairForce(norm, r) + ManyBodies(norm, rho[i], rho[nn.j_index], r);
          local.AddPair(i, nn.j_index, nn.rij, force, forcefactor,
                        atoms[i].Mass(), nn.j->Mass());
        }
      }
      local.energy += F(rho[i]);
    }
  }
  detail::MergeForces(conf, partial, energycache);
  energycache += du;
  conf.Virial() += dvir;
}
