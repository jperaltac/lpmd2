//
//
//

#ifndef __LPVISUAL_DISPLAY_H__
#define __LPVISUAL_DISPLAY_H__

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <lpmd/vector.h>
#include <lpmd/simulation.h>

using namespace lpmd;

typedef struct 
{
	float crd[3];
	int marked;
}	atomstruct;

class SharedSegment;       // forward

class Display
{
 public:
   // 
   Display(const lpmd::Simulation & sim, SharedSegment & sharedseg);
   virtual ~Display();

   void SetupDisplay(int width, int height, int nplon, int nplan, double azimuth, double zenith);
   void MainLoop();
   void MakeZoomer(GLuint listid);   
   void makeCurframeGeom();


 private:
   //
   SharedSegment * sseg;
   long int natoms; // number of atoms
   GLuint sphereid; // display-list id of atom sphere geom
   int nlon, nlat;  // Number of polygons for a sphere (longitude, latitude)

   //
   void MakeFastNiceSphere(GLuint listid, double radius);
   void MakeAtomPoint(GLuint listid);
   void MakeCylinder(GLuint listid);
   void MakeAtoms();
   void MakeBox(bool, double cell[3][3]);
   void MakeAxes(double cell[3][3], GLuint id);
};

// SPECIAL FUNCTIONS TO BE USED IN GLUT FUNCTIONS
void RotateVector(float *v, float *axis, float ang);
void ClipObjects(double dist);
void SetCamera(double dist);
void SetCamera(double dist, double theta, double phi);
void MoveCamera(double dx, double dy);
void SolidCylinder(double r, double h);
void Print(char TXT[], void *letter, std::string st);
void Print(char TXT[], void *letter, std::string st, double posx, double posy, double posz);
void Print(char TXT[], void *letter, std::string st, double value, double posx, double posy, double posz);
void Print(char TXT[], void *letter, std::stringstream &txt, double posx, double posy, double posz);
void Zoomer(void);
void InitWIN1(void);
void Init(void);
void DrawStatistics(void);
void DrawStatistics2(void);
void DrawStatistics3(void);
void CreateWIN1(void);
void CreateWIN2(void);
void CreateWIN3(void);
void CreateWIN4(void);
void MainMenu(int option);
void MenuWindow2(int option);
void MenuWindow3(int option);
void MenuWindow4(int option);

// GLUT CALLBACK FUNCTIONS
void Reshape(int, int);
void CheckUpdates(void);
void CheckKeyboard(unsigned char key, int x, int y);
void CheckDirectionalKeys(int key, int x, int y);
void CheckMouse(int button , int state, int x, int y);
void CheckMouseMove(int x, int y);
void Display1(void);
void Display2(void);
void Display3(void);
void Display4(void);

class SharedSegment
{
 public:
   double atomrad;
   double cell[3][3];    // cell[i][j] is the j component of the cell vector i
   int numstat;          // number of statistics to monitor on the screen
   int numplots;         // number of properties that will be plot
   int numxrange;        // number of parameters of xrange (start, end, each)
   int numyrange;        // number of parameters of yrange (start, end, each)
   int each;             // Molecular-Dynamics-Step Gap
   double statistics[50];// Keeps the information of the statistics that will be drawn on the screen
   char * stname[50];    // Keeps the names of the statistics that will be drawn on the screen
   char * ptname[50];    // Keeps the names of the properties tha will be plot
   double *color[3];     // color[0][i] is the red color of the atom i, color[1][i] the green one, and color[2][i] blue one.
   double *data[50];     // Points of the graphics
   double xrange[3], yrange[3]; // [3]: start, end, each
   double camerapos[3];  // [3]: (x,y,z) position of the camera
   double cameraobj[3];  // [3]: (x,y,z) objective point of the camera
   double cameraup[3];   // [3]: (x,y,z) position of the camera
   long mdstep;          // Time-step of the simulation
   long start;           // First step of the simulation cell
   long steps;           // Amount of total steps of the simulation
   long natoms;          // Amount of atoms of the configuration
   bool mustUpdate;      // Checks updatings
   bool updating;        // Verifies if the data is updating
   bool paused;          // State of the simulation (paused/playing)
   bool liveWIN1;
   bool liveWIN2;
   bool liveWIN3;
   bool liveWIN4;
   bool persp;
   bool noprop;
   bool autorot;
   char * bg;
   char * gbg;
   Display * disp;
   int WIN1, width1, height1;
   int WIN2, width2, height2;
   int WIN3, width3, height3;
   int WIN4, width4, height4;
   int markedatom;
   int quality;
};

#endif

