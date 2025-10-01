//
//
//

#ifndef __CYLINDER_FILTER_H__
#define __CYLINDER_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class CylinderFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  CylinderFilter(std::string args);
  ~CylinderFilter();

  void ShowHelp() const;

  void Update(lpmd::Simulation & sim);
  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   lpmd::Vector S, origin;
   double rmax,rmin;
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
   lpmd::BasicCell *mycell;
};

#endif

