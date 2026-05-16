//
//
//

#include "minimumimage.h"
#include <lpmd/configuration.h>

#include <cmath>
#include <cstdint>
#include <vector>

#ifdef LPMD_ENABLE_RUST_HOTSPOTS
#include <lpmd/rust_hotspots.h>
#endif

using namespace lpmd;

MinimumImageCellManager::MinimumImageCellManager(std::string args) : Plugin("minimumimage", "2.0") {
#if defined(LPMD_ENABLE_RUST_HOTSPOTS) && !defined(_OPENMP)
  rust_last_center_index = -1;
  rust_cached_atom_count = 0;
  rust_sequence_active = false;
#endif
  ParamList& params = (*this);
  //
  DefineKeyword("cutoff", "0.0");
  // hasta aqui los valores por omision
  ProcessArguments(args);
  rcut = params["cutoff"];
}

MinimumImageCellManager::~MinimumImageCellManager() {}

void MinimumImageCellManager::ShowHelp() const {
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Module Name        = minimumimage                                             \n";
  std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Info      >>                                                          \n";
  std::cout << "      This plugin implements the minimum image method for making neighbors     \n";
  std::cout << "      lists.It is one of the available cell managers.                          \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Options   >>                                                          \n";
  std::cout << "      cutoff        : Sets the cutoff radius for the evaluation.               \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Example           >>                                                          \n";
  std::cout << " #Loading the plugin :                                                         \n";
  std::cout << " use minimumimage                                                              \n";
  std::cout << "     cutoff 8.5                                                                \n";
  std::cout << " enduse                                                                        \n";
  std::cout << " #Applying the plugin :                                                        \n";
  std::cout << " cellmanager minimumimage                                                    \n\n";
  std::cout << "      The plugin is used to select the minimumimage method for making the lists\n";
  std::cout << "      of atoms' neighbors.                                                     \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void MinimumImageCellManager::Show(std::ostream& os) const {
  Module::Show(os);
  if (fabs(rcut) < 1E-10)
    os << "   No cutoff was defined." << '\n';
}

void MinimumImageCellManager::Reset() {
#if defined(LPMD_ENABLE_RUST_HOTSPOTS) && !defined(_OPENMP)
  rust_positions.clear();
  rust_indices.clear();
  rust_rij.clear();
  rust_r2.clear();
  rust_last_center_index = -1;
  rust_cached_atom_count = 0;
  rust_sequence_active = false;
#endif
}

void MinimumImageCellManager::UpdateCell(Configuration& conf) {
  BasicCell& cell = conf.Cell();
  if (fabs(rcut) < 1E-10) {
    for (int q = 0; q < 3; ++q)
      if (0.5 * cell[q].Module() > rcut)
        rcut = 0.5 * cell[q].Module();
  }
}

void MinimumImageCellManager::UpdateAtom(Configuration& conf, long i) { UpdateCell(conf); }

double MinimumImageCellManager::Cutoff() const { return rcut; }

namespace {
void BuildNeighborListCpp(Configuration& conf, long i, NeighborList& nlist, bool full, double rcu) {
  BasicParticleSet& atoms = conf.Atoms();
  BasicCell& cell = conf.Cell();
  const long int n = atoms.Size();
  nlist.Clear();
  AtomPair nn;
  nn.i = &atoms[i];
  nn.i_index = i;
  const Vector& ipos = atoms[i].Position();
  if (full) {
    for (long int j = 0; j < n; ++j)
      if (i != j) {
        nn.j = &(atoms[j]);
        nn.rij = cell.Displacement(ipos, nn.j->Position());
        nn.r2 = nn.rij.SquareModule();
        nn.j_index = j;
        if (nn.r2 < rcu * rcu)
          nlist.Append(nn);
      }
  } else {
    for (long int j = i + 1; j < n; ++j) {
      nn.j = &(atoms[j]);
      nn.rij = cell.Displacement(ipos, nn.j->Position());
      nn.r2 = nn.rij.SquareModule();
      nn.j_index = j;
      if (nn.r2 < rcu * rcu)
        nlist.Append(nn);
    }
  }
}
} // namespace

void MinimumImageCellManager::BuildNeighborList(Configuration& conf, long i, NeighborList& nlist,
                                                bool full, double rcu) {
  if (rcu < 1.0E-10)
    EndWithError("MinimumImage cutoff equals zero...");

#ifdef LPMD_ENABLE_RUST_HOTSPOTS
  BasicParticleSet& atoms = conf.Atoms();
  BasicCell& cell = conf.Cell();
  const long int n = atoms.Size();

  if (cell.IsOrthogonal() && i >= 0 && i < n) {
#if defined(_OPENMP)
    std::vector<double> positions(static_cast<std::size_t>(n) * 3);
    for (long int atom_index = 0; atom_index < n; ++atom_index) {
      const Vector& position = atoms[atom_index].Position();
      const std::size_t offset = static_cast<std::size_t>(atom_index) * 3;
      positions[offset] = position[0];
      positions[offset + 1] = position[1];
      positions[offset + 2] = position[2];
    }
    const double* positions_data = positions.data();
#else
    const bool can_reuse_positions =
        rust_sequence_active && rust_cached_atom_count == n && i == rust_last_center_index + 1;
    if (!can_reuse_positions) {
      rust_positions.resize(static_cast<std::size_t>(n) * 3);
      for (long int atom_index = 0; atom_index < n; ++atom_index) {
        const Vector& position = atoms[atom_index].Position();
        const std::size_t offset = static_cast<std::size_t>(atom_index) * 3;
        rust_positions[offset] = position[0];
        rust_positions[offset + 1] = position[1];
        rust_positions[offset + 2] = position[2];
      }
      rust_cached_atom_count = n;
      rust_sequence_active = (i == 0);
    }
    const double* positions_data = rust_positions.data();
#endif

    double cell_lengths[3] = {cell[0][0], cell[1][1], cell[2][2]};
    std::uint8_t periodic[3] = {static_cast<std::uint8_t>(cell.Periodicity(0)),
                                static_cast<std::uint8_t>(cell.Periodicity(1)),
                                static_cast<std::uint8_t>(cell.Periodicity(2))};
    const std::size_t capacity = full ? (n > 0 ? static_cast<std::size_t>(n - 1) : 0)
                                      : (i + 1 < n ? static_cast<std::size_t>(n - i - 1) : 0);
#if defined(_OPENMP)
    std::vector<std::size_t> indices(capacity);
    std::vector<double> rij(capacity * 3);
    std::vector<double> r2(capacity);
    std::size_t* indices_data = capacity > 0 ? indices.data() : nullptr;
    double* rij_data = capacity > 0 ? rij.data() : nullptr;
    double* r2_data = capacity > 0 ? r2.data() : nullptr;
#else
    if (rust_indices.size() < capacity)
      rust_indices.resize(capacity);
    if (rust_rij.size() < capacity * 3)
      rust_rij.resize(capacity * 3);
    if (rust_r2.size() < capacity)
      rust_r2.resize(capacity);
    std::size_t* indices_data = capacity > 0 ? rust_indices.data() : nullptr;
    double* rij_data = capacity > 0 ? rust_rij.data() : nullptr;
    double* r2_data = capacity > 0 ? rust_r2.data() : nullptr;
#endif
    std::size_t count = 0;

    const int status = lpmd_build_neighbor_list_orthogonal(
        positions_data, static_cast<std::size_t>(n), static_cast<std::size_t>(i), cell_lengths,
        periodic, rcu, full ? 1 : 0, indices_data, rij_data, r2_data, capacity, &count);

    if (status == 0) {
      nlist.Clear();
      AtomPair nn;
      nn.i = &atoms[i];
      nn.i_index = i;
      for (std::size_t k = 0; k < count; ++k) {
        const long int j = static_cast<long int>(indices_data[k]);
        nn.j = &(atoms[j]);
        nn.j_index = j;
        const std::size_t offset = k * 3;
        nn.rij = Vector(rij_data[offset], rij_data[offset + 1], rij_data[offset + 2]);
        nn.r2 = r2_data[k];
        nlist.Append(nn);
      }
#if !defined(_OPENMP)
      rust_last_center_index = i;
#endif
      return;
    }
  }
#if !defined(_OPENMP)
  rust_sequence_active = false;
#endif
#endif

  BuildNeighborListCpp(conf, i, nlist, full, rcu);
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin* create(std::string args) { return new MinimumImageCellManager(args); }
void destroy(Plugin* m) { delete m; }
