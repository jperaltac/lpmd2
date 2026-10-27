//
//
//

#include "verletlist.h"
#include <lpmd/configuration.h>

#include <cmath>

using namespace lpmd;

VerletListCellManager::VerletListCellManager(std::string args)
    : Plugin("verletlist", "1.0"), step_count(0), old_size(-1) {
  ParamList& params = (*this);
  //
  DefineKeyword("cutoff", "0.0");
  DefineKeyword("extcutoff", "0.0");
  DefineKeyword("maxneighbors", "80");
  DefineKeyword("each", "5");
  // hasta aqui los valores por omision
  ProcessArguments(args);
  rcut = params["cutoff"];
  extcut = params["extcutoff"];
  maxneighbors = int(params["maxneighbors"]);
  each = int(params["each"]);
  if (!std::isfinite(rcut) || rcut < 0.0 || !std::isfinite(extcut) || extcut < 0.0 ||
      each <= 0 || maxneighbors <= 0)
    throw PluginError("verletlist", "Invalid cutoff, extcutoff, each, or maxneighbors");
  if (extcut == 0.0)
    extcut = 1.08 * rcut;
  if (!std::isfinite(extcut) || extcut < rcut)
    throw PluginError("verletlist", "extcutoff must be at least cutoff");
}

VerletListCellManager::~VerletListCellManager() = default;

void VerletListCellManager::ShowHelp() const {
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Module Name        = verletlist                                               \n";
  std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Info      >>                                                          \n";
  std::cout << "      This plugin implements the verlet-list method for making neighbors lists.\n";
  std::cout << "      It is one of the available cell managers.                                \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Options   >>                                                          \n";
  std::cout << "      cutoff        : Sets the inner cutoff radius for the list (in angstrom). \n";
  std::cout << "      extcutoff     : Sets the outter cutoff radius for the list. Its default  \n";
  std::cout << "                      value is (cutoff + .08*cutoff).                          \n";
  std::cout << "      maxneighbors  : Set the maximum number of neighbors that have a atom in  \n";
  std::cout << "                      the list.                                                \n";
  std::cout << "      each          : Set the frequency to 'rebuild' the atomic neighbors list.\n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Example           >>                                                          \n";
  std::cout << " #Loading the plugin :                                                         \n";
  std::cout << " use verletlist                                                                \n";
  std::cout << "     cutoff 5.0                                                                \n";
  std::cout << "     maxneighbors 15                                                           \n";
  std::cout << "     each 10                                                                   \n";
  std::cout << " enduse                                                                        \n";
  std::cout << " #Applying the plugin :                                                        \n";
  std::cout << " cellmanager verletlist                                                      \n\n";
  std::cout << "      The plugin is used to select the verlet-list method for making the lists \n";
  std::cout << "      of atoms' neighbors. In this case, the maximum number of neighbors allowed\n";
  std::cout << "      is 15, and the list is updated each 10 steps.                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void VerletListCellManager::Reset() {
  old_size = -1;
  step_count = 0;
}

void VerletListCellManager::UpdateVerletList(Configuration& conf) {
  if (fabs(rcut) < 1E-10)
    throw PluginError("verletlist", "No cutoff was defined");
  const BasicCell& cell = conf.Cell();
  BasicParticleSet& atoms = conf.Atoms();
  const long int n = atoms.Size();
  // Mark the cache invalid until the complete list has been built. A capacity
  // error must never leave a partially populated list marked as usable.
  old_size = -1;
  vlist.resize(n);
  reference_positions.resize(n);
  for (auto& neighbors : vlist)
    neighbors.clear();
  for (long i = 0; i < n - 1; ++i)
    for (long j = i + 1; j < n; ++j) {
      const Vector rij = cell.Displacement(atoms[i].Position(), atoms[j].Position());
      if (rij.SquareModule() < extcut * extcut) {
        if (vlist[j].size() >= static_cast<std::size_t>(maxneighbors) ||
            vlist[i].size() >= static_cast<std::size_t>(maxneighbors))
          throw PluginError("verletlist", "maxneighbors is too small!");
        // Add i to verlet list of j
        vlist[j].push_back(i);
        // Add j to verlet list of i
        vlist[i].push_back(j);
      }
    }
  for (long i = 0; i < n; ++i)
    reference_positions[i] = atoms[i].Position();
  for (int axis = 0; axis < 3; ++axis) {
    reference_cell[axis] = cell[axis];
    reference_periodicity[axis] = cell.Periodicity(axis);
  }
  old_size = n;
  step_count = 0;
}

void VerletListCellManager::UpdateAtom(Configuration& conf, long i) {
  (void)i;
  // Both endpoints must be updated, including atoms which were not neighbors
  // before the move (e.g. a Monte Carlo proposal).
  UpdateVerletList(conf);
}

void VerletListCellManager::UpdateCell(Configuration& conf) {
  BasicParticleSet& atoms = conf.Atoms();
  const BasicCell& cell = conf.Cell();
  bool mustupdate = (step_count >= each || atoms.Size() != old_size);
  if (!mustupdate) {
    for (int axis = 0; axis < 3; ++axis)
      if ((cell[axis] - reference_cell[axis]).SquareModule() != 0.0 ||
          cell.Periodicity(axis) != reference_periodicity[axis])
        mustupdate = true;
    // If every atom moves less than half the skin, no pair can enter the
    // inner cutoff from outside the cached outer cutoff. Raw displacements
    // conservatively rebuild on periodic wrapping as well.
    const double limit2 = 0.25 * (extcut - rcut) * (extcut - rcut);
    for (long i = 0; i < atoms.Size() && !mustupdate; ++i)
      if ((atoms[i].Position() - reference_positions[i]).SquareModule() >= limit2)
        mustupdate = true;
  }
  if (mustupdate)
    UpdateVerletList(conf);
  step_count++;
}

double VerletListCellManager::Cutoff() const { return rcut; }

void VerletListCellManager::BuildNeighborList(Configuration& conf, long i, NeighborList& nlist,
                                              bool full, double rcu) {
  BasicParticleSet& atoms = conf.Atoms();
  const BasicCell& cell = conf.Cell();
  nlist.Clear();
  if (std::isnan(rcu) || rcu <= 0.0)
    throw PluginError("verletlist", "Requested neighbor cutoff must be positive");
  if (atoms.Size() != old_size)
    throw PluginError("verletlist", "Call UpdateCell before querying neighbors");
  AtomPair nn;
  nn.i = &atoms[i];
  nn.i_index = i;
  const Vector& ipos = atoms[i].Position();
  if (rcu > rcut) {
    // The skin only guarantees completeness for radii up to rcut. Larger
    // analysis queries must not silently use an incomplete cached list.
    for (long j = 0; j < atoms.Size(); ++j) {
      if (j == i || (!full && j < i))
        continue;
      nn.j = &atoms[j];
      nn.j_index = j;
      nn.rij = cell.Displacement(ipos, nn.j->Position());
      nn.r2 = nn.rij.SquareModule();
      if (nn.r2 < rcu * rcu)
        nlist.Append(nn);
    }
    return;
  }
  if (full) {
    for (long j : vlist[i]) {
      nn.j = &(atoms[j]);
      nn.rij = cell.Displacement(ipos, nn.j->Position());
      nn.r2 = nn.rij.SquareModule();
      nn.j_index = j;
      if (nn.r2 < rcu * rcu)
        nlist.Append(nn);
    }
  } else {
    for (long j : vlist[i]) {
      if (j < i)
        continue;
      nn.j = &(atoms[j]);
      nn.rij = cell.Displacement(ipos, nn.j->Position());
      nn.r2 = nn.rij.SquareModule();
      nn.j_index = j;
      if (nn.r2 < rcu * rcu)
        nlist.Append(nn);
    }
  }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin* create(std::string args) { return new VerletListCellManager(args); }
void destroy(Plugin* m) { delete m; }
