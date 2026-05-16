# LPMD examples

This directory contains runnable LPMD control files plus the sample structures
needed by those controls. The examples are useful as smoke tests, input-format
references, plugin demonstrations, and starting points for small molecular
simulation experiments.

## Before running examples

Build the project from the repository root first:

```bash
cmake -S . -B build
cmake --build build
```

Then run most simulator and analyzer examples from this directory so relative
input paths resolve correctly:

```bash
cd /path/to/lpmd2/lpmd/examples
../../build/bin/lpmd smoke.control
```

Generated files are written to the current working directory. If you want to
keep the checkout clean, copy the control file and any required input data to a
scratch directory, or run from a disposable work tree. Subdirectories such as
`filters/` and `plotter/*/` contain controls that should be launched from those
subdirectories because their inputs are referenced by local relative paths.

For the examples below, replace `/path/to/lpmd2` with your checkout path. From
`lpmd/examples`, the main tools are available as:

```bash
../../build/bin/lpmd <control-file>
../../build/bin/lpmd-analyzer <control-file>
../../build/bin/lpmd-converter <control-file>
../../build/bin/lpmd-visualizer <control-file>
PYTHONPATH=../../build/lib python3 ../../build/bin/lpmd-plotter.py <control-file>
```

> Note: graphical visualization examples need OpenGL/GLUT at build time and a
> usable display at run time. Plotter examples additionally need Povray, and
> movie/GIF generation needs the encoder selected by the control file.

## Minimal and utility examples

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `smoke.control` | Minimal one-step, headless Ar FCC simulation. It checks plugin loading, the crystal generator, cell manager setup, null potential/integrator, and XYZ output. Writes `smoke.xyz`. | `../../build/bin/lpmd smoke.control` |
| `createcrystal.control` | Generates a small FCC Ar crystal and writes it in LPMD format as `crystal.lpmd`. | `../../build/bin/lpmd createcrystal.control` |
| `formatconversion.control` | Reads `ZrO2-100conf.zlp`, colors Zr and O atoms, and writes `test2.lpmd`. | `../../build/bin/lpmd formatconversion.control` |
| `undopbc.control` | Builds a small Ar crystal, applies the `undopbc` modifier, and writes `sinpbc.xyz`. | `../../build/bin/lpmd undopbc.control` |
| `visualization.control` | Loads `ZrO2-100conf.zlp`, colors atoms by element, and opens the `lpvisual` visualizer. | `../../build/bin/lpmd visualization.control` |

## Argon simulation examples

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `argon.control` | Basic Lennard-Jones Ar simulation with Beeman integration, temperature scaling, radial distribution output, and `output.xyz`. | `../../build/bin/lpmd argon.control` |
| `argon-zmethod.control` | Ar Z-method-style run with Velocity Verlet, Lennard-Jones interactions, and linked-cell neighbor management. | `../../build/bin/lpmd argon-zmethod.control` |
| `argon-metropoli.control` | Ar Monte Carlo example using the `metropolis` integrator and live `lpvisual` output. | `../../build/bin/lpmd argon-metropoli.control` |
| `argon-lpvisual.control` | Longer Ar dynamics example with temperature scaling, XYZ output, and periodic `lpvisual` visualization. | `../../build/bin/lpmd argon-lpvisual.control` |
| `argon-cellscaling-lpvisual.control` | Ar dynamics with the `cellscaling` modifier, linked-cell neighbors, Lennard-Jones interactions, and visualization. | `../../build/bin/lpmd argon-cellscaling-lpvisual.control` |
| `argon-rotate.control` | Demonstrates rotating a generated Ar crystal with null dynamics and live visualization. | `../../build/bin/lpmd argon-rotate.control` |
| `argon-temp.control` | Larger Ar example that colors atoms by local temperature while applying temperature scaling to a region. | `../../build/bin/lpmd argon-temp.control` |
| `argon-thermalneedle.control` | Thermal-needle demonstration: heats a spherical region, colors by temperature, visualizes, and writes `sphere.lpmd`. | `../../build/bin/lpmd argon-thermalneedle.control` |

## Metal simulation examples

These examples are longer 20,000-step runs and write both trajectory data and a
`properties.dat` monitor file.

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `metals/Au/simulation-au.control` | FCC gold simulation using the Gupta potential and Velocity Verlet integration. Writes `au-output.lpmd`. | `../../build/bin/lpmd metals/Au/simulation-au.control` |
| `metals/Cu/simulation-cu.control` | FCC copper simulation using the Sutton-Chen potential. Writes `cu-output.lpmd`. | `../../build/bin/lpmd metals/Cu/simulation-cu.control` |
| `metals/Fe/simulation-fe.control` | BCC iron simulation using the Finnis-Sinclair potential. Writes `fe-output.lpmd`. | `../../build/bin/lpmd metals/Fe/simulation-fe.control` |
| `metals/Mo/simulation-mo.control` | BCC molybdenum simulation using the extended Finnis-Sinclair potential. Writes `mo-output.lpmd`. | `../../build/bin/lpmd metals/Mo/simulation-mo.control` |
| `fe-zmethod.control` | Iron Z-method-style example using a BCC crystal and the Sutton-Chen plugin. | `../../build/bin/lpmd fe-zmethod.control` |

## Filters and selections

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `selectsphere.control` | Selects a spherical region from a generated BCC Fe crystal and writes `ironball.xyz`. | `../../build/bin/lpmd selectsphere.control` |
| `selectcylinder.control` | Selects a cylindrical region from a generated BCC Fe crystal and writes `ironcylinder.xyz`. | `../../build/bin/lpmd selectcylinder.control` |
| `filter-over-filter.control` | Applies a sphere filter constrained by a box filter and writes `ironball.xyz`. | `../../build/bin/lpmd filter-over-filter.control` |
| `applywithfilter.control` | Applies `setvelocity` only to atoms inside a sphere and writes `velocities.xyz`. | `../../build/bin/lpmd applywithfilter.control` |
| `filters/box.control` | Generates a Voronoi Ar structure and keeps atoms inside a box slab. Writes `output-box.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter box.control` |
| `filters/cone.control` | Generates a Voronoi Ar structure and keeps atoms in a cone. Writes `output-cone.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter cone.control` |
| `filters/cylinder.control` | Generates a Voronoi Ar structure and keeps atoms between cylindrical radii. Writes `output-cylinder.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter cylinder.control` |
| `filters/random.control` | Generates a Voronoi Ar structure and keeps a random percentage of atoms. Writes `output-random.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter random.control` |
| `filters/sphere.control` | Generates a Voronoi Ar structure and keeps atoms inside a sphere. Writes `output-sphere.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter sphere.control` |
| `filters/element.control` | Reads `filters/quartz.lpmd` and keeps oxygen atoms. Writes `output-element.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter element.control` |
| `filters/index.control` | Reads `filters/quartz.lpmd` and keeps atoms by index range. Writes `output-index.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter index.control` |
| `filters/external.control` | Reads `filters/quartz.lpmd` and filters using values from `filters/data.dat`. Writes `output-external.lpmd`. | `cd filters && ../../../build/bin/lpmd-converter external.control` |

## Analyzer examples

Analyzer controls read the bundled `output.xyz` trajectory and write `.dat`
property files. Run them from `lpmd/examples` so `output.xyz` is found.

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `analyzer-cna.control` | Common-neighbor analysis with `minimumimage`; writes `cna.dat`. | `../../build/bin/lpmd-analyzer analyzer-cna.control` |
| `analyzer-gdr.control` | Radial distribution function analysis; writes `gdr.dat`. | `../../build/bin/lpmd-analyzer analyzer-gdr.control` |
| `analyzer-mobility.control` | Mobility analysis with inner/outer cutoff radii; writes `mobility.dat`. | `../../build/bin/lpmd-analyzer analyzer-mobility.control` |
| `analyzer-msd.control` | Mean-square displacement analysis; writes `msd.dat`. | `../../build/bin/lpmd-analyzer analyzer-msd.control` |
| `analyzer-overlap.control` | Overlap function analysis with 200 bins; writes `overlap-200K.dat`. | `../../build/bin/lpmd-analyzer analyzer-overlap.control` |
| `analyzer-vacf.control` | Velocity autocorrelation function analysis; writes `vacf.dat`. | `../../build/bin/lpmd-analyzer analyzer-vacf.control` |

## Visualization and impact examples

| Example | Description | How to run from `lpmd/examples` |
| --- | --- | --- |
| `lpvisual-filter.control` | Opens an Fe crystal in `lpvisual` and visualizes only a spherical region. | `../../build/bin/lpmd-visualizer lpvisual-filter.control` |
| `lpvisual-filter-box.control` | Opens an Fe crystal in `lpvisual` and visualizes a box-filtered region. | `../../build/bin/lpmd-visualizer lpvisual-filter-box.control` |
| `lpvisual-ballfloor.control` | Tags a floor region, filters a ball-shaped Fe region, and visualizes the setup. | `../../build/bin/lpmd-visualizer lpvisual-ballfloor.control` |
| `lpmd-ballfloor.control` | Dynamic ball-floor Fe example using Sutton-Chen, Beeman integration, tags, velocity/color modifiers, and visualization. | `../../build/bin/lpmd lpmd-ballfloor.control` |
| `proyectil.control` | Projectile-style Lennard-Jones example starting from `proyectil.lpmd`, tagging and moving a spherical projectile region. | `../../build/bin/lpmd proyectil.control` |

## Plotter examples

The `plotter/` directory documents `lpmd-plotter.py`, a Python utility that
uses Povray to render high-quality images or movies from `.lpmd` files. Run its
examples from their own subdirectories because the controls use relative input
paths:

```bash
cd /path/to/lpmd2/lpmd/examples/plotter/ex1
PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control
```

| Example | Description | How to run |
| --- | --- | --- |
| `plotter/ex1/movie.control` | Orthographic still-render example based on `final.lpmd`; keeps Povray files and adds a logo. | `cd plotter/ex1 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex2/movie.control` | Perspective still-render example with stronger lighting and automatic Povray cleanup. | `cd plotter/ex2 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex3/movie.control` | Still render with antialiasing disabled, a custom background, and non-default image size. | `cd plotter/ex3 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex4/movie.control` | Still render demonstrating a custom output filename format and reduced lighting. | `cd plotter/ex4 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex5/movie.control` | Movie example from `half.lpmd` without camera rotation. | `cd plotter/ex5 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex6/movie.control` | Movie example from `half.lpmd` with camera rotation and extra frames. | `cd plotter/ex6 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/ex7/plotter-voro.control` | Rotating Voronoi nanostructure render, suitable for animated GIF output. | `cd plotter/ex7 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py plotter-voro.control` |
| `plotter/ex8/movie.control` | TiO2 surface movie generated from a single structure using camera rotation. | `cd plotter/ex8 && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py movie.control` |
| `plotter/logo/plotter.control` | Color LPMD logo render. | `cd plotter/logo && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py plotter.control` |
| `plotter/logo/plotter-bw.control` | Black-and-white LPMD logo render. | `cd plotter/logo && PYTHONPATH=../../../../build/lib python3 ../../../../build/bin/lpmd-plotter.py plotter-bw.control` |

## Input data files

- `output.xyz` – trajectory used by the analyzer examples.
- `ZrO2-100conf.zlp` – compressed zirconia configurations used by conversion
  and visualization examples.
- `zirconia-color.lpmd` – colored zirconia structure in LPMD text format.
- `proyectil.lpmd` – initial structure for `proyectil.control`.
- `filters/quartz.lpmd` and `filters/data.dat` – inputs for filter converter
  examples.
- `plotter/*.lpmd` and `plotter/*/*.png` – plotter input structures and
  reference screenshots.
- `ar108-1.tgz` and `ar108-scalet.tgz` – archived argon sample data retained
  from the original examples collection.
