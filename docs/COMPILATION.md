# Compilation Verification

This document records the commands that can be used to check that the project
compiles successfully from a clean checkout.

## Steps

1. Install the system requirements (compiler toolchain, CMake, GLUT headers).
   The provided helper script covers the most common Linux distributions:
   ```bash
   ./scripts/install_requirements.sh
   ```

2. Configure the project and generate the build system files:
   ```bash
   cmake -S . -B build
   ```
3. Build all targets:
   ```bash
   cmake --build build
   ```

The build should finish without errors, producing the following artifacts:

- `build/bin/lpmd` – the main executable.
- `build/lib/liblpmd.a` – the static library that exposes the simulation
  core.
- `build/lib/constantforce.so` – the constant-force plugin.

These paths match the default output configuration of the CMake project and
confirm that the compilation stage completed successfully.

The plotter examples require Povray. On Debian/Ubuntu, install it (and common
movie encoders) with:

```bash
sudo apt-get update
sudo apt-get install povray imagemagick ffmpeg
```

## OpenMP build and shock-wave example

OpenMP is optional and disabled by default. Enable it explicitly to compile
the core pair and metallic force loops with the compiler's OpenMP runtime:

```bash
cmake -S . -B build-openmp -DCMAKE_BUILD_TYPE=Release \
  -DLPMD_ENABLE_OPENMP=ON -DLPMD_ENABLE_TESTS=ON
cmake --build build-openmp --parallel 4
ctest --test-dir build-openmp --output-on-failure
```

CMake must report `Found OpenMP_CXX`; configuration fails if support is missing.
No manual compiler or linker flags are needed. OpenMP flags apply to the core;
legacy parallel directives in plugin-only implementations remain disabled.
The tests include numerical force, energy, virial and stress comparisons for
the parallel accumulation.

`--parallel 4` controls compilation jobs. Set `OMP_NUM_THREADS=4` separately
for simulation threads. See the [shock-wave guide](../lpmd/examples/shock-wave/README.md)
for a quick check, the full example, output analysis and reproducible timing
comparisons using 1, 2 and 4 threads.

## Installation verification

To verify installation without writing to system directories, configure a
temporary prefix with **Unix Makefiles**, prepare its installation binaries,
and run the installed executable:

```bash
cmake -S . -B build-install -G "Unix Makefiles" \
  -DCMAKE_INSTALL_PREFIX=/tmp/lpmd2-install
cmake --build build-install --target preinstall --parallel 4
cmake --install build-install
/tmp/lpmd2-install/bin/lpmd /path/to/lpmd2/lpmd/examples/smoke.control
```

For a system-wide install from an existing Unix Makefiles build, use the
configured prefix (normally `/usr/local`):

```bash
cmake --build build --target preinstall --parallel 4
sudo cmake --install build
```

`cmake --install` invokes the installer directly; it does not build the
installation relink files. With Unix Makefiles, a plugin that needs its
runtime path changed for installation may have a separate binary under
`CMakeRelink.dir`. Thus, even after a successful normal build, installation
can report that `CMakeRelink.dir/<plugin>.so` is missing, for example
`CMakeRelink.dir/addvelocity.so`. The `preinstall` step above creates the
installation binaries before the installer copies them. Compilation and
`preinstall` run as the current user; only the system installation uses `sudo`.

For existing build directories, check `CMAKE_GENERATOR` in `CMakeCache.txt`.
The `preinstall` target here belongs to Unix Makefiles; do not request it for
Ninja or change an existing build directory's generator. The normal build
commands in the earlier sections still apply when running from the build tree.

## Smoke example

After building, run the minimal headless example from a scratch directory so
generated files do not dirty the checkout:

```bash
mkdir -p /tmp/lpmd-smoke-run
cd /tmp/lpmd-smoke-run
/path/to/lpmd2/build/bin/lpmd /path/to/lpmd2/lpmd/examples/smoke.control
```

A successful run prints `SIMULATION FINISHED` and writes `smoke.xyz`.

## Testing

Unit tests are gated behind the `LPMD_ENABLE_TESTS` CMake option. To compile
and run them alongside the core library:

1. Configure the build with testing enabled (the option defaults to CTest's
   `BUILD_TESTING` flag):

   ```bash
   cmake -S . -B build -DLPMD_ENABLE_TESTS=ON
   ```

2. Generate the test binaries. The helper target regenerates the LPUnit-based
   sources and builds the corresponding executables:

   ```bash
   cmake --build build --target liblpmd_unit_tests
   ```

3. Execute the suite via CTest:

   ```bash
   ctest --test-dir build --output-on-failure
   ```

New checks can be registered by adding a `.unit` file to
`liblpmd/tests/` and listing it in `liblpmd/tests/CMakeLists.txt`. The
LPUnit generator converts each `.unit` specification into a dedicated test
executable that is picked up automatically by the `liblpmd_unit_tests`
aggregate target.

## Diagnosing slow builds under WSL

Compiling tests and executing them are separate stages. A line such as
`Building CXX object .../paramvaluetest.cc.o` reports a compilation job being
started; with parallel builds, the last printed filename does not establish
which job is slow. Disabling tests reduces the build workload but does not
identify the cause of a stall.

From the checkout on the affected WSL machine, keep tests enabled and first
build with a bounded number of jobs:

```bash
cmake -S . -B build-wsl -DCMAKE_BUILD_TYPE=Release -DLPMD_ENABLE_TESTS=ON
cmake --build build-wsl --parallel 2 --verbose
ctest --test-dir build-wsl --output-on-failure --timeout 60 -j 1
```

To isolate the file shown in the original report:

```bash
cmake --build build-wsl --target paramvaluetest_unit --parallel 1 --verbose
```

While it is stalled, collect these diagnostics in another WSL terminal:

```bash
free -h
vmstat 1 10
ps -C cc1plus,c++,g++,make,ninja -o pid,ppid,stat,pcpu,pmem,rss,etime,args
dmesg -T | grep -Ei 'out of memory|oom|killed process' | tail -20
```

`cc1plus` is GCC's C++ compiler process. Sustained swap activity suggests
memory pressure; an OOM message confirms a process was killed for lack of
memory. CPU activity alone cannot distinguish useful compilation from a
compiler bug. These observations must come from the affected machine.

The test runner now rejects missing `@end` directives, and the CTest entries
have timeouts. Timeouts apply to execution, not to C++ compilation.

## Memory and undefined-behavior checks

With GCC or Clang, use a separate Debug build:

```bash
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DLPMD_ENABLE_TESTS=ON -DLPMD_ENABLE_OPENMP=OFF \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-sanitize --target liblpmd_unit_tests --parallel 2
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  ctest --test-dir build-sanitize --output-on-failure --timeout 60 -j 1
```

LeakSanitizer requires access to process information and should run outside
debuggers or restricted sandboxes that prevent it from inspecting threads.
The regression suite covers species selection, vector parsing, test-runner
lifecycle, cell transforms, and neighbor lists for both `linkedcell` and
`verletlist`, including changing radii, moving atoms and changing cell geometry.
