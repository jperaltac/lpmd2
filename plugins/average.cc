//
//
//

#include "average.h"

#include <lpmd/simulation.h>

#include <iostream>
#include <fstream>

using namespace lpmd;

AverageVisualizer::AverageVisualizer(std::string args): Plugin("average", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("properties", "step");
 DefineKeyword("output", "-");
 DefineKeyword("interval", "100");
 DefineKeyword("debug", "none");
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 if (params["output"] != "-") output_stream = new std::ofstream(params["output"].c_str());
 else output_stream = &(std::cout);
 interval = int(params["interval"]);
 property_array = StringSplit(params["properties"], ',');
 property_averages = new double[property_array.Size()];
 last_value = new double[property_array.Size()];
 for (int i=0;i<property_array.Size();++i) property_averages[i] = last_value[i] = 0.0;
 average_count = 0;
}

AverageVisualizer::~AverageVisualizer()
{ 
 ParamList & params = (*this);
 if (params["output"] != "-") delete output_stream;
 delete [] property_averages;
 delete [] last_value;
}

void AverageVisualizer::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = average                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to export averaged physical properties of the sample  \n";
 std::cout << "      to a file. The properties are averaged each 'interval' steps.            \n";
 std::cout << "      Available properties :                                                   \n";
 std::cout << "         step, temperature, volume, volume-per-atom,                           \n";
 std::cout << "         cell-a, cell-b, cell-c, particle-density,                             \n";
 std::cout << "         density, momentum, px, py, pz,                                        \n";
 std::cout << "         potential-energy, kinetic-energy, total-energy,                       \n";
 std::cout << "         virial-pressure, kinetic-pressure, pressure                           \n";
 std::cout << "         sxx, sxy, sxz, syx, syy, syz, szx, szy, szz.                          \n";
 std::cout << "      For a detailed information about units, refeer to the 'General Properties'\n";
 std::cout << "      section of the lpmd's manual.                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      properties    : Sets the properties to average.                        . \n";
 std::cout << "      output        : Output file.                                             \n";
 std::cout << "      interval      : Sets the interval in which the properties will be averaged.\n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " average properties=temperature,sxx interval=100 start=0 end=-1 each=50 output=prop.dat\n";
 std::cout << " average temperature,sxx interval=100 start=0 end=-1 each=500 output=out.dat  \n\n";
 std::cout << "      The plugin is used to export the 100-steps-averaged temperature and the  \n";
 std::cout << "      100-steps-averaged xx component of the the stress tensor (sxx) in the    \n";
 std::cout << "      file prop.dat during all the simulation (end=-1), each 50 simulation steps.\n";
 std::cout << "      As you can see, 'properties=' can be ommited.                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void AverageVisualizer::Apply(const Simulation & sim)
{ 
 for (int p=0;p<property_array.Size();++p) 
 {
  double value = double(Parameter(sim.GetTag(sim, Tag(property_array[p]))));
  if (average_count < interval)
  {
   property_averages[p] += value;
  }
  else
  {
   property_averages[p] -= last_value[p];
   property_averages[p] += value;
  }
  (*output_stream) << property_averages[p]/double(average_count) << " ";
  last_value[p] = value;
 }
 (*output_stream) << '\n';
 if (average_count < interval) average_count++;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AverageVisualizer(args); }
void destroy(Plugin * m) { delete m; }

