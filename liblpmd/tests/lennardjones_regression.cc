#include "lennardjones.h"
#include <runtime/runtime_context.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
constexpr double sigma = 3.4;
constexpr double epsilon = 0.0103;

double ReferenceEnergy(const lpmd::Vector& r) {
  return 4.0 * epsilon * (std::pow(sigma / r.Module(), 12) - std::pow(sigma / r.Module(), 6));
}

void Check(double actual, double expected, double tolerance) {
  if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance * std::max(1.0, std::abs(expected)))
    throw std::runtime_error("Lennard-Jones energy/force differs from analytic or finite-difference reference");
}
}

int main() {
  try {
    lpmd::RuntimeContext context;
    lpmd::RuntimeContextScope scope(context);
    LennardJones potential("sigma 3.4 epsilon 0.0103 cutoff 8.5");
    for (const auto& direction : {lpmd::Vector(1, 0, 0), lpmd::Vector(1, 2, -3)}) {
      for (double scale : {0.8, 1.0, 1.1, 1.3, 2.0, 3.0}) {
        const lpmd::Vector r = direction * (sigma * scale / direction.Module());
        Check(potential.pairEnergy(r.Module()), ReferenceEnergy(r), 1e-12);
        const lpmd::Vector force = potential.pairForce(r);
        const double step = 1e-5 * sigma;
        for (int axis = 0; axis < 3; ++axis) {
          lpmd::Vector plus = r, minus = r;
          plus[axis] += step;
          minus[axis] -= step;
          const double derivative = (ReferenceEnergy(plus) - ReferenceEnergy(minus)) / (2 * step);
          Check(force[axis], derivative, 2e-7);
          Check(potential.pairForce(-1.0 * r)[axis], -force[axis], 1e-12);
        }
      }
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
