//
//
//

#include "pdb.h"

#include <lpmd/util.h>
#include <lpmd/simulation.h>
#include <algorithm>
#include <cctype>
#include <iomanip>

using namespace lpmd;

PDBFormat::PDBFormat(std::string args): Plugin("pdb", "1.0")
{
 ParamList & params = (*this);
 //
 DefineKeyword("file");
 DefineKeyword("each", "1");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 writefile = params["file"];
 interval = int(params["each"]);
}

PDBFormat::~PDBFormat() { }

void PDBFormat::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = pdb                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to read/write atomic configurations files in PDB format.\n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      file          : Input/output file that contains the atomic configurations\n";
 std::cout << "                      in PDB format.                                           \n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      input/output file must be read/written.                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " input module=pdb file=configuration.pdb                                       \n";
 std::cout << " output module=pdb file=outputfile.pdb each=5                                \n\n";
 std::cout << "      The plugin is used to read and write atomic configurations in PDB format.\n";
 std::cout << "      The file extension (.pdb) is irrelevant, what matters is the module      \n";
 std::cout << "      loaded (module=pdb).                                                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void PDBFormat::WriteHeader(std::ostream & os, SimulationHistory * sh) const
{
 assert(&os != 0); //icc 869
 assert(sh >= (void *)NULL); //icc 869
 // PDB no tiene ningun header especial
}

void PDBFormat::WriteCell(std::ostream & out, Configuration & con) const
{
 BasicParticleSet & part = con.Atoms();
 //BasicCell & cell = con.Cell();

 out << "COMPND   From lpmd\n";
 out << "AUTHOR   pdb plugin for lpmd\n";
 for (long int i=0;i<part.Size();++i)
 {
  const Vector & pos = part[i].Position();
  out << std::setw(6) << "HETATM"; //1-6
  out << std::setw(5) << (i+1); //7-11
  std::string sy = part[i].Symbol();
  std::transform(sy.begin(), sy.end(), sy.begin(), (int(*) (int)) std::toupper);
  out << std::setw(1) << " ";
  out << std::setw(4) <<std::left<< sy ; //13-16
  out << std::setw(1) << " "; //17
  out << std::setw(3) <<std::left<< "LIG"; //18-20
  out << std::right;
  out << std::setw(1) << " ";
  out << std::setw(1) << " "; //22
  out << std::setw(4) << (i+1); //23-26
  out << std::setw(1) << " "; //27
  out << std::setw(3) << "  ";
  out << std::setw(8) << std::fixed <<std::setprecision(3)<< pos[0]; //31-38
  out << std::setw(8) << std::fixed <<std::setprecision(3)<< pos[1]; //39-46
  out << std::setw(8) << std::fixed <<std::setprecision(3)<< pos[2]; //47-54
  out << std::right;
  out << std::setw(7) << "1.00"; //55-60
  out << std::setw(6) << "0.00"; //61-66
  out << std::setw(8) << "        ";
  out << std::setw(4) << " "; //73-76
  out << std::setw(2) << sy ; //77-78
  out << std::setw(2) << " "; //79-80
  out << '\n';
 }
 out << "END\n\n";
}

void PDBFormat::ReadHeader(std::istream & is) const
{
 assert(&is != 0); //icc 869
 // PDB no tiene header especial para leer
}

bool PDBFormat::ReadCell(std::istream & is, Configuration & con) const
{
 BasicParticleSet & part = con.Atoms();
 //BasicCell & cell = con.Cell();
 assert (part.Size() == 0);

 long natoms=0;
 std::vector<lpmd::Atom> atomlist;
 if(is.eof()) return false;
 while(!is.eof())
 {
  std::string tmp;
  getline(is,tmp);
  if(tmp[0]!='#')
  {
   if (tmp.compare(0,6,"HETATM")==0)
   {
    natoms++;
    Array<std::string> linea = StringSplit(tmp,' ');
    std::string symb = linea[2];
    if(symb.length()>1)
    {
     for(unsigned int j=1;j<symb.length();j++) symb[j]=tolower(symb[j]);
    }
    lpmd::Vector pos(atof(linea[5].c_str()),atof(linea[6].c_str()),atof(linea[7].c_str()));
    part.Append(Atom(symb,pos));
   }
   if (tmp.compare(0,3,"END")==0)
   {
    return true;
   }
  }
 }
 return false;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new PDBFormat(args); }
void destroy(Plugin * m) { delete m; }

