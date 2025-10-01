//
//
//

#ifndef __RANDOM_FILTER_H__
#define __RANDOM_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class RandomFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  RandomFilter(std::string args);
  ~RandomFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   double percent;
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
};

#endif

