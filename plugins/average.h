//
//
//

#ifndef __AVERAGE_VIS_H__
#define __AVERAGE_VIS_H__

#include <iostream>
#include <lpmd/visualizer.h>
#include <lpmd/plugin.h>
#include <lpmd/array.h>

class AverageVisualizer: public lpmd::Visualizer, public lpmd::Plugin
{
 public:
   AverageVisualizer(std::string args);
   ~AverageVisualizer();
   void ShowHelp() const;

   void Apply(const lpmd::Simulation & sim);

 private:
   std::ostream * output_stream; 
   long int interval, average_count;
   lpmd::Array<std::string> property_array;
   double * property_averages, * last_value;
};

#endif

