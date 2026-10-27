#include <lpunit/macros.h>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
void Require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

void Pass() {}
void Fail() { lpunit::Assert(false); }
void Throw() { throw std::runtime_error("test body"); }

class LifecycleTest : public lpunit::SimpleTest {
public:
  explicit LifecycleTest(void (*body)()) : SimpleTest("lifecycle", body) {}
  void Setup() override { ++setups; }
  void Teardown() override { ++teardowns; }
  int setups = 0, teardowns = 0;
};
}

int main(int argc, char** argv) {
  try {
    const std::string mode = argc > 1 ? argv[1] : "lifecycle";
    if (mode == "lifecycle") {
      LifecycleTest success(Pass);
      Require(success.Check(), "successful test reported failure");
      Require(success.setups == 1 && success.teardowns == 1, "successful test must clean up exactly once");
      LifecycleTest failure(Fail);
      Require(!failure.Check(), "failed assertion reported success");
      Require(failure.teardowns == 1, "failed test must clean up exactly once");
      LifecycleTest exception(Throw);
      bool thrown = false;
      try { exception.Check(); } catch (const std::runtime_error&) { thrown = true; }
      Require(thrown && exception.teardowns == 1, "unexpected exceptions must also clean up");
    } else if (mode == "nan") {
      bool failed = false;
      const double nan = std::numeric_limits<double>::quiet_NaN();
      try { LPUNIT_APPROX(nan, 1.0, "nan", "1.0", 1e-8); }
      catch (const lpunit::TestFailed&) { failed = true; }
      Require(failed, "NaN incorrectly passed an approximate comparison");
    } else {
      throw std::runtime_error("unknown test case");
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
