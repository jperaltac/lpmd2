#include "lpmd/plugin_loader.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace lpmd {
namespace {

void* open_library(const std::filesystem::path& path) {
#if defined(_WIN32)
    HMODULE module = LoadLibraryW(path.wstring().c_str());
    if (!module) {
        std::ostringstream message;
        message << "Failed to load plugin '" << path.string() << "'";
        throw std::runtime_error(message.str());
    }
    return module;
#else
    void* handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle) {
        std::ostringstream message;
        message << "Failed to load plugin '" << path.string() << "': " << dlerror();
        throw std::runtime_error(message.str());
    }
    return handle;
#endif
}

void close_library(void* handle) {
    if (!handle) {
        return;
    }
#if defined(_WIN32)
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

ForceFieldFactory load_factory(void* handle) {
#if defined(_WIN32)
    auto symbol = reinterpret_cast<ForceFieldFactory>(GetProcAddress(
        static_cast<HMODULE>(handle), kForceFieldFactorySymbol));
#else
    auto symbol = reinterpret_cast<ForceFieldFactory>(dlsym(handle, kForceFieldFactorySymbol));
#endif
    if (!symbol) {
        throw std::runtime_error("The loaded plugin does not expose the expected factory function");
    }
    return symbol;
}

}  // namespace

PluginLoader::PluginLoader(std::filesystem::path library_path)
    : library_path_(std::filesystem::absolute(std::move(library_path))) {
    handle_ = open_library(library_path_);
    factory_ = load_factory(handle_);
}

PluginLoader::PluginLoader(PluginLoader&& other) noexcept {
    *this = std::move(other);
}

PluginLoader& PluginLoader::operator=(PluginLoader&& other) noexcept {
    if (this != &other) {
        close();
        library_path_ = std::move(other.library_path_);
        handle_ = other.handle_;
        factory_ = other.factory_;
        other.handle_ = nullptr;
        other.factory_ = nullptr;
    }
    return *this;
}

PluginLoader::~PluginLoader() {
    close();
}

ForceFieldPtr PluginLoader::create_force_field() const {
    if (!factory_) {
        throw std::runtime_error("Plugin factory was not loaded correctly");
    }
    return factory_();
}

void PluginLoader::close() {
    if (handle_) {
        close_library(handle_);
        handle_ = nullptr;
        factory_ = nullptr;
    }
}

std::filesystem::path append_platform_suffix(const std::filesystem::path& base_name) {
    std::filesystem::path with_extension = base_name;
#if defined(_WIN32)
    with_extension += ".dll";
#elif defined(__APPLE__)
    with_extension += ".dylib";
#else
    with_extension += ".so";
#endif
    return with_extension;
}

std::filesystem::path default_plugin_name() {
    return append_platform_suffix("lpmd_force_constant");
}

}  // namespace lpmd
