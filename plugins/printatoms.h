//
//
//

#ifndef __PRINTATOMS_SM_H__
#define __PRINTATOMS_SM_H__

#include <lpmd/array.h>
#include <lpmd/visualizer.h>
#include <lpmd/plugin.h>
#include <string>

class PrintAtomsVisualizer: public lpmd::Visualizer, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  PrintAtomsVisualizer(std::string args);
  ~PrintAtomsVisualizer();
  void ShowHelp() const;

  //Metodos Propios de modulo printatoms
  void Apply(const lpmd::Simulation & sim);

 private:
   lpmd::Array<std::string> tags;
};

#endif



