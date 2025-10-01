//
//
//

#ifndef __EXTERNAL_FILTER_H__
#define __EXTERNAL_FILTER_H__

#include <lpmd/systemfilter.h>
#include <lpmd/atomselector.h>
#include <lpmd/plugin.h>

class ExternalFilter: public lpmd::SystemFilter, public lpmd::Plugin
{
 public:
  ExternalFilter(std::string args);
  ~ExternalFilter();

  void ShowHelp() const;

  lpmd::Selector<lpmd::BasicParticleSet> & CreateSelector();

  private:
   lpmd::Selector<lpmd::BasicParticleSet> * selector;
   double vmin, vmax;
   int column, extheader;
   std::ifstream * extfile;
};

#endif

