# lpmd_stats

This crate contains experimental Rust utilities for the LPMD core that rely on the
[`csta`](https://crates.io/crates/csta) statistics library. The initial FFI surface is the
`lpmd_histogram_summary` function, which builds a histogram for a slice of `f64` samples and
returns the average and variance computed by `csta`.

The crate compiles as both an `rlib` (for unit testing) and a `cdylib` so that the function can
be consumed from C or C++ code. Build and test the crate locally with:

```bash
cargo test
```

The resulting shared library can be linked into `liblpmd` once the build system is updated to
load Rust components.
