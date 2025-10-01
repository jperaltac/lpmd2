//
//
//


#include "searchfill.h"

#include <lpmd/matrix.h>
#include <lpmd/util.h>
#include <lpmd/atompair.h>
#include <lpmd/atom.h>
#include <lpmd/configuration.h>
#include <fstream>
#include <set>
#include <sstream>

using namespace lpmd;

double fover(double r)
{
 return (1.0 - 0.5*r - r*r/8.0 + r*r*r/16.0);
}

SearchFill::SearchFill(std::string args): Plugin("searchfill", "1.0")
{
 ParamList & param = (*this);
 //
 DefineKeyword("start","0");
 DefineKeyword("end","-1");
 DefineKeyword("each","1");
 DefineKeyword("R0","1.0");
 DefineKeyword("K", "4.1589");
 DefineKeyword("VACOVP", "0.1");
 DefineKeyword("VACMAX", "100000");
 DefineKeyword("GoodEn", "-1.0");
 DefineKeyword("MaxSteps", "750000");
 DefineKeyword("RiskSteps", "1000");
 DefineKeyword("RiskLimit", "80");
 DefineKeyword("debug", "none");
 DefineKeyword("VacSym", "V");
 DefineKeyword("Boundary", "e");
 DefineKeyword("min_coord", "1");
 DefineKeyword("max_coord", "15");
 // 
 ProcessArguments(args);
 start = int(param["start"]);
 end = int(param["end"]);
 each = int(param["each"]);
 R0 = double(param["R0"]);
 K = double(param["K"]);
 VACOVP = double(param["VACOVP"]);
 VACMAX = int(param["VACMAX"]);
 GoodEn = double(param["GoodEn"]);
 MaxSteps = int(param["MaxSteps"]);
 vacsymbol = ElemNum(std::string(param["VacSym"]));
 boundary = ElemNum(std::string(param["Boundary"]));
 RiskSteps = int(param["RiskSteps"]);
 RiskLimit = int(param["RiskLimit"]);
 min_coord = int(param["min_coord"]);
 max_coord = int(param["max_coord"]);
}

SearchFill::~SearchFill()
{
}

void SearchFill::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = searchfill                                               \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      The plugins is used to search atomic vacancies in an atomic structure    \n";
 std::cout << " using the method described by Davis et al. Computer Physics Communications    \n";
 std::cout << " 182, 1105-1110 (2011). This also have support for boundaries using a risk     \n";
 std::cout << " parameter around particular taged atoms.                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      R0            : Radius of the vaccum sphere to check the vacancy.        \n";
 std::cout << "                      Half the size of first neighbor distance.                \n";
 std::cout << "      K             : Parameter used by the internal montecarlo algorithm.     \n";
 std::cout << "                      Change this will modify the code performance.            \n";
 std::cout << "      VACOVP        : Maximum allowed overlap of the vaccum spheres.           \n";
 std::cout << "      VACMAX        : Maximum nmber of vacancies allowed. (Default 1E6).       \n";
 std::cout << "      VacSym        : Atomic symbol of to use in the vacancies. (Default V).   \n";
 std::cout << "      Boundary      : Atomic symbol of boundary atoms. If atoms are labeled as \n";
 std::cout << "                      boundary, then vacancies are not allowed close to it.    \n";
 std::cout << "      GoodEn        : If you know the overlap, this could be used as a criteria\n";
 std::cout << "                      to fast the code. (Default not used = -1)                \n";
 std::cout << "      MaxSteps      : Maximum number of montecarlo steps in the position of the\n";
 std::cout << "                      vacancies. (Default=750000)                              \n";
 std::cout << "      RiskSteps     : Maximum number of steps with high-risk (close to a       \n";
 std::cout << "                      boundary) allowed in the search. Default (1000).         \n";
 std::cout << "      min_coord     : Minimum vacancy coordination number allowed. Default 1.  \n";
 std::cout << "      max_coord     : Maximum vacancy coordination number allowed. Default 15. \n";
 std::cout << "      RiskLimit     : Maximum percentage of atoms to be labeled as risky atoms.\n";
 std::cout << "                      Deafult value is 80 percent                              \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
 std::cout << " Example          >>                                                           \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use searchfill                                                                \n";
 std::cout << "     VACOVP 1.4                                                                \n";
 std::cout << "     VACMAX 50000                                                              \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Apply the plugin :                                                           \n";
 std::cout << " property angdist start=0 each=1 end=100                                     \n\n";
 std::cout << "      The plugin is used to evaluate the angular distribution function of the  \n";
 std::cout << "      cell each one step between the step 0 and 100.                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"; 
}

void SearchFill::Apply(Configuration & con)
{
 lpmd::BasicParticleSet & part = con.Atoms();
 lpmd::BasicCell & cell = con.Cell();
 unsigned long int iN = part.Size(); //initial number of atoms.
 srand48((long int)(time(NULL)));
 double minov,abs_min=1.0e-4;
 long int nvac = 0;
 if (boundary!=0)
 {
  int risk = 0; //The vacancy position it's risky because it is close to a surface 'taged' atom. 0 = non risky.
  std::set<int> risk_id;
  risk_id.clear(); risk_id.insert(-1);
  //Identify all the atoms_id number with high risk.
  for (unsigned long int i=0;i<part.Size(); ++i)
  {
   if (part[i].Z()==boundary)
   {
    risk_id.insert(i);
   }
   else
   {
    lpmd::NeighborList & nlist = con.Neighbors(i, true, 2*R0);
    for (unsigned long int j=0; j < nlist.Size(); ++j)
    {
     const lpmd::AtomPair & nn =nlist[j];
     if(nn.j->Z() == boundary) risk_id.insert(i);
    }
   }
  }
  std::cout << "Number of atoms to avoid because of boundary or risk = " << risk_id.size() << '\n';
  while(nvac<VACMAX)
  {
   if(risk_id.size() > int(iN*RiskLimit/100))
   {
    std::cout << "***********************************************************" << '\n';
    std::cout << "The simulation was finished because the risky atoms exceed*" << '\n';
    std::cout << "the " << RiskLimit << " percent of the original atomic data.************" << '\n';
    std::cout << "***********************************************************" << '\n';
    break;
   }
   minov = 100.0;
   double to=0.0e0;
   Vector r1;
   risk = 0;
   //***************************************************************************************************************
   //Search a nice place to start(minimum overlap of an atom in the cell). Overlap must be major than abs_min.******
   //This is the initial position to start looking around.**********************************************************
   //***************************************************************************************************************
   int coord = 0;
   int nnan = 0;
   unsigned long int kmo = 0;
   std::cout << "=====================================================\n";
   std::cout << "-> Searching for a good place to start minimizing... \n";
   int oldperc = 0;
   if(drand48()<=0.25) abs_min = abs_min/10.0;
   if(drand48()<0.05)
   {
    for (unsigned long int i=0;i < iN;++i)
    {
     std::set<int>::iterator it;
     it = risk_id.find(i);
     if ( it == risk_id.end() )
     {
      to = 0.0e0;
      lpmd::NeighborList nlist;
      con.GetCellManager().BuildNeighborList(con, i, nlist, true, 2*R0);
      for (unsigned long int j=0; j<nlist.Size(); ++j)
      {
       lpmd::AtomPair nn = nlist[j];
       to += fover(sqrt(nn.r2)/(R0));
      }
      if ((to < minov && to > abs_min))
      {
       minov = to;
       kmo = i;
      }
      if ((GoodEn > 0.0e0) && (to < GoodEn)) break;    // test criterion if apply
     }
    }
   }
   else
   {
    srand(time(0));
    kmo = rand() % iN;
   }
   std::cout << "-> Minimum overlap on atom " << kmo << ", ov = " << minov << '\n';
   std::cout << "-> Position " << part[kmo].Position() << '\n';
   Vector r0 = part[kmo].Position();
   r1 = r0;
   //rellocate the abs_min. If we don't do that the new atom (vacancy) will have the minimum overlap now.
   //The actual total overlap is the minimum overlap.
   abs_min = minov; 
   to = minov;
   //********************************************************************************************************
   //If there is a big vaccum region some atoms could have 'to' close to zero.*******************************
   //In those cases, we will not pass thru the while sentence below, so we need to re-alocate the position **
   //before go inside the while. If we don't do this, we will add a vacancy overlapping the original atom****
   //with lower 'to'.****************************************************************************************
   //********************************************************************************************************
   if(to <= VACOVP)
   {
    double real_jump = 2.5*R0;
    double real_k = K;
    if ( to <= 1.0 )
    {
     real_jump = to*R0;
     real_k = to*K;
    }
    double dx = drand48()*real_jump, dy = drand48()*real_jump, dz = drand48()*real_jump;
    r1 = cell.FittedInside(r0 + Vector(dx,dy,dz));
    part.Append(Atom(ElemSym[vacsymbol],r1));
    con.Update();
    lpmd::NeighborList & nlist = con.Neighbors(part.Size()-1, true, 2*R0);
    coord = 0;
    for (unsigned long int j=0; j<nlist.Size(); ++j)
    {
     const lpmd::AtomPair & nn = nlist[j];
     if(nn.j->Z()==boundary) risk++;
     to += fover(sqrt(nn.r2)/(R0));
     coord++;
    }
    part.Delete(part.Size()-1);
    con.Update();
   }
   //Now we can really aprove this new position like a real vacancy and not overlap with an original atom.
   int iter = 0;
   bool accept;
   lpmd::Atom vce(ElemSym[vacsymbol],r1);
   oldperc = 0;
   int new_risk=0;
   int rs = 0; //RiskySteps
   //*********************************************************************************************************
   //*Start the vacancy search process.***********************************************************************
   //*********************************************************************************************************
   while ((to > VACOVP) && (iter < MaxSteps) && risk <=1)
   {
    double real_jump = 2.5*R0;
    double real_k = K;
    if ( to <= 1.0 )
    {
     real_jump = to*R0;
     real_k = to*K;
    }
    double dx = drand48()*real_jump, dy = drand48()*real_jump, dz = drand48()*real_jump;
    r1 = cell.FittedInside(r0 + Vector(dx,dy,dz));
    vce = Atom(ElemSym[vacsymbol],r1);
    part.Append(vce);
    con.Update();
    lpmd::NeighborList & nlist = con.Neighbors(part.Size()-1, true, 2*R0);
    double newto = 0.0e0;
    coord = 0;
    new_risk = 0;
    nnan = 0 ; //number of non-atomic neighbors (at least one atom of the original cell must be a neighbor).
    for (unsigned long int j=0; j<nlist.Size(); ++j)
    {
     const lpmd::AtomPair & nn = nlist[j];
     int nnz = nn.j->Z();
     if(nnz==boundary) {new_risk++; nnan++;}
     if(nnz==vacsymbol) {nnan++;}
     newto += fover(sqrt(nn.r2)/(R0));
     coord++;
    }
    if(risk-new_risk>=0)
    {
     double delta = newto - to;
     if (delta <= 0.0) accept = true;
     else
     {
      double prech = exp(real_k*delta)-1.0;
      if (drand48() <= prech) accept = false;
      else {accept = true;}
     }
     if (accept)
     {
      r0 = r1;
      to = newto;
     }
    }
    else
    {
     rs++;
    }
    part.Delete(part.Size()-1);
    con.Update();
    iter ++;
    int perc = int(floor(10.0*double(iter)/MaxSteps));
    if (perc != oldperc)
    {
     std::cerr << "#";
     oldperc = perc;
    }
    if(rs > RiskSteps) break;
   }
   //Let's see if we got a vacancy
   if ((to < VACOVP && risk-new_risk==0))
   {
    if((coord >= min_coord && coord <= max_coord) && nnan!=coord)
    {
     std::cout << "-> Found a Vacancy! (ov = " << to << ", coord = " << coord << " ) = " << vce.Position() << '\n';
     part.Append(vce);
     con.Update();
     nvac++;
     std::cout << "Vacancies up to now   : " << nvac << '\n';
     std::cout << "Total number of Atoms : " << part.Size() << '\n';
    }
    else if(nnan==coord)
    {
     risk_id.insert(kmo);
     std::cout << "-> New risky atom list size = " << risk_id.size() << '\n';
     std::cout << "-> Risk atom ID = " << kmo << '\n';
     continue;
    }
    else continue;
   }
   else if(risk-new_risk<0 || risk-new_risk>0)
   {
    risk_id.insert(kmo);
    std::cout << "-> New risky atom list size = " << risk_id.size() << '\n';
    std::cout << "-> Risk atom ID = " << kmo << '\n';
    continue;
   }
   else 
    break;
  }
 }
 //No boundary definition ... Faster?
 else
 {
  int rc = 0;
  int MaxSearch = int(iN*RiskLimit/100);
  int oldperc = 0;
  while(nvac<VACMAX)
  {
   minov = 100.0;
   double to=0.0e0;
   Vector r1;
   //***************************************************************************************************************
   //Search a nice place to start(minimum overlap of an atom in the cell). Overlap must be major than abs_min.******
   //This is the initial position to start looking around.**********************************************************
   //***************************************************************************************************************
   int coord = 0;
   int nnan = 0;
   unsigned long int kmo = 0;
//   std::cout << "=====================================================\n";
//   std::cout << "-> Searching for a good place to start minimizing... \n";
   int oldperc1=0;
   int perc = int(floor(50.0*double(rc)/double(MaxSearch/4.0)));
   if (perc != oldperc)
   {
    std::cerr << "#";
    oldperc = perc;
   }
   if(drand48()<=0.25) abs_min = abs_min/10.0;
   if(drand48() > 0.05)
   {
    for (unsigned long int i=0;i < iN;++i)
    {
     to = 0.0e0;
     lpmd::NeighborList nlist;
     con.GetCellManager().BuildNeighborList(con, i, nlist, true, 2*R0);
     for (unsigned long int j=0; j<nlist.Size(); ++j) //only original atoms.
     {
      lpmd::AtomPair nn = nlist[j];
      to += fover(sqrt(nn.r2)/(R0));
     }
     if ((to < minov && to > abs_min))
     {
      minov = to;
      kmo = i;
     }
     if ((GoodEn > 0.0e0) && (to < GoodEn)) break;    // test criterion if apply
    }
   }
   else
   {
    srand(time(0));
    kmo = rand() % iN;
   }
   rc ++;
   Vector r0 = part[kmo].Position();
   r1 = r0;
   //rellocate the abs_min. If we don't do that the new atom (vacancy) will have the minimum overlap now.
   //The actual total overlap is the minimum overlap.
   abs_min = minov; 
   to = minov;
   //********************************************************************************************************
   //If there is a big vaccum region some atoms could have 'to' close to zero.*******************************
   //In those cases, we will not pass thru the while sentence below, so we need to re-alocate the position **
   //before go inside the while. If we don't do this, we will add a vacancy overlapping the original atom****
   //with lower 'to'.****************************************************************************************
   //********************************************************************************************************
   if(to <= VACOVP)
   {
    double real_jump = 2.5*R0;
    double real_k = K;
    if ( to <= 1.0 )
    {
     real_jump = to*R0;
     real_k = to*K;
    }
    double dx = drand48()*real_jump, dy = drand48()*real_jump, dz = drand48()*real_jump;
    r1 = cell.FittedInside(r0 + Vector(dx,dy,dz));
    part.Append(Atom(ElemSym[vacsymbol],r1));
    con.Update();
    lpmd::NeighborList & nlist = con.Neighbors(part.Size()-1, true, 2*R0);
    coord = 0;
    for (unsigned long int j=0; j<nlist.Size(); ++j)
    {
     const lpmd::AtomPair & nn = nlist[j];
     to += fover(sqrt(nn.r2)/(R0));
     coord++;
    }
    part.Delete(part.Size()-1);
    con.Update();
   }
   //Now we can really aprove this new position like a real vacancy and not overlap with an original atom.
   int iter = 0;
   bool accept;
   lpmd::Atom vce(ElemSym[vacsymbol],r1);
   oldperc = 0;
   //*********************************************************************************************************
   //*Start the vacancy search process.***********************************************************************
   //*********************************************************************************************************
   while ((to > VACOVP) && (iter < MaxSteps))
   {
    double real_jump = 2.5*R0;
    double real_k = K;
    if ( to <= 1.0 )
    {
     real_jump = to*R0;
     real_k = to*K;
    }
    double dx = drand48()*real_jump, dy = drand48()*real_jump, dz = drand48()*real_jump;
    r1 = cell.FittedInside(r0 + Vector(dx,dy,dz));
    vce = Atom(ElemSym[vacsymbol],r1);
    part.Append(vce);
    con.Update();
    lpmd::NeighborList & nlist = con.Neighbors(part.Size()-1, true, 2*R0);
    double newto = 0.0e0;
    coord = 0;
    nnan = 0 ; //number of non-atomic neighbors (at least one atom of the original cell must be a neighbor).
    for (unsigned long int j=0; j<nlist.Size(); ++j)
    {
     const lpmd::AtomPair & nn = nlist[j];
     if(nn.j->Z()==vacsymbol) {nnan++;}
     newto += fover(sqrt(nn.r2)/(R0));
     coord++;
    }
    double delta = newto - to;
    if (delta <= 0.0) accept = true;
    else
    {
     double prech = exp(real_k*delta)-1.0;
     if (drand48() <= prech) accept = false;
     else {accept = true;}
    }
    if (accept)
    {
     r0 = r1;
     to = newto;
    }
    part.Delete(part.Size()-1);
    con.Update();
    iter ++;
    int perc1 = int(floor(50.0*double(iter)/MaxSteps));
    if (perc1 != oldperc1)
    {
     std::cerr << "*";
     oldperc1 = perc1;
    }
   }
   //Let's see if we got a vacancy
   if ((to < VACOVP))
   {
    if((coord >= min_coord && coord <= max_coord) && nnan!=coord)
    {
     rc = 0;
     oldperc = 0;
     std::cerr << '\n';
     std::cout << "-> Minimum overlap on atom " << kmo << ", ov = " << minov << '\n';
     std::cout << "-> Position " << part[kmo].Position() << '\n';
     std::cout << "-> Found a Vacancy! (ov = " << to << ", coord = " << coord << " ) = " << vce.Position() << '\n';
     part.Append(vce);
     con.Update();
     nvac++;
     std::cout << "Vacancies up to now   : " << nvac << '\n';
     std::cout << "Total number of Atoms : " << part.Size() << '\n';
    }
    if(rc > int(iN*RiskLimit/100))
    {
     std::cout << "***********************************************************" << '\n';
     std::cout << "The simulation was finished because the good place to start try exceed*" << '\n';
     std::cout << "the " << RiskLimit << " percent of the original atomic data.************" << '\n';
     std::cout << "***********************************************************" << '\n';
     break;
    }
    else
    {
     continue;
    }
   }
   else 
    break;
  }
 }
}

void SearchFill::Apply(Simulation & md)
{
 Configuration & sc = md;
 Apply(sc);
}
// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new SearchFill(args); }
void destroy(Plugin * m) { delete m; }

