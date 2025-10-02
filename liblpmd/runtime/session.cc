//
//
//

#include <lpmd/session.h>
#include "config.h"
#include <iomanip>
#include <sstream>

using namespace lpmd;

class lpmd::SessionImpl
{
 public:
};

namespace
{
 std::string FormatDouble(double value)
 {
  std::ostringstream out;
  out.setf(std::ios::fixed, std::ios::floatfield);
  out << std::setprecision(8) << value;
  return out.str();
 }
}

Session::Session(const UnitSystem & units): Module("session")
{
 simpl = NULL;
 ProcessArguments("module session");

 AssignParameter("libraryversion", VERSION);

 AssignParameter("forcefactor", FormatDouble(units.forcefactor));
 AssignParameter("kboltzmann", FormatDouble(units.kboltzmann));
 AssignParameter("kin2ev", FormatDouble(units.kin2ev));
 AssignParameter("pressfactor", FormatDouble(units.pressfactor));
 AssignParameter("ua3togrcm3", FormatDouble(units.ua3togrcm3));
 AssignParameter("q2a2force", FormatDouble(units.q2a2force));
 AssignParameter("q2a2ev", FormatDouble(units.q2a2ev));
}

Session::~Session() { }

