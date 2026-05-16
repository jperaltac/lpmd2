#ifndef LPMD_RUST_HOTSPOTS_H
#define LPMD_RUST_HOTSPOTS_H

#include <cstddef>
#include <cstdint>

extern "C" int lpmd_histogram_summary(const double* data, std::size_t len, double range_min,
                                      double range_max, std::size_t buckets, double* out_average,
                                      double* out_variance);

extern "C" int lpmd_build_neighbor_list_orthogonal(const double* positions_xyz,
                                                   std::size_t atom_count, std::size_t center_index,
                                                   const double* cell_lengths,
                                                   const std::uint8_t* periodic, double cutoff,
                                                   int full, std::size_t* out_indices,
                                                   double* out_rij_xyz, double* out_r2,
                                                   std::size_t capacity, std::size_t* out_count);

extern "C" int lpmd_lennard_jones_batch(const double* rij_xyz, const double* r2, std::size_t count,
                                        double sigma, double epsilon, double cutoff,
                                        double* out_force_xyz, double* out_energy,
                                        double* out_virial, double* out_stress);

#endif // LPMD_RUST_HOTSPOTS_H
