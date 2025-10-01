//
//
//

#ifndef __LCBINARY_MODULE_H
#define __LCBINARY_MODULE_H

#include <lpmd/configuration.h>
#include <lpmd/cellmanager.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class LCBinary: public lpmd::CellManager, public lpmd::Plugin
{
 public:
   LCBinary(std::string args);
   ~LCBinary();
   void ShowHelp() const;

   void Reset();
   void UpdateCell(Configuration & conf);
   void UpdateAtom(Configuration & conf, long i);
   double Cutoff() const { return cutoff; }

   void BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double rcut);

  private:
    double cutoff;
    int nx, ny, nz;
    int * subcell;
    long cells_inside;
    double nwin, nfail;
    long * atomlist;
    bool mode;
};

#endif

