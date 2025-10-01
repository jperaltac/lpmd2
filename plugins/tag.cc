//
//
//

#include "tag.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

class TagSelector: public Selector<BasicParticleSet>
{
 public:
   TagSelector(std::string n,std::string v) {name = n;value =v;}

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i)
    { 
     if (ps.Have(ps[i],Tag(name)) && (ps.GetTag(ps[i], Tag(name))==value)) innerps.Append(ps[i]);
    }
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
    {
     if (!ps.Have(ps[i],Tag(name)) || !(ps.GetTag(ps[i], Tag(name))==value)) innerps.Append(ps[i]);
    }
    return innerps;
   }

 private:
   std::string name, value;
   RefParticleSet innerps;
};

TagFilter::TagFilter(std::string args): Plugin("tag", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("name","fixedvel");
 DefineKeyword("value","true");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 name = params["name"];
 value = params["value"];
 except = params["except"];
}

TagFilter::~TagFilter() { delete selector; }

void TagFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = tag                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to apply properties or plugins over certain atoms    \n";
 std::cout << "      of the simulation cell, identified by a label put by the settag plugin.  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      name          : Sets the name of the tag.                                \n";
 std::cout << "      value         : Sets the initial status of the tag (true / false).       \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use settag as floor                                                           \n";
 std::cout << "  tag floor_atoms                                                              \n";
 std::cout << "  value true                                                                   \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " use setcolor as red                                                           \n";
 std::cout << "  color <1.0,0.0,0.0>                                                          \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply floor over box x=0-10 y=15-20 z=5-10                                    \n"; 
 std::cout << " apply red over tag name=floor_atoms value=true start=100 end=200 each=1     \n\n"; 
 std::cout << "      The 'settag' plugin (alias 'floor') is used first to put the label to    \n";
 std::cout << "      the atoms in the box [0,10]x[15,20]x[5,10]. These atoms can be identified\n";
 std::cout << "      now as 'floor_atoms'. Then, the 'setcolor' plugin (alias 'red') is applied\n";
 std::cout << "      over the atoms that have the tag 'floor_atoms' with value 'true'.        \n";
 std::cout << "      The difference between applying the 'settag' plugin over a box and then  \n";
 std::cout << "      the 'setcolor' plugin over the tagged atoms in the box, and applying the \n";
 std::cout << "      'setcolor' plugin directly to the box, is that once the atoms are labeled,\n";
 std::cout << "      they keep its 'name' during the simulation, so a plugin can be applied to\n";
 std::cout << "      then at any time in the simulation, regardless of their position.        \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & TagFilter::CreateSelector()
{
 if (selector != 0) delete selector;
 selector = new TagSelector(name,value);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TagFilter(args); }
void destroy(Plugin * m) { delete m; }

