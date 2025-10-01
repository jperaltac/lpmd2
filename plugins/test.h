/*
 *
 *
 *
 */

#include <lpmd/plugin.h>
#include <lpmd/simulation.h>

using namespace lpmd;

class Test: public Module
{
 public:
    Test(std::string args);
    ~Test();
    
    void PerformTest(Simulation & s);
};

