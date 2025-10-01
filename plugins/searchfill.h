//
//
//

#ifndef __SEARCHFILL_H__
#define __SEARCHFILL_H__

#include <lpmd/systemmodifier.h>
#include <lpmd/configuration.h>
#include <lpmd/plugin.h>

#include <map>

class SearchFill: public lpmd::SystemModifier, public lpmd::Plugin
{
 public:
    //Metodos Generales
    SearchFill(std::string args);
    ~SearchFill();

    //void Show(std::ostream & os) const;
    void ShowHelp() const;

    //Metodos Propios del modulo angdist
    //const lpmd::Matrix & CurrentValue() const { return *m; }
    void Apply(lpmd::Configuration & con);
    void Apply(lpmd::Simulation & md);

 private:
    double R0,K,VACOVP,GoodEn;
    int MaxSteps,vacsymbol,boundary,VACMAX,RiskSteps;
    int min_coord,max_coord,RiskLimit;
};

#endif

