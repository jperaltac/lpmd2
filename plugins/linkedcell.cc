//
//
//

#include "linkedcell.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace lpmd;

namespace {
int WrapAround(int i, int n) {
  const int remainder = i % n;
  return remainder < 0 ? remainder + n : remainder;
}
}

LinkedCell::LinkedCell(std::string args) : Plugin("linkedcell", "1.0") {
  ParamList& params = (*this);
  DefineKeyword("cutoff", "7.0");
  DefineKeyword("nx", "0");
  DefineKeyword("ny", "0");
  DefineKeyword("nz", "0");
  DefineKeyword("mode", "noauto");
  DefineKeyword("warn-outside", "true");
  ProcessArguments(args);
  cutoff = double(params["cutoff"]);
  nx = int(params["nx"]);
  ny = int(params["ny"]);
  nz = int(params["nz"]);
  if (!std::isfinite(cutoff) || cutoff <= 0.0 || nx < 0 || ny < 0 || nz < 0)
    throw PluginError("linkedcell", "cutoff must be positive and subdivisions nonnegative");
  mode = nx == 0 || ny == 0 || nz == 0 || params["mode"] == "auto" || params["mode"] == "AUTO";
  warn_outside = (params["warn-outside"] == "true");
}

LinkedCell::~LinkedCell() = default;

void LinkedCell::ShowHelp() const {
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Module Name        = linkedcell                                               \n";
  std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Info      >>                                                          \n";
  std::cout << "      This plugin implements the linked-cell method for making neighbors lists.\n";
  std::cout << "      It is one of the available cell managers.                                \n";
  std::cout << "      Advice: When dividing the simulation cell in cells, their edge should    \n";
  std::cout << "      be at least the cutoff the cutoff of the potential.                      \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Options   >>                                                          \n";
  std::cout << "      cutoff        : Sets the cutoff radius for the evaluation.               \n";
  std::cout << "      nx            : Sets the number of subdivisions in the X axis.           \n";
  std::cout << "      ny            : Sets the number of subdivisions in the Y axis.           \n";
  std::cout << "      nz            : Sets the number of subdivisions in the Z axis.           \n";
  std::cout << "      mode          : Sets manual (default) or automatic selection of the      \n";
  std::cout << "                      values of nx, ny and nz (auto / noauto).                 \n";
  std::cout << "                      auto: values are chosen automatically.                   \n";
  std::cout << "                      noauto: values must be entered by the user.              \n";
  std::cout << "      warn-outside  : Shows a warning if an atom if an atom is detected outside\n";
  std::cout << "                      the simulation box (true / false):                       \n";
  std::cout << "                      true: show warning.                                      \n";
  std::cout << "                      false: do not show warning.                              \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Example           >>                                                          \n";
  std::cout << " #Loading the plugin :                                                         \n";
  std::cout << " use linkedcell                                                                \n";
  std::cout << "     cutoff 8.5                                                                \n";
  std::cout << "     nx 15                                                                     \n";
  std::cout << "     ny 15                                                                     \n";
  std::cout << "     nz 15                                                                     \n";
  std::cout << "     mode noauto                                                               \n";
  std::cout << "     warn-outside false                                                        \n";
  std::cout << " enduse                                                                        \n";
  std::cout << " #Applying the plugin :                                                        \n";
  std::cout << " cellmanager linkedcell                                                      \n\n";
  std::cout << "      The plugin is used to select the linkedcell method for making the lists  \n";
  std::cout << "      of atoms' neighbors. In this case, the simulation cell is divided 5x5x5  \n";
  std::cout << "      (125) cubic cells, whose edge is L/15, being L the edge of the cubic     \n";
  std::cout << "      simulation cell. In this case, L/15 should be greater than rc, the cutoff\n";
  std::cout << "      of the potential. For cubic cells, it is recommended to use nx=ny=nz=L/n,\n";
  std::cout << "      where n=[L/rc] and [] is the floor function.                             \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void LinkedCell::Reset() {
  head.clear();
  atomlist.clear();
  indexc.clear();
  atomptr.clear();
  offsets.clear();
}

void LinkedCell::UpdateCell(Configuration& conf) {
  BasicParticleSet& atoms = conf.Atoms();
  const BasicCell& cell = conf.Cell();
  const double volume = std::abs(Dot(Cross(cell[0], cell[1]), cell[2]));
  if (!std::isfinite(volume) || volume <= 0.0)
    throw PluginError("linkedcell", "Simulation cell must have a finite, positive volume");

  // The distance between cell faces, rather than the length of a lattice
  // vector, bounds the fractional displacement in skew cells.
  double height[3];
  int dimensions[3] = {nx, ny, nz};
  for (int axis = 0; axis < 3; ++axis) {
    height[axis] = volume / Cross(cell[(axis + 1) % 3], cell[(axis + 2) % 3]).Module();
    if (!std::isfinite(height[axis]) || height[axis] <= 0.0)
      throw PluginError("linkedcell", "Invalid cell geometry");
    if (mode) {
      const double divisions = std::max(1.0, std::floor(height[axis] / cutoff));
      if (divisions > std::numeric_limits<int>::max())
        throw PluginError("linkedcell", "Too many subdivisions");
      dimensions[axis] = static_cast<int>(divisions);
    }
  }
  nx = dimensions[0];
  ny = dimensions[1];
  nz = dimensions[2];
  std::size_t cell_count = 1;
  for (int count : dimensions) {
    if (cell_count > head.max_size() / static_cast<std::size_t>(count))
      throw PluginError("linkedcell", "Too many subdivisions");
    cell_count *= static_cast<std::size_t>(count);
  }
  head.assign(cell_count, -1);
  atomlist.assign(atoms.Size(), -1);
  indexc.resize(atoms.Size());
  atomptr.resize(atoms.Size());

  // Unique periodic offsets avoid counting a neighbor several times when an
  // axis has fewer bins than the search stencil. A single stencil replaces
  // the previous table of neighbors for every bin.
  int first[3], count[3];
  for (int axis = 0; axis < 3; ++axis) {
    const double reach = std::ceil(cutoff * dimensions[axis] / height[axis]);
    if (reach >= dimensions[axis] / 2) {
      first[axis] = -(dimensions[axis] / 2);
      count[axis] = dimensions[axis];
    } else {
      first[axis] = -static_cast<int>(reach);
      count[axis] = 2 * static_cast<int>(reach) + 1;
    }
  }
  offsets.clear();
  for (int z = 0; z < count[2]; ++z)
    for (int y = 0; y < count[1]; ++y)
      for (int x = 0; x < count[0]; ++x)
        offsets.push_back({first[0] + x, first[1] + y, first[2] + z});

  for (long atom = 0; atom < atoms.Size(); ++atom) {
    // Refresh pointers after every update/resizing. Neighbor queries can then
    // avoid a virtual particle-container lookup for every candidate pair.
    atomptr[atom] = &atoms[atom];
    const Vector fractional = cell.Fractional(atomptr[atom]->Position());
    int bin[3];
    for (int axis = 0; axis < 3; ++axis) {
      if (!std::isfinite(fractional[axis]))
        throw PluginError("linkedcell", "Non-finite atom position");
      const double wrapped = fractional[axis] - std::floor(fractional[axis]);
      bin[axis] = std::min(dimensions[axis] - 1, static_cast<int>(wrapped * dimensions[axis]));
    }
    const long index = (static_cast<long>(bin[2]) * ny + bin[1]) * nx + bin[0];
    indexc[atom] = index;
    atomlist[atom] = head[index];
    head[index] = atom;
  }
}

void LinkedCell::UpdateAtom(Configuration& conf, long i) {
  (void)i;
  // Rebuild the links so both the old and new bins remain valid. Appending an
  // already linked atom could previously create cycles and stale neighbors.
  UpdateCell(conf);
}

void LinkedCell::BuildNeighborList(Configuration& conf, long i, NeighborList& nlist, bool full,
                                   double rcut) {
  BasicParticleSet& atoms = conf.Atoms();
  const BasicCell& cell = conf.Cell();
  nlist.Clear();
  if (std::isnan(rcut) || rcut <= 0.0)
    throw PluginError("linkedcell", "Requested neighbor cutoff must be positive");
  if (indexc.size() != static_cast<std::size_t>(atoms.Size()) || head.empty())
    throw PluginError("linkedcell", "Call UpdateCell before querying neighbors");

  AtomPair pair;
  pair.i = atomptr[i];
  pair.i_index = i;
  const Vector& position = pair.i->Position();
  const double radius2 = rcut * rcut;
  auto append = [&](long j) {
    if (j == i || (!full && j > i))
      return;
    pair.j = atomptr[j];
    pair.j_index = j;
    pair.rij = cell.Displacement(position, pair.j->Position());
    pair.r2 = pair.rij.SquareModule();
    if (pair.r2 < radius2)
      nlist.Append(pair);
  };

  // A query larger than the configured stencil needs an exhaustive search;
  // silently using the smaller stencil would miss valid neighbors.
  if (rcut > cutoff) {
    for (long j = 0; j < atoms.Size(); ++j)
      append(j);
    return;
  }

  const long index = indexc[i];
  const int x = index % nx;
  const int y = (index / nx) % ny;
  const int z = index / (static_cast<long>(nx) * ny);
  for (const auto& offset : offsets) {
    const long neighbor = (static_cast<long>(WrapAround(z + offset[2], nz)) * ny +
                           WrapAround(y + offset[1], ny)) * nx +
                          WrapAround(x + offset[0], nx);
    for (long j = head[neighbor]; j != -1; j = atomlist[j])
      append(j);
  }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin* create(std::string args) { return new LinkedCell(args); }
void destroy(Plugin* m) { delete m; }
