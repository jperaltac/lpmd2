#include <filesystem>
#include <iostream>
#include <vector>

#include "lpmd/plugin_loader.hpp"
#include "lpmd/simulation.hpp"

namespace fs = std::filesystem;

fs::path resolve_plugin_path(const char* executable_path, const char* user_path) {
    if (user_path) {
        return fs::absolute(user_path);
    }

    const fs::path exe_dir = fs::absolute(fs::path(executable_path).parent_path());
    const fs::path plugin_name = lpmd::default_plugin_name();

    const fs::path candidates[] = {
        exe_dir / plugin_name,
        exe_dir / ".." / plugin_name,
        exe_dir / "../lib" / plugin_name,
        fs::current_path() / plugin_name,
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return candidate;
        }
    }

    return candidates[0];
}

int main(int argc, char** argv) {
    try {
        const char* user_path = argc > 1 ? argv[1] : nullptr;
        fs::path plugin_path = resolve_plugin_path(argv[0], user_path);

        lpmd::PluginLoader loader(plugin_path);
        auto force_field = loader.create_force_field();
        std::vector<lpmd::Particle> particles{
            {{0.0, 0.0, 10.0}, {1.0, 0.0, 0.0}, 1.0},
            {{1.0, 0.0, 15.0}, {0.0, 1.0, 0.0}, 2.0},
        };

        lpmd::Simulation simulation(std::move(particles), std::move(force_field));
        simulation.run(10, 0.1);

        std::cout << "Simulation time: " << simulation.time() << "s\n";
        std::size_t index = 0;
        for (const auto& particle : simulation.particles()) {
            std::cout << "Particle " << index++ << ": position = (" << particle.position[0] << ", "
                      << particle.position[1] << ", " << particle.position[2] << ")" << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [plugin-path]" << std::endl;
        return 1;
    }

    return 0;
}
