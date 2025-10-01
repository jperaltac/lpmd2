//
//
//

#include "nullmixer.h"

#include <iostream>

using namespace lpmd;

NullMixer::NullMixer(std::string args): Plugin("nullmixer", "1.0")
{
 ProcessArguments(args);
}

NullMixer::~NullMixer() { }

void NullMixer::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = nullmixer                                                \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin implements ...                                               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      None.                                                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use nullmixer                                                                 \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply nullmixer                                                             \n\n";
 std::cout << "      The plugin implements ...                                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Configuration & NullMixer::Apply(Configuration & config1, Configuration & config2)
{
 assert(&config2 != 0); //icc 869
 return config1;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new NullMixer(args); }
void destroy(Plugin * m) { delete m; }

