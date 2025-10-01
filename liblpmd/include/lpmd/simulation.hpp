#pragma once

#include <cstddef>
#include <vector>

#include "lpmd/force_field.hpp"
#include "lpmd/particle.hpp"

namespace lpmd {

class Simulation {
public:
    Simulation() = default;
    Simulation(std::vector<Particle> particles, ForceFieldPtr force_field);

    void set_force_field(ForceFieldPtr force_field);
    void set_particles(std::vector<Particle> particles);

    void step(double dt);
    void run(std::size_t steps, double dt);

    [[nodiscard]] const std::vector<Particle>& particles() const noexcept { return particles_; }
    [[nodiscard]] double time() const noexcept { return time_; }

private:
    std::vector<Particle> particles_{};
    ForceFieldPtr force_field_{};
    double time_{0.0};
};

}  // namespace lpmd
