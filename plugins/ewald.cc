//
//
//

#include "ewald.h"

#include <lpmd/session.h>
#include <lpmd/configuration.h>
#include <lpmd/timer.h>

using namespace lpmd;

Ewald::Ewald(std::string args): Plugin("ewald","2.0") 
{ 
 ParamList & params = (*this);
 DefineKeyword("kmax", "7");
 DefineKeyword("surfacedipole", "false");
// DefineKeyword("alpha");
// DefineKeyword("etol");
 ProcessArguments(args); 
// alpha = double(params["alpha"]);
// etol = double(params["etol"]);
 kmax = int(params["kmax"]);
 surfdip = bool(params["surfacedipole"]);
 kpoints = NULL;
 kfac = NULL;
}

Ewald::~Ewald() { delete kpoints; delete kfac;}

void Ewald::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = ewald                                                    \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin activates the electrostatic potential between the atomic     \n";
 std::cout << "      species using the ewald method.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      This plugin does not need options, it is activated when the user applies \n";
 std::cout << "      the plugin. However, it is important to set the atomic charges in the    \n";
 std::cout << "      control file, for example, 'charge 1.9 O #The units are in eV.' sets a   \n";
 std::cout << "      charge of 1.9 eV to the oxigen (O) atoms.                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use ewald                                                                     \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " potential ewald                                                             \n\n";
 std::cout << "      This instruction activates the ewald potential between all atomic        \n";
 std::cout << "      species. Note: you can set the charge to 0 when you don't want to disable\n";
 std::cout << "      a specific interaction.                                                  \n"; 
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void Ewald::BuildKPointMesh(Configuration & conf)
{
 if (kfac != NULL) delete [] kfac;
 BasicCell & cell = conf.Cell();
 BasicParticleSet & atoms = conf.Atoms();
 alpha = sqrt(M_PI)*powl(5.5*atoms.Size()/(cell.Volume()*cell.Volume()),0.166666667);
 rcut = sqrt(-log(0.001))/alpha;
 kcut = 2.0*alpha*sqrt(-log(0.001));
 Vector R[3], K[3];  // real and reciprocal vectors
 for (int q=0;q<3;++q) R[q] = cell[q];
 K[0] = (2*M_PI/(Dot(R[0],Cross(R[1],R[2]))))*Cross(R[1],R[2]);
 K[1] = (2*M_PI/(Dot(R[1],Cross(R[2],R[0]))))*Cross(R[2],R[0]);
 K[2] = (2*M_PI/(Dot(R[2],Cross(R[0],R[1]))))*Cross(R[0],R[1]);
 kpoints = new std::vector<Vector>;
 int nrep = kmax;
 for (int pp=0;pp<=nrep;++pp)
  for (int qq=(pp == 0 ? 0 : -nrep);qq<=nrep;++qq)
   for (int rr=(pp == 0 && qq == 0 ? 1 : -nrep);rr<=nrep;++rr)
   {
    Vector k = pp*K[0]+qq*K[1]+rr*K[2];
    if ((fabs(k.Module()) > 1.0E-10) && (k.Module() < kcut)) kpoints->push_back(k);
   }
 kfac = new double[kpoints->size()]; 
 for (unsigned int nk=0 ; nk < kpoints->size() ; ++nk)
 {
  Vector & tmp = (*kpoints)[nk];
  kfac[nk] = 4.0*M_PI*(1.0/tmp.SquareModule())*(1.0/cell.Volume())*exp(-tmp.SquareModule()/(4.0*alpha*alpha));
 }
 ecorr = EnergyConstantCorrection(conf); 
}

void Ewald::RealSpace(Configuration & conf, double & e)
{
 BasicParticleSet & atoms = conf.Atoms();
 double ep, e0;
 const double Q2a2EV = GlobalSession["q2a2ev"];
 const double Q2a2FORCE = GlobalSession["q2a2force"];

 e0 = 1.1/Q2a2EV;
 ep = 0.0;
#ifdef _OPENMP
#pragma omp parallel for reduction ( +: ep )
#endif 
 for (long int i=0;i<atoms.Size();++i)
 {
  NeighborList nlist;
  conf.GetCellManager().BuildNeighborList(conf, i, nlist, false, rcut);
  for (long int j=0;j<nlist.Size();++j)
  {
   AtomPair & nn = nlist[j];
   if (nn.r2 < rcut*rcut)
   {
    const double qi = atoms[i].Charge();
    const double qj = nn.j->Charge();
    double rmod = sqrt(nn.r2);
    Vector ff;
    ep += qi*qj*erfc(alpha*rmod)/rmod;
    ff = (nn.rij)*(1.0/rmod);
    ff = ff*qi*qj*(erfc(alpha*rmod)/rmod + 2.0*(alpha/sqrt(M_PI))*exp(-alpha*alpha*rmod*rmod));
    ff = ff*(-1.0/rmod);
    atoms[i].Acceleration() += ff*(Q2a2FORCE/atoms[i].Mass());
    nn.j->Acceleration() -= ff*(Q2a2FORCE/nn.j->Mass());
   }
  }
 }
 e0 = ep;
 e = ep*Q2a2EV;
}

void Ewald::ReciprocalSpace(Configuration & conf, double & e)
{
 BasicParticleSet & atoms = conf.Atoms();
 //BasicCell & cell = conf.Cell();
 const double Q2a2EV = GlobalSession["q2a2ev"];
 const double Q2a2FORCE = GlobalSession["q2a2force"];
 double ep = 0.0;
 if (kpoints == NULL) BuildKPointMesh(conf);
#ifdef _OPENMP
#pragma omp parallel for
#endif
 for (unsigned int nk=0;nk<kpoints->size();++nk)
 {
  const Vector & k = (*kpoints)[nk];
  double sumqcos = 0.0, sumqsin = 0.0;
  double *dkr = new double[atoms.Size()];
  for (long int i=0;i<atoms.Size();++i)
  {
   dkr[i] = Dot(k, atoms[i].Position());
   double qi = atoms[i].Charge();
   sumqcos += qi*cos(dkr[i]);
   sumqsin += qi*sin(dkr[i]);
  }
  for (long int i=0;i<atoms.Size();++i)
  {
   const double qi = atoms[i].Charge();
   atoms[i].Acceleration() = atoms[i].Acceleration() + (k*2.0*qi*kfac[nk]*(sin(dkr[i])*sumqcos-cos(dkr[i])*sumqsin))*Q2a2FORCE/atoms[i].Mass();
  }
  ep += kfac[nk]*(sumqcos*sumqcos+sumqsin*sumqsin);
  delete [] dkr;
 }
 e = ep*Q2a2EV;
}

void Ewald::SurfaceDipole(Configuration & conf, Vector * forces, double & e)
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const double Q2a2EV = GlobalSession["q2a2ev"];
 const double Q2a2FORCE = GlobalSession["q2a2force"];
 e = 0.0;
 Vector v(0.0, 0.0, 0.0), sf(0.0, 0.0, 0.0);
 for (long int i=0;i<atoms.Size();++i)
 {
  forces[i][0] = 0.0e0;
  forces[i][1] = 0.0e0;
  forces[i][2] = 0.0e0;
  sf = sf + atoms[i].Charge()*atoms[i].Position();
 }
 for (long int i=0;i<atoms.Size();++i)
 {
  v = v + atoms[i].Charge()*atoms[i].Position();
  forces[i] = forces[i] - 4.0*M_PI/(6.0*cell.Volume())*atoms[i].Charge()*sf;
 }
 for (long int i=0;i<atoms.Size();++i)
  atoms[i].Acceleration() = atoms[i].Acceleration()+forces[i]*Q2a2FORCE; 
 e = 4.0*M_PI*v.SquareModule()/(6.0*cell.Volume())*Q2a2EV;
}

double Ewald::EnergyConstantCorrection(Configuration & conf)
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const double Q2a2EV = GlobalSession["q2a2ev"];
 double e = 0.0;
 for (long int i=0;i<atoms.Size();++i) e += pow(atoms[i].Charge(), 2.0);
 e *= -alpha*(1.0/sqrt(M_PI));
 double totch = 0.0;
 for (long int i=0;i<atoms.Size();++i) totch += atoms[i].Charge();
 e -= pow(totch, 2.0)*M_PI/(4.0*cell.Volume()*alpha*alpha);
 return e*Q2a2EV;
}

double Ewald::energy(Configuration & conf) 
{ 
 if (conf.HaveAny(Tag("pe"))) return double(Parameter(conf.GetTag(conf, Tag("pe"))));
 throw PluginError("ewald", "This shouldn\'t happen");
}

double AtomEnergy(lpmd::Configuration & conf, long i)
{
 ShowWarning("ewald", "Potential::AtomEnergy not defined (yet) for ewald");
 return 0.0;
}

void Ewald::UpdateForces(Configuration & conf) 
{ 
 BasicParticleSet & atoms = conf.Atoms();
 double ereal=0.0, erecip=0.0, edip=0.0;
 Vector * forces = new Vector[atoms.Size()];
 RealSpace(conf, ereal);
 ReciprocalSpace(conf, erecip); 
 if (surfdip) {SurfaceDipole(conf, forces, edip);}
 double e = ereal+erecip+edip+ecorr;
 conf.SetTag(conf, Tag("pe"), e);
 delete [] forces;
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new Ewald(args); }
void destroy(Plugin * m) { delete m; }


