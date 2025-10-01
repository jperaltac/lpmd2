#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "lpmd/force_field.hpp"

namespace lpmd {

class PluginLoader {
public:
    explicit PluginLoader(std::filesystem::path library_path);
    PluginLoader(PluginLoader&& other) noexcept;
    PluginLoader& operator=(PluginLoader&& other) noexcept;
    ~PluginLoader();

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    [[nodiscard]] ForceFieldPtr create_force_field() const;
    [[nodiscard]] const std::filesystem::path& library_path() const noexcept { return library_path_; }

private:
    void close();

    std::filesystem::path library_path_{};
    void* handle_{nullptr};
    ForceFieldFactory factory_{nullptr};
};

std::filesystem::path default_plugin_name();
std::filesystem::path append_platform_suffix(const std::filesystem::path& base_name);

}  // namespace lpmd
