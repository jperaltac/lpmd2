#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "lpmd/force_field.hpp"

namespace lpmd {

/// RAII wrapper around a dynamically loaded force-field plugin.
class PluginLoader {
public:
    explicit PluginLoader(std::filesystem::path library_path);
    PluginLoader(PluginLoader&& other) noexcept;
    PluginLoader& operator=(PluginLoader&& other) noexcept;
    ~PluginLoader();

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    /// Instantiate a new force field using the plugin's exported factory.
    [[nodiscard]] ForceFieldPtr create_force_field() const;
    [[nodiscard]] const std::filesystem::path& library_path() const noexcept { return library_path_; }

private:
    void close() noexcept;

    std::filesystem::path library_path_{};
    void* handle_{nullptr};
    ForceFieldFactory factory_{nullptr};
};

/// Return the default plugin shared library name for the current platform.
std::filesystem::path default_plugin_name();
/// Append the operating system specific extension to a plugin base name.
std::filesystem::path append_platform_suffix(const std::filesystem::path& base_name);

}  // namespace lpmd
