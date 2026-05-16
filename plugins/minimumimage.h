//
//
//

#ifndef __MINIMUMIMAGE_MODULE_H
#define __MINIMUMIMAGE_MODULE_H

#include <lpmd/cellmanager.h>
#include <lpmd/plugin.h>

#ifdef LPMD_ENABLE_RUST_HOTSPOTS
#ifndef _OPENMP
#include <cstddef>
#include <vector>
#endif
#endif

using namespace lpmd;

class MinimumImageCellManager : public CellManager, public Plugin {
public:
  MinimumImageCellManager(std::string args);
  ~MinimumImageCellManager();
  void ShowHelp() const;

  void Show(std::ostream& os) const;

  void Reset();
  void UpdateCell(Configuration& conf);
  void UpdateAtom(Configuration& conf, long i);
  void BuildNeighborList(Configuration& conf, long i, NeighborList& nlist, bool full, double);
  double Cutoff() const;

private:
  double rcut;

#if defined(LPMD_ENABLE_RUST_HOTSPOTS) && !defined(_OPENMP)
  std::vector<double> rust_positions;
  std::vector<std::size_t> rust_indices;
  std::vector<double> rust_rij;
  std::vector<double> rust_r2;
  long rust_last_center_index;
  long rust_cached_atom_count;
  bool rust_sequence_active;
#endif
};

#endif
