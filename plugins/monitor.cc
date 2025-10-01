//
//
//

#include "monitor.h"

#include <lpmd/simulation.h>

#include <iostream>
#include <fstream>

using namespace lpmd;

MonitorVisualizer::MonitorVisualizer(std::string args): Plugin("monitor", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("properties", "step");
 DefineKeyword("output", "-");
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 if (params["output"] != "-") output_stream = new std::ofstream(params["output"].c_str());
 else output_stream = &(std::cout);
 property_array = StringSplit(params["properties"], ',');
}

MonitorVisualizer::~MonitorVisualizer()
{ 
 ParamList & params = (*this);
 if (params["output"] != "-") delete output_stream;
}

void MonitorVisualizer::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = monitor                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugin is used to export physical properties of the sample to a file.\n";
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
 std::cout << "      properties    : Sets the properties to monitor.                          \n";
 std::cout << "      output        : Output file.                                             \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " monitor properties=temperature,sxx start=0 end=-1 each=50 output=prop.dat     \n";
 std::cout << " monitor temperature,sxx start=0 end=-1 each=50 output=out.dat               \n\n";
 std::cout << "      The plugin is used to export the temperature and the xx component of the \n";
 std::cout << "      the stress tensor (sxx) in the file prop.dat during all the simulation   \n";
 std::cout << "      (end=-1), each 50 simulation steps.                                      \n";
 std::cout << "      As you can see, 'properties=' can be ommited.                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void MonitorVisualizer::Apply(const Simulation & sim)
{ 
 for (int p=0;p<property_array.Size();++p) 
     (*output_stream) << sim.GetTag(sim, Tag(property_array[p])) << "  ";
 (*output_stream) << std::endl;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new MonitorVisualizer(args); }
void destroy(Plugin * m) { delete m; }

