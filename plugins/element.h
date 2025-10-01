//
//
//

#ifndef __ELEMENT_FILTER_H__
#define __ELEMENT_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class ElementFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  ElementFilter(std::string args);
  ~ElementFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   std::string sym;
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
};

#endif

