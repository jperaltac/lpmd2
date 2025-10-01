#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

#include "lpmd/force_field.hpp"

namespace {

class ConstantForceField : public lpmd::ForceField {
public:
    ConstantForceField(std::array<double, 3> force) : force_(force) {}

    void apply(std::vector<lpmd::Particle>& particles, double dt) override {
        for (auto& particle : particles) {
            std::array<double, 3> acceleration{};
            for (std::size_t i = 0; i < 3; ++i) {
                acceleration[i] = force_[i] / particle.mass;
                particle.velocity[i] += acceleration[i] * dt;
                particle.position[i] += particle.velocity[i] * dt;
            }
        }
    }

    std::string name() const override { return "constant-force"; }

private:
    std::array<double, 3> force_;
};

}  // namespace

extern "C" lpmd::ForceFieldPtr create_force_field() {
    return std::make_unique<ConstantForceField>(std::array<double, 3>{0.0, 0.0, -9.81});
}
