/*
 *
 *
 *
 */

#ifndef __LPMD_VECTOR_H__
#define __LPMD_VECTOR_H__

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <lpmd/error.h>
#include <string.h>

namespace lpmd {

template <typename T> class GenericVector {
public:
  GenericVector() {
    for (int q = 0; q < 3; ++q)
      v[q] = T();
  }

  GenericVector(const GenericVector<T>& a) {
    for (int q = 0; q < 3; ++q)
      v[q] = a.v[q];
  }

  GenericVector<T>& operator=(const GenericVector<T>& a) {
    if (this != &a) {
      for (int q = 0; q < 3; ++q)
        v[q] = a.v[q];
    }
    return (*this);
  }

  GenericVector(const T& x, const T& y, const T& z) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
  }

  GenericVector(T* w) {
    for (int q = 0; q < 3; ++q)
      v[q] = w[q];
  }

  GenericVector(const char* str) {
    if (str == nullptr)
      throw SyntaxError("GenericVector(null)");
    const char* p = str;
    auto skip_space = [&]() {
      while (std::isspace(static_cast<unsigned char>(*p)))
        ++p;
    };
    auto invalid = [&]() {
      throw SyntaxError("GenericVector(" + std::string(str) + ")");
    };
    skip_space();
    const bool bracketed = (*p == '<');
    if (bracketed)
      ++p;
    for (int component = 0; component < 3; ++component) {
      skip_space();
      char* end = nullptr;
      const double value = std::strtod(p, &end);
      if (end == p)
        invalid();
      v[component] = static_cast<T>(value);
      p = end;
      if (component < 2) {
        if (bracketed) {
          skip_space();
          if (*p != ',')
            invalid();
          ++p;
        } else if (!std::isspace(static_cast<unsigned char>(*p))) {
          invalid();
        }
      }
    }
    skip_space();
    if (bracketed) {
      if (*p != '>')
        invalid();
      ++p;
      skip_space();
    }
    if (*p != '\0')
      invalid();
  }

  inline T& operator[](int q) { return v[q]; }

  inline const T& operator[](int q) const { return v[q]; }

  inline void Normalize() {
    const T mod = Module();
    for (int q = 0; q < 3; ++q)
      v[q] /= mod;
  }

  inline T Module() const { return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }

  inline T SquareModule() const { return v[0] * v[0] + v[1] * v[1] + v[2] * v[2]; }

  inline T Azimuth() const { return atan2(v[1], v[0]); }

  inline T Zenith() const { return acos(v[2] / Module()); }

  inline GenericVector<T>& operator*=(const T a) {
    for (int q = 0; q < 3; ++q)
      v[q] *= a;
    return (*this);
  }

  inline GenericVector<T>& operator+=(const GenericVector<T>& a) {
    for (int q = 0; q < 3; ++q)
      v[q] += a[q];
    return (*this);
  }

  inline GenericVector<T>& operator-=(const GenericVector<T>& a) {
    for (int q = 0; q < 3; ++q)
      v[q] -= a[q];
    return (*this);
  }

private:
  T v[3];
};

typedef GenericVector<double> Vector;

//
//
//

inline double Dot(const Vector& a, const Vector& b) {
  return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

inline bool operator==(const Vector& a, const Vector& b) {
  for (int q = 0; q < 3; ++q)
    if (fabs(a[q] - b[q]) >= 1.0E-10)
      return false;
  return true;
}

inline void FormattedWrite(std::ostream& os, const Vector& v) {
  os << "<" << v[0] << "," << v[1] << "," << v[2] << ">";
}

inline std::ostream& operator<<(std::ostream& os, const Vector& v) {
  os << v[0] << " " << v[1] << " " << v[2];
  return os;
}

inline Vector RandomVector(double m = 1.0) {
  double w[3];
  for (int q = 0; q < 3; ++q)
    w[q] = 2.0 * drand48() - 1.0;
  double r = Vector(w).Module();
  return Vector(m * w[0] / r, m * w[1] / r, m * w[2] / r);
}

inline Vector operator+(const Vector& a, const Vector& b) {
  return Vector(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline Vector operator-(const Vector& a, const Vector& b) {
  return Vector(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

inline Vector operator*(const Vector& a, const double b) {
  return Vector(a[0] * b, a[1] * b, a[2] * b);
}

inline Vector operator*(const double& b, const Vector& a) {
  return Vector(a[0] * b, a[1] * b, a[2] * b);
}

inline Vector operator/(const Vector& a, const double b) {
  return Vector(a[0] / b, a[1] / b, a[2] / b);
}

inline Vector Cross(const Vector& a, const Vector& b) {
  return Vector(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

//
//
//

const Vector e1(1.0, 0.0, 0.0);
const Vector e2(0.0, 1.0, 0.0);
const Vector e3(0.0, 0.0, 1.0);

const Vector identity[3] = {e1, e2, e3};

} // namespace lpmd

#endif
