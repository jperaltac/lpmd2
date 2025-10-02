//
//
//

#ifndef __MONITOR_VIS_H__
#define __MONITOR_VIS_H__

#include <iostream>
#include <lpmd/array.h>
#include <lpmd/plugin.h>
#include <lpmd/visualizer.h>

class MonitorVisualizer : public lpmd::Visualizer, public lpmd::Plugin {
public:
  MonitorVisualizer(std::string args);
  ~MonitorVisualizer();
  void ShowHelp() const;

  void Apply(const lpmd::Simulation& sim);

private:
  std::ostream* output_stream;
  lpmd::Array<std::string> property_array;
};

#endif
