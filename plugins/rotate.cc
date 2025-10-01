//
//
//

#include "rotate.h"

#include <lpmd/util.h>

#include <iostream>

using namespace lpmd;

RotateModifier::RotateModifier(std::string args): Plugin("rotate", "2.0")
{
 ParamList & params = (*this);
 DefineKeyword("x", "1.0");
 DefineKeyword("y", "0.0");
 DefineKeyword("z", "0.0");
 DefineKeyword("angle", "0.0");
 // 
 ProcessArguments(args);
 axis = Vector(double(params["x"]), double(params["y"]), double(params["z"]));
 axis.Normalize();
 angle = M_PI*double(params["angle"])/180.0;
 start = int(params["start"]);
 end = int(params["end"]);
 each = int(params["each"]);
 //
 double c=cos(angle), s=sin(angle), t=1.0-c;
 double x=axis[0], y=axis[1], z=axis[2];
 rotmat[0][0] = t*x*x+c;
 rotmat[0][1] = t*x*y+s*z;
 rotmat[0][2] = t*x*z-s*y;
 rotmat[1][0] = t*x*y-s*z;
 rotmat[1][1] = t*y*y+c;
 rotmat[1][2] = t*y*z+s*x;
 rotmat[2][0] = t*x*z+s*y;
 rotmat[2][1] = t*y*z-s*x;
 rotmat[2][2] = t*z*z+c;
}

RotateModifier::~RotateModifier() { }

void RotateModifier::ShowHelp() const
{
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Module Name        = rotate                                                   \n";
 std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Info      >>                                                          \n";
 std::cout << "      This plugin is used to rotate a group of atoms in a given angle about    \n";
 std::cout << "      a given axis of rotation.                                                \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " General Options   >>                                                          \n";
 std::cout << "      x             : Sets the X-coordinate of the rotation axis.              \n";
 std::cout << "      y             : Sets the Y-coordinate of the rotation axis.              \n";
 std::cout << "      z             : Sets the Z-coordinate of the rotation axis.              \n";
 std::cout << "      angle         : Sets the angle in which the sample will be rotated about \n";
 std::cout << "                      the rotation axis.                                       \n";
 std::cout << "      start         : Determines in which step the plugin begins to be applied.\n";
 std::cout << "      end           : Determines in which step the plugin ceases to be applied.\n";
 std::cout << "      each          : Determines how often (each how many time-steps) the      \n";
 std::cout << "                      plugin must be applied.                                  \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
 std::cout << " Example           >>                                                          \n";
 std::cout << " #Loading the plugin :                                                         \n";
 std::cout << " prepare rotate x=0.0 y=0.0 z=1.0 angle=45.0                                 \n\n";
 std::cout << "      The plugin is used to rotate the sample of atoms in the simulation cell  \n";
 std::cout << "      in 45 degrees about the z-axis.                                          \n";
 std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

void RotateVector(double * vect, double * axis, float ang)
{
 double u=axis[0],v=axis[1],w=axis[2];
 double x=vect[0],y=vect[1],z=vect[2];
 double norm2=u*u+v*v+w*w;

 double rv[3];
 rv[0] = u*(u*x+v*y+w*z)+(x*(v*v+w*w)-u*(v*y+w*z))*cos(ang)+sqrt(u*u+v*v+w*w)*(-w*y+v*z)*sin(ang);
 rv[1] = v*(u*x+v*y+w*z)+(y*(u*u+w*w)-v*(u*x+w*z))*cos(ang)+sqrt(u*u+v*v+w*w)*(w*x-u*z)*sin(ang);
 rv[2] = w*(u*x+v*y+w*z)+(z*(u*u+v*v)-w*(u*x+v*y))*cos(ang)+sqrt(u*u+v*v+w*w)*(-v*x+u*y)*sin(ang);
 for (int q=0; q<3; ++q) vect[q]=rv[q]/norm2;
}

void RotateModifier::Apply(Configuration & conf)
{
 BasicParticleSet & atoms = conf.Atoms();
 BasicCell & cell = conf.Cell();
 Vector center = (cell[0]+cell[1]+cell[2])*0.5;
 for (long i=0;i<atoms.Size();++i)
 {
  // translate so that the center of the cell is (0, 0, 0)
  Vector pos = atoms[i].Position() - center; 
  // rotate
  double rv[3], ax[3];
  /*
  // FIXME: Algo pasa con la matriz de rotacion original, rotmat!! la norma no se conserva
  for (int j=0;j<3;++j)
  {
   rv[j] = 0.0;
   for (int k=0;k<3;++k) rv[j] += rotmat[j][k]*pos[k];
   pos[j] = rv[j];
  }
  */
  for (int q=0;q<3;++q) { rv[q] = pos[q]; ax[q] = axis[q]; }
  // La funcion RotateVector de FG en lpvisual 'does its job'
  RotateVector(rv, ax, angle);
  for (int q=0;q<3;++q) pos[q] = rv[q];
  atoms[i].Position() = cell.FittedInside(pos+center);
 }
}

void RotateModifier::Apply(Simulation & md)
{
 Configuration & sc = md;
 Apply(sc);
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin * create(std::string args) { return new RotateModifier(args); }
void destroy(Plugin * m) { delete m; }

