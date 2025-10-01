//
//
//

#include "box.h"

#include <lpmd/simulation.h>
#include <lpmd/refparticleset.h>
#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

#define DELTA (1.0E+12)

void BoxFilter::Update(lpmd::Simulation & sim) { mycell = &(sim.Cell()); }

class BoxSelector: public Selector<BasicParticleSet>
{
 public:
   BoxSelector(const Box & b): box(b) { }

   const BasicParticleSet & SelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (box.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

   const BasicParticleSet & InverseSelectFrom(const BasicParticleSet & ps) 
   { 
    innerps.Clear();
    for (long int i=0;i<ps.Size();++i) 
      if (!box.IsInside(ps[i].Position())) innerps.Append(ps[i]);
    return innerps;
   }

 private:
   Box box;
   RefParticleSet innerps;
};

BoxFilter::BoxFilter(std::string args): Plugin("box", "1.0"), selector(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("x","0-0");
 DefineKeyword("y","0-0");
 DefineKeyword("z","0-0");
 DefineKeyword("except", "");
 DefineKeyword("debug","none");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 except = params["except"];
 if (params["x"] == "all")
 {
  x[0] = -DELTA;
  x[1] = +DELTA;
 }
 else
 {
  lpmd::Array<std::string> sx = StringSplit(params["x"],'-');
  if (sx.Size() != 2) throw PluginError("box", "Invalid x range");
  x[0] = atof(sx[0].c_str());
  x[1] = atof(sx[1].c_str());
 }

 if (params["y"] == "all")
 {
  y[0] = -DELTA;
  y[1] = +DELTA;
 }
 else
 {
  lpmd::Array<std::string> sy = StringSplit(params["y"],'-');
  if (sy.Size() != 2) throw PluginError("box", "Invalid y range");
  y[0] = atof(sy[0].c_str());
  y[1] = atof(sy[1].c_str());
 }

 if (params["z"] == "all")
 {
  z[0] = -DELTA;
  z[1] = +DELTA;
 } 
 else
 {
  lpmd::Array<std::string> sz = StringSplit(params["z"],'-');
  if (sz.Size() != 2) throw PluginError("box", "Invalid z range");
  z[0] = atof(sz[0].c_str());
  z[1] = atof(sz[1].c_str());
 }
}

BoxFilter::~BoxFilter() { delete selector; }

void BoxFilter::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = box                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to select a box region of the simulation cell.       \n";
 std::cout << "      You can assign properties or tags to this region.  It can be called with \n";
 std::cout << "      the 'filter' or 'over' keyword.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      x             : Select range in x axis. x=0-10 or x=all                  \n";
 std::cout << "      y             : Select range in y axis. x=10-100 or x=all                \n";
 std::cout << "      z             : Select range in z axis. x=5-20 or x=all                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Applying plugin :                                                            \n";
 std::cout << " filter box x=0-10 y=15-20 z=5-10                                              \n";
 std::cout << " apply tempscaling each=1 start=1 end=-1 over box x=0-68.8 y=0-68.8 z=0-20   \n\n";
 std::cout << "      The plugin is used to eliminate atoms outside the specified region, in   \n";
 std::cout << "      first case, and used to apply a property (tempscaling) over the specified\n";
 std::cout << "      region, in the second case.                                              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

Selector<BasicParticleSet> & BoxFilter::CreateSelector()
{
 Vector a = (*mycell)[0]; Vector b = (*mycell)[1] ; Vector c = (*mycell)[2];
 //Angles in radian for each axis with standard system.
 double aa = acos(Dot(a,Vector(1,0,0))/a.Module());//*180.0/M_PI;
 double ab = acos(Dot(b,Vector(0,1,0))/b.Module());//*180.0/M_PI;
 double ac = acos(Dot(c,Vector(0,0,1))/c.Module());//*180.0/M_PI;

 double minx=0.0e0,miny=0.0e0,minz=0.0e0;
 double maxx=0.0e0,maxy=0.0e0,maxz=0.0e0;
 if(x[0] < 0 ) minx=x[0];
 else minx = fabs(x[0]*cos(aa));
 maxx = fabs(x[1]*cos(aa));
 if(y[0] < 0 ) miny=y[0];
 else miny = fabs(y[0]*cos(ab));
 maxy = fabs(y[1]*cos(ab));
 if(z[0] < 0) minz=z[0];
 else minz = fabs(z[0]*cos(ac));
 maxz = fabs(z[1]*cos(ac));

 Box box_region(minx, maxx, miny, maxy, minz, maxz, a, b, c);
 ParamList & params = (*this);
 DebugStream() << "-> Selecting range: ";
 DebugStream() << "X=";
 if (params["x"] != "all") DebugStream() << "[ " << x[0] << ", " << x[1] << " ] ";
 else DebugStream() << "all "; 
 DebugStream() << "Y=";
 if (params["y"] != "all") DebugStream() << "[ " << y[0] << ", " << y[1] << " ] ";
 else DebugStream() << "all "; 
 DebugStream() << "Z=";
 if (params["z"] != "all") DebugStream() << "[ " << z[0] << ", " << z[1] << " ] ";
 else DebugStream() << "all "; 
 DebugStream() << "\n";
 if (selector != 0) delete selector;
 selector = new BoxSelector(box_region);
 return *selector;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new BoxFilter(args); }
void destroy(Plugin * m) { delete m; }

