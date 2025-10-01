//
//
//

#include "external.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>
#include <fstream>

using namespace lpmd;

class ExternalSelector: public Selector<BasicParticleSet>
{
 public:
   ExternalSelector(std::ifstream * extfile, int column, int extheader, double vmin, double vmax)
   { 
    ef = extfile;
    col = column;
    header = extheader;
    this->vmin = vmin;
    this->vmax = vmax;
   }

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    // Manage header line(s) for external mode
    std::string line;
    for (int k=0;k<header;k++) getline(*ef, line);
    //
    for (long int i=0;i<ps.Size();++i)
    {
     getline(*ef, line);
     Array<std::string> lspl = StringSplit(line);
     double v = strtod(lspl[col-1].c_str(), 0);
     if ((v >= vmin) && (v <= vmax)) innerps.Append(ps[i]);
    }
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    // Manage header line(s) for external mode
    std::string line;
    for (int k=0;k<header;k++) getline(*ef, line);
    //
    for (long int i=0;i<ps.Size();++i) 
    {
     getline(*ef, line);
     Array<std::string> lspl = StringSplit(line);
     double v = strtod(lspl[col-1].c_str(), 0);
     if (!((v >= vmin) && (v <= vmax))) innerps.Append(ps[i]);
    }
    return innerps;
   }

 private:
   int col, header;
   std::ifstream * ef;
   RefParticleSet innerps;
   double vmin, vmax;
};

ExternalFilter::ExternalFilter(std::string args): Plugin("external", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("min", "0.0");
 DefineKeyword("max", "1.0");
 DefineKeyword("extfile");
 DefineKeyword("extcolumn", "2");
 DefineKeyword("extheader", "1");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 except = params["except"];
 vmin = double(params["min"]);
 vmax = double(params["max"]);
 extfile = new std::ifstream(params["extfile"].c_str());
 column = int(params["extcolumn"]);
 extheader = int(params["extheader"]);
}

ExternalFilter::~ExternalFilter() 
{ 
 delete extfile;
 delete selector; 
}

void ExternalFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = external                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to filter atoms by some external property stored in  \n";
 std::cout << "      a file.                                                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      extfile       : Specify the file where the properties are stored.        \n";
 std::cout << "      extcolumn     : Choose the column that has the data to check for filter. \n";
 std::cout << "      extheader     : HELP NOT AVAILABLE YET                                   \n";
 std::cout << "      min           : Minimum value of the property.                           \n";
 std::cout << "      max           : Maximum value of the property.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " filter external extfile=datos.dat extcolumn=3 extheader=1 min=2.0 max=3.0     \n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over external extfile=datos.dat extcolumn=3 extheader=1 min=2.0 max=3.0\n\n";
 std::cout << "      The plugin is used, in the first case, to eliminate (filter) all the     \n";
 std::cout << "      atoms that has a value of the property, indicated by the third column    \n";
 std::cout << "      of datos.dat, between the values 2.0 and 3.0. The external file datos.dat\n";
 std::cout << "      has a comment in the first line, indicated by keyword extheader=1.       \n";
 std::cout << "      In the second case, instead of eliminate atoms, a property (tempscaling) \n";
 std::cout << "      is applied to the atoms indicated by the file.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & ExternalFilter::CreateSelector()
{
 if (selector != 0) delete selector;
 selector = new ExternalSelector(extfile, column, extheader, vmin, vmax);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ExternalFilter(args); }
void destroy(Plugin * m) { delete m; }

