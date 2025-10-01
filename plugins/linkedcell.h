//
//
//

#ifndef __LINKEDCELL_MODULE_H
#define __LINKEDCELL_MODULE_H

#include <lpmd/configuration.h>
#include <lpmd/cellmanager.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class LinkedCell: public lpmd::CellManager, public lpmd::Plugin
{
 public:
   LinkedCell(std::string args);
   ~LinkedCell();
   void ShowHelp() const;
   
   void Reset();
   void UpdateCell(Configuration & conf);
   void UpdateAtom(Configuration & conf, long i);
   double Cutoff() const { return cutoff; }

   void BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double rcut);

  private:
    double cutoff;
    int nx, ny, nz;
    int * head, * tail, * subcell;
    long cells_inside, last_atoms_size;
    long * atomlist, *indexc;
    bool mode, warn_outside;
    NeighborList * full_list_half;
    NeighborList * full_list_full;
};

#endif

