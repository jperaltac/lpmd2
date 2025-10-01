//
//
//

#include "lammps.h"

#include <arpa/inet.h>
#include <zlib.h>

#include <lpmd/util.h>
#include <lpmd/simulation.h>
#include <lpmd/atom.h>
#include <lpmd/colorhandler.h>
#include <lpmd/elements.h>
#include <stdio.h>
#include <string.h>

using namespace lpmd;

LAMMPSFormat::LAMMPSFormat(std::string args): Plugin("lammps", "1.0")
{
 hdr.Clear();
 ParamList & params = (*this);
 //
 linecounter = new long int;
 DefineKeyword("file" ,"noname");
 DefineKeyword("species", "NULL");
 DefineKeyword("each", "1");
 DefineKeyword("level", "0");
 DefineKeyword("inside" ,"false");
 DefineKeyword("displace" ,"true");
 DefineKeyword("replacecell", "false");
 DefineKeyword("ftype","dump");
 DefineKeyword("datatype","NULL");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 ftype = params["ftype"];
 datatype = params["datatype"];
 readfile = writefile = (*this)["file"];
 std::string q = readfile.substr(readfile.size()-4,4);
 if (strcmp(q.c_str(),"dump")==0) { ftype = "dump"; }
 else if (strcmp(q.c_str(),".data")==0) { ftype = "data"; }
 interval = int(params["each"]);
 level = int(params["level"]);
 rcell = params["replacecell"];
 inside = params["inside"];
 displace = params["displace"];
 species = StringSplit((*this)["species"],'-');
}

LAMMPSFormat::~LAMMPSFormat()
{ 
}

void LAMMPSFormat::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = lammps                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to read/write LAMMPS's atomic configurations files   \n";
 std::cout << "      (dump and data file ftypes). Not all customized dump files can be read    \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      file          : Input/output file that contains the atomic configurations\n";
 std::cout << "                      in LAMMPS format.                                        \n";
 std::cout << "      species       : Atomic species list (separated by - [dash]).             \n";
 std::cout << "      displace      : Shift the origin of the simulation box from (xlo,ylo,zlo)\n";
 std::cout << "                      to (0,0,0). Since LAMMPS works with cells given by the   \n";
 std::cout << "                      limits xlo, xhi, ylo, yhi, zlo, zhi, this 'displace'     \n";
 std::cout << "                      option will substract the vector (xlo,ylo,zlo) to the    \n";
 std::cout << "                      position vector of every atom. (true / false)            \n";
 std::cout << "                      * This opperation is applied before 'inside' flag.       \n";
 std::cout << "      inside        : Impose periodic boundary conditions over the atoms,      \n";
 std::cout << "                      according to the cell vectors given, to fit the atoms    \n";
 std::cout << "                      inside the cell. (true / false)                          \n";
 std::cout << "                      * If displace=true, inside is applied after displace.    \n";
 std::cout << "      level         : Determines the file format level (0/1):                  \n";
 std::cout << "                      0 -> Only positions are written in the output file       \n";
 std::cout << "                      1 -> Positions and velocities are written in the output  \n";
 std::cout << "                           file. It works for both dump and data format.       \n";
 std::cout << "      ftype         : File type (dump / data)                                  \n";
 std::cout << "                      dump: Read/write LAMMPS dump file                        \n";
 std::cout << "                      data: Read/write LAMMPS data file                        \n";
 std::cout << "      datatype      : When ftype = data, you have to select the LAMMPS atom style\n";
 std::cout << "                      used by the data file.                                   \n";
 std::cout << "                     (angle / atomic / bond / charge / dipole / full / molecular)\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin from control file:                                        \n";
 std::cout << " input module=lammps file=inputfile.lammps ftype=data species=Pb               \n";
 std::cout << " input module=lammps file=atoms.dump species=C-O-N ftype=dump level=1          \n";
 std::cout << " output module=lammps file=output.dump species=Co-Ni level=1 each=5            \n";
 std::cout << " output module=lammps file=outputfile.lpmd level=1 each=5                      \n";
 std::cout << "                                                                               \n";
 std::cout << " #Loading the plugin from control file:                                        \n";
 std::cout << " lpmd-converter lammps:myfile.data,species=C-Fe-Pb -r -o xyz:myfile.xyz        \n";
 std::cout << "                                                                               \n";
 std::cout << "      The plugin is used to read and write atomic configurations in LAMMPS's   \n";
 std::cout << "      formats. The file extension (.lpmd or .data) is irrelevant, what matters  \n";
 std::cout << "      is the module loaded (module=lpmd).                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void LAMMPSFormat::ReadHeader(std::istream & is) const
{

 int len = is.tellg();
 std::string line;
 getline(is, line);
 Array<std::string> lspl = StringSplit(line);

 // First line is a comment => .data  // First line says 'ITEM' => .dump
 if (lspl[0] == "ITEM:")
 {
  ftype = "dump";
  for(int i=0;i<8;++i) getline(is,line);
  lspl = StringSplit(line);

  bool found=false;
  for (int k=0; k<lspl.Size(); ++k) if (lspl[k]=="element") found=true;
  if(!found && species[0]=="NULL") throw PluginError("lammps", "Error, you must give a species list.");

  is.seekg(len, std::ios_base::beg); // Go to the beggining of the file
  getline(is,line);
 }
 else
 {
  if(species[0]=="NULL") throw PluginError("lammps", "Error, you must give a species list.");
  is.seekg(len, std::ios_base::beg); // Go to the beggining of the file
  ReadHeaderData(is);
  return;
 }
 

 //is.seekg(len, std::ios_base::beg); // Go to the beggining of the file

 (*linecounter) = 1;

}


void LAMMPSFormat::ReadHeaderData(std::istream & is) const
{
 int len = is.tellg();
 std::string line;
 getline(is, line);  // First word should be a comment
 hdr.Append(line);
 (*linecounter) = 1;
 natoms = 0; 

 if (datatype=="NULL") { ShowWarning("lammps", "No datatype specified. Assuming 'datatype = atomic'.");  datatype="atomic"; }
  
 
 Array<std::string> lspl = StringSplit(line, ' ');
 while(lspl[0]!="Atoms" && !is.eof() )
 {
  getline(is, line);
  hdr.Append(line);
  (*linecounter)++;
  lspl = StringSplit(line, ' ');
  if (lspl.Size()==2) { if (lspl[1]=="atoms") natoms=atoi(lspl[0].c_str()); }
  else if (lspl.Size()==4)
  {
   if (lspl[3]=="xhi")
   {
    if ((*this)["replacecell"] == "true")
    {
     double xlo, xhi, ylo, yhi, zlo, zhi, xy=0.0, xz=0.0, yz=0.0;
     xlo = atof(lspl[0].c_str()); xhi = atof(lspl[1].c_str());
     getline(is, line); lspl = StringSplit(line, ' '); (*linecounter)++; hdr.Append(line); 
     ylo = atof(lspl[0].c_str()); yhi = atof(lspl[1].c_str());
     getline(is, line); lspl = StringSplit(line, ' '); (*linecounter)++; hdr.Append(line);
     zlo = atof(lspl[0].c_str()); zhi = atof(lspl[1].c_str());
     getline(is, line); lspl = StringSplit(line, ' '); (*linecounter)++; hdr.Append(line);
     if (lspl.Size()==6) { xy = atof(lspl[0].c_str()); xz = atof(lspl[1].c_str()); yz = atof(lspl[2].c_str());}
     CellVect[0] = Vector(xhi - xlo, 0.0, 0.0);
     CellVect[1] = Vector(xy, yhi - ylo, 0.0);
     CellVect[2] = Vector(xz, yz, zhi - zlo);
    }
    else
    {
     for(int q=0; q<2; ++q) { getline(is,line); hdr.Append(line); (*linecounter)++; }
    }
   }
  }
 }

 if (natoms==0) throw PluginError("lammps", "File "+readfile+" doesn't seem to be in LAMMPS format (not a LAMMPS dump/data file)");
     
 (*linecounter) = 0;

 is.seekg(len, std::ios_base::beg); // Go to the beggining of the file again
 ftype = "data";
}


// 
// Reads a configuration from a LAMMPS file 
//
bool LAMMPSFormat::ReadCell(std::istream & is, Configuration & con) const
{
 BasicCell & cell = con.Cell();
 BasicParticleSet & part = con.Atoms();

  
 assert(part.Size() == 0);
 std::string tmp;
 double xlo, xhi, ylo, yhi, zlo, zhi, xy=0.0, xz=0.0, yz=0.0;

 Array<std::string> words = StringSplit(tmp, ' '); 
 if (ftype == "dump")
 {
  getline(is, tmp);                                     // timestep number
  getline(is, tmp);                                     // "ITEM: NUMBER OF ATOMS"
  getline(is, tmp);                                     // Numero de atomos
  (*linecounter)+=3;
  words = StringSplit(tmp, ' '); 
  if (words.Size() == 0) return false;
  natoms = atoi(words[0].c_str());

  getline(is, tmp);          // Something like "ITEM: BOX BOUNDS pp ss pp" 
  getline(is, tmp);          // Cell vectors 
  (*linecounter)+=2;

  words = StringSplit(tmp, ' '); 
  con.SetTag(con, Tag("level"), level);
  if(words.Size()==2)
  {
   if ((*this)["replacecell"] == "true")
   {
    xlo = atof(words[0].c_str()); xhi = atof(words[1].c_str());
    getline(is, tmp); words = StringSplit(tmp, ' '); (*linecounter)++;
    ylo = atof(words[0].c_str()); yhi = atof(words[1].c_str());
    getline(is, tmp); words = StringSplit(tmp, ' '); (*linecounter)++;
    zlo = atof(words[0].c_str()); zhi = atof(words[1].c_str());
    cell[0] = Vector(xhi - xlo, 0.0, 0.0);
    cell[1] = Vector(0.0, yhi - ylo, 0.0);
    cell[2] = Vector(0.0, 0.0, zhi - zlo);
   }
   else {getline(is,tmp); getline(is,tmp); (*linecounter)+=2;}
  }
  else if(words.Size()==3)
  {
   if ((*this)["replacecell"] == "true")
   {
    xlo = atof(words[0].c_str()); xhi = atof(words[1].c_str()); xy  = atof(words[2].c_str());
    getline(is, tmp); words = StringSplit(tmp, ' '); (*linecounter)++;
    ylo = atof(words[0].c_str()); yhi = atof(words[1].c_str()); xz  = atof(words[2].c_str());
    getline(is, tmp); words = StringSplit(tmp, ' '); (*linecounter)++;
    zlo = atof(words[0].c_str()); zhi = atof(words[1].c_str()); yz  = atof(words[2].c_str());
    if (xy>0) xhi -= xy; else xlo -=xy;
    if (xz>0) xhi -= xz; else xlo -=xz;
    if (yz>0) yhi -= yz; else ylo -=yz;

    cell[0] = Vector(xhi - xlo, 0.0, 0.0);
    cell[1] = Vector(xy , yhi - ylo, 0.0);
    cell[2] = Vector(xz , yz , zhi - zlo);
   }
   else {getline(is,tmp); getline(is,tmp); (*linecounter)+=2;}
  }
  else throw PluginError("lammps", "Error ocurred when reading box dimensions, file \""+readfile+"\", line "+ToString<int>(*linecounter));

  getline(is, tmp);          // Something like "ITEM: ATOMS id type xu yu zu vx vy vz c_poteng" 
  words = StringSplit(tmp, ' ');
  (*linecounter)++;
  if ( words[1]!="ATOMS" ) throw PluginError("lammps", "Error ocurred after reading box dimensions, file \""+readfile+"\", line "+ToString<int>(*linecounter));
  hdr.Clear();
  for (long int i=0;i<words.Size();++i) hdr.Append(std::string(words[i]));

  bool coords=false;
  for (long int k=2 ; k < hdr.Size() ; ++k)
   if (hdr[k][0] == 'x' || hdr[k][0] == 'y') coords=true;
  if(!coords) throw PluginError("lpmd", "Coordinates x y z not found in line "+ToString<int>(*linecounter)+": \""+tmp+"\"");
 }

 else if (ftype == "data")
 {
  if ((*this)["replacecell"] == "true")
  {
   for(int q=0; q<3; ++q) cell[q] = CellVect[q];
  }
  
  for (int q=0; q<hdr.Size(); ++q) { getline(is, tmp); (*linecounter)++; }

  words = StringSplit(tmp, ' '); 
  if (words.Size() == 0) return false;
  getline(is, tmp); (*linecounter)++;
 }

 bool shrink_wrapped=false;
 for (long int i=0;i<natoms;++i) part.Append(Atom());
 Vector pos(0,0,0);
 Vector vel(0,0,0);
 for (long int i=0;i<natoms;++i)
 {
  getline(is, tmp);
  (*linecounter)++;
  words = StringSplit(tmp, ' ');
  if (words.Size() == 0) throw PluginError("lammps", "Error ocurred, the input file does not have elements!"); 
  else if (words.Size() >=1)
  {
   int spec_num=1;
   std::string elem="e";
   long int id=0;
   double X=0.0e0,Y=0.0e0,Z=0.0e0;
   double VX=0.0e0,VY=0.0e0,VZ=0.0e0;
   if (ftype == "dump")
   {
    for (long int k=2 ; k < hdr.Size() ; ++k)
    {
     if      (hdr[k] == "id")      id=atoi(words[k-2].c_str());
     else if (hdr[k] == "type")    spec_num=atoi(words[k-2].c_str());
     else if (hdr[k] == "element") elem=words[k-2].c_str();
     else if (hdr[k][0] == 'x')    { X=atof(words[k-2].c_str()); if (hdr[k][1] == 's') shrink_wrapped=true; }
     else if (hdr[k][0] == 'y')    { Y=atof(words[k-2].c_str()); if (hdr[k][1] == 's') shrink_wrapped=true; }
     else if (hdr[k][0] == 'z')    { Z=atof(words[k-2].c_str()); if (hdr[k][1] == 's') shrink_wrapped=true; }
     else if (hdr[k] == "vx")      VX=atof(words[k-2].c_str());
     else if (hdr[k] == "vy")      VY=atof(words[k-2].c_str());
     else if (hdr[k] == "vz")      VZ=atof(words[k-2].c_str());
    }
    pos = Vector(X,Y,Z);
    vel = Vector(VX,VY,VZ);
    if (shrink_wrapped) pos = cell.Cartesian(Vector(X,Y,Z));
   }
   else if (ftype == "data")
   {
    int c_id=0, c_sn=1, c_x=2, c_y=3, c_z=4;
    if      (words.Size()==5)                          { c_id=0; c_sn=1; c_x=2; c_y=3; c_z=4; }
    else if (words.Size()==6 && datatype=="molecular") { c_id=0; c_sn=2; c_x=3; c_y=4; c_z=5; }
    else if (words.Size()==6 && datatype=="charge")    { c_id=0; c_sn=1; c_x=3; c_y=4; c_z=5; }
    else if (words.Size()>=7) { c_id=0; c_sn=2; c_x=4; c_y=5; c_z=6; }
    id=atoi(words[c_id].c_str());
    spec_num=atoi(words[c_sn].c_str());
    X=atof(words[c_x].c_str());
    Y=atof(words[c_y].c_str());
    Z=atof(words[c_z].c_str());
    pos = Vector(X,Y,Z);
   }

   if (spec_num>species.Size()) throw PluginError("lammps", "Missing species in \""+readfile+"\", line "+ToString<int>(*linecounter)+". Only "+ToString<int>(species.Size())+" given, but more than "+ToString<int>(species.Size())+" were found.");

   lpmd::Atom atm(species[spec_num-1],pos,vel);
   if (elem!="e") { atm.Z() = ElemNum(elem);  }
   part[id-1] = atm;
   part[id-1].ID() = id;
  }
  else throw PluginError("lammps", "An unidentified line was found in the file \""+readfile+"\", line "+ToString<int>(*linecounter));
 }

 //Reubica los atomos dentro de la celda.
 if (displace == "true")
 {
  if(ftype=="data")
  {
   words = StringSplit(hdr[3], ' '); xlo = atof(words[0].c_str());
   words = StringSplit(hdr[4], ' '); ylo = atof(words[0].c_str());
   words = StringSplit(hdr[5], ' '); zlo = atof(words[0].c_str());
  }
  for(int i=0;i<natoms;i++)  part[i].Position() -= Vector(xlo,ylo,zlo);
 }

 //Reubica los atomos dentro de la celda.
 if (inside == "true")
 {
  for(int i=0;i<natoms;i++)  part[i].Position() = cell.FittedInside(part[i].Position());
 }
 

 if (ftype == "dump") { getline(is, tmp);  (*linecounter)++; }
 else if (ftype == "data")
 {
  // READING VELOCITIES
  int len = is.tellg();
  getline(is,tmp);
  getline(is,tmp);
  if (tmp=="Velocities")
  {
   for(int q=0; q<1; ++q) {getline(is,tmp); (*linecounter)++; }
   for (long int i=0;i<natoms;++i)
   {
    long int id=0;
    double VX=0.0e0,VY=0.0e0,VZ=0.0e0;
    getline(is,tmp);
    words = StringSplit(tmp, ' ');
    (*linecounter)++;

    id=atoi(words[0].c_str());
    VX=atof(words[1].c_str());
    VY=atof(words[2].c_str());
    VZ=atof(words[3].c_str());
    vel = Vector(VX,VY,VZ);
    part[id-1].Velocity() = vel;
   }
  }
  else is.seekg(len, std::ios_base::beg); // Go to the beggining of the block

  len = is.tellg();
  getline(is,tmp);
  if (tmp==hdr[0]) is.seekg(len, std::ios_base::beg); // Go to the beggining of the file
  else
  {
   while (!is.eof())  getline(is,tmp);  // Ignore everything else after velocities
  }
 }

 return true;
}

bool LAMMPSFormat::SkipCell(std::istream & is) const
{
 //Type for level > 1 in data and lpmd
 std::string tmp;
 if (ftype == "dump")
 {
  getline(is, tmp);                                     // timestep number
  getline(is, tmp);                                     // "ITEM: NUMBER OF ATOMS"
  getline(is, tmp);                                     // Numero de atomos
  (*linecounter)+=3;
 
  Array<std::string> words = StringSplit(tmp, ' '); 
  if (words.Size() == 0) return false;
  long int natoms = atoi(words[0].c_str());

  getline(is, tmp);                                     // ITEM: BOX BOUNDS pp ss pp
  getline(is, tmp); getline(is, tmp); getline(is, tmp); // Cell vectors 
  getline(is, tmp);                                     // Something like "ITEM: ATOMS id type xu yu zu vx vy vz c_poteng" 
  (*linecounter)+=5;
 }
 else if (ftype == "data")
 {
  for (int i=0; i<hdr.Size(); ++i)  getline(is,tmp);
  getline(is,tmp); // Empty line after "Atoms"
 }
 for (long int i=0;i<natoms;++i)
 {
  getline(is, tmp);
  (*linecounter)++;
 }

 // Read the first line of the next configuration ("ITEM: TIMESTEP") or End of File
 if (ftype == "dump") {getline(is, tmp);  (*linecounter)++;}
 return true;
}

void LAMMPSFormat::WriteHeader(std::ostream & os, SimulationHistory * sh) const
{
 assert (sh >= (void *)NULL); //icc 869
 if (ftype!="dump" && ftype!="data")  throw PluginError("lammps", "File format ftype must be either 'dump' or 'data'"); 
 (*linecounter)=0;
}

void LAMMPSFormat::WriteCell(std::ostream & out, Configuration & con) const
{
 BasicParticleSet & part = con.Atoms();
 BasicCell & cell = con.Cell();

 if(ftype=="dump")
 {
  out << "ITEM: TIMESTEP" << std::endl;
  out << (*linecounter) << std::endl;
  out << "ITEM: NUMBER OF ATOMS" << std::endl;
  out << part.Size() << std::endl;
  if (cell.IsOrthogonal())
  {
   out << "ITEM: BOX BOUNDS pp pp pp" << std::endl;
   out << 0.0 << "  "  << cell[0].Module() << std::endl;
   out << 0.0 << "  "  << cell[1].Module() << std::endl;
   out << 0.0 << "  "  << cell[2].Module() << std::endl;
  }
  else
  {
   double xy, xz, yz, xlo, xhi, ylo, yhi, zlo, zhi;
   xy = cell[1][0];
   xz = cell[2][0];
   yz = cell[2][1];
   xlo = 0.0; xhi = cell[0][0];
   ylo = 0.0; yhi = cell[1][1];
   zlo = 0.0; zhi = cell[2][2];
   if (xy>0) xhi += xy; else xlo +=xy;
   if (xz>0) xhi += xz; else xlo +=xz;
   if (yz>0) yhi += yz; else ylo +=yz;
   out << "ITEM: BOX BOUNDS xy xz yz pp pp pp" << std::endl;
   out << xlo << "  "  << xhi << "  " << xy << std::endl;
   out << ylo << "  "  << yhi << "  " << xz << std::endl;
   out << zlo << "  "  << zhi << "  " << yz << std::endl;
  }

  Array<int> esp=part.Elements();
  std::map<int, int> elemtype;
  for (long i=0;i<esp.Size();++i) elemtype[esp[i]] = i+1;
  
  // Shift indices if they are not in the range 1...N
  int min=part.Size();
  for (long int i=0; i<part.Size(); ++i) min = (part[i].ID()<min)? part[i].ID() : min;
  min--;
   
  if(level==0)
  {
   out << "ITEM: ATOMS id type x y z" << std::endl;
   for(long int i=0;i<part.Size();i++) out << part[i].ID()-min << " " << elemtype[part[i].Z()] << " "<< part[i].Position() << std::endl;
  }
  else if(level==1)
  {
   out << "ITEM: ATOMS id type x y z vx vy vz" << std::endl;
   for(long int i=0;i<part.Size();i++) out << part[i].ID()-min << " " << elemtype[part[i].Z()] << " "<< part[i].Position() << " " << part[i].Velocity() << std::endl;
  }
  else
  {
   throw PluginError("lammps", "You must choose an appropriate value of 'level'."); 
  }

 }
 else if(ftype=="data")
 {
  out << "# LAMMPS data file written by LPMD" << std::endl;
  out << part.Size() << " atoms\n";

  Array<int> esp=part.Elements();
  Array<lpmd::Atom> at;
  std::map<int, int> elemtype;
  
  for (long i=0;i<esp.Size();++i){ elemtype[esp[i]] = i+1; at.Append(Atom(esp[i]));}

  // Shift indices if they are not in the range 1...N
  int min=part.Size();
  for (long int i=0; i<part.Size(); ++i) min = (part[i].ID()<min)? part[i].ID() : min;
  min--;
  
  out << esp.Size() << " atom types\n";
  if (cell.IsOrthogonal())
  {
   out << "0.0 " << cell[0].Module() << " xlo xhi\n";
   out << "0.0 " << cell[1].Module() << " ylo yhi\n";
   out << "0.0 " << cell[2].Module() << " zlo zhi\n";
  }
  else
  {
   double xy, xz, yz, xlo, xhi, ylo, yhi, zlo, zhi;
   xy = cell[1][0];
   xz = cell[2][0];
   yz = cell[2][1];
   xlo = 0.0; xhi = cell[0][0];
   ylo = 0.0; yhi = cell[1][1];
   zlo = 0.0; zhi = cell[2][2];
   // En el data, primero se escribe la celda ortogonal...
   out << xlo << "  "  << xhi << " xlo xhi\n";
   out << ylo << "  "  << yhi << " ylo yhi\n";
   out << zlo << "  "  << zhi << " zlo zhi\n";
   // ... y luego se le agregan los tilt
   out << xy << " "  << xz << " " << yz << " xy xz yz" << std::endl;
  }

  
  out << "\nMasses\n\n";
  for(long int i=0;i<esp.Size();i++) out << elemtype[esp[i]] << " "<< at[i].Mass() << " # " << at[i].Symbol() << std::endl;

  out << "\nAtoms\n\n";
  for(long int i=0;i<part.Size();i++) out << part[i].ID()-min << " " << elemtype[part[i].Z()] << " "<< part[i].Position() << std::endl;

  // Print velocities, if there are.
  if (level==1)
  {
   out << "\nVelocities\n\n";
   for(long int i=0;i<part.Size();i++) out << part[i].ID()-min << " "<< part[i].Velocity() << std::endl;
  }
 }

}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new LAMMPSFormat(args); }
void destroy(Plugin * m) { delete m; }

