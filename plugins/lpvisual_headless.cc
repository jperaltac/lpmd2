#include <lpmd/plugin.h>
#include <lpmd/simulation.h>
#include <lpmd/visualizer.h>

#include <iostream>

using namespace lpmd;

class LPVisualHeadless : public lpmd::Visualizer, public lpmd::Plugin {
public:
  explicit LPVisualHeadless(std::string args) : Plugin("lpvisual", "2.0") {
    DefineKeyword("start", "0");
    DefineKeyword("end", "-1");
    DefineKeyword("each", "1");
    DefineKeyword("width", "640");
    DefineKeyword("height", "480");
    DefineKeyword("radius", "0.5");
    DefineKeyword("quality", "2");
    DefineKeyword("azimuth", "0.0");
    DefineKeyword("zenith", "0.0");
    DefineKeyword("mark", "-1");
    DefineKeyword("paused", "false");
    DefineKeyword("autorotate", "false");
    DefineKeyword("background", "black");
    DefineKeyword("graphbg", "white");
    DefineKeyword("perspective", "true");
    DefineKeyword("properties", "total-energy,temperature,pressure");
    DefineKeyword("plot", "temperature");
    DefineKeyword("xrange", "");
    DefineKeyword("yrange", "");
    DefineKeyword("camerapos", "");
    DefineKeyword("cameraobj", "");
    DefineKeyword("cameraup", "");
    ProcessArguments(args);
    start = int((*this)["start"]);
    end = int((*this)["end"]);
    each = int((*this)["each"]);
  }

  void ShowHelp() const override {
    std::cout << "lpvisual headless fallback: accepts lpvisual options but does not open an OpenGL "
                 "window.\n";
  }

  void Apply(const lpmd::Simulation&) override {}
};

Plugin* create(std::string args) { return new LPVisualHeadless(args); }
void destroy(Plugin* m) { delete m; }
