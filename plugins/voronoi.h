//
//
//

#ifndef __VORONOIMODULE_H__
#define __VORONOIMODULE_H__

#include <lpmd/cellgenerator.h>
#include <lpmd/plugin.h>
#include <lpmd/particleset.h>
#include <lpmd/configuration.h>
#include <lpmd/orthogonalcell.h>

using namespace lpmd;

class VoronoiGenerator: public lpmd::CellGenerator, public lpmd::Plugin
{
 public:
  //Metodos Generales
  VoronoiGenerator(std::string args); 
  virtual ~VoronoiGenerator();
  void ShowHelp() const;

  //Metodos propios del modulo voronoi
  void Generate(lpmd::Configuration & conf) const;

 private:
  std::string spc;     // which species (atomic number)
  std::string type;    // type of base cell
  std::string cts;     // how the centers of the grain are distributed
  double a;            // Lattice constant (size of the base cell of each grain)
  int grains;          // Number of cells (grains) in to put in the configuration
  double rperc;        // Percentual variation of minimal distance allowed.
};

#endif


