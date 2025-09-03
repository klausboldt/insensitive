//
//  insensitive_library.h
//  Insensitive
//
//  Created by Klaus Boldt on 16.11.11.
//  Copyright (c) 2009-2023 Klaus Boldt. All rights reserved.
//

#include "insensitive.h"
#include <glib.h>

#ifndef Insensitive_library_h
#define Insensitive_library_h

#define IDENTITY 0
#define BASIS_Z 1
#define BASIS_X 2
#define BASIS_Y 3

#define FFT_FORWARD -1
#define FFT_BACKWARD 1

#define ROUND(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define SIGN(a,b) ((b) >= 0.0 ? fabsf(a) : -fabsf(a))

typedef struct DSPComplex {
	float real;
	float imag;
} DSPComplex;

typedef struct DSPDoubleComplex {
	double real;
	double imag;
} DSPDoubleComplex;

typedef struct DSPSplitComplex {
	float  *realp;
	float  *imagp;
} DSPSplitComplex;

// Simple Math
unsigned int lb(float value);
unsigned int log16(float value);
unsigned int pow2(unsigned int value);
unsigned int pow16(unsigned int value);
double sinc(double x);
float sincf(float x);
int numberOfSetBits(int i);
int test_for_simple_coherence(int row, int column);
int component_from_base4_coded_product_operator(int index, int spin);
int component_from_base16_coded_product_operator(int index, int spin);
float window_function(enum WindowFunctionType type, float t, float tmax);
float lorentz_gauss_transformation(float t, float tmax, float T2, float sigma, float shift);
float traf(float t, float tmax, float LB);
float trafs(float t, float tmax, float LB);
float dipolar_coupling_constant(float distance, float gyro1, float gyro2);
float distance_between_spins(float b, float gyro1, float gyro2);
float spectral_density(float tau, float omega);
float W0(float b, float tau, float omega1, float omega2);
float W1(float b, float tau, float omega);
float W2(float b, float tau, float omega1, float omega2);
float random_noise(float noise_level);
float gyro_for_code(unsigned int code);
int indirect_datapoints(enum PurePhaseDetectionMethod method, int direct_datapoints);
float pulseDurationToSliderScale(float value, float flipAngle);
float sliderScaleToPulseDuration(float value, float flipAngle);
void coupling_partners_from_index(int *spin1type, int *spin2type, int index, int spins);
unsigned int gcd(unsigned int a, unsigned int b);
int gcd_list(int *numbers, unsigned int size);

// Complex Algebra
DSPComplex complex_rect(float re, float im);
DSPComplex complex_mul(DSPComplex z1, DSPComplex z2);

// Matrix Algebra
void matrix_multiply(DSPComplex *matrix1, DSPComplex *matrix2, unsigned int size);
DSPComplex *kronecker_multiply(DSPComplex *matrix1, unsigned int size1, DSPComplex *matrix2, unsigned int size2);
DSPComplex trace(DSPComplex *matrix, unsigned int size);
DSPComplex expectation_value(DSPComplex *density_matrix, DSPComplex *operator, unsigned int size);
float calculate_xi(float *matrix);
void swap_columns_in_matrix_of_size(int col1, int col2, float *matrix, int size);
void swap_rows_in_matrix_of_size(int row1, int row2, float *matrix, int size);

// Matrix Constructors
void set_complex_zero_matrix(DSPComplex *matrix, unsigned int size);
void set_complex_identity_matrix(DSPComplex *matrix, unsigned int size);
DSPComplex *cartesian_operator(DSPComplex *one_spin, unsigned int index, unsigned int spins);
DSPComplex *Iz(unsigned int index, unsigned int spins);
DSPComplex *Ix(unsigned int index, unsigned int spins);
DSPComplex *Iy(unsigned int index, unsigned int spins);
DSPComplex *Iminus(unsigned int index, unsigned int spins);
DSPComplex *Iplus(unsigned int index, unsigned int spins);
DSPComplex *IzSz(unsigned int array1, unsigned int array2, unsigned int spins);
DSPComplex *I1I2(unsigned int array1, unsigned int array2, unsigned int spins);
DSPComplex *rotation(unsigned int spins, unsigned int array, DSPComplex *one_spin_rotation);
DSPComplex *xy_rotation(int spins, int array, float angle, float phase);
DSPComplex *arbitrary_rotation(int spins, int array, float flip_angle, float phase, float axis_angle);
DSPComplex *z_rotation(int spins, int array, float angle);
DSPComplex *strong_coupling_operator(unsigned int spins, unsigned int array1, unsigned int array2, float angle);
void set_thermal_equilibrium(DSPComplex *matrix, unsigned int spins, unsigned int spin_type_array,
			     float gyroI, float gyroS);
DSPComplex *matrix_from_base4_array(int *index, int spins, int number_of_operators,
				    int spin_type_array, float *coefficient, float gyroI, float gyroS);
DSPComplex *relaxation_matrix(int spins, int spin_type_array, float *tensor, float tau, float gyroI, float gyroS, int transverse);
//DSPComplex *transverse_relaxation_matrix(int spins, int spin_type_array, float *tensor, float tau, float gyroI, float gyroS);
DSPComplex *solomon_transformation_matrix(int spins);
DSPComplex *irreducible_spherical_tensor(unsigned int spins, unsigned int array, unsigned int j, int m);
char *irreducible_spherical_tensor_label(unsigned int spins, unsigned int array, unsigned int j, int m);

// NMR Operations
void equation_of_motion(DSPComplex *matrix, DSPComplex *propagator, unsigned int size);
void longitudinal_relaxation(DSPComplex *matrix, unsigned int size, int spin_type_array, float T1,
			     float time, float gyroI, float gyroS);
void transverse_relaxation(DSPComplex *matrix, unsigned int size, float T2, float time);
void transverse_longitudinal_relaxation(DSPComplex *matrix, unsigned int size, float T1rho, float time);
void dipolar_relaxation(DSPComplex *matrix, unsigned int size, unsigned int spin_type_array,
			DSPComplex *relaxation, float time, float gyroI, float gyroS);
void transverse_dipolar_relaxation(DSPComplex *matrix, unsigned int size,
				   DSPComplex *relaxation, float time);

// Gradient Operations
DSPComplex **create_matrix_array_for_gradients(DSPComplex *start_matrix, unsigned int size, unsigned int slices);
void gradient(DSPComplex **matrix_array, float *coupling_matrix, unsigned int spins,
	      int spin_type_array, float strength, float time, int slices);
void average_matrix_array(DSPComplex *matrix, DSPComplex **array, unsigned int size, unsigned int slices);

// Spectrum processing
void derivative(float *f, int N);
int peak_list(float *spectrum, int N, int *peaks, float threshold);
void tilt_x(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints);
void tilt_y(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints);
void tilt_stretch_y(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints);
void foldover_correction(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints);
int multiple_lorentzian_peak_fit(float *spectrum, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters);
int dosy_fit(float *spectrum, int t1datapoints, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters);
void dosy_spectrum(float *spectrum, int t1datapoints, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters);
double monoexponential_function(double *parameter, int n, void *fdata);
void monoexponential_grad(double *g, double *p, int x, void *fdata);
double biexponential_function(double *parameter, int n, void *fdata);
void biexponential_grad(double *g, double *p, int x, void *fdata);
double multiple_lorentzian_function(double *p, int x, void *fdata);
void multiple_lorentzian_grad(double *g, double *p, int x, void *fdata);

// Input & Output
void string_for_complex_number(char *string, DSPComplex z);
void string_for_double_complex_number(char *string, DSPDoubleComplex z);
void print_real_matrix(float *matrix, unsigned int size);
void print_double_matrix(double *matrix, unsigned int size);
void print_complex_matrix(DSPComplex *matrix, unsigned int size);
void print_double_complex_matrix(DSPDoubleComplex *matrix, unsigned int size);
char *cartesian_product_operators(DSPComplex *matrix, int size, int spin_type_array);
char *spherical_product_operators(DSPComplex *matrix, int size, int spin_type_array);
gchar *substring_for_keyword_in_string(gchar *keyword, gchar *text, gsize length);
void coherence_orders_for_matrix(DSPComplex *matrix, int *orders, int spins);
char *product_operator_from_base4(int index, int number_of_spins, int spin_type_array, float coefficient);
char *replace_numbers_by_indices(char *string);
char *replace_numbers_by_exponents(char *string);
gboolean insensitive_g_string_replace(GString *g_origString, char *stringToReplace, char *replaceWith, GString *g_output);
VectorCoordinates *alloc_vector_coordinates(int size);
void free_vector_coordinates(VectorCoordinates *structure);

// vDSP FFT interfaces
void vDSP_fft_zip(fftw_plan __Plan, const DSPSplitComplex *__C, unsigned int __stride, unsigned int __Log2N, int __Direction);
void vDSP_fft2d_zip(fftw_plan __Plan, const DSPSplitComplex *__C, unsigned int __IC0, unsigned int __IC1, unsigned int __Log2N0, unsigned int __Log2N1, int __Direction);

// LAPACK ssyevr_ bug workaround
int eigen_symmv(float *matrix, float *eval, float *evec, int size);
int linalg_LU_decomp(float *LU, size_t *p, int *signum, int size);
int linalg_LU_invert(float *LU, size_t *p, float *inverse, int size);
DSPComplex *strong_coupling_operator_workaround(unsigned int spins, unsigned int array1, unsigned int array2, float angle);

// LAPACK prototypes
typedef int __CLPK_integer;
typedef float __CLPK_real;
typedef int armpl_int_t;

void sgetrf_(armpl_int_t *m, armpl_int_t *n, float *a, armpl_int_t *lda, armpl_int_t *ipiv, armpl_int_t *info);
void sgetri_(armpl_int_t *n, float *a, armpl_int_t *lda, armpl_int_t *ipiv, float *work, armpl_int_t *lwork, armpl_int_t *info);
float slamch_(char *cmach);
void ssyevr_(char *jobz, char *range, char *uplo, armpl_int_t *n, float *a, armpl_int_t *lda, float *vl, float *vu, armpl_int_t *il, armpl_int_t *iu, float *abstol, armpl_int_t *m, float *w, float *z, armpl_int_t *ldz, armpl_int_t *isuppz, float *work, armpl_int_t *lwork, armpl_int_t *iwork, armpl_int_t *liwork, armpl_int_t *info);

/*void catlas_cset(const int __N, const void *__alpha, void *__X, const int __incX);
void cblas_cscal(const int __N, const void *__alpha, void *__X, const int __incX);
void cblas_scopy(const int __N, const float *__X, const int __incX, float *__Y, const int __incY);
void cblas_ccopy(const int __N, const void *__X, const int __incX, void *__Y, const int __incY);
void cblas_caxpy(const int __N, const void *__alpha, const void *__X, const int __incX, void *__Y, const int __incY);
void cblas_cgemm(const enum CBLAS_ORDER __Order, const enum CBLAS_TRANSPOSE __TransA, const enum CBLAS_TRANSPOSE __TransB, const int __M, const int __N, const int __K, const void *__alpha, const void *__A, const int __lda, const void *__B, const int __ldb, const void *__beta, void *__C, const int __ldc);*/

#endif
