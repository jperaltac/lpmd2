//
//
//

#include "random.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

class RandomSelector: public Selector<BasicParticleSet>
{
 public:
   RandomSelector(double percent) {p=percent;}

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    lpmd::Array<int> random;
    random.Clear();
    innerps.Clear();
    //generate a random list.
    int tochange = (int)(ps.Size()*p/100.0);
    std::cerr << "-> Atoms not deleted = " << tochange << '\n';
    while(random.Size()<tochange)
    {
      int rnd = int(drand48()*ps.Size());
      if (rnd!=0) random.AppendUnique(rnd);
    }
    std::cerr << "-> random Size = " << random.Size() << '\n';
    for(int i=0;i<random.Size();++i)
    {
     innerps.Append(ps[random[i]]);
    }
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   {
    lpmd::Array<int> random;
    random.Clear(); 
    innerps.Clear();
    //generate a random list
    int tochange = (int)ps.Size()-(int)(ps.Size()*p/100.0);
    std::cerr << "-> Atoms to delete = " << tochange << '\n';
    while(random.Size()<tochange)
    {
     int rnd = int(drand48()*ps.Size());
     if (rnd!=0) random.AppendUnique(rnd);
    }
    for(int i=0;i<random.Size();++i)
    {
     innerps.Append(ps[random[i]]);
    }
    return innerps;
   }

 private:
   double p;
   RefParticleSet innerps;
};

RandomFilter::RandomFilter(std::string args): Plugin("random", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("percent");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 percent = double(params["percent"]);
 except = params["except"];
}

RandomFilter::~RandomFilter() { delete selector; }

void RandomFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = random                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to delete randomly a percentage of atoms of the      \n";
 std::cout << "      simulation cell. This plugin is a filter, so it is supposed to used      \n";
 std::cout << "      with the 'filter' statement (see also 'randomatom' plugin).              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      percent       : Sets the percentage of atoms that will NOT be deleted.   \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " filter random percent=90                                                      \n";
 std::cout << " apply tempscaling over random percent=90 start=0 end=1 each=1               \n\n";
 std::cout << "      The plugin is used to eliminate, randomly, 10\% of the atoms of the      \n";
 std::cout << "      simulation cell, and to apply a property (tempscaling) over the 10\% of  \n";
 std::cout << "      the atoms of the simulation cell (selected randomly).                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & RandomFilter::CreateSelector()
{
 if (selector != 0) delete selector;
 selector = new RandomSelector(percent);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new RandomFilter(args); }
void destroy(Plugin * m) { delete m; }

