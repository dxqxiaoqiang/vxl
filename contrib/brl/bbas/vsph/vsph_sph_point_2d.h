#ifndef vsph_sph_point_2d_h_
#define vsph_sph_point_2d_h_
//:
// \file
#include <vsl/vsl_binary_io.h>
#include <vcl_iostream.h>

//: a point in the spherical coordinate system on the surface of a unit sphere
// theta is elevation with zero at the North Pole, 180 degress at the South Pole
// phi is azimuth with zero pointing East, positive heading North
class vsph_sph_point_2d
{
 public:
  //: Default constructor
  vsph_sph_point_2d() : theta_(0.0), phi_(0.0) {}

 vsph_sph_point_2d(double theta, double phi, bool in_radians = true) : theta_(theta), phi_(phi), in_radians_(in_radians) {}

  ~vsph_sph_point_2d() {}

  void set(double theta, double phi, bool in_radians = true) 
  { in_radians_ = in_radians; theta_=theta; phi_=phi; }

  void print(vcl_ostream& os) const;

  void b_read(vsl_b_istream& is);

  void b_write(vsl_b_ostream& os);

  short version() const {return 1;}
  bool in_radians_;
  double theta_;
  double phi_;
};

vcl_ostream& operator<<(vcl_ostream& os, vsph_sph_point_2d const& p);

#endif