#include <lpmd/atom.h>
#include <lpmd/fixedsizeparticleset.h>
#include <lpmd/orthogonalcell.h>
#include <lpmd/simulation.h>
#include <lpmd/simulationbuilder.h>
#include <lpmd/vector.h>

#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
constexpr double kSigma = 3.41;          // Å
constexpr double kEpsilon = 0.0103408;   // eV
constexpr double kForceFactor = 0.0096485341;

using lpmd::FixedSizeParticleSet;
using lpmd::OrthogonalCell;
using lpmd::Simulation;
using lpmd::Vector;

Vector pair_force(const Vector& r) {
    const double rr2 = r.SquareModule();
    const double r6 = std::pow(kSigma * kSigma / rr2, 3.0);
    const double r12 = r6 * r6;
    const double factor = -48.0 * (kEpsilon / rr2) * (r12 - 0.5 * r6);
    return r * factor;
}

double update_forces(FixedSizeParticleSet& atoms, OrthogonalCell& cell) {
    double potential_energy = 0.0;
    for (long i = 0; i < atoms.Size() - 1; ++i) {
        for (long j = i + 1; j < atoms.Size(); ++j) {
            const Vector displacement = cell.Displacement(atoms[i].Position(), atoms[j].Position());
            const double distance = displacement.Module();
            if (distance <= 0.0) {
                continue;
            }
            const Vector force = pair_force(displacement);
            potential_energy += 4.0 * kEpsilon * (std::pow(kSigma / distance, 12) - std::pow(kSigma / distance, 6));
            atoms[i].Acceleration() += force * (kForceFactor / atoms[i].Mass());
            atoms[j].Acceleration() -= force * (kForceFactor / atoms[j].Mass());
        }
    }
    return potential_energy;
}

void update_positions(FixedSizeParticleSet& atoms, OrthogonalCell& cell, double dt) {
    for (long i = 0; i < atoms.Size(); ++i) {
        auto& atom = atoms[i];
        atom.Position() = cell.FittedInside(atom.Position() + atom.Velocity() * dt);
        atom.Velocity() += atom.Acceleration() * dt;
    }
}

void initialize_system(FixedSizeParticleSet& atoms, OrthogonalCell& cell) {
    cell[0] = 10.0 * lpmd::e1;
    cell[1] = 10.0 * lpmd::e2;
    cell[2] = 10.0 * lpmd::e3;

    atoms[0].Position() = cell.ScaleByCell(Vector(0.25, 0.25, 0.25));
    atoms[1].Position() = cell.ScaleByCell(Vector(0.75, 0.75, 0.75));

    atoms[0].Velocity() = Vector(0.0, 0.0, 0.0);
    atoms[1].Velocity() = Vector(0.0, 0.0, 0.0);

    atoms[0].Acceleration() = Vector(0.0, 0.0, 0.0);
    atoms[1].Acceleration() = Vector(0.0, 0.0, 0.0);
}

}  // namespace

int main() {
    try {
        Simulation& simulation = lpmd::SimulationBuilder::CreateFixedOrthogonal(2, lpmd::Atom("Ar"));
        auto& atoms = dynamic_cast<FixedSizeParticleSet&>(simulation.Atoms());
        auto& cell = dynamic_cast<OrthogonalCell&>(simulation.Cell());

        initialize_system(atoms, cell);

        constexpr double dt = 0.5;
        constexpr long steps = 200;

        for (long step = 0; step < steps; ++step) {
            for (long i = 0; i < atoms.Size(); ++i) {
                atoms[i].Acceleration() = Vector(0.0, 0.0, 0.0);
            }

            const double potential_energy = update_forces(atoms, cell);
            update_positions(atoms, cell, dt);

            std::cout << "Step " << step << ": potential energy = " << potential_energy << " eV" << std::endl;
        }

        std::cout << "Final positions:" << std::endl;
        for (long i = 0; i < atoms.Size(); ++i) {
            const Vector& pos = atoms[i].Position();
            std::cout << "  Atom " << i << " -> (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
