#include <runtime/runtime_context.h>

#include <stdexcept>

namespace lpmd
{
 namespace
 {
  thread_local RuntimeContext * current_context = 0;
 }

 RuntimeContext::RuntimeContext(const UnitSystem & units): units_(units), session_(units_)
 {
 }

 RuntimeContext & CurrentContext()
 {
  if (current_context == 0)
  {
   throw std::runtime_error("No runtime context has been set for the current thread");
  }
  return *current_context;
 }

 void SetCurrentContext(RuntimeContext & context)
 {
  current_context = &context;
 }

 void ClearCurrentContext()
 {
  current_context = 0;
 }

 bool HasCurrentContext()
 {
  return current_context != 0;
 }

 RuntimeContextScope::RuntimeContextScope(RuntimeContext & context)
 {
  previous_ = current_context;
  current_context = &context;
 }

 RuntimeContextScope::~RuntimeContextScope()
 {
  current_context = previous_;
 }
}
