//
//
//

#include "lpmd.h"

#include <arpa/inet.h>
#include <zlib.h>

#include <lpmd/util.h>
#include <lpmd/simulation.h>
#include <lpmd/atom.h>
#include <lpmd/colorhandler.h>
#include <stdio.h>
#include <string.h>

#define ZLP_NONE 0
#define ZLP_READ 1
#define ZLP_WRITE 2

using namespace lpmd;

LPMDFormat::LPMDFormat(std::string args): Plugin("lpmd", "2.0")
{
 v0 = 0;
 hdr.Clear();
 ParamList & params = (*this);
 //
 linecounter = new long int;
 DefineKeyword("file" ,"noname");
 DefineKeyword("each", "1");
 DefineKeyword("level", "0");
 DefineKeyword("extra" ,"");
 DefineKeyword("replacecell", "false");
 DefineKeyword("blocksize","1024");
 DefineKeyword("compression","6");
 DefineKeyword("type","lpmd");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 type = params["type"];
 readfile = writefile = (*this)["file"];
 std::string q = readfile.substr(readfile.size()-4,4);
 if (strcmp(q.c_str(),"lpmd")==0) { type = "lpmd"; }
 else if (strcmp(q.c_str(),".zlp")==0) { type = "zlp"; }
 interval = int(params["each"]);
 level = int(params["level"]);
 blocksize = int(params["blocksize"]);
 complev = int(params["compression"]);
 rcell = params["replacecell"];
 extra = StringSplit((*this)["extra"],',');
 // inicializa la estructura z_stream
 if(type=="zlp")
 {
  zstr = (void *)(new z_stream);
  lastop = new int(ZLP_NONE);
  inbuf = new unsigned char[blocksize];
  outbuf = new unsigned char[blocksize];
 }
 else
 {
  zstr = NULL;
  lastop = NULL;
  inbuf = NULL;
  outbuf = NULL;
 }
}

LPMDFormat::~LPMDFormat()
{ 
 if (type == "zlp")
 {
  if ((*lastop) == ZLP_WRITE) deflate((z_stream *)(zstr), Z_FINISH);
  if ((*lastop) == ZLP_READ) inflateEnd((z_stream *)(zstr));
  if ((*lastop) == ZLP_WRITE) deflateEnd((z_stream *)(zstr));
  delete lastop;
  delete [] inbuf;
  delete [] outbuf;
  delete (z_stream *)(zstr);
 }
 delete linecounter;
}

void LPMDFormat::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = lpmd                                                     \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to read/write LPMD's atomic configurations files     \n";
 std::cout << "      (lpmd or zlp). This is the LPMD's own format, with scaled positions      \n";
 std::cout << "      (between 0 and 1).                                                       \n";  
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      file          : Input/output file that contains the atomic configurations\n";
 std::cout << "                      in LPMD format.                                          \n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      input/output file must be read/written.                  \n";
 std::cout << "      level         : Determines the file format level (0/1/2):                \n";
 std::cout << "                      0/1/2 <-> pos/pos-vel/pos-vel-acel.                      \n";
 std::cout << "      extra         : Extra information about the file (RGB/C/TYPE):           \n";
 std::cout << "                      RGB (red, green, blue): The atoms in the file (will) have\n";
 std::cout << "                       colors in the format <R,G,B> in the input/output file,  \n";
 std::cout << "                       with R, G and B between 0 and 1.                        \n";
 std::cout << "                      C: The color of the atoms is represented by a real number\n";
 std::cout << "                       between 0 and 1 in the input/ouput file.                \n";
 std::cout << "      type          : File type (lpmd / zlp):                                  \n";
 std::cout << "                      lpmd: normal mode.                                       \n";
 std::cout << "                      zlp: binary (compressed) mode.                           \n";
 std::cout << "      blocksize     : Compression block size (in bytes) for zlp files.         \n";
 std::cout << "      compression   : Compression level for zlp files (from 0 to 9, being 9 the\n";
 std::cout << "                      maximum compression level; but the slowest).             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input module=lpmd file=inputfile.lpmd level=0                                 \n";
 std::cout << " input module=lpmd file=inputfile.zlp level=1                                  \n";
 std::cout << " output module=lpmd file=output.zlp level=1 type=zlp each=5                    \n";
 std::cout << " output module=lpmd file=outputfile.lpmd level=1 each=5                      \n\n";
 std::cout << "      The plugin is used to read and write atomic configurations in LPMD's     \n";
 std::cout << "      formats. The file extension (.lpmd or .zlp) is irrelevant, what matters  \n";
 std::cout << "      is the module loaded (module=lpmd).                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void LPMDFormat::ReadHeader(std::istream & is) const
{
 char first[2] = { NULL, NULL };
 is.read(first, 1);
 if (first[0] == 'Z')
 {
  ReadHeaderZLPOne(is);
  return;
 }
 else if (first[0] != 'L') throw PluginError("lpmd", "Wrong header (not an LPMD/ZLP file)");
 std::string line, header;
 getline(is, line);
 line = "L"+line;
 Array<std::string> lspl = StringSplit(line);
 // First word must be LPMD
 if (lspl[0] != "LPMD") throw PluginError("lpmd", "Wrong header (not an LPMD/ZLP file)");
 // Second word is the version number
 std::string version = lspl[1];
 // Third word is the compression flag
 type = "lpmd";
 if (lspl.Size() > 2)
 {
  if (lspl[2] == "Z") type = "zlp";
  else if (lspl[2] == "L") type = "lpmd";
  else throw PluginError("lpmd", "Wrong compression flag (corrupted LPMD/ZLP file?)");
 }

 (*linecounter) = 1;

 if (version == "1.0") ReadHeaderLPMDOne(is);
 else if (version == "2.0")
 {
  // Second line is the header
  getline(is, header);
  (*linecounter)++;
  Array<std::string> words = StringSplit(header);
  if (words[0] != "HDR") throw PluginError("lpmd", "File "+readfile+" doesn't seem to be in LPMD 2.0 format (wrong HDR)");
  for (long int i=0;i<words.Size();++i) hdr.Append(std::string(words[i]));
 }
 else throw PluginError("lpmd", "Format version "+version+" in file "+readfile+" is not supported.");
 if (type == "zlp") InitDecompression();
}

void LPMDFormat::ReadHeaderLPMDOne(std::istream & is) const
{
 //assume 1.0 format
 int where = is.tellg();
 std::string info;
 getline(is, info);
 getline(is, info);
 getline(is, info);
 Array<std::string> words = StringSplit(info, ' ');
 is.seekg(where);
 if (words.Size()==4)
 {
  hdr.Append(std::string("HDR"));
  hdr.Append(std::string("SYM"));
  hdr.Append(std::string("X"));hdr.Append(std::string("Y"));hdr.Append(std::string("Z"));
  level = 0;
 }
 else if (words.Size()==7)
 {
  hdr.Append(std::string("HDR"));
  hdr.Append(std::string("SYM"));
  hdr.Append(std::string("X"));hdr.Append(std::string("Y"));hdr.Append(std::string("Z"));
  hdr.Append(std::string("VX"));hdr.Append(std::string("VY"));hdr.Append(std::string("VZ"));
  level = 1;
 }
 else if (words.Size()==10)
 {
  hdr.Append(std::string("HDR"));
  hdr.Append(std::string("SYM"));
  hdr.Append(std::string("X"));hdr.Append(std::string("Y"));hdr.Append(std::string("Z"));
  hdr.Append(std::string("VX"));hdr.Append(std::string("VY"));hdr.Append(std::string("VZ"));
  hdr.Append(std::string("FX"));hdr.Append(std::string("FY"));hdr.Append(std::string("FZ"));
  level = 2;
 }
 else throw PluginError("lpmd", "File "+readfile+" not have a apropiate 1.0 version");
}

void LPMDFormat::ReadHeaderZLPOne(std::istream & is) const
{
 char hdr[10];
 hdr[0] = 'Z';
 is.read(&hdr[1], 9);
 if ((hdr[2] != 'L') || (hdr[4] != 'P')) throw PluginError("zlp", "Wrong header");
 if ((hdr[1] != '4') || (hdr[3] != '2')) throw PluginError("zlp", "Wrong header");
 v0 = hdr[5];   // major version number
 unsigned short int v1 = hdr[6];   // minor version number
 unsigned short int v2 = hdr[7];   // revision 
 unsigned short int bf0 = hdr[8];  // reservado para 8-bit flag
 unsigned short int bf1 = hdr[9];  // reservado para 8-bit flag
 DebugStream() << "[zlp] version number from file is " << v0 << "." << v1 << "." << v2 << '\n';
 DebugStream() << "[zlp] format flags are " << bf0 << " and " << bf1 << '\n';
 InitDecompression();
}

void LPMDFormat::InitDecompression() const
{
 *lastop = ZLP_READ;
 z_stream & stream = *((z_stream *)(zstr));
 stream.zalloc = Z_NULL;
 stream.zfree = Z_NULL;
 stream.opaque = Z_NULL;
 if (inflateInit(&stream) != Z_OK) throw PluginError("zlp", "Decompression failed");
}

// 
// Reads a configuration from a LPMD file 
//
bool LPMDFormat::ReadCell(std::istream & is, Configuration & con) const
{
 BasicCell & cell = con.Cell();
 BasicParticleSet & part = con.Atoms();
 assert(part.Size() == 0);
 
 std::istringstream bufstr(std::istringstream::in);
 std::string * istr = new std::string;
 if(type=="zlp")
 {
  *lastop = ZLP_READ;
  z_stream & stream = *((z_stream *)(zstr));
  long int ts = 0;                                // total de bytes leidos hasta ahora
  unsigned char foo;
  unsigned long int cbufs;
  is.read((char *)&foo, 1);
  if (is.eof()) return false;
  is.read((char *)&cbufs, int(foo));
  cbufs = ntohl(cbufs);
  while (1)
  {
   long int rem = cbufs - ts;
   if (rem == 0) break;
   long int chunk = blocksize;
   if (chunk > rem) chunk = rem;
   is.read((char *)inbuf, chunk);
   ts += is.gcount();
   stream.avail_in = is.gcount();
   stream.next_in = (unsigned char *)(inbuf);
   do
   {
    stream.avail_out = blocksize;
    stream.next_out = (unsigned char *)(outbuf);
    int f = (is.eof() ? Z_FINISH : Z_SYNC_FLUSH);
    inflate(&stream, f);                           // no chequea el estado aun 
    int have = blocksize-stream.avail_out;
    for (int q=0;q<have;++q) (*istr) += (char)(outbuf[q]);
   } while (stream.avail_out == 0);
  }
 }
 std::istringstream ibufstr(*istr);
 std::string tmp;
 if ( v0 == 1 )
 {
  int lvl;
  long int natoms;
  ibufstr >> lvl;
  ibufstr >> natoms;
  con.SetTag(con, Tag("level"), lvl);
  for (int j=0;j<3;++j)
  {
   Vector v;
   double vq[3];
   for (int i=0;i<3;++i)
   {
    ibufstr >> vq[i];
    v[i] = vq[i];
   }
   if ((*this)["replacecell"] == "true") cell[j] = v;
  }
  for (long int i=0;i<natoms;++i)
  {
   std::string sym;
   ibufstr >> sym;
   Vector fpos, vel, acc;
   double vq[3];
   for (int q=0;q<3;++q)
   {
    ibufstr >> vq[q];
    fpos[q] = vq[q];
   }
   part.Append(Atom(ElemNum(sym)));
   part[i].Position() = cell.Cartesian(fpos);
   if (lvl > 0)
   {
    for (int q=0;q<3;++q)
    {
     ibufstr >> vq[q];
     vel[q] = vq[q];
    }
    part[i].Velocity() = vel;
   }
   if (lvl > 1) 
   {
    for (int q=0;q<3;++q)
    {
     ibufstr >> vq[q];
     acc[q] = vq[q];
    }
    part[i].Acceleration() = acc;
   }
  }
  delete istr;
  return true;
 }


 //Type for level > 1 in zlp and lpmd
 if (type == "lpmd")
 {
  getline(is, tmp);                                     // Numero de atomos
 }
 else if (type == "zlp")
 {
  getline(ibufstr, tmp);
 }
 (*linecounter)++;
 Array<std::string> words = StringSplit(tmp, ' '); 
 if (words.Size() == 0) return false;
 long int natoms = atoi(words[0].c_str());
 if (type == "lpmd") getline(is, tmp);                                     // Vectores de la celda
 else if (type == "zlp") getline(ibufstr, tmp);
 (*linecounter)++;
 words = StringSplit(tmp, ' '); 
 con.SetTag(con, Tag("level"), level);
 if(words.Size()==9)
 {
  if ((*this)["replacecell"] == "true")
  {
   cell[0] = Vector(atof(words[0].c_str()), atof(words[1].c_str()), atof(words[2].c_str()));
   cell[1] = Vector(atof(words[3].c_str()), atof(words[4].c_str()), atof(words[5].c_str()));
   cell[2] = Vector(atof(words[6].c_str()), atof(words[7].c_str()), atof(words[8].c_str()));
  }
 }
 else if(words.Size()==6)
 {
  if ((*this)["replacecell"] == "true")
  {
   Cell tmp2(atof(words[0].c_str()),atof(words[1].c_str()),atof(words[2].c_str()),atof(words[3].c_str()),atof(words[4].c_str()),atof(words[5].c_str()));
   for (int q=0;q<3;++q) cell[q] = tmp2[q];
  }
 }
 else throw PluginError("lpmd", "Error ocurred when reading the base vectors, file \""+readfile+"\", line "+ToString<int>(*linecounter));
 long int atomcount = 0;
 for (long int i=0;i<natoms;++i)
 {
  if (type == "lpmd") getline(is, tmp);
  else if (type == "zlp") getline(ibufstr, tmp);
  (*linecounter)++;
  words = StringSplit(tmp, ' ');
  if (words.Size() == 0) 
  {
   throw PluginError("lpmd", "Error ocurred, the atom file not have elements!"); 
  }
  else if (words.Size() >=1)
  {
   std::string sym;
   double X=0.0e0,Y=0.0e0,Z=0.0e0;
   double VX=0.0e0,VY=0.0e0,VZ=0.0e0;
   double AX=0.0e0,AY=0.0e0,AZ=0.0e0;
   lpmd::Color color(0,0,0);
   bool color_active = false;
   for (long int k=1 ; k < hdr.Size() ; ++k)
   {
    if (hdr[k] == "SYM") sym=words[k-1];
    if (hdr[k] == "X") X=atof(words[k-1].c_str());
    if (hdr[k] == "Y") Y=atof(words[k-1].c_str());
    if (hdr[k] == "Z") Z=atof(words[k-1].c_str());
    if (hdr[k] == "VX") VX=atof(words[k-1].c_str());
    if (hdr[k] == "VY") VY=atof(words[k-1].c_str());
    if (hdr[k] == "VZ") VZ=atof(words[k-1].c_str());
    if (hdr[k] == "AX") AX=atof(words[k-1].c_str());
    if (hdr[k] == "AY") AY=atof(words[k-1].c_str());
    if (hdr[k] == "AZ") AZ=atof(words[k-1].c_str());
    if (hdr[k] == "RGB" || hdr[k] == "rgb") { color = Vector(words[k-1].c_str()); color_active = true; }
    if (hdr[k] == "C") { color=ColorFromScalar(atof(words[k-1].c_str())); color_active = true; }
   }
   Vector pos = cell.Cartesian(Vector(X,Y,Z));
   Vector vel(VX,VY,VZ);
   Vector ace(AX,AY,AZ);
   lpmd::Atom atm(sym,pos,vel,ace);
   part.Append(atm);
   const BasicAtom & realatom = part[atomcount];
   if (color_active) ColorHandler::ColorOfAtom(realatom) = color;
  }
  else throw PluginError("lpmd", "An unidentified line was found in the file \""+readfile+"\", line "+ToString<int>(*linecounter));
  atomcount++;
 }
 delete istr;
 return true;
}

bool LPMDFormat::SkipCell(std::istream & is) const
{
 if (type=="zlp") *lastop = ZLP_READ;
 z_stream & stream = *((z_stream *)(zstr));
 std::istringstream bufstr(std::istringstream::in);
 std::string * istr = new std::string;
 if (type=="zlp")
 {
  long int ts = 0;                                // total de bytes leidos hasta ahora
  unsigned char foo;
  unsigned long int cbufs;
  is.read((char *)&foo, 1);
  if (is.eof()) return false;
  is.read((char *)&cbufs, int(foo));
  cbufs = ntohl(cbufs);
  while (1)
  {
   long int rem = cbufs - ts;
   if (rem == 0) break;
   long int chunk = blocksize;
   if (chunk > rem) chunk = rem;
   is.read((char *)inbuf, chunk);
   ts += is.gcount();
   stream.avail_in = is.gcount();
   stream.next_in = (unsigned char *)(inbuf);
   do
   {
    stream.avail_out = blocksize;
    stream.next_out = (unsigned char *)(outbuf);
    int f = (is.eof() ? Z_FINISH : Z_SYNC_FLUSH);
    inflate(&stream, f);                           // no chequea el estado aun 
    int have = blocksize-stream.avail_out;
    for (int q=0;q<have;++q) (*istr) += (char)(outbuf[q]);
   } while (stream.avail_out == 0);
  }
 }
 std::istringstream ibufstr(*istr);
 if ( v0 == 1 )
 {
  int lvl;
  long int natoms;
  ibufstr >> lvl;
  ibufstr >> natoms;
  for (int j=0;j<3;++j)
  {
   double vq[3];
   for (int i=0;i<3;++i) ibufstr >> vq[i];
  }
  for (long int i=0;i<natoms;++i)
  {
   std::string sym;
   ibufstr >> sym;
   double vq[3];
   for (int q=0;q<3;++q) ibufstr >> vq[q];
   if (lvl > 0)
   {
    for (int q=0;q<3;++q) ibufstr >> vq[q];
   }
   if (lvl > 1) 
   {
    for (int q=0;q<3;++q) ibufstr >> vq[q];
   }
  }
  delete istr;
  return true;
 }

 //Type for level > 1 in zlp and lpmd
 std::string tmp;
 if (type == "lpmd") getline(is, tmp);
 else if (type == "zlp") getline(ibufstr, tmp);
 Array<std::string> words = StringSplit(tmp, ' '); 
 if (words.Size() == 0) return false;
 long int natoms = atoi(words[0].c_str());
 if (type == "lpmd") getline(is, tmp);                                     // Vectores de la celda
 else if (type == "zlp") getline(ibufstr, tmp);
 for (long int i=0;i<natoms;++i)
 {
  if (type == "lpmd") getline(is, tmp);
  else if (type == "zlp") getline(ibufstr, tmp);
 }
 delete istr;
 return true;
}

void LPMDFormat::WriteHeader(std::ostream & os, SimulationHistory * sh) const
{
 assert (sh >= (void *)NULL); //icc 869
 os << "LPMD 2.0 ";
 if (type == "zlp")
 {
  // Initialize buffers for compression
  *lastop = ZLP_WRITE;
  z_stream & stream = *((z_stream *)(zstr));
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  if (deflateInit(&stream, complev) != Z_OK) throw PluginError("zlp", "Compression failed");
  os << "Z\n";
 }
 else os << "L\n";
 os << "HDR ";
 if (hdr.Size() < 2)
 {
  //hdr not set, using the plugin information.
  hdr.Clear();
  hdr.Append("SYM");
  hdr.Append("X");hdr.Append("Y");hdr.Append("Z");
  if (level >= 1)
  {
   hdr.Append("VX");
   hdr.Append("VY");
   hdr.Append("VZ");
  }
  if (level >= 2)
  {
   hdr.Append("AX");
   hdr.Append("AY");
   hdr.Append("AZ");
  }
  if (extra.Size() > 0)
  {
   for(long int i=0;i<extra.Size();++i) hdr.Append(extra[i].c_str());
  }
 }
 for (long int i=0;i<hdr.Size();++i) os << hdr[i] << " ";
 os << '\n';
 os.flush();
}

void LPMDFormat::WriteCell(std::ostream & out, Configuration & con) const
{
 BasicParticleSet & part = con.Atoms();
 BasicCell & cell = con.Cell();

 if(type=="zlp"){*lastop = ZLP_WRITE;}
 z_stream & stream = *((z_stream *)(zstr));
 std::ostringstream * ostr = new std::ostringstream();
 std::ostringstream & obufstr = *ostr;

 obufstr << part.Size() << std::endl;
 obufstr << std::fixed << cell[0] << " " << cell[1] << " " << cell[2] << std::endl;
 for (long int i=0;i<part.Size();i++)
 {
  if (level>=0)
  {
   obufstr << std::fixed << part[i].Symbol() << " " << cell.Fractional(part[i].Position()) ;
  }
  if (level>=1)
  {
   obufstr <<std::fixed << " "<< part[i].Velocity();
  }
  if (level>=2)
  {
   obufstr <<std::fixed << " "<< part[i].Acceleration();
  }
  if (extra.Size()>=1)
  {
   for (long int j=0 ; j < extra.Size() ; ++j)
   {
    if ((extra[j] == "RGB") || (extra[j] == "rgb"))
    { 
     lpmd::Vector tmp = (ColorHandler::HaveColor(part[i]) ? ColorHandler::ColorOfAtom(part[i]) : ColorHandler::DefaultColor(part[i])); 
     obufstr << " ";
     FormattedWrite(obufstr,tmp);
    }
    else if ((extra[j] == "TYPE") || (extra[j] == "type")) { obufstr << "          " << "ATOMTYPE"; }
   }
  }
  obufstr << '\n';
 }
 //Comprime o no segun sea lpmd o zlp.
 std::istringstream * istr = new std::istringstream(obufstr.str());
 std::istringstream & ibufstr = *istr;

 if(type=="lpmd")
 {
  out << obufstr.str();
 }
 else if(type=="zlp")
 {
  std::string cbuf;
  while (1)
  {
   ibufstr.read((char *)inbuf, blocksize);
   if (ibufstr.gcount() == 0) break;
   stream.avail_in = ibufstr.gcount();
   stream.next_in = inbuf;
   do
   {
    stream.avail_out = blocksize;
    stream.next_out = outbuf;
    if (deflate(&stream, Z_SYNC_FLUSH) == Z_STREAM_ERROR) throw PluginError("zlp", "Compression failed");
    int have = blocksize-stream.avail_out;
    for (int q=0;q<have;++q) cbuf += (char)(outbuf[q]);
   } while (stream.avail_out == 0);
  }
  unsigned char foo = sizeof(unsigned long int);
  unsigned long int cbufs = htonl(cbuf.size());
  out.write((char *)&foo, 1);
  out.write((char *)&cbufs, int(foo));
  out.write(cbuf.c_str(), cbuf.size());
  out.flush();
 }
 else throw PluginError("lpmd", "Not defined the correct type file to write header.");
 delete istr;
 delete ostr;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new LPMDFormat(args); }
void destroy(Plugin * m) { delete m; }

