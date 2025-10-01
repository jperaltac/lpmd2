//
//
//

#include "cylinder.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

void CylinderFilter::Update(lpmd::Simulation & sim) { mycell = &(sim.Cell()); }

class CylinderSelector: public Selector<BasicParticleSet>
{
 public:
   CylinderSelector(const Cylinder & b): cyl(b) { }

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (cyl.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (!cyl.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

 private:
   Cylinder cyl;
   RefParticleSet innerps;
};

CylinderFilter::CylinderFilter(std::string args): Plugin("cylinder", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("rmax","0");
 DefineKeyword("rmin","0");
 DefineKeyword("origin","<0,0,0>");
 DefineKeyword("endpoint","null");
 DefineKeyword("except", "");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 except = params["except"];
 //if (params["endpoint"] == "null") throw PluginError("cylinder", "Invalid end point");
 //if (params["rmax"] == "0") throw PluginError("rmax", "Invalid maxim radius");
 origin = Vector(params["origin"].c_str());
 S = Vector(params["endpoint"].c_str()) - Vector(params["origin"].c_str());
 rmax = double(params["rmax"]);
 rmin = double(params["rmin"]);
}

CylinderFilter::~CylinderFilter() { delete selector; }

void CylinderFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = cylinder                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to cut or select a cylindrical region of the         \n";
 std::cout << "      simulation cell. Use 'filter' to eliminate atoms in or out of a given    \n";
 std::cout << "      cylindrical region, or use 'over' to apply certain plugin to the         \n";
 std::cout << "      cylindrical region.                                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      rmin          : Sets the inner radius of the cylinder.                   \n";
 std::cout << "      rmax          : Sets the outer radius of the cylinder.                   \n";
 std::cout << "      origin        : Sets the position (vector) of the center of the base of  \n";
 std::cout << "                      the cylinder.                                            \n";
 std::cout << "      endpoint      : Sets the position (vector) of the center of the top of   \n";
 std::cout << "                      the cylinder.                                            \n";
 std::cout << "      except        : Tag of the atoms to which the plugin will not be applied \n";
 std::cout << "                      (see use of 'tag' and 'settag' plugins).                 \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying plugin :                                                            \n";
 std::cout << " filter cylinder origin=<5,5,5> endpoint=<6,6,6> rmax=10 rmin=0 except=my-atoms\n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over cylinder origin=<5,5,5> endpoint=<6,6,6> rmax=10 rmin=0\n\n";
 std::cout << "      The plugin is used to eliminate atoms outside the specified region, in   \n";
 std::cout << "      first case, and used to apply a property (tempscaling) over the specified\n";
 std::cout << "      region, in the second case.                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & CylinderFilter::CreateSelector()
{
 //change origin and S based on cell-vectors
 lpmd::Vector a = (*mycell)[0]; double ma = (*mycell)[0].Module();
 lpmd::Vector b = (*mycell)[1]; double mb = (*mycell)[1].Module();
 lpmd::Vector c = (*mycell)[2]; double mc = (*mycell)[2].Module();
 lpmd::Vector newS(S[0]*a/ma+S[1]*b/mb+S[2]*c/mc); 
 lpmd::Vector neworigin(origin[0]*a/ma+origin[1]*b/mb+origin[2]*c/mc);
 Cylinder cyl_region(newS, neworigin, rmax, rmin);
 ParamList & params = (*this);
 DebugStream() << "-> Selecting cylinder: "<< '\n';
 DebugStream() << "origin   = " << params["origin"] << '\n';
 DebugStream() << "endpoint = " << params["endpoint"] << '\n';
 DebugStream() << "rmax     = " << params["rmax"] << '\n';
 DebugStream() << "rmin     = " << params["rmin"] << '\n';
 DebugStream() << "Volume   = " << cyl_region.Volume() << '\n';
 if (selector != 0) delete selector;
 selector = new CylinderSelector(cyl_region);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new CylinderFilter(args); }
void destroy(Plugin * m) { delete m; }

