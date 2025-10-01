//
//
//

#include "lcbinary.h"

using namespace lpmd;

LCBinary::LCBinary(std::string args): Plugin("lcbinary", "1.0")
{ 
 ParamList & params = (*this);
 DefineKeyword("cutoff", "7.0");
 DefineKeyword("nx", "0");
 DefineKeyword("ny", "0");
 DefineKeyword("nz", "0");
 DefineKeyword("mode", "noauto");
 // 
 mode = false;
 ProcessArguments(args);
 cutoff = double(params["cutoff"]);
 nx = int(params["nx"]);
 ny = int(params["ny"]);
 nz = int(params["nz"]);
 if (nx==0 || ny==0 || nz==0) mode=true;
 else if((nx>=1 && ny>=1) && nz>=1) mode=false;
 if (params["mode"]=="auto" || params["mode"]=="AUTO") mode=true;
 // 
 //
 //
 atomlist = 0;
 subcell = 0;
 nwin = nfail = 0.0;
}

LCBinary::~LCBinary() 
{ 
 if (nwin+nfail > 0.0) DebugStream() << "-> lcbinary efficiency: " << 100.0*nwin/(nwin+nfail) << "%\n";
 delete [] atomlist;
 delete [] subcell;
}

void LCBinary::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = lcbinary                                                 \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin corresponds to a modification of the linked-cell method for  \n";
 std::cout << "      making neighbors lists. This method was developed by our group and it is \n";
 std::cout << "      one of the available cell managers. The difference between linked-cell   \n";
 std::cout << "      and lcbinary is that subcells are chosen such that each one of them can  \n";
 std::cout << "      accommodate at most one atom. In this way atoms lists can be avoided,    \n";
 std::cout << "      just a binary value is stored for each subcell (occupied/empty).         \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      cutoff        : Sets the cutoff radius for the evaluation.               \n";
 std::cout << "      nx            : Sets the number of subdivisions in the X axis.           \n";
 std::cout << "      ny            : Sets the number of subdivisions in the Y axis.           \n";
 std::cout << "      nz            : Sets the number of subdivisions in the Z axis.           \n";
 std::cout << "      mode          : Sets manual (default) or automatic selection of the      \n";
 std::cout << "                      values of nx, ny and nz (auto / noauto).                 \n";
 std::cout << "                      auto: values are chosen automatically.                   \n";
 std::cout << "                      noauto: values must be entered by the user.              \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " use lcbinary                                                                  \n";
 std::cout << "     cutoff 8.5                                                                \n";
 std::cout << "     nx 15                                                                     \n";
 std::cout << "     ny 15                                                                     \n";
 std::cout << "     nz 15                                                                     \n";
 std::cout << "     mode noauto                                                               \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " #Applying the plugin :                                                        \n";
 std::cout << " cellmanager lcbinary                                                        \n\n";
 std::cout << "      The plugin is used to select the lcbinary method for making the lists    \n";
 std::cout << "      of atoms' neighbors.                                                     \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void LCBinary::Reset() { }

void LCBinary::UpdateCell(Configuration & conf) 
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 if (mode == true)
 {
  double minx = cell[0].Module();
  double miny = cell[1].Module();
  double minz = cell[2].Module();
  double fnn = cutoff/5.0;
  int n = 1;
  while(true)
  {
   double previo = fabs(minx/n - fnn);
   n++;
   double actual = fabs(minx/n-fnn);
   if (actual<1E-3) break;
   else if (actual>previo) {n--;break;}
   else continue;
  }
  if ((n%2)!=0) n++;
  nx = n;
  DebugStream() << "-> Using nx = " << nx << " subdivision scheme."<<'\n';
  n=1;
  while(true)
  {
   double previo = fabs(miny/n - fnn);
   n++;
   double actual = fabs(miny/n - fnn);
   if(actual<1E-3) break;
   else if (actual>previo) {n--;break;}
   else continue;
  }
  if ((n%2)!=0) n++;
  ny = n;
  DebugStream() << "-> Using ny = " << ny << " subidvision scheme."<<'\n';
  n=1;
  while(true)
  {
   double previo = fabs(minz/n - fnn);
   n++;
   double actual = fabs(minz/n - fnn);
   if(actual<1E-3) break;
   else if (actual>previo) {n--;break;}
   else continue;
  }
  if ((n%2)!=0) n++;
  nz = n;
  DebugStream() << "-> Using nz = " << nz << " subdivision scheme." << '\n';
  n=1;
  mode=false;
 }
 //
 //
 //
 if (subcell == 0)
 {
  double d = cell[0].Module()/double(nx);
  if (cell[1].Module()/double(ny) < d) d = cell[1].Module()/double(ny);
  if (cell[2].Module()/double(nz) < d) d = cell[2].Module()/double(nz);
  int side = int(ceil((cutoff/d)-0.5));
  cells_inside = (2*side+1)*(2*side+1)*(2*side+1);
  DebugStream() << "-> Using " << cells_inside << " neighboring subcells\n";
  subcell = new int[(nx*ny*nz)*cells_inside];
  int z[3];
  for (int k=0;k<nz;++k)
   for (int j=0;j<ny;++j)
    for (int i=0;i<nx;++i)
    {
     int r = 0;
     long q = k*(nx*ny)+j*nx+i;
     for (int dz=-side;dz<=side;++dz)
      for (int dy=-side;dy<=side;++dy)
       for (int dx=-side;dx<=side;++dx)
       {
        z[0] = i+dx;
        z[1] = j+dy;
        z[2] = k+dz;
        if (z[0] < 0) z[0] += nx;
        else if (z[0] > nx-1) z[0] -= nx;
        if (z[1] < 0) z[1] += ny;
        else if (z[1] > ny-1) z[1] -= ny;
        if (z[2] < 0) z[2] += nz;
        else if (z[2] > nz-1) z[2] -= nz;
        long p = z[2]*(nx*ny)+z[1]*nx+z[0];
        subcell[q*cells_inside+(r++)] = p;
       }
    }
 }

 if (atomlist == 0) atomlist = new long[nx*ny*nz];
 //
 for (long i=0;i<(nx*ny*nz);++i) atomlist[i] = -1;
 bool update=false;
 long r=0;
#ifdef _OPENMP
#pragma omp parallel for private ( r )
#endif
 for (r=0;r<atoms.Size();++r)
 {
  const Vector fpos = cell.Fractional(atoms[r].Position());
  //
  int i = int(floor(nx*fpos[0]));
  int j = int(floor(ny*fpos[1]));
  int k = int(floor(nz*fpos[2]));
  //if (i < 0) i += nx;
  if (i > nx-1) i -= nx;
  //if (j < 0) j += ny;
  if (j > ny-1) j -= ny;
  //if (k < 0) k += nz;
  if (k > nz-1) k -= nz;
  //
  long q = k*(nx*ny)+j*nx+i;
  //assert(atomlist[q] == -1);
  if(atomlist[q] != -1)   
  {
   update=true;
  }
  atomlist[q] = r;
 }
 if (update==true) 
 {  
  DebugStream() << "-> Update nx-ny-nz to = " << nx <<"-"<<ny<<"-"<<nz<<'\n';
  nx=nx+2;
  ny=ny+2;
  nz=nz+2;
  if (subcell != 0) { delete [] subcell; subcell = 0; }
  if (atomlist !=0 ) { delete [] atomlist; atomlist = 0; }
  nwin=nfail=0;  
  UpdateCell(conf);
 }
}

void LCBinary::UpdateAtom(Configuration & conf, long r)
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const Vector fpos = cell.Fractional(atoms[r].Position());
 //
 int i = int(floor(nx*fpos[0]));
 int j = int(floor(ny*fpos[1]));
 int k = int(floor(nz*fpos[2]));
 //if (i < 0) i += nx;
 if (i > nx-1) i -= nx;
 //if (j < 0) j += ny;
 if (j > ny-1) j -= ny;
 //if (k < 0) k += nz;
 if (k > nz-1) k -= nz;
 //
 long q = k*(nx*ny)+j*nx+i;
 //assert(atomlist[q] == -1);
 atomlist[q] = r;
}

void LCBinary::BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double rcut)
{ 
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 const Vector fpos = cell.Fractional(atoms[i].Position());
 //
 int p = int(floor(nx*fpos[0]));
 int q = int(floor(ny*fpos[1]));
 int r = int(floor(nz*fpos[2]));
 //if (p < 0) p += nx;
 if (p > nx-1) p -= nx;
 //if (q < 0) q += ny;
 if (q > ny-1) q -= ny;
 //if (r < 0) r += nz;
 if (r > nz-1) r -= nz;
 long cind = r*(nx*ny)+q*nx+p;
 //
 AtomPair nn;
 nlist.Clear();
 nn.i = &atoms[i];
 nn.i_index = i;
 int * c=0;long z=0;
 c = &(subcell[cind*cells_inside]);
 for (int s=0;s<cells_inside;++s)
 {
  z = atomlist[*(c++)];
  if ((z < 0) || (z == i)) continue;
  if ((z > i) && (full == false)) continue;
  nn.j = &atoms[z];
  nn.rij = cell.Displacement(nn.i->Position(), nn.j->Position());
  nn.r2 = nn.rij.SquareModule();
  nn.j_index = z ;
  if (nn.r2 < rcut*rcut) { nlist.Append(nn); nwin += 1.0; }
  else { nfail += 1.0; }
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new LCBinary(args); }
void destroy(Plugin * m) { delete m; }

