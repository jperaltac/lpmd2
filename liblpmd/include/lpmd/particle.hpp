#pragma once

#include <array>

namespace lpmd {

struct Particle {
    std::array<double, 3> position{0.0, 0.0, 0.0};
    std::array<double, 3> velocity{0.0, 0.0, 0.0};
    double mass{1.0};
};

}  // namespace lpmd
