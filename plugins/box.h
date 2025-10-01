//
//
//

#ifndef __BOX_FILTER_H__
#define __BOX_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class BoxFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  BoxFilter(std::string args);
  ~BoxFilter();

  void ShowHelp() const;
  void Update(lpmd::Simulation & sim);

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   double x[2],y[2],z[2];
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
   lpmd::BasicCell *mycell;
};

#endif

