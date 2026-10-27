//
//
//

#ifndef __LINKEDCELL_MODULE_H
#define __LINKEDCELL_MODULE_H

#include <lpmd/cellmanager.h>
#include <lpmd/configuration.h>
#include <lpmd/plugin.h>
#include <array>
#include <vector>

using namespace lpmd;

class LinkedCell : public lpmd::CellManager, public lpmd::Plugin {
public:
  LinkedCell(std::string args);
  ~LinkedCell();
  void ShowHelp() const;

  void Reset();
  void UpdateCell(Configuration& conf);
  void UpdateAtom(Configuration& conf, long i);
  double Cutoff() const { return cutoff; }

  void BuildNeighborList(Configuration& conf, long i, NeighborList& nlist, bool full, double rcut);

private:
  double cutoff;
  int nx, ny, nz;
  std::vector<long> head, atomlist, indexc;
  std::vector<lpmd::BasicAtom*> atomptr;
  std::vector<std::array<int, 3>> offsets;
  bool mode, warn_outside;
};

#endif
