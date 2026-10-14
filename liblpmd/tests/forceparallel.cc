#include <lpmd/metalpotential.h>
#include <lpmd/pairpotential.h>
#include <lpmd/storedconfiguration.h>
#include <runtime/runtime_context.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace lpmd;

namespace {
constexpr double cutoff = 2.75;
constexpr double stiffness = 0.31;

class ReferenceNeighbors : public CellManager {
public:
  void Reset() override {}
  void UpdateCell(Configuration&) override {}
  void UpdateAtom(Configuration&, long) override {}
  void BuildNeighborList(Configuration& conf, long i, NeighborList& list,
                         bool full, double radius) override {
    list.Clear();
    for (long j = 0; j < conf.Atoms().Size(); ++j) {
      if (i == j || (!full && j > i))
        continue;
      AtomPair pair;
      pair.i = &conf.Atoms()[i];
      pair.j = &conf.Atoms()[j];
      pair.i_index = i;
      pair.j_index = j;
      pair.rij = conf.Cell().Displacement(pair.i->Position(), pair.j->Position());
      pair.r2 = pair.rij.SquareModule();
      if (pair.r2 < radius * radius)
        list.Append(pair);
    }
  }
};

class HarmonicPairs : public PairPotential {
public:
  double pairEnergy(const double& r) const override {
    // The callback verifies that the worker inherited the runtime context.
    (void)CurrentContext();
    return 0.5 * stiffness * r * r;
  }
  Vector pairForce(const Vector& r) const override { return stiffness * r; }
};

class EmbeddedPairs : public MetalPotential {
public:
  double pairEnergy(const double& r) const override { return 0.5 * stiffness * r * r; }
  double rhoij(const double& r) const override {
    (void)CurrentContext();
    return std::exp(-r);
  }
  double F(const double& density) const override { return 0.5 * density * density; }
  Vector PairForce(const Vector& norm, const double& r) const override {
    return norm * (stiffness * r);
  }
  Vector ManyBodies(const Vector& norm, const double& rhoi, const double& rhoj,
                    const double& r) const override {
    return norm * (-(rhoi + rhoj) * std::exp(-r));
  }
  Vector UpdateCorrections(const double&, const int&, const double&) const override { return Vector(); }
};

void Check(double actual, double expected, const char* quantity) {
  if (!std::isfinite(actual) || std::abs(actual - expected) > 1.0e-11 * std::max(1.0, std::abs(expected)))
    throw std::runtime_error(std::string(quantity) + " differs from the independent serial reference");
}

void CheckPotential(Potential& potential, bool embedded, int threads) {
#ifdef _OPENMP
  omp_set_dynamic(0);
  omp_set_num_threads(threads);
#else
  (void)threads;
#endif
  RuntimeContext context;
  RuntimeContextScope scope(context);
  StoredConfiguration conf;
  conf.SetRuntimeContext(context);
  conf.Cell()[0] = Vector(16.0, 0.0, 0.0);
  conf.Cell()[1] = Vector(1.2, 16.0, 0.0);
  conf.Cell()[2] = Vector(0.8, 0.4, 16.0);
  for (int k = 0; k < 3; ++k)
    for (int j = 0; j < 4; ++j)
      for (int i = 0; i < 4; ++i) {
        Atom atom("Ar");
        atom.Position() = Vector(2.0 + 1.21 * i + 0.06 * j, 2.0 + 1.27 * j, 2.0 + 1.32 * k);
        atom.Mass() = 30.0 + i + 2 * j + k;
        conf.Atoms().Append(atom);
      }
  ReferenceNeighbors neighbors;
  conf.SetCellManager(neighbors);
  potential.SetCutoff(cutoff);
  potential.SetValidSpecies(-1, -1);
  potential.Initialize(conf);

  const long n = conf.Atoms().Size();
  const double factor = context.session()["forcefactor"];
  std::vector<double> density(n, 0.0);
  std::vector<Vector> expected_acceleration(n, Vector(0.003, -0.002, 0.001));
  double expected_energy = 0.0, expected_virial = 0.75;
  double expected_stress[3][3];
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      expected_stress[row][col] = 0.05 * (row + 1) * (col + 1);

  // Compute all quantities directly, without the threaded implementation or
  // its accumulator helper. This also checks both endpoints and tensor axes.
  conf.Cell().Fractional(Vector());
  if (embedded) {
    for (long i = 0; i < n; ++i)
      for (long j = 0; j < n; ++j) {
        if (i == j)
          continue;
        const double r = conf.Cell().Displacement(conf.Atoms()[i].Position(), conf.Atoms()[j].Position()).Module();
        if (r < cutoff)
          density[i] += std::exp(-r);
      }
    for (double rho : density)
      expected_energy += 0.5 * rho * rho;
  }
  for (long i = 0; i < n; ++i)
    for (long j = 0; j < i; ++j) {
      const Vector rij = conf.Cell().Displacement(conf.Atoms()[i].Position(), conf.Atoms()[j].Position());
      const double r = rij.Module();
      if (r >= cutoff)
        continue;
      expected_energy += 0.5 * stiffness * r * r;
      Vector force = stiffness * rij;
      if (embedded)
        force -= rij * ((density[i] + density[j]) * std::exp(-r) / r);
      expected_acceleration[i] += force * (factor / conf.Atoms()[i].Mass());
      expected_acceleration[j] -= force * (factor / conf.Atoms()[j].Mass());
      expected_virial -= Dot(rij, force);
      for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
          expected_stress[row][col] -= rij[row] * force[col];
    }

  for (int repeat = 0; repeat < 3; ++repeat) {
    for (long i = 0; i < n; ++i)
      conf.Atoms()[i].Acceleration() = Vector(0.003, -0.002, 0.001);
    conf.Virial() = 0.75;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        conf.StressTensor().Set(row, col, 0.05 * (row + 1) * (col + 1));
    // Invalidate the transform cache to exercise the serial warm-up.
    conf.Cell()[0] = Vector(16.0, 0.0, 0.0);
    potential.UpdateForces(conf);
    Check(potential.energy(conf), expected_energy, "energy");
    Check(conf.Virial(), expected_virial, "virial");
    for (long i = 0; i < n; ++i)
      for (int axis = 0; axis < 3; ++axis)
        Check(conf.Atoms()[i].Acceleration()[axis], expected_acceleration[i][axis], "acceleration");
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        Check(conf.StressTensor().Get(row, col), expected_stress[row][col], "stress");
  }
}
} // namespace

int main() {
  try {
    for (int threads : {1, 2, 4}) {
      HarmonicPairs pair;
      CheckPotential(pair, false, threads);
      EmbeddedPairs metal;
      CheckPotential(metal, true, threads);
    }
    std::cout << "Pair and metallic forces, energy, virial, and stress match the serial reference.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
