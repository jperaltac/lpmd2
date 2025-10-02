/*
 *
 *
 *
 */

#ifndef __LPMD_ATOMSELECTOR_H__
#define __LPMD_ATOMSELECTOR_H__

#include <lpmd/basicparticleset.h>
#include <lpmd/region.h>
#include <lpmd/selector.h>
#include <lpmd/taghandler.h>

namespace lpmd {

class AtomSelector {
public:
  AtomSelector();

  ~AtomSelector();

  static Selector<BasicParticleSet>& WithZ(int z);

  static Selector<BasicParticleSet>& WithTag(Tag t);

  static Selector<BasicParticleSet>& Inside(const Region& r);

private:
  static class AtomSelectorImpl impl;
};

} // namespace lpmd

#endif
