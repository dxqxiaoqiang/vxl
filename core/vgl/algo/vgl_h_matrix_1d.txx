// This is core/vgl/algo/vgl_h_matrix_1d.txx
#ifndef vgl_h_matrix_1d_txx_
#define vgl_h_matrix_1d_txx_

#include "vgl_h_matrix_1d.h"
#include <vgl/vgl_homg_point_1d.h>
#include <vnl/vnl_inverse.h>
#include <vnl/vnl_vector_fixed.h>
#include <vnl/algo/vnl_svd.h>
#include <vcl_cstdlib.h> // for exit()
#include <vcl_fstream.h>
#include <vcl_cassert.h>

template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(const vgl_h_matrix_1d<T>& M)
  : t12_matrix_(M.get_matrix())
{
}

template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(vnl_matrix_fixed<T,2,2> const& M)
  : t12_matrix_(M)
{
}

//--------------------------------------------------------------------------------
template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(const vgl_h_matrix_1d<T>& A,const vgl_h_matrix_1d<T>& B)
  : t12_matrix_(A.t12_matrix_ * B.t12_matrix_)
{
}

template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(const T* H)
  : t12_matrix_(H)
{
}

template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(vcl_istream &is)
{
  t12_matrix_.read_ascii(is);
}

template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(vcl_vector<vgl_homg_point_1d<T> > const& points1,
                                    vcl_vector<vgl_homg_point_1d<T> > const& points2)
{
  vnl_matrix<T> W;
  assert(points1.size() == points2.size());
  unsigned int numpoints = points1.size();
  if (numpoints < 3)
  {
    vcl_cerr << "\nvhl_h_matrix_1d - minimum of 3 points required\n";
    vcl_exit(0);
  }

  W.set_size(numpoints, 4);

  for (unsigned int i = 0; i < numpoints; i++)
  {
    T x1 = points1[i].x(), w1 = points1[i].w();
    T x2 = points2[i].x(), w2 = points2[i].w();

    W[i][0]=x1*w2;    W[i][1]=w1*w2;
    W[i][2]=-x1*x2;   W[i][3]=-w1*x2;
  }

  vnl_svd<T> SVD(W);
  t12_matrix_ = vnl_matrix_fixed<T,2,2>(SVD.nullvector().data_block()); // 4-dim. nullvector
}

// == OPERATIONS ==

template <class T>
vgl_homg_point_1d<T> vgl_h_matrix_1d<T>::operator()(const vgl_homg_point_1d<T>& x1) const
{
  vnl_vector_fixed<T,2> v = t12_matrix_ * vnl_vector_fixed<T,2>(x1.x(),x1.w());
  return vgl_homg_point_1d<T>(v[0], v[1]);
}

template <class T>
vgl_homg_point_1d<T> vgl_h_matrix_1d<T>::preimage(const vgl_homg_point_1d<T>& x2) const
{
  vnl_vector_fixed<T,2> v = vnl_inverse(t12_matrix_) *
                            vnl_vector_fixed<T,2>(x2.x(),x2.w());
  return vgl_homg_point_1d<T>(v[0], v[1]);
}

template <class T>
vgl_h_matrix_1d<T> vgl_h_matrix_1d<T>::get_inverse() const
{
  return vgl_h_matrix_1d<T>(vnl_inverse(t12_matrix_));
}

//-----------------------------------------------------------------------------
template <class T>
vcl_ostream& operator<<(vcl_ostream& s, const vgl_h_matrix_1d<T>& h)
{
  return s << h.get_matrix();
}

template <class T>
vcl_istream& operator >> (vcl_istream& s, vgl_h_matrix_1d<T>& H)
{
  H = vgl_h_matrix_1d<T>(s);
  return s;
}

template <class T>
bool vgl_h_matrix_1d<T>::read(vcl_istream& s)
{
  t12_matrix_.read_ascii(s);
  return s.good() || s.eof();
}


template <class T>
vgl_h_matrix_1d<T>::vgl_h_matrix_1d(char const* filename)
{
  vcl_ifstream f(filename);
  if (!f.good())
    vcl_cerr << "vgl_h_matrix_1d::read: Error opening " << filename << vcl_endl;
  else
    t12_matrix_.read_ascii(f);
}

template <class T>
bool vgl_h_matrix_1d<T>::read(char const* filename)
{
  vcl_ifstream f(filename);
  if (!f.good())
    vcl_cerr << "vgl_h_matrix_1d::read: Error opening " << filename << vcl_endl;
  return read(f);
}

// == DATA ACCESS ==

//-----------------------------------------------------------------------------
template <class T>
T vgl_h_matrix_1d<T>::get(unsigned int row_index, unsigned int col_index) const
{
  return t12_matrix_.get(row_index, col_index);
}

template <class T>
void vgl_h_matrix_1d<T>::get(T *H) const
{
  T const* data = t12_matrix_.data_block();
  for (int index = 0; index < 4; ++index)
    *H++ = data[index];
}

template <class T>
void vgl_h_matrix_1d<T>::get(vnl_matrix_fixed<T,2,2>* H) const
{
  *H = t12_matrix_;
}

template <class T>
void vgl_h_matrix_1d<T>::get(vnl_matrix<T>* H) const
{
  *H = t12_matrix_.as_ref(); // size 2x2
}

template <class T>
vgl_h_matrix_1d<T>&
vgl_h_matrix_1d<T>::set(const T* H)
{
  T* data = t12_matrix_.data_block();
  for (int index = 0; index < 4; ++index)
    data[index] = *H++;
  return *this;
}

template <class T>
vgl_h_matrix_1d<T>&
vgl_h_matrix_1d<T>::set(vnl_matrix_fixed<T,2,2> const& H)
{
  t12_matrix_ = H;
  return *this;
}

//-------------------------------------------------------------------
template <class T>
bool vgl_h_matrix_1d<T>::
projective_basis(vcl_vector<vgl_homg_point_1d<T> > const& points)
{
  if (points.size()!=3)
    return false;
  vnl_vector_fixed<T, 2> p0(points[0].x(), points[0].w());
  vnl_vector_fixed<T, 2> p1(points[1].x(), points[1].w());
  vnl_vector_fixed<T, 2> p2(points[2].x(), points[2].w());
  vnl_matrix_fixed<T, 2, 3> point_matrix;
  point_matrix.set_column(0, p0);
  point_matrix.set_column(1, p1);
  point_matrix.set_column(2, p2);

  if (! point_matrix.is_finite() || point_matrix.has_nans())
  {
    vcl_cerr << "vgl_h_matrix_1d<T>::projective_basis():\n"
             << " given points have infinite or NaN values\n";
    this->set_identity();
    return false;
  }
  vnl_svd<T> svd1(point_matrix.as_ref(), 1e-8); // size 2x3
  if (svd1.rank() < 2)
  {
    vcl_cerr << "vgl_h_matrix_1d<T>::projective_basis():\n"
             << " At least two out of the three points are nearly identical\n";
    this->set_identity();
    return false;
  }

  vnl_matrix_fixed<T, 2, 2> back_matrix;
  back_matrix.set_column(0, p0);
  back_matrix.set_column(1, p1);

  vnl_vector_fixed<T, 2> scales_vector = vnl_inverse(back_matrix) * p2;

  back_matrix.set_column(0, scales_vector[0] * p0);
  back_matrix.set_column(1, scales_vector[1] * p1);

  if (! back_matrix.is_finite() || back_matrix.has_nans())
  {
    vcl_cerr << "vgl_h_matrix_1d<T>::projective_basis():\n"
             << " back matrix has infinite or NaN values\n";
    this->set_identity();
    return false;
  }
  this->set(vnl_inverse(back_matrix));
  return true;
}


//----------------------------------------------------------------------------
#undef VGL_H_MATRIX_1D_INSTANTIATE
#define VGL_H_MATRIX_1D_INSTANTIATE(T) \
template class vgl_h_matrix_1d<T >; \
template vcl_ostream& operator << (vcl_ostream& s, const vgl_h_matrix_1d<T >& h); \
template vcl_istream& operator >> (vcl_istream& s, vgl_h_matrix_1d<T >& h)

#endif // vgl_h_matrix_1d_txx_
