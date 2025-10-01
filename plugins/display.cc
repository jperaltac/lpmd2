//
// Adapted from "Visualizing MD simulation" demo by Aiichiro Nakano (atomv.h and atomv.c)
// at http://cacs.usc.edu/education/cs596.html 
//

#include "display.h"
#include <lpmd/particleset.h>

#include <cmath>
#include <unistd.h>
#include <string.h>

/************************************************** CONSTANTS TO USE ***************************************************/
static long SctAtm=0;                             // Selected atom
static float Ratom=0.0, Gatom = 0.0, Batom = 0.0; // RGB color of an atom
static float eye[3];                              // position of eye point
static float center[3];                           // position of looking reference point
static float distance, orth_dist;                 // distance to the camera
static float camdir[3];                           // camera direction (unitary vector from the center to the camera)
static float v[3];                                // 90° spherical-theta rotation of camdir used to calc up[3]
static float up[3];                               // up direction for camera
static float diag[3];                             // Range of atomic coordinates: (0,0,0) to diag=(right,top,front)
static float zoom_factor=1;                       // zoom "velocity"
static float tx = 0, ty = 0, gap=1;               // translation of the scene
static float marker=0, marker2=0, markerZ=0;      // position of the ball-markers on the screen
static float dm=40/72.0, dm2=50/72.0;             // movement of the ball-markers
static float diagonal;                                 // average length of the axis
static float move_help=0;

static bool RefFrame=true;                        // Show/Hide reference frame
static bool axis=true;                            // Show/Hide axes
static bool Lines_Or_Quads=true;                  // Chage Lines to Quads
static bool wrt=true;                             // Show/Hide text in the window
static bool AtomSel=false;                        // Enable atom selection
static bool Zup=false, Zdown=false, RotUp=false, RotDown=false, RotRight=false, RotLeft=false; // Zoomer
static bool grid=false;                           // Show/Hide grid in graphics window
static bool z_held_down=false, Z_held_down=false; // is 'z' or 'Z' held down?
static bool u_arrow_down=false;                   // is an arrow held down?
static bool d_arrow_down=false;                   // is an arrow held down?
static bool l_arrow_down=false;                   // is an arrow held down?
static bool r_arrow_down=false;                   // is an arrow held down?
static bool point=false;                          // Asked or not asked fixed LookAt point

unsigned char Buttons[3] = {0};
static int mousecoord[2];                         // pixel-coordinates of the mouse
static int vel=4;                                 // Scene-Rotation velocity
static GLuint atomsid, cylID, axesID, zoomID;     // display-list id's of all atoms, cylinder, axes and zoom
static GLdouble fovy, aspect, near, far;// parameters for gluPerspective()
static GLdouble pos3D_x, pos3D_y, pos3D_z;        // parameters for gluUnproject
static GLdouble model_view[16];                   // parameters for gluUnproject
static GLdouble projection[16];                   // parameters for gluUnproject
static GLint viewport[4];                         // parameters for gluUnproject
static atomstruct * atoms;                        // array of atoms 
static void * shpointer;                          // Pointer to shared memory

enum {EXIT, OPEN_WIN2, OPEN_WIN3, OPEN_WIN4, HIDE2, HIDE3, HIDE4, GRID4};
enum
{
 B9_BY_15,
 B8_BY_13,
 TIMES_ROMAN_10,
 TIMES_ROMAN_24,
 HELVETICA_10,
 HELVETICA_12,
 HELVETICA_18
};
static void *Letter=GLUT_BITMAP_8_BY_13;
static int op=HELVETICA_18;
std::string SA="";
/***********************************************************************************************************************/

/********************************** IMPLEMENTATION OF THE CLASS PRIVATE MEMBERS ****************************************/
//--------------------------------------------- MakeFastNiceSphere ----------------------------------------------------//
// Called once to generate and compile sphere geometry into the given display list id.
void Display::MakeFastNiceSphere(GLuint listid, double radius) 
{
 int i,j;
 float lon,lat;
 float loninc,latinc;
 float x,y,z;

 loninc = 2*M_PI/nlon;
 latinc = M_PI/nlat;

 glNewList(listid,GL_COMPILE);

 /* South-pole triangular fan */
 glBegin(GL_TRIANGLE_FAN);
 glNormal3f(0,-1,0);
 glVertex3f(0,-radius,0);
 lon = 0;
 lat = -M_PI/2 + latinc;
 y = sin(lat);
 for (i=0; i<=nlon; i++)
 {
  x = cos(lon)*cos(lat);
  z = -sin(lon)*cos(lat);
  glNormal3f(x,y,z);
  glVertex3f(x*radius,y*radius,z*radius);
  lon += loninc;
 }
 glEnd();

 /* Quadrilateral stripes to cover the sphere */
 for (j=1; j<nlat-1; j++) {
 lon = 0;
 glBegin(GL_QUAD_STRIP);
 for (i=0; i<=nlon; i++)
 {
  x = cos(lon)*cos(lat);
  y = sin(lat);
  z = -sin(lon)*cos(lat);
  glNormal3f(x,y,z);
  glVertex3f(x*radius,y*radius,z*radius);
  x = cos(lon)*cos(lat+latinc);
  y = sin(lat+latinc);
  z = -sin(lon)*cos(lat+latinc);
  glNormal3f(x,y,z);
  glVertex3f(x*radius,y*radius,z*radius);
  lon += loninc;
 }
 glEnd();
 lat += latinc;
}

 /* North-pole triangular fan */
 glBegin(GL_TRIANGLE_FAN);
 glNormal3f(0,1,0);
 glVertex3f(0,radius,0);
 y = sin(lat);
 lon = 0;
 for (i=0; i<=nlon; i++)
 {
  x = cos(lon)*cos(lat);
  z = -sin(lon)*cos(lat);
  glNormal3f(x,y,z);
  glVertex3f(x*radius,y*radius,z*radius);
  lon += loninc;
 }
 glEnd();

 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------- MakeAtomPoint -------------------------------------------------------//
void Display::MakeAtomPoint(GLuint listid)
{
 glNewList(listid, GL_COMPILE);
 glPointSize(2.0f);
 glBegin(GL_POINTS);
 glVertex3f(0.0f, 0.0f, 0.0f);
 glEnd();
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------- MakeCylinder --------------------------------------------------------//
void Display::MakeCylinder(GLuint listid)
{
 glNewList(listid, GL_COMPILE);
  SolidCylinder(1,1);
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------- MakeZoomer ----------------------------------------------------------//
void Display::MakeZoomer(GLuint listid)
{
 glNewList(listid, GL_COMPILE);
  Zoomer();
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- MakeAtoms ..-------------------------------------------------------//
// Makes display-list of all atoms in the current frame using spheres.
void Display::MakeAtoms() 
{
 int i;
 glNewList(atomsid, GL_COMPILE);
 for (i=0; i < natoms; i++) 
 {
  glPushMatrix();
  Ratom=sseg->color[0][i]; Gatom=sseg->color[1][i]; Batom=sseg->color[2][i];  /* RGB color of an atom */
  glTranslatef(atoms[i].crd[0],atoms[i].crd[1],atoms[i].crd[2]);
  if (atoms[i].marked == 0) glColor3f(Ratom,Gatom,Batom);
  else glColor3f(1.0, 0.0, 0.0);
  glCallList(sphereid);
  glPopMatrix();
 }
 if (AtomSel)
 {
  glPushMatrix();
  glTranslatef(atoms[SctAtm].crd[0],atoms[SctAtm].crd[1],atoms[SctAtm].crd[2]);
  glColor3f(1.0, 0.0, 0.0);
  glutSolidSphere(2*sseg->atomrad,20,16);
  glPopMatrix();
 }
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------- MakeBox -------------------------------------------------------//
void Display::MakeBox(bool lines, double cell[3][3])
{
 Vector A(cell[0][0],cell[0][1],cell[0][2]);
 Vector B(cell[1][0],cell[1][1],cell[1][2]);
 Vector C(cell[2][0],cell[2][1],cell[2][2]);
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);
 glDisable(GL_LIGHTING);

 glColor3f(0,1,0);
 std::string fondo=sseg->bg;
 if      (fondo == "white") glColor3f(0,0,0);
 else if (fondo == "gray") glColor3f(1,1,1);
 else glColor3f(0,1,0);


 GLint GLAlgo = (lines) ? GL_LINE_LOOP : GL_QUADS;
 // Bottom face formed with the vectors 0 and 1 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(0,0,0);
  glVertex3d(A[0],A[1],A[2]);
  glVertex3d(A[0]+B[0],A[1]+B[1],A[2]+B[2]);
  glVertex3d(B[0],B[1],B[2]);
 glEnd();
 // Top face formed with the vectors 0 and 1 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(C[0],C[1],C[2]);
  glVertex3d(A[0]+C[0],A[1]+C[1],A[2]+C[2]);
  glVertex3d(A[0]+B[0]+C[0],A[1]+B[1]+C[1],A[2]+B[2]+C[2]);
  glVertex3d(B[0]+C[0],B[1]+C[1],B[2]+C[2]);
 glEnd();
 // Bottom face formed with the vectors 0 and 2 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(0,0,0);
  glVertex3d(A[0],A[1],A[2]);
  glVertex3d(A[0]+C[0],A[1]+C[1],A[2]+C[2]);
  glVertex3d(C[0],C[1],C[2]);
 glEnd();
 // Top face formed with the vectors 0 and 2 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(B[0],B[1],B[2]);
  glVertex3d(A[0]+B[0],A[1]+B[1],A[2]+B[2]);
  glVertex3d(A[0]+C[0]+B[0],A[1]+C[1]+B[1],A[2]+C[2]+B[2]);
  glVertex3d(C[0]+B[0],C[1]+B[1],C[2]+B[2]);
 glEnd();
 // Bottom face formed with the vectors 1 and 2 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(0,0,0);
  glVertex3d(B[0],B[1],B[2]);
  glVertex3d(B[0]+C[0],B[1]+C[1],B[2]+C[2]);
  glVertex3d(C[0],C[1],C[2]);
 glEnd();
 // Top face formed with the vectors 1 and 2 of the simulation cell 
 glBegin(GLAlgo);
  glVertex3d(A[0],A[1],A[2]);
  glVertex3d(B[0]+A[0],B[1]+A[1],B[2]+A[2]);
  glVertex3d(B[0]+C[0]+A[0],B[1]+C[1]+A[1],B[2]+C[2]+A[2]);
  glVertex3d(C[0]+A[0],C[1]+A[1],C[2]+A[2]);
 glEnd();

 glPopAttrib();
 glPopMatrix();
}
//-------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------ MakeAxes ---------------------------------------------------------//
// Makes the axes of the simulation cell
void Display::MakeAxes(double cell[3][3], GLuint listid)
{
 Vector A(cell[0][0],cell[0][1],cell[0][2]);
 Vector B(cell[1][0],cell[1][1],cell[1][2]);
 Vector C(cell[2][0],cell[2][1],cell[2][2]);

 double radii;
 radii=(A.Module() < B.Module())? A.Module() : B.Module();
 radii=(radii < C.Module())? radii : C.Module();
 radii*=0.01;
 char X[2];
 void *tp=GLUT_BITMAP_HELVETICA_18;
 std::string fondo=sseg->bg;
 
 glNewList(listid, GL_COMPILE);
 
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);
 glDisable(GL_LIGHTING);
 // Z-axis
 glPushMatrix();
  glRotatef(90,0,0,1);
  glPushMatrix();
   glColor3f(0,0,1);
   glScaled(radii,radii,10*radii);
   glCallList(cylID);
  glPopMatrix();
  glColor3f(1,0,0);
  glPushMatrix();
   glTranslatef(0,0,10*radii);
   glutSolidCone(3*radii,6*radii,20,10);
   if (fondo == "white") glColor3f(0,0,0);
   else glColor3f(1,1,0);
   Print(X,tp,"Z",0,0,10*radii);
  glPopMatrix();
 glPopMatrix();
 // Y-axis
 glPushMatrix();
  glRotatef(-90,1,0,0);
  glPushMatrix();
   glColor3f(0,0,1);
   glScaled(radii,radii,10*radii);
   glCallList(cylID);
  glPopMatrix();
  glColor3f(1,0,0);
  glPushMatrix();
   glTranslatef(0,0,10*radii);
   glutSolidCone(3*radii,6*radii,20,10);
   if (fondo == "white") glColor3f(0,0,0);
   else glColor3f(1,1,0);
   Print(X,tp,"Y",0,0,10*radii);
  glPopMatrix();
 glPopMatrix();
 // X-axis
 glPushMatrix();
  glRotatef(90,0,1,0);
  glPushMatrix();
   glColor3f(0,0,1);
   glScaled(radii,radii,10*radii);
   glCallList(cylID);
  glPopMatrix();
  glColor3f(1,0,0);
  glPushMatrix();
   glTranslatef(0,0,10*radii);
   glutSolidCone(3*radii,6*radii,20,10);
   if (fondo == "white") glColor3f(0,0,0);
   else glColor3f(1,1,0);
   Print(X,tp,"X",0,0,10*radii);
  glPopMatrix();
 glPopMatrix();
 
 glPopAttrib();
 glPopMatrix();
 
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//
/*********************************************************************************************************************/

/********************************** IMPLEMENTATION OF THE CLASS PUBLIC MEMBERS ***************************************/
//------------------------------------------- Constructor -----------------------------------------------------------//
Display::Display(const lpmd::Simulation & sim, SharedSegment & sharedseg): sseg(&sharedseg)
{
 // Copiamos los SharedSegment construidos en lpvisual.cc en la variable provada sseg:
 shpointer = (void *)(sseg);

 const lpmd::BasicCell & celda = sim.Cell();
 const lpmd::BasicParticleSet & sim_atoms = sim.Atoms();

 /* Get the cell diagonal vector */
 for (int q=0;q<3;++q) diag[q] = celda[0][q]+celda[1][q]+celda[2][q];

 /* Get the diagonal module */
 diagonal = (celda[0]+celda[1]+celda[2]).Module();

 /* Get number of atoms */
 natoms = sim_atoms.Size();
 atoms = new atomstruct[sim_atoms.Size()]; 
 for (long int i=0;i<natoms;++i)
 {
  for (int q=0;q<3;++q) atoms[i].crd[q] = sim_atoms[i].Position()[q];
  atoms[i].marked = 0;
 }
 if (sseg->markedatom != -1) atoms[sseg->markedatom].marked = 1;
}
//-------------------------------------------------------------------------------------------------------------------//
//------------------------------------------- Destructor -----------------------------------------------------------//
Display::~Display() { }
//-------------------------------------------------------------------------------------------------------------------//

//------------------------------------------- SetupDisplay -----------------------------------------------------------//
void Display::SetupDisplay(int width, int height, int nplon, int nplat, double phi, double theta)
{
 int argc = 1;
 char argvstring[8];
 strncpy(argvstring, "lpvisual", 8);
 char * argv = argvstring;
 glutInit(&argc, &argv);
 glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);        // Initialize display mode

 SharedSegment * sseg = (SharedSegment *)(shpointer);
 // 
 nlon = nplon;
 nlat = nplat;
 sseg->width1 = width;
 sseg->height1 = height;

 /* Set center in world space */
 if (sseg->cameraobj[0]==M_PI && sseg->cameraobj[1]==M_PI && sseg->cameraobj[2]==M_PI)
  for (int q=0;q<3;++q) center[q] = 0.5*diag[q]; // default
 else // user's choice
 {
  for (int q=0;q<3;++q) center[q]=sseg->cameraobj[q];
  point=true;
 }

 /* Near- & far clip-plane distances for gluPerspective() */
 near = (GLdouble)( 0.5*(diagonal-0.5*diag[2]) );
 far  = (GLdouble)( 2.0*(diagonal+0.5*diag[2]) );

  /* Finding a distance similar to the side of a cube to set the field of view 'fovy' */
 double min=diagonal/sqrt(3);
 fovy = (GLdouble)( 0.5*min/(2*near) );
 fovy = (GLdouble)( 2*atan((double)fovy)/M_PI*180.0 ); // Field of view touches exactly the borders of the cell
 fovy = (GLdouble)( 1.2*fovy); // We open the field of view a little bit more.

 /* Fix initial distance to the camera */
 distance = diagonal;
 orth_dist = diagonal;

 /* If the user fix the camera position, fix distance=||camerapos-center|| */
 double x=sseg->camerapos[0];
 double y=sseg->camerapos[1];
 double z=sseg->camerapos[2];
 if (x!=M_PI && y!=M_PI && z!=M_PI)
 {
  double x=sseg->camerapos[0]-center[0];
  double y=sseg->camerapos[1]-center[1];
  double z=sseg->camerapos[2]-center[2];
  double r2 = x*x+y*y+z*z;
 // ReSet theta and phi if the user moved the camera
  theta = acos(z/sqrt(r2));
  if(y==0 && x>0) phi=0;
  else if (x>0 && y>0) phi = atan(y/x);
  else if(x==0 && y>0) phi=0.5*M_PI;
  else if (x < 0 && y > 0) phi = atan(y/x)+M_PI;
  else if(y==0 && x<0) phi=M_PI;
  else if (x < 0 && y < 0) phi = atan(y/x)+M_PI;
  else if(x==0 && y<0) phi=1.5*M_PI;
  else if (x>0 && y<0) phi = atan(y/x)+2*M_PI;
  else phi=0;
  // Set the distance to the camera
  double diff[3], diff2=0.0;
  for (int q=0;q<3;++q)  diff[q]=sseg->camerapos[q]-center[q];
  for (int q=0;q<3;++q)  diff2+=diff[q]*diff[q];
  distance=(float)sqrt((double)diff2);
  orth_dist=(float)sqrt((double)diff2);
 }

  /* Set 'camdir', the unitary vector that points from the center of the cell towards the camera */
 camdir[0] = sin(theta)*cos(phi);
 camdir[1] = sin(theta)*sin(phi);
 camdir[2] = cos(theta);
 float phz = 0.5*M_PI;
 /* Set 'v', a 90° rotation of the vector 'camdir' (adding 0.5*M_PI to the "theta" angle in spherical coordinates) along the plane phi=const. */
 v[0] = sin(theta+phz)*cos(phi); 
 v[1] = sin(theta+phz)*sin(phi);
 v[2] = cos(theta+phz);
 /* Set 'up'='camdir'x'v' vector, that indicates which direction is up (the direction from the bottom to the top of the viewing volume) */
 up[0] = camdir[1]*v[2] - camdir[2]*v[1];
 up[1] = camdir[2]*v[0] - camdir[0]*v[2];
 up[2] = camdir[0]*v[1] - camdir[1]*v[0];
 float cm = 0.0;
 for (int q=0;q<3;++q) cm += up[q]*up[q];
 for (int q=0;q<3;++q) up[q] /= sqrt(cm);
 
 if (sseg->cameraup[0]!=M_PI && sseg->cameraup[1]!=M_PI && sseg->cameraup[2]!=M_PI)
 {
  for (int q=0;q<3;++q) up[q]=sseg->cameraup[q];
  v[0] = up[1]*camdir[2] - up[2]*camdir[1];
  v[1] = up[2]*camdir[0] - up[0]*camdir[2];
  v[2] = up[0]*camdir[1] - up[1]*camdir[0];
 }

 //WINDOW 3: Simulation Data
 CreateWIN3();

 // WINDOW 1: Main Window
 CreateWIN1();

 /* generate an OpenGL display list for single sphere */
 sphereid = glGenLists(1);
 if ((nlon == 0) && (nlat == 0)) MakeAtomPoint(sphereid);
 else MakeFastNiceSphere(sphereid,sseg->atomrad);
 
 /* generate an OpenGL display list for single cylinder */
 cylID = glGenLists(1);
 MakeCylinder(cylID);

 /* generate an OpenGL display list for the coordinate axes */
 axesID = glGenLists(1);
 MakeAxes(sseg->cell, axesID);
 
 /* generate an OpenGL display list for the coordinate axes */
 zoomID = glGenLists(1);
 MakeZoomer(zoomID);
 
 /* generate an OpenGL display list for the atoms' geometry */
 atomsid = glGenLists(1);
 /* make the geometry of the current frame's atoms */
 makeCurframeGeom();
}
//-------------------------------------------------------------------------------------------------------------------//

//----------------------------------------- Main Loop ---------------------------------------------------------------//
void Display::MainLoop()
{
 glutMainLoop();  // Start main display loop
}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------- makeCurframeGeom --------------------------------------------------------//
// Reads the atoms information for the current time frame and makes the
// display-list of all the atoms geometry.
void Display::makeCurframeGeom()
{
 glNewList(atomsid, GL_COMPILE ) ;
  if (RefFrame) MakeBox(Lines_Or_Quads, sseg->cell);
  if (axis) glCallList(axesID);
  MakeAtoms();
 glEndList();
}
//-------------------------------------------------------------------------------------------------------------------//

/***********************************************************************************************************************/
/********************************************** SPECIAL FUNCTIONS ******************************************************/
//---------------------------------------------------- RotateVector ------------------------------------------------------//
// It is used to rotate the camera in the "MoveCamera" function
// Method taken from http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/
void RotateVector(float * vect, float * axis, float ang)
{
 float u=axis[0],v=axis[1],w=axis[2];
 float x=vect[0],y=vect[1],z=vect[2];
 float norm2=u*u+v*v+w*w;

 double rv[3];
 rv[0] = u*(u*x+v*y+w*z)+(x*(v*v+w*w)-u*(v*y+w*z))*cos(ang)+sqrt(u*u+v*v+w*w)*(-w*y+v*z)*sin(ang);
 rv[1] = v*(u*x+v*y+w*z)+(y*(u*u+w*w)-v*(u*x+w*z))*cos(ang)+sqrt(u*u+v*v+w*w)*(w*x-u*z)*sin(ang);
 rv[2] = w*(u*x+v*y+w*z)+(z*(u*u+v*v)-w*(u*x+v*y))*cos(ang)+sqrt(u*u+v*v+w*w)*(-v*x+u*y)*sin(ang);

 for (int q=0; q<3; ++q) vect[q]=rv[q]/norm2;

}
//------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------- Clip Objects ------------------------------------------------------//
void ClipObjects(double dist)
{
// Set the far-clip (perspetive mode) always beyond the objects
 near = (GLdouble)( 0.5*(fabs(dist-0.5*diag[2])) );
 far  = (GLdouble)( 2.0*(fabs(dist+0.5*diag[2])) );
}
//------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------- SetCamera ...------------------------------------------------------//
// This function will be used in any function that involves a camera movement
void SetCamera(double dist)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 if (!point) for (int q=0;q<3;++q) center[q] = 0.5*diag[q] + tx*v[q] + ty*up[q];
 else for (int q=0;q<3;++q) center[q] = sseg->cameraobj[q];

 for (int q=0;q<3;++q) eye[q] = center[q] + dist*camdir[q];

 ClipObjects(dist);
}
void SetCamera(double theta, double phi)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 // 'camdir' is the unitary vector that points from the center of the cell towards the camera
 camdir[0] = sin(theta)*cos(phi);
 camdir[1] = sin(theta)*sin(phi);
 camdir[2] = cos(theta);
 float phz = 0.5*M_PI;
 // 'v' is a 90° rotation of the vector 'camdir' (adding 0.5*M_PI to the "theta" angle in spherical coordinates) along the plane phi=const.
 v[0] = sin(theta+phz)*cos(phi);
 v[1] = sin(theta+phz)*sin(phi);
 v[2] = cos(theta+phz);
 // 'up'='camdir'x'v' vector indicates which direction is up (the direction from the bottom to the top of the viewing volume)
 up[0] = camdir[1]*v[2] - camdir[2]*v[1];
 up[1] = camdir[2]*v[0] - camdir[0]*v[2];
 up[2] = camdir[0]*v[1] - camdir[1]*v[0];

 markerZ=0; marker=0; marker2=0;
 if(sseg->persp) SetCamera(distance);
 else            SetCamera(orth_dist);
}

//-------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------ Move Camera -----------------------------------------------------------//
// If you call MoveCamera(0,3), the camera will move around the vertical axis +3 degrees (right-hand rule assumed)
// If you call MoveCamera(2,0), the camera will move around the horizontal axis +2 degrees (right-hand rule assumed)
void MoveCamera(double angx, double angy)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 float rotx = M_PI*angx/180.0;
 float roty = M_PI*angy/180.0;

 float horiz[3];
 // 'horiz'='up'x'camdir'='v'
 horiz[0] = up[1]*camdir[2] - up[2]*camdir[1];
 horiz[1] = up[2]*camdir[0] - up[0]*camdir[2];
 horiz[2] = up[0]*camdir[1] - up[1]*camdir[0];

 RotateVector(camdir, horiz, roty);
 RotateVector(camdir, up, rotx);

 RotateVector(v, horiz, roty);
 RotateVector(v, up, rotx);
 
 // 'up'='camdir'x'v'
 up[0] = camdir[1]*v[2] - camdir[2]*v[1];
 up[1] = camdir[2]*v[0] - camdir[0]*v[2];
 up[2] = camdir[0]*v[1] - camdir[1]*v[0];

 if (sseg->persp) SetCamera(distance);
 else SetCamera(orth_dist);

}
//------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- SolidCylinder ------------------------------------------------------//
// Draw a cone of radii r and height h
void SolidCylinder(double r, double h)
{
 double vtx=10;
 glBegin(GL_POLYGON);
  for (int n=0; n<=vtx; n++) glVertex3d(r*cos(2*M_PI*n/vtx),r*sin(2*M_PI*n/vtx),0);
 glEnd();
 glBegin(GL_POLYGON);
  for (int n=0; n<vtx; n++)
  {
   glVertex3d(r*cos(2*M_PI*n/vtx),r*sin(2*M_PI*n/vtx),0);
   glVertex3d(r*cos(2*M_PI*(n+1)/vtx),r*sin(2*M_PI*(n+1)/vtx),0);
   glVertex3d(r*cos(2*M_PI*(n+1)/vtx),r*sin(2*M_PI*(n+1)/vtx),h);
   glVertex3d(r*cos(2*M_PI*n/vtx),r*sin(2*M_PI*n/vtx),h);
   glVertex3d(r*cos(2*M_PI*n/vtx),r*sin(2*M_PI*n/vtx),0);
  }
 glEnd();
}
//-------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------ Print ------------------------------------------------------------//
// Print a string at the origin
void Print(char TXT[], void *letter, std::string st)
{
 std::stringstream txt; txt << st;
 strcpy(TXT,(txt.str()).c_str());
 glRasterPos3f(0,0,0);
 for (unsigned int i = 0; i < strlen(TXT); i++){glutBitmapCharacter(letter, TXT[i]);}
}
// Print a string in position (posx,posy,posz)
void Print(char TXT[], void *letter, std::string st, double posx, double posy, double posz)
{
 std::stringstream txt; txt << st;
 strcpy(TXT,(txt.str()).c_str());
 glRasterPos3f(posx,posy,posz);
 for (unsigned int i = 0; i < strlen(TXT); i++){glutBitmapCharacter(letter, TXT[i]);}
}
// Print a string and a value
void Print(char TXT[], void *letter, std::string st, double value, double posx, double posy, double posz)
{
 std::stringstream txt; txt << st << ": "<< value;
 strcpy(TXT,(txt.str()).c_str());
 glRasterPos3f(posx,posy,posz);
 for (unsigned int i = 0; i < strlen(TXT); i++){glutBitmapCharacter(letter, TXT[i]);}
}
// Print an ostream
void Print(char TXT[], void *letter, std::stringstream &txt, double posx, double posy, double posz)
{
 strcpy(TXT,(txt.str()).c_str());
 glRasterPos3f(posx,posy,posz);
 for (unsigned int i = 0; i < strlen(TXT); i++){glutBitmapCharacter(letter, TXT[i]);}
}
//------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------- Draw Zoomer ----------------------------------------------------------//
void Zoomer()
{
 double largo=15, ancho=2;
 int palitos=11;
 const int MAX=2048;
 char TXT[MAX];
 void *rm=GLUT_BITMAP_TIMES_ROMAN_10;
 
 //--- ZOOMER ----//
 glPushMatrix();
 glTranslated(-55,0,0);
  // ZOOM-ARROW UP
  glPushMatrix();
   glColor3f (.75, .75, .75);
   if (Zup) glColor3f (0, 1, 0);
   glTranslated(4*ancho,0,0);
   glRotated(-90,1,0,0);
   glutSolidCone(largo,10*ancho,20,16);
  glPopMatrix();

  // ZOOM-VERTICAL STICKS
  glPushMatrix();
   for(int i=0; i<palitos; ++i)
   {
    glColor3f(0,1,1);
    if (i==5) glColor3f(1,0,0);
    glTranslated(0,-4*ancho,0);
    glPushMatrix();
     glRotated(90,0,1,0);
     glPushMatrix();
      glScaled(ancho,ancho,largo);
      glCallList(cylID);
     glPopMatrix();
    glPopMatrix();
    glPushMatrix();
     glutSolidSphere(ancho,20,16);
     glTranslated(largo,0,0);
     glutSolidSphere(ancho,20,16);
    glPopMatrix();
   }
  glPopMatrix();

  // ZOOM-ARROW DOWN
  glPushMatrix();
   glColor3f (.75, .75, .75);
   if (Zdown) glColor3f (0, 1, 0);
   glTranslated(4*ancho,-(palitos+1)*4*ancho,0);
   glRotated(90,1,0,0);
   glutSolidCone(largo,10*ancho,20,16);
  glPopMatrix();

  // ZOOM-RED BALL
  glPushMatrix();
   glColor3f (1, 0, 0);
   glTranslated(8,-6*4*ancho,0);
   glTranslated(0,markerZ,0);
   glutSolidSphere(8,20,17);
  glPopMatrix();
 
 glPopMatrix();

 //--- ROTATER ----//
 // ROT-ARROW UP
 glPushMatrix();
  glColor3f (.75, .75, .75);
  if (RotUp) glColor3f (0, 1, 0);
  glTranslated(4*ancho,0,0);
  glRotated(-90,1,0,0);
  glutSolidCone(largo,10*ancho,20,16);
 glPopMatrix();

 // ROT-VERTICAL STICKS
 glPushMatrix();
  for(int i=0; i<palitos; i++)
  {
   glColor3f (.75, .75, .75);
   glTranslated(0,-4*ancho,0);
   glPushMatrix();
    glRotated(90,0,1,0);
     glPushMatrix();
      glScaled(ancho,ancho,largo);
      glCallList(cylID);
     glPopMatrix();
   glPopMatrix();
   glPushMatrix();
    glutSolidSphere(ancho,20,16);
    glTranslated(largo,0,0);
    glutSolidSphere(ancho,20,16);
   glPopMatrix();
  }
 glPopMatrix();

 // ROT-ARROW DOWN
 glPushMatrix();
  glColor3f (.75, .75, .75);
  if (RotDown) glColor3f (0, 1, 0);
  glTranslated(4*ancho,-(palitos+1)*4*ancho,0);
  glRotated(90,1,0,0);
  glutSolidCone(largo,10*ancho,20,16);
 glPopMatrix();

 // ROT-RED BALL VERTICAL
 glPushMatrix();
  glColor3f (1, 0, 0);
  glTranslated(8,-6*4*ancho,0);
  glTranslated(0,marker,0);
  glutSolidSphere(8,20,17);
 glPopMatrix();
 
 // ROT-DEGREES ON SCREEN FOR VERTICAL STICKS
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glColor3f (0.0, 0.8, 0);
  
  Print(TXT,rm,"   360",20,-10,0);
  Print(TXT,rm,"   270",20,-20,0);
  Print(TXT,rm,"   180",20,-30,0);
  Print(TXT,rm,"   90", 20,-40,0);
  Print(TXT,rm,"    0", 20,-50,0);
  Print(TXT,rm,"-90",   20,-60,0);
  Print(TXT,rm,"-180",  20,-70,0);
  Print(TXT,rm,"-270",20,-80,0);
  Print(TXT,rm,"-360",20,-90,0);

 glPopAttrib();
 glPopMatrix();
 
 glTranslated(30,-135,0);
 glRotated(-90,0,0,1);

 // ROT-ARROW RIGHT
 glPushMatrix();
  glColor3f (.75, .75, .75);
  if (RotRight) glColor3f (0, 1, 0);
  glTranslated(4*ancho,0,0);
  glRotated(-90,1,0,0);
  glutSolidCone(largo,10*ancho,20,16);
 glPopMatrix();

 // ROT-HORIZONTAL STICKS
 glPushMatrix();
  glColor3f (.75, .75, .75);
  for(int i=0; i<palitos; ++i)
  {
   glTranslated(0,-5*ancho,0);
   glPushMatrix();
    glRotated(90,0,1,0);
     glPushMatrix();
      glScaled(ancho,ancho,largo);
      glCallList(cylID);
     glPopMatrix();
   glPopMatrix();
   glPushMatrix();
    glutSolidSphere(ancho,20,16);
    glTranslated(largo,0,0);
    glutSolidSphere(ancho,20,16);
   glPopMatrix();
  }
 glPopMatrix();
 
 // ROT-ARROW LEFT
 glPushMatrix();
  glColor3f (.75, .75, .75);
  if (RotLeft) glColor3f (0, 1, 0);
  glTranslated(4*ancho,-(palitos+1)*5*ancho,0);
  glRotated(90,1,0,0);
  glutSolidCone(largo,10*ancho,20,16);
 glPopMatrix();

 // ROT-RED BALL HORIZONTAL
 glPushMatrix();
  glColor3f (1, 0, 0);
  glTranslated(8,-6*5*ancho,0);
  glTranslated(0,marker2,0);
  glutSolidSphere(8,20,17);
 glPopMatrix();
 
 // ROT-DEGREES ON SCREEN FOR HORIZONTAL STICKS
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);
 glDisable(GL_LIGHTING);
  glColor3f (0.0, 0.8, 0);

  Print(TXT,rm,"   360",30,-24,0);
  Print(TXT,rm,"   270",-6,-36,0);
  Print(TXT,rm,"   180",30,-48,0);
  Print(TXT,rm,"   90",-6,-58,0);
  Print(TXT,rm,"    0",30,-70,0);
  Print(TXT,rm,"-90",-6,-83,0);
  Print(TXT,rm,"-180",30,-98,0);
  Print(TXT,rm,"-270",-6,-112,0);
  Print(TXT,rm,"-360",30,-125,0);

  glPopAttrib();
 glPopMatrix();

}
//------------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------- InitWIN1 -------------------------------------------------------//
// Initializes global viewing, lighting, and projection values.

void InitWIN1(void) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 GLfloat light_diffuse[]   = {1.0, 1.0, 1.0, 1.0};
 GLfloat light_position1[] = { 1, 1, 1, 0};
 GLfloat light_position2[] = { -1, -1, -1, 0};

 GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
 GLfloat mat_shininess[] = { 50.0 };
 glShadeModel (GL_SMOOTH);
 
 /* Define normal light */
 glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
 glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
 glLightfv(GL_LIGHT0,GL_POSITION,light_position1);
 glLightfv(GL_LIGHT1,GL_POSITION,light_position2);

 /* Define material of atoms */
 glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
 glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

 /* Enable a single OpenGL light */
 if (sseg->quality!=0) glEnable(GL_LIGHTING);
 glEnable(GL_LIGHT0);
 glEnable(GL_LIGHT1);

 /* Use depth buffering for hidden surface elimination */
 glEnable(GL_DEPTH_TEST);
 
 /* Enable the color material mode */
 glEnable(GL_COLOR_MATERIAL);
 
  /* SetCamera(distance) or SetCamera(orth_distance) */
 MoveCamera(0,0);

}
//------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------ Init ------------------------------------------------------------//
// Initializes global viewing, lighting, and projection values.
void Init(void) 
{
 GLfloat light_diffuse[]   = {1.0, 1.0, 1.0, 1.0};
 GLfloat light_position[] = {0, 0, 0, 1};

 GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
 GLfloat mat_shininess[] = { 50.0 };
 glShadeModel (GL_SMOOTH);
 
 /* Define normal light */
 glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
 glLightfv(GL_LIGHT0,GL_POSITION,light_position);

 /* Define material of atoms */
 glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
 glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

 /* Enable a single OpenGL light */
 glEnable(GL_LIGHTING);
 glEnable(GL_LIGHT0);

 /* Use depth buffering for hidden surface elimination */
 glEnable(GL_DEPTH_TEST);

 /* Enable the color material mode */
 glEnable(GL_COLOR_MATERIAL);
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- DrawStatistics -----------------------------------------------------//
// Called by Display1() to draw the view of the current scene.
void DrawStatistics(void) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 const int MAX=2048;
 char TXT[MAX]; 
 void *rm=GLUT_BITMAP_TIMES_ROMAN_10;
 /**************** Writing in the window ****************/
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);

  glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0,sseg->width1,-sseg->height1,0,-10,10);
   glMatrixMode(GL_MODELVIEW);
   GLfloat pos[] = {1.5*sseg->width1, -sseg->height1/2, 10, 0};
   glLightfv(GL_LIGHT0,GL_POSITION,pos);
   
   glPushMatrix();
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glTranslated(0.93*sseg->width1,-0.1*sseg->height1,-5);
    if (wrt)  glCallList(zoomID);
   glPopMatrix();
 
   glDisable(GL_LIGHTING);
   if (wrt)
   {
    std::string fondo=sseg->bg;
    if      (fondo == "white") glColor3f(0,0,0);
    else if (fondo == "gray") glColor3f(1,1,1);
    else glColor3f(0,1,0);

    if (AtomSel) {  Print(TXT,rm,"Atom Selection Enabled",SctAtm,10,-150,0); }
    Print(TXT,rm,"Zoom Speed",zoom_factor,10,-100,0);
    Print(TXT,rm,"Translation Speed",gap,10,-120,0);
   }
   glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double fac=1e-3*fabs(orth_dist);
    if (sseg->persp)   gluPerspective(fovy,aspect,near,far); 
    else glOrtho(-fac*sseg->width1,fac*sseg->width1,-fac*sseg->height1,fac*sseg->height1,-20*far,20*far);
   glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

 glPopAttrib();
 glPopMatrix();

 /********************************************************/
}
//------------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- DrawStatistics2 -----------------------------------------------------//
// Called by Display2() to draw the view of the current scene.
void DrawStatistics2(void) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 const int MAX=2048;
 char TXT[MAX];
 static void *tq=GLUT_BITMAP_HELVETICA_18;
 static void *tr=GLUT_BITMAP_HELVETICA_10;
 /**************** Writing in the window ****************/
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);

  glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0,sseg->width2,-sseg->height2,0,-10,10);
   glMatrixMode(GL_MODELVIEW);
   int sep=12;
   int sep2=15;
   if (Letter == GLUT_BITMAP_TIMES_ROMAN_24){ sep=20; sep2=25;}

   glTranslated(10,-60+move_help,0);
   glTranslated(0,-sep,0); Print(TXT,tq,"HELP MENU:");
   glTranslated(0,-sep,0); Print(TXT,tr,"KEYBOARD OPTIONS:");
   glTranslated(10,-sep,0);Print(TXT,Letter,"  'a' : Hide/show the coordinate axes.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'c' : Hide/show the simulation cell borders.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'f' : FullScreen.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'F' : Reshape the window to its normal size.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'g' : Pressed on the plots window, hide/show the grid.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'i' : Hide/show the zoom and rotations drivers.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'l' : Change the borders of the simulation cell between quads and contours.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'o' : Perspective selection (orthographic/perspective).");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'p' : Pause.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'q' : Exit.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'r' : Undo rotations, translations and zoom (restore the scene).");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  's' : Enable/Disable the Atom Selection Mode (see '+' and '-' keys.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  't' : Enable/Disable auto-rotation of the scene.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'z' : Begin/stop zoom in.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'Z' : Begin/stop zoom out.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  '+' : In Atom Selection Mode, go to next atom; else, increase translation speed.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  '-' : In Atom Selection Mode, go to the previous atom; else, decrease translation speed.");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  '1'->'9' : In Atom Selection Mode, go to the atom typed (for example:'38').");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'CTRL'+'+' : Increase the Zoom speed (zoom in/out faster).");
   glTranslated(0,-sep,0); Print(TXT,Letter,"  'CTRL'+'-' : Decrease the Zoom speed (zoom in/out slower).");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'KeyUp', 'Key Down' : 90 deg. vertical rotations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'KeyLeft', 'KeyRight' : 90 deg. horizontal rotations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'SHIFT'+'KeyUp', 'SHIFT'+KeyDown  : 5 deg. vertical rotations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'SHIFT'+'KeyLeft', 'SHIFT'+'KeyRight' : 5 deg. horizontal rotations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'CTRL'+'KeyUp', 'CTRL'+'KeyDown' : Vertical translations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'CTRL'+'KeyLeft', 'CTRL'+'KeyRight' : Horizontal translations.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'F1' : Open this window.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'F2' : Open the Simulation Data window.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'F3' : Open the plots window.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'RePag' : Changes fonts to next available.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  'AvPag' : Changes fonts to previous available.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"MOUSE MOTION OPTIONS.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Left Button : Rotates the scene.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Right Button : Display menu in any window.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Middle Button : Zoom to the scene.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"MOUSE CLICKING OPTIONS.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Click on the horizontal arrows : Rotates the scene sloyly to the right/left.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Click on the vertical arrows in the corner : Rotates the scene slowly up/down.");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"  Click on the vertical arrows at the left : Zoom in/out.");
   glTranslated(200,-50,0);Print(TXT,Letter,"Designed By:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Sergio Davis:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Joaquin Peralta:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Claudia Loyola:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Felipe Gonzalez:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Pablo Ravelo:");
   glTranslated(0,-sep,0); Print(TXT,Letter,"- Yasmin Navarrete:");
   glTranslated(0,-sep2,0);Print(TXT,Letter,"     www.gnm.cl");

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

 glPopAttrib();
 glPopMatrix();

 /********************************************************/

}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- DrawStatistics 3 ---------------------------------------------------//
// Called by Display3() to draw the view of the current scene.
void DrawStatistics3(void) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 const int MAX=2048;
 char TXT[MAX];
 int sep=12;
 if (Letter == GLUT_BITMAP_TIMES_ROMAN_24) sep=20;
 /**************** Writing in the window ****************/
 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);

  glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0,sseg->width3,-sseg->height3,0,-10,10);
   glMatrixMode(GL_MODELVIEW);
   
   glDisable(GL_LIGHTING);
   glColor3f (0.0, 0.0, 0.0);

    glColor3f(1,0,0); glTranslated(10,-30,0); Print(TXT,GLUT_BITMAP_9_BY_15,"LPVISUAL 2.0",sseg->width3/3.0,0,0);
    glColor3f(0,0,0); glTranslated(0,-30,0); Print(TXT,Letter,"Atoms",(double)sseg->natoms,0,0,0);
    if (!sseg->noprop)
    {
     for (int i=0; i<sseg->numstat; ++i)
     {
      glTranslated(0,-sep,0);
      Print(TXT,Letter,sseg->stname[i],sseg->statistics[i],0,0,0);
     }
    }

    glTranslated(0,-50,0);
    {
     std::stringstream txt; txt << "CameraPosition = (" << eye[0]<<", "<< eye[1] <<", "<< eye[2] << ")";
     Print(TXT,Letter,txt,0,0,0);
    }
    
    glTranslated(0,-20,0);
    {
     std::stringstream txt; txt << "LookingAt      = (" << center[0]<<", "<< center[1] <<", "<< center[2] << ")";
     Print(TXT,Letter,txt,0,0,0);
    }
 
    glTranslated(0,-20,0);
    {
     std::stringstream txt; txt  << "UpDirection    = (" << up[0]<<", "<< up[1] <<", "<< up[2] << ")";
     Print(TXT,Letter,txt,0,0,0);
    }
    
    glTranslated(0,-20,0);
    {
     std::stringstream txt; txt  << "Window Size    =  " << sseg->width1 <<" x "<< sseg->height1;
     Print(TXT,Letter,txt,0,0,0);
    }

    glTranslated(0,-50,0);
    Print(TXT,Letter,"PRESS 'F1' FOR HELP, 'F3' FOR GRAPHICS, 'q' TO QUIT");

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

 glPopAttrib();
 glPopMatrix();

 /********************************************************/


}
//--------------------------------------------------------------------------------------------------------------------//

//-------------------------------------------- Create WIN1 -----------------------------------------------------------//
void CreateWIN1()
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 glutInitWindowSize(sseg->width1, sseg->height1);                     // Specify window size
 glutInitWindowPosition (sseg->width1,0);
 sseg->WIN1 = glutCreateWindow("LPVisual");             // Open window
 InitWIN1();
 if (!sseg->paused) glutSetWindowTitle("LPVisual");
 else glutSetWindowTitle("LPVisual (paused)");
 /* Set a glut callback functions */
 glutDisplayFunc(Display1);
 glutReshapeFunc(Reshape);
 glutIdleFunc(CheckUpdates);
 glutIgnoreKeyRepeat(1);
 glutKeyboardFunc(CheckKeyboard);
 glutSpecialFunc(CheckDirectionalKeys);
 glutMouseFunc(CheckMouse);
 glutMotionFunc(CheckMouseMove);
 /* Creating Main Menu */
 glutCreateMenu(MainMenu);
 glutAddMenuEntry("Simulation Data", OPEN_WIN3);
 glutAddMenuEntry("Graphics", OPEN_WIN4);
 glutAddMenuEntry("About LPVISUAL 0.6.1post", OPEN_WIN2);
 glutAddMenuEntry("Exit", EXIT);
 glutAttachMenu(GLUT_RIGHT_BUTTON);
 sseg->liveWIN1 = true;
}
//--------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------- Create WIN2 -----------------------------------------------------------//
void CreateWIN2()
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 glutInitWindowSize(sseg->width2, sseg->height2);
 glutInitWindowPosition(0, 0);
 sseg->WIN2 = glutCreateWindow("LPVISUAL 2.0 for LPMD 0.6.1post");
 Init();                                   // Initialize view
 // Set a glut callback functions //
 glutDisplayFunc(Display2);
 glutReshapeFunc(Reshape);
 glutKeyboardFunc(CheckKeyboard);
 glutSpecialFunc(CheckDirectionalKeys);
 glutMouseFunc(CheckMouse);
 glutMotionFunc(CheckMouseMove);
 //- Creating Menu WIN2 -//
 glutCreateMenu(MenuWindow2);
 glutAddMenuEntry("Close", HIDE2);
 glutAttachMenu (GLUT_RIGHT_BUTTON);
 sseg->liveWIN2 = true;
}
//--------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------- Create WIN3 -----------------------------------------------------------//
void CreateWIN3()
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 glutInitWindowSize(sseg->width3, sseg->height3);
 glutInitWindowPosition(sseg->width1-1.1*sseg->width3, 0);
 sseg->WIN3 = glutCreateWindow("Simulation Data");
 Init();                                   // Initialize view
 // Set a glut callback functions //
 glutDisplayFunc(Display3);
 glutReshapeFunc(Reshape);
 glutIgnoreKeyRepeat(1);
 glutKeyboardFunc(CheckKeyboard);
 glutSpecialFunc(CheckDirectionalKeys);
 glutMouseFunc(CheckMouse);
 glutMotionFunc(CheckMouseMove);
 //- Creating Menu WIN3 -//
 glutCreateMenu(MenuWindow3);
 glutAddMenuEntry("Close", HIDE3);
 glutAttachMenu (GLUT_RIGHT_BUTTON);
 sseg->liveWIN3 = true;
}

//--------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------- Create WIN4 -----------------------------------------------------------//
void CreateWIN4()
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 glutInitWindowSize(sseg->width4, sseg->height4);
 glutInitWindowPosition(0, 1.2*sseg->height3);
 sseg->WIN4 = glutCreateWindow("Graphics");
 Init();                                   // Initialize view
 // Set a glut callback functions //
 glutDisplayFunc(Display4);
 glutReshapeFunc(Reshape);
 glutIgnoreKeyRepeat(1);
 glutKeyboardFunc(CheckKeyboard);
 glutSpecialFunc(CheckDirectionalKeys);
 glutMouseFunc(CheckMouse);
 glutMotionFunc(CheckMouseMove);
 //- Creating Menu WIN4 -//
 glutCreateMenu(MenuWindow4);
 glutAddMenuEntry("Grid", GRID4);
 glutAddMenuEntry("Close", HIDE4);
 glutAttachMenu (GLUT_RIGHT_BUTTON);
 sseg->liveWIN4 = true;
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Main Menu ----------------------------------------------------------//
// Callback for making the main window menu. It is called by for glutCreateMenu()
void MainMenu(int opcion)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 switch(opcion)
 {
  case OPEN_WIN2:
  {
   if (!sseg->liveWIN2) CreateWIN2();
   else
   {
    glutSetWindow(sseg->WIN2);
    glutShowWindow();
    glutSetWindow(sseg->WIN1);
    glutShowWindow();
   }
  }
  break;
  case OPEN_WIN3:
  {
   if (!sseg->liveWIN3) CreateWIN3();
   else
   {
    glutSetWindow(sseg->WIN3);
    glutShowWindow();
    glutSetWindow(sseg->WIN1);
    glutShowWindow();
   }
  }
  break;
  case OPEN_WIN4:
  {
   if (!sseg->liveWIN4) CreateWIN4();
   else
   {
    glutSetWindow(sseg->WIN4);
    glutShowWindow();
    glutSetWindow(sseg->WIN1);
    glutShowWindow();
   } 
  }
  break;
  case EXIT:                      //Cierra el Programa
  {
   exit(0);
  }
   break;
 }
 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Menu Window 2 ------------------------------------------------------//
// Callback for making the secondary window menu. It is called by for glutCreateMenu()
void MenuWindow2(int opcion)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 switch(opcion)
 {
  case HIDE2:
  {
   glutHideWindow();
   glutSetWindow(sseg->WIN1);
   glutShowWindow();
  }
  break;
 }
 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Menu Window 3 ------------------------------------------------------//
// Callback for making the statistics window menu. It is called by for glutCreateMenu()
void MenuWindow3(int opcion)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 switch(opcion)
 {
  case HIDE3:
  {
   glutHideWindow();
   glutSetWindow(sseg->WIN1);
   glutShowWindow();
  }
  break;
 }
 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Menu Window 4 ------------------------------------------------------//
// Callback for making the graphics window menu. It is called by for glutCreateMenu()
void MenuWindow4(int opcion)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 switch(opcion)
 {
  case HIDE4:
  {
   glutHideWindow();
   glutSetWindow(sseg->WIN1);
   glutShowWindow();
  }
  break;
  case GRID4:
  {
   grid=!grid;
  }
  break;

 }
 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//

/***********************************************************************************************************************/







/****************************************** GLUT CALLBACKS FUNCTIONS *************************************************/
//----------------------------------------------- Reshape -----------------------------------------------------------//
// Callback for glutReshapeFunc()
void Reshape(int w, int h) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 if (glutGetWindow()==sseg->WIN1)
 {
  sseg->width1=w;
  sseg->height1=h;
  // set the GL viewport to match the full size of the window
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  aspect = w/(float)h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double fac=1e-3*fabs(orth_dist);
  // we math the aspect ratio (w/h) with the viewport ratio in both perspectives
  if (sseg->persp) gluPerspective(fovy,aspect,near,far);
  else glOrtho(-fac*sseg->width1,fac*sseg->width1,-fac*sseg->height1,fac*sseg->height1,-20*far,20*far);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
 }
 else if(glutGetWindow() == sseg->WIN2)
 {
  sseg->width2=w;
  sseg->height2=h;
  // set the GL viewport to match the full size of the window
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-w,w,-h,h,-w,w);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Display2();
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if(glutGetWindow() == sseg->WIN3)
 {
  sseg->width3=w;
  sseg->height3=h;
  // set the GL viewport to match the full size of the window
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-w,w,-h,h,-w,w);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Display3();
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if(glutGetWindow() == sseg->WIN4)
 {
  sseg->width4=w;
  sseg->height4=h;
  // set the GL viewport to match the full size of the window
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-w,w,-h,h,-w,w);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Display4();
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }

}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- CheckUpdates ------------------------------------------------------//
// Callback for Checking updates on atom positions
void CheckUpdates(void)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 // EVENTS THAT MUST BE ALWAYS UPDATED
 //----- Keyboard zoom ------//
 if (z_held_down)
 {
  if (sseg->persp){ distance-= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*distance+40; }
  else            { orth_dist-= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*orth_dist+40; }
  MoveCamera(0,0);
 }
 else if (Z_held_down)
 {
  if (sseg->persp){ distance+= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*distance+40; }
  else            { orth_dist+= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*orth_dist+40; }
  MoveCamera(0,0);
 }
 else if (u_arrow_down){  ty-=0.01*gap*diagonal; MoveCamera(0,0); }
 else if (d_arrow_down){  ty+=0.01*gap*diagonal; MoveCamera(0,0); }
 else if (r_arrow_down){  tx-=0.01*gap*diagonal; MoveCamera(0,0); }
 else if (l_arrow_down){  tx+=0.01*gap*diagonal; MoveCamera(0,0); }

 //----- Autorotate --------//
 if (sseg->autorot) MoveCamera(-5,0);

 // Avoid selecting an atom out of range
 if (SctAtm >= (sseg->natoms) || SctAtm<0){ SctAtm=0; SA="";}

 //------- Zoomer -------//
 // Increase vertical angle
 if (RotUp && marker<40)
 {
  marker+=dm;
  MoveCamera(0,-5);
  (sseg->disp)->MakeZoomer(zoomID);
 }
 // Decrease vertical angle
 else if (RotDown && marker>-40)
 {
  marker-=dm;
  MoveCamera(0,5);
  (sseg->disp)->MakeZoomer(zoomID);  
 }
 // Increase horizontal angle
 else if (RotRight && marker2<50-dm2)
 {
  marker2+=dm2;
  MoveCamera(5,0);
  (sseg->disp)->MakeZoomer(zoomID);  
 }
 // Decrease horizontal angle
 else if (RotLeft && marker2>-50+dm2)
 {
  marker2-=dm2;
  MoveCamera(-5,0);
  (sseg->disp)->MakeZoomer(zoomID);  
 }
 else if (Zup)
 {
  // markerZ(distance=0)=40, markerZ(distance=diagonal)=0
  if(sseg->persp){ distance -= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*distance+40;}
  // markerZ(10)=0, markerZ(0.7)=40
  else     { orth_dist-= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*orth_dist+40; }
  MoveCamera(0,0);
  (sseg->disp)->MakeZoomer(zoomID);  
 }
 else if (Zdown)
 {
  if(sseg->persp){ distance += 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*distance+40;}
  else     { orth_dist+= 0.05*diagonal*zoom_factor; markerZ=-40.0/diagonal*orth_dist+40; }
  MoveCamera(0,0);
  (sseg->disp)->MakeZoomer(zoomID);  
 }
 if (marker>40) marker=40;
 if (marker<-40) marker=-40;
 if (marker2>50) marker2=50;
 if (marker2<-50) marker2=-50;
 if (markerZ>40) markerZ=40;
 if (markerZ<-40) markerZ=-40;


 // MOLECULAR DYNAMICS UPDATING
 if (sseg->mustUpdate)
 {
  sseg->updating = true;
  float * coordbuffer = (float *)((char *)(shpointer)+sizeof(SharedSegment));
  for (long i=0; i<(sseg->natoms); ++i) 
   for (int q=0;q<3;++q)
   {
    atoms[i].crd[q] = coordbuffer[i*3+q];
   }
 
  // If the cell gets bigger, update de diagonal, then, center and the camera
  for (int q=0;q<3;++q) diag[q] = sseg->cell[0][q]+sseg->cell[1][q]+sseg->cell[2][q];
  MoveCamera(0,0);

  // Calculate the diagonal module again
  diagonal = sqrt(diag[0]*diag[0]+diag[1]*diag[1]+diag[2]*diag[2]);

  // Redraw the scene
  (sseg->disp)->makeCurframeGeom();
  sseg->updating = false;
  sseg->mustUpdate = false;

  // Change window title when paused
  if (sseg->liveWIN1)
  {
   if (sseg->paused)
   {
    glutSetWindow(sseg->WIN1);
    glutSetWindowTitle(("LPVisual: step "+ToString<int>(sseg->mdstep)+" (paused)").c_str());
   }
   else
   {
    glutSetWindow(sseg->WIN1);
    glutSetWindowTitle(("LPVisual: step "+ToString<int>(sseg->mdstep)).c_str());
   }
  }
 }
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- CheckKeyboard ------------------------------------------------------//
// Callback for reading keyboard events
void CheckKeyboard(unsigned char key, int x, int y)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 if (glutGetWindow() == sseg->WIN1)
 {
  switch(key)
  {
   // ATOM SELECTION
   case '+':
   {
    if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     if (zoom_factor>1)zoom_factor++;
     else zoom_factor*=2.0;
    }
    else if (AtomSel)
    {
     SctAtm++;
     if (SctAtm==(sseg->natoms)){ SctAtm=0;}
     std::ostringstream o; o << SctAtm; SA=o.str();
    }
    else
    {
     if (gap>1) gap++;
     else gap*=2.0;
    }
    (sseg->disp)->makeCurframeGeom();
   }
    break;
   // ATOM SELECTION
   case '-':
   {
    if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     if (zoom_factor>1) zoom_factor--;
     else zoom_factor*=0.5;
    }
    else if (AtomSel)
    {
     SctAtm--;
     if (SctAtm==-1){ SctAtm=(sseg->natoms-1);}
     std::ostringstream o; o << SctAtm; SA=o.str();
    }
    else
    {
     if (gap>1) gap--;
     else gap*=0.5;
    }
    (sseg->disp)->makeCurframeGeom();
   }
    break;
   // ATOM SELECTION
   case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
   {
    if (AtomSel) { SA+=key; std::istringstream i(SA); i>>SctAtm; }
    (sseg->disp)->makeCurframeGeom();
   }
    break;
   // AXIS OF THE REFERENCE FRAME 
   case 'a': {axis=!axis; (sseg->disp)->makeCurframeGeom();}
	     break;
   // TYPE OF REFERENCE FRAME (lines <--> quads)
   case 'c': {RefFrame=!RefFrame; (sseg->disp)->makeCurframeGeom();}
	     break;
   // FULLSCREEN
   case 'f': glutFullScreen();
	     break;
   // FULLSCREEN
   case'F': {glutReshapeWindow(640,480); glutShowWindow(); glutPositionWindow(640,0);}
	    break;
   // STATISTICS ON THE WINDOW
   case 'i': wrt=!wrt;
	     break;
   // REFERENCE FRAME (none <--> some)
   case 'l': {Lines_Or_Quads=!Lines_Or_Quads; (sseg->disp)->makeCurframeGeom();}
	     break;
   // PERSPECTIVE SELECTION
   case 'o':
   {
    if(sseg->persp)
    {
     SetCamera(orth_dist);
     markerZ=-40.0/diagonal*orth_dist+40;
    }
    else
    {
     SetCamera(distance);
     markerZ=-40.0/diagonal*distance+40;
    }
    sseg->persp=!sseg->persp;
    MoveCamera(0,0);
    Reshape(sseg->width1,sseg->height1);
   }
    break;
   // PAUSE
   case 'p': case ' ':
    { 
     sseg->paused = ! (sseg->paused);
     z_held_down=false;
     Z_held_down=false;
     u_arrow_down=false;
     d_arrow_down=false;
     r_arrow_down=false;
     l_arrow_down=false;
    }
    break;
   // QUIT
   case 'q':
   {
    sseg->liveWIN1 = false;
    glutDestroyWindow(sseg->WIN1);
    if (sseg->liveWIN2) glutDestroyWindow(sseg->WIN2);
    if (sseg->liveWIN3) glutDestroyWindow(sseg->WIN3);
    if (sseg->liveWIN4) glutDestroyWindow(sseg->WIN4);
   }
    break;
   // RESTORE THE SCENE
   case 'r':
   {
    tx = 0;
    ty = 0;
    z_held_down=false;
    Z_held_down=false;
    u_arrow_down=false;
    d_arrow_down=false;
    r_arrow_down=false;
    l_arrow_down=false;
    point=false;
    sseg->autorot=false;
    distance = diagonal;
    orth_dist = diagonal;

    SetCamera(0,0);
   }
    break;
   // ENABLE ATOM SELECTION
   case 's': {AtomSel=!AtomSel; (sseg->disp)->makeCurframeGeom();}
	     break;
   // AUTO-ROTATE
   case 't': sseg->autorot=!sseg->autorot;
	     break;
   // UNZOOM
   case 'z': { z_held_down=!z_held_down; Z_held_down=false;}
	     break;
   // ZOOM
   case 'Z': {Z_held_down=!Z_held_down; z_held_down=false;}
	     break;
  }
 }
 else if(glutGetWindow() == sseg->WIN2)
 {
  switch (key)
  {
   case 'q': glutHideWindow();
    break;
   case 'p': case ' ': sseg->paused = ! (sseg->paused);
    break;
  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if(glutGetWindow() == sseg->WIN3)
 {
  switch (key)
  {
   case 'q': glutHideWindow();
    break;
   case 'p': case ' ': sseg->paused = ! (sseg->paused);
    break;
  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if(glutGetWindow() == sseg->WIN4)
 {
  switch (key)
  {
   case 'q': glutHideWindow();
    break;
   case 'p': case ' ': sseg->paused = ! (sseg->paused);
    break;
   case 'g' : grid = ! grid;
    break;
  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }

}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- CheckDirectionalKeys -----------------------------------------------//
// Callback for reading keyboard directional keys' events
void CheckDirectionalKeys(int key, int x, int y)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
  
 if (glutGetWindow() == sseg->WIN1)
 {
  switch(key)
  {
   case GLUT_KEY_F1:
   {
    if (!sseg->liveWIN2) CreateWIN2();
    else
    {
     glutSetWindow(sseg->WIN2);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F2:
   {
    if (!sseg->liveWIN3) CreateWIN3();
    else
    {
     glutSetWindow(sseg->WIN3);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F3:
   {
    if (!sseg->liveWIN4) CreateWIN4();
    else
    {
     glutSetWindow(sseg->WIN4);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_UP:
   {
    float deg=0;
    if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) deg=5;
    else if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     u_arrow_down=!u_arrow_down;
     d_arrow_down=false;
     r_arrow_down=false;
     l_arrow_down=false;
    }
    else deg=90;
 
    for (int i=0; i<deg/5; ++i)
    {
     MoveCamera(0,-5);
     if(i%vel==0) Display1();
    }
   }
    break;
   case GLUT_KEY_DOWN:
   {
    float deg=0;
    if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) deg=5;
    else if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     d_arrow_down=!d_arrow_down;
     u_arrow_down=false;
     r_arrow_down=false;
     l_arrow_down=false;
    }
    else deg=90;
 
    for (int i=0; i<deg/5; ++i)
    {
     MoveCamera(0,5);
     if(i%vel==0) Display1();
    }
   }
    break;
   case GLUT_KEY_RIGHT:
   {
    float deg=0;
    if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) deg=5;
    else if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     r_arrow_down=!r_arrow_down;
     u_arrow_down=false;
     d_arrow_down=false;
     l_arrow_down=false;
    }
    else deg=90;

    for (int i=0; i<deg/5; ++i)
    {
     MoveCamera(5,0);
     if(i%vel==0) Display1();
    }
  
   }
    break;
   case GLUT_KEY_LEFT:
   {
    float deg=0;
    if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) deg=5;
    else if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
    {
     l_arrow_down=!l_arrow_down;
     u_arrow_down=false;
     d_arrow_down=false;
     r_arrow_down=false;
    }
    else deg=90;
 
    for (int i=0; i<deg/5; ++i)
    {
     MoveCamera(-5,0);
     if(i%vel==0) Display1();
    }
   }
    break;
   case GLUT_KEY_PAGE_UP:
   {
    op++;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=B9_BY_15; Letter=GLUT_BITMAP_9_BY_15;}
      break;
    }
   }
    break;
   case GLUT_KEY_PAGE_DOWN:
   {
    op--;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=HELVETICA_18; Letter=GLUT_BITMAP_HELVETICA_18;}
      break;
    }
   }
    break;
   default:
    break;
  }
 }
 //---------- WINDOW 2 (Help) -----------//
 else if (glutGetWindow() == sseg->WIN2)
 {
  switch(key)
  {
   case GLUT_KEY_F1:
   {
    if (!sseg->liveWIN2) CreateWIN2();
    else
    {
     glutSetWindow(sseg->WIN2);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
   case GLUT_KEY_F2:
   {
    if (!sseg->liveWIN3) CreateWIN3();
    else
    {
     glutSetWindow(sseg->WIN3);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F3:
   {
    if (!sseg->liveWIN4) CreateWIN4();
    else
    {
     glutSetWindow(sseg->WIN4);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_UP:
   {
    move_help-=10;
    if (move_help<0) move_help=0;
   }
    break;
   case GLUT_KEY_DOWN: move_help+=10;
    break;
   case GLUT_KEY_PAGE_UP:
   {
    op++;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=B9_BY_15; Letter=GLUT_BITMAP_9_BY_15;}
      break;
    }
   }
    break;
   case GLUT_KEY_PAGE_DOWN:
   {
    op--;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=HELVETICA_18; Letter=GLUT_BITMAP_HELVETICA_18;}
      break;
    }
   }
    break;

  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 //-------- WINDOW 3 (Statistics) -------//
 else if (glutGetWindow() == sseg->WIN3)
 {
  switch(key)
  {
   case GLUT_KEY_F1:
   {
    if (!sseg->liveWIN2) CreateWIN2();
    else
    {
     glutSetWindow(sseg->WIN2);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F2:
   {
    if (!sseg->liveWIN3) CreateWIN3();
    else
    {
     glutSetWindow(sseg->WIN3);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F3:
   {
    if (!sseg->liveWIN4) CreateWIN4();
    else
    {
     glutSetWindow(sseg->WIN4);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_PAGE_UP:
   {
    op++;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=B9_BY_15; Letter=GLUT_BITMAP_9_BY_15;}
      break;
    }
   }
    break;
   case GLUT_KEY_PAGE_DOWN:
   {
    op--;
    switch(op)
    {
     case B9_BY_15:       Letter=GLUT_BITMAP_9_BY_15;
      break;
     case B8_BY_13:       Letter=GLUT_BITMAP_8_BY_13;
      break;
     case TIMES_ROMAN_10: Letter=GLUT_BITMAP_TIMES_ROMAN_10;
      break;
     case TIMES_ROMAN_24: Letter=GLUT_BITMAP_TIMES_ROMAN_24;
      break;
     case HELVETICA_10:   Letter=GLUT_BITMAP_HELVETICA_10;
      break;
     case HELVETICA_12:   Letter=GLUT_BITMAP_HELVETICA_12;
      break;
     case HELVETICA_18:   Letter=GLUT_BITMAP_HELVETICA_18;
      break;
     default:{op=HELVETICA_18; Letter=GLUT_BITMAP_HELVETICA_18;}
      break;
    }
   }
    break;

  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 //-------- WINDOW 4 (Graphics) -------//
 else if (glutGetWindow() == sseg->WIN4)
 {
  switch(key)
  {
   case GLUT_KEY_F1:
   {
    if (!sseg->liveWIN2) CreateWIN2();
    else
    {
     glutSetWindow(sseg->WIN2);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F2:
   {
    if (!sseg->liveWIN3) CreateWIN3();
    else
    {
     glutSetWindow(sseg->WIN3);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
   case GLUT_KEY_F3:
   {
    if (!sseg->liveWIN4) CreateWIN4();
    else
    {
     glutSetWindow(sseg->WIN4);
     glutShowWindow();
     glutSetWindow(sseg->WIN1);
     glutShowWindow();
    }
   }
    break;
  }
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }

 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- CheckMouse ---------------------------------------------------------//
// Callback for reading mouse clicking events. It calls for glutMouseFunc()
void CheckMouse(int button, int state, int x, int y)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 mousecoord[0] = x;
 mousecoord[1] = y;
 switch(button)
 {
  case GLUT_LEFT_BUTTON:
   Buttons[0] = (state==GLUT_DOWN)?1:0;
   if (state==GLUT_UP)
   {
    RotUp=false; RotDown=false; RotLeft=false; RotRight=false; Zup=false; Zdown=false;
    (sseg->disp)->MakeZoomer(zoomID);
   }
   break;
  case GLUT_MIDDLE_BUTTON:
   Buttons[1] = (state==GLUT_DOWN)?1:0;
   break;
  case GLUT_RIGHT_BUTTON:
   Buttons[2] = (state==GLUT_DOWN)?1:0;
   break;
  default:
   break;
 }
 mousecoord[0] = x;
 mousecoord[1] = y;

 if (glutGetWindow() == sseg->WIN1)
 {
  // MANAGE THE ZOOMER
  if(Buttons[0])
  {
   gluUnProject(mousecoord[0],sseg->height1-mousecoord[1],0,model_view,projection,viewport,&pos3D_x,&pos3D_y,&pos3D_z);
   double orgX=590*sseg->width1/640, orgY=-50*sseg->height1/480;
   // If you press the top arrow, shine it
   if (pos3D_x>orgX && pos3D_x<orgX+30 && pos3D_y>orgY && pos3D_y<orgY+22) RotUp=true;
   // If you press the bottom arrow, shine it
   else if (pos3D_x>orgX && pos3D_x<orgX+30 && pos3D_y>orgY-115 && pos3D_y<orgY-115+22) RotDown=true;
   // If you press the left arrow, shine it
   else if (pos3D_x>orgX-103 && pos3D_x<orgX-103+22 && pos3D_y>orgY-155 && pos3D_y<orgY-155+30) RotLeft=true;
   // If you press the right arrow, shine it
   else if (pos3D_x>orgX-103+137 && pos3D_x<orgX-103+22+137 && pos3D_y>orgY-155 && pos3D_y<orgY-155+30) RotRight=true;
   // If you press the top arrow of the zoom, shine it
   else if (pos3D_x>orgX-60 && pos3D_x<orgX-30 && pos3D_y>orgY && pos3D_y<orgY+22) Zup=true;
   // If you press the bottom arrow of the zoom, shine it
   else if (pos3D_x>orgX-60 && pos3D_x<orgX-30 && pos3D_y>orgY-115 && pos3D_y<orgY-115+22) Zdown=true;
   // If you press the angles (vertical text), change to that angle
   else if (pos3D_x>orgX+30 && pos3D_x<orgX+50 && pos3D_y>orgY-90 && pos3D_y<orgY)
   {
    if      (pos3D_y<orgY && pos3D_y>orgY-10)    marker=40;
    else if (pos3D_y<orgY-10 && pos3D_y>orgY-20) marker=30;
    else if (pos3D_y<orgY-20 && pos3D_y>orgY-30) marker=20;
    else if (pos3D_y<orgY-30 && pos3D_y>orgY-40) marker=10;
    else if (pos3D_y<orgY-40 && pos3D_y>orgY-50) marker=0;
    else if (pos3D_y<orgY-50 && pos3D_y>orgY-60) marker=-10;
    else if (pos3D_y<orgY-60 && pos3D_y>orgY-70) marker=-20;
    else if (pos3D_y<orgY-70 && pos3D_y>orgY-80) marker=-30;
    else if (pos3D_y<orgY-80 && pos3D_y>orgY-90) marker=-40;
    else return; // exit CheckMouse if you didn't hit the text

    // Depending on the angle (verticals) you pressed, rotate the scene that angle
    if (marker>=0 && pos3D_y>orgY-50)
    {
     for (int i=0; i<marker/dm; ++i)
     {
      MoveCamera(0,5);
      if(i%vel==0) Display1();
     }
    }
    else if (marker<=0 && pos3D_y<orgY-50)
    {
     for (int i=0; i>marker/dm; --i)
     {
      MoveCamera(0,-5);
      if (i%vel==0) Display1();
     }
    }
   }
   // If you press the angles (horizontal text), change to that angle
   else if (pos3D_x>orgX-90 && pos3D_x<orgX+60 && pos3D_y>orgY-165 && pos3D_y<orgY-115)
   {
    if      (pos3D_x>orgX-87 && pos3D_x<orgX-64 && pos3D_y>orgY-165 && pos3D_y<orgY-150) marker2=-50;
    else if (pos3D_x>orgX-75 && pos3D_x<orgX-50 && pos3D_y>orgY-130 && pos3D_y<orgY-115) marker2=-37.5;
    else if (pos3D_x>orgX-62 && pos3D_x<orgX-39 && pos3D_y>orgY-165 && pos3D_y<orgY-150) marker2=-25;
    else if (pos3D_x>orgX-48 && pos3D_x<orgX-26 && pos3D_y>orgY-130 && pos3D_y<orgY-115) marker2=-12.5;
    else if (pos3D_x>orgX-28 && pos3D_x<orgX-16 && pos3D_y>orgY-165 && pos3D_y<orgY-150) marker2=0;
    else if (pos3D_x>orgX-18 && pos3D_x<orgX-3 && pos3D_y>orgY-130 && pos3D_y<orgY-115)  marker2=12.5;
    else if (pos3D_x>orgX-7 && pos3D_x<orgX+13 && pos3D_y>orgY-165 && pos3D_y<orgY-150)  marker2=25;
    else if (pos3D_x>orgX+5 && pos3D_x<orgX+25 && pos3D_y>orgY-130 && pos3D_y<orgY-115)  marker2=37.5;
    else if (pos3D_x>orgX+16 && pos3D_x<orgX+36 && pos3D_y>orgY-165 && pos3D_y<orgY-150) marker2=50;
    else return; // exit CheckMouse if you didn't hit the text
    
  // Depending on the angle (horozontals) you pressed, rotate the scene that angle
    if (marker2>=0 && pos3D_x>orgX-22)
    {
     for (int i=0; i<marker2/dm2-dm2; ++i)
     {
      MoveCamera(5,0);
      if (i%vel==0) Display1();
     }
    }
    else if (marker2<=0 && pos3D_x<orgX-22)
    {
     for (int i=0; i>marker2/dm2+dm2; --i)
     {
      MoveCamera(-5,0);
      if (i%vel==0) Display1();
     }
    }
   }
   // If you press the center of the zoomer, make markerZ=0
   else if (pos3D_x>orgX-50 && pos3D_x<orgX-35 && pos3D_y>orgY-50 && pos3D_y<orgY-40)
   {
    markerZ=0;
    if(sseg->persp) distance = sqrt(diag[0]*diag[0]+diag[1]*diag[1]+diag[2]*diag[2]);
    else orth_dist = sqrt(diag[0]*diag[0]+diag[1]*diag[1]+diag[2]*diag[2]);;
    MoveCamera(0,0);
   }
   // In any case, redraw the zoomer
   (sseg->disp)->MakeZoomer(zoomID);
  }
  else if (!Buttons[0]){ RotUp=false; RotDown=false; RotLeft=false; RotRight=false; Zup=false; Zdown=false;}
 }
 else if (glutGetWindow() == sseg->WIN2)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if (glutGetWindow() == sseg->WIN3)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if (glutGetWindow() == sseg->WIN4)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }

 glutPostRedisplay(); 
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- CheckMouseMove -----------------------------------------------------//
// Callback for reading mouse movement events. It calls for glutMotionFunc()
void CheckMouseMove(int x, int y)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);
 
 // drag and drop
 int dx = x-mousecoord[0];
 int dy = y-mousecoord[1];
 mousecoord[0] = x;
 mousecoord[1] = y;
 if (glutGetWindow() == sseg->WIN1)
 {
  // MOUSE MIDDLE-BUTTON: Zoom control
  if( Buttons[1] )
  {
   if(sseg->persp)
   {
    distance -= 0.005*dx*diagonal*zoom_factor;
    markerZ=-40.0/diagonal*distance+40;
    if (markerZ>40) markerZ=40;
    else if (markerZ<-40) markerZ=-40;
   }
   else	
   {
    orth_dist -= 0.005*dx*diagonal*zoom_factor;
    markerZ=-40.0/diagonal*orth_dist+40;
    if (markerZ>40) markerZ=40;
    else if (markerZ<-40) markerZ=-40;
   }
   MoveCamera(0,0);
  }
  // MOUSE LEFT-BUTTON: Rotations control
  else if( Buttons[0] )
  {
   MoveCamera(-0.3*dx,-0.3*dy);
  }
  // MOUSE RIGHT-BUTTON: Do nothing (just display menu)
  else if( Buttons[2] )
  {
   glutSetWindow(sseg->WIN1);
   glutShowWindow();
  }
 }
 else if (glutGetWindow() == sseg->WIN2)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if (glutGetWindow() == sseg->WIN3)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 else if (glutGetWindow() == sseg->WIN4)
 {
  glutSetWindow(sseg->WIN1);
  glutShowWindow();
 }
 glutPostRedisplay();
}
//--------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Display1 -----------------------------------------------------------//
// Callback for glutDisplayFunc().  It clears the frame and depth buffers and draws the atoms in the current frame.
void Display1(void) 
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 std::string fondo=sseg->bg;
 if      (fondo == "white") glClearColor(1.0,1.0,1.0,0.0);
 else if (fondo == "gray") glClearColor(0.35,0.35,0.35,0.0);
 else if (fondo == "orange") glClearColor(1.0,0.27,0.0,0.0);
 else glClearColor(0.0,0.0,0.0,0.0);
 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

 glLoadIdentity();

 glPushMatrix();

 /* Define viewing transformation */
 gluLookAt(
 (GLdouble)eye[0],(GLdouble)eye[1],(GLdouble)eye[2],
 (GLdouble)center[0],(GLdouble)center[1],(GLdouble)center[2],
 (GLdouble)up[0],(GLdouble)up[1],(GLdouble)up[2]);

 glCallList(atomsid);

 glPopMatrix();

 glPushMatrix();
 DrawStatistics(); 
 glPopMatrix();
 
 glutPostRedisplay();
 glutSwapBuffers();
}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Display2 ----------------------------------------------------------//
// Callback for glutDisplayFunc().  It clears the frame and depth buffers and draws the atoms in the current frame.
void Display2(void)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 glClearColor(1,1,1,1);
 glLoadIdentity();
 
 DrawStatistics2();

 glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,sseg->width2,-sseg->height2,0,-10,10);
  glMatrixMode(GL_MODELVIEW);
 
 double cs=15;
 glPushMatrix();
 
  glColor3d(1,0,0);
  glTranslated(200,-50,-10);
  // The "L"
  glPushMatrix(); glRotatef(-sseg->mdstep,0,1,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,cs,0);   glRotatef(sseg->mdstep,0,1,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,2*cs,0); glRotatef(-sseg->mdstep,0,1,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(cs,0,0);   glRotatef(sseg->mdstep,0,1,0); glutSolidCube(cs); glPopMatrix();
  // The "P"
  glTranslated(3*cs,0,0); glColor3d(0,1,0);
  glPushMatrix(); glRotatef(sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,cs,0);   glRotatef(sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,2*cs,0); glRotatef(sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix();
   glTranslated(cs,1.5*cs,0);
   glRotatef(45,1,1,1);
   glRotatef(-sseg->mdstep,1,0,0);
   glutSolidCube(1.3*cs);
  glPopMatrix();
  // The "M"
  glTranslated(3*cs,0,0); glColor3d(0,0,1);
  glPushMatrix(); glRotatef(-sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,cs,0);   glRotatef(-sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,2*cs,0); glRotatef(-sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix();
   glTranslated(cs,1.5*cs,0);glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,1,0,0);
   glutSolidCube(0.8*cs);
  glPopMatrix();
  glPushMatrix(); glTranslated(2*cs,0,0);  glRotatef(sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(2*cs,cs,0);  glRotatef(sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(2*cs,2*cs,0);  glRotatef(sseg->mdstep,1,0,0); glutSolidCube(cs); glPopMatrix();
  // The "D"
  glTranslated(4*cs,0,0); glColor3d(0.5,0.5,0);
  glPushMatrix(); glRotatef(sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,cs,0);   glRotatef(-sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix(); glTranslated(0,2*cs,0); glRotatef(sseg->mdstep,0.1,1,.1); glutSolidCube(cs); glPopMatrix();
  glPushMatrix();
   glTranslated(0.8*cs,0,0);
   glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,0.1,1,.1);
   glutSolidCube(0.6*cs);
  glPopMatrix();
  glPushMatrix();
   glTranslated(0.8*cs,2*cs,0);
   glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,0.1,1,.1);
   glutSolidCube(0.6*cs);
  glPopMatrix();
  glPushMatrix();
   glTranslated(1.4*cs,0.5*cs,0);
   glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,0.1,1,.1);
   glutSolidCube(0.6*cs);
  glPopMatrix();
  glPushMatrix();
   glTranslated(1.4*cs,1.6*cs,0); 
   glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,0.1,1,.1);
   glutSolidCube(0.6*cs);
  glPopMatrix();
  glPushMatrix();
   glTranslated(2*cs,cs,0);
   glRotatef(45,1,1,1);
   glRotatef(sseg->mdstep,0.1,1,.1);
   glutSolidCube(0.6*cs);
  glPopMatrix();

 glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-sseg->width2,sseg->width2,-sseg->height2,sseg->height2,-sseg->width2,sseg->width2);
   glMatrixMode(GL_MODELVIEW);
  glPopMatrix();




 glutPostRedisplay();

 glutSwapBuffers();
}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Display3 ----------------------------------------------------------//
// Callback for glutDisplayFunc().  It clears the frame and depth buffers and draws the atoms in the current frame.
void Display3(void)
{
// SharedSegment * sseg = (SharedSegment *)(shpointer);

 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 glClearColor(1,1,1,1);
 glLoadIdentity();

 DrawStatistics3();
 
 glutPostRedisplay();
 glutSwapBuffers();
}
//-------------------------------------------------------------------------------------------------------------------//
//----------------------------------------------- Display4 ----------------------------------------------------------//
// Callback for glutDisplayFunc().  It clears the frame and depth buffers and draws the atoms in the current frame.
void Display4(void)
{
 SharedSegment * sseg = (SharedSegment *)(shpointer);

 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 std::string fondo=sseg->gbg;
 if      (fondo == "black") glClearColor(0.0,0.0,0.0,0.0);
 else if (fondo == "gray") glClearColor(0.35,0.35,0.35,0.0);
 else if (fondo == "orange") glClearColor(1.0,0.27,0.0,0.0);
 else glClearColor(1.0,1.0,1.0,0.0);
 glLoadIdentity();


 
 const int MAX=2048;
 char TXT[MAX];

 // Resize x-range and y_range
 if (sseg->numyrange == 0)
 {
  for (int i=0; i<sseg->numplots; ++i)
  {
   if (sseg->data[i][sseg->mdstep] > sseg->yrange[1] || sseg->data[i][sseg->mdstep] < sseg->yrange[0]){ sseg->yrange[0]*=2; sseg->yrange[1]*=2;}
  }
 }

 if (sseg->numxrange == 0)
 {
  if (sseg->mdstep>sseg->xrange[1]) sseg->xrange[1]*=2;
 }

 glPushMatrix();
 glPushAttrib(GL_LIGHTING_BIT);
 glDisable(GL_LIGHTING);
 
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double axisxpos, axisypos;
  double xsep=sseg->xrange[1] - sseg->xrange[0];
  double ysep=sseg->yrange[1] - sseg->yrange[0];
  axisypos = (sseg->xrange[0]>0) ? sseg->xrange[0] : 0;
  if (sseg->yrange[0]*sseg->yrange[1] < 0) axisxpos=0;
  else if (sseg->yrange[1] >  0)           axisxpos=sseg->yrange[0];
  else if (sseg->yrange[1] <= 0)           axisxpos=sseg->yrange[1];

  //++++++++++ WINDOW GEOMETRY +++++++++++//
  glOrtho(sseg->xrange[0]-0.15*xsep, sseg->xrange[1]+0.2*xsep, sseg->yrange[0]-0.2*ysep, sseg->yrange[1]+0.2*ysep,-10,10);
  glMatrixMode(GL_MODELVIEW);

  //++++ ARROWS +++++++//
  glBegin(GL_TRIANGLES);
   glColor3d(0,0,1);
   // Y-Arrow
   glVertex2d(axisypos,sseg->yrange[1]+0.1*ysep);
   glVertex2d(axisypos+0.01*xsep, sseg->yrange[1]+0.05*ysep);
   glVertex2d(axisypos-0.01*xsep, sseg->yrange[1]+0.05*ysep);
   // X-Arrow
   glVertex2d(sseg->xrange[1]+0.1*xsep,  axisxpos);
   glVertex2d(sseg->xrange[1]+0.07*xsep, axisxpos+0.015*ysep);
   glVertex2d(sseg->xrange[1]+0.07*xsep, axisxpos-0.015*ysep);
  glEnd();

  //+++++ AXES ++++++++++//
  glBegin(GL_LINES);
   if (fondo!="black" && fondo!="orange" && fondo!="gray") glColor3f(0,0,0); else glColor3f(0,1,0);
   // X-Axis
   glVertex2d(-sseg->xrange[1],axisxpos);      glVertex2d(sseg->xrange[1]+0.1*xsep, axisxpos);
   // Y-Axis
   if (sseg->yrange[0]*sseg->yrange[1] < 0){ glVertex2d(axisypos,2*sseg->yrange[0]);  glVertex2d(axisypos,sseg->yrange[1]+0.1*ysep);}
   else if (sseg->yrange[1] > 0)           { glVertex2d(axisypos,-sseg->yrange[0]);   glVertex2d(axisypos,sseg->yrange[1]+0.1*ysep);}
   else                                    { glVertex2d(axisypos,2*sseg->yrange[0]);  glVertex2d(axisypos,sseg->yrange[1]+0.1*ysep);}
  glEnd();

  //++++++++ TICKS +++++++++++//
  if (fondo!="black" && fondo!="orange" && fondo!="gray") glColor3f(0,0,0); else glColor3f(0,1,0);
  for (int i=-10; i<=10; ++i)
  {
   // X-Ticks
   glBegin(GL_LINES);
    glVertex2d((i/10.0)*sseg->xrange[1], axisxpos+0.015*ysep);
    glVertex2d((i/10.0)*sseg->xrange[1], axisxpos-0.015*ysep);
   glEnd();
  }
  for (int i=-10; i<=10; ++i)
  {
   // Y-Ticks
   glBegin(GL_LINES);
    glVertex2d(axisypos-0.01*xsep, (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1] );
    glVertex2d(axisypos+0.01*xsep, (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1] );
   glEnd();
  }

  //+++++ LABELS +++++++++++//
  // X-Labels
  for (int i=-10; i<=10; ++i)
  {
   std::stringstream Xtxt;  Xtxt << (i/10.0)*sseg->xrange[1]; 
   Print(TXT,GLUT_BITMAP_HELVETICA_10,Xtxt, (i/10.0)*sseg->xrange[1]-0.02*sseg->xrange[1], axisxpos-0.05*ysep, 0);
  }
  // Y-Labels
  for (int i=-10; i<=10; ++i)
  {
   std::stringstream Ytxt;  Ytxt << (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1]; 
   if ( fabs((i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1]) > 0.001 )
    Print(TXT,GLUT_BITMAP_HELVETICA_10,Ytxt, axisypos-0.08*xsep, (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1], 0);
   else
    Print(TXT,GLUT_BITMAP_HELVETICA_10,"0", -0.08*xsep, 0, 0);
  }

  //++++++++++ AXIS NAME +++++++++++//
  glColor3d(1,0,0);
  // X-Axis Name
  Print(TXT,GLUT_BITMAP_HELVETICA_18,"time",sseg->xrange[1], axisxpos-0.15*ysep,0);
  // Y-Axis Name
  for (int i=0; i<sseg->numplots; ++i)
  {
   Vector clr=ColorFromScalar((double)i/sseg->numplots);
   glColor3d(clr[0],clr[1],clr[2]);
   Print(TXT,GLUT_BITMAP_HELVETICA_18,sseg->ptname[i], axisypos+0.05*xsep, (sseg->yrange[1]+0.1*ysep)-0.08*i*ysep,0);
  }

  //+++++++++++ THE POINTS ++++++++++//
  for (int i=0; i<sseg->numplots; ++i)
  {
   Vector clr=ColorFromScalar((double)i/sseg->numplots);
   glColor3d(clr[0],clr[1],clr[2]);
   glBegin(GL_LINE_STRIP);
    for (int j=sseg->start; j<sseg->mdstep; j+=sseg->each)
    {
     glVertex2d(j,sseg->data[i][j]);
    }
   glEnd();
  }

  //++++++++++ GRID +++++++++++++++//
  glEnable(GL_LINE_STIPPLE);
  if (grid)
  {
   glColor3d(0.5,0.5,0);
   for (int i=1; i<=10; ++i)
   {
    // Vertical lines
    glLineStipple(1,0x00FF);
    glBegin(GL_LINES);
     glVertex2d((i/10.0)*sseg->xrange[1], sseg->yrange[0]);
     glVertex2d((i/10.0)*sseg->xrange[1], sseg->yrange[1]);
    glEnd();
   }
   for (int i=-10; i<=10; ++i)
   {
    // Horizontal lines
    glBegin(GL_LINES);
     glVertex2d(0,               (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1] );
     glVertex2d(sseg->xrange[1], (i-10)*(sseg->yrange[1]-sseg->yrange[0])/20.0+sseg->yrange[1] );
    glEnd();
   }
  }
  glDisable(GL_LINE_STIPPLE);

 glPopAttrib();
 glPopMatrix();



 glutPostRedisplay();
 glutSwapBuffers();
}
//-------------------------------------------------------------------------------------------------------------------//

/***********************************************************************************************************************/
