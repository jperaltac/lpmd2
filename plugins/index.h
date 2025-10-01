//
//
//

#ifndef __INDEX_FILTER_H__
#define __INDEX_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class IndexFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  IndexFilter(std::string args);
  ~IndexFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector(); 

 private:
  std::string file;
  lpmd::Array<std::string> index;
  lpmd::Selector<lpmd::BasicParticleSet> * selector;
};

#endif

