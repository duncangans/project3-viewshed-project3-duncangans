/* Graphics Math
   http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
   http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation */

#include <math.h>

void gmath_norm_vec(float* vec, float* vec_res) {
  float x = vec[0];
  float y = vec[1];
  float z = vec[2];
  float mag = sqrt((x * x) + (y * y) + (z * z));
  vec_res[0] = x / mag;
  vec_res[1] = y / mag;
  vec_res[2] = z / mag;
}

void gmath_add_vec(float* veca, float* vecb, float* vec_res) {
  vec_res[0] = veca[0] + vecb[0];
  vec_res[1] = veca[1] + vecb[1];
  vec_res[2] = veca[2] + vecb[2];
}

void gmath_make_quat(float* vec, float angle, float* quat_res) {
  float half_angle;
  float sin_half_angle;
  float norm_vec[3];

  half_angle = angle * 0.5;
  gmath_norm_vec(vec, norm_vec);
  sin_half_angle = sin(half_angle);

  quat_res[0] = cos(half_angle);
  quat_res[1] = norm_vec[0] * sin_half_angle;
  quat_res[2] = norm_vec[1] * sin_half_angle;
  quat_res[3] = norm_vec[2] * sin_half_angle;
}

void gmath_norm_quat(float* quat, float* quat_res) {
  float w = quat[0];
  float x = quat[1];
  float y = quat[2];
  float z = quat[3];
  float mag = sqrt((w * w) + (x * x) + (y * y) + (z * z));
  quat_res[0] = w / mag;
  quat_res[1] = x / mag;
  quat_res[2] = y / mag;
  quat_res[3] = z / mag;
}

void gmath_conj_quat(float* quat, float* quat_res) {
  quat_res[0] = quat[0];
  quat_res[1] = -quat[1];
  quat_res[2] = -quat[2];
  quat_res[3] = -quat[3];
}

void gmath_mult_quat(float* quata, float* quatb, float* quat_res) {
  float wa = quata[0];
  float xa = quata[1];
  float ya = quata[2];
  float za = quata[3];
  float wb = quatb[0];
  float xb = quatb[1];
  float yb = quatb[2];
  float zb = quatb[3];
  quat_res[0] = wa * wb - xa * xb - ya * yb - za * zb;
  quat_res[1] = wa * xb + xa * wb + ya * zb - za * yb;
  quat_res[2] = wa * yb + ya * wb + za * xb - xa * zb;
  quat_res[3] = wa * zb + za * wb + xa * yb - ya * xb;
}

void gmath_mult_quat_vec(float* quat, float* vec, float* vec_res) {
  float v1 = vec[0];
  float v2 = vec[1];
  float v3 = vec[2];
  float a =  quat[0];
  float b =  quat[1];
  float c =  quat[2];
  float d =  quat[3];
  float t2 =   a*b;
  float t3 =   a*c;
  float t4 =   a*d;
  float t5 =  -b*b;
  float t6 =   b*c;
  float t7 =   b*d;
  float t8 =  -c*c;
  float t9 =   c*d;
  float t10 = -d*d;
  vec_res[0] = 2*( (t8 + t10)*v1 + (t6 -  t4)*v2 + (t3 + t7)*v3 ) + v1;
  vec_res[1] = 2*( (t4 +  t6)*v1 + (t5 + t10)*v2 + (t9 - t2)*v3 ) + v2;
  vec_res[2] = 2*( (t7 -  t3)*v1 + (t2 +  t9)*v2 + (t5 + t8)*v3 ) + v3;
}

void gmath_quat_as_matrix(float* quat, float* matrix_res) {
  float w = quat[0];
  float x = quat[1];
  float y = quat[2];
  float z = quat[3];
  float x2 = x * x;
  float y2 = y * y;
  float z2 = z * z;
  float xy = x * y;
  float xz = x * z;
  float yz = y * z;
  float wx = w * x;
  float wy = w * y;
  float wz = w * z;
  matrix_res[0] =  1.0 - 2.0 * (y2 + z2);
  matrix_res[1] =  2.0 * (xy - wz);
  matrix_res[2] =  2.0 * (xz + wy);
  matrix_res[3] =  0.0;
  matrix_res[4] =  2.0 * (xy + wz);
  matrix_res[5] =  1.0 - 2.0 * (x2 + z2);
  matrix_res[6] =  2.0 * (yz - wx);
  matrix_res[7] =  0.0;
  matrix_res[8] =  2.0 * (xz - wy);
  matrix_res[9] =  2.0 * (yz + wx);
  matrix_res[10] = 1.0 - 2.0 * (x2 + y2);
  matrix_res[11] = 0.0;
  matrix_res[12] = 0.0;
  matrix_res[13] = 0.0;
  matrix_res[14] = 0.0;
  matrix_res[15] = 1.0;
}
