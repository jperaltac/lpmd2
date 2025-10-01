//
//
//

#ifndef __MINIMUMIMAGE_MODULE_H
#define __MINIMUMIMAGE_MODULE_H

#include <lpmd/cellmanager.h>
#include <lpmd/plugin.h>

using namespace lpmd;

class MinimumImageCellManager: public CellManager, public Plugin
{
 public:

   MinimumImageCellManager(std::string args);
   ~MinimumImageCellManager();
   void ShowHelp() const;

   void Show(std::ostream & os) const;

   void Reset();
   void UpdateCell(Configuration & conf);
   void UpdateAtom(Configuration & conf, long i);
   void BuildNeighborList(Configuration & conf, long i, NeighborList & nlist, bool full, double);
   double Cutoff() const;

 private:
   double rcut;
};


#endif


