//
//
//

#include "tempprofile.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/simulation.h>
#include <lpmd/plugin.h>
#include <lpmd/session.h>

#include <sstream>

using namespace lpmd;

TempProfile::TempProfile(std::string args): Plugin("tempprofile", "2.0")
{
 ParamList & param = (*this);
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("output");
 DefineKeyword("bins", "200");
 range[0][0]=0.0e0;range[0][1]=0.0e0;
 range[1][0]=0.0e0;range[1][1]=0.0e0;
 range[2][0]=0.0e0;range[2][1]=0.0e0;
 ProcessArguments(args);
 bins = int(param["bins"]);
 start = int(param["start"]);
 end = int(param["end"]);
 each = int(param["each"]);
 OutputFile() = param["output"];
}

TempProfile::~TempProfile()
{
}

void TempProfile::SetParameter(std::string name)
{
 //#warning "SetParameter es horrible! mejorar los parametros"
 if (name == "axis") 
 {
  AssignParameter("axis", GetNextWord());
  std::string eje=(*this)["axis"];
  if((eje == "X" || eje == "x") || eje == "0") axis=0;
  else if((eje == "Y" || eje == "y") || eje == "1") axis=1;
  else if((eje == "Z" || eje == "z") || eje == "2") axis=2;
  else 
  {
   axis=-1;
   ShowWarning("plugin tempprofile", "Wrong setting of axis.");
  }
 }
 else if (name == "range")
 {
  int tmp=0;
  std::string eje=GetNextWord();
  if((eje == "X" || eje == "x") || eje == "0") tmp=0;
  else if((eje == "Y" || eje == "y") || eje == "1") tmp=1;
  else if((eje == "Z" || eje == "z") || eje == "2") tmp=2;
  else 
  {
   tmp=-1;
   ShowWarning("plugin tempprofile", "Wrong setting of axis range.");
  }
  std::string smin=GetNextWord();
  if(smin=="ALL" || smin == "all")
  {
   range[tmp][0]=0;
   range[tmp][1]=0;
  }
  else
  {
   std::string smax=GetNextWord();
   range[tmp][0]=atof(smin.c_str());
   range[tmp][1]=atof(smax.c_str());
  }
  if(range[tmp][0]>=range[tmp][1] &&  smin!="all" && smin!="ALL") ShowWarning("tempprofile", "min and max values are not consistent.");
 }
 else Module::SetParameter(name);
}

void TempProfile::Show(std::ostream & os) const
{
 Module::Show(os);
 os << "   axis         = " << axis << '\n';
 os << "   range       = X  [" <<range[0][0]<<","<<range[0][1]<<"]"<< '\n';
 os << "               = Y  [" <<range[1][0]<<","<<range[1][1]<<"]"<< '\n';
 os << "               = Z  [" <<range[2][0]<<","<<range[2][1]<<"]"<< '\n';
}

void TempProfile::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = tempprofile                                              \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to evaluate a one-dimensional temperature profile    \n";
 std::cout << "      of the simulation cell.                                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      axis          : Set the axis where the evaluation will be realized.      \n";
 std::cout << "      bins          : Set the number of bins for the required axis.            \n";
 std::cout << "      range         : Set the range for the evaluation in each axis.           \n";
 std::cout << "      output        : Output file with the temperature.                        \n";
 std::cout << "      average       : Evaluate an average over the configurations or not.      \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use tempprofile                                                               \n";
 std::cout << "     axis X                                                                    \n";
 std::cout << "     bins 200                                                                  \n";
 std::cout << "     range Y 10 20                                                             \n";
 std::cout << "     range Z all                                                               \n";
 std::cout << "     range X all                                                               \n";
 std::cout << "     output temperature.out                                                    \n";
 std::cout << "     average true                                                              \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";  
 std::cout << " property tempprofile start=1 each=10 end=100                                \n\n";
 std::cout << "      The plugin is used to perform a temperature profile of the sample in     \n";
 std::cout << "      X-axis, divided in 200 slices, and just for the atoms with Y-coordinate  \n";
 std::cout << "      between 10 and 20. The data is written in the file temperature.dat.      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void TempProfile::Evaluate(Configuration & con, Potential & pot)
{
 assert(&pot != 0); // icc 869
 lpmd::BasicParticleSet & atoms = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 lpmd::Array<int> elements = atoms.Elements();
 if (bins == 0) throw PluginError("tempprofile", "Error in calculation: Wrong value for \"bins\".");

 //Vectores base, celda de simulacion.
 if(fabs(range[0][0])<1E-5 && fabs(range[0][1])<1E-5) {range[0][0]=0;range[0][1]=cell[0].Module();}
 if(fabs(range[1][0])<1E-5 && fabs(range[1][1])<1E-5) {range[1][0]=0;range[1][1]=cell[1].Module();}
 if(fabs(range[2][0])<1E-5 && fabs(range[2][1])<1E-5) {range[2][0]=0;range[2][1]=cell[2].Module();}

 if(fabs(range[0][0]-range[0][1])<1E-5) throw PluginError("tempprofile", "Error in cell range in axis X.");
 if(fabs(range[1][0]-range[1][1])<1E-5) throw PluginError("tempprofile", "Error in cell range in axis Y.");
 if(fabs(range[2][0]-range[2][1])<1E-5) throw PluginError("tempprofile", "Error in cell range in axis Z.");

 Vector na = cell[0]; na.Normalize();
 Vector nb = cell[1]; nb.Normalize();
 Vector nc = cell[2]; nc.Normalize();

 Vector la = na*range[0][1]-na*range[0][0];
 Vector lb = nb*range[1][1]-nb*range[1][0];
 Vector lc = nc*range[2][1]-nc*range[2][0];

 double dr=0.0e0;

 if(axis==0) {dr=la.Module()/double(bins);}
 else if(axis==1) {dr=lb.Module()/double(bins);}
 else if(axis==2) {dr=lc.Module()/double(bins);}
 else {throw PluginError("tempprofile", "Error in axis setting to set 'dr'!.");}

 int nsp = elements.Size();
 long int N = atoms.Size();
 long int *nab = new long int[bins];
 double **temp, *tempt;
 temp = new double*[bins];
 for(int i=0;i<bins;i++) { temp[i]=new double[(int)(nsp)]; }
 tempt = new double[bins]; //temperatura total
 for (int i=0;i<bins;i++) 
 { 
  tempt[i]=0.0e0;
  nab[i] = 0;
  for (int j=0;j<(int)(nsp);j++) temp[i][j]=0.0e0;
 }
 int s=0;
 const double kin2ev = double(GlobalSession["kin2ev"]);
 const double kboltzmann = double(GlobalSession["kboltzmann"]);
 for (int i=0;i<elements.Size();++i)	   
 {
  //Asigna la especie correspondiente.
  int e = elements[i];
  //Cuenta los atomos de la especie e.
  int ne=0;
  for(long int m=0;m<N;m++)
  {
   if(atoms[m].Z()==e) ne++;
  }
  //Comienza la iteracion principal para el calculo.
  for(long int j=0;j<N;++j)
  {
   if(atoms[j].Z()==e)
   {
    //vemos la ubicacion atomica respecto a nuestra "rejilla".
    lpmd::Vector position = atoms[j].Position();
    lpmd::Vector velocity = atoms[j].Velocity();
    double m = atoms[j].Mass();
    double x = position[0];
    double y = position[1];
    double z = position[2];
    if(x>=range[0][0] && x<=range[0][1])
    {
     if(y>=range[1][0] && y<=range[1][1])
     {
      if(z>=range[2][0] && z<=range[2][1])
      {
       //Entonces esta dentro de nuestros "ranges"
       //ahora la ubicamos segun nuestro eje preferencial 'axis'.
       double pp=0.0e0;
       if (axis==0) pp = (x-range[0][0]);
       else if(axis==1) pp = (y-range[1][0]);
       else if(axis==2) pp = (z-range[2][0]);
       else ShowWarning("plugin tempprofile", "Bad calculation of tempprofile, check your 'axis' option.");
       int ir = (long) floor(pp/dr);
       if ((ir >= 0) && (ir < bins))
       {
        temp[ir][s] += 0.5*m*velocity.SquareModule()*kin2ev; //Solo son los aportes a la energia cinetica.
        nab[ir]++;
       }
      }
     }
    }
   }
  }
  //Reasigna el verdadero valor de tempratura, segun la cantidad de atomos de la especie considerada 
  //en cada bins (nab).
  for(int j=0;j<bins;j++)
  {
   temp[j][s] = ((2.0/3.0)*temp[j][s])/(kboltzmann*double(nab[j]));
  }
  s++;
 }
 //Calcula el valor de temp(r) total.
 int j=0;
 for(int i=0;i<elements.Size();++i)
 {
  for(int k=0;k<bins;k++)
  {
   tempt[k] += temp[k][j];
  }
  j++;
 }
 //
 // Output of rho(r)
 //
 lpmd::Matrix & m  = CurrentValue();
 m = lpmd::Matrix(3 + nsp, bins);

 // Asigna los labels al objeto Matrix para cada columna
 m.SetLabel(0, "r");
 m.SetLabel(1, "t");
 m.SetLabel(nsp+2, "total Temp(r)");
 j=2;
 for (int i=0;i<elements.Size();++i)
 {
  m.SetLabel(j, elements[i]+" temp(r)");
  j++;
 }
 // 
 for(int i=0;i<bins;i++)
 {
  m.Set(0, i, dr*i);
  m.Set(1, i, (double)counter);
  for(j=0;j<(int)(nsp);j++)
  {
   m.Set(j+2, i, temp[i][j]);
  }
  m.Set(nsp+2, i, tempt[i]);
 }
 delete [] tempt;
 for (int i=0;i<bins;i++) delete [] temp[i];
 delete [] temp;
 delete [] nab; 
 counter++;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TempProfile(args); }
void destroy(Plugin * m) { delete m; }


