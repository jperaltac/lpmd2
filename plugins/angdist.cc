//
//
//


#include "angdist.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/atompair.h>
#include <lpmd/configuration.h>

#include <sstream>

using namespace lpmd;

AngDist::AngDist(std::string args): Plugin("angdist", "2.0")
{
 trcut = 0.0;
 ParamList & param = (*this);
 //
 DefineKeyword("rcut","1");
 DefineKeyword("atoms");
 DefineKeyword("start","0");
 DefineKeyword("end","-1");
 DefineKeyword("each","1");
 DefineKeyword("output","angdist_output.dat");
 DefineKeyword("bins", "200");
 DefineKeyword("debug", "none");
 // 
 ProcessArguments(args);
 nb = int(param["bins"]);
 start = int(param["start"]);
 end = int(param["end"]);
 each = int(param["each"]);
 OutputFile() = param["output"];
}

AngDist::~AngDist()
{
}

void AngDist::SetParameter(std::string name)
{
 //#warning "SetParameter es horrible! mejorar los parametros"
 if (name == "atoms")
 {
  AssignParameter("atoms", GetNextWord());
  na = (*this)["atoms"];
  for(int i=0;i<na;i++) { satoms.push_back(GetNextWord()); }
 }
 else if (name == "rcut") 
 {
  std::string atom1 = GetNextWord();
  std::string atom2 = GetNextWord();
  std::string tmp = GetNextWord();
  double cf = atof(tmp.c_str());
  rcut[atom1+"-"+atom2] = cf;
  rcut[atom2+"-"+atom1] = cf;
  trcut += cf;
 }
 else Module::SetParameter(name);
}

void AngDist::Show(std::ostream & os) const
{
 Module::Show(os);
 if(na > 0)
 {
  os << "   Atom Number = " << na << '\n';
  os << "   Atoms       = ";
  for (unsigned int i=0;i<satoms.size();i++) os << satoms[i] << "\t";
  os << '\n';
  os << "   Cutoffs     : " << std::endl;
  for(unsigned int i=0;i<satoms.size();i++)
  {
   for(unsigned int j=i;j<satoms.size();j++)
   {
    std::string spec1=satoms[i];
    std::string spec2=satoms[j];
    std::string tmp = spec1+"-"+spec2;
    // Truco para acceder a rcut[tmp] desde un metodo const 
    const std::map<std::string, double>::const_iterator & p = rcut.find(tmp);
    double cut = (*p).second;
    // fin del truco
    os << "\t" << spec1 << "-" << spec2 << " = " << cut << std::endl;
   }
  }
 }
}

void AngDist::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = angdist                                                  \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugins is used to evaluate the angular distribution function of a   \n";
 std::cout << " simulation cell, using the cutoff as a input parameter.                       \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      bins          : Set the number of subdivisions of the grid between 0 and \n";
 std::cout << "                      180 degrees.                                             \n";
 std::cout << "      atoms         : Set the number of atomic species and the symbol of theses\n";
 std::cout << "                      in order to calculate the angular distribution.          \n";
 std::cout << "      rcut          : Set two atomic species followed by the cutoff distance   \n";
 std::cout << "                      between them.                                            \n";
 std::cout << "      output        : Output File.                                             \n";
 std::cout << "      average       : True/False Average or not the different distributions.   \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use angdist                                                                   \n";
 std::cout << "     bins 200                                                                  \n";
 std::cout << "     atoms 2 Ge O                                                              \n";
 std::cout << "     rcut Ge Ge 3.60                                                           \n";
 std::cout << "     rcut Ge O  1.90                                                           \n";
 std::cout << "     rcut O  O  3.20                                                           \n";
 std::cout << "     output angdist.dat                                                        \n";
 std::cout << "     average false                                                             \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Apply the plugin :                                                           \n";
 std::cout << " property angdist start=0 each=1 end=100                                     \n\n";
 std::cout << "      The plugin is used to evaluate the angular distribution function of the  \n";
 std::cout << "      cell each one step between the step 0 and 100.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void AngDist::Evaluate(lpmd::Configuration & con, lpmd::Potential & pot)
{
 assert(&pot != 0);//icc 869
 lpmd::BasicParticleSet & part = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 if (nb <= 0 ||  na <=0) throw PluginError("angdist", "Error with angular distribution calculation.");
 int nsp = na;
 unsigned long int N = part.Size();
 double **ang;
 ang = new double*[nb];
 for(int i=0;i<nb;i++) { ang[i]=new double[(int)(nsp*nsp*nsp)]; }
 for (int i=0;i<nb;i++) 
 { 
  for (int j=0;j<(int)(nsp*nsp*nsp);j++) ang[i][j]=0.0e0;
 }
 lpmd::Array <int> elements = part.Elements();
 lpmd::Array <std::string> lst;
 lst.Clear();
 for (int i=0;i<elements.Size();++i)
 {
  for (int j=0;j<elements.Size();++j)
  {
   for (int k=0;k<elements.Size();++k)
   {
    std::ostringstream ostr ;
    ostr << ElemSym[elements[i]] << "-" << ElemSym[elements[j]] << "-" <<ElemSym[elements[k]];
    lst.Append(ostr.str());
   }
  }
 }

 int s=0;
 for(int i=0;i<lst.Size();++i)
 {
  //Hace funcional el triplete de atomos y asigna los dos rcut
  Array<std::string> loa = StringSplit(lst[i],'-');
  int e1 = ElemNum(loa[0]);
  int e2 = ElemNum(loa[1]);
  int e3 = ElemNum(loa[2]);
  double rc12 = rcut[loa[0]+"-"+loa[1]];
  double rc23 = rcut[loa[1]+"-"+loa[2]];
  if(fabs(trcut)<1E-1) trcut = rc12+rc23;
  for(unsigned long int j=0;j<N;++j)
  {
   if (part[j].Z()==e2)
   {
    lpmd::NeighborList & nlist = con.Neighbors(j,true,trcut);
    for (long int k=0;k<nlist.Size();++k)
    {
     const lpmd::AtomPair & nn = nlist[k];
     if(nn.j->Z()==e1 && nn.r2 <= rc12*rc12)
     {
      for (long int l=0;l<nlist.Size();++l)
      {
       const lpmd::AtomPair & mm = nlist[l];
       if(&mm!=&nn)
       {
	if(mm.j->Z()==e3 && mm.r2 <= rc23*rc23)
	{
	 Vector a = cell.Displacement(part[j].Position(), nn.j->Position());
	 Vector b = cell.Displacement(part[j].Position(), mm.j->Position());
	 double angle= Angle(a,b);
	 int ig=(long)floor(nb*(angle/M_PI));
	 if(nn.j->Z()==mm.j->Z())
	 {
	  ang[ig][s]=ang[ig][s]+0.5;
	 }
	 else if(nn.j->Z()!=mm.j->Z())
	 {
	  ang[ig][s]=ang[ig][s]+1.0;
	 }
	}
       }
      }
     }
    }
   }
  }
  s++; 
 }

 //
 // Output of ang
 //
 lpmd::Matrix & m = CurrentValue();
 m = lpmd::Matrix(1 + nsp*nsp*nsp, nb);

 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "angle");
 int j=1;
 for (int i=0;i < lst.Size() ; ++i)
 {
  m.SetLabel(j, lst[i]);
  j++;
 }
 //
 for(int i=0;i<nb;i++)
 {
  m.Set(0, i, (double)180*i/nb);
  for(j=0;j<(int)(nsp*nsp*nsp);j++)
  {
   m.Set(j+1, i, ang[i][j]);
  }
 }
 for (int i=0;i<nb;i++) delete [] ang[i];
 delete [] ang;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new AngDist(args); }
void destroy(Plugin * m) { delete m; }

