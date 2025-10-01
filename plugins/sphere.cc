//
//
//

#include "sphere.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

void SphereFilter::Update(lpmd::Simulation & sim) { mycell = &(sim.Cell()); }

class SphereSelector: public Selector<BasicParticleSet>
{
 public:
   SphereSelector(const Sphere & sph): sphere(sph) { }

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (sphere.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (!sphere.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

 private:
   Sphere sphere;
   RefParticleSet innerps;
};

SphereFilter::SphereFilter(std::string args): Plugin("sphere", "2.1"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("radius");
 DefineKeyword("center");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 radius = double(params["radius"]);
 center = Vector(params["center"].c_str());
 except = params["except"];
}

SphereFilter::~SphereFilter() { delete selector; }

void SphereFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = sphere                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to select a spherical region of the simulation cell. \n";
 std::cout << "      You can assign properties or tags to this region.  It can be called with \n";
 std::cout << "      the 'filter' or 'over' keyword.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      radius        : Set the radius value of the sphere.                      \n";
 std::cout << "      center        : Set the vector center of the sphere.                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying plugin :                                                            \n";
 std::cout << " filter sphere radius=5.0 center=<10.0,10.0,10.0>                              \n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over sphere radius=5.0 center=<10.0,10.0,10.0>\n\n";
 std::cout << "      The plugin is used to eliminate atoms outside the specified spherical    \n";
 std::cout << "      region in the first case, and used to apply a property (tempscaling) over\n";
 std::cout << "      the specified region, in the second case.                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << '\n';
}

Selector<BasicParticleSet> & SphereFilter::CreateSelector()
{
 //rellocate the center based in the cell-vectors
 lpmd::Vector a = (*mycell)[0]; double ma = (*mycell)[0].Module();
 lpmd::Vector b = (*mycell)[1]; double mb = (*mycell)[1].Module();
 lpmd::Vector c = (*mycell)[2]; double mc = (*mycell)[2].Module();
 lpmd::Vector newcenter(center[0]*a/ma+center[1]*b/mb+center[2]*c/mc);
 Sphere spheric_region(newcenter, radius);
 if (selector != 0) delete selector;
 selector = new SphereSelector(spheric_region);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new SphereFilter(args); }
void destroy(Plugin * m) { delete m; }

