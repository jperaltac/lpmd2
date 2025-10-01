//
//
//

#include "dlpoly.h"

#include <lpmd/util.h>
#include <lpmd/simulation.h>
#include <sstream>
#include <iomanip>
#include <lpmd/simulationhistory.h>
#include <lpmd/particleset.h>
#include <cstdio>

using namespace lpmd;

DlPolyFormat::DlPolyFormat(std::string args): Plugin("dlpoly", "2.0"), dt(0.0)
{
 lpmd::ParamList & param = (*this);
 DefineKeyword("file");
 DefineKeyword("each", "1");
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("level", "0");
 DefineKeyword("periodicity", "1");
 DefineKeyword("replacecell", "false");
 DefineKeyword("ftype","CONFIG");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 readfile = writefile = param["file"];
 interval = int(param["each"]);
 level = int(param["level"]);
 pbkey = int(param["periodicity"]);
 rcell = bool(param["replacecell"]);
 ftype = param["ftype"];
 if (Defined("dt")) dt = double(param["dt"]);
 if (Defined("configs")) Nconfigs = int(param["config"]);
 if (Defined("atoms")) Natoms = int(param["atoms"]); 
 stepcnt = new long int;
}

DlPolyFormat::~DlPolyFormat() { delete stepcnt; }

void DlPolyFormat::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = dlpoly                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to read/write DL_POLY's atomic configurations files. \n";
 std::cout << "      (HISTORY or CONFIG files).                                               \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      file          : Input/output file that contains the atomic configurations\n";
 std::cout << "                      in DLPOLY format.                                        \n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      input/output file must be read/written.                  \n";
 std::cout << "      level         : Determines the file format level (0/1/2):                \n";
 std::cout << "                      0/1/2 <-> pos/pos-vel/pos-vel-acel.                      \n";
 std::cout << "      periodicity   : Periodic boundary key of the CONFIG file (0/1/2/3/6):    \n";
 std::cout << "                      0/1/2/3/6 <-> no periodic boundaries / cubic boundary    \n";
 std::cout << "                      conditions / orthorhombic boundary conditions /          \n";
 std::cout << "                      parallelepiped boundary conditions / x-y parallelogram   \n";
 std::cout << "                      boundary conditions with no periodicity in z-direction.  \n";
 std::cout << "      ftype         : File type (CONFIG / HISTORY).                            \n";
 std::cout << "      replacecell   : Replace the dimensions of the cell by those found in the \n";
 std::cout << "                      CONFIG or HISTORY file (true / false).                   \n";
 std::cout << " HISTORY output options >>                                                     \n";
 std::cout << "      dt            : Step time used in HISTORY format.                        \n";
 std::cout << "      configs       : Number of configurations to write in the HISTORY file.   \n";
 std::cout << "      atoms         : Number of atoms of the initial configuration.            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input module=dlpoly file=CONFIG level=0                                       \n";
 std::cout << " output module=dlpoly file=CONFIG.output level=1 periodicity=2 each=5        \n\n";
 std::cout << "      In this way we can read and write atomic configurations in DL_POLY's     \n";
 std::cout << "      format. The file extension (.output) is irrelevant, what matters         \n";
 std::cout << "      is the module loaded (module=dlpoly).                                    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void DlPolyFormat::ReadHeader(std::istream & is) const
{
 // El formato CONFIG de DLPOLY no tiene ningun header especial
 // El formato HISTORY de DLPOLY posee un titulo y un set de valores de la simulacion.
 if(ftype=="HISTORY")
 {
  std::string tmp;
  getline(is,tmp); // lee la linea de titulo.
  getline(is,tmp);
  std::istringstream ost(tmp);
  int a1,a2,a3,a4,a5;
  ost >> a1 >> a2 >> a3 >> a4 >> a5;
  if (a1!=level) throw PluginError("dlpoly", "Error the level-file are diferent that the specified.");
 }
}

// 
// Lee una configuracion desde un archivo CONFIG 
//
bool DlPolyFormat::ReadCell(std::istream & is, Configuration & con) const
{
 lpmd::BasicParticleSet & atoms = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 con.SetTag(con, Tag("level"), level);
 int fkey, pbk;
 std::string tmp;
 Vector cv[3];
 double x, y, z;
 if(ftype=="CONFIG")
 {
  getline(is, tmp);           // lee la linea de titulo
  if (is.eof()) return false; // no hay mas configuraciones que leer
  getline(is, tmp);
  std::istringstream ost(tmp);
  ost >> fkey >> pbk;
  for (int i=0;i<3;++i)
  {
   getline(is, tmp);
   std::istringstream vst(tmp);
   vst >> x >> y >> z;
   cv[i] = Vector(x, y, z);
   if ((*this)["replacecell"] == "true") cell[i] = cv[i];
  } 
  long cc = 0;
  while (1)
  {
   cc++;
   getline(is, tmp);
   if (is.eof()) break;
   std::istringstream vst(tmp); 
   std::string symbol;
   vst >> symbol;
   Atom this_atom(ElemNum(symbol));
   for (int i=0;i<=fkey;++i)
   {
    getline(is, tmp);
    std::istringstream vst2(tmp);
    vst2 >> x >> y >> z;
    if (i == 0) this_atom.Position() = Vector(x+0.5*cv[0].Module(), y+0.5*cv[1].Module(), z+0.5*cv[2].Module());
    if (i == 1) this_atom.Velocity() = Vector(x, y, z);
    if (i == 2) this_atom.Acceleration() = Vector(x, y, z);
   }
   atoms.Append(this_atom);
  }
 }
 else if(ftype=="HISTORY")
 {
  getline(is, tmp);
  if (is.eof()) return false; // no hay mas configuraciones que leer
  std::istringstream tst(tmp);
  std::string ttmp;
  int megatm;
  tst >> ttmp >> ttmp ;
  tst >> megatm ;
  tst >> fkey >> pbk ;
  for(int j=0;j<3;++j)
  {
   getline(is, tmp);
   std::istringstream vst(tmp);
   vst >> x >> y >> z;
   cv[j] = Vector(x,y,z);
   if((*this)["replacecell"] == "true") cell[j] = cv[j];
  }
  for(int k=0;k<megatm;++k)
  {
   getline(is,tmp);
   std::istringstream vst(tmp); 
   std::string symbol;
   vst >> symbol;
   Atom this_atom(ElemNum(symbol));
   for (int i=0;i<=fkey;++i)
   {
    getline(is, tmp);
    std::istringstream vst3(tmp);
    vst3 >> x >> y >> z;
    if (i == 0) this_atom.Position() = Vector(x+0.5*cv[0].Module(), y+0.5*cv[1].Module(), z+0.5*cv[2].Module());
    if (i == 1) this_atom.Velocity() = Vector(x, y, z);
    if (i == 2) this_atom.Acceleration() = Vector(x, y, z);
   }
   atoms.Append(this_atom);
  }
 }
 else
 {
  throw PluginError("dlpoly", "The fkey="+ftype+" is not a valid fkey value.");
 }
 return true;
}

bool DlPolyFormat::SkipCell(std::istream & is) const
{
 int fkey, pbk;
 std::string tmp;
 if (ftype == "CONFIG")
 {
  getline(is, tmp);
  if (is.eof()) return false; // no hay mas configuraciones que leer
  getline(is, tmp);
  std::istringstream ost(tmp);
  ost >> fkey >> pbk;
  for (int i=0;i<3;++i) getline(is, tmp);
  long cc = 0;
  while (1)
  {
   cc++;
   getline(is, tmp);
   if (is.eof()) break;
   for (int i=0;i<=fkey;++i) getline(is, tmp);
  }
 }
 else if (ftype=="HISTORY")
 {
  getline(is, tmp);
  if (is.eof()) return false; // no hay mas configuraciones que leer
  std::istringstream tst(tmp);
  std::string ttmp;
  int megatm;
  tst >> ttmp >> ttmp ;
  tst >> megatm ;
  tst >> fkey >> pbk;
  for(int j=0;j<3;++j) getline(is, tmp);
  for(int k=0;k<megatm;++k)
  {
   getline(is,tmp);
   for (int i=0;i<=fkey;++i) getline(is, tmp);
  }
 }
 else throw PluginError("dlpoly", "The fkey="+ftype+" is not a valid fkey value.");
 return true;
}

void DlPolyFormat::WriteHeader(std::ostream & os, lpmd::SimulationHistory  *sh) const
{
 // El formato CONFIG de DLPOLY no tiene ningun header especial
 // El formato HISTORY de DLPOLY tiene dos lineas de HEADER.
 if (ftype=="HISTORY")
 {
  if (! Defined("dt")) throw PluginError("dlpoly", "HISTORY format needs the \"dt\" parameter");
  char * buf = new char[512];
  int frame = 0,atoms=0;
  if (Defined("configs")) frame = Nconfigs;
  else 
  {
   if (sh==NULL) throw PluginError("dlpoly", "Please set the number of configurations, use the configs=N and atoms=N parameters.");
   frame = (*sh).Size();
  }
  if (Defined("atoms")) atoms = Natoms;
  else
  { 
   atoms = (*sh)[frame-1].Atoms().Size();
  }
  int records = frame*atoms*(2+level) + frame*4 + 2;
  os << "Generated by LPMD dlpoly plugin\n";
  sprintf(buf, "%10d%10d%10d%10d%10d\n", level, pbkey, frame, atoms, records);
  os << buf;
  *(stepcnt) = 0;
 }
 else os << "Generated by LPMD dlpoly plugin\n";
}

void DlPolyFormat::WriteCell(std::ostream & out, Configuration & con) const
{
 lpmd::BasicParticleSet & atoms = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 //Sort by species
 lpmd::Array<int> elements = atoms.Elements();
 lpmd::ParticleSet tmp;
 tmp.Clear();
 for(int i=0;i<elements.Size();++i)
 {
  for(int j=0;j<atoms.Size();++j)
  {
   if(atoms[j].Z() == elements[i]) tmp.Append(atoms[j]);
  }
 }
 atoms.Clear();
 for(int i=0;i<tmp.Size();++i)
 {
  atoms.Append(tmp[i]);
 }
 //End Sort
 con.SetTag(con, Tag("level"), level);
 char * buf = new char[512];
 if (ftype=="CONFIG")
 {
  sprintf(buf, "%10d%10d\n", level, pbkey);
  out << buf;
  for (int i=0;i<3;++i)
  {
   sprintf(buf, "%20.10f%20.10f%20.10f\n", cell[i][0], cell[i][1], cell[i][2]);
   out << buf; 
  }
  for (long i=0;i<atoms.Size();++i)
  {
   const Atom & at = atoms[i];
   sprintf(buf, "%-5s%13lu\n", at.Symbol().c_str(), i+1);
   out << buf;
   Vector v;
   if (level >= 0)
   {
    v = at.Position();
    for (int q=0;q<3;++q) v[q] = v[q]-0.5*cell[q].Module(); 
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
   if (level >= 1)
   {
    v = at.Velocity();
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
   if (level >= 2)
   {
    v = at.Acceleration();
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
  } 
  delete [] buf;
 }
 else if(ftype=="HISTORY")
 {
  //Est primera linea en history posee mas informacion.
  //timestep         0       256         0         1    0.001000    0.000000
  //timestep es un string standard; 0 es el current timstep (poner un counter?)
  //256 es el numero de atomos; 0 es el level; 1 es la peridicidad; 0.001000 es el paso del integrador
  //0.000000 es el paso de tiempo real (integrador por current timestep).
  (*stepcnt)++;
  sprintf(buf, "timestep%10ld%10d%10d%10d%12.6g    %12.6g\n", *(stepcnt), int(con.Atoms().Size()), level, pbkey, dt, *(stepcnt)*dt);
  out << buf;
  for (int i=0;i<3;++i)
  {
   sprintf(buf, "%20.10f%20.10f%20.10f\n", cell[i][0], cell[i][1], cell[i][2]);
   out << buf; 
  }
  for (long i=0;i<atoms.Size();++i)
  {
   const Atom & at = atoms[i];
   sprintf(buf, "%-5s%13lu\n", at.Symbol().c_str(), i+1);
   out << buf;
   Vector v;
   if (level >= 0)
   {
    v = at.Position();
    for (int q=0;q<3;++q) v[q] = v[q]-0.5*cell[q].Module(); 
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
   if (level >= 1)
   {
    v = at.Velocity();
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
   if (level >= 2)
   {
    v = at.Acceleration();
    sprintf(buf, "    %12.10g        %12.10g        %12.10g\n", v[0], v[1], v[2]);
    out << buf;
   }
  } 
  delete [] buf;
 }
 else 
 {
  throw PluginError("dlpoly","The fkey="+ftype+" value, is not a valid value.");
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new DlPolyFormat(args); }
void destroy(Plugin * m) { delete m; }

