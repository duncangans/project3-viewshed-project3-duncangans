void gmath_norm_vec(float* vec);
void gmath_add_vec(float* veca, float* vecb, float* vec_res);
void gmath_make_quat(float* vec, float angle, float* quat_res);
void gmath_norm_quat(float* quat, float* quat_res);
void gmath_conj_quat(float* quat, float* quat_res);
void gmath_mult_quat(float* quata, float* quatb, float* quat_res);
void gmath_mult_quat_vec(float* quat, float* vec, float* vec_res);
void gmath_quat_as_matrix(float* quat, float* matrix_res);
