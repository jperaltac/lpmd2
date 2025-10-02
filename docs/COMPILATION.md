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
- `build/lib/libliblpmd.a` – the static library that exposes the simulation
  core.
- `build/lib/lpmd_force_constant.so` – the sample force field plugin.

These paths match the default output configuration of the CMake project and
confirm that the compilation stage completed successfully.

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
