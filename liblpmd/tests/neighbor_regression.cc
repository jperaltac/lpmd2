#ifdef TEST_VERLETLIST
#include "verletlist.h"
using TestedManager = VerletListCellManager;
#else
#include "linkedcell.h"
using TestedManager = LinkedCell;
#endif

#include <lpmd/storedconfiguration.h>
#include <runtime/runtime_context.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using namespace lpmd;

namespace {
void Require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

void Add(StoredConfiguration& conf, const Vector& position) {
  Atom atom("Ar");
  atom.Position() = position;
  conf.Atoms().Append(atom);
}

// Compare both full and half lists to an independent all-pairs search. Half
// lists may use either index ordering, but each unordered pair must occur once.
void Check(CellManager& manager, StoredConfiguration& conf, double radius) {
  const long n = conf.Atoms().Size();
  std::vector<int> pairs(n * n, 0);
  for (long i = 0; i < n; ++i) {
    NeighborList list;
    manager.BuildNeighborList(conf, i, list, true, radius);
    std::vector<long> actual, expected;
    for (long k = 0; k < list.Size(); ++k) {
      const auto& pair = list[k];
      Require(pair.i_index == i && pair.j_index >= 0 && pair.j_index < n, "invalid neighbor index");
      const Vector rij = conf.Cell().Displacement(conf.Atoms()[i].Position(), conf.Atoms()[pair.j_index].Position());
      Require((pair.rij - rij).Module() < 1e-12 && std::abs(pair.r2 - rij.SquareModule()) < 1e-12,
              "stale neighbor distance");
      actual.push_back(pair.j_index);
    }
    for (long j = 0; j < n; ++j)
      if (j != i && conf.Cell().Displacement(conf.Atoms()[i].Position(), conf.Atoms()[j].Position()).SquareModule() < radius * radius)
        expected.push_back(j);
    std::sort(actual.begin(), actual.end());
    Require(actual == expected, "full list has missing, duplicate, or out-of-cutoff neighbors");
    manager.BuildNeighborList(conf, i, list, false, radius);
    for (long k = 0; k < list.Size(); ++k) {
      const long j = list[k].j_index;
      Require(j >= 0 && j < n && j != i, "invalid half-list index");
      ++pairs[std::min(i, j) * n + std::max(i, j)];
    }
  }
  for (long i = 0; i < n; ++i)
    for (long j = i + 1; j < n; ++j) {
      const bool expected = conf.Cell().Displacement(conf.Atoms()[i].Position(), conf.Atoms()[j].Position()).SquareModule() < radius * radius;
      Require(pairs[i * n + j] == int(expected), "half list must contain each neighbor pair exactly once");
    }
}
}

int main(int argc, char** argv) {
  try {
    RuntimeContext context;
    RuntimeContextScope scope(context);
    StoredConfiguration conf;
    conf.Cell()[0] = Vector(12, 0, 0);
    conf.Cell()[1] = Vector(0, 12, 0);
    conf.Cell()[2] = Vector(0, 0, 12);
    Add(conf, Vector(0.1, 1, 1));
    Add(conf, Vector(0.9, 1, 1));
    Add(conf, Vector(2.2, 1, 1));
    Add(conf, Vector(11.8, 1, 1));
    const std::string mode = argc > 1 ? argv[1] : "basic";
#ifdef TEST_VERLETLIST
    std::string args = "cutoff 2.5 extcutoff 3.0 maxneighbors 20 each 5";
#else
    std::string args = "cutoff 2.5 nx 4 ny 4 nz 4";
#endif
    if (mode == "smallgrid")
      args = "cutoff 7 nx 2 ny 2 nz 2";
    if (mode == "widebins")
      args = "cutoff 2.5 nx 2 ny 2 nz 2";
    if (mode == "defaultgrid")
      args = "cutoff 2.5";
    if (mode == "capacity")
      args = "cutoff 2.5 extcutoff 3.0 maxneighbors 3 each 1";
    if (mode == "invalid") {
#ifdef TEST_VERLETLIST
      const std::vector<std::string> invalid = {"cutoff 2.5 each 0", "cutoff 2.5 maxneighbors 0", "cutoff 2.5 extcutoff 2"};
#else
      const std::vector<std::string> invalid = {"cutoff 0", "cutoff -1", "cutoff 2.5 nx -1 ny 2 nz 2"};
#endif
      for (const auto& parameters : invalid) {
        bool rejected = false;
        try { TestedManager bad(parameters); bad.UpdateCell(conf); }
        catch (const PluginError&) { rejected = true; }
        Require(rejected, "invalid neighbor parameters were accepted");
      }
      return 0;
    }
    TestedManager manager(args);
    manager.UpdateCell(conf);
    if (mode == "cutoffs") {
      Check(manager, conf, 1.0);
      Check(manager, conf, 2.5);
      Check(manager, conf, 0.5);
      Check(manager, conf, 8.0);
      Check(manager, conf, std::numeric_limits<double>::infinity());
    } else if (mode == "move") {
      Check(manager, conf, 2.5);
      conf.Atoms()[1].Position() = Vector(6, 1, 1);
      manager.UpdateAtom(conf, 1);
      Check(manager, conf, 2.5);
      conf.Atoms()[1].Position() = Vector(0.9, 1, 1);
      manager.UpdateAtom(conf, 1);
      Check(manager, conf, 2.5);
    } else if (mode == "resize") {
      conf.Atoms().Delete(0);
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
      conf.Atoms().Clear();
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
      Add(conf, Vector(1, 1, 1));
      Add(conf, Vector(1.5, 1, 1));
      manager.Reset();
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
    } else if (mode == "skin") {
      conf.Atoms()[1].Position() = Vector(6, 1, 1);
      manager.Reset();
      manager.UpdateCell(conf);
      conf.Atoms()[1].Position() = Vector(0.5, 1, 1);
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
    } else if (mode == "widebins") {
      conf.Atoms()[0].Position() = Vector(5.8, 1, 1);
      conf.Atoms()[1].Position() = Vector(6.1, 1, 1);
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
    } else if (mode == "geometry") {
      std::mt19937 rng(12345);
      std::uniform_real_distribution<double> unit(0.0, 1.0);
      conf.Atoms().Clear();
      for (int i = 0; i < 64; ++i)
        Add(conf, Vector(12 * unit(rng), 12 * unit(rng), 12 * unit(rng)));
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
      for (int step = 0; step < 3; ++step) {
        conf.Cell()[2] = Vector(6.0, 1.5, 6.0 + step);
        manager.UpdateCell(conf);
        Check(manager, conf, 1.0);
        Check(manager, conf, 2.5);
      }
      conf.Cell().Periodicity(0) = false;
      manager.UpdateCell(conf);
      Check(manager, conf, 2.5);
    } else {
      Check(manager, conf, 2.5);
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
