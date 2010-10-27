// This is core/vgl/algo/vgl_h_matrix_3d.txx
#ifndef vgl_h_matrix_3d_txx_
#define vgl_h_matrix_3d_txx_

#include "vgl_h_matrix_3d.h"
#include <vcl_iostream.h>
#include <vcl_fstream.h>
#include <vcl_cmath.h>
#include <vcl_limits.h>
#include <vcl_cassert.h>
#include <vcl_cstdlib.h> // for exit()
#include <vnl/vnl_inverse.h>
#include <vnl/vnl_numeric_traits.h>
#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_quaternion.h>
#include <vnl/algo/vnl_svd.h>

template <class T>
vgl_h_matrix_3d<T>::vgl_h_matrix_3d(vcl_vector<vgl_homg_point_3d<T> > const& points1,
                                    vcl_vector<vgl_homg_point_3d<T> > const& points2)
{
  vnl_matrix<T> W;
  assert(points1.size() == points2.size());
  unsigned int numpoints = points1.size();
  if (numpoints < 5)
  {
    vcl_cerr << "\nvhl_h_matrix_3d - minimum of 5 points required\n";
    vcl_exit(0);
  }

  W.set_size(3*numpoints, 16);

  for (unsigned int i = 0; i < numpoints; i++)
  {
    T x1 = points1[i].x(), y1 = points1[i].y(), z1 = points1[i].z(), w1 = points1[i].w();
    T x2 = points2[i].x(), y2 = points2[i].y(), z2 = points2[i].z(), w2 = points2[i].w();

    W[i*3][0]=x1*w2;     W[i*3][1]=y1*w2;     W[i*3][2]=z1*w2;     W[i*3][3]=w1*w2;
    W[i*3][4]=0.0;       W[i*3][5]=0.0;       W[i*3][6]=0.0;       W[i*3][7]=0.0;
    W[i*3][8]=0.0;       W[i*3][9]=0.0;       W[i*3][10]=0.0;      W[i*3][11]=0.0;
    W[i*3][12]=-x1*x2;   W[i*3][13]=-y1*x2;   W[i*3][14]=-z1*x2;   W[i*3][15]=-w1*x2;

    W[i*3+1][0]=0.0;     W[i*3+1][1]=0.0;     W[i*3+1][2]=0.0;     W[i*3+1][3]=0.0;
    W[i*3+1][4]=x1*w2;   W[i*3+1][5]=y1*w2;   W[i*3+1][6]=z1*w2;   W[i*3+1][7]=w1*w2;
    W[i*3+1][8]=0.0;     W[i*3+1][9]=0.0;     W[i*3+1][10]=0.0;    W[i*3+1][11]=0.0;
    W[i*3+1][12]=-x1*y2; W[i*3+1][13]=-y1*y2; W[i*3+1][14]=-z1*y2; W[i*3+1][15]=-w1*y2;

    W[i*3+2][0]=0.0;     W[i*3+2][1]=0.0;     W[i*3+2][2]=0.0;     W[i*3+2][3]=0.0;
    W[i*3+2][4]=0.0;     W[i*3+2][5]=0.0;     W[i*3+2][6]=0.0;     W[i*3+2][7]=0.0;
    W[i*3+2][8]=x1*w2;   W[i*3+2][9]=y1*w2;   W[i*3+2][10]=z1*w2;  W[i*3+2][11]=w1*w2;
    W[i*3+2][12]=-x1*z2; W[i*3+2][13]=-y1*z2; W[i*3+2][14]=-z1*z2; W[i*3+2][15]=-w1*z2;
  }

  vnl_svd<T> SVD(W);
  t12_matrix_ = vnl_matrix_fixed<T,4,4>(SVD.nullvector().data_block()); // 16-dim. nullvector
}

template <class T>
vgl_h_matrix_3d<T>::vgl_h_matrix_3d(vcl_istream& s)
{
  t12_matrix_.read_ascii(s);
}

template <class T>
vgl_h_matrix_3d<T>::vgl_h_matrix_3d(char const* filename)
{
  vcl_ifstream f(filename);
  if (!f.good())
    vcl_cerr << "vgl_h_matrix_3d::read: Error opening " << filename << vcl_endl;
  else
    t12_matrix_.read_ascii(f);
}

template <class T>
vgl_h_matrix_3d<T>::vgl_h_matrix_3d(vnl_matrix_fixed<T,3,3> const& M,
                                    vnl_vector_fixed<T,3> const& m)
{
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c)
      (t12_matrix_)(r, c) = M(r,c);
    (t12_matrix_)(r, 3) = m(r);
  }
  for (int c = 0; c < 3; ++c)
    (t12_matrix_)(3,c) = 0;
  (t12_matrix_)(3,3) = 1;
}

// == OPERATIONS ==

//-----------------------------------------------------------------------------
//
template <class T>
vgl_homg_point_3d<T>
vgl_h_matrix_3d<T>::operator()(vgl_homg_point_3d<T> const& x) const
{
  vnl_vector_fixed<T, 4> v;
  v[0]=x.x();  v[1]=x.y();   v[2]=x.z();   v[3]=x.w();
  vnl_vector_fixed<T,4> v2 = t12_matrix_ * v;
  return vgl_homg_point_3d<T>(v2[0], v2[1], v2[2], v2[3]);
}

template <class T>
vgl_homg_plane_3d<T>
vgl_h_matrix_3d<T>::preimage(vgl_homg_plane_3d<T> const& p) const
{
  vnl_vector_fixed<T, 4> v;
  v[0]=p.a();  v[1]=p.b();   v[2]=p.c();   v[3]=p.d();
  vnl_vector_fixed<T,4> v2 = t12_matrix_.transpose() * v;
  return vgl_homg_plane_3d<T>(v2[0], v2[1], v2[2], v2[3]);
}

template <class T>
vgl_homg_point_3d<T>
vgl_h_matrix_3d<T>::preimage(vgl_homg_point_3d<T> const& x) const
{
  vnl_vector_fixed<T, 4> v;
  v[0]=x.x();  v[1]=x.y();   v[2]=x.z();   v[3]=x.w();
  v = vnl_inverse(t12_matrix_) * v;
  return vgl_homg_point_3d<T>(v[0], v[1], v[2], v[3]);
}

template <class T>
vgl_homg_plane_3d<T>
vgl_h_matrix_3d<T>::operator()(vgl_homg_plane_3d<T> const& p) const
{
  vnl_vector_fixed<T, 4> v;
  v[0]=p.a();  v[1]=p.b();   v[2]=p.c();   v[3]=p.d();
  v = vnl_inverse_transpose(t12_matrix_) * v;
  return vgl_homg_plane_3d<T>(v[0], v[1], v[2], v[3]);
}

template <class T>
vcl_ostream& operator<<(vcl_ostream& s, vgl_h_matrix_3d<T> const& h)
{
  return s << h.get_matrix();
}

template <class T>
bool vgl_h_matrix_3d<T>::read(vcl_istream& s)
{
  t12_matrix_.read_ascii(s);
  return s.good() || s.eof();
}

template <class T>
bool vgl_h_matrix_3d<T>::read(char const* filename)
{
  vcl_ifstream f(filename);
  if (!f.good())
    vcl_cerr << "vgl_h_matrix_3d::read: Error opening " << filename << vcl_endl;
  return read(f);
}

template <class T>
vcl_istream& operator>>(vcl_istream& s, vgl_h_matrix_3d<T>& H)
{
  H.read(s);
  return s;
}

// == DATA ACCESS ==

template <class T>
T vgl_h_matrix_3d<T>::get (unsigned int row_index, unsigned int col_index) const
{
  return t12_matrix_.get(row_index, col_index);
}

template <class T>
void vgl_h_matrix_3d<T>::get (T* t_matrix) const
{
  T const* data = t12_matrix_.data_block();
  for (int index = 0; index < 16; ++index)
    *t_matrix++ = data[index];
}

template <class T>
void vgl_h_matrix_3d<T>::get (vnl_matrix_fixed<T, 4, 4>* t_matrix) const
{
  *t_matrix = t12_matrix_;
}

template <class T>
void vgl_h_matrix_3d<T>::get (vnl_matrix<T>* t_matrix) const
{
  *t_matrix = t12_matrix_.as_ref(); // size 4x4
}

template <class T>
vgl_h_matrix_3d<T> vgl_h_matrix_3d<T>::get_inverse() const
{
  return vgl_h_matrix_3d<T>(vnl_inverse(t12_matrix_));
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::set (T const* H)
{
  T* data = t12_matrix_.data_block();
  for (int index = 0; index < 16; ++index)
    data[index] = *H++;
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::set (vnl_matrix_fixed<T,4,4> const& H)
{
  t12_matrix_ = H;
  return *this;
}

template <class T>
bool vgl_h_matrix_3d<T>::
projective_basis(vcl_vector<vgl_homg_point_3d<T> > const& /*five_points*/)
{
  vcl_cerr << "vgl_h_matrix_3d<T>::projective_basis(5pts) not yet implemented\n";
  return false;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::set_identity ()
{
  t12_matrix_.set_identity();
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::set_translation(T tx, T ty, T tz)
{
  t12_matrix_(0, 3)  = tx;
  t12_matrix_(1, 3)  = ty;
  t12_matrix_(2, 3)  = tz;
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::
set_rotation_about_axis(vnl_vector_fixed<T,3> const& axis, T angle)
{
  vnl_quaternion<T> q(axis, angle);
  //get the transpose of the rotation matrix
  vnl_matrix_fixed<T,3,3> R = q.rotation_matrix_transpose();
  //fill in with the transpose
  for (int c = 0; c<3; c++)
    for (int r = 0; r<3; r++)
      t12_matrix_[r][c]=R[c][r];
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::
set_rotation_roll_pitch_yaw(T yaw, T pitch, T roll)
{
  typedef typename vnl_numeric_traits<T>::real_t real_t;
  real_t ax = yaw/2, ay = pitch/2, az = roll/2;

  vnl_quaternion<T> qx((T)vcl_sin(ax),0,0,(T)vcl_cos(ax));
  vnl_quaternion<T> qy(0,(T)vcl_sin(ay),0,(T)vcl_cos(ay));
  vnl_quaternion<T> qz(0,0,(T)vcl_sin(az),(T)vcl_cos(az));
  vnl_quaternion<T> q = qz*qy*qx;

  vnl_matrix_fixed<T,3,3> R = q.rotation_matrix_transpose();
  //fill in with the transpose
  for (int c = 0; c<3; c++)
    for (int r = 0; r<3; r++)
      t12_matrix_[r][c]=R[c][r];
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::set_rotation_euler(T rz1, T ry, T rz2)
{
  typedef typename vnl_numeric_traits<T>::real_t real_t;
  real_t az1 = rz1/2, ay = ry/2, az2 = rz2/2;

  vnl_quaternion<T> qz1(0,0,T(vcl_sin(az1)),T(vcl_cos(az1)));
  vnl_quaternion<T> qy(0,T(vcl_sin(ay)),0,T(vcl_cos(ay)));
  vnl_quaternion<T> qz2(0,0,T(vcl_sin(az2)),T(vcl_cos(az2)));
  vnl_quaternion<T> q = qz2*qy*qz1;

  vnl_matrix_fixed<T,3,3> R = q.rotation_matrix_transpose();
  //fill in with the transpose
  for (int c = 0; c<3; c++)
    for (int r = 0; r<3; r++)
      t12_matrix_[r][c]=R[c][r];
  return *this;
}

template <class T>
vgl_h_matrix_3d<T>& vgl_h_matrix_3d<T>::
set_rotation_matrix(vnl_matrix_fixed<T, 3, 3> const& R)
{
  for (unsigned r = 0; r<3; ++r)
    for (unsigned c = 0; c<3; ++c)
      t12_matrix_[r][c] = R[r][c];
  return *this;
}

template <class T>
bool vgl_h_matrix_3d<T>::is_rotation() const
{
  return t12_matrix_.get(0,3) == (T)0
      && t12_matrix_.get(1,3) == (T)0
      && t12_matrix_.get(2,3) == (T)0
      && this->is_euclidean();
}


template <class T>
bool vgl_h_matrix_3d<T>::is_euclidean() const
{
  if ( t12_matrix_.get(3,0) != (T)0 ||
       t12_matrix_.get(3,1) != (T)0 ||
       t12_matrix_.get(3,2) != (T)0 ||
       t12_matrix_.get(3,3) != (T)1 )
    return false; // should not have a translation part

  // use an error tolerance on the orthonormality constraint
  vnl_matrix_fixed<T, 3,3> R = get_upper_3x3_matrix();
  R *= R.transpose();
  R(0,0) -= T(1);
  R(1,1) -= T(1);
  R(2,2) -= T(1);
  return R.absolute_value_max() <= 10*vcl_numeric_limits<T>::epsilon();
}


template <class T>
vgl_h_matrix_3d<T>
vgl_h_matrix_3d<T>::get_upper_3x3() const
{
  //only sensible for affine transformations
  T d = t12_matrix_[3][3];
  assert(d<-1e-9 || d>1e-9);
  vnl_matrix_fixed<T,4,4> m(0.0);
  for (unsigned r = 0; r<3; r++)
    for (unsigned c = 0; c<3; c++)
      m[r][c] = t12_matrix_[r][c]/d;
  m[3][3]=1.0;
  return vgl_h_matrix_3d<T>(m);
}

template <class T>
vnl_matrix_fixed<T, 3,3> vgl_h_matrix_3d<T>::get_upper_3x3_matrix() const
{
  vnl_matrix_fixed<T,3,3> R;
  vgl_h_matrix_3d<T> m = this->get_upper_3x3();
  for (unsigned r = 0; r<3; r++)
    for (unsigned c = 0; c<3; c++)
      R[r][c] = m.get(r,c);
  return R;
}

template <class T>
vgl_homg_point_3d<T>
vgl_h_matrix_3d<T>::get_translation() const
{
  //only sensible for affine transformations
  T d = t12_matrix_[3][3];
  assert(d<-1e-9 || d>1e-9);
  return vgl_homg_point_3d<T>(t12_matrix_[0][3]/d,
                              t12_matrix_[1][3]/d,
                              t12_matrix_[2][3]/d,
                              (T)1);
}

template <class T>
vnl_vector_fixed<T, 3>
vgl_h_matrix_3d<T>::get_translation_vector() const
{
  vgl_homg_point_3d<T> p = this->get_translation();
  return vnl_vector_fixed<T,3>(p.x(), p.y(), p.z());
}

//----------------------------------------------------------------------------
#undef VGL_H_MATRIX_3D_INSTANTIATE
#define VGL_H_MATRIX_3D_INSTANTIATE(T) \
template class vgl_h_matrix_3d<T >; \
template vcl_ostream& operator<<(vcl_ostream&, vgl_h_matrix_3d<T > const& ); \
template vcl_istream& operator>>(vcl_istream&, vgl_h_matrix_3d<T >& )

#endif // vgl_h_matrix_3d_txx_
