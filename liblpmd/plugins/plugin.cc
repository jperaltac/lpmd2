//
//
//

#include <lpmd/error.h>
#include <lpmd/plugin.h>
#include <lpmd/session.h>

#include <cassert>
#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace lpmd;

//
// Dynamic loading of modules
//
namespace {
struct DlCloser {
  void operator()(void* handle) const noexcept {
    if (handle != nullptr)
      dlclose(handle);
  }
};
} // namespace

Plugin* lpmd::PluginLoader(std::string path, std::string args) {
  struct stat sst;
  if (stat(path.c_str(), &sst) == -1)
    return NULL; // could not 'stat' the plugin file...
  std::cerr << "-> Plugin file found at " << path << '\n';
  std::unique_ptr<void, DlCloser> mymodule(dlopen(path.c_str(), RTLD_LAZY));
  if (!mymodule) {
    throw PluginError(path, std::string("dlopen failed: ") + dlerror());
  }
  dlerror();
  create_t* create_module = function_cast<create_t*>(dlsym(mymodule.get(), "create"));
  const char* dlsym_error = dlerror();
  if (dlsym_error) {
    throw PluginError(path, std::string("Missing create symbol: ") + dlsym_error);
  }
  destroy_t* destroy_module = function_cast<destroy_t*>(dlsym(mymodule.get(), "destroy"));
  dlsym_error = dlerror();
  if (dlsym_error) {
    throw PluginError(path, std::string("Missing destroy symbol: ") + dlsym_error);
  }
  Plugin* tmp = create_module(args);
  std::unique_ptr<Plugin, destroy_t*> plugin_guard(tmp, destroy_module);
  tmp->unloader = destroy_module;
  tmp->SetLibraryHandle(mymodule.get());
  tmp->AssignParameter("fullpath", path);
  plugin_guard.release();
  mymodule.release();
  return tmp;
}

//
template <class TYPE> TYPE function_cast(void* symbol) {
  assert(sizeof(void*) == sizeof(TYPE));
  union {
    void* symbol;
    TYPE function;
  } cast;
  cast.symbol = symbol;
  return cast.function;
}

//
//
//
Plugin::Plugin(const std::string& pluginname, const std::string& pluginversion)
    : Module(pluginname), used(false), library_handle(NULL) {
  AssignParameter("apirequired", "2.0");
  AssignParameter("version", pluginversion);
}

Plugin::Plugin(const std::string& pluginname, const std::string& pluginversion,
               const std::string& bugreport)
    : Module(pluginname), used(false), library_handle(NULL) {
  AssignParameter("apirequired", "2.0");
  AssignParameter("version", pluginversion);
  AssignParameter("bugreport", bugreport);
}

Plugin::Plugin(const Plugin& mod) : Module(mod), used(mod.used), library_handle(NULL) {}

Plugin::~Plugin() {}

bool Plugin::Used() const { return used; }

void Plugin::SetUsed() { used = true; }

void Plugin::SetLibraryHandle(void* handle) { library_handle = handle; }

void* Plugin::LibraryHandle() const { return library_handle; }

bool Plugin::AutoTest() { throw PluginError(Name(), "Autotest not implemented."); }

void Plugin::CheckConsistency() {}
