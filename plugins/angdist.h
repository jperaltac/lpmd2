//
//
//

#ifndef __ANGDIST_H__
#define __ANGDIST_H__

#include <lpmd/matrix.h>
#include <lpmd/storedvalue.h>
#include <lpmd/property.h>
#include <lpmd/configuration.h>
#include <lpmd/plugin.h>

#include <map>

class AngDist: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
    //Metodos Generales
    AngDist(std::string args);
    ~AngDist();

    void SetParameter(std::string name);
    void Show(std::ostream & os) const;
    void ShowHelp() const;

    //Metodos Propios del modulo angdist
    //const lpmd::Matrix & CurrentValue() const { return *m; }
    void Evaluate(lpmd::Configuration & con, lpmd::Potential & pot);

 private:
    int nb;
    std::map<std::string, double> rcut;
    int na;
    std::vector<std::string> satoms; //simbolos de los atomos.
    double trcut;
};

#endif

