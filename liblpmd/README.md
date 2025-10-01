# liblpmd

`liblpmd` is the reusable core of the lpmd2 project. It exposes the data
structures and abstractions required to integrate particles in time while
allowing force-field implementations to be loaded at run time from plugins.

## Architecture at a glance

The library is intentionally small and layered so downstream executables can
orchestrate simulations without duplicating logic:

| Component | Responsibility |
|-----------|----------------|
| [`Particle`](include/lpmd/particle.hpp) | Immutable description of a particle's instantaneous state (position, velocity and mass). |
| [`ForceField`](include/lpmd/force_field.hpp) | Polymorphic interface implemented by plugins. The `apply()` method mutates a particle array in place, advancing the system by a time step. |
| [`Simulation`](include/lpmd/simulation.hpp) | Small driver that owns the particle container, keeps track of simulation time and repeatedly applies a force field. |
| [`PluginLoader`](include/lpmd/plugin_loader.hpp) | Cross-platform wrapper for dynamically loading a shared library and retrieving the exported factory symbol. |

The implementation files under [`src/`](src) provide the runtime behaviour for
the headers above. The public CMake target exported by the directory is
`lpmd::core`, an alias for the actual `liblpmd` static/shared library.

## Simulation lifecycle

1. **Load a force field** – Use `PluginLoader` with either an explicit path or
   `default_plugin_name()`/`append_platform_suffix()` to resolve a platform
   specific library name. The loader validates that the expected
   `create_force_field` factory is present before exposing it to callers.
2. **Instantiate the simulation** – Provide an initial particle array and a
   `ForceFieldPtr` to the `Simulation` constructor, or create the simulation
   first and call `set_force_field()` once the plugin is available. Both paths
   enforce that the force field pointer is non-null, maintaining internal
   invariants.
3. **Integrate** – Call `step(dt)` to advance the system once or `run(steps,
   dt)` for repeated integration. The simulation updates its internal clock via
   `time()`, and exposes the evolving particle container through `particles()`.

Each integration step delegates to the active force field implementation. This
separation keeps `Simulation` agnostic to the specific physics while providing
consistent bookkeeping of time and particle state.

## Error handling and invariants

* `Simulation::set_force_field()` now rejects null pointers, throwing
  `std::invalid_argument`. This prevents accidental usage of an uninitialised
  simulation that would otherwise fail later during integration.
* `Simulation::step()` throws `std::runtime_error` when no force field is
  configured. Client code should catch this error to surface meaningful messages
  to end users.
* `PluginLoader` normalises the library path to an absolute path and releases
  native handles automatically when the loader is destroyed or moved from.

None of the core classes are thread-safe by themselves. Callers that manipulate
shared simulations or plugin loaders must introduce external synchronisation.

## Building only `liblpmd`

`liblpmd` is integrated into the repository's top-level CMake project. The
following commands configure a build tree and compile only the core library:

```bash
cmake -S . -B build/liblpmd -DCMAKE_BUILD_TYPE=Release
cmake --build build/liblpmd --target liblpmd
```

The first command configures the entire project, but the second command limits
the actual compilation step to the `liblpmd` target. Replace `Release` with
`Debug` (or any other CMake build type) as needed. To install just the library
and its headers you can run `cmake --install build/liblpmd --component liblpmd`
once a corresponding install rule is added to the project.

## Opportunities for further improvement

While reviewing the codebase a few possible enhancements emerged:

* **Time-step validation** – Guarding against non-positive `dt` values in
  `Simulation::step()` would catch configuration errors earlier.
* **Plugin diagnostics** – Extending the Windows loader branch to surface the
  message associated with `GetLastError()` would improve debugging failed
  loads.
* **Optional logging hooks** – Thread-safe logging callbacks inside
  `PluginLoader` and `Simulation` would simplify integrating the library into
  larger applications that require detailed audit trails.

These ideas are not required for correct behaviour today but could make the
library even more robust and user friendly in the future.
