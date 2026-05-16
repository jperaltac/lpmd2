#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/simulation.h>
#include <lpmd/storedvalue.h>

#include <algorithm>
#include <iostream>

using namespace lpmd;

class Mobility : public lpmd::StoredValue<lpmd::Matrix>,
                 public lpmd::InstantProperty,
                 public lpmd::Plugin {
public:
  explicit Mobility(std::string args) : Plugin("mobility", "2.0") {
    DefineKeyword("start", "0");
    DefineKeyword("end", "-1");
    DefineKeyword("each", "1");
    DefineKeyword("output", "mobility.dat");
    DefineKeyword("rcutmin", "0.0");
    DefineKeyword("rcutmax", "0.0");
    ProcessArguments(args);
    start = int((*this)["start"]);
    end = int((*this)["end"]);
    each = int((*this)["each"]);
    OutputFile() = (*this)["output"];
    reference_ready = false;
  }

  void ShowHelp() const override {
    std::cout << "Computes a simple per-frame mean displacement from the first configuration.\n";
  }

  void Evaluate(lpmd::Configuration& conf, lpmd::Potential&) override {
    if (!reference_ready) {
      reference.Clear();
      for (long i = 0; i < conf.Atoms().Size(); ++i)
        reference.Append(conf.Atoms()[i].Position());
      reference_ready = true;
    }
    double sum = 0.0;
    long n = conf.Atoms().Size();
    long count = std::min<long>(n, reference.Size());
    for (long i = 0; i < count; ++i)
      sum += (conf.Atoms()[i].Position() - reference[i]).Module();
    Matrix out(2, 1);
    out.SetLabel(0, "step");
    out.SetLabel(1, "mean-displacement");
    out.Set(0, 0, 0);
    out.Set(1, 0, count > 0 ? sum / double(count) : 0.0);
    CurrentValue() = out;
  }

private:
  bool reference_ready;
  Array<Vector> reference;
};

Plugin* create(std::string args) { return new Mobility(args); }
void destroy(Plugin* m) { delete m; }
