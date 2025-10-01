//
//
//

#ifndef __CONE_FILTER_H__
#define __CONE_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class ConeFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  ConeFilter(std::string args);
  ~ConeFilter();

  void ShowHelp() const;

  void Update(lpmd::Simulation & sim);
  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   double alpha, beta;
   lpmd::Vector tip, bot;
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
   lpmd::BasicCell * mycell;
};

#endif

