//
//
//

#ifndef __NULLMIXER_H__
#define __NULLMIXER_H__

#include <lpmd/systemmixer.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class NullMixer: public SystemMixer, public Plugin
{
 public:

  //Metodos Generales 
  NullMixer(std::string args);
  ~NullMixer();
  void ShowHelp() const;

  //Metodos Propios de modulo nullmixer
  Configuration & Apply(Configuration & config1, Configuration & config2);
};

#endif

