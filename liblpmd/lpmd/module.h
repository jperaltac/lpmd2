#ifndef LPMD_MODULE_H
#define LPMD_MODULE_H

#include <iosfwd>
#include <string>

#include <lpmd/paramlist.h>

namespace lpmd {

class PluginManager;

class Module : public ParamList {
public:
  Module(std::string modulename, bool strictkw = true);
  Module(const Module& mod);
  virtual ~Module();

  Module& operator=(const Module& mod);

  void ProcessArguments(std::string line);
  std::string GetNextWord();
  std::string GetNextWord(char delimiter);

  virtual void ShowHelp() const;

  std::string Name() const;

  PluginManager& GetManager() const;

  void AssignParameter(const std::string& key, std::string value);
  void DefineKeyword(const std::string kw, const std::string defvalue);
  void DefineKeyword(const std::string kw);

  void SetDebugFile(const std::string debugfile);
  void SetDebugStream(std::ostream& ostr);
  std::ostream& DebugStream() const;

  virtual void Show() const;
  virtual void Show(std::ostream& os) const;
  virtual std::string Provides() const;
  virtual double GetProperty(const std::string& name);
  virtual void SetParameter(std::string name);
  virtual std::string Keywords() const;

  void SetManager(PluginManager& pm);

private:
  class ModuleImpl* impl;
};

} // namespace lpmd

#endif // LPMD_MODULE_H
