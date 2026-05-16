use std::{ptr, slice};

use csta::hist::Hist;

/// Compute the average and variance of a histogram built from a slice of `f64` values.
///
/// The histogram is created using the [`csta`] crate. The caller specifies the range of
/// the histogram and the number of buckets. Any values that fall outside the requested
/// range are clamped to the nearest boundary bucket.
///
/// # Safety
///
/// - `data` must point to `len` consecutive `f64` values.
/// - `out_average` and `out_variance`, when non-null, must be valid pointers to writable
///   `f64` locations.
/// - The function performs no synchronization; the provided pointers must not be shared
///   across threads without external coordination.
///
/// # Errors
///
/// The function returns `-1` when the inputs are invalid (null data pointer, empty slice,
/// invalid range or bucket count). It returns `-2` if any sample is not a finite value.
/// On success the function writes the requested statistics (when the corresponding output
/// pointers are non-null) and returns `0`.
#[no_mangle]
pub unsafe extern "C" fn lpmd_histogram_summary(
    data: *const f64,
    len: usize,
    range_min: f64,
    range_max: f64,
    buckets: usize,
    out_average: *mut f64,
    out_variance: *mut f64,
) -> i32 {
    if data.is_null()
        || len == 0
        || !range_min.is_finite()
        || !range_max.is_finite()
        || !(range_min < range_max)
        || buckets == 0
    {
        return -1;
    }

    let samples = unsafe { slice::from_raw_parts(data, len) };

    if samples.iter().any(|v| !v.is_finite()) {
        return -2;
    }

    let mut hist = Hist::with_buckets(range_min, range_max, buckets);
    for value in samples {
        hist.add_clamped(value);
    }

    let (average, variance) = hist.average_variange();

    if !out_average.is_null() {
        unsafe {
            ptr::write(out_average, average);
        }
    }

    if !out_variance.is_null() {
        unsafe {
            ptr::write(out_variance, variance);
        }
    }

    0
}

/// Build the minimum-image neighbor list for one atom in an orthogonal cell.
///
/// The function is intentionally shaped as a C ABI batch primitive: C++ owns all memory,
/// Rust receives plain arrays, and Rust fills preallocated output buffers with the
/// neighbor index, displacement vector, and squared distance for every pair that falls
/// inside `cutoff`.
///
/// # Safety
///
/// - `positions_xyz` must point to `atom_count * 3` consecutive `f64` values laid out as
///   `x0, y0, z0, x1, y1, z1, ...`.
/// - `cell_lengths` must point to three finite positive `f64` values.
/// - `periodic` must point to three bytes, where zero means non-periodic and non-zero
///   means periodic.
/// - Output arrays must point to at least `capacity`, `capacity * 3`, and `capacity`
///   writable elements respectively, unless `capacity == 0`.
/// - `out_count` must be a valid writable pointer.
///
/// # Return codes
///
/// - `0`: success.
/// - `-1`: invalid pointer, range, atom index, cutoff, or non-finite input.
/// - `-2`: output capacity is too small; `out_count` is set to the required number of
///   entries when possible.
#[no_mangle]
pub unsafe extern "C" fn lpmd_build_neighbor_list_orthogonal(
    positions_xyz: *const f64,
    atom_count: usize,
    center_index: usize,
    cell_lengths: *const f64,
    periodic: *const u8,
    cutoff: f64,
    full: i32,
    out_indices: *mut usize,
    out_rij_xyz: *mut f64,
    out_r2: *mut f64,
    capacity: usize,
    out_count: *mut usize,
) -> i32 {
    if positions_xyz.is_null()
        || cell_lengths.is_null()
        || periodic.is_null()
        || out_count.is_null()
        || atom_count == 0
        || center_index >= atom_count
        || !cutoff.is_finite()
        || cutoff <= 0.0
    {
        return -1;
    }

    if capacity > 0 && (out_indices.is_null() || out_rij_xyz.is_null() || out_r2.is_null()) {
        return -1;
    }

    let positions = unsafe { slice::from_raw_parts(positions_xyz, atom_count * 3) };
    let cell = unsafe { slice::from_raw_parts(cell_lengths, 3) };
    let periodic = unsafe { slice::from_raw_parts(periodic, 3) };

    if cell.iter().any(|value| !value.is_finite() || *value <= 0.0) {
        return -1;
    }

    let cutoff2 = cutoff * cutoff;
    if !cutoff2.is_finite() {
        return -1;
    }

    let center_offset = center_index * 3;
    let center = [
        positions[center_offset],
        positions[center_offset + 1],
        positions[center_offset + 2],
    ];
    if center.iter().any(|value| !value.is_finite()) {
        return -1;
    }

    let include_all_pairs = full != 0;
    let start = if include_all_pairs {
        0
    } else {
        center_index + 1
    };
    let mut count = 0usize;
    for atom_index in start..atom_count {
        if atom_index == center_index {
            continue;
        }

        let offset = atom_index * 3;
        let candidate = [
            positions[offset],
            positions[offset + 1],
            positions[offset + 2],
        ];
        if candidate.iter().any(|value| !value.is_finite()) {
            return -1;
        }
        let mut displacement = [
            candidate[0] - center[0],
            candidate[1] - center[1],
            candidate[2] - center[2],
        ];

        for axis in 0..3 {
            if periodic[axis] == 0 {
                continue;
            }
            let length = cell[axis];
            let half = 0.5 * length;
            if displacement[axis] >= half {
                displacement[axis] -= length;
            } else if displacement[axis] < -half {
                displacement[axis] += length;
            }
        }

        let r2 = displacement[0] * displacement[0]
            + displacement[1] * displacement[1]
            + displacement[2] * displacement[2];

        if r2 < cutoff2 {
            if count >= capacity {
                let mut required = count + 1;
                for rest_index in (atom_index + 1)..atom_count {
                    if !include_all_pairs && rest_index <= center_index {
                        continue;
                    }
                    let rest_offset = rest_index * 3;
                    let rest_candidate = [
                        positions[rest_offset],
                        positions[rest_offset + 1],
                        positions[rest_offset + 2],
                    ];
                    if rest_candidate.iter().any(|value| !value.is_finite()) {
                        return -1;
                    }
                    let mut rest_displacement = [
                        rest_candidate[0] - center[0],
                        rest_candidate[1] - center[1],
                        rest_candidate[2] - center[2],
                    ];
                    for axis in 0..3 {
                        if periodic[axis] == 0 {
                            continue;
                        }
                        let length = cell[axis];
                        let half = 0.5 * length;
                        if rest_displacement[axis] >= half {
                            rest_displacement[axis] -= length;
                        } else if rest_displacement[axis] < -half {
                            rest_displacement[axis] += length;
                        }
                    }
                    let rest_r2 = rest_displacement[0] * rest_displacement[0]
                        + rest_displacement[1] * rest_displacement[1]
                        + rest_displacement[2] * rest_displacement[2];
                    if rest_r2 < cutoff2 {
                        required += 1;
                    }
                }
                unsafe {
                    ptr::write(out_count, required);
                }
                return -2;
            }

            unsafe {
                ptr::write(out_indices.add(count), atom_index);
                ptr::write(out_rij_xyz.add(count * 3), displacement[0]);
                ptr::write(out_rij_xyz.add(count * 3 + 1), displacement[1]);
                ptr::write(out_rij_xyz.add(count * 3 + 2), displacement[2]);
                ptr::write(out_r2.add(count), r2);
            }
            count += 1;
        }
    }

    unsafe {
        ptr::write(out_count, count);
    }
    0
}

/// Compute Lennard-Jones pair energies, force vectors, virial, and stress terms in batch.
///
/// `rij_xyz` and `r2` describe `count` already-filtered pair displacements. C++ owns the
/// buffers; Rust only validates inputs and writes per-pair forces plus accumulated scalar
/// and tensor contributions.
///
/// # Safety
///
/// - `rij_xyz` must point to `count * 3` consecutive `f64` values.
/// - `r2` must point to `count` consecutive positive finite `f64` values.
/// - `out_force_xyz` must point to `count * 3` writable `f64` values when `count > 0`.
/// - `out_energy`, `out_virial`, and `out_stress` must be writable pointers; `out_stress`
///   must point to nine `f64` values in row-major order.
///
/// # Return codes
///
/// - `0`: success.
/// - `-1`: invalid pointers, non-finite parameters, non-positive distances, or a pair
///   outside the cutoff.
#[no_mangle]
pub unsafe extern "C" fn lpmd_lennard_jones_batch(
    rij_xyz: *const f64,
    r2: *const f64,
    count: usize,
    sigma: f64,
    epsilon: f64,
    cutoff: f64,
    out_force_xyz: *mut f64,
    out_energy: *mut f64,
    out_virial: *mut f64,
    out_stress: *mut f64,
) -> i32 {
    if rij_xyz.is_null()
        || r2.is_null()
        || out_energy.is_null()
        || out_virial.is_null()
        || out_stress.is_null()
        || (count > 0 && out_force_xyz.is_null())
        || !sigma.is_finite()
        || sigma <= 0.0
        || !epsilon.is_finite()
        || !cutoff.is_finite()
        || cutoff <= 0.0
    {
        return -1;
    }

    let rij = unsafe { slice::from_raw_parts(rij_xyz, count * 3) };
    let r2_values = unsafe { slice::from_raw_parts(r2, count) };

    let sigma2 = sigma * sigma;
    let cutoff2 = cutoff * cutoff;
    if !sigma2.is_finite() || !cutoff2.is_finite() {
        return -1;
    }

    let mut energy = 0.0;
    let mut virial = 0.0;
    let mut stress = [0.0_f64; 9];

    for pair_index in 0..count {
        let rr2 = r2_values[pair_index];
        if !rr2.is_finite() || rr2 <= 0.0 || rr2 >= cutoff2 {
            return -1;
        }

        let offset = pair_index * 3;
        let rx = rij[offset];
        let ry = rij[offset + 1];
        let rz = rij[offset + 2];
        if !rx.is_finite() || !ry.is_finite() || !rz.is_finite() {
            return -1;
        }

        let r6 = (sigma2 / rr2).powi(3);
        let r12 = r6 * r6;
        let force_factor = -48.0 * (epsilon / rr2) * (r12 - 0.5 * r6);
        let fx = rx * force_factor;
        let fy = ry * force_factor;
        let fz = rz * force_factor;

        unsafe {
            ptr::write(out_force_xyz.add(offset), fx);
            ptr::write(out_force_xyz.add(offset + 1), fy);
            ptr::write(out_force_xyz.add(offset + 2), fz);
        }

        energy += 4.0 * epsilon * (r12 - r6);
        virial -= rx * fx + ry * fy + rz * fz;
        stress[0] += -rx * fx;
        stress[1] += -rx * fy;
        stress[2] += -rx * fz;
        stress[3] += -ry * fx;
        stress[4] += -ry * fy;
        stress[5] += -ry * fz;
        stress[6] += -rz * fx;
        stress[7] += -rz * fy;
        stress[8] += -rz * fz;
    }

    unsafe {
        ptr::write(out_energy, energy);
        ptr::write(out_virial, virial);
        for (index, value) in stress.iter().enumerate() {
            ptr::write(out_stress.add(index), *value);
        }
    }

    0
}

#[cfg(test)]
mod tests {
    use super::{
        lpmd_build_neighbor_list_orthogonal, lpmd_histogram_summary, lpmd_lennard_jones_batch,
    };
    use std::ptr;

    #[test]
    fn computes_average_and_variance() {
        let samples = [0.5, 1.5, 2.5, 3.5, 4.5];
        let mut avg = 0.0;
        let mut var = 0.0;

        let status = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                0.0,
                5.0,
                5,
                &mut avg,
                &mut var,
            )
        };

        assert_eq!(status, 0);
        assert!((avg - 2.5).abs() < 1e-6);
        assert!(var >= 0.0);
    }

    #[test]
    fn rejects_invalid_input() {
        let mut avg = 0.0;
        let status = unsafe {
            lpmd_histogram_summary(ptr::null(), 0, 0.0, 1.0, 1, &mut avg, ptr::null_mut())
        };
        assert_eq!(status, -1);
    }

    #[test]
    fn rejects_non_finite_samples() {
        let samples = [0.0, f64::NAN];
        let mut avg = 123.0;
        let mut var = 456.0;

        let status = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                0.0,
                1.0,
                4,
                &mut avg,
                &mut var,
            )
        };

        assert_eq!(status, -2);
        assert_eq!(avg, 123.0);
        assert_eq!(var, 456.0);
    }

    #[test]
    fn accepts_null_output_pointers() {
        let samples = [1.0, 2.0, 3.0];

        let status = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                0.0,
                4.0,
                4,
                ptr::null_mut(),
                ptr::null_mut(),
            )
        };

        assert_eq!(status, 0);
    }

    #[test]
    fn rejects_invalid_ranges_and_bucket_counts() {
        let samples = [1.0, 2.0, 3.0];
        let mut avg = 0.0;

        let zero_buckets = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                0.0,
                3.0,
                0,
                &mut avg,
                ptr::null_mut(),
            )
        };
        let reversed_range = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                3.0,
                0.0,
                3,
                &mut avg,
                ptr::null_mut(),
            )
        };
        let infinite_range = unsafe {
            lpmd_histogram_summary(
                samples.as_ptr(),
                samples.len(),
                f64::NEG_INFINITY,
                3.0,
                3,
                &mut avg,
                ptr::null_mut(),
            )
        };

        assert_eq!(zero_buckets, -1);
        assert_eq!(reversed_range, -1);
        assert_eq!(infinite_range, -1);
    }

    #[test]
    fn builds_half_neighbor_list_with_minimum_image() {
        let positions = [
            0.0, 0.0, 0.0, // center
            1.0, 0.0, 0.0, // direct neighbor
            9.5, 0.0, 0.0, // periodic neighbor at -0.5
            5.0, 0.0, 0.0, // outside cutoff
        ];
        let cell = [10.0, 10.0, 10.0];
        let periodic = [1_u8, 1, 1];
        let mut indices = [0_usize; 4];
        let mut rij = [0.0_f64; 12];
        let mut r2 = [0.0_f64; 4];
        let mut count = 0_usize;

        let status = unsafe {
            lpmd_build_neighbor_list_orthogonal(
                positions.as_ptr(),
                4,
                0,
                cell.as_ptr(),
                periodic.as_ptr(),
                2.0,
                0,
                indices.as_mut_ptr(),
                rij.as_mut_ptr(),
                r2.as_mut_ptr(),
                indices.len(),
                &mut count,
            )
        };

        assert_eq!(status, 0);
        assert_eq!(count, 2);
        assert_eq!(&indices[..count], &[1, 2]);
        assert_eq!(&rij[..3], &[1.0, 0.0, 0.0]);
        assert_eq!(&rij[3..6], &[-0.5, 0.0, 0.0]);
        assert!((r2[0] - 1.0).abs() < 1e-12);
        assert!((r2[1] - 0.25).abs() < 1e-12);
    }

    #[test]
    fn full_neighbor_list_skips_center_atom() {
        let positions = [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0];
        let cell = [10.0, 10.0, 10.0];
        let periodic = [0_u8, 0, 0];
        let mut indices = [0_usize; 3];
        let mut rij = [0.0_f64; 9];
        let mut r2 = [0.0_f64; 3];
        let mut count = 0_usize;

        let status = unsafe {
            lpmd_build_neighbor_list_orthogonal(
                positions.as_ptr(),
                3,
                1,
                cell.as_ptr(),
                periodic.as_ptr(),
                3.0,
                1,
                indices.as_mut_ptr(),
                rij.as_mut_ptr(),
                r2.as_mut_ptr(),
                indices.len(),
                &mut count,
            )
        };

        assert_eq!(status, 0);
        assert_eq!(count, 2);
        assert_eq!(&indices[..count], &[0, 2]);
    }

    #[test]
    fn reports_required_neighbor_capacity() {
        let positions = [0.0, 0.0, 0.0, 1.0, 0.0, 0.0];
        let cell = [10.0, 10.0, 10.0];
        let periodic = [0_u8, 0, 0];
        let mut count = 0_usize;

        let status = unsafe {
            lpmd_build_neighbor_list_orthogonal(
                positions.as_ptr(),
                2,
                0,
                cell.as_ptr(),
                periodic.as_ptr(),
                2.0,
                0,
                ptr::null_mut(),
                ptr::null_mut(),
                ptr::null_mut(),
                0,
                &mut count,
            )
        };

        assert_eq!(status, -2);
        assert_eq!(count, 1);
    }

    #[test]
    fn computes_lennard_jones_batch_terms() {
        let rij = [1.0_f64, 0.0, 0.0];
        let r2 = [1.0_f64];
        let mut forces = [0.0_f64; 3];
        let mut energy = 0.0;
        let mut virial = 0.0;
        let mut stress = [0.0_f64; 9];

        let status = unsafe {
            lpmd_lennard_jones_batch(
                rij.as_ptr(),
                r2.as_ptr(),
                1,
                1.0,
                1.0,
                2.0,
                forces.as_mut_ptr(),
                &mut energy,
                &mut virial,
                stress.as_mut_ptr(),
            )
        };

        assert_eq!(status, 0);
        assert!((energy - 0.0).abs() < 1e-12);
        assert!((forces[0] + 24.0).abs() < 1e-12);
        assert_eq!(forces[1], 0.0);
        assert_eq!(forces[2], 0.0);
        assert!((virial - 24.0).abs() < 1e-12);
        assert!((stress[0] - 24.0).abs() < 1e-12);
    }

    #[test]
    fn rejects_lennard_jones_pairs_outside_cutoff() {
        let rij = [3.0_f64, 0.0, 0.0];
        let r2 = [9.0_f64];
        let mut forces = [0.0_f64; 3];
        let mut energy = 0.0;
        let mut virial = 0.0;
        let mut stress = [0.0_f64; 9];

        let status = unsafe {
            lpmd_lennard_jones_batch(
                rij.as_ptr(),
                r2.as_ptr(),
                1,
                1.0,
                1.0,
                2.0,
                forces.as_mut_ptr(),
                &mut energy,
                &mut virial,
                stress.as_mut_ptr(),
            )
        };

        assert_eq!(status, -1);
    }
}
