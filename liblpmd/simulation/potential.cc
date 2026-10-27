//
//
//

#include <lpmd/potential.h>
#include <lpmd/simulation.h>
#include <lpmd/util.h>

#include <cmath>

using namespace lpmd;

Potential::Potential() : species1(-1), species2(-1), rcutoff(HUGE_VAL), energycache(0.0) {}

Potential::~Potential() {}

void Potential::Initialize(Configuration& conf) {
  assert(&conf != 0);
  energycache = 0;
} // icc 869

// s1 == s2 == -1 is used as a wildcard
void Potential::SetValidSpecies(int s1, int s2) {
  species1 = s1;
  species2 = s2;
}

bool Potential::AppliesTo(int s1, int s2) const {
  return (species1 == -1 && species2 == -1) ||
         (s1 == species1 && s2 == species2) ||
         (s1 == species2 && s2 == species1);
}
