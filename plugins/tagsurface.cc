//
//
//

#include "tagsurface.h"

#include <lpmd/simulation.h>
#include <lpmd/util.h>
#include <lpmd/particleset.h>

#include <iostream>
#include <sstream>

using namespace lpmd;

TagSurfaceModifier::TagSurfaceModifier(std::string args): Plugin("addvelocity", "1.0")
{ 
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("symbol", "S");
 DefineKeyword("R", "1.0");
 DefineKeyword("direction","Z+");
 DefineKeyword("debug","none");
 ProcessArguments(args); 
 start = int((*this)["start"]);
 end = int((*this)["end"]);
 each = int((*this)["each"]);
 symbol = (*this)["symbol"];
 R0 = double((*this)["R"]);
 direction = (*this)["direction"];
}

TagSurfaceModifier::~TagSurfaceModifier() { }

void TagSurfaceModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = tagsurface                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to tag(using the atomic symbol S) atoms on a surface.\n";
 std::cout << "      This plugin subdivide the unit cell in and use the surface direction.    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      R             : Subcell boxes lenght. We suggest half of the distance to \n";
 std::cout << "                      first neighbor.                                          \n";
 std::cout << "      symbol        : New atomic symbol for surface atoms, default (S).        \n";
 std::cout << "      direction     : the surface is located at direction positive or both     \n";
 std::cout << "                      options are : Z+ and Z / X+ and X / Y+ and Y.            \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use tagsurface                                                                \n";
 std::cout << "     R0 1.7                                                                    \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " apply tagsurface start=0 each=10 end=100                                    \n\n";
 std::cout << "      The plugin is used to apply a tag to all the atoms with Z greater.       \n";
 std::cout << "      steps 0 and 100, each 10 steps.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void TagSurfaceModifier::Apply(Simulation & sim)
{
 lpmd::BasicParticleSet & atoms = sim.Atoms();
 lpmd::BasicCell & cell = sim.Cell();
 lpmd::Vector a = cell[0];
 lpmd::Vector b = cell[1];
 lpmd::Vector c = cell[2];
 lpmd::Array<int> elements = atoms.Elements();

 int nx = floor(a.Module()/R0);
 int ny = floor(b.Module()/R0);
 int nz = floor(c.Module()/R0);
 
 DebugStream() << "-> Subdivision in x = " << nx << '\n';
 DebugStream() << "-> Subdivision in y = " << ny << '\n';
 DebugStream() << "-> Subdivision in z = " << nz << '\n';
 //DebugStream() << "-> Total of " << nx*ny << " subcells.\n";
 //Take the cell and subdivide.
 double dx = 1.0e0/double(nx);
 double dy = 1.0e0/double(ny);
 double dz = 1.0e0/double(nz);
 std::vector<std::string>names_;
 std::map<std::string, lpmd::ParticleSet> grid_;
 int na=0,nb=0;
 if(strcmp(direction.c_str(),"Z+")==0 || strcmp(direction.c_str(),"Z")==0) { na=nx; nb=ny;}
 else if(strcmp(direction.c_str(),"Y+")==0 || strcmp(direction.c_str(),"Y")==0) { na=nx; nb=nz;}
 else if(strcmp(direction.c_str(),"X+")==0 || strcmp(direction.c_str(),"X")==0) { na=ny; nb=nz;}
 else {throw PluginError("tagsurface", "Error with the direction option, not recognized.");}
 for(int i=0; i<na ; ++i)
 {
  for(int j=0; j<nb; ++j)
  {
   std::string tmp ;
   std::ostringstream ostmp;
   ostmp << i << "-" << j;
   tmp = ostmp.str();
   names_.push_back(tmp);
   lpmd::ParticleSet bpstmp;
   grid_[tmp] = bpstmp;
   grid_[tmp].Clear();
  }
 }
 //Asignate atom list for each subcell
 int ca = 0, cb = 0;
 for(int i=0 ; i<atoms.Size() ; ++i)
 {
  Vector fpos=cell.Fractional(atoms[i].Position());
  if(strcmp(direction.c_str(),"Z+")==0 || strcmp(direction.c_str(), "Z")==0)
  {
   ca = floor(fpos[0]/dx);
   cb = floor(fpos[1]/dy);
  }
  else if(strcmp(direction.c_str(), "Y+")==0 || strcmp(direction.c_str(), "Y")==0)
  {
   ca = floor(fpos[0]/dx);
   cb = floor(fpos[2]/dz);
  }
  else if(strcmp(direction.c_str(), "X+")==0 || strcmp(direction.c_str(), "X")==0)
  {
   ca = floor(fpos[1]/dy);
   cb = floor(fpos[2]/dz);
  }
  std::string cell;
  std::ostringstream ostmp;
  ostmp << ca << "-" << cb;
  cell = ostmp.str();
  grid_[cell].Append(atoms[i]);
 }
 //Search correspond atom in the subcell
 for(unsigned int i=0; i < names_.size() ; i++)
 {
  if(grid_[names_[i]].Size()>0)
  {
   int imax=0,imin=0;
   if(strcmp(direction.c_str(),"Z+") == 0 || strcmp(direction.c_str(),"Z") ==0)
   {
    for(int j=0; j<grid_[names_[i]].Size() ; ++j)
    {
     lpmd::Atom atm = grid_[names_[i]][j];
     lpmd::Vector pos = atm.Position();
     double lz = pos[2];
     if(j==0) {imax=0;imin=0;}
     else
     {
      lpmd::Vector oldmax = grid_[names_[i]][imax].Position();
      double pmax = oldmax[2];
      if(lz > pmax) {imax=j;}
      if(strcmp(direction.c_str(),"Z")==0)
      {
       lpmd::Vector oldmin = grid_[names_[i]][imin].Position();
       double pmin = oldmin[2];
       if(lz < pmin) {imin=j;}
      }
     }
    }
    //repalce the atomic symbol.
    lpmd::Atom max("S", grid_[names_[i]][imax].Position());
    long int index = atoms.Find(grid_[names_[i]][imax]);
    atoms[index]=max;
    if(strcmp(direction.c_str(),"Z")==0 && imin!=imax)
    {
     lpmd::Atom min("S", grid_[names_[i]][imin].Position());
     long int indexmin = atoms.Find(grid_[names_[i]][imin]);
     atoms[indexmin]=min;
    }
   }
   else if(strcmp(direction.c_str(),"Y+") == 0 || strcmp(direction.c_str(), "Y") == 0)
   {
    for(int j=0; j<grid_[names_[i]].Size() ; ++j)
    {
     lpmd::Atom atm = grid_[names_[i]][j];
     lpmd::Vector pos = atm.Position();
     double ly = pos[1];
     if(j==0) {imax=0;imin=0;}
     else
     {
      lpmd::Vector oldmax = grid_[names_[i]][imax].Position();
      double pmax = oldmax[1];
      if(ly > pmax) {imax=j;}
      if(strcmp(direction.c_str(),"Y")==0)
      {
       lpmd::Vector oldmin = grid_[names_[i]][imin].Position();
       double pmin = oldmin[1];
       if(ly < pmin) {imin=j;}
      }
     }
    }
    //repalce the atomic symbol.
    lpmd::Atom max("S", grid_[names_[i]][imax].Position());
    long int index = atoms.Find(grid_[names_[i]][imax]);
    atoms[index]=max;
    if(strcmp(direction.c_str(),"Y")==0 && imin!=imax)
    {
     lpmd::Atom min("S", grid_[names_[i]][imin].Position());
     long int indexmin = atoms.Find(grid_[names_[i]][imin]);
     atoms[indexmin]=min;
    }
   }
   else if(strcmp(direction.c_str(),"X+") == 0 || strcmp(direction.c_str(), "X") == 0)
   {
    for(int j=0; j<grid_[names_[i]].Size() ; ++j)
    {
     lpmd::Atom atm = grid_[names_[i]][j];
     lpmd::Vector pos = atm.Position();
     double lx = pos[0];
     if(j==0) {imax=0;imin=0;}
     else
     {
      lpmd::Vector oldmax = grid_[names_[i]][imax].Position();
      double pmax = oldmax[0];
      if(lx > pmax) {imax=j;}
      if(strcmp(direction.c_str(),"X")==0)
      {
       lpmd::Vector oldmin = grid_[names_[i]][imin].Position();
       double pmin = oldmin[0];
       if(lx < pmin) {imin=j;}
      }
     }
    }
    //repalce the atomic symbol.
    lpmd::Atom max("S", grid_[names_[i]][imax].Position());
    long int index = atoms.Find(grid_[names_[i]][imax]);
    atoms[index]=max;
    if(strcmp(direction.c_str(),"X")==0 && imin!=imax)
    {
     lpmd::Atom min("S", grid_[names_[i]][imin].Position());
     long int indexmin = atoms.Find(grid_[names_[i]][imin]);
     atoms[indexmin]=min;
    }
   }
   else {throw PluginError("tagsurface", "Error identifying 'direction' tag.");}
  }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TagSurfaceModifier(args); }
void destroy(Plugin * m) { delete m; }

