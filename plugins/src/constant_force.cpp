#include <lpmd/plugin.h>
#include <lpmd/module.h>
#include <lpmd/vector.h>
#include <lpmd/particleset.h>
#include <lpmd/error.h>

#include <cstdlib>
#include <iostream>

namespace {

class ConstantForcePlugin : public lpmd::Plugin {
public:
    ConstantForcePlugin()
        : lpmd::Plugin("constant-force", "1.0.0") {
        DefineKeyword("fx", "0.0");
        DefineKeyword("fy", "0.0");
        DefineKeyword("fz", "-9.81");
    }

    std::string Provides() const override { return "force-field"; }

    void ShowHelp() const override {
        std::cout << "Applies a constant acceleration to every particle in the active configuration.\n"
                  << "Keywords: fx fy fz (acceleration components in Å/ps²)." << std::endl;
    }

    void Apply(lpmd::ParticleSet& particles, double dt) {
        const lpmd::Vector acceleration(
            atof((*this)["fx"].c_str()),
            atof((*this)["fy"].c_str()),
            atof((*this)["fz"].c_str()));

        for (long i = 0; i < particles.Size(); ++i) {
            auto& atom = particles[i];
            atom.Velocity() += acceleration * dt;
            atom.Position() += atom.Velocity() * dt;
        }
    }
};

}  // namespace

extern "C" lpmd::Plugin* create(std::string args) {
    auto* plugin = new ConstantForcePlugin();
    if (!args.empty()) {
        plugin->ProcessArguments(args);
    }
    return plugin;
}

extern "C" void destroy(lpmd::Plugin* plugin) {
    delete plugin;
}
