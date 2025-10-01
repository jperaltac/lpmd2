#pragma once

#include <cstddef>
#include <vector>

#include "lpmd/force_field.hpp"
#include "lpmd/particle.hpp"

namespace lpmd {

/// Lightweight driver that advances a collection of particles using a force
/// field provided by plugins.
class Simulation {
public:
    Simulation() = default;
    /// Construct a simulation with an initial particle set and a force field.
    ///
    /// The force field must be non-null; use the default constructor followed
    /// by set_force_field() when deferring plugin loading.
    Simulation(std::vector<Particle> particles, ForceFieldPtr force_field);

    /// Install the force field used by subsequent integration steps.
    ///
    /// Passing a null pointer is rejected to keep the object in a valid
    /// state. Clients that need to unload a force field should create a fresh
    /// Simulation instance instead.
    void set_force_field(ForceFieldPtr force_field);

    /// Replace the particle container. The integration time is reset because
    /// the new state represents the beginning of a trajectory.
    void set_particles(std::vector<Particle> particles);

    /// Advance the system by a single time step.
    void step(double dt);

    /// Execute multiple time steps back-to-back.
    void run(std::size_t steps, double dt);

    [[nodiscard]] const std::vector<Particle>& particles() const noexcept { return particles_; }
    [[nodiscard]] double time() const noexcept { return time_; }

private:
    std::vector<Particle> particles_{};
    ForceFieldPtr force_field_{};
    double time_{0.0};
};

}  // namespace lpmd
