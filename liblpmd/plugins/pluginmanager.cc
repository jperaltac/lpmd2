//
//
//

#include "config.h"
#include <lpmd/pluginmanager.h>
#include <lpmd/util.h>

#include <dlfcn.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace lpmd {

struct PluginDeleter {
  void operator()(Plugin* plugin) const noexcept {
    if (plugin == NULL)
      return;
    void* handle = plugin->LibraryHandle();
    destroy_t* destroy = plugin->unloader;
    if (destroy != NULL)
      destroy(plugin);
    else
      delete plugin;
    if (handle != NULL)
      dlclose(handle);
  }
};

} // namespace lpmd

using namespace lpmd;

namespace {
char PathListSeparator() {
#ifdef _WIN32
  return ';';
#else
  return ':';
#endif
}

std::vector<std::filesystem::path> SplitPathList(const std::string& paths) {
  std::vector<std::filesystem::path> result;
  std::size_t start = 0;
  const char separator = PathListSeparator();
  while (start <= paths.size()) {
    const std::size_t end = paths.find(separator, start);
    const std::string item =
        paths.substr(start, end == std::string::npos ? std::string::npos : end - start);
    if (!item.empty())
      result.emplace_back(item);
    if (end == std::string::npos)
      break;
    start = end + 1;
  }
  return result;
}

std::filesystem::path NormalizePath(const std::filesystem::path& path) {
  if (path.empty())
    return path;
  std::filesystem::path normalized = path;
  normalized.make_preferred();
  return normalized.lexically_normal();
}

} // namespace

PluginManager::PluginManager() {
  // Inicializa el pluginpath
  const char* pathvariable = std::getenv("LPMD_PATH");
  if (pathvariable != NULL) {
    for (const auto& entry : SplitPathList(pathvariable))
      AddToPluginPath(entry);
  }
  for (const auto& entry : SplitPathList(PLUGINPATH))
    AddToPluginPath(entry);
}

PluginManager::~PluginManager() {
  Array<std::string> kill_list;
  for (auto it = modules.begin(); it != modules.end(); ++it)
    kill_list.Append(it->first);
  for (long int i = 0; i < kill_list.Size(); ++i)
    UnloadPlugin(kill_list[i]);
}

Array<std::string> PluginManager::Plugins() const {
  Array<std::string> pluginlist;
  for (auto it = modules.begin(); it != modules.end(); ++it)
    pluginlist.Append((*it).first);
  return pluginlist;
}

void PluginManager::AddToPluginPath(const std::filesystem::path& pdir) {
  if (pdir.empty())
    return;
  const std::filesystem::path normalized = NormalizePath(pdir);
  if (std::find(pluginpath.begin(), pluginpath.end(), normalized) == pluginpath.end()) {
    pluginpath.push_back(normalized);
  }
}

void PluginManager::LoadPluginFile(std::string path, std::string id, std::string args) {
  Plugin* mm = PluginLoader(path, args);
  if (mm == NULL)
    throw InvalidRequest("Plugin at " + path);
  mm->SetManager(*this); // FIXME: Por ahora se hace aqui, deberia ir en un constructor de Module en
                         // el nuevo API
  // Agrega nuevas propiedades a la lista
  Array<std::string> proplist = StringSplit(mm->Provides());
  for (long int i = 0; i < proplist.Size(); ++i) {
    namedprops[proplist[i]] = mm;
  }
  modules[id] = std::unique_ptr<Plugin, PluginDeleter>(mm);
}

void PluginManager::LoadPlugin(std::string name, std::string id, std::string args) {
  Plugin* mm = NULL;
  for (const auto& plugindir : pluginpath) {
    mm = NULL;
    const std::filesystem::path candidate = plugindir / (name + ".so");
    mm = PluginLoader(candidate.string(), args);
    if (mm != NULL)
      break;
  }
  if (mm == NULL)
    throw InvalidRequest("Plugin " + name);
  mm->SetManager(*this); // FIXME: Por ahora se hace aqui, deberia ir en un constructor de Module en
                         // el nuevo API
  modules[id] = std::unique_ptr<Plugin, PluginDeleter>(mm);
  DefineAlias(id, name);
  // Agrega nuevas propiedades a la lista
  Array<std::string> proplist = StringSplit(mm->Provides());
  for (long int i = 0; i < proplist.Size(); ++i) {
    namedprops[proplist[i]] = mm;
  }
}

void PluginManager::LoadPlugin(std::string name, std::string args) { LoadPlugin(name, name, args); }

void PluginManager::LoadPlugin(ModuleInfo info) { LoadPlugin(info.name, info.id, info.args); }

void PluginManager::UnloadPlugin(std::string id) {
  auto it = modules.find(id);
  if (it == modules.end())
    return;
  Plugin* m = it->second.get();
  if (m != NULL) {
    // Elimina las propiedades de la lista
    Array<std::string> proplist = StringSplit(m->Provides());
    for (long int i = 0; i < proplist.Size(); ++i) {
      namedprops.erase(proplist[i]);
    }
  }
  modules.erase(it);
}

bool PluginManager::IsLoaded(std::string id) { return (modules.count(id) > 0); }

void PluginManager::UpdatePlugin(std::string id, std::string new_args) {
  std::string realname = id;
  if (aliasdict.Defined(id))
    realname = aliasdict[id];
  UnloadPlugin(id);
  LoadPlugin(realname, id, new_args);
}

std::string PluginManager::GetPluginKeywords(std::string name) {
  if (aliasdict.Defined(name))
    LoadPlugin(aliasdict[name], "tmp_getpluginkeywords", "dummyargument");
  else
    LoadPlugin(name, "tmp_getpluginkeywords", "dummyargument");
  Plugin* m = modules["tmp_getpluginkeywords"].get();
  std::string kw = m->Keywords();
  UnloadPlugin("tmp_getpluginkeywords");
  return kw;
}

bool PluginManager::IsProvided(const std::string property) {
  return (namedprops.count(property) > 0);
}

Plugin& PluginManager::Provider(const std::string property) {
  if (!IsProvided(property)) {
    throw InvalidRequest("Property " + property);
  } else
    return *(namedprops[property]);
}

Array<std::string> PluginManager::NamedProperties() {
  Array<std::string> tmp;
  for (std::map<std::string, Plugin*>::iterator it = namedprops.begin(); it != namedprops.end();
       ++it) {
    tmp.Append(it->first);
  }
  return tmp;
}

void PluginManager::DefineAlias(const std::string id, const std::string name) {
  aliasdict[id] = name;
}

Plugin& PluginManager::operator[](std::string id) {
  if (modules.count(id) == 0)
    throw InvalidRequest("Plugin " + id);
  else
    return *(modules[id].get());
}

const Plugin& PluginManager::operator[](std::string id) const {
  if (modules.count(id) == 0)
    throw InvalidRequest("Plugin " + id);
  const std::map<std::string, std::unique_ptr<Plugin, PluginDeleter>>::const_iterator& p =
      modules.find(id);
  return *((*p).second);
}

std::ostream& operator<<(std::ostream& os, const lpmd::PluginManager& pm) {
  const Array<std::string> plugins = pm.Plugins();
  for (int i = 0; i < plugins.Size(); ++i) {
    std::string pname = plugins[i];
    os << "Plugin " << pname << " -> ";
    os << pm[pname].Name() << " loaded.\n";
    pm[pname].Show(os);
  }
  return os;
}
