#ifndef __LPMD_RUNTIME_CONTEXT_H__
#define __LPMD_RUNTIME_CONTEXT_H__

#include <lpmd/session.h>
#include <runtime/units.h>

namespace lpmd {
class RuntimeContext {
public:
  explicit RuntimeContext(const UnitSystem& units = DefaultUnitSystem());

  Session& session() { return session_; }
  const Session& session() const { return session_; }

  const UnitSystem& units() const { return units_; }

private:
  UnitSystem units_;
  Session session_;
};

class RuntimeContextScope {
public:
  explicit RuntimeContextScope(RuntimeContext& context);
  ~RuntimeContextScope();

  RuntimeContextScope(const RuntimeContextScope&) = delete;
  RuntimeContextScope& operator=(const RuntimeContextScope&) = delete;

private:
  RuntimeContext* previous_;
};

RuntimeContext& CurrentContext();
void SetCurrentContext(RuntimeContext& context);
void ClearCurrentContext();
bool HasCurrentContext();

} // namespace lpmd

#endif
