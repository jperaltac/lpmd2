//
//
//

#include "tabulatedpair.h"

#include <iostream>
#include <fstream>

using namespace lpmd;

TabulatedPair::TabulatedPair(std::string args): Plugin("tabulatedpair", "2.1")
{ 
 ParamList & params = (*this);
 //
 DefineKeyword("file","");
 // hasta aqui los valores por omision
 ProcessArguments(args);
 cutoff = 0.0e0;
 bins = 0;
 file = params["file"];
 if(file!="") ReadTable();
}

TabulatedPair::~TabulatedPair() 
{ 
 delete [] etable;
 delete [] ftable;
}

void TabulatedPair::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = tabulatedpair                                            \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to read a tabulated pair potential from a data file. \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      file           : Specifies the filename that contains the potential in   \n";
 std::cout << "                       the form of tabulated data.                             \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use tabulatedpair as TABLJ                                                    \n";
 std::cout << "     file lennardjones.dat                                                     \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential TABLJ Ar Ar                                                       \n\n";
 std::cout << "      The plugin implements the potential contained in the file lennardjones.dat\n";
 std::cout << "      between argon (Ar).                                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void TabulatedPair::ReadTable()
{
 DebugStream() << "-> Opening Table-Potential filetype '"<<file<<"'."<<'\n';
 std::ifstream input;
 input.open(file.c_str());
 if(!input.is_open())
 {
  EndWithError("Impossible open the potential data file.");
 }

 //count the number lines ignore #.
 int n = 0;
 std::string line;
 while(!input.eof()) 
 {
  getline(input,line);
  RemoveUnnecessarySpaces(line);
  lpmd::Array<std::string> words = StringSplit(line,' ');
  if(line[0]!='#' && words.Size()==3) n++;
 }
 input.clear();input.seekg(0);
 DebugStream() << "-> Readed "<<n<<" lines " << '\n';
 bins = n;
 etable = new double[bins];
 ftable = new double[bins];
 int i=0;
 double previo = 0.0e0;
 //Asign values
 while(!input.eof())
 {
  getline(input,line);
  RemoveUnnecessarySpaces(line);
  lpmd::Array<std::string> words = StringSplit(line,' ');
  if(words.Size()==3 && line[0]!='#')
  {
   if(atof(words[0].c_str())>previo) { cutoff = atof(words[0].c_str()); previo = cutoff;}
   etable[i] = atof(words[1].c_str());
   ftable[i] = atof(words[2].c_str());
   i++;
  }
 }
 SetCutoff(cutoff);
 DebugStream() << "-> Table was readed correctly." << '\n';
}

double TabulatedPair::pairEnergy(const double & r) const
{
 // Interpolate the value of the energy from the table
 long k = long(floor(bins*r/cutoff));
 return etable[k];
}

Vector TabulatedPair::pairForce(const Vector & r) const
{
 double rr2 = r.SquareModule();
 double rr = sqrt(rr2);
 Vector fv = r;
 long k = long(floor(bins*rr/cutoff));
 fv = fv*ftable[k];
 return fv;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new TabulatedPair(args); }
void destroy(Plugin * m) { delete m; }
