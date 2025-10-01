//
//
//

#include "element.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

class ElementSelector: public Selector<BasicParticleSet>
{
 public:
   ElementSelector(std::string s) {symbol = s;}

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (ps[i].Symbol()==symbol) innerps.Append(ps[i]);
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (ps[i].Symbol()!=symbol) innerps.Append(ps[i]);
    return innerps;
   }

 private:
   std::string symbol;
   RefParticleSet innerps;
};

ElementFilter::ElementFilter(std::string args): Plugin("element", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("symbol","e");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 sym = params["symbol"];
 except = params["except"];
}

ElementFilter::~ElementFilter() { delete selector; }

void ElementFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = element                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to select atoms by their atomic symbol. It can be    \n";
 std::cout << "      called with the 'filter' or 'over' keyword.                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      symbol        : Sets the atomic symbol of the element.  .                \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying plugin :                                                            \n";
 std::cout << " filter element symbol=Ar                                                      \n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over element symbol=Kr start=0 end=1 each=1\n\n";
 std::cout << "      The plugin is used to eliminate atoms outside the specified region, in   \n";
 std::cout << "      first case, and used to apply a property (tempscaling) over the specified\n";
 std::cout << "      region, in the second case.                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & ElementFilter::CreateSelector()
{
 if (selector != 0) delete selector;
 selector = new ElementSelector(sym);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ElementFilter(args); }
void destroy(Plugin * m) { delete m; }

