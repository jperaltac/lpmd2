//
//
//

#include "lennardjones.h"

#include <lpmd/configuration.h>
#include <runtime/runtime_context.h>

#include <cmath>
#include <iostream>
#include <vector>

#ifdef LPMD_ENABLE_RUST_HOTSPOTS
#include <lpmd/rust_hotspots.h>
#endif

using namespace lpmd;

LennardJones::LennardJones(std::string args) : Plugin("lennardjones", "2.0") {
  ParamList& params = (*this);
  //
  DefineKeyword("sigma");
  DefineKeyword("epsilon");
  DefineKeyword("cutoff");
  ProcessArguments(args);
  sigma = params["sigma"];
  epsilon = params["epsilon"];
  cutoff = params["cutoff"];
  SetCutoff(cutoff);
}

void LennardJones::ShowHelp() const {
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Module Name        = lennardjones                                             \n";
  std::cout << " Problems Report to = admin@lpmd.cl                                            \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Info      >>                                                          \n";
  std::cout << "      This plugin implements the Lenard-Jones potential for pairs interaction, \n";
  std::cout << "      which has the form                                                       \n";
  std::cout << "            V(r)=4*epsilon( (sigma/r)^12 - (sigma/r)^6 )                       \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " General Options   >>                                                          \n";
  std::cout << "      sigma         : Sets de value of sigma for the potential (in angstrom).  \n";
  std::cout << "      epsilon       : Sets de value of epsilon for the potential (in eV).      \n";
  std::cout << "      cutoff        : Cutoff for the potential (in angstrom).                  \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << " Example           >>                                                          \n";
  std::cout << " #Loading the plugin :                                                         \n";
  std::cout << " use lennardjones as lj                                                        \n";
  std::cout << "     sigma 3.4                                                                 \n";
  std::cout << "     epsilon 2.0                                                               \n";
  std::cout << "     cutoff 1.90                                                               \n";
  std::cout << " enduse                                                                        \n";
  std::cout << " #Applying the plugin :                                                        \n";
  std::cout << " potential lj Ar Ar                                                          \n\n";
  std::cout
      << "      The plugin implements the Lennard-Jones potential between argon (Ar) atoms.\n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
}

double LennardJones::pairEnergy(const double& r) const {
  double rtmp = sigma / r;
  double r6 = rtmp * rtmp * rtmp * rtmp * rtmp * rtmp;
  double r12 = r6 * r6;
  return 4.0e0 * epsilon * (r12 - r6);
}

Vector LennardJones::pairForce(const Vector& r) const {
  double rr2 = r.SquareModule();
  double r6 = pow(sigma * sigma / rr2, 3.0e0);
  double r12 = r6 * r6;
  double ff = -48.0e0 * (epsilon / rr2) * (r12 - 0.50e0 * r6);
  Vector fv = r * ff;
  return fv;
}

void LennardJones::UpdateForces(Configuration& conf) {
#ifndef LPMD_ENABLE_RUST_HOTSPOTS
  PairPotential::UpdateForces(conf);
#else
  const double forcefactor = conf.Context().session()["forcefactor"];
  BasicParticleSet& atoms = conf.Atoms();
  const long int n = atoms.Size();
  energycache = 0.0;
  double stress[3][3];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j)
      stress[i][j] = 0.0e0;
  }
  const double cutoff = GetCutoff();

  double etmp = 0.0e0, tmpvir = 0.0e0;
  std::vector<double> rij_buffer;
  std::vector<double> r2_buffer;
  std::vector<double> force_buffer;
  std::vector<long int> neighbor_indices;

  for (long i = 0; i < n; ++i) {
    NeighborList nlist;
    conf.GetCellManager().BuildNeighborList(conf, i, nlist, false, cutoff);
    const long neighbor_count = nlist.Size();
    rij_buffer.clear();
    r2_buffer.clear();
    neighbor_indices.clear();
    rij_buffer.reserve(static_cast<std::size_t>(neighbor_count) * 3);
    r2_buffer.reserve(static_cast<std::size_t>(neighbor_count));
    neighbor_indices.reserve(static_cast<std::size_t>(neighbor_count));

    for (long k = 0; k < neighbor_count; ++k) {
      const AtomPair& nn = nlist[k];
      if (AppliesTo(atoms[i].Z(), nn.j->Z()) && nn.r2 < cutoff * cutoff) {
        rij_buffer.push_back(nn.rij[0]);
        rij_buffer.push_back(nn.rij[1]);
        rij_buffer.push_back(nn.rij[2]);
        r2_buffer.push_back(nn.r2);
        neighbor_indices.push_back(nn.j_index);
      }
    }

    const std::size_t pair_count = r2_buffer.size();
    if (pair_count == 0)
      continue;

    force_buffer.resize(pair_count * 3);
    double batch_energy = 0.0;
    double batch_virial = 0.0;
    double batch_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    const int status = lpmd_lennard_jones_batch(rij_buffer.data(), r2_buffer.data(), pair_count,
                                                sigma, epsilon, cutoff, force_buffer.data(),
                                                &batch_energy, &batch_virial, batch_stress);
    if (status != 0) {
      for (long k = 0; k < neighbor_count; ++k) {
        const AtomPair& nn = nlist[k];
        if (AppliesTo(atoms[i].Z(), nn.j->Z()) && nn.r2 < cutoff * cutoff) {
          etmp += pairEnergy(sqrt(nn.r2));
          Vector ff = pairForce(nn.rij);
          atoms[i].Acceleration() += ff * (forcefactor / atoms[i].Mass());
          nn.j->Acceleration() -= ff * (forcefactor / nn.j->Mass());
          tmpvir -= Dot(nn.rij, ff);
          for (int l = 0; l < 3; ++l) {
            stress[0][l] += -(nn.rij)[0] * ff[l];
            stress[1][l] += -(nn.rij)[1] * ff[l];
            stress[2][l] += -(nn.rij)[2] * ff[l];
          }
        }
      }
      continue;
    }

    etmp += batch_energy;
    tmpvir += batch_virial;
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
        stress[row][col] += batch_stress[row * 3 + col];

    for (std::size_t k = 0; k < pair_count; ++k) {
      const Vector ff(force_buffer[k * 3], force_buffer[k * 3 + 1], force_buffer[k * 3 + 2]);
      atoms[i].Acceleration() += ff * (forcefactor / atoms[i].Mass());
      atoms[neighbor_indices[k]].Acceleration() -=
          ff * (forcefactor / atoms[neighbor_indices[k]].Mass());
    }
  }

  double& config_virial = conf.Virial();
  energycache += etmp;
  config_virial += tmpvir;
  Matrix& config_stress = conf.StressTensor();
  for (int p = 0; p < 3; p++)
    for (int q = 0; q < 3; q++)
      config_stress.Set(q, p, config_stress.Get(q, p) + stress[q][p]);
#endif
}

// Esto se incluye para que el modulo pueda ser cargado dinamicamente
Plugin* create(std::string args) { return new LennardJones(args); }
void destroy(Plugin* m) { delete m; }
