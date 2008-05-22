/*
  Matrix44.h
  (c) 2008 Tod D. Romo


  Grossfield Lab
  Department of Biochemistry and Biophysics
  University of Rochester Medical School

  4x4 Matrix class for handling coordinate transforms...
*/




#if !defined(MATRIX44_HPP)
#define MATRIX44_HPP


#include <iostream>
#include <stdexcept>
#include <string.h>

#include <Coord.hpp>


using namespace std;


// Forward declaration for matrix-vector multiply
template<class T> Coord<T> operator*(const Matrix44<T>&, const Coord<T>&);


//! Specialized 4x4 Matrix class for handling coordinate transforms.
template<class T>
class Matrix44 {

  T matrix[16];

public:

  //! Create a new identity matrix
  Matrix44() { identity(); }

  //! Create a new matrix with all elements set to v
  Matrix44(const T v) { for (int i = 0; i < 16; i++) matrix[i] = v; }

  //! Zero all elements
  void zero(void) { memset(matrix, 0, 16 * sizeof(T)); }

  //! Identity matrix
  void identity(void) { zero(); matrix[0] = 1; matrix[5] = 1; matrix[10] = 1; matrix[15] = 1; }

  //! Index the matrix element at row j and col i
  T& operator()(const int j, const int i) {
    if (j < 0 || i < 0 || i > 3 || j > 3)
      throw(range_error("Indices into matrix are out of range"));
    return(matrix[j*4+i]);
  }

  //! Index the matrix element at row j and col i
  const T& operator()(const int j, const int i) const {
    if (j < 0 || i < 0 || i > 3 || j > 3)
      throw(range_error("Indices into matrix are out of range"));
    return(matrix[j*4+i]);
  }


  //! Allow access to the linear array of matrix elements
  T& operator[](const int i) {
    if (i < 0 || i > 15)
      throw(range_error("Index into matrix is out of range"));
    return(matrix[i]);
  }

  //! Allow access to the linear array of matrix elements
  const T& operator[](const int i) const {
    if (i < 0 || i > 15)
      throw(range_error("Index into matrix is out of range"));
    return(matrix[i]);
  }

  //! Returns the array pointer
  T* data(void) { return(matrix); }


  //! Addition of two matrices
  Matrix44<T>& operator+=(const Matrix44<T>& rhs) {
    int i;
    for (i=0; i<16; i++)
      matrix[i] += rhs.matrix[i];
    return(*this);
  }

  //! Addition of two matrices
  Matrix44<T> operator+(const Matrix44<T>& rhs) {
    Matrix44<T> res(*this);
    res += rhs;
    return(res);
  }


  //! Addition of a matrix and a constant.
  //! Each element in the matrix is added with the constant...
  //! Relies on the constructor from a constant to handle the case where
  //! you have a matrix + a constant...

  friend Matrix44<T> operator+(const T lhs, const Matrix44<T>& rhs) {
    Matrix44<T> res(rhs);
    res += lhs;
    return(res);
  }

  //! Subtracting matrices
  Matrix44<T>& operator-=(const Matrix44<T>& rhs) {
    int i;
    for (i=0; i<16; i++)
      matrix[i] -= rhs.matrix[i];
    return(*this);
  }

  //! Subtracting matrices
  Matrix44<T> operator-(const Matrix44<T>& rhs) {
    Matrix44<T> res(*this);
    res -= rhs;
    return(res);
  }

  //! Subtraction of a constant from a matrix
  friend Matrix44<T> operator-(const T lhs, const Matrix44<T>& rhs) {
    Matrix44<T> res(rhs);
    res -= lhs;
    return(res);
  }



  //! Friend declaration for matrix-vector multiply...
  friend Coord<T> operator*<>(const Matrix44<T>&, const Coord<T>&);

  //! Matrix-matrix multiply...
  //! Uses temporary storage for the product, then copies back into
  //! the current matrix.
  Matrix44<T>& operator*=(const Matrix44<T>& rhs) {
    T tmp[16];

    tmp[0] = matrix[0]*rhs.matrix[0] + matrix[1]*rhs.matrix[4] + matrix[2]*rhs.matrix[8] + matrix[3]*rhs.matrix[12];
    tmp[1] = matrix[0]*rhs.matrix[1] + matrix[1]*rhs.matrix[5] + matrix[2]*rhs.matrix[9] + matrix[3]*rhs.matrix[13];
    tmp[2] = matrix[0]*rhs.matrix[2] + matrix[1]*rhs.matrix[6] + matrix[2]*rhs.matrix[10] + matrix[3]*rhs.matrix[14];
    tmp[3] = matrix[0]*rhs.matrix[3] + matrix[1]*rhs.matrix[7] + matrix[2]*rhs.matrix[11] + matrix[3]*rhs.matrix[15];

    tmp[4] = matrix[4]*rhs.matrix[0] + matrix[5]*rhs.matrix[4] + matrix[6]*rhs.matrix[8] + matrix[7]*rhs.matrix[12];
    tmp[5] = matrix[4]*rhs.matrix[1] + matrix[5]*rhs.matrix[5] + matrix[6]*rhs.matrix[9] + matrix[7]*rhs.matrix[13];
    tmp[6] = matrix[4]*rhs.matrix[2] + matrix[5]*rhs.matrix[6] + matrix[6]*rhs.matrix[10] + matrix[7]*rhs.matrix[14];
    tmp[7] = matrix[4]*rhs.matrix[3] + matrix[5]*rhs.matrix[7] + matrix[6]*rhs.matrix[11] + matrix[7]*rhs.matrix[15];

    tmp[8] = matrix[8]*rhs.matrix[0] + matrix[9]*rhs.matrix[4] + matrix[10]*rhs.matrix[8] + matrix[11]*rhs.matrix[12];
    tmp[9] = matrix[8]*rhs.matrix[1] + matrix[9]*rhs.matrix[5] + matrix[10]*rhs.matrix[9] + matrix[11]*rhs.matrix[13];
    tmp[10] = matrix[8]*rhs.matrix[2] + matrix[9]*rhs.matrix[6] + matrix[10]*rhs.matrix[10] + matrix[11]*rhs.matrix[14];
    tmp[11] = matrix[8]*rhs.matrix[3] + matrix[9]*rhs.matrix[7] + matrix[10]*rhs.matrix[11] + matrix[11]*rhs.matrix[15];

    tmp[12] = matrix[12]*rhs.matrix[0] + matrix[13]*rhs.matrix[4] + matrix[14]*rhs.matrix[8] + matrix[15]*rhs.matrix[12];
    tmp[13] = matrix[12]*rhs.matrix[1] + matrix[13]*rhs.matrix[5] + matrix[14]*rhs.matrix[9] + matrix[15]*rhs.matrix[13];
    tmp[14] = matrix[12]*rhs.matrix[2] + matrix[13]*rhs.matrix[6] + matrix[14]*rhs.matrix[10] + matrix[15]*rhs.matrix[14];
    tmp[15] = matrix[12]*rhs.matrix[3] + matrix[13]*rhs.matrix[7] + matrix[14]*rhs.matrix[11] + matrix[15]*rhs.matrix[15];

    memcpy(matrix, tmp, 16*sizeof(T));
    return(*this);
  }

  //! Matrix-matrix multiply
  Matrix44<T> operator*(const Matrix44<T>& rhs) {
    Matrix44<T> res(*this);
    res *= rhs;

    return(res);
  }

  //! Multiplication by a constant.
  //! Each element is multiplied by the same constant
  //! This operator [hopefully] prevents auto-casting
  //! of the constant to a matrix and then a matrix-matrix
  //! multiply, but beware...

  Matrix44<T>& operator*=(const T x) {
    for (int i = 0; i < 16; i++)
      matrix[i] *= x;
    return(*this);
  }

  //! Multiplication by a constant
  Matrix44<T> operator*(const T x) {
    Matrix44<T> res(*this);
    
    res *= x;
    return(res);
  }

  //! Handle the constant * matrix case
  friend Matrix44<T> operator*(const T x, const Matrix44<T>& rhs) {
    Matrix44<T> res(rhs);
    
    res *= x;
    return(res);
  }

  
  //! Output the matrix in pseudo-XML
  friend ostream& operator<<(ostream&os, const Matrix44& m) {
    int i, j, k;

    os << "[";
    for (k=j=0; j<4; j++) {
      os << "[";
      for (i=0; i<4; i++)
	os << m[k++] << ((i == 3) ? "]" : ",");
      os << ((j == 3) ? "" : ", ");
    }
    os << "]";

    return(os);
  }



};


//! Matrix-vector multiply
//! This has to be a friend outside the class for GCC to be happy...
template<class T> Coord<T> operator*(const Matrix44<T>& M, const Coord<T>& v) {
    Coord<T> result;

    result.v[0] = v.v[0]*M.matrix[0] + v.v[1]*M.matrix[1] + v.v[2]*M.matrix[2] + v.v[3]*M.matrix[3];
    result.v[1] = v.v[0]*M.matrix[4] + v.v[1]*M.matrix[5] + v.v[2]*M.matrix[6] + v.v[3]*M.matrix[7];
    result.v[2] = v.v[0]*M.matrix[8] + v.v[1]*M.matrix[9] + v.v[2]*M.matrix[10] + v.v[3]*M.matrix[11];
    result.v[3] = v.v[0]*M.matrix[12] + v.v[1]*M.matrix[13] + v.v[2]*M.matrix[14] + v.v[3]*M.matrix[15];

    return(result);
  }


#endif

