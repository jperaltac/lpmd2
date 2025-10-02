//
//
//

#ifndef __LPMD_SESSION_H__
#define __LPMD_SESSION_H__

#include <lpmd/module.h>
#include <runtime/units.h>

namespace lpmd {

class Session : public Module {
public:
  //
  explicit Session(const UnitSystem& units = DefaultUnitSystem());
  ~Session();

private:
  class SessionImpl* simpl;
};

} // namespace lpmd

#endif
