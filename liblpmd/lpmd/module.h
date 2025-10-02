//
//
//

/**
 *  \file module.h
#include <list>
namespace lpmd {
class PluginManager; // forward declaration
class Module : public ParamList {
public:
  /// Constructor que recibe un nombre para el mdulo.
  Module(std::string modulename, bool strictkw = true);
  /// Constructor de copia
  Module(const Module& mod);
  /// Destructor.
  virtual ~Module();

  void ProcessArguments(std::string line);
  std::string GetNextWord();
  std::string GetNextWord(char);

  virtual void ShowHelp() const;

  std::string Name() const;

  PluginManager& GetManager() const;

  //
  void AssignParameter(const std::string& key, std::string value);
  void DefineKeyword(const std::string kw, const std::string defvalue);
  void DefineKeyword(const std::string kw);

  // Flujo de depuracion
  void SetDebugFile(const std::string debugfile);
  void SetDebugStream(std::ostream& ostr);
  std::ostream& DebugStream() const;

  // Operador de asignacion
  Module& operator=(const Module& mod);

  // Virtuales con implementacion por omisin
  virtual void
  Show() const; // This defaults to Show(std::cout) but it depends on the implementation of liblpmd
  virtual void Show(std::ostream& os) const;
  virtual std::string Provides() const;
  virtual double GetProperty(const std::string& name);
  virtual void SetParameter(std::string name);
  virtual std::string Keywords() const;

  // FIXME : Cambiar este metodo por un parametro extra en el constructor
  void SetManager(PluginManager& pm);

private:
  class ModuleImpl* impl;
} // namespace lpmd

   virtual void Show(std::ostream & os) const; 
   virtual std::string Provides() const;
   virtual double GetProperty(const std::string & name);
   virtual void SetParameter(std::string name);
   virtual std::string Keywords() const;

   // FIXME : Cambiar este metodo por un parametro extra en el constructor
   void SetManager(PluginManager & pm);

 private:
   class ModuleImpl * impl;
};

} // lpmd

#endif


