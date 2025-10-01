//
//
//

#ifndef __CNA_H__
#define __CNA_H__

#include <lpmd/matrix.h>
#include <lpmd/storedvalue.h>
#include <lpmd/property.h>
#include <lpmd/configuration.h>
#include <lpmd/plugin.h>
#include <lpmd/atom.h>

#include <map>

class IndexTrio
{
 public:
  unsigned int j, k, l;

  IndexTrio(unsigned int j0, unsigned int k0, unsigned int l0): j(j0), k(k0), l(l0) { }
};

inline bool operator<(const IndexTrio & t1, const IndexTrio & t2)
{
 if (t1.j < t2.j) return true;
 if (t1.j > t2.j) return false;
 if (t1.k < t2.k) return true;
 if (t1.k > t2.k) return false;
 if (t1.l < t2.l) return true;
 return false;
}

class CommonNeighborAnalysis: public lpmd::StoredValue<lpmd::Matrix>, public lpmd::InstantProperty, public lpmd::Plugin
{
 public:
  //Metodos Generales
  CommonNeighborAnalysis(std::string args);
  ~CommonNeighborAnalysis();
  void ShowHelp() const;

  void Evaluate(lpmd::Configuration & config, lpmd::Potential & pot);

 private:
    int mode, spc1, spc2;
    double rcut;
    std::map<IndexTrio, int> refmap;
    std::string filterout;
};

#endif

