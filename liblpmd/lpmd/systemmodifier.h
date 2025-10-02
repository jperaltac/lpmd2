//
//
//

#ifndef __LPMD_SYSTEMMODIFIER_H__
#define __LPMD_SYSTEMMODIFIER_H__

#include <lpmd/simulation.h>
#include <lpmd/stepper.h>

namespace lpmd {
class SystemModifier : public Stepper {
public:
  //
  SystemModifier();
  virtual ~SystemModifier();

  virtual void Apply(Simulation& sim) = 0;
};

} // namespace lpmd

#endif
