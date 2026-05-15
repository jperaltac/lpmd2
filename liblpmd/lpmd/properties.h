/*
 *
 *
 *
 */

#ifndef __LPMD_PROPERTIES_H__
#define __LPMD_PROPERTIES_H__

#define KIN2EV (103.64269)
#define KBOLTZMANN (8.6173422E-05)

#include <lpmd/configuration.h>
#include <lpmd/matrix.h>
#include <lpmd/potential.h>
#include <lpmd/simulationhistory.h>
#include <lpmd/storedconfiguration.h>
#include <lpmd/util.h>

namespace lpmd {

template <typename T> double KineticEnergy(const T& atomcont, bool tag = false) {
  double K = 0.0;
  if (tag == false) {
    for (long int i = 0; i < atomcont.Size(); ++i)
      K += 0.5 * atomcont[i].Mass() * atomcont[i].Velocity().SquareModule();
  } else {
    for (long int i = 0; i < atomcont.Size(); ++i) {
      if (atomcont.Have(atomcont[i], Tag("fixedvel")) ||
          atomcont.Have(atomcont[i], Tag("fixedpos")))
        continue;
      else
        K += 0.5 * atomcont[i].Mass() * atomcont[i].Velocity().SquareModule();
    }
  }
  return K * KIN2EV;
}

template <typename T> double Temperature(const T& atomcont, const bool tag = false) {
  const long int atom_count = atomcont.Size();
  if (atom_count == 0)
    return 0.0; //

  double kinetic_energy = 0.0;
  long int fixed_count = 0;
  for (long int i = 0; i < atom_count; ++i) {
    if (tag &&
        (atomcont.Have(atomcont[i], Tag("fixedvel")) ||
         atomcont.Have(atomcont[i], Tag("fixedpos")))) {
      ++fixed_count;
      continue;
    }
    kinetic_energy += 0.5 * atomcont[i].Mass() * atomcont[i].Velocity().SquareModule();
  }

  return (2.0 / 3.0) * (kinetic_energy * KIN2EV) /
         (KBOLTZMANN * double(atom_count - fixed_count));
}

template <typename T, typename V> double Density(const T& atomcont, const V& cell) {
  if (atomcont.Size() == 0)
    return 0.0;
  double total_mass = 0.0;
  for (long int i = 0; i < atomcont.Size(); ++i)
    total_mass += atomcont[i].Mass();
  return (total_mass / cell.Volume());
}

template <typename T> Vector Momentum(const T& atomcont, bool tag = false) {
  if (atomcont.Size() == 0)
    return Vector(0.0, 0.0, 0.0);
  Vector p(0.0, 0.0, 0.0);
  if (tag == false) {
    for (long int i = 0; i < atomcont.Size(); ++i)
      p += (atomcont[i].Mass() * atomcont[i].Velocity());
  } else {
    for (long int i = 0; i < atomcont.Size(); ++i) {
      if (atomcont.Have(atomcont[i], Tag("fixedvel")) ||
          atomcont.Have(atomcont[i], Tag("fixedpos")))
        continue;
      else
        p += (atomcont[i].Mass() * atomcont[i].Velocity());
    }
  }
  return p;
}

void gdr(lpmd::Configuration& con, lpmd::Potential& pot, long int nb, double rcut, lpmd::Matrix& m);

void vacf(lpmd::ConfigurationSet& sim, lpmd::Potential& pot, double dt, lpmd::Matrix& m);

} // namespace lpmd

#endif
