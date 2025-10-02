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

#[cfg(test)]
mod tests {
    use super::lpmd_histogram_summary;
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
}
