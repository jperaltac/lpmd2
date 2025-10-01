//
//
//

#ifndef __LPVISUAL_H__
#define __LPVISUAL_H__

#include "display.h"
#include <lpmd/plugin.h>
#include <lpmd/visualizer.h>
#include <lpmd/array.h>

using namespace lpmd;

class LPVisual:public lpmd::Visualizer, public lpmd::Plugin
{
 public:

  //Metodos Generales 
  LPVisual(std::string args);
  ~LPVisual();
  void ShowHelp() const;

  //Metodos Propios de modulo lpvisual
  void SpawnWindowThread(const lpmd::Simulation & sim);
  void Apply(const lpmd::Simulation & sim);

 private:
   bool active, applied_once;
   Display * dispman;
   void * shm;
   long shmsize, clrsize, datasize, ptsize, stsize, bgsize, gbgsize; // size of shared memory blocks
   Array<std::string> tags;
   Array<std::string> plot;
   Array<std::string> xrange;
   Array<std::string> yrange;
   Array<std::string> camerapos;
   Array<std::string> cameraobj;
   Array<std::string> cameraup;
};

#endif


