//
//
//

#ifndef __LPMD_MAP_H__
#define __LPMD_MAP_H__

#include <cstdlib>
#include <iostream>
#include <lpmd/array.h>
#include <lpmd/util.h>
#include <lpmd/vector.h>
#include <string>

#include <lpmd/parameter.h>

namespace lpmd {
//

class Map {
public:
  Map() {}
  virtual ~Map() {}

  virtual bool Defined(const std::string& key) const = 0;

  virtual Parameter& operator[](const std::string& key) = 0;

  virtual const Parameter& operator[](const std::string& key) const = 0;

  virtual void Remove(const std::string& key) = 0;

  virtual Array<Parameter> Parameters() const = 0;
};

//
inline std::ostream& operator<<(std::ostream& os, const lpmd::Map& m) {
  Array<Parameter> params = m.Parameters();
  for (long int i = 0; i < params.Size(); ++i) {
    os << params[i] << " = " << m[params[i]] << '\n';
  }
  return os;
}

} // namespace lpmd

#endif
