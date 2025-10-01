//
//
//

#include "msd.h"
#include <lpmd/simulationhistory.h>
#include <lpmd/storedconfiguration.h>

using namespace lpmd;

MSD::MSD(std::string args): Plugin("msd", "2.0")
{
 ParamList & params = (*this);

 DefineKeyword("rcutmin");
 DefineKeyword("rcutmax");
 DefineKeyword("zerocm", "false");
 ProcessArguments(args);
 //
 rcutmin = double(params["rcutmin"]);
 rcutmax = double(params["rcutmax"]);
 zerocm = bool(params["zerocm"] == "true");
}

void MSD::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = msd                                                      \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to calculate de mean square displacement (msd) of    \n";
 std::cout << "      the atoms. For more information, see S. Davis et al. Phys. Rev. B 84,    \n";
 std::cout << "      064102 (2011).                                                           \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      rcutmin       : Sets the minimum cutoff to calculate the mobility curve. \n";
 std::cout << "      rcutmax       : Sets the maximum cutoff to calculate the mobility curve. \n";
 std::cout << "      zerocm        : ...                                                      \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin:                                                          \n";
 std::cout << " use msd                                                                       \n";
 std::cout << "  rcutmin 4                                                                    \n";
 std::cout << "  rcutmax 20                                                                   \n";
 std::cout << "  zerocm true                                                                  \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin                                                          \n";
 std::cout << " property msd start=0 end=-1 each=5                                          \n\n";
 std::cout << "      The plugin is used to calculate the mean square displacement             \n";
 std::cout << "      of the atoms.                                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void MSD::ZeroCM(Configuration & conf)
{
 Vector cm(0.0, 0.0, 0.0);
 Vector newcm(0.0, 0.0, 0.0);
 BasicCell & cell = conf.Cell();
 BasicParticleSet & atoms = conf.Atoms();
 const Vector boxcenter = cell.Cartesian(Vector(0.5, 0.5, 0.5));
 double m = 0.0;
 for (long i=0;i<atoms.Size();++i)
 {
  cm += (atoms[i].Mass()*atoms[i].Position());
  m += atoms[i].Mass();
 }
 cm = cm * (1.0/m);
 for (long i=0;i<atoms.Size();++i)
 {
  atoms[i].Position() = atoms[i].Position() - cm + boxcenter;
  newcm += (atoms[i].Mass()*atoms[i].Position());
 }
 newcm = newcm * (1.0/m);
 assert ((newcm-boxcenter).Module() < 1.0E-06);
}

void MSD::Evaluate(ConfigurationSet & hist, Potential & pot)
{
 assert(&pot != 0);//icc 869
 long int N = hist.Size(); // number of configurations, not number of atoms
 DebugStream() << "-> Computing MSD over " << N << " configurations\n";
 long int nat = hist[0].Atoms().Size(); // number of atoms
 DebugStream() << "-> First configuration has " << nat << " atoms\n";
 const Array<int> & elements = hist[0].Atoms().Elements();
 int nsp = elements.Size();
 int * natsp = new int[nsp];
 double ** msd = new double*[(int)(N-1)/2];
 double ** J0 = new double*[(int)(N-1)/2];
 double ** J1 = new double*[(int)(N-1)/2];
 double ** Jr = new double*[(int)(N-1)/2];
 for (int i=0;i<(int)(N-1)/2;i++) 
 {
  msd[i] = new double[nsp];
  J0[i] = new double[nsp];
  J1[i] = new double[nsp];
  Jr[i] = new double[nsp];
  for (int j=0;j<nsp;j++) { msd[i][j]=J0[i][j]=J1[i][j]=Jr[i][j]=0.0e0; }
 }

 //
 // Undo periodicity 
 //
 if (zerocm) ZeroCM(hist[0]);
 StoredConfiguration scratch(hist[0]);
 Vector ** noperiodic = new Vector*[N];
 for (int t=0;t<N;++t) noperiodic[t] = new Vector[nat];
 BasicParticleSet & scratch_atoms = scratch.Atoms();
 BasicCell & cell = scratch.Cell();
 for (long int i=0;i<nat;++i)
 {
  noperiodic[0][i] = scratch_atoms[i].Position();
 }
 for (int t=1;t<N;++t)
 {
  if (zerocm) ZeroCM(hist[t]);
  DebugStream() << "-> MSD: Undoing periodicity, configuration " << t << '\n';
  for (long int i=0;i<nat;++i)
  {
   const Vector & v0 = scratch_atoms[0].Position() = hist[t-1].Atoms()[i].Position();
   const Vector & v1 = scratch_atoms[1].Position() = hist[t].Atoms()[i].Position();
   noperiodic[t][i] = noperiodic[t-1][i] + cell.Displacement(v0, v1);
  }
 }

 //
 //
 //

 int s=0;
 for(int e1=0;e1<nsp;e1++)
 {		 	
  int ne = 0;
  for (long int i=0;i<nat;i++) 
  {
   if (hist[0].Atoms()[i].Z() == elements[e1]) ne++;
  }
  natsp[e1] = ne;
  for (int t0=0;t0<(int)(N-1)/2;t0++) // loop sobre todos los origenes
  {
   DebugStream() << "-> MSD: Processing time origin " << t0 << " of " << (int)(N-1)/2 << '\n';
   for (int t=0;t<(int)(N-1)/2;t++) // loop sobre la separacion en tiempo
   {
    for (long int i=0;i<nat;i++)  // loop sobre todos los atomos
    {
     if (hist[t0].Atoms()[i].Z() == elements[e1])
     {
      double rr2 = (noperiodic[t0+t][i]-noperiodic[t0][i]).SquareModule();
      msd[t][e1] += (rr2);
      if (rr2 < rcutmin*rcutmin) J0[t][e1]++; 
      else if (rr2 > rcutmax*rcutmax) Jr[t][e1]++;
      else J1[t][e1]++;
     }
    }
   }
  }
  s++;
 }
 for (int i=0;i<N;++i) delete [] noperiodic[i];
 delete [] noperiodic;

 //
 // Output MSD
 //
 ParamList & params = (*this);
 Matrix & m = CurrentValue();
 bool jmode = (params.Defined("rcutmin") && (fabs(double(params["rcutmin"])) >= 0.1));
 if (jmode) m = Matrix(4*nsp+1, (int)(N-1)/2);
 else m = Matrix(nsp+1, (int)(N-1)/2);
 m.SetLabel(0, "time");

 int k = 1;
 for (int q=0;q<elements.Size();++q)
 { 
  const std::string spec = ElemSym[elements[q]];
  m.SetLabel(k, "MSD-"+spec);
  if (jmode)
  {
   m.SetLabel(k+1, "J0-"+spec);
   m.SetLabel(k+2, "J1-"+spec);
   m.SetLabel(k+3, "Jr-"+spec);
   k += 4;
  }
  else k++;
 }
 for(int i=0;i<(int)(N-1)/2;++i)
 {
  m.Set(0, i, i);
  int j = 0;
  for(int e1=0;e1<nsp;e1++)
  {
   m.Set(j+1, i, msd[i][j]/double(natsp[e1]*(N-1)/2));
   if (jmode)
   {
    double integ = J0[i][j]+J1[i][j]+Jr[i][j];
    m.Set(j+2, i, J0[i][j]/integ);
    m.Set(j+3, i, J1[i][j]/integ);
    m.Set(j+4, i, Jr[i][j]/integ);
    j += 4;
   }
   else j++;
  }
 }

 for (int i=0;i<(int)(N-1)/2;i++) 
 {
  delete [] msd[i];
  delete [] J0[i];
  delete [] J1[i];
  delete [] Jr[i];
 }
 std::cerr << "DEBUG " << elements[0] << " " << elements[1] << "\n";
 std::cerr << "DEBUG " << natsp[0] << " " << natsp[1] << "\n";
 delete [] msd;
 delete [] J0;
 delete [] J1;
 delete [] Jr;
 delete [] natsp;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new MSD(args); }
void destroy(Plugin * m) { delete m; }

