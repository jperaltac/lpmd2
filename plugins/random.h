//
//
//

#ifndef __RANDOM_FILTER_H__
#define __RANDOM_FILTER_H__

#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>
#include <lpmd/systemfilter.h>

class RandomFilter : public lpmd::SystemFilter, public lpmd::Plugin {
public:
  RandomFilter(std::string args);
  ~RandomFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet>& CreateSelector();

private:
  double percent;
  lpmd::Selector<lpmd::BasicParticleSet>* selector;
};

#endif
