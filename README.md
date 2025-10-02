# lpmd2

A modernized CMake build that combines the core `liblpmd` library, runtime plugins, and the `lpmd` command line executable into a single project. The reorganisation makes it easy to build all components together and experiment with a simple molecular dynamics workflow.

## Building

Before configuring the project make sure the required system packages are
available. The repository ships with a small helper script that installs the
compiler toolchain, CMake, and the GLUT development headers on the most common
Linux distributions:

```bash
./scripts/install_requirements.sh
```

Once the dependencies are in place you can configure and build the project:

```bash
cmake -S . -B build
cmake --build build
```

The executable is placed in `build/bin/lpmd` and plugins in `build/lib`.

For a more detailed walkthrough of verifying the compilation outputs, see
[`docs/COMPILATION.md`](docs/COMPILATION.md).

## Running

Run the simulator and let it locate the default plugin automatically:

```bash
./build/bin/lpmd
```

You can also specify a plugin explicitly:

```bash
./build/bin/lpmd /path/to/plugin
```

## Project layout

- `liblpmd/` – Core simulation primitives and the plugin loader.
- `plugins/` – Sample plugins implementing `lpmd::ForceField`.
- `lpmd/` – Command line executable that wires everything together.
