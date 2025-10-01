# plugins

Shared libraries that implement `lpmd::ForceField` instances. The build currently provides a `lpmd_force_constant` plugin that applies a uniform acceleration to all particles.

Additional plugins can be added by creating a new shared library that exposes the `create_force_field` factory declared in `lpmd/force_field.hpp`.
