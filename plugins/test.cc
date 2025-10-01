/*
 *
 *
 *
 */

#include <iostream>
#include <lpmd/array.h>
#include <lpmd/atom.h>
#include <lpmd/timer.h>
#include <lpmd/basicparticleset.h>

#include "test.h"

Test::Test(std::string args): Module("test")
{


}

Test::~Test()
{


}

void Test::PerformTest(Simulation & s)
{
 std::cout << "This is the test from \"test\" plugin!\n";
 Array<Atom> & darray = s.DirectArray();
 BasicParticleSet & iarray = s.IndirectArray();
 Timer t;
 const Atom un_atomo("Cu");
 std::cout << "Append de hartos atomos en arreglo directo\n";
 t.Start();
 for (int k=0;k<100;++k)
 {
  for (long int i=0;i<100000;++i) darray.Append(un_atomo);
  assert(darray.Size() == 100000);
  darray.Clear();
 }
 t.Stop();
 t.ShowElapsedTimes();

 std::cout << "Append de hartos atomos en arreglo indirecto\n";
 t.Start();
 for (int k=0;k<100;++k)
 {
  for (long int i=0;i<100000;++i) iarray.Append(un_atomo);
  assert(iarray.Size() == 100000);
  iarray.Clear();
 }
 t.Stop();
 t.ShowElapsedTimes();

 for (long int i=0;i<100000;++i) darray.Append(un_atomo);
 assert(darray.Size() == 100000);
 std::cout << "Seteo y lectura de posiciones de hartos atomos en arreglo directo\n";
 t.Start();
 for (int k=0;k<100;++k)
 {
  double ss = 0.0;
  for (long int i=0;i<100000;++i)
  { 
   darray[i].Position() = RandomVector(1.0);
   ss += darray[i].Position().Module();
  }
 }
 t.Stop();
 t.ShowElapsedTimes();

 for (long int i=0;i<100000;++i) iarray.Append(un_atomo);
 assert(iarray.Size() == 100000);
 std::cout << "Seteo de posiciones de hartos atomos en arreglo indirecto\n";
 t.Start();
 for (int k=0;k<100;++k)
 {
  double ss = 0.0;
  for (long int i=0;i<100000;++i)
  {
   iarray[i].Position() = RandomVector(1.0);
   ss += iarray[i].Position().Module();
  }
 }
 t.Stop();
 t.ShowElapsedTimes();

}


// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Module * create(std::string args) { return new Test(args); }
void destroy(Module * m) { delete m; }

