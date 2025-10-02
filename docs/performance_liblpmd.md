# liblpmd Performance Notes

This document captures a quick profiling session of the `liblpmd` stack when
running the bundled Argon simulation (`lpmd/examples/argon.control`). The goal
was to identify hotspots that could benefit from deeper optimization work or a
potential port to Rust.

## Methodology

* Build type: `Release` (`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`).
* Profiler: `valgrind --tool=callgrind` on `./build/bin/lpmd lpmd/examples/argon.control`.
  * The run was interrupted after ~4.4 billion IR (instructions retired), which
    still covers several hundred integration steps and offers a stable
    distribution of costs for the inner loops.【14ad63†L1-L27】

## Primary hotspots

| Inclusive IR % | Function | Notes |
| --- | --- | --- |
| ~20.9% | `lpmd::PairPotential::UpdateForces` | Force accumulation dominates runtime. The function repeatedly builds neighbour lists and performs scalar work per pair.【F:liblpmd/simulation/pairpotential.cc†L34-L87】【14ad63†L1-L27】 |
| ~15.0% | `lpmd::OrthogonalCell::Displacement` | Called for every atom pair to apply periodic minimum-image corrections.【F:liblpmd/lpmd/orthogonalcell.h†L21-L144】【14ad63†L24-L27】 |
| ~11.0% | `MinimumImageCellManager::BuildNeighborList` | Generates neighbour lists on the fly for every atom, causing repeated traversal and dynamic allocations.【F:plugins/minimumimage.cc†L74-L108】【14ad63†L1-L27】 |
| ~6.4% | `LennardJones::pairForce` | Uses `pow` to compute `r⁻⁶`; this alone costs ~300M IR because it is evaluated for every interacting pair.【F:plugins/lennardjones.cc†L54-L70】【14ad63†L27-L32】 |
| ~5.9% | `std::vector::_M_realloc_insert` inside neighbour list | Indicates the neighbour list container keeps reallocating while being filled for each atom.【F:liblpmd/lpmd/array.h†L16-L59】【14ad63†L27-L34】 |

Other notable costs include repeated evaluations of `std::pow` in the math
library (used indirectly by `pairForce`) and per-step console printing.

## Recommendations

1. **Target `PairPotential::UpdateForces` first.** It represents the single
   largest block of work and naturally exposes a parallel-friendly structure. A
   Rust port could focus on batching neighbour list iteration and force updates
   while maintaining the existing C++ interface via FFI.【F:liblpmd/simulation/pairpotential.cc†L34-L87】
2. **Replace expensive math in `LennardJones::pairForce`.** Precompute
   `sigma²` and reuse inverse distances to avoid `pow`, or hoist the Lennard–
   Jones force evaluation into Rust where vectorization is easier to control.【F:plugins/lennardjones.cc†L54-L70】
3. **Reduce neighbour list churn.**
   * Pre-size `NeighborList` (`lpmd::Array<AtomPair>`) based on an estimate of
     close neighbours to avoid repeated `std::vector` reallocations, or switch
     to a small-buffer optimization in Rust.【F:liblpmd/lpmd/array.h†L16-L59】
   * Cache per-atom neighbour lists instead of rebuilding them from scratch for
     every `UpdateForces` call, or port `MinimumImageCellManager::BuildNeighborList`
     to Rust and expose a reusable neighbour iterator.【F:plugins/minimumimage.cc†L74-L108】
4. **Consider a cell/neighbor traversal rewrite.** Both
   `OrthogonalCell::Displacement` and `MinimumImageCellManager::BuildNeighborList`
   are pure, allocation-free computations that map cleanly onto Rust’s
   ownership model. Moving them to Rust could eliminate virtual dispatch inside
   the hottest loops while keeping the higher-level C++ API intact.【F:liblpmd/lpmd/orthogonalcell.h†L21-L144】【F:plugins/minimumimage.cc†L74-L108】

Even partial reimplementation of these hotspots in Rust—exposed as C ABI
functions that operate on plain buffers—would shift a majority of the runtime to
Rust code without forcing a rewrite of the surrounding simulation pipeline.
