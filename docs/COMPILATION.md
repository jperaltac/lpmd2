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
