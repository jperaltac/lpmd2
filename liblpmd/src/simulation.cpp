#include "lpmd/simulation.hpp"

#include <stdexcept>

namespace lpmd {

Simulation::Simulation(std::vector<Particle> particles, ForceFieldPtr force_field)
    : particles_(std::move(particles)), force_field_(std::move(force_field)) {}

void Simulation::set_force_field(ForceFieldPtr force_field) {
    force_field_ = std::move(force_field);
}

void Simulation::set_particles(std::vector<Particle> particles) {
    particles_ = std::move(particles);
    time_ = 0.0;
}

void Simulation::step(double dt) {
    if (!force_field_) {
        throw std::runtime_error("No force field configured for the simulation");
    }
    force_field_->apply(particles_, dt);
    time_ += dt;
}

void Simulation::run(std::size_t steps, double dt) {
    for (std::size_t i = 0; i < steps; ++i) {
        step(dt);
    }
}

}  // namespace lpmd
