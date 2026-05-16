# lpmd2

`lpmd2` is a modern CMake-based distribution of the LPMD (Large-scale
Molecular-dynamics Package) codebase. It builds the core `liblpmd` simulation
library, the runtime plugin collection, and the command-line tools from a
single repository so that the whole molecular-dynamics workflow can be compiled
and exercised together.

The project keeps the original LPMD concepts—control files, loadable modules,
cell managers, integrators, potentials, filters, analyzers, converters, and
visualizers—while replacing the legacy build glue with predictable CMake output
locations.

## What gets built

A default build produces:

- `build/bin/lpmd` – main simulator that reads an LPMD control file.
- `build/bin/lpmd-analyzer` – analyzer-oriented entry point for property
  calculations.
- `build/bin/lpmd-converter` – conversion/filtering utility for supported
  structure formats.
- `build/bin/lpmd-visualizer` – visualization-oriented entry point.
- `build/bin/lpmd-plotter.py` – Python/Povray-based high-quality renderer
  script.
- `build/lib/` – the `liblpmd` static library, Python plotter package, and one
  shared object per runtime plugin.

OpenGL/GLUT are optional for the build. When they are unavailable, CMake skips
OpenGL-only plugins and builds a headless `lpvisual` fallback so non-graphical
examples can still run.

## Requirements

The repository includes a helper script that installs the common Linux build
requirements: a C++ toolchain, CMake, zlib development headers, and GLUT/OpenGL
headers where available.

```bash
./scripts/install_requirements.sh
```

If you install dependencies manually, make sure these tools and libraries are
available before configuring the project:

- CMake 3.16 or newer.
- A C++17-capable compiler.
- zlib development files.
- Optional: OpenGL and GLUT development files for graphical visualization
  plugins.
- Optional for `lpmd-plotter.py`: Python, Povray, and a movie/GIF encoder such
  as `mencoder` or ImageMagick, depending on the requested output.

## Build

Configure and compile from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

The project defaults to a `Release` build for single-configuration generators.
Use the standard CMake build-type switch if you need a different variant:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

For a focused compilation checklist and expected artifacts, see
[`docs/COMPILATION.md`](docs/COMPILATION.md).

## Run a quick smoke test

After building, run the smallest headless example from a scratch directory so
that generated files do not dirty the checkout:

```bash
mkdir -p /tmp/lpmd-smoke-run
cd /tmp/lpmd-smoke-run
/path/to/lpmd2/build/bin/lpmd /path/to/lpmd2/lpmd/examples/smoke.control
```

A successful run prints `SIMULATION FINISHED` and writes `smoke.xyz` in the
scratch directory.

## Run the bundled examples

The example control files live in [`lpmd/examples/`](lpmd/examples/). Most of
those examples write output files to the current working directory, and several
refer to input files with paths relative to the examples tree. The safest way to
try them is therefore:

```bash
cd /path/to/lpmd2/lpmd/examples
../../build/bin/lpmd smoke.control
../../build/bin/lpmd argon.control
cd filters
../../../build/bin/lpmd-converter sphere.control
```

For descriptions of each example, prerequisites, and command lines for the
simulator, converter, visualizer, analyzer, and plotter examples, read
[`lpmd/examples/README.md`](lpmd/examples/README.md).

## Tests

Unit tests are controlled by the `LPMD_ENABLE_TESTS` CMake option, which follows
CTest's `BUILD_TESTING` flag. To build and run them:

```bash
cmake -S . -B build -DLPMD_ENABLE_TESTS=ON
cmake --build build --target liblpmd_unit_tests
ctest --test-dir build --output-on-failure
```

## Project layout

- `CMakeLists.txt` – top-level build that wires together the library, plugins,
  and applications.
- `liblpmd/` – core simulation API, runtime support, IO, units, tests, and
  legacy auxiliary libraries.
- `plugins/` – loadable runtime modules: potentials, integrators, filters,
  analyzers, IO formats, generators, modifiers, and visualization support.
- `lpmd/` – command-line applications, parser support, quick-mode helpers, and
  the Python plotter package.
- `lpmd/examples/` – runnable control files and sample structures.
- `docs/` – build verification notes and additional technical documentation.
- `scripts/` – helper scripts for local setup.

## Development notes

- Plugin filenames are significant: the runtime loader expects modules named
  after the control-file module names, for example `lennardjones.so` or
  `minimumimage.so`.
- CMake writes executables to `build/bin` and libraries/plugins to `build/lib`
  for predictable local execution.
- Prefer running examples in a temporary or dedicated output directory when you
  do not want generated `.xyz`, `.lpmd`, `.dat`, `.png`, `.pov`, or movie files
  in the source tree.
