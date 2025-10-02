#include <runtime/units.h>

namespace lpmd {
namespace {
const UnitSystem kDefaultUnitSystem = {
    0.0096485341,  // forcefactor (eV/angstrom -> amu*angstrom/(fs^2))
    8.6173422E-05, // kboltzmann (eV/K)
    103.64269,     // kin2ev (amu*(angstrom/fs)^2 -> eV)
    160217.65,     // pressfactor (eV/(angstrom^3) -> MPa)
    1.6605,        // ua3togrcm3 (uma/A^3 -> g/cm^3)
    0.13893546,    // q2a2force (qe^2/angstrom^2 -> uma*angstrom/(fs^2))
    14.4004        // q2a2ev (qe^2/angstrom -> eV)
};
}

const UnitSystem& DefaultUnitSystem() { return kDefaultUnitSystem; }
} // namespace lpmd
