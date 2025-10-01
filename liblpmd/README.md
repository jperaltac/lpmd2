# liblpmd

The core library that exposes simulation primitives and plugin utilities for lpmd2.

## Contents

- `include/lpmd/particle.hpp` – Particle definition used by the simulation.
- `include/lpmd/force_field.hpp` – Abstract interface implemented by force-field plugins.
- `include/lpmd/simulation.hpp` – Lightweight integrator that advances particles in time.
- `include/lpmd/plugin_loader.hpp` – Cross-platform dynamic loader for plugins.
- `src/` – Implementations for the headers above.

The library exports the `lpmd::core` target. Downstream components simply link against it.
