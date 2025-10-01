//
//
//

#include "cone.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

void ConeFilter::Update(lpmd::Simulation & sim) { mycell = &(sim.Cell()); }

class ConeSelector: public Selector<BasicParticleSet>
{
 public:
   ConeSelector(const Cone & cn): cone(cn) { }

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (cone.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (!cone.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

 private:
   Cone cone;
   RefParticleSet innerps;
};

ConeFilter::ConeFilter(std::string args): Plugin("cone", "2.1"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("alpha");
 DefineKeyword("beta","0.0");
 DefineKeyword("bot");
 DefineKeyword("tip");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 alpha = double(params["alpha"]);
 beta = double(params["beta"]);
 bot = Vector(params["bot"].c_str());
 tip = Vector(params["tip"].c_str());
 except = params["except"];
}

ConeFilter::~ConeFilter() { delete selector; }

void ConeFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = cone                                                     \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to select a group of atoms in a conical region.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      bot           : Set the bottom position vector of the cone.              \n";
 std::cout << "      tip           : Set the tip position vector of the cone.                 \n";
 std::cout << "      alpha         : Set the alpha angle of the cone.                         \n";
 std::cout << "      beta          : Set internatl beta angle, by default is zero.            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying plugin :                                                            \n";
 std::cout << " filter cone tip=<25,25,25> bot=<25,25,0> alpha=45                             \n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over cone tip=<25,25,25> bot=<25,25,0> alpha=45\n\n";
 std::cout << "      The plugin is used to eliminate atoms outside the specified region, in   \n";
 std::cout << "      first case, and used to apply a property (tempscaling) over the specified\n";
 std::cout << "      region, in the second case.                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & ConeFilter::CreateSelector()
{
 //Set tip and both using cell-vectors
 lpmd::Vector a = (*mycell)[0]; double ma = (*mycell)[0].Module();
 lpmd::Vector b = (*mycell)[1]; double mb = (*mycell)[1].Module();
 lpmd::Vector c = (*mycell)[2]; double mc = (*mycell)[2].Module();
 lpmd::Vector newtip(tip[0]*a/ma+tip[1]*b/mb+tip[2]*c/mc);
 lpmd::Vector newbot(bot[0]*a/ma+bot[1]*b/mb+bot[2]*c/mc);
 Cone cone_region(newtip, newbot, alpha, beta);
 if (selector != 0) delete selector;
 selector = new ConeSelector(cone_region);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new ConeFilter(args); }
void destroy(Plugin * m) { delete m; }

