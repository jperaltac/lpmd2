//
//
//

#ifndef __TAG_FILTER_H__
#define __TAG_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class TagFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  TagFilter(std::string args);
  ~TagFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

 private:
  std::string name, value;
  lpmd::Selector<lpmd::BasicParticleSet> * selector;
};

#endif

