/*
 *
 *
 *
 */

#ifndef __LPMDUTIL_VISUALIZER_H__
#define __LPMDUTIL_VISUALIZER_H__

#include "application.h"
#include <iostream>

class VisualizerControl : public UtilityControl {
public:
  VisualizerControl(PluginManager& pm) : UtilityControl(pm) {
    DeclareStatement("antialias", "value");
    DeclareStatement("warning", "value");
    DeclareStatement("cameraLocation", "value");
    DeclareStatement("cameraUp", "value");
    DeclareStatement("cameraLookat", "value");
    DeclareStatement("camera", "value");
    DeclareStatement("cameraLight", "value");
    DeclareStatement("cameraRotate", "center axis angle");
    DeclareStatement("startframes", "value");
    DeclareStatement("finalframes", "value");
    DeclareStatement("startrotate", "value");
    DeclareStatement("endrotate", "value");
    DeclareStatement("background", "value");
    DeclareStatement("radius", "value");
    DeclareStatement("atomcolor", "value");
    DeclareStatement("box", "radius color");
    DeclareStatement("povfiles", "value");
    DeclareStatement("size", "value");
    DeclareStatement("logo", "position text");
    DeclareStatement("format", "value");
    DeclareStatement("movie", "fps file type");
    DeclareStatement("extraLight", "value");
    DeclareStatement("plane1", "color alpha");
    DeclareStatement("plane2", "color alpha");
    DeclareStatement("plane3", "color alpha");
    DeclareStatement("plane4", "color alpha");
    DeclareStatement("plane5", "color alpha");
    DeclareStatement("plane6", "color alpha");
    (*this)["replacecell"] = "true";
  }
};

class Visualizer : public Application {
public:
  Visualizer(int argc, const char* argv[]);
  ~Visualizer();

  int Run();
  void FillAtoms();
  void Iterate();
  void IterateSequential();
  void IterateReplay();

private:
  VisualizerControl control;
};

#endif
