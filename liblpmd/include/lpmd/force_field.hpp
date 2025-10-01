#pragma once

#include <memory>
#include <string>
#include <vector>

#include "lpmd/particle.hpp"

namespace lpmd {

class ForceField {
public:
    virtual ~ForceField() = default;
    virtual void apply(std::vector<Particle>& particles, double dt) = 0;
    virtual std::string name() const = 0;
};

using ForceFieldPtr = std::unique_ptr<ForceField>;
using ForceFieldFactory = ForceFieldPtr (*)();

inline constexpr const char* kForceFieldFactorySymbol = "create_force_field";

}  // namespace lpmd
