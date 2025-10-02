/*
 * Simple plugin used to stress-test the container interfaces exposed by LPMD.
 */

#ifndef __LPMD_TEST_PLUGIN_H__
#define __LPMD_TEST_PLUGIN_H__

#include <lpmd/plugin.h>
#include <lpmd/simulation.h>

class Test: public lpmd::Plugin
{
 public:
    explicit Test(std::string args);
    ~Test() override;

    void ShowHelp() const override;

    void PerformTest(lpmd::Simulation & sim);

 private:
    int iterations_;
    long atoms_per_iteration_;
};

#endif

