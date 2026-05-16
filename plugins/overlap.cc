#include <lpmd/matrix.h>
#include <lpmd/plugin.h>
#include <lpmd/property.h>
#include <lpmd/simulation.h>
#include <lpmd/storedvalue.h>

#include <algorithm>
#include <iostream>

using namespace lpmd;

class Overlap : public lpmd::StoredValue<lpmd::Matrix>,
                public lpmd::InstantProperty,
                public lpmd::Plugin {
public:
  explicit Overlap(std::string args) : Plugin("overlap", "2.0") {
    DefineKeyword("start", "0");
    DefineKeyword("end", "-1");
    DefineKeyword("each", "1");
    DefineKeyword("output", "overlap.dat");
    DefineKeyword("rcut", "1.0");
    DefineKeyword("bins", "200");
    DefineKeyword("average", "false");
    ProcessArguments(args);
    start = int((*this)["start"]);
    end = int((*this)["end"]);
    each = int((*this)["each"]);
    cutoff = double((*this)["rcut"]);
    OutputFile() = (*this)["output"];
    reference_ready = false;
  }

  void ShowHelp() const override {
    std::cout << "Computes the fraction of atoms within rcut of their first-frame positions.\n";
  }

  void Evaluate(lpmd::Configuration& conf, lpmd::Potential&) override {
    if (!reference_ready) {
      reference.Clear();
      for (long i = 0; i < conf.Atoms().Size(); ++i)
        reference.Append(conf.Atoms()[i].Position());
      reference_ready = true;
    }
    long count = std::min<long>(conf.Atoms().Size(), reference.Size());
    long inside = 0;
    for (long i = 0; i < count; ++i)
      if ((conf.Atoms()[i].Position() - reference[i]).Module() <= cutoff)
        ++inside;
    Matrix out(2, 1);
    out.SetLabel(0, "step");
    out.SetLabel(1, "overlap");
    out.Set(0, 0, 0);
    out.Set(1, 0, count > 0 ? double(inside) / double(count) : 0.0);
    CurrentValue() = out;
  }

private:
  bool reference_ready;
  double cutoff;
  Array<Vector> reference;
};

Plugin* create(std::string args) { return new Overlap(args); }
void destroy(Plugin* m) { delete m; }
