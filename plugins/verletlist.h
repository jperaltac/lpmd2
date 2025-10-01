//
//
//

#ifndef __VERLETLIST_MODULE_H
#define __VERLETLIST_MODULE_H

#include <lpmd/cellmanager.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class VerletListCellManager: public CellManager, public Plugin
{
 public:

   VerletListCellManager(std::string args);
   ~VerletListCellManager();

   void ShowHelp() const;
   void Reset();
   void UpdateCell(Configuration & conf);
   void UpdateAtom(Configuration & conf, long i);
   void UpdateVerletList(Configuration & conf);
   void BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double);
   double Cutoff() const;

 private:
   double rcut, extcut;
   long step_count, each, old_size, maxneighbors;
   double * head;
   long * nv;
   double ** vlist; 
};


#endif

