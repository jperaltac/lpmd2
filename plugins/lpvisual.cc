//
//
//

#include "lpvisual.h"
#include <lpmd/simulation.h>
#include <lpmd/colorhandler.h>
#include <lpmd/session.h>

#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
// para compatibilidad con Mac OS X
#define MAP_ANONYMOUS MAP_ANON
#endif

using namespace lpmd;

int polig[][2] = { {0, 0}, {4,3}, {8,5}, {14, 7}, {18, 9}, {24, 12} };
/*
inline bool IsBonded(const SimulationCell & sc, long int i, long int j)
{
 const std::string spc1(sc[i].Symb());
 const std::string spc2(sc[j].Symb());
 SimulationCell & sch = const_cast<SimulationCell &>(sc);  // FIXME: ugly... i know... :(
 double r = sch.Distance(i, j);
 const std::string tag_direct = "bond-"+spc1+"-"+spc2;
 if (sc.MetaData().Defined(tag_direct) && sc.MetaData().GetDouble(tag_direct) >= r) return true;
 const std::string tag_reverse = "bond-"+spc2+"-"+spc1;
 if (sc.MetaData().Defined(tag_reverse) && sc.MetaData().GetDouble(tag_reverse) >= r) return true;
 return false;
}
*/

double string_to_double( const std::string& s )
{
 std::istringstream i(s);
 double x;
 if (!(i >> x))
  return 0;
 return x;
} 


LPVisual::LPVisual(std::string args): Plugin("lpvisual", "2.0"), active(false), dispman(0), shm(0), shmsize(0)
{
 ParamList & params = (*this);
 //
 DefineKeyword("start", "0");
 DefineKeyword("end", "-1");
 DefineKeyword("each", "1");
 DefineKeyword("width", "640");
 DefineKeyword("height", "480");
 DefineKeyword("radius", "0.5");
 DefineKeyword("quality", "2");
 DefineKeyword("azimuth", "0.0");
 DefineKeyword("zenith", "0.0");
 DefineKeyword("mark", "-1");
 DefineKeyword("paused", "false");
 DefineKeyword("autorotate", "false");
 DefineKeyword("background","black");
 DefineKeyword("graphbg","white");
 DefineKeyword("perspective","true");
 DefineKeyword("properties", "total-energy,temperature,pressure");
 DefineKeyword("plot","temperature");
 DefineKeyword("xrange");
 DefineKeyword("yrange");
 DefineKeyword("camerapos");
 DefineKeyword("cameraobj");
 DefineKeyword("cameraup");
 // hasta aqui los valores por omision
 applied_once = false;
 ProcessArguments(args);
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 tags = StringSplit(params["properties"], ',');
 plot = StringSplit(params["plot"], ',');
 xrange = StringSplit(params["xrange"], ',');
 yrange = StringSplit(params["yrange"], ',');
 camerapos = StringSplit(params["camerapos"], ',');
 cameraobj = StringSplit(params["cameraobj"], ',');
 cameraup = StringSplit(params["cameraup"], ',');
 if (tags.Size()>50 || plot.Size()>50)
  throw PluginError("lpvisual", "Only a maximum of 50 values can be shown simultaneously");
 if (xrange.Size()>3)
  throw PluginError("lpvisual","Number of 'xrange' parameters exceeded. See manual.");
 else if (params["xrange"]=="0,0" || params["xrange"]=="0,0,0")
  throw PluginError("lpvisual","Non-sense 'xrange'. See manual.");
 if (yrange.Size()>3)
  throw PluginError("lpvisual","Number of 'yrange' parameters exceeded. See manual.");
 else if (params["yrange"]=="0,0" || params["yrange"]=="0,0,0")
  throw PluginError("lpvisual","Non-sense 'yrange'. See manual.");
 if (camerapos.Size()>3)
  throw PluginError("lpvisual","Number of 'camerapos' parameters exceeded. See manual.");
 if (cameraobj.Size()>3)
  throw PluginError("lpvisual","Number of 'cameraobj' parameters exceeded. See manual.");
}

LPVisual::~LPVisual() 
{ 
 if (shm != 0)
 {
  SharedSegment * sseg = (SharedSegment *)(shm);
  for (int q=0;q<3;++q)  munmap((void *)(sseg->color[q]), clrsize);
  for (int q=0;q<50;++q) munmap((void *)(sseg->stname[q]), stsize);
  for (int q=0;q<50;++q) munmap((void *)(sseg->ptname[q]), ptsize);
  for (int q=0;q<50;++q) munmap((void *)(sseg->data[q]), datasize);
  for (int q=0;q<1;++q) munmap((void *)(sseg->bg), bgsize);
  for (int q=0;q<1;++q) munmap((void *)(sseg->gbg), gbgsize);
 }
 munmap(shm, shmsize);
 
 delete dispman; 
}

void LPVisual::ShowHelp() const
{
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      El modulo es utilizado para visualizar la celda de simulacion.           \n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      width         : Ancho de la ventana.                                     \n";
 std::cout << "      height        : Alto de la ventana.                                      \n";
 std::cout << "      radius        : Radio de los atomos.                                     \n";
 std::cout << "      quality       : Resolucion de los atomos (calidad).                      \n";
 std::cout << "      azimuth       : Angulo azimutal inicial (en coord. esfericas, phi).      \n";
 std::cout << "      zenith        : Angulo de cenit inicial (en coord. esfericas, theta).    \n";
 std::cout << "      paused        : Inicializa la simulacion en modo pausa.                  \n";
 std::cout << "      background    : Define el color de fondo de la ventana de simulacion.    \n";
 std::cout << "      graphbg       : Define el color de fondo de la ventana de graficos.      \n";
 std::cout << "      autorotate    : Inicia la simulacion con rotacion automatica.            \n";
 std::cout << "      perspective   : Define la perspectiva inicial (ortografica/perspectiva). \n";
 std::cout << "      properties    : Propiedades que se desean monitorear numericamente       \n";
 std::cout << "                      en la ventana secundaria de simulacion.                  \n";
 std::cout << "      plot          : Propiedades que se desean graficar instantaneamente      \n";
 std::cout << "                      a medida que la simulacion transcurre.                   \n";
 std::cout << "      xrange        : Rango del eje X (tiempo) en el graficador.               \n";
 std::cout << "      yrange        : Rango del eje Y (propiedades) en el graficador.          \n";
 std::cout << "      camerapos     : Posicion inicial de la camara en el espacio.             \n";
 std::cout << "      cameraobj     : Punto hacia donde la camara esta mirando (objetivo).     \n";
 std::cout << "      cameraup      : Indica que direccion es 'arriba'.                        \n";
 std::cout << '\n';
 std::cout << " Example                                                                       \n";
 std::cout << " Cargando el Modulo :                                                          \n";
 std::cout << " use lpvisual                                                                  \n";
 std::cout << "     width  640                                                                \n";
 std::cout << "     height 480                                                                \n";
 std::cout << "     radius 0.5                                                                \n";
 std::cout << "     quality 4                                                                 \n";
 std::cout << "     azimuth 0.0                                                               \n";
 std::cout << "     zenith -90.0                                                              \n";
 std::cout << "     background white                                                          \n";
 std::cout << "     autorotate true                                                           \n";
 std::cout << "     perspective false                                                         \n";
 std::cout << "     properties potential-energy,kinetic-energy                                \n";
 std::cout << "     plot total-energy,temperature                                             \n";
 std::cout << "     xrange 200,1000                                                           \n";
 std::cout << "     yrange -400,-100                                                          \n";
 std::cout << " enduse                                                                        \n";
 std::cout << " Llamando al modulo                                                            \n";
 std::cout << " visualize lpvisual start=0 each=50 end=1000                                 \n\n";
 std::cout << "    - Esto visualiza la simulacion cada 50 pasos (each) durante los primeros   \n";
 std::cout << "      1000 pasos (end) de simulacion, comenzando desde el paso inicial (start).\n";
 std::cout << "    - Los parametros 'width' y 'height' dan el ancho de la ventana deseados,   \n";
 std::cout << "      mientras que 'radius' y 'quality' dan el radio de los atomos y su        \n";
 std::cout << "      calidad (que tan cercanos a una esfera seran), respectivamente.          \n";
 std::cout << "    - Con azimuth (phi) y zenith (theta) podemos dar las coordenadas esfericas \n";
 std::cout << "      angulares que determinan el vector posicion inicial de la camara.        \n";
 std::cout << "    - Con 'background', elegimos el fondo de la pantalla blanco (white).       \n";
 std::cout << "      Fondos disponibles: white, gray, orange, black (default)                 \n";
 std::cout << "    - Con 'autorotate', iniciamos la simulacion en rotacion automatica.        \n";
 std::cout << "    - 'perspective' inicia una visualizacion ortorombica (perspective=false).  \n";
 std::cout << "    - 'properties' son las propiedades cuyo valor numerico sera                \n";
 std::cout << "      monitoreado en una ventana secundaria del visualizador, mientras que     \n";
 std::cout << "      'plot' seran las propiedades que se graficaran en otra ventana           \n";
 std::cout << "      independiente mientras transcurre la simulacion.                         \n";
 std::cout << "    - 'xrange' e 'yrange' elegimos los rangos de graficacion.                  \n";
 std::cout << "    - 'camerapos' es el lugar del espacio en el que la camara sera situada.    \n";
 std::cout << "    - 'cameraobj' es el lugar del espacio hacia el cual la camara mira,        \n";
 std::cout << "       en torno al cual hara las rotaciones.                                   \n";
 std::cout << "    - 'cameraup' indica que la direccion 'arriba'.                             \n";
}

void LPVisual::SpawnWindowThread(const lpmd::Simulation & sim)
{
 ParamList & params = (*this);
 const BasicParticleSet & atoms = sim.Atoms();
 const BasicCell & celda = sim.Cell();

 // Sets up shared memory communication
 shmsize = sizeof(SharedSegment)+3*sizeof(float)*atoms.Size();
 shm = mmap(NULL, shmsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 if (shm == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
 //
 // We build the shared segment and set the default values
 SharedSegment * sseg = (SharedSegment *)(shm);
 sseg->mdstep = 0;
 sseg->start = start;
 sseg->steps = (end==-1) ? 100000:end;
 sseg->each = each;
 sseg->natoms = 0;
 sseg->mustUpdate = false;
 sseg->updating = false;
 sseg->liveWIN1 = false;
 sseg->liveWIN2 = false;
 sseg->liveWIN3 = false;
 sseg->liveWIN4 = false;
 sseg->noprop=true;
 sseg->disp = NULL;
 sseg->WIN1 = 1;
 sseg->WIN2 = 2;
 sseg->WIN3 = 3;
 sseg->WIN4 = 4;
 sseg->width1 = 640; sseg->height1 = 480;
 sseg->width2 = 620; sseg->height2 = 720;
 sseg->width3 = 420; sseg->height3 = 300;
 sseg->width4 = 600; sseg->height4 = 400;
 sseg->xrange[0]=0; sseg->xrange[1]=1000, sseg->xrange[2]=1;
 sseg->yrange[0]=-5; sseg->yrange[1]=10, sseg->yrange[2]=1;
 sseg->camerapos[0]=M_PI; sseg->camerapos[1]=M_PI, sseg->camerapos[2]=M_PI;
 sseg->cameraobj[0]=M_PI; sseg->cameraobj[1]=M_PI, sseg->cameraobj[2]=M_PI;
 sseg->cameraup[0] =M_PI; sseg->cameraup[1] =M_PI, sseg->cameraup[2] =M_PI;
 sseg->markedatom = -1;
 sseg->numstat = tags.Size();
 sseg->numplots = plot.Size();
 sseg->numxrange = xrange.Size();
 sseg->numyrange = yrange.Size();
 sseg->paused = bool(params["paused"]);
 sseg->autorot = bool(params["autorotate"]);
 sseg->atomrad = double(params["radius"]);
 sseg->natoms = atoms.Size();
 sseg->markedatom = int(params["mark"]);
 sseg->quality = int(params["quality"]);
 sseg->persp = bool(params["perspective"]);
 
 //----------------------------------  RESERVA DE MEMORIA DE PUNTEROS -----------------------------------------------//
 // COLORES: Reserva memoria para los 3 punteros que almacenarán los colores de cada átomo
 clrsize= (sseg->natoms)*sizeof(double);
 for (int q=0; q<3; ++q)
 {
  void *pointer = mmap(NULL, clrsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->color[q] = (double *)(pointer); 
 }
 // ESTADISTICAS: Reserva memoria para los 50 punteros que almacenarán los nombres de todas las estadisticas
 stsize=100;
 for (int q=0;q<50;++q)
 {
  void *pointer = mmap(NULL, stsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->stname[q] = (char *)(pointer); 
 }
 // GRAFICOS: Reserva memoria para los 50 punteros que almacenarán los nombres de las propiedades que se graficaran
 ptsize=100;
 for (int q=0;q<50;++q)
 {
  void *pointer = mmap(NULL, ptsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->ptname[q] = (char *)(pointer); 
 }
 // DATA: Reserva memoria para los 50 punteros que almacenarán las estadísticas en cada time-step
 datasize = (sseg->steps)*sizeof(double);
 for (int q=0;q<50;++q)
 {
  void *pointer = mmap(NULL, datasize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->data[q] = (double *)(pointer); 
 }
 // BACKGROUND: Reserva memoria para el puntero que almacenara el nombre del color de fondo
 bgsize = params["background"].size()*sizeof(char);
 for (int q=0;q<1;++q)
 {
  void *pointer = mmap(NULL, bgsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->bg = (char *)(pointer);
 }
 // BACKGROUND: Reserva memoria para el puntero que almacenara el nombre del color de fondo
 gbgsize = params["graphbg"].size()*sizeof(char);
 for (int q=0;q<1;++q)
 {
  void *pointer = mmap(NULL, gbgsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (pointer == MAP_FAILED) throw PluginError("lpvisual", "Shared memory failed.");
  sseg->gbg = (char *)(pointer);
 }

 //---------------------------------  INICIALIZACION DE PUNTEROS  ---------------------------------------------------//
 // Inicializar los colores
 for (long int i=0;i<(sseg->natoms);++i)
 {
  if (ColorHandler::HaveColor(atoms[i])) for (int j=0; j<3; ++j) sseg->color[j][i]=ColorHandler::ColorOfAtom(atoms[i])[j];
  else for (int j=0; j<3; ++j) sseg->color[j][i]=ColorHandler::DefaultColor(atoms[i])[j];
 }
 // Inicializar los nombres y valores de las estadisticas
 for (int q=0;q<tags.Size();++q)
 {
  const std::string & tag = tags[q];
  if (sim.Have(sim, Tag(tag)))
  {
   sseg->statistics[q] = double(Parameter(sim.GetTag(sim, Tag(tag))));
   strncpy(&(sseg->stname[q][0]), tags[q].c_str(), strlen(tags[q].c_str())+1);
   sseg->noprop=false;
  }
 }
 // Inicializar los nombres y valores de las propiedades en el grafico
 for (int q=0;q<plot.Size();++q)
 {
  const std::string & tag = plot[q];
  if (sim.Have(sim, Tag(tag)))
  {
   sseg->data[q][sseg->mdstep] = double(Parameter(sim.GetTag(sim, Tag(tag))));
   strncpy(&(sseg->ptname[q][0]), plot[q].c_str(), strlen(plot[q].c_str())+1);
  }
 }
 // Inicializar los parametros de xrange
 if (xrange.Size() > 1)
 {
  for (int q=0;q<xrange.Size();++q)
  {
   const std::string & tag = xrange[q];
   double par = string_to_double(tag);
   sseg->xrange[q] = par;
  }
 }
 // Inicializar los parametros de yrange
 if (yrange.Size() > 1)
 {
  for (int q=0;q<yrange.Size();++q)
  {
   const std::string & tag = yrange[q];
   double par = string_to_double(tag);
   sseg->yrange[q] = par;
  }
 }
 // Inicializar los parametros de camerapos
 if (camerapos.Size() > 1)
 {
  for (int q=0;q<camerapos.Size();++q)
  {
   const std::string & tag = camerapos[q];
   double par = string_to_double(tag);
   sseg->camerapos[q] = par;
  }
 }
 // Inicializar los parametros de cameraobj
 if (cameraobj.Size() > 1)
 {
  for (int q=0;q<cameraobj.Size();++q)
  {
   const std::string & tag = cameraobj[q];
   double par = string_to_double(tag);
   sseg->cameraobj[q] = par;
  }
 }
 // Inicializar los parametros de cameraup
 if (cameraup.Size() > 1)
 {
  for (int q=0;q<cameraup.Size();++q)
  {
   const std::string & tag = cameraup[q];
   double par = string_to_double(tag);
   sseg->cameraup[q] = par;
  }
 }

 // Inicializar el nombre del color de fondo
 for (int q=0;q<1;++q)
 {
  strncpy(&(sseg->bg[0]), params["background"].c_str(), strlen(params["background"].c_str())+1);
 }
 // Inicializar el nombre del color de fondo del graficador
 for (int q=0;q<1;++q)
 {
  strncpy(&(sseg->gbg[0]), params["graphbg"].c_str(), strlen(params["graphbg"].c_str())+1);
 }
 // Inicializa los vectores de la celda en sseg->cell
 for (int i=0;i<3;++i) for (int j=0;j<3;++j) sseg->cell[i][j] = celda[i][j];

 float * coordbuffer = (float *)((char *)(shm)+sizeof(SharedSegment));
 for (long q=0;q<3*atoms.Size();++q) coordbuffer[q] = 0.0;

 pid_t pid = fork();
 if (pid == 0)                  // Este es el hilo que controla la ventana grafica
 {
  int w = int(params["width"]);
  int h = int(params["height"]);
  int qual = int(params["quality"]);
  if (qual > 5) 
  {
   ShowWarning("plugin lpvisual", "using maximum quality = 5");
   qual = 5;
  }
  int plon = polig[qual][0];
  int plat = polig[qual][1];
  double phi = M_PI*double(params["azimuth"])/180.0;
  double theta = M_PI*double(params["zenith"])/180.0;
 
  dispman = new Display(sim, *sseg);
  dispman->SetupDisplay(w, h, plon, plat, phi, theta);
  sseg->disp = dispman;        // Este puntero a Display solo tiene validez dentro del hilo de la ventana
  dispman->MainLoop();
  active = true;
 }
 else AssignParameter("windowpid", ToString<int>(pid));   // Este es el hilo original
}

void LPVisual::Apply(const lpmd::Simulation & sim) 
{ 
 const BasicParticleSet & atoms = sim.Atoms();
 const BasicCell & celda = sim.Cell();

 // Si no hay ventana grafica todavia, abrir una
 if ((! Defined("windowpid")) && (! active))
 {
  DebugStream() << "-> Creating a new LPVisual window\n";
  SpawnWindowThread(sim); 
 }
 // Cuando haya una ventana grafica, hacer todo
 if (Defined("windowpid"))
 {
  SharedSegment * sseg = (SharedSegment *)(shm);
  if ((! sseg->liveWIN1) && (applied_once)) throw MissingComponent("lpvisual window");
  else if (! sseg->liveWIN1) return;
  sseg->mdstep = sim.CurrentStep();
  if (sseg->paused) 
  {
   // La ventana fue pausada, pausamos el hilo de la simulacion hasta que se suelte la pausa
   while (sseg->paused) usleep(50000);
  }
  float * coordbuffer = (float *)((char *)(shm)+sizeof(SharedSegment));

  // Actualizar las posiciones atomicas en coordbuffer
  for (long int i=0;i<atoms.Size();++i) 
  {
   for (int q=0;q<3;++q) coordbuffer[i*3+q] = atoms[i].Position()[q];
  }

  // Actualiza los vectores de la celda en SharedSegment
  for (int i=0;i<3;++i) for (int j=0;j<3;++j) sseg->cell[i][j] = celda[i][j];

  // Actualizar los colores
  for (long int i=0;i<(sseg->natoms);++i)
  {
   if (ColorHandler::HaveColor(atoms[i])) for (int j=0; j<3; ++j) sseg->color[j][i]=ColorHandler::ColorOfAtom(atoms[i])[j];
   else for (int j=0; j<3; ++j) sseg->color[j][i]=ColorHandler::DefaultColor(atoms[i])[j];
  }

  // Actualizar los nombres y valores de las estadisticas
  for (int q=0;q<tags.Size();++q)
  {
   const std::string & tag = tags[q];
   if (sim.Have(sim, Tag(tag)))
   {
    sseg->statistics[q] = double(Parameter(sim.GetTag(sim, Tag(tag))));
    strncpy(&(sseg->stname[q][0]), tags[q].c_str(), strlen(tags[q].c_str())+1);
   }
  }

  // Actualizar los nombres y valores de las propiedades en el grafico
  for (int q=0;q<plot.Size();++q)
  {
   const std::string & tag = plot[q];
   if (sim.Have(sim, Tag(tag)))
   {
    sseg->data[q][sseg->mdstep] = double(Parameter(sim.GetTag(sim, Tag(tag))));
    strncpy(&(sseg->ptname[q][0]), plot[q].c_str(), strlen(plot[q].c_str())+1);
   }
  }
  applied_once = true;
  if (!sseg->updating) sseg->mustUpdate = true;
 }
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new LPVisual(args); }
void destroy(Plugin * m) { delete m; }

