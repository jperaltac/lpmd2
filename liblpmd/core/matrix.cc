//
//
//

#include <lpmd/matrix.h>
#include <lpmd/error.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

using namespace lpmd;

namespace
{
inline std::size_t Index(long columns, long col, long row)
{
 return static_cast<std::size_t>(row) * static_cast<std::size_t>(columns) + static_cast<std::size_t>(col);
}
}

Matrix::Matrix(long cols, long rows): nr(rows), nc(cols), values(static_cast<std::size_t>(rows*cols), 0.0)
{
 col_labels.Clear();
 for (long i=0;i<cols;++i) col_labels.Append(" ? ");
}

Matrix::Matrix(): nr(0), nc(0), values()
{
}

Matrix::Matrix(const Matrix & m): nr(m.Rows()), nc(m.Columns()), values(m.values), col_labels(m.col_labels)
{
}

Matrix::~Matrix() = default;

long Matrix::Rows() const { return nr; }

long Matrix::Columns() const { return nc; }

double Matrix::Get(long col, long row) const { return values[Index(nc, col, row)]; }

void Matrix::Set(long col, long row, double v) { values[Index(nc, col, row)] = v; }

void Matrix::SetLabel(long col, std::string lbl) { col_labels[col] = lbl; }

std::string Matrix::GetLabel(long col) const { return col_labels[col]; }

double Matrix::Det() const
{
 if (nr != nc) throw InvalidOperation("Determinant of non-square matrix");
 if (nr == 0) return 0.0;

 const long n = nr;
 std::vector<double> a = values;
 double det = 1.0;
 for (long i=0;i<n;++i)
 {
  long pivot_row = i;
  double pivot_value = std::abs(a[Index(n, i, i)]);
  for (long r=i+1;r<n;++r)
  {
   double candidate = std::abs(a[Index(n, i, r)]);
   if (candidate > pivot_value)
   {
    pivot_value = candidate;
    pivot_row = r;
   }
  }
  if (pivot_value <= std::numeric_limits<double>::epsilon()) return 0.0;
  if (pivot_row != i)
  {
   for (long c=0;c<n;++c)
   {
    std::swap(a[Index(n, c, i)], a[Index(n, c, pivot_row)]);
   }
   det = -det;
  }

  const double pivot = a[Index(n, i, i)];
  det *= pivot;
  if (pivot == 0.0) return 0.0;
  for (long r=i+1;r<n;++r)
  {
   const double factor = a[Index(n, i, r)]/pivot;
   if (factor == 0.0) continue;
   for (long c=i;c<n;++c)
   {
    a[Index(n, c, r)] -= factor * a[Index(n, c, i)];
   }
  }
 }
 return det;
}

void Matrix::Inverse()
{
 if (nc != nr) throw InvalidOperation("Inverse of non-square matrix");
 const long n = nr;
 if (n == 0) throw InvalidOperation("Inverse of empty matrix");

 std::vector<double> a = values;
 std::vector<double> inverse(static_cast<std::size_t>(n * n), 0.0);
 for (long i=0;i<n;++i) inverse[Index(n, i, i)] = 1.0;

 const double eps = std::numeric_limits<double>::epsilon();
 for (long i=0;i<n;++i)
 {
  long pivot_row = i;
  double pivot_value = std::abs(a[Index(n, i, i)]);
  for (long r=i+1;r<n;++r)
  {
   double candidate = std::abs(a[Index(n, i, r)]);
   if (candidate > pivot_value)
   {
    pivot_value = candidate;
    pivot_row = r;
   }
  }
  if (pivot_value <= eps) throw InvalidOperation("Matrix is singular and cannot be inverted");
  if (pivot_row != i)
  {
   for (long c=0;c<n;++c)
   {
    std::swap(a[Index(n, c, i)], a[Index(n, c, pivot_row)]);
    std::swap(inverse[Index(n, c, i)], inverse[Index(n, c, pivot_row)]);
   }
  }

  const double pivot = a[Index(n, i, i)];
  if (std::abs(pivot) <= eps) throw InvalidOperation("Matrix is singular and cannot be inverted");
  for (long c=0;c<n;++c)
  {
   a[Index(n, c, i)] /= pivot;
   inverse[Index(n, c, i)] /= pivot;
  }

  for (long r=0;r<n;++r)
  {
   if (r == i) continue;
   const double factor = a[Index(n, i, r)];
   if (std::abs(factor) <= eps) continue;
   for (long c=0;c<n;++c)
   {
    a[Index(n, c, r)] -= factor * a[Index(n, c, i)];
    inverse[Index(n, c, r)] -= factor * inverse[Index(n, c, i)];
   }
  }
 }

 values = std::move(inverse);
}

Matrix & Matrix::operator=(const Matrix & m)
{
 if (this == &m) return *this;
 nr = m.nr;
 nc = m.nc;
 values = m.values;
 col_labels = m.col_labels;
 return (*this);
}

Matrix & Matrix::operator+=(const Matrix & m)
{
 if ((Rows() == 0) && (Columns() == 0)) return (this->operator=(m));
 else if ((Rows() == m.Rows()) && (Columns() == m.Columns()))
 {
  for (long j=0;j<m.Rows();++j)
    for (long i=0;i<m.Columns();++i) Set(i, j, Get(i, j) + m.Get(i, j));
  return (*this);
 }
 else throw InvalidOperation("Adding matrices with different size");
}

Matrix lpmd::operator+(const Matrix & a, const Matrix & b)
{
 if ((a.Rows() == b.Rows()) && (a.Columns() == b.Columns()))
 {
  Matrix s = a;
  for (long j=0;j<a.Rows();++j)
    for (long i=0;i<a.Columns();++i) s.Set(i, j, s.Get(i, j) + b.Get(i, j));
  return s;
 }
 else throw InvalidOperation("Adding matrices with different size");
}

Matrix lpmd::operator*(const Matrix & a, double f)
{
 Matrix s = a;
 for (long j=0;j<a.Rows();++j)
   for (long i=0;i<a.Columns();++i) s.Set(i, j, s.Get(i, j)*f);
 return s;
}

Matrix lpmd::operator/(const Matrix & a, double f)
{
 Matrix s = a;
 for (long j=0;j<a.Rows();++j)
   for (long i=0;i<a.Columns();++i) s.Set(i, j, s.Get(i, j)/f);
 return s;
}

std::ostream & lpmd::operator<<(std::ostream & os, const Matrix & m)
{
 os << "#"; // << std::setfill(' ');
 for (long i=0;i<m.Columns();++i) os << std::setw(19) << m.GetLabel(i) << " ";
 os << '\n';
 for (long j=0;j<m.Rows();++j)
 {
  for (long i=0;i<m.Columns();++i)
  {
   os.setf(std::ios::scientific);
   //os.setf(std::ios::showpoint);
   os.precision(15);
   //os << os.width(20);
   os << "   " << m.Get(i, j);
   /*
   os.setf(std::ios::fixed);
   //os.setf(std::ios::showpoint);
   os << std::setw(19);
   // os << std::setfill(' ');
   os << m.Get(i, j);
   */
  }
  os << '\n';
 }
 return os;
}

