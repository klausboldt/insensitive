//
//  insensitive_library.c
//  Insensitive
//
//  Created by Klaus Boldt on 16.11.11.
//  Copyright (c) 2009-2023 Klaus Boldt. All rights reserved.
//

#include "insensitive.h"
#include "insensitive-config.h"
#include "insensitive-library.h"
#include "levmarq.h"


/*******************************************************************/
/**                                                               **/
/**  Index array code to rotate selected spins in the system:     **/
/**                                                               **/
/**  The spins are numbered from 1 to n with I_nz being the       **/
/**  operator for the z-component of the n-th spin. To address    **/
/**  which spins need to be rotated a binary code is used:        **/
/**                                                               **/
/**  101001 == 41 means that spins 1, 4 and 6 in a system of six  **/
/**  or more spins are rotated. (The binary code must be read     **/
/**  from right to left!)                                         **/
/**                                                               **/
/**  To rotate all n spins:   array = (2^n) - 1                   **/
/**  To rotate spin number i: array = 2^(i - 1)                   **/
/**                                                               **/
/*******************************************************************/

/////// // ///    /// //////  //      ///////     ///    ///  /////  //////// //   //
//      // ////  //// //   // //      //          ////  //// //   //    //    //   //
/////// // // //// // //////  //      /////       // //// // ///////    //    ///////
     // // //  //  // //      //      //          //  //  // //   //    //    //   //
/////// // //      // //      /////// ///////     //      // //   //    //    //   //

unsigned int lb(float value)
{
	return (unsigned int)(float)(log(value) / M_LN2);
}


unsigned int log16(float value)
{
	return (unsigned int)(float)(log(value) / M_LN16);
}


unsigned int pow2(unsigned int value)
{
	return 1 << value;
}


unsigned int pow16(unsigned int value)
{
	return 1 << (4 * value);
}


double sinc(double x)
{
	if (x == 0)
		return 1.0;
	else
		return sin(x) / x;
}


float sincf(float x)
{
	if (x == 0)
		return 1.0;
	else
		return sinf(x) / x;
}


int numberOfSetBits(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);

	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}


int test_for_simple_coherence(int row, int column)
{
	if (log(row ^ column) / M_LN2 == (float)lb(row ^ column))
		return (row > column) ? row ^ column : -(row ^ column);
	else
		return 0;
}


int component_from_base4_coded_product_operator(int index, int spin)
{
	int mask;

	/**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-4    */
	/*   system is used (for spin-1/2 particles):             */
	/*   With the basis vectors coded as 0 = 0, z = 1, x = 2  */
	/*   and y = 3. The product operator 4 I_2z I3_y in a     */
	/*   system of three spins would be encoded as:           */
	/*     index = 310 (base-4) = 28 (base-10)                */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/

	mask = 3 * pow2(2 * spin);
	mask = index & mask;
	mask = mask >> (2 * spin);

	return mask;
}


int component_from_base16_coded_product_operator(int index, int spin)
{
	int mask;

	/**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-9    */
	/*   system is used (up to spin-3/2 particles):           */
	/*                                                        */
	/*   Spin-1/2 basis operators are coded as 0 = 0, z = 1,  */
	/*   x = 2 and y = 3. Digits > 3 are not assigned.        */
	/*   The product operator 4 I_2z I3_y in a system of      */
	/*   three spins would be encoded as:                     */
	/*     index = 310 (base-16) = 0011|0001|0000 (base-2)    */
	/*                                                        */
	/*   Spin-1 basis operators are coded as 0 = 0, z = 1,    */
	/*   x = 2, y = 3, z² = 4, [Sx,Sz]₊ = 5, [Sy,Sz]₊ = 6,    */
	/*   [Sx,Sy]₊ = 7 and (Sx²-Sy²) = 8. Digits > 8 are not   */
	/*   assigned.                                            */
	/*   The product operator 4 I_(Sx²-Sy²) I_2z² I3_y in a   */
	/*   system of three spins would be encoded as:           */
	/*     index = 347 (base-16) = 0011|0100|0111 (base-2)    */
	/*                                                        */
	/*   Spin-3/2 basis operators are coded as 0 = 0, z = 1,  */
	/*   x = 2, y = 3, z² = 4, ...                            */
	/*   The product operator ??? in a                        */
	/*   system of three spins would be encoded as:           */
	/*     index = 347 (base-16) = 0011|0100|0111 (base-2)    */
	/*                                                        */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/

	mask = 15 * pow16(spin);
	mask = index & mask;
	mask = mask >> (4 * spin);

	return mask;
}


float window_function(enum WindowFunctionType type, float t, float tmax)
{
	switch (type) {
	case WFExp:
		return exp(-t / (0.25 * tmax));
	case WFCosine:
		return cos(M_PI * t / (2 * tmax));
	case WFTriangle:
		return 1 - (t / tmax);
	case WFHann:
		return 0.5 + 0.5 * cos(M_PI * t / tmax);
	case WFWeightedHann:
		return 0.5 * (1 + cos(M_PI * t / tmax)) * exp(2 * M_PI * t / tmax);
	case WFSineBell:
		return sin(M_PI * t / tmax);
	case WFTraficante:
		return traf(t, tmax, 1 / tmax);         //LB_Traficante * 1024);
	case WFTraficanteSN:
		return trafs(t, tmax, 1 / tmax);        //LB_Traficante * 1024);
	case WFNone:
	default:
		return 1;
	}
}


float lorentz_gauss_transformation(float t, float tmax, float T2, float sigma, float shift)
{
	/* f(x) = A·exp(-(x-b)²/2σ²) with FWHM = 2σ·√(2·ln(2)) */
	return exp(t / (5 * T2)) * exp(-pow(t - shift * tmax, 2) / (2 * pow(5 * sigma, 2)));
}


float traf(float t, float tmax, float LB)
{
	float E, epsilon, T2;

	T2 = 1 / (M_PI * LB);
	E = expf(-t / T2);
	epsilon = expf(-(tmax - t) / T2);

	return E / (E * E + epsilon * epsilon); // * (E + epsilon);
}


float trafs(float t, float tmax, float LB)
{
	float E, epsilon, T2;

	T2 = 1 / (M_PI * LB);
	E = expf(-t / T2);
	epsilon = expf(-(tmax - t) / T2);

	return E * E / (E * E * E + epsilon * epsilon * epsilon) * (E +
								    epsilon);
}


float dipolar_coupling_constant(float distance, float gyro1, float gyro2)
{
	return -1e-7 * h_bar * gyro1 * gyro2 / pow(1e-9 * distance, 3);
}


float distance_between_spins(float b, float gyro1, float gyro2)
{
	return 1e9 * cbrt(-1e-7 * h_bar * gyro1 * gyro2 / b);
}


float spectral_density(float tau, float omega)
{
	// Bloembergen-Purcell-Pound model
	return tau / (1 + pow(omega * tau, 2));
}


float W0(float b, float tau, float omega1, float omega2)
{
	return pow(b, 2) * spectral_density(tau, omega1 - omega2) / 10;
}


float W1(float b, float tau, float omega)
{
	return 3 * pow(b, 2) * spectral_density(tau, omega) / 20;
}


float W2(float b, float tau, float omega1, float omega2)
{
	return 3 * pow(b, 2) * spectral_density(tau, omega1 + omega2) / 5;
}


float random_noise(float noise_level)
{
	float random_number;

	/* Generate number between 1 and -1 */
	random_number = (float)rand();
	random_number *= 2;
	random_number /= (float)RAND_MAX;
	random_number = (random_number - 1) * (noise_level / 1000);

	return random_number;
}


float gyro_for_code(unsigned int code)
{
	switch (code) {
	case 1:
		return gyro_13C;
	case 2:
		return gyro_15N;
	case 3:
		return gyro_19F;
	case 4:
		return gyro_29Si;
	case 5:
		return gyro_31P;
	case 6:
		return gyro_57Fe;
	case 7:
		return gyro_77Se;
	case 8:
		return gyro_113Cd;
	case 9:
		return gyro_119Sn;
	case 10:
		return gyro_129Xe;
	case 11:
		return gyro_183W;
	case 12:
		return gyro_195Pt;
    case 13:
        return gyro_electron;
	case 0:
	default:
		return gyro_1H;
	}
}


int indirect_datapoints(enum PurePhaseDetectionMethod method, int direct_datapoints)
{
	int value;

	value = 128;
	if (value > direct_datapoints)
		value = direct_datapoints;
	if (method == TPPI || method == StatesTPPI)
		value *= 2;

	return value;
}


float pulseDurationToSliderScale(float value, float flipAngle)
{
	return ((value * 360 / flipAngle) + 1.55616) / 1.95616;
}


float sliderScaleToPulseDuration(float value, float flipAngle)
{
	return (1.95616 * value - 1.55616) * flipAngle / 360;
}


void coupling_partners_from_index(int *spin1type, int *spin2type, int index, int spins)
{
	switch (index) {
	case 0:
		*spin1type = 1;
		*spin2type = 2;
		break;
	case 1:
		*spin1type = 1;
		*spin2type = 3;
		break;
	case 2:
		if (spins == 3) {
			*spin1type = 2;
			*spin2type = 3;
		} else {
			*spin1type = 1;
			*spin2type = 4;
		}
		break;
	case 3:
		*spin1type = 2;
		*spin2type = 3;
		break;
	case 4:
		*spin1type = 2;
		*spin2type = 4;
		break;
	case 5:
		*spin1type = 3;
		*spin2type = 4;
		break;
	}
}


 //////  //////  ///    /// //////  //      /////// //   //      /////  //       //////  /////// //////  //////   /////
//      //    // ////  //// //   // //      //       // //      //   // //      //       //      //   // //   // //   //
//      //    // // //// // //////  //      /////     ///       /////// //      //   /// /////   //////  //////  ///////
//      //    // //  //  // //      //      //       // //      //   // //      //    // //      //   // //   // //   //
 //////  //////  //      // //      /////// /////// //   //     //   // ///////  //////  /////// //////  //   // //   //

DSPComplex complex_rect(float re, float im)
{
	DSPComplex temp;

	temp.real = re;
	temp.imag = im;

	return temp;
}


DSPComplex complex_mul(DSPComplex z1, DSPComplex z2)
{
	DSPComplex temp;

	temp.real = z1.real * z2.real - z1.imag * z2.imag;
	temp.imag = z1.real * z2.imag + z1.imag * z2.real;

	return temp;
}


///    ///  /////  //////// //////  // //   //      /////  //       //////  /////// //////  //////   /////
////  //// //   //    //    //   // //  // //      //   // //      //       //      //   // //   // //   //
// //// // ///////    //    //////  //   ///       /////// //      //   /// /////   //////  //////  ///////
//  //  // //   //    //    //   // //  // //      //   // //      //    // //      //   // //   // //   //
//      // //   //    //    //   // // //   //     //   // ///////  //////  /////// //////  //   // //   //

void matrix_multiply(DSPComplex *matrix1, DSPComplex *matrix2, unsigned int size)
{
	DSPComplex alpha, beta, *temp;

	if ((matrix1 == NULL) || (matrix2 == NULL))
		puts("matrix_multiply: matrix is not properly initialized!");
	else {
		alpha = complex_rect(1.0, 0.0);
		beta = complex_rect(0.0, 0.0);
		temp = malloc(size * size * sizeof(DSPComplex));

		cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, size,
			    size, size, &alpha, matrix2, size, matrix1, size,
			    &beta, temp, size);
		cblas_ccopy(size * size, temp, 1, matrix1, 1);
		free(temp);
	}
}


DSPComplex *kronecker_multiply(DSPComplex *matrix1, unsigned int size1, DSPComplex *matrix2, unsigned int size2)
{
	unsigned int i, j, m, n;
	size_t size;
	DSPComplex *temp;

	if (matrix1 == NULL) {
		puts("kronecker_multiply: matrix 1 is not properly initialized!");
		return NULL;
	}
	if (matrix2 == NULL) {
		puts("kronecker_multiply: matrix 2 is not properly initialized!");
		return NULL;
	}

	size = size1 * size2;
	temp = malloc(size * size * sizeof(DSPComplex));

  #pragma omp parallel for
	for (n = 0; n < size1; n++)
		for (m = 0; m < size1; m++)
			for (j = 0; j < size2; j++)
				for (i = 0; i < size2; i++)
					temp[((n * size2) + j) * size +
					     ((m * size2) + i)] =
						complex_mul(matrix1[n * size1 + m],
							    matrix2[j * size2 + i]);

	return temp;
}


DSPComplex trace(DSPComplex *matrix, unsigned int size)
{
	unsigned int i;
	DSPComplex z;

	z = complex_rect(0, 0);

	for (i = 0; i < size; i++) {
		z.real += matrix[i * size + i].real;
		z.imag += matrix[i * size + i].imag;
	}

	return z;
}


DSPComplex expectation_value(DSPComplex *density_matrix, DSPComplex *operator, unsigned int size)
{
	unsigned int size_square;
	DSPComplex trace_value;
	DSPComplex *product;

	if ((density_matrix == NULL) || (operator == NULL)) {
		puts("expectation_value: matrix is not properly initialized!");
		return complex_rect(0, 0);
	} else {
		size_square = size * size;
		product = malloc(size_square * sizeof(DSPComplex));
		cblas_ccopy(size_square, density_matrix, 1, product, 1);
		matrix_multiply(product, operator, size);
		trace_value = trace(product, size);
		free(product);
		return trace_value;
	}
}


float calculate_xi(float *matrix)
{
	__CLPK_integer size, numberOfEigenvalues, *ISUPPZ;
	__CLPK_integer *IWORK, sizeWORK, sizeIWORK, info, zeroInt = 0;
	__CLPK_real abstol, *WORK, zeroDouble = 0.0;
	float *eigenvalues, *eigenvectors, return_value;
	char jobz = 'V', range = 'A', uplo = 'L', cmach = 'S';

	if (matrix == NULL) {
		puts("calculate_xi: matrix is not properly initialized!");
		return 0;
	}
	// Solve eigenvalue equation
	size = 2;
	eigenvalues = malloc(size * sizeof(__CLPK_real));
	eigenvectors = malloc(size * size * sizeof(__CLPK_real));
	ISUPPZ = malloc(2 * size * sizeof(__CLPK_integer));
	WORK = malloc(26 * size * sizeof(__CLPK_real));
	IWORK = malloc(10 * size * sizeof(__CLPK_integer));
	sizeWORK = -1;
	sizeIWORK = -1;
	abstol = slamch_(&cmach);
	sizeWORK = -1;
	sizeIWORK = -1;
	ssyevr_(&jobz, &range, &uplo, &size, matrix, &size,
		&zeroDouble, &zeroDouble, &zeroInt, &zeroInt, &abstol,
		&numberOfEigenvalues, eigenvalues, eigenvectors, &size,
		ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
	sizeWORK = (__CLPK_integer)WORK[0];
	sizeIWORK = IWORK[0];
	free(WORK);
	free(IWORK);
	WORK = malloc(sizeWORK * sizeof(__CLPK_real));
	IWORK = malloc(sizeIWORK * sizeof(__CLPK_integer));
	ssyevr_(&jobz, &range, &uplo, &size, matrix, &size,
		&zeroDouble, &zeroDouble, &zeroInt, &zeroInt, &abstol,
		&numberOfEigenvalues, eigenvalues, eigenvectors, &size,
		ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
	free(ISUPPZ);
	free(IWORK);
	free(WORK);

	// xi is the value that occurrs in each eigenvector
	if (eigenvectors[0] == eigenvectors[3])
		return_value = eigenvectors[0];
	else if (eigenvectors[1] == eigenvectors[2])
		return_value = eigenvectors[1];
	else {
		return_value = FLT_MAX;
		puts("calculate_xi: no value for xi was found!");
	}

	if (return_value != FLT_MAX)
		return_value = 2 * acosf(return_value);
	else
		return_value = 0;

	free(eigenvectors);
	free(eigenvalues);

	return return_value;
}


void swap_columns_in_matrix_of_size(int col1, int col2, float *matrix, int size)
{
	int i;
	float *temp_vector;

	temp_vector = malloc(size * sizeof(float));
	for (i = 0; i < size; i++) {
		temp_vector[i] = matrix[col1 * size + i];
		matrix[col1 * size + i] = matrix[col2 * size + i];
		matrix[col2 * size + i] = temp_vector[i];
	}
	free(temp_vector);
}


void swap_rows_in_matrix_of_size(int row1, int row2, float *matrix, int size)
{
	int i;
	float *temp_vector;

	temp_vector = malloc(size * sizeof(float));
	for (i = 0; i < size; i++) {
		temp_vector[i] = matrix[i * size + row1];
		matrix[i * size + row1] = matrix[i * size + row2];
		matrix[i * size + row2] = temp_vector[i];
	}
	free(temp_vector);
}


///    ///  /////  //////// //////  // //   //      //////  //////  ///    // /////// //////// //////  //    //  ////// ////////  //////  //////  ///////
////  //// //   //    //    //   // //  // //      //      //    // ////   // //         //    //   // //    // //         //    //    // //   // //
// //// // ///////    //    //////  //   ///       //      //    // // //  // ///////    //    //////  //    // //         //    //    // //////  ///////
//  //  // //   //    //    //   // //  // //      //      //    // //  // //      //    //    //   // //    // //         //    //    // //   //      //
//      // //   //    //    //   // // //   //      //////  //////  //   //// ///////    //    //   //  //////   //////    //     //////  //   // ///////

void set_complex_zero_matrix(DSPComplex *matrix, unsigned int size)
{
    unsigned int i;
	DSPComplex z;

	if (matrix) {
		z.real = 0.0;
		z.imag = 0.0;
        for (i = 0; i < size * size; i++) {
            matrix[i].real = z.real;
            matrix[i].imag = z.imag;
        }
		//catlas_cset(size * size, &z, matrix, 1);
	}
}


void set_complex_identity_matrix(DSPComplex *matrix, unsigned int size)
{
	unsigned int i;

	if (matrix) {
		set_complex_zero_matrix(matrix, size);
		for (i = 0; i < size; i++)
			matrix[i * size + i].real = 1.0 / size;
	}
}


DSPComplex *cartesian_operator(DSPComplex *one_spin, unsigned int index, unsigned int spins)
{
	unsigned int i;
	DSPComplex *aux_matrix, *identity, *temp_matrix;

	if (index > (unsigned int)pow2(spins)) {
		puts("cartesian_operator: index exceeds number of spins!");
		return NULL;
	}

	identity = malloc(4 * sizeof(DSPComplex));
	set_complex_identity_matrix(identity, 2);
	aux_matrix = malloc(sizeof(DSPComplex));
	aux_matrix->real = 1.0;
	aux_matrix->imag = 0.0;

	for (i = 0; i < spins; i++) {
		if (i == index)
			temp_matrix =
				kronecker_multiply(aux_matrix, pow2(i), one_spin, 2);
		else
			temp_matrix =
				kronecker_multiply(aux_matrix, pow2(i), identity, 2);

		free(aux_matrix);
		aux_matrix = temp_matrix;
	}
	free(identity);

	return aux_matrix;
}


DSPComplex *Iz(unsigned int index, unsigned int spins)
{
	DSPComplex *spin_z, *return_matrix;

	spin_z = malloc(4 * sizeof(DSPComplex));
	spin_z[0] = complex_rect(0.5, 0.0);
	spin_z[1] = complex_rect(0.0, 0.0);
	spin_z[2] = complex_rect(0.0, 0.0);
	spin_z[3] = complex_rect(-0.5, 0.0);

	return_matrix = cartesian_operator(spin_z, index, spins);
	free(spin_z);

	return return_matrix;
}


DSPComplex *Ix(unsigned int index, unsigned int spins)
{
	DSPComplex *spin_x, *return_matrix;

	spin_x = malloc(4 * sizeof(DSPComplex));
	spin_x[0] = complex_rect(0.0, 0.0);
	spin_x[1] = complex_rect(0.5, 0.0);
	spin_x[2] = complex_rect(0.5, 0.0);
	spin_x[3] = complex_rect(0.0, 0.0);

	return_matrix = cartesian_operator(spin_x, index, spins);
	free(spin_x);

	return return_matrix;
}


DSPComplex *Iy(unsigned int index, unsigned int spins)
{
	DSPComplex *spin_y, *return_matrix;

	spin_y = malloc(4 * sizeof(DSPComplex));
	spin_y[0] = complex_rect(0.0, 0.0);
	spin_y[1] = complex_rect(0.0, 0.5);
	spin_y[2] = complex_rect(0.0, -0.5);
	spin_y[3] = complex_rect(0.0, 0.0);

	return_matrix = cartesian_operator(spin_y, index, spins);
	free(spin_y);

	return return_matrix;
}


DSPComplex *Iminus(unsigned int index, unsigned int spins)
{
	DSPComplex *spin_minus, *return_matrix;

	spin_minus = malloc(4 * sizeof(DSPComplex));
	spin_minus[0] = complex_rect(0.0, 0.0);
	spin_minus[1] = complex_rect(1.0, 0.0);
	spin_minus[2] = complex_rect(0.0, 0.0);
	spin_minus[3] = complex_rect(0.0, 0.0);

	return_matrix = cartesian_operator(spin_minus, index, spins);
	free(spin_minus);

	return return_matrix;
}


DSPComplex *Iplus(unsigned int index, unsigned int spins)
{
	DSPComplex *spin_plus, *return_matrix;

	spin_plus = malloc(4 * sizeof(DSPComplex));
	spin_plus[0] = complex_rect(0.0, 0.0);
	spin_plus[1] = complex_rect(0.0, 0.0);
	spin_plus[2] = complex_rect(-1.0, 0.0);
	spin_plus[3] = complex_rect(0.0, 0.0);

	return_matrix = cartesian_operator(spin_plus, index, spins);
	free(spin_plus);

	return return_matrix;
}


DSPComplex *IzSz(unsigned int array1, unsigned int array2, unsigned int spins)
{
	unsigned int i, size, size_square;
	DSPComplex *spin_group_1, *spin_group_2, *zmag, factor;

	if ((array1 >= (unsigned int)pow2(spins)) || (array2 >= (unsigned int)pow2(spins))) {
		puts("IzSz: index array exceeds number of spins!");
		return NULL;
	}
	if ((array1 & array2) != 0) {
		puts("IzSz: coupling of a spin with itself!");
		return NULL;
	}

	size = pow2(spins);
	size_square = size * size;
	zmag = NULL;
	spin_group_1 = malloc(size_square * sizeof(DSPComplex));
	spin_group_2 = malloc(size_square * sizeof(DSPComplex));
	set_complex_zero_matrix(spin_group_1, size);
	set_complex_zero_matrix(spin_group_2, size);
	factor = complex_rect(1.0, 0.0);
	for (i = 0; i < spins; i++) {
		// Test whether bit at first position is 1 (odd number)
		if ((array1 % 2) != 0) {
			zmag = Iz(i, spins);
			cblas_caxpy(size_square, &factor, zmag, 1, spin_group_1, 1);
		}
		array1 >>= 1;
		if ((array2 % 2) != 0) {
			if (zmag == NULL)
				zmag = Iz(i, spins);
			cblas_caxpy(size_square, &factor, zmag, 1, spin_group_2, 1);
		}
		array2 >>= 1;
		if (zmag != NULL) {
			free(zmag);
			zmag = NULL;
		}
	}
	matrix_multiply(spin_group_1, spin_group_2, size);
	factor = complex_rect(pow(size, 2), 0);
	cblas_cscal(size_square, &factor, spin_group_1, 1);
	free(spin_group_2);

	return spin_group_1;
}


DSPComplex *I1I2(unsigned int array1, unsigned int array2, unsigned int spins)
{
	unsigned int i, size, size_square;
	DSPComplex *zmag, *xmag, *ymag, factor;
	DSPComplex *spin_group_1z, *spin_group_1x, *spin_group_1y;
	DSPComplex *spin_group_2z, *spin_group_2x, *spin_group_2y;

	if ((array1 >= (unsigned int)pow2(spins)) || (array2 >= (unsigned int)pow2(spins))) {
		puts("I1I2: index array exceeds number of spins!");
		return NULL;
	}
	if ((array1 & array2) != 0) {
		puts("I1I2: coupling of a spin with itself!");
		return NULL;
	}

	size = pow2(spins);
	size_square = size * size;
	zmag = NULL;
	xmag = NULL;
	ymag = NULL;
	spin_group_1z = malloc(size_square * sizeof(DSPComplex));
	spin_group_1x = malloc(size_square * sizeof(DSPComplex));
	spin_group_1y = malloc(size_square * sizeof(DSPComplex));
	spin_group_2z = malloc(size_square * sizeof(DSPComplex));
	spin_group_2x = malloc(size_square * sizeof(DSPComplex));
	spin_group_2y = malloc(size_square * sizeof(DSPComplex));
	set_complex_zero_matrix(spin_group_1z, size);
	set_complex_zero_matrix(spin_group_1x, size);
	set_complex_zero_matrix(spin_group_1y, size);
	set_complex_zero_matrix(spin_group_2z, size);
	set_complex_zero_matrix(spin_group_2x, size);
	set_complex_zero_matrix(spin_group_2y, size);
	factor = complex_rect(1.0, 0.0);
	for (i = 0; i < spins; i++) {
		// Test whether bit at first position is 1 (odd number)
		if ((array1 % 2) != 0) {
			zmag = Iz(i, spins);
			xmag = Ix(i, spins);
			ymag = Iy(i, spins);
			cblas_caxpy(size_square, &factor, zmag, 1, spin_group_1z, 1);
			cblas_caxpy(size_square, &factor, xmag, 1, spin_group_1x, 1);
			cblas_caxpy(size_square, &factor, ymag, 1, spin_group_1y, 1);
		}
		array1 >>= 1;
		if ((array2 % 2) != 0) {
			if (zmag == NULL) {
				zmag = Iz(i, spins);
				xmag = Ix(i, spins);
				ymag = Iy(i, spins);
			}
			cblas_caxpy(size_square, &factor, zmag, 1, spin_group_2z, 1);
			cblas_caxpy(size_square, &factor, xmag, 1, spin_group_2x, 1);
			cblas_caxpy(size_square, &factor, ymag, 1, spin_group_2y, 1);
		}
		array2 >>= 1;
		if (zmag != NULL) {
			free(zmag);
			free(xmag);
			free(ymag);
			zmag = NULL;
			xmag = NULL;
			ymag = NULL;
		}
	}
	matrix_multiply(spin_group_1z, spin_group_2z, size);
	matrix_multiply(spin_group_1x, spin_group_2x, size);
	matrix_multiply(spin_group_1y, spin_group_2y, size);
	cblas_caxpy(size_square, &factor, spin_group_1y, 1, spin_group_1x, 1);
	cblas_caxpy(size_square, &factor, spin_group_1x, 1, spin_group_1z, 1);
	factor = complex_rect(pow(size, 2), 0);
	cblas_cscal(size_square, &factor, spin_group_1z, 1);
	free(spin_group_1x);
	free(spin_group_1y);
	free(spin_group_2z);
	free(spin_group_2x);
	free(spin_group_2y);

	return spin_group_1z;
}


DSPComplex *rotation(unsigned int spins, unsigned int array, DSPComplex *one_spin_rotation)
{
	unsigned int i, rotated_spins;
	DSPComplex *rotation_matrix, *identity, *temp_matrix, factor;

	if (array >= pow2(spins)) {
		puts("rotation: index array exceeds number of spins!");
		return NULL;
	}

	identity = malloc(4 * sizeof(DSPComplex));
	set_complex_identity_matrix(identity, 2);
	rotation_matrix = malloc(sizeof(DSPComplex));
	rotation_matrix->real = 1.0;
	rotation_matrix->imag = 0.0;
	rotated_spins = 0;

	for (i = 0; i < spins; i++) {
		if ((array % 2) == 0)
			temp_matrix = kronecker_multiply(rotation_matrix, pow2(i), identity, 2);
		else {
			temp_matrix = kronecker_multiply(rotation_matrix, pow2(i), one_spin_rotation, 2);
			rotated_spins++;
		}
		array >>= 1;
		free(rotation_matrix);
		rotation_matrix = temp_matrix;
	}
	free(identity);

	/******************************************************/
	/**  Calculate coefficient for R rotated spins in a  **/
	/**  of N spins in total: i = 2^N / 2^R              **/
	/******************************************************/
	i = (1 << spins) >> rotated_spins;
	factor = complex_rect(i, 0);
	cblas_cscal(pow2(spins) * pow2(spins), &factor, rotation_matrix, 1);

	return rotation_matrix;
}


DSPComplex *xy_rotation(int spins, int array, float angle, float phase)
{
	DSPComplex *one_spin_rotation, *rotation_matrix;

	angle = angle / 180 * M_PI;
	phase = phase / 180 * M_PI;

	one_spin_rotation = malloc(4 * sizeof(DSPComplex));
	one_spin_rotation[0] = complex_rect(cos(0.5 * angle), 0.0);
	one_spin_rotation[1] = complex_rect(sin(0.5 * angle) * sin(phase), -sin(0.5 * angle) * cos(phase));
	one_spin_rotation[2] = complex_rect(-sin(0.5 * angle) * sin(phase), -sin(0.5 * angle) * cos(phase));
	one_spin_rotation[3] = complex_rect(cos(0.5 * angle), 0.0);

	rotation_matrix = rotation(spins, array, one_spin_rotation);
	free(one_spin_rotation);

	return rotation_matrix;
}


DSPComplex *z_rotation(int spins, int array, float angle)
{
	DSPComplex *one_spin_rotation, *rotation_matrix;

	angle = angle / 180 * M_PI;

	one_spin_rotation = malloc(4 * sizeof(DSPComplex));
	one_spin_rotation[0] = complex_rect(cos(0.5 * angle), -sin(0.5 * angle));
	one_spin_rotation[1] = complex_rect(0.0, 0.0);
	one_spin_rotation[2] = complex_rect(0.0, 0.0);
	one_spin_rotation[3] = complex_rect(cos(0.5 * angle), sin(0.5 * angle));

	rotation_matrix = rotation(spins, array, one_spin_rotation);
	free(one_spin_rotation);

	return rotation_matrix;
}


DSPComplex *arbitrary_rotation(int spins, int array, float flip_angle, float phase, float axis_angle)
{
	DSPComplex *one_spin_rotation, *rotation_matrix;
	float real, imag;
	float cosb, cosb2, cosmb2, cosp, cost2, powcost2;
	float sinb, sinb2, sinmb2, sinp, sint2, powsint2, sint;

	flip_angle = flip_angle / 180 * M_PI;
	phase = (90 - phase) / 180 * M_PI;

	cost2 = cosf(axis_angle / 2);
	sint2 = sinf(axis_angle / 2);
	sint = sinf(axis_angle);
	powcost2 = pow(cost2, 2);
	powsint2 = pow(sint2, 2);
	cosb = cosf(flip_angle);
	sinb = sinf(flip_angle);
	cosb2 = cosf(flip_angle / 2);
	sinb2 = sinf(flip_angle / 2);
	cosmb2 = cosf(-flip_angle / 2);
	sinmb2 = sinf(-flip_angle / 2);
	cosp = cosf(phase);
	sinp = sinf(phase);

	one_spin_rotation = malloc(4 * sizeof(DSPComplex));
	real = cosb2 * powcost2 + cosmb2 * cosb * powsint2 - sinmb2 * sinb * powsint2;
	imag = sinmb2 * powcost2 + sinmb2 * cosb * powsint2 + cosmb2 * sinb * powsint2;
	one_spin_rotation[0] = complex_rect(real, imag);
	real = sint * sinb2 * cosp;
	imag = -sint * sinb2 * sinp;
	one_spin_rotation[1] = complex_rect(real, imag);
	real *= -1;
	one_spin_rotation[2] = complex_rect(real, imag);
	real = cosmb2 * powsint2 + cosmb2 * cosb * powcost2 - sinmb2 * sinb * powcost2;
	imag = sinmb2 * powsint2 + sinmb2 * cosb * powcost2 + cosmb2 * sinb * powcost2;
	one_spin_rotation[3] = complex_rect(real, imag);

	rotation_matrix = rotation(spins, array, one_spin_rotation);
	free(one_spin_rotation);

	return rotation_matrix;
}


DSPComplex *strong_coupling_operator(unsigned int spins, unsigned int array1, unsigned int array2, float angle)
{
	int m, n;
	__CLPK_integer size, vec_size, numberOfEigenvalues, *pivotMatrix, *ISUPPZ;
	__CLPK_integer *IWORK, sizeWORK, sizeIWORK, info, zeroInt = 0;
	__CLPK_real abstol, *WORK, zeroFloat = 0.0;
	__CLPK_real *colMajorMatrix, *eigenvalues, *eigenvectors, *inverse_eigenvectors;
	char jobz = 'V', range = 'A', uplo = 'L', cmach = 'S';
	DSPComplex *complex_eigenvectors, *inverse_complex_eigenvectors, *aux_matrix, zalpha, zbeta, zangle;
	DSPComplex *operator;

	// Create strong coupling operator
	size = pow2(spins);
	vec_size = size * size;
	operator = I1I2(array1, array2, spins);

	// Convert matrix to column major form
	colMajorMatrix = malloc(vec_size * sizeof(__CLPK_real));
	for (m = 0; m < size; m++)
		for (n = 0; n < size; n++)
			colMajorMatrix[n * size + m] =
				operator[n * size + m].real;

	// Solve eigenvalue equation
	abstol = slamch_(&cmach);
	eigenvalues = malloc(size * sizeof(__CLPK_real));
	eigenvectors = malloc(size * size * sizeof(__CLPK_real));
	ISUPPZ = malloc(2 * size * sizeof(__CLPK_integer));
	WORK = malloc(26 * size * sizeof(__CLPK_real));
	IWORK = malloc(10 * size * sizeof(__CLPK_integer));
	sizeWORK = -1;
	sizeIWORK = -1;
	ssyevr_(&jobz, &range, &uplo, &size, colMajorMatrix, &size,
		&zeroFloat, &zeroFloat, &zeroInt, &zeroInt, &abstol,
		&numberOfEigenvalues, eigenvalues, eigenvectors, &size,
		ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
	sizeWORK = (__CLPK_integer)WORK[0];
	sizeIWORK = IWORK[0];
	free(WORK);
	free(IWORK);
	//sizeWORK = 33 * size; sizeIWORK = 10 * size;
	WORK = malloc(sizeWORK * sizeof(__CLPK_real));
	IWORK = malloc(sizeIWORK * sizeof(__CLPK_integer));
	ssyevr_(&jobz, &range, &uplo, &size, colMajorMatrix, &size,
		&zeroFloat, &zeroFloat, &zeroInt, &zeroInt, &abstol,
		&numberOfEigenvalues, eigenvalues, eigenvectors, &size,
		ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
	free(ISUPPZ);

	// Perform LU decomposition and invert eigenvector matrix
	inverse_eigenvectors = malloc(vec_size * sizeof(__CLPK_real));
	cblas_scopy(vec_size, eigenvectors, 1, inverse_eigenvectors, 1);
	pivotMatrix = malloc(size * sizeof(__CLPK_integer));
	sgetrf_(&size, &size, inverse_eigenvectors, &size, pivotMatrix, &info);
	sgetri_(&size, inverse_eigenvectors, &size, pivotMatrix, WORK, &sizeWORK, &info);
	free(WORK);
	free(IWORK);
	free(pivotMatrix);

	// Revert to complex matrices
	complex_eigenvectors = malloc(vec_size * sizeof(DSPComplex));
	inverse_complex_eigenvectors = malloc(vec_size * sizeof(DSPComplex));
	for (m = 0; m < vec_size; m++) {
		// LAPACK inverts rows and columns -> use transpose matrices later!
		complex_eigenvectors[m] = complex_rect(eigenvectors[m], 0);
		inverse_complex_eigenvectors[m] = complex_rect(inverse_eigenvectors[m], 0);
	}

	free(colMajorMatrix);
	free(eigenvectors);
	free(inverse_eigenvectors);
	free(eigenvalues);

	// Perform matrix transformation D = X^-1.A.X
	zalpha = complex_rect(1, 0);
	zbeta = complex_rect(0, 0);
	zangle = complex_rect(0, 0.5 * angle);
	aux_matrix = malloc(vec_size * sizeof(DSPComplex));
	// aux_matrix = operator.eigenvectors
	cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size,
		    size, &zalpha, operator, size, complex_eigenvectors, size,
		    &zbeta, aux_matrix, size);
	// operator = zangle * inverse_eigenvectors.aux_matrix
	cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size,
		    size, &zangle, inverse_complex_eigenvectors, size,
		    aux_matrix, size, &zbeta, operator, size);

	// Calculate exp(D)
	for (m = 0; m < size; m++)
		operator[m * size + m] =
			complex_rect(exp(operator[m * size + m].real) *
				     cos(operator[m * size + m].imag),
				     exp(operator[m * size + m].real) *
				     sin(operator[m * size + m].imag));

	// Perform matrix transformation A = X D X^-1
	// aux_matrix = operator.inverse_eigenvectors
	cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size,
		    size, &zalpha, operator, size, inverse_complex_eigenvectors,
		    size, &zbeta, aux_matrix, size);
	// operator = eigenvectors.aux_matrix
	cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size,
		    size, &zalpha, complex_eigenvectors, size, aux_matrix,
		    size, &zbeta, operator, size);

	free(aux_matrix);
	free(complex_eigenvectors);
	free(inverse_complex_eigenvectors);

	return operator;
}


void set_thermal_equilibrium(DSPComplex *matrix, unsigned int spins, unsigned int spin_type_array, float gyroI, float gyroS)
{
	unsigned int i, size;
	DSPComplex factorI, factorS, *zmag;

	size = pow2(spins);

	if (matrix) {
		factorI = complex_rect(gyroI, 0);
		factorS = complex_rect(gyroS, 0);
		set_complex_zero_matrix(matrix, size);
		for (i = 0; i < spins; i++) {
			zmag = Iz(i, spins);
			if ((spin_type_array & pow2(i)) == 0)
				cblas_caxpy(size * size, &factorI, zmag, 1, matrix, 1);
			else
				cblas_caxpy(size * size, &factorS, zmag, 1, matrix, 1);
			free(zmag);
		}
	}
}


DSPComplex *matrix_from_base4_array(int *index, int spins, int number_of_operators, int spin_type_array, float *coefficient, float gyroI, float gyroS)
{
	int i, size_square, spin, mask;
	int all_spins_are_of_type_S;
	int transverse_S_spins_are_present;
	int number_of_transverse_spins;
	DSPComplex *return_matrix, *aux_matrix, *temp_matrix, *zmag, *xmag, *ymag, *identity, z;

	size_square = pow2(spins) * pow2(spins);
	return_matrix = malloc(size_square * sizeof(DSPComplex));
	set_complex_zero_matrix(return_matrix, pow2(spins));
	identity = malloc(4 * sizeof(DSPComplex));
	set_complex_identity_matrix(identity, 2);
	zmag = Iz(0, 1);
	xmag = Ix(0, 1);
	ymag = Iy(0, 1);

	for (i = 0; i < number_of_operators; i++) {
		if (index[i] != 0) {
			aux_matrix = malloc(sizeof(DSPComplex));
			aux_matrix->real = 1.0;
			aux_matrix->imag = 0.0;
			/****************************************************/
			/* Presence of S-spins scales operator to 0.25 if:  */
			/*  - All operator spins are of either type S or    */
			/*    are E/2 (pure spin states, multi-quantum      */
			/*    coherences)                                   */
			/*  - There is exactly one operator spin in the     */
			/*    x,y-plane and that is of type S (antiphase    */
			/*    magnetization)                                */
			/****************************************************/
			all_spins_are_of_type_S = 1;
			transverse_S_spins_are_present = 0;
			number_of_transverse_spins = 0;
			for (spin = 0; spin < spins; spin++) {
				/* Test for presence of I-spins */
				if ((spin_type_array & pow2(spin)) != 1) {
					/* Test if I-spin is not E/2 */
					if (component_from_base4_coded_product_operator(index[i], spin) != 0)
						all_spins_are_of_type_S = 0;
				}
				/* Test if S-spin is transverse magnetization */
				else if (component_from_base4_coded_product_operator(index[i], spin) > 1)
					transverse_S_spins_are_present = 1;

				/* Count number of transverse spins */
				if (component_from_base4_coded_product_operator(index[i], spin) > 1)
					number_of_transverse_spins++;

				/* Select spin from coded product operator */
				mask = component_from_base4_coded_product_operator(index[i], spin);
				switch (mask) {
				case 1:
					temp_matrix = kronecker_multiply(aux_matrix, pow2(spin), zmag, 2);
					break;
				case 2:
					temp_matrix = kronecker_multiply(aux_matrix, pow2(spin), xmag, 2);
					break;
				case 3:
					temp_matrix = kronecker_multiply(aux_matrix, pow2(spin), ymag, 2);
					break;
				default:
					temp_matrix = kronecker_multiply(aux_matrix, pow2(spin), identity, 2);
				}
				free(aux_matrix);
				aux_matrix = temp_matrix;
			}

			/* Scale factor if spin type is S (spinTypeS = 1) */
			if (all_spins_are_of_type_S || (transverse_S_spins_are_present && (number_of_transverse_spins == 1)))
				z = complex_rect(gyroS * coefficient[i], 0.0);
			else
				z = complex_rect(gyroI * coefficient[i], 0.0);

			/* Multiply operator by coefficient */
			cblas_cscal(size_square, &z, aux_matrix, 1);

			/* Add product operator to total density matrix */
			z = complex_rect(1.0, 0.0);
			cblas_caxpy(size_square, &z, aux_matrix, 1, return_matrix, 1);
			free(aux_matrix);
		}
	}
	free(zmag);
	free(xmag);
	free(ymag);
	free(identity);

	return return_matrix;
}

DSPComplex *relaxation_matrix(int spins, int spin_type_array, float *tensor, float tau, float gyroI, float gyroS, int transverse)
{
	int row, column, size, size_square;
	int i, j, type_i, type_j;
	int matrix_element;
	float W, b, omega, omega_partner, delta, sum, T2, multi_quantum_T2;
	float *Wsingle, Wzero, Wdouble, *R2;
	float gyro1, gyro2;
	DSPComplex *relaxation;
	DSPComplex *ymag, *identity;
	DSPComplex *higher_order_matrix, *aux_matrix;

	size = pow2(spins);
	size_square = size * size;
	relaxation = malloc(size_square * sizeof(DSPComplex));
	set_complex_zero_matrix(relaxation, size);
	ymag = Iy(0, 1);
	identity = malloc(4 * sizeof(DSPComplex));
	set_complex_identity_matrix(identity, 2);

	/* Calculate all transition probabilities */
	Wsingle = malloc(spins * sizeof(float));
	R2 = malloc(spins * sizeof(float));
	multi_quantum_T2 = (spins - 1) * 3.69;
	for (i = 0; i < spins; i++) {
		type_i = (spin_type_array & pow2(i)) >> i;
		omega = (type_i == (int)spinTypeS) ? spectrometer_frequency / 4 : spectrometer_frequency;
		omega += tensor[i * spins + i];
		/* W1 transitions are the sum of all transitions from one spin to all other coupled spins */
		/* W0 and W2 transitions exist between every spin pair */
		Wsingle[i] = 0;
		R2[i] = 0;
		for (j = 0; j < spins; j++) {
			if (i != j) {
				type_j = (spin_type_array & pow2(j)) >> j;
				/* Get frequency of the coupled spin */
				omega_partner = (type_j == (int)spinTypeS) ? spectrometer_frequency / 4 : spectrometer_frequency;
				omega_partner += tensor[j * spins + j];
				delta = omega - omega_partner;
				sum = omega + omega_partner;

				/* Get dipolar coupling constant b */
				gyro1 = (type_i == (int)spinTypeS) ? gyroS : gyroI;
				gyro2 = (type_j == (int)spinTypeS) ? gyroS : gyroI;
				if (j > i)
					b = dipolar_coupling_constant(tensor[j * spins + i], gyro1, gyro2);
				else if (j < i)
					b = dipolar_coupling_constant(tensor[i * spins + j], gyro1, gyro2);
				else
					b = 0;

				/* Calculate transition probabilities */
				Wsingle[i] += W1(b, tau, omega);
				Wzero = W0(b, tau, omega, omega_partner);
				Wdouble = W2(b, tau, omega, omega_partner);
				/* M. H. Levitt */
				/*R2[i] += 0.15 * pow(b, 2) * (3 * spectral_density(tau, delta)
				 + 5 * spectral_density(tau, omega_partner)
				 + 2 * spectral_density(tau, sum));*/
				/* R. R. Ernst, G. Bodenhausen, A. Wokaun */
				R2[i] += 0.05 * pow(b, 2) * (4 * spectral_density(tau, 0)
							     + spectral_density(tau, delta)
							     + 3 * spectral_density(tau, omega)
							     + 6 * spectral_density(tau, omega_partner) // 3
							     + 6 * spectral_density(tau, sum));

				/* Higher order transitions for every spin to every spin are needed */
				higher_order_matrix = malloc(sizeof(DSPComplex));
				higher_order_matrix->real = 1.0;
				higher_order_matrix->imag = 0.0;
				/* Build transition matrix for transition i <-> j */
				for (row = 0; row < spins; row++) {
					if ((row == spins - i - 1) || (row == spins - j - 1))
						aux_matrix = kronecker_multiply(ymag, 2, higher_order_matrix, pow2(row));
					else
						aux_matrix = kronecker_multiply(identity, 2, higher_order_matrix, pow2(row));
					free(higher_order_matrix);
					higher_order_matrix = aux_matrix;
				}
				for (row = 0; row < size; row++)
					for (column = 0; column < size;
					     column++) {
						/* In the product Iy (x) Iy the double quantum transitions are < 0 */
						if (higher_order_matrix [column * size + row].real < 0) {
							T2 = 1 / (0.05 * pow(b, 2) * (3 * spectral_density(tau, omega)
										      + 3 * spectral_density(tau, omega_partner)
										      + 12 * spectral_density(tau, sum)));
							if (transverse)
								relaxation[column * size + row] = complex_rect(2.25 * Wzero + 0.5 * Wdouble, T2);
							else
								relaxation[column * size + row] = complex_rect(Wdouble, T2);
						}
						/* while the zero quantum transitions are > 0 */
						else if (higher_order_matrix[column * size + row].real > 0) {
							T2 = 1 / (0.05 * pow(b, 2) * (3 * spectral_density(tau, omega)
										      + 3 * spectral_density(tau, omega_partner)
										      + 2 * spectral_density(tau, delta)));
							if (transverse)
								relaxation[column * size + row] = complex_rect(0.25 * Wzero, T2);
							else
								relaxation[column * size + row] = complex_rect(Wzero, T2);
						}
					}
				free(higher_order_matrix);
			}
		}
	}

	/* Construct relaxation matrix (single-quantum transitions and T2) */
	for (column = 0; column < size; column++) {
		/* Calculate cross relaxation */
		for (row = 0; row < size; row++) {
			matrix_element =
				abs(test_for_simple_coherence(column, row));
			/* single-quantum transitions */
			if (matrix_element != 0) {
				/* Determine which spin transition to calculate */
				matrix_element = spins - lb(matrix_element) - 1;
				/* Get Larmor frequency for the spin transition */
				W = Wsingle[matrix_element];
				if (transverse)
					W *= 1.5;
				T2 = 1 / R2[matrix_element];
				relaxation[column * size + row] =
					complex_rect(W, T2);
			}
			/* Also relax all other higher order coherences (Hack!) */
			else if ((relaxation[column * size + row].real == 0) && (relaxation[column * size + row].imag == 0)) {
				relaxation[column * size + row] =
					complex_rect(0, multi_quantum_T2);
			}
		}
		/* Calculate auto relaxation */
		for (row = 0; row < size; row++) {
			if (row != column) {
				relaxation[column * size + column].real -= relaxation[column * size + row].real;
			}
		}
	}
	free(ymag);
	free(identity);
	free(Wsingle);
	free(R2);

	return relaxation;
}


DSPComplex *solomon_transformation_matrix(int spins)
{
	int i, j, size;
	DSPComplex z, *operator, *temp, *z_operator, *identity_operator,
		   *transformation_matrix;

	size = pow2(spins);
	transformation_matrix = malloc(size * size * sizeof(DSPComplex));
	z_operator = Iz(0, 1);
	identity_operator = malloc(4 * sizeof(DSPComplex));
	set_complex_identity_matrix(identity_operator, 2);

	for (i = 0; i < size; i++) {
		/* Build longitudinal operator */
		operator = malloc(sizeof(DSPComplex));
		*operator = complex_rect(1, 0);
		for (j = 0; j < spins; j++) {
			if ((i & pow2(j)) != 0) {
				temp = kronecker_multiply(operator, pow2(j), z_operator, 2);
			} else {
				temp = kronecker_multiply(operator, pow2(j), identity_operator, 2);
			}
			free(operator);
			operator = temp;
		}
		/* Copy trace into transformation matrix */
		for (j = 0; j < size; j++) {
			transformation_matrix[i * size + j].real = operator[j * size + j].real;
			transformation_matrix[i * size + j].imag = operator[j * size + j].imag;
		}
		free(operator);
	}
	z = complex_rect(size, 0);
	cblas_cscal(size * size, &z, transformation_matrix, 1);
	free(z_operator);
	free(identity_operator);

	return transformation_matrix;
}


DSPComplex *irreducible_spherical_tensor(unsigned int spins, unsigned int array, unsigned int j, int m)
{
	unsigned int participatingSpins, dimension, dimSquared, spin1, spin2;
	DSPComplex *tensor, *firstMatrix, *secondMatrix, alpha, beta;

	dimension = pow2(spins);
	dimSquared = dimension * dimension;
	participatingSpins = numberOfSetBits(array);

	if (participatingSpins > spins) {
		printf("error: rank > number of spins in the system.\n");
		tensor = malloc(dimSquared * sizeof(DSPComplex));
		set_complex_identity_matrix(tensor, dimension);
	} else if (participatingSpins == 0) {   // || (participatingSpins == 1 && j == 0 && m == 0)) {
		// T(0,0) = E/2
		tensor = malloc(dimSquared * sizeof(DSPComplex));
		set_complex_identity_matrix(tensor, dimension);
	} else if (participatingSpins == 1 && j == 1 && abs(m) <= 1) {
		// T(1,m) = {I+, Iz, I-}
		switch (m) {
		case 1:
			tensor = Iplus(array, spins);
			break;
		case 0:
			tensor = Iz(array, spins);
			break;
		case -1:
			tensor = Iminus(array, spins);
			break;
		default:
			tensor = malloc(dimSquared * sizeof(DSPComplex));
			set_complex_identity_matrix(tensor, dimension);
			break;
		}
	} else if (participatingSpins == 2) {
		// tensors for 2-spin interactions: 1 x T(0,0), 3 x T(1,m), 5 x T(2,m)
		// determine interacting spins
		spin1 = 1;
		while (!(array & spin1)) {
			spin1 <<= 1;
		}
		spin2 = spin1 << 1;
		while (!(array & spin2)) {
			spin2 <<= 1;
		}
		spin1 = lb(spin1);
		spin2 = lb(spin2);
		tensor = malloc(dimSquared * sizeof(DSPComplex));
		if (j == 0 && m == 0) {
			alpha = complex_rect(1 / (2 * sqrt(3)), 0.0);
			beta = complex_rect(0.0, 0.0);
			firstMatrix = Iminus(spin1, spins);
			secondMatrix = Iplus(spin2, spins);
			cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
				    dimension, dimension, dimension, &alpha,
				    firstMatrix, dimension, secondMatrix,
				    dimension, &beta, tensor, dimension);
			free(firstMatrix);
			free(secondMatrix);
			firstMatrix = Iplus(spin1, spins);
			secondMatrix = Iminus(spin2, spins);
			beta = complex_rect(1.0, 0.0);
			cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
				    dimension, dimension, dimension, &alpha,
				    firstMatrix, dimension, secondMatrix,
				    dimension, &beta, tensor, dimension);
			free(firstMatrix);
			free(secondMatrix);
			alpha = complex_rect(-1 / sqrt(3), 0.0);
			firstMatrix = Iz(spin1, spins);
			secondMatrix = Iz(spin2, spins);
			cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
				    dimension, dimension, dimension, &alpha,
				    firstMatrix, dimension, secondMatrix,
				    dimension, &beta, tensor, dimension);
			free(firstMatrix);
			free(secondMatrix);
		} else if (j == 1 && abs(m) <= 1) {
			switch (abs(m)) {
			case 0:
				// Coherence order 0
				alpha = complex_rect(1 / (2 * sqrt(2)), 0.0);
				beta = complex_rect(0.0, 0.0);
				firstMatrix = Iminus(spin1, spins);
				secondMatrix = Iplus(spin2, spins);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				free(secondMatrix);
				firstMatrix = Iplus(spin1, spins);
				secondMatrix = Iminus(spin2, spins);
				alpha = complex_rect(-1 / (2 * sqrt(2)), 0.0);
				beta = complex_rect(1.0, 0.0);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				free(secondMatrix);
				break;
			case 1:
				// Coherence order 1
				beta = complex_rect(0.0, 0.0);
				if (m == -1) {
					alpha = complex_rect(0.5, 0.0);
					firstMatrix = Iminus(spin1, spins);
				} else if (m == 1) {
					alpha = complex_rect(-0.5, 0.0);
					firstMatrix = Iplus(spin1, spins);
				} else {
					firstMatrix = NULL;
				}
				secondMatrix = Iz(spin2, spins);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				if (firstMatrix != NULL)
					free(firstMatrix);
				free(secondMatrix);
				if (m == -1) {
					alpha = complex_rect(-0.5, 0.0);
					secondMatrix = Iminus(spin2, spins);
				} else if (m == 1) {
					alpha = complex_rect(0.5, 0.0);
					secondMatrix = Iplus(spin2, spins);
				} else {
					secondMatrix = NULL;
				}
				firstMatrix = Iz(spin1, spins);
				beta = complex_rect(1.0, 0.0);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				if (secondMatrix != NULL)
					free(secondMatrix);
				break;
			}
		} else if (j == 2 && abs(m) <= 2) {
			switch (abs(m)) {
			case 0:
				// Coherence order 0
				// T(0,0) =
				alpha = complex_rect(1 / (2 * sqrt(6)), 0.0);
				beta = complex_rect(0.0, 0.0);
				firstMatrix = Iminus(spin1, spins);
				secondMatrix = Iplus(spin2, spins);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				free(secondMatrix);
				firstMatrix = Iplus(spin1, spins);
				secondMatrix = Iminus(spin2, spins);
				beta = complex_rect(1.0, 0.0);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				free(secondMatrix);
				alpha = complex_rect(sqrt(2.0 / 3.0), 0.0);
				firstMatrix = Iz(spin1, spins);
				secondMatrix = Iz(spin2, spins);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				free(secondMatrix);
				break;
			case 1:
				// Coherence order 1
				// T(1,±1) =
				beta = complex_rect(0.0, 0.0);
				if (m == -1) {
					alpha = complex_rect(0.5, 0.0);
					firstMatrix = Iminus(spin1, spins);
				} else if (m == 1) {
					alpha = complex_rect(+0.5, 0.0);
					firstMatrix = Iplus(spin1, spins);
				} else {
					firstMatrix = NULL;
				}
				secondMatrix = Iz(spin2, spins);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				if (firstMatrix != NULL)
					free(firstMatrix);
				free(secondMatrix);
				if (m == -1) {
					secondMatrix = Iminus(spin2, spins);
				} else if (m == 1) {
					secondMatrix = Iplus(spin2, spins);
				} else {
					secondMatrix = NULL;
				}
				firstMatrix = Iz(spin1, spins);
				beta = complex_rect(1.0, 0.0);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				free(firstMatrix);
				if (secondMatrix != NULL)
					free(secondMatrix);
				break;
			case 2:
				// Coherence order 2
				if (m == -2) {
					firstMatrix = Iminus(spin1, spins);
					secondMatrix = Iminus(spin2, spins);
				} else if (m == 2) {
					firstMatrix = Iplus(spin1, spins);
					secondMatrix = Iplus(spin2, spins);
				} else {
					firstMatrix = NULL;
					secondMatrix = NULL;
				}
				alpha = complex_rect(0.5, 0.0);
				beta = complex_rect(0.0, 0.0);
				cblas_cgemm(CblasRowMajor, CblasNoTrans,
					    CblasNoTrans, dimension, dimension,
					    dimension, &alpha, firstMatrix,
					    dimension, secondMatrix, dimension,
					    &beta, tensor, dimension);
				if (firstMatrix != NULL)
					free(firstMatrix);
				if (secondMatrix != NULL)
					free(secondMatrix);
				break;
			}
		} else {
			printf
				("error: j or |m| > number of interacting spins.\n");
			tensor = malloc(dimSquared * sizeof(DSPComplex));
			set_complex_identity_matrix(tensor, dimension);
		}
	} else if (participatingSpins > 2) {
		printf
			("error: cannot compute for number of interacting spins > 2.\n");
		tensor = malloc(dimSquared * sizeof(DSPComplex));
		set_complex_identity_matrix(tensor, dimension);
	} else {
		printf
			("error: number of interacting spins does not match rank.\n");
		tensor = malloc(dimSquared * sizeof(DSPComplex));
		set_complex_identity_matrix(tensor, dimension);
	}

	return tensor;
}


char *irreducible_spherical_tensor_label(unsigned int spins, unsigned int array, unsigned int j, int m)
{
	char *label, *spinLabel;
	unsigned int participatingSpins, index, count;

	// determine interacting spins
	participatingSpins = numberOfSetBits(array);
	label = malloc(59 * sizeof(char));
	spinLabel = malloc(3 * sizeof(char));

	if (array >= (unsigned int)pow2(spins)) {
		sprintf(label, "error: interacting spin indices out of range");
	} else if (participatingSpins < j) {
		sprintf(label, "error: number of interacting spins < j");
	} else if (participatingSpins > spins) {
		sprintf(label,
			"error: number of interacting spins > total number of spins");
	} else if (abs(m) > j) {
		sprintf(label, "error: |m| > j");
	} else if (participatingSpins == 0
		   || (participatingSpins == 1 && j == 0 && m == 0)) {
		sprintf(label, "T[0,0]");
	} else {
		sprintf(label, "T[%d,%d]", j, m);
		if (participatingSpins <= spins) {
			strcat(label, "{");
			index = 1;
			count = 0;
			while (count < participatingSpins) {
				if (array & index) {
					if (count > 0)
						strcat(label, ",");
					sprintf(spinLabel, "%d", lb(index) + 1);
					strcat(label, spinLabel);
					count++;
				}
				index <<= 1;
			}
			strcat(label, "}");
		}
	}
	free(spinLabel);

	// Set script for the brackets as follows:
	// [l,m]       in subscript w/o brackets
	// (symmetry)  change to (τ₁...τ₁₀)
	// {spin,...}  leave as is, but add space

	// Examples:
	// T₂‚₋₂(τ₁){2,3}
	// T₀‚₀(τ₄){1,2,3}

	return label;
}


///    // ///    /// //////       //////  //////  /////// //////   /////  //////// //  //////  ///    // ///////
////   // ////  //// //   //     //    // //   // //      //   // //   //    //    // //    // ////   // //
// //  // // //// // //////      //    // //////  /////   //////  ///////    //    // //    // // //  // ///////
//  // // //  //  // //   //     //    // //      //      //   // //   //    //    // //    // //  // //      //
//   //// //      // //   //      //////  //      /////// //   // //   //    //    //  //////  //   //// ///////

void equation_of_motion(DSPComplex *matrix, DSPComplex *propagator, unsigned int size)
{
	DSPComplex alpha, beta, *temp;

	if (matrix == NULL)
		puts("pulse: matrix is not properly initialized!");
	else if (propagator == NULL)
		puts("pulse: propagator is not properly initialized!");
	else {
		alpha = complex_rect(1, 0);
		beta = complex_rect(0, 0);
		temp = malloc(size * size * sizeof(DSPComplex));
		cblas_cgemm(CblasColMajor, CblasNoTrans, CblasConjTrans, size,
			    size, size, &alpha, matrix, size, propagator, size,
			    &beta, temp, size);
		cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size,
			    size, size, &alpha, propagator, size, temp, size,
			    &beta, matrix, size);
		free(temp);
	}
}

void longitudinal_relaxation(DSPComplex *matrix, unsigned int size, int spin_type_array, float T1, float time, float gyroI, float gyroS)
{
	unsigned int spins, size_square;
	DSPComplex *equilibrium, *difference, z;

	if (matrix == NULL)
		puts("longitudinal_relaxation: matrix is not properly initialized!");
	else {
		spins = lb(size);
		size_square = size * size;
		equilibrium = malloc(size_square * sizeof(DSPComplex));
		set_thermal_equilibrium(equilibrium, spins, spin_type_array, gyroI, gyroS);
		z = complex_rect(-1, 0);
		cblas_cscal(size_square, &z, equilibrium, 1);
		difference = malloc(size_square * sizeof(DSPComplex));
		cblas_ccopy(size_square, matrix, 1, difference, 1);
		z = complex_rect(1, 0);
		cblas_caxpy(size_square, &z, equilibrium, 1, difference, 1);
		z = complex_rect(-1 / T1 * time, 0);
		cblas_cscal(size_square, &z, difference, 1);
		z = complex_rect(1, 0);
		cblas_caxpy(size_square, &z, difference, 1, matrix, 1);
		free(difference);
		free(equilibrium);
	}
}


void transverse_relaxation(DSPComplex *matrix, unsigned int size, float T2, float time)
{
	unsigned int row, column;
	DSPComplex dampening, matrix_element, z;

	if (matrix == NULL)
		puts("transverse_relaxation: matrix is not properly initialized!");
	else {
		dampening = complex_rect(-time / T2, 0);
		for (row = 0; row < size; row++)
			for (column = 0; column < size; column++)
				if (row != column) {
					matrix_element = matrix[column * size + row];
					z = complex_mul(matrix_element, dampening);
					matrix[column * size + row].real += z.real;
					matrix[column * size + row].imag += z.imag;
				}
	}
}

void transverse_longitudinal_relaxation(DSPComplex *matrix, unsigned int size, float T1rho, float time)
{
	unsigned int size_square;
	DSPComplex *identity, *difference, z;

	if (matrix == NULL)
		puts("transverse_longitudinal_relaxation: matrix is not properly initialized!");
	else {
		size_square = size * size;
		identity = malloc(size_square * sizeof(DSPComplex));
		set_complex_identity_matrix(identity, size);
		z = complex_rect(-1, 0);
		cblas_cscal(size_square, &z, identity, 1);
		difference = malloc(size_square * sizeof(DSPComplex));
		cblas_ccopy(size_square, matrix, 1, difference, 1);
		z = complex_rect(1, 0);
		cblas_caxpy(size_square, &z, identity, 1, difference, 1);
		z = complex_rect(-1 / T1rho * time, 0);
		cblas_cscal(size_square, &z, difference, 1);
		z = complex_rect(1, 0);
		cblas_caxpy(size_square, &z, difference, 1, matrix, 1);
		free(difference);
		free(identity);
	}
}


void dipolar_relaxation(DSPComplex *matrix, unsigned int size, unsigned int spin_type_array, DSPComplex *relaxation, float time, float gyroI, float gyroS)
{
	unsigned int spins, size_square, i, j, level;
	DSPComplex dampening, matrix_element, *equilibrium, z;
	float *populations, *after_relaxation;

	if (matrix == NULL)
		puts("dipolar_relaxation: matrix is not properly initialized!");
	else if (relaxation == NULL)
		puts("dipolar_relaxation: relaxation matrix is not properly initialized!");
	else if (size == 0)
		puts("spin system has a size of 0");
	else {
		spins = lb(size);
		size_square = size * size;

		/* Create vector with populations */
		equilibrium = malloc(size_square * sizeof(DSPComplex));
		set_thermal_equilibrium(equilibrium, spins, spin_type_array, gyroI, gyroS);
		populations = malloc(size * sizeof(float));
		for (i = 0; i < size; i++)
			populations[i] = matrix[i * size + i].real - equilibrium[i * size + i].real;
		free(equilibrium);

		/* Perform longitudinal relaxation and save populations into new vector */
		after_relaxation = malloc(size * sizeof(float));
		for (level = 0; level < size; level++) {
			after_relaxation[level] = 0;
			for (i = 0; i < size; i++)
				after_relaxation[level] += relaxation[i * size + level].real * time * populations[i];
		}
		/* Add difference vector to original matrix */
		for (i = 0; i < size; i++)
			matrix[i * size + i] = complex_rect(matrix[i * size + i].real + after_relaxation[i], 0);

		/* Perform transverse relaxation */
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				if (i != j) {
					if (relaxation[j * size + i].imag != 0)
						dampening = complex_rect(-time / relaxation[j * size + i].imag, 0);
					else
						dampening = complex_rect(0, 0);
					matrix_element = matrix[j * size + i];
					z = complex_mul(matrix_element, dampening);
					matrix[j * size + i].real += z.real;
					matrix[j * size + i].imag += z.imag;
				}
			}
		}
		free(populations);
		free(after_relaxation);
	}
}


void transverse_dipolar_relaxation(DSPComplex *matrix, unsigned int size, DSPComplex *relaxation, float time)
{
	unsigned int i, j, level;
	DSPComplex dampening, matrix_element, z;
	float *populations, *after_relaxation;

	if (matrix == NULL)
		puts("dipolar_relaxation: matrix is not properly initialized!");
	else if (relaxation == NULL)
		puts("dipolar_relaxation: relaxation matrix is not properly initialized!");
	else if (size == 0)
		puts("spin system has a size of 0");
	else {
		/* Create vector with populations */
		populations = malloc(size * sizeof(float));
		for (i = 0; i < size; i++)
			populations[i] = matrix[i * size + i].real;

		/* Perform longitudinal relaxation and save populations into new vector */
		after_relaxation = malloc(size * sizeof(float));
		for (level = 0; level < size; level++) {
			after_relaxation[level] = 0;
			for (i = 0; i < size; i++)
				after_relaxation[level] += relaxation[i * size + level].real * time * populations[i];
		}
		/* Add difference vector to original matrix */
		for (i = 0; i < size; i++)
			matrix[i * size + i] =
				complex_rect(matrix[i * size + i].real + after_relaxation[i], 0);

		/* Perform transverse relaxation */
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				if (i != j) {
					if (relaxation[j * size + i].imag != 0)
						dampening =
							complex_rect(-time /
								     relaxation[j * size + i].imag, 0);
					else
						dampening = complex_rect(0, 0);
					matrix_element = matrix[j * size + i];
					z = complex_mul(matrix_element, dampening);
					matrix[j * size + i].real += z.real;
					matrix[j * size + i].imag += z.imag;
				}
			}
		}
		free(populations);
		free(after_relaxation);
	}
}


 //////  //////   /////  //////  // /////// ///    // ////////      //////  //////  /////// //////   /////  //////// //  //////  ///    // ///////
//       //   // //   // //   // // //      ////   //    //        //    // //   // //      //   // //   //    //    // //    // ////   // //
//   /// //////  /////// //   // // /////   // //  //    //        //    // //////  /////   //////  ///////    //    // //    // // //  // ///////
//    // //   // //   // //   // // //      //  // //    //        //    // //      //      //   // //   //    //    // //    // //  // //      //
 //////  //   // //   // //////  // /////// //   ////    //         //////  //      /////// //   // //   //    //    //  //////  //   //// ///////

DSPComplex **create_matrix_array_for_gradients(DSPComplex *start_matrix, unsigned int size, unsigned int slices)
{
	int i, size_square;
	DSPComplex **matrix_array;

	size_square = size * size;
	matrix_array = malloc(slices * sizeof(DSPComplex *));
	for (i = 0; i < (int)slices; i++) {
		matrix_array[i] = malloc(size_square * sizeof(DSPComplex));
		cblas_ccopy(size_square, start_matrix, 1, matrix_array[i], 1);
	}

	return matrix_array;
}


void gradient(DSPComplex **matrix_array, float *coupling_matrix, unsigned int spins, int spin_type_array, float strength, float time, int slices)
{
	unsigned int i, j, k;
	unsigned int spinlist, array, size;
	float angle;
	DSPComplex alpha, beta, *temp, *operator;

	if (matrix_array == NULL)
		puts("gradient: gradient array is not properly initialized!");
	else if (coupling_matrix == NULL)
		puts("gradient: coupling matrix is not properly initialized!");
	else {
		size = pow2(spins);
		/* spinlist: convert position X from 1 to 0 if chemical shift has been performed */
		spinlist = size - 1;
		beta = complex_rect(0, 0);
		alpha = complex_rect(1, 0);
		time /= 1000;
		temp = malloc(size * size * sizeof(DSPComplex));
		for (i = 0; i < spins; i++) {
			/* Set array to 1 for spin number i + 1 */
			array = pow2(i);
			if (spinlist & array) {
				/* Test of chemical shift occurs more than once */
				for (j = i + 1; j < spins; j++)
					if ((coupling_matrix[i * spins + i] == coupling_matrix[j * spins + j])
					    && !(((spin_type_array & pow2(i)) >> i) ^ ((spin_type_array & pow2(j)) >> j))) {
						/* Add equivalent spin to array... */
						array += pow2(j);
						/* ...and delete it from the list */
						spinlist -= pow2(j);
					}
				// Calculate angle
				// add 500 MHz (laboratory frame) to make sure that Omega = 0 experiences gradient
				angle = (500e6 + coupling_matrix[i * spins + i]) * 360 * time;
				if ((spin_type_array & pow2(i)) > 0)
					angle *= 0.25;
				// Perform chemical Shift for every slice
				for (k = 0; k < (unsigned int)slices; k++) {
					operator = z_rotation(spins, array, angle * (k - 0.5 * slices - 0.5) * strength);
					cblas_cgemm(CblasColMajor, CblasNoTrans, CblasConjTrans, size, size,
						    size, &alpha, matrix_array[k], size, operator, size, &beta, temp, size);
					cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size,
						    size, &alpha, operator, size, temp, size, &beta, matrix_array[k], size);
					free(operator);
				}
			}
		}
		free(temp);
	}
}


void average_matrix_array(DSPComplex *matrix, DSPComplex **array, unsigned int size, unsigned int slices)
{
	unsigned int i, size_square;
	DSPComplex z;

	if (array == NULL)
		puts("average_matrix_array: array is not properly initialized!");
	else if (matrix == NULL)
		puts("average_matrix_array: matrix is not properly initialized!");
	else {
		size_square = size * size;
		set_complex_zero_matrix(matrix, size);
		z = complex_rect(1.0, 0.0);
		for (i = 0; i < slices; i++)
			cblas_caxpy(size_square, &z, array[i], 1, matrix, 1);
		z = complex_rect(1 / (float)slices, 0);
		cblas_cscal(size_square, &z, matrix, 1);
	}
}


/////// //////  ///////  ////// //////// //////  //    // ///    ///     //////  //////   //////   ////// /////// /////// /////// // ///    //  //////
//      //   // //      //         //    //   // //    // ////  ////     //   // //   // //    // //      //      //      //      // ////   // //
/////// //////  /////   //         //    //////  //    // // //// //     //////  //////  //    // //      /////   /////// /////// // // //  // //   ///
     // //      //      //         //    //   // //    // //  //  //     //      //   // //    // //      //           //      // // //  // // //    //
/////// //      ///////  //////    //    //   //  //////  //      //     //      //   //  //////   ////// /////// /////// /////// // //   ////  //////

void derivative(float *f, int N)
{
	int i, x1, x2;
	float *df;

	df = malloc(N * sizeof(float));
	for (i = 0; i < N; i++) {
		if (i == 0) {
			x1 = i;
		} else {
			x1 = i - 1;
		}
		if (i == N - 1) {
			x2 = i;
		} else {
			x2 = i + 1;
		}
		df[i] = (f[x2] - f[x1]) / (x2 - x1);
	}
	for (i = 0; i < N; i++) {
		f[i] = df[i];
	}
	free(df);
}


int peak_list(float *spectrum, int N, int *peaks, float threshold)
{
	/*
	 * The function will return the number of identified peaks.
	 * The peak indices will be saved directly in 'peaks' for
	 * positive peaks and multiplied by -1 for negative peaks.
	 * For indexing the spectrum the absolute value must be used!
	 *
	 * The array 'peaks' needs to be allocated for N/2+2 integers.
	 */

	int i, pos, n_peaks;
	float *deriv1, *deriv2, diff;

	if (peaks == NULL) {
		return -1;
	}
	deriv1 = malloc(N * sizeof(float));
	deriv2 = malloc(N * sizeof(float));
	/* Calculate first derivative */
	for (i = 0; i < N; i++) {
		deriv1[i] = spectrum[i];
	}
	derivative(deriv1, N);
	/* Calculate second derivative */
	for (i = 0; i < N; i++) {
		deriv2[i] = deriv1[i];
	}
	derivative(deriv2, N);
	/* Disregard changes below the third decimal place */
	for (i = 0; i < N; i++) {
		deriv2[i] = roundf(deriv2[i] * 1000);
	}
	/* Look for zero crossings */
	n_peaks = 0;
	for (i = 0; i < N - 1; i++) {
		diff = fabsf(deriv1[i] - deriv1[i + 1]);
		/* Check if there is a zero crossing between two data points */
		if (diff >= fabsf(deriv1[i]) && diff >= fabsf(deriv1[i + 1])) {
			/* Use the data point which is closer to zero */
			pos =
				(fabsf(deriv2[i]) >=
				 fabsf(deriv2[i + 1])) ? i : i + 1;
			/* Positive peaks: second derivative is < 0 and the closest data points are both larger */
			if (deriv2[pos] < 0 && spectrum[pos] > threshold) {
				peaks[n_peaks] = pos;
				n_peaks++;
			}
			/* Negative peaks: second derivative is > 0 and the closest data points are both smaller */
			if (deriv2[pos] > 0 && spectrum[pos] < -threshold) {
				peaks[n_peaks] = -pos;
				n_peaks++;
			}
			/* This check is deliberately sloppy. Uncomment checking for minima/maxima in the second
			   derivative and peak picking will not identify merged peaks due to low resolution. */
		}
	}
	free(deriv1);
	free(deriv2);

	return n_peaks;
}

void tilt_x(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints)
{
	int x, y, newX, totalDataPoints;
	float tilt;
	DSPSplitComplex tiltedDataSet;

	totalDataPoints = directDataPoints * indirectDataPoints;
	tiltedDataSet.realp = malloc(totalDataPoints * sizeof(float));
	tiltedDataSet.imagp = malloc(totalDataPoints * sizeof(float));

	for (y = 0; y < indirectDataPoints; y++) {
		tilt = (((float)indirectDataPoints / 2) - y) * (float)directDataPoints / (float)indirectDataPoints;
		for (x = 0; x < directDataPoints; x++) {
			newX = (int)(x + tilt);
			if (newX >= directDataPoints)
				newX -= directDataPoints;
			else if (newX < 0)
				newX += directDataPoints;
			tiltedDataSet.realp[y * directDataPoints + newX] = sourceDataSet.realp[y * directDataPoints + x];
			tiltedDataSet.imagp[y * directDataPoints + newX] = sourceDataSet.imagp[y * directDataPoints + x];
		}
	}
	for (x = 0; x < totalDataPoints; x++) {
		sourceDataSet.realp[x] = tiltedDataSet.realp[x];
		sourceDataSet.imagp[x] = tiltedDataSet.imagp[x];
	}
	free(tiltedDataSet.realp);
	free(tiltedDataSet.imagp);
}


void tilt_y(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints)
{
	int x, y, newY, totalDataPoints;
	float tilt;
	DSPSplitComplex tiltedDataSet;

	totalDataPoints = directDataPoints * indirectDataPoints;
	tiltedDataSet.realp = malloc(totalDataPoints * sizeof(float));
	tiltedDataSet.imagp = malloc(totalDataPoints * sizeof(float));

	for (y = 0; y < indirectDataPoints; y++) {
		for (x = 0; x < directDataPoints; x++) {
			tilt = (((float)directDataPoints / 2) - x) / (2 * (float)directDataPoints / (float)indirectDataPoints);
			newY = (int)(y + tilt);
			if (newY >= indirectDataPoints)
				newY -= indirectDataPoints;
			else if (newY < 0)
				newY += indirectDataPoints;
			tiltedDataSet.realp[newY * directDataPoints + x] = sourceDataSet.realp[y * directDataPoints + x];
			tiltedDataSet.imagp[newY * directDataPoints + x] = sourceDataSet.imagp[y * directDataPoints + x];
		}
	}
	for (x = 0; x < totalDataPoints; x++) {
		sourceDataSet.realp[x] = tiltedDataSet.realp[x];
		sourceDataSet.imagp[x] = tiltedDataSet.imagp[x];
	}
	free(tiltedDataSet.realp);
	free(tiltedDataSet.imagp);
}


void tilt_stretch_y(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints)
{
	int x, y, newY, totalDataPoints, quarterDataPoints;
	float tilt;
	DSPSplitComplex tiltedDataSet;

	totalDataPoints = directDataPoints * indirectDataPoints;
	tiltedDataSet.realp = malloc(totalDataPoints * sizeof(float));
	tiltedDataSet.imagp = malloc(totalDataPoints * sizeof(float));

	for (y = 0; y < indirectDataPoints; y++) {
		for (x = 0; x < directDataPoints; x++) {
			tilt = (((float)directDataPoints / 2) - x) / (2 * (float)directDataPoints / (float)indirectDataPoints);
			newY = (int)(y + tilt);
			if (newY >= indirectDataPoints)
				newY -= indirectDataPoints;
			else if (newY < 0)
				newY += indirectDataPoints;
			tiltedDataSet.realp[newY * directDataPoints + x] = sourceDataSet.realp[y * directDataPoints + x];
			tiltedDataSet.imagp[newY * directDataPoints + x] = sourceDataSet.imagp[y * directDataPoints + x];
		}
	}
	// SECSY stretching
	// magnify to show between 1/4 and 3/4 of the indirect dimension
	quarterDataPoints = indirectDataPoints / 4;
	for (y = quarterDataPoints; y < 3 * quarterDataPoints; y++) {
		newY = y - quarterDataPoints;
		for (x = 0; x < directDataPoints; x++) {
			sourceDataSet.realp[(2 * newY) * directDataPoints + x] = tiltedDataSet.realp[y * directDataPoints + x];
			sourceDataSet.imagp[(2 * newY) * directDataPoints + x] = tiltedDataSet.imagp[y * directDataPoints + x];
			sourceDataSet.realp[(2 * newY + 1) * directDataPoints + x] = tiltedDataSet.realp[y * directDataPoints + x];
			sourceDataSet.imagp[(2 * newY + 1) * directDataPoints + x] = tiltedDataSet.imagp[y * directDataPoints + x];
		}
	}
	free(tiltedDataSet.realp);
	free(tiltedDataSet.imagp);
}


void foldover_correction(DSPSplitComplex sourceDataSet, int directDataPoints, int indirectDataPoints)
{
	int x, y, newY, totalDataPoints, omega1, omega2, omegaN;
	DSPSplitComplex tiltedDataSet;

	totalDataPoints = directDataPoints * indirectDataPoints;
	omegaN = indirectDataPoints / 2;
	tiltedDataSet.realp = malloc(totalDataPoints * sizeof(float));
	tiltedDataSet.imagp = malloc(totalDataPoints * sizeof(float));

	for (y = 0; y < indirectDataPoints; y++) {
		for (x = 0; x < directDataPoints; x++) {
			omega1 = y - omegaN;
			omega2 = x - (directDataPoints / 2);
			newY = ((omega1 - omega2 + omegaN) % (2 * omegaN)) - omegaN;
			newY += omegaN; /* Obsolete with the last subtraction above */
			if (newY >= indirectDataPoints)
				newY -= indirectDataPoints;
			else if (newY < 0)
				newY += indirectDataPoints;
			printf("%d\n", newY);
			tiltedDataSet.realp[newY * directDataPoints + x] = sourceDataSet.realp[y * directDataPoints + x];
			tiltedDataSet.imagp[newY * directDataPoints + x] = sourceDataSet.imagp[y * directDataPoints + x];
		}
	}
	for (x = 0; x < totalDataPoints; x++) {
		sourceDataSet.realp[x] = tiltedDataSet.realp[x];
		sourceDataSet.imagp[x] = tiltedDataSet.imagp[x];
	}
	free(tiltedDataSet.realp);
	free(tiltedDataSet.imagp);
}


int multiple_lorentzian_peak_fit(float *spectrum, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters)
{
	/*
	 * dataset = 0: real spectrum
	 * dataset = 1: imaginary spectrum
	 * dataset = 2: absolute value
	 */

	int i, p, number_of_parameters;
	LMstat *lmstat;
	double *trace, *parameter, *parameter_grad;

	double (*function)(double *, int, void *) = NULL;
	void (*gradient)(double *, double *, int, void *) = NULL;

	function = &multiple_lorentzian_function;
	gradient = &multiple_lorentzian_grad;
	number_of_parameters = 3 * number_of_peaks + 1;
	trace = malloc(t2datapoints * sizeof(double));
	parameter = malloc(number_of_parameters * sizeof(double));
	parameter_grad = malloc(number_of_parameters * sizeof(double));
	lmstat = malloc(sizeof(LMstat));
	for (i = 0; i < t2datapoints; i++) {
		trace[i] = (double)spectrum[i];
	}
	parameter[0] = 0;
	for (i = 1; i < number_of_parameters; i += 3) {
		parameter[i] = 10.0;
		parameter[i + 1] = 5.0;
		parameter[i + 2] = fabs((double)dosy_parameters[(i - 1) / 3].x);
	}
	levmarq_init(lmstat);
	lmstat->max_it = 400;
	levmarq(number_of_parameters, parameter, t2datapoints, trace, NULL,
		function, gradient, &number_of_peaks, lmstat);

	for (i = 1; i < number_of_parameters; i += 3) {
		dosy_parameters[(i - 1) / 3].A = parameter[i];
		dosy_parameters[(i - 1) / 3].wx = parameter[i + 1];
	}

	/* Replace source spectrum with fit */
	for (i = 0; i < t2datapoints; i++) {
		spectrum[i] = 0.0;
		for (p = 1; p < number_of_parameters; p += 3) {
			spectrum[i] += parameter[p] * parameter[p + 1] / (pow(parameter[p + 1], 2) + pow(i - parameter[p + 2], 2));
		}
	}

	free(trace);
	free(parameter);
	free(parameter_grad);
	free(lmstat);

	return 1;
}

int dosy_fit(float *spectrum, int t1datapoints, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters)
{
	int x, y, peak;
	unsigned int exponentials = 1;
	int number_of_parameters = 2 * exponentials + 1;
	double *trace, *parameter, *parameter_grad;
	LMstat *lmstat;

	double (*function)(double *, int, void *) = NULL;
	void (*gradient)(double *, double *, int, void *) = NULL;

	trace = malloc(t1datapoints * sizeof(double));
	parameter = malloc(number_of_parameters * sizeof(double));
	parameter_grad = malloc(number_of_parameters * sizeof(double));
	lmstat = malloc(sizeof(LMstat));
	function = &monoexponential_function;
	gradient = &monoexponential_grad;
	for (peak = 0; peak < (int)number_of_peaks; peak++) {
		x = dosy_parameters[peak].x;
		for (y = 0; y < t1datapoints; y++) {
			trace[y] = spectrum[y * t2datapoints + x];
		}
		parameter[0] = trace[t1datapoints - 1];
		parameter[1] = trace[0] - parameter[0];
		parameter[2] = 1.0;
		levmarq_init(lmstat);
		lmstat->max_it = 400;
		// The first datapoint is rubbish and messes up the fit!
		levmarq(number_of_parameters, parameter, t1datapoints - 1, trace + 1, NULL, function, gradient, NULL, lmstat);
		if (exponentials >= 1) {
			if (parameter[2] != parameter[2]) {     // Check for NaN
				dosy_parameters[peak].y = 0;
				dosy_parameters[peak].wy = 0;
			} else {
				dosy_parameters[peak].y =
					(int)(parameter[2] * t2datapoints / 2);
				dosy_parameters[peak].wy =
					lmstat->final_err * 2;
			}
		}
	}
	free(trace);
	free(parameter);
	free(parameter_grad);
	free(lmstat);

	return 1;
}


void dosy_spectrum(float *spectrum, int t1datapoints, int t2datapoints, unsigned int number_of_peaks, FittedSpectrum *dosy_parameters)
{
	int x, y, peak;
	float value;

	//int logscale = 0;

	for (y = 0; y < t1datapoints; y++) {
		for (x = 0; x < t2datapoints; x++) {
			value = 0.0;
			/*if(logscale) {
			   for(peak = 0; peak < number_of_peaks; peak++) {
			   value += dosy_parameters[peak].A * dosy_parameters[peak].wx / (pow(dosy_parameters[peak].wx, 2) + pow(x - dosy_parameters[peak].x, 2))
			 * logf(dosy_parameters[peak].wy) / (pow(logf(dosy_parameters[peak].wy), 2) + pow(logf(y) - logf(dosy_parameters[peak].y), 2));
			   }
			   } else { */
			for (peak = 0; peak < (int)number_of_peaks; peak++) {
				value += dosy_parameters[peak].A * dosy_parameters[peak].wx / (pow(dosy_parameters[peak].wx, 2) +
											       pow(x - dosy_parameters[peak].x, 2)) * dosy_parameters[peak].wy / (pow(dosy_parameters[peak].wy, 2) + pow(y - dosy_parameters[peak].y, 2));
			}
			//}
			spectrum[y * t2datapoints + x] = value;
		}
	}
}


double monoexponential_function(double *p, int x, void *fdata)
{
	/*
	 * Monoexponential function for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + A * exp(-b * x)
	 *
	 * p[0] = y0, constant baseline
	 * p[1] = A, amplitude
	 * p[2] = b, exponential factor
	 */

	return p[0] + p[1] * exp(-p[2] * x);
}


void monoexponential_grad(double *g, double *p, int x, void *fdata)
{
	/*
	 * Monoexponential derivatives for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + A * exp(-b * x)
	 *
	 * g[0] = df/dy0 = 1
	 * g[1] = df/dA = exp(-b * x)
	 * g[2] = df/db = -A * x * exp(-b * x)
	 */

	g[0] = 1;
	g[1] = exp(-p[2] * x);
	g[2] = -p[1] * x * exp(-p[2] * x);
}


double biexponential_function(double *p, int x, void *fdata)
{
	/*
	 * Biexponential function for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + A1 * exp(-b1 * x) + A2 * exp(-b2 * x)
	 *
	 * p[0] = y0, constant baseline
	 * p[1] = A1, amplitude of function 1
	 * p[2] = b1, exponential factor for function 1
	 * p[3] = A2, amplitude of function 2
	 * p[4] = b2, exponential factor for function 2
	 */

	return p[0] + p[1] * exp(-p[2] * x) + p[3] * exp(-p[4] * x);
}


void biexponential_grad(double *g, double *p, int x, void *fdata)
{
	/*
	 * Biexponential derivatives for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + A1 * exp(-b1 * x) + A2 * exp(-b2 * x)
	 *
	 * g[0] = df/dy0 = 1
	 * g[1] = df/dA1 = exp(-b1 * x)
	 * g[2] = df/db1 = -A1 * x * exp(-b1 * x)
	 * g[3] = df/dA2 = exp(-b2 * x)
	 * g[4] = df/db2 = -A2 * x * exp(-b2 * x)
	 */

	g[0] = 1;
	g[1] = exp(-p[2] * x);
	g[2] = -p[1] * x * exp(-p[2] * x);
	g[3] = exp(-p[4] * x);
	g[4] = -p[3] * x * exp(-p[4] * x);
}


double multiple_lorentzian_function(double *p, int x, void *fdata)
{
	/*
	 * Biexponential function for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + Σ A₁ * λ₁ / (λ₁² + (x - Ω₁)²)
	 *
	 * fdata is int with number of peaks
	 *
	 * p[0] = y0, constant baseline
	 * p[1] = A₁, amplitode for function 1
	 * p[2] = λ₁, width of function 1
	 * p[3] = Ω₁, shift for function 1
	 * ...
	 */

	int i, *count;
	float value = p[0];

	count = fdata;
	for (i = 1; i <= 3 * *count; i += 3) {
		value +=
			p[i] * p[i + 1] / (pow(p[i + 1], 2) + pow(x - p[i + 2], 2));
	}

	return value;
}


void multiple_lorentzian_grad(double *g, double *p, int x, void *fdata)
{
	/*
	 * Biexponential derivatives for Levenberg-Marquardt algorithm
	 *
	 * f(x) = y0 + Σ A₁ * λ₁ / (λ₁² + (x - Ω₁)²)
	 *
	 * fdata is int with number of peaks
	 *
	 * g[0] = df/dy0 = 1
	 * g[1] = df/dA₁ = λ₁ / (λ₁² + (x - Ω₁)²)
	 * g[2] = df/dλ₁ = -2 * A₁ * λ₁² / (λ₁² + (x - Ω₁)²)² + A₁ / (λ₁² + (x - Ω₁)²)
	 * g[3] = df/dΩ₁ = 2 * A₁ * λ₁ * (x - Ω₁) / (λ₁² + (x - Ω₁)²)²
	 * ...
	 */

	int i, *count;
	double denominator;

	count = fdata;
	g[0] = 1;
	for (i = 1; i <= 3 * *count; i += 3) {
		denominator = (pow(p[i + 1], 2) + pow(x - p[i + 2], 2));
		g[i] = p[i + 1] / denominator;
		g[i + 1] =
			-2 * p[i] * pow(p[i + 1], 2) / pow(denominator,
							   2) + p[i] / denominator;
		g[i + 2] =
			2 * p[i] * p[i + 1] * (x - p[i + 2]) / pow(denominator, 2);
	}
}


// ///    // //////  //    // ////////     //  //////  //    // //////// //////  //    // ////////
// ////   // //   // //    //    //       //  //    // //    //    //    //   // //    //    //
// // //  // //////  //    //    //      //   //    // //    //    //    //////  //    //    //
// //  // // //      //    //    //     //    //    // //    //    //    //      //    //    //
// //   //// //       //////     //    //      //////   //////     //    //       //////     //

void string_for_complex_number(char *string, DSPComplex z)
{
	char number[10];

	if (string != NULL) {
		/* If Im(z) == 0 move cursor 5 positions */
		if ((z.imag > -0.005) && (z.imag < 0.005))
			strcat(string, "      ");
		/* If Re(z) is 0 or +/- 1 move cursor 3 positions */
		if (((z.real < -0.995) && (z.real > -1.005))
		    || ((z.real >= 0.995) && (z.real < 1.005))
		    || ((z.real > -0.005) && (z.real < 0.005)))
			strcat(string, "   ");
		/* If Re(z) < 0 print sign(Re(z)), else print blank */
		if (z.real <= -0.005)
			strcat(string, "-");
		else
			strcat(string, " ");
		/* Test if Re(z) is 0 */
		if ((z.real > -0.005) && (z.real < 0.005)) {
			if ((z.imag > -0.005) && (z.imag < 0.005))
				strcat(string, "0");
			else {
				strcat(string, " ");
				if (((z.imag < -0.995) && (z.imag > -1.005))
				    || ((z.imag >= 0.995) && (z.imag < 1.005)))
					strcat(string, "   ");
			}
		}
		/* Test if Re(z) is +/- 1 */
		else if (((z.real < -0.995) && (z.real > -1.005))
			 || ((z.real >= 0.995) && (z.real < 1.005)))
			strcat(string, "1");
		/* Else print Re(z) with 2 decimal places */
		else if (z.real <= -0.005) {
			sprintf(number, "%.2f", -z.real);
			strcat(string, number);
		} else {
			sprintf(number, "%.2f", z.real);
			strcat(string, number);
		}
		/* If Re(z) != 0 print sign(Im(z)), else print blank */
		if (z.imag <= -0.005) {
			if ((z.imag < -0.995) && (z.imag > -1.005))
				strcat(string, " ");
			else
				strcat(string, "-");
		} else if (z.imag >= 0.005) {
			if ((z.real > -0.005) && (z.real < 0.005))
				strcat(string, " ");
			else
				strcat(string, "+");
		}
		/* Test if Im(z) != 0 */
		if (!((z.imag > -0.005) && (z.imag < 0.005))) {
			/* Test if Im(z) is -1 */
			if ((z.imag < -0.995) && (z.imag > -1.005))
				strcat(string, "-i");
			/* Test if Im(z) is +1 */
			else if ((z.imag >= 0.995) && (z.imag < 1.005))
				strcat(string, " i");
			/* Else print Im(z) with 2 decimal places */
			else if (z.imag <= -0.005) {
				sprintf(number, "%.2fi", -z.imag);
				strcat(string, number);
			} else {
				sprintf(number, "%.2fi", z.imag);
				strcat(string, number);
			}
		}
	}
}


void string_for_double_complex_number(char *string, DSPDoubleComplex z)
{
	char number[10];

	if (string != NULL) {
		/* If Im(z) == 0 move cursor 5 positions */
		if ((z.imag > -0.005) && (z.imag < 0.005))
			strcat(string, "      ");
		/* If Re(z) is 0 or +/- 1 move cursor 3 positions */
		if (((z.real < -0.995) && (z.real > -1.005))
		    || ((z.real >= 0.995) && (z.real < 1.005))
		    || ((z.real > -0.005) && (z.real < 0.005)))
			strcat(string, "   ");
		/* If Re(z) < 0 print sign(Re(z)), else print blank */
		if (z.real <= -0.005)
			strcat(string, "-");
		else
			strcat(string, " ");
		/* Test if Re(z) is 0 */
		if ((z.real > -0.005) && (z.real < 0.005)) {
			if ((z.imag > -0.005) && (z.imag < 0.005))
				strcat(string, "0");
			else {
				strcat(string, " ");
				if (((z.imag < -0.995) && (z.imag > -1.005))
				    || ((z.imag >= 0.995) && (z.imag < 1.005)))
					strcat(string, "   ");
			}
		}
		/* Test if Re(z) is +/- 1 */
		else if (((z.real < -0.995) && (z.real > -1.005))
			 || ((z.real >= 0.995) && (z.real < 1.005)))
			strcat(string, "1");
		/* Else print Re(z) with 2 decimal places */
		else if (z.real <= -0.005) {
			sprintf(number, "%.2f", -z.real);
			strcat(string, number);
		} else {
			sprintf(number, "%.2f", z.real);
			strcat(string, number);
		}
		/* If Re(z) != 0 print sign(Im(z)), else print blank */
		if (z.imag <= -0.005) {
			if ((z.imag < -0.995) && (z.imag > -1.005))
				strcat(string, " ");
			else
				strcat(string, "-");
		} else if (z.imag >= 0.005) {
			if ((z.real > -0.005) && (z.real < 0.005))
				strcat(string, " ");
			else
				strcat(string, "+");
		}
		/* Test if Im(z) != 0 */
		if (!((z.imag > -0.005) && (z.imag < 0.005))) {
			/* Test if Im(z) is -1 */
			if ((z.imag < -0.995) && (z.imag > -1.005))
				strcat(string, "-i");
			/* Test if Im(z) is +1 */
			else if ((z.imag >= 0.995) && (z.imag < 1.005))
				strcat(string, " i");
			/* Else print Im(z) with 2 decimal places */
			else if (z.imag <= -0.005) {
				sprintf(number, "%.2fi", -z.imag);
				strcat(string, number);
			} else {
				sprintf(number, "%.2fi", z.imag);
				strcat(string, number);
			}
		}
	}
}


void print_real_matrix(float *matrix, unsigned int size)
{
	unsigned int m, n;

	if (matrix == NULL)
		printf("print_real_matrix: matrix not properly initialized!\n");
	else {
		printf("\n");
		for (m = 0; m < size; m++) {
			printf("|  ");
			for (n = 0; n < size; n++) {
				if (matrix[n * size + m] > -0.01
				    && matrix[n * size + m] < 0.01)
					printf(" 0     ");
				else {
					if (matrix[n * size + m] > 0)
						printf(" ");
					printf("%.2f  ", matrix[n * size + m]);
				}
			}
			printf("|\n");
		}
		printf("\n");
	}
}


void print_double_matrix(double *matrix, unsigned int size)
{
	unsigned int m, n;

	if (matrix == NULL)
		printf("print_real_matrix: matrix not properly initialized!\n");
	else {
		printf("\n");
		for (m = 0; m < size; m++) {
			printf("|  ");
			for (n = 0; n < size; n++) {
				if (matrix[n * size + m] > -0.01
				    && matrix[n * size + m] < 0.01)
					printf(" 0     ");
				else {
					if (matrix[n * size + m] > 0)
						printf(" ");
					printf("%.2f  ", matrix[n * size + m]);
				}
			}
			printf("|\n");
		}
		printf("\n");
	}
}


void print_complex_matrix(DSPComplex *matrix, unsigned int size)
{
	unsigned int m, n;
	char matrixElementString[20];

	if (matrix == NULL)
		printf
			("print_complex_matrix: matrix not properly initialized!\n");
	else {
		printf("\n");
		for (m = 0; m < size; m++) {
			printf("|  ");
			for (n = 0; n < size; n++) {
				strcpy(matrixElementString, "\0");
				string_for_complex_number(matrixElementString,  matrix[n * size + m]);
				printf("%s  ", matrixElementString);
			}
			printf("|\n");
		}
		printf("\n");
	}
}


void print_double_complex_matrix(DSPDoubleComplex *matrix, unsigned int size)
{
	unsigned int m, n;
	char matrixElementString[20];

	if (matrix == NULL)
		printf
			("print_complex_matrix: matrix not properly initialized!\n");
	else {
		printf("\n");
		for (m = 0; m < size; m++) {
			printf("|  ");
			for (n = 0; n < size; n++) {
				strcpy(matrixElementString, "\0");
				string_for_double_complex_number(matrixElementString, matrix[n * size + m]);
				printf("%s  ", matrixElementString);
			}
			printf("|\n");
		}
		printf("\n");
	}
}


char *cartesian_product_operators(DSPComplex *matrix, int size, int spin_type_array)
{
	char *return_string, *po_string, *aux_string, *temp_string, spin_type_char;
    int return_string_len, po_string_len, aux_string_len, temp_string_len;
	int number_of_spins, spins_in_operator, spin, index, mask, m, n;
	float value;
	DSPComplex *po_matrix, z;
	DSPComplex **zmag, **xmag, **ymag;

	number_of_spins = lb(size);
	po_matrix = malloc(size * size * sizeof(DSPComplex));

	/* Initialize base matrices for all spins 1 - n */
	zmag = malloc(number_of_spins * sizeof(DSPComplex *));
	xmag = malloc(number_of_spins * sizeof(DSPComplex *));
	ymag = malloc(number_of_spins * sizeof(DSPComplex *));
	for (spin = 0; spin < number_of_spins; spin++) {
		zmag[spin] = Iz(spin, number_of_spins);
		xmag[spin] = Ix(spin, number_of_spins);
		ymag[spin] = Iy(spin, number_of_spins);
	}

    temp_string_len = 13;
    aux_string_len = 5 * number_of_spins;
    po_string_len = aux_string_len + 9;
    return_string_len = po_string_len * (pow(4, number_of_spins) + 1);

    /* return_spin takes the full string of product operators */
	return_string = calloc(return_string_len, sizeof(char));
	strcat(return_string, "E/2");
	/* po_string takes the string for one product operator */
	po_string = malloc(po_string_len * sizeof(char));
    /* aux_string takes the combined product operator components */
	aux_string = malloc(aux_string_len * sizeof(char));
    /* temp_string takes one component of a product operator (e.g. I_1z) */
	temp_string = malloc(temp_string_len * sizeof(char));

	/**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-4    */
	/*   system is used:                                      */
	/*   With the basis vectors coded as 0 = 0, z = 1, x = 2  */
	/*   and y = 3. The product operator 4 I_2z I3_y in a     */
	/*   system of three spins would be encoded as:           */
	/*     index = 310 (base-4) = 28 (base-10)                */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/
	for (index = 1; index < pow(4, number_of_spins); index++) {
		/* Initialize po_matrix to identity matrix */
		for (m = 0; m < size; m++) {
			for (n = 0; n < size; n++) {
				if (m == n)
					po_matrix[n * size + m] = complex_rect(1 / pow(2, number_of_spins), 0);
				else
					po_matrix[n * size + m] = complex_rect(0, 0);
			}
        }
		memset(po_string, '\0', po_string_len);
        memset(aux_string, '\0', aux_string_len);
		memset(temp_string, '\0', temp_string_len);
		spins_in_operator = 0;
		for (spin = 0; spin <= number_of_spins; spin++) {
			/* Select spin from base4 index by mask */
			mask = component_from_base4_coded_product_operator(index, spin);
			/* Chose between I and S spin character */
			if ((spin_type_array & pow2(spin)) >> spin == spinTypeS)
				spin_type_char = 'S';
			else
				spin_type_char = 'I';
			/* multiply E/2, Iz, Ix or Iy to product operator */
			switch (mask) {
			case BASIS_Z:
				matrix_multiply(po_matrix, zmag[spin], size);
				sprintf(temp_string, "%c%dz", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				break;
			case BASIS_X:
				matrix_multiply(po_matrix, xmag[spin], size);
				sprintf(temp_string, "%c%dx", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				break;
			case BASIS_Y:
				matrix_multiply(po_matrix, ymag[spin], size);
				sprintf(temp_string, "%c%dy", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				break;
			}
		}
		/**********************************************************/
		/*                                                        */
		/*   The factor to with which the expectation value for   */
		/*   a product of one-spin matrix operators must be mul-  */
		/*   tiplied to get the corresponding coefficient for     */
		/*   the product operator is calculated from the number   */
		/*   of spins in the system (s) and the number of factors */
		/*   in the product operator (n) by a power of 2:         */
		/*                                                        */
		/*                  f = 2^(s * (n + 1))                   */
		/*                                                        */
		/*   s is the variable named "number_of_spins"            */
		/*   n is the variable named "spins_in_operator"          */
		/*                                                        */
		/**********************************************************/
		z = expectation_value(matrix, po_matrix, size);
		value = pow2(number_of_spins * (spins_in_operator + 1)) * z.real;

		/* format the product operator coefficient */
		if ((value >= 0.005) || (value < -0.005)) {
			if (value <= -0.005)
				strcat(po_string, " -");
			else if (value >= 0.005)
				strcat(po_string, " +");
			if (((value >= -1.005) && (value < -0.995)) || ((value >= 0.995) && (value < 1.005)))
				strcat(po_string, " ");
			else {
				sprintf(temp_string, " %.2f ", fabs(value));
				strcat(po_string, temp_string);
			}
			if (spins_in_operator > 1) {
				sprintf(temp_string, "%d", pow2(spins_in_operator - 1));
				strcat(po_string, temp_string);
			}
			strcat(po_string, aux_string);
			strcat(return_string, po_string);
		}
	}

	/* Free base matrices for all spins 1 - n */
	for (spin = 0; spin < number_of_spins; spin++) {
		free(zmag[spin]);
		free(xmag[spin]);
		free(ymag[spin]);
	}
	free(zmag);
	free(xmag);
	free(ymag);
	free(po_matrix);
	free(po_string);
	free(aux_string);
	free(temp_string);

	return return_string;
}


char *spherical_product_operators(DSPComplex *matrix, int size, int spin_type_array)
{
	char *return_string, *po_string, *aux_string, *temp_string, spin_type_char;
    int return_string_len, po_string_len, aux_string_len, temp_string_len;
	int number_of_spins, spins_in_operator, spin, index, mask, m, n;
    int number_shift_operators;
	float resize_factor, arg_value, abs_value;
	DSPComplex *po_matrix, z;
	DSPComplex **zmag, **shiftplus, **shiftminus;

	number_of_spins = lb(size);
	po_matrix = malloc(size * size * sizeof(DSPComplex));

	/* Initialize base matrices for all spins 1 - n */
	zmag = malloc(number_of_spins * sizeof(DSPComplex *));
	shiftplus = malloc(number_of_spins * sizeof(DSPComplex *));
	shiftminus = malloc(number_of_spins * sizeof(DSPComplex *));
	for (spin = 0; spin < number_of_spins; spin++) {
		zmag[spin] = Iz(spin, number_of_spins);
		shiftplus[spin] = Iplus(spin, number_of_spins);
		z = complex_rect(-1, 0);
		cblas_cscal(size * size, &z, shiftplus[spin], 1);
		shiftminus[spin] = Iminus(spin, number_of_spins);
	}

    temp_string_len = 6;
    aux_string_len = 5 * number_of_spins + 1;
    po_string_len = aux_string_len + 20;
    return_string_len = po_string_len * (pow(4, number_of_spins) + 1);

    /* return_spin takes the full string of product operators */
	return_string = calloc(return_string_len, sizeof(char));
	strcat(return_string, "E/2");
	/* po_string takes the string for one product operator */
	po_string = malloc(po_string_len * sizeof(char));
    /* aux_string takes the combined product operator components */
	aux_string = malloc(aux_string_len * sizeof(char));
    /* temp_string takes one component of a product operator (e.g. I_1z) */
	temp_string = malloc(temp_string_len * sizeof(char));

	/**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-4    */
	/*   system is used:                                      */
	/*   With the basis vectors coded as 0 = 0, z = 1, x = 2  */
	/*   and y = 3. The product operator 4 I_2z I3_y in a     */
	/*   system of three spins would be encoded as:           */
	/*     index = 310 (base-4) = 28 (base-10)                */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/
	for (index = 1; index < pow(4, number_of_spins); index++) {
		/* Initialize po_matrix to identity matrix */
		for (m = 0; m < size; m++) {
			for (n = 0; n < size; n++) {
				if (m == n)
					po_matrix[n * size + m] = complex_rect(1 / pow(2, number_of_spins), 0);
				else
					po_matrix[n * size + m] = complex_rect(0, 0);
			}
        }
		memset(po_string, '\0', po_string_len);
        memset(aux_string, '\0', aux_string_len);
		memset(temp_string, '\0', temp_string_len);
		spins_in_operator = 0;
        number_shift_operators = 0;
		for (spin = 0; spin <= number_of_spins; spin++) {
			/* Select spin from base4 index by mask */
			mask = component_from_base4_coded_product_operator(index, spin);
			/* Chose between I and S spin character */
			if ((spin_type_array & pow2(spin)) >> spin == spinTypeS)
				spin_type_char = 'S';
			else
				spin_type_char = 'I';
			/* multiply E/2, Iz, I+ or I- to product operator */
			switch (mask) {
			case BASIS_Z:
				matrix_multiply(po_matrix, zmag[spin], size);
				sprintf(temp_string, "%c%dz", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				break;
			case BASIS_X:
				matrix_multiply(po_matrix, shiftplus[spin], size);
				sprintf(temp_string, "%c%d[+]", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				number_shift_operators++;
				break;
			case BASIS_Y:
				matrix_multiply(po_matrix, shiftminus[spin], size);
				sprintf(temp_string, "%c%d[-]", spin_type_char, spin + 1);
				strcat(aux_string, temp_string);
				spins_in_operator++;
				number_shift_operators++;
				break;
			}
		}
		/**********************************************************/
		/*                                                        */
		/*   The factor to with which the expectation value for   */
		/*   a product of one-spin matrix operators must be mul-  */
		/*   tiplied to get the corresponding coefficient for     */
		/*   the product operator is calculated from the number   */
		/*   of spins in the system (s) and the number of factors */
		/*   in the product operator (n) by a power of 2:         */
		/*                                                        */
		/*                  f = 2^(s * (n + 1))                   */
		/*                                                        */
		/*   s is the variable named "number_of_spins"            */
		/*   n is the variable named "spins_in_operator"          */
		/*                                                        */
		/**********************************************************/
		resize_factor = 0.5 * pow(sqrt(2), (number_shift_operators + 2 * (spins_in_operator - number_shift_operators)));
		z = expectation_value(matrix, po_matrix, size);
		abs_value = hypotf(z.real, z.imag);
		if ((z.real == 0) && (z.imag == 0))
			arg_value = 0;
		else if ((z.real < 0) && (z.imag == 0))
			arg_value = M_PI;
		else
			arg_value = 2 * atan(z.imag / (abs_value + z.real));
		abs_value *= pow2(number_of_spins * (spins_in_operator + 1)) * resize_factor;

		if ((number_shift_operators == 0) && (arg_value >= 3.135) && (arg_value) < 3.145)
			abs_value *= -1;

		if ((abs_value < -0.005) || (abs_value >= 0.005)) {
			/* Print appropriate sign */
			if (abs_value < -0.005)
				strcat(po_string, " - ");
			else
				strcat(po_string, " + ");

			/* Print Abs value */
			if ((fabsf(abs_value) < 0.708) && (fabsf(abs_value) >= 0.706)) {
				sprintf(temp_string, "1/√2 ");
				strcat(po_string, temp_string);
			} else if ((fabsf(abs_value) < 1.42) && (fabsf(abs_value) >= 1.41)) {
				sprintf(temp_string, "√2 ");
				strcat(po_string, temp_string);
			} else if ((fabsf(abs_value) < 0.995) || (fabsf(abs_value) >= 1.005)) {
				sprintf(temp_string, "%.2f ", fabsf(abs_value));
				strcat(po_string, temp_string);
			} else if ((abs_value < -0.005 && abs_value > -0.995) || abs_value < -1.005)
				if (fabsf(arg_value) < 0.005)
					strcat(po_string, " ");

			/* Print Arg value */
			if (abs_value > 0.005) {
				if (fabsf(arg_value) >= 0.005 && fabsf(arg_value) < 3.136) {
					strcat(po_string, "exp(");
					if ((arg_value < -0.005))
						strcat(po_string, "-");
					if ((fabsf(arg_value) >= 0.995) && (fabsf(arg_value) < 1.005))
						strcat(po_string, "i");
					else if ((fabsf(arg_value) >= 1.565) && (fabsf(arg_value) < 1.575))
						strcat(po_string, "ipi/2");
					else if ((fabsf(arg_value) >= 1.995) && (fabsf(arg_value) < 2.005))
						strcat(po_string, "2i");
					else if ((fabsf(arg_value) >= 2.995) && (fabsf(arg_value) < 3.005))
						strcat(po_string, "3i");
					/*else if ((fabsf(arg_value) >= 3.135) && (fabsf(arg_value) < 3.145))
						strcat(po_string, "ipi");*/
					else {
						sprintf(temp_string, "%.2fi", fabsf(arg_value));
						strcat(po_string, temp_string);
					}
					strcat(po_string, ") ");
				}
			}
			strcat(po_string, aux_string);
            strcat(return_string, po_string);
		}
	}

	/* Free base matrices for all spins 1 - n */
	for (spin = 0; spin < number_of_spins; spin++) {
		free(zmag[spin]);
		free(shiftplus[spin]);
		free(shiftminus[spin]);
	}
	free(zmag);
	free(shiftplus);
	free(shiftminus);
	free(po_matrix);
	free(po_string);
	free(aux_string);
	free(temp_string);

	return return_string;
}


char *product_operator_from_base4(int index, int number_of_spins, int spin_type_array, float coefficient)
{
	char *po_string, *aux_string, *temp_string, spin_type_char;
	int spins_in_operator, spin, mask;

	/**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-4    */
	/*   system is used:                                      */
	/*   With the basis vectors coded as 0 = 0, z = 1, x = 2  */
	/*   and y = 3. The product operator 4 I_2z I3_y in a     */
	/*   system of three spins would be encoded as:           */
	/*     index = 310 (base-4) = 28 (base-10)                */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/
	/* po_string takes the string for one product operator */
	po_string = malloc(30 * sizeof(char));
	memset(po_string, '\0', 21);

	/* aux_string takes the combined product operator components */
	aux_string = malloc(13 * sizeof(char));
	memset(aux_string, '\0', 13);

	/* temp_string takes one component of a product operator (e.g. I_1z) */
	temp_string = malloc(13 * sizeof(char));
	memset(temp_string, '\0', 13);

	spins_in_operator = 0;
	for (spin = 0; spin <= number_of_spins; spin++) {
		/* Select spin from base4 index by mask */
		mask = component_from_base4_coded_product_operator(index, spin);
		/* Chose between I and S spin character */
		if ((spin_type_array & pow2(spin)) >> spin == spinTypeS)
			spin_type_char = 'S';
		else
			spin_type_char = 'I';
		/* multiply E/2, Iz, Ix or Iy to product operator */
		switch (mask) {
		case BASIS_Z:
			sprintf(temp_string, "%c%dz", spin_type_char, spin + 1);
			strcat(aux_string, temp_string);
			spins_in_operator += 1;
			break;
		case BASIS_X:
			sprintf(temp_string, "%c%dx", spin_type_char, spin + 1);
			strcat(aux_string, temp_string);
			spins_in_operator += 1;
			break;
		case BASIS_Y:
			sprintf(temp_string, "%c%dy", spin_type_char, spin + 1);
			strcat(aux_string, temp_string);
			spins_in_operator += 1;
			break;
		}
	}

	/* If operator is identity operator skip calculation of the coefficient */
	if ((spins_in_operator == 0) || (coefficient == 0)) {
		strcpy(po_string, " + E/2\0");

		free(aux_string);
		free(temp_string);

		return po_string;
	}

	/* format the product operator coefficient */
	if ((coefficient >= 0.005) || (coefficient < -0.005)) {
		if (coefficient <= -0.005)
			strcat(po_string, " -");
		else if (coefficient >= 0.005)
			strcat(po_string, " +");
		if (((coefficient >= -1.005) && (coefficient < -0.995)) || ((coefficient >= 0.995) && (coefficient < 1.005)))
			strcat(po_string, " ");
		else {
			sprintf(temp_string, " %.2f ", fabs(coefficient));
			strcat(po_string, temp_string);
		}
		if (spins_in_operator > 1) {
			sprintf(temp_string, "%d", pow2(spins_in_operator - 1));
			strcat(po_string, temp_string);
		}
		strcat(po_string, aux_string);
	}

	free(aux_string);
	free(temp_string);

	return po_string;
}


char *replace_numbers_by_indices(char *string)
{
    unsigned int i, len;
    char *new_string;

    len = strlen(string);
    new_string = malloc(len * sizeof(wchar_t)); //malloc(2 * len * sizeof(char));
    strcpy(new_string, "");

    for (i = 0; i < len; i++) {
        switch(*(string + i)) {
        case '0':
            strcat(new_string, "₀");
            break;
        case '1':
            strcat(new_string, "₁");
            break;
        case '2':
            strcat(new_string, "₂");
            break;
        case '3':
            strcat(new_string, "₃");
            break;
        case '4':
            strcat(new_string, "₄");
            break;
        case '5':
            strcat(new_string, "₅");
            break;
        case '6':
            strcat(new_string, "₆");
            break;
        case '7':
            strcat(new_string, "₇");
            break;
        case '8':
            strcat(new_string, "₈");
            break;
        case '9':
            strcat(new_string, "₉");
            break;
        case '-':
            strcat(new_string, "₋");
            break;
        default:
            strncat(new_string, string + i, 1);
        }
    }
    return new_string;

    /*[string replaceOccurrencesOfString:@","
                            withString:@"‚"
                               options:NSLiteralSearch
                                 range:NSMakeRange(0, [string length])];
    [string replaceOccurrencesOfString:@"-"
                            withString:@"₋"
                               options:NSLiteralSearch
                                 range:NSMakeRange(0, [string length])];
    [string replaceOccurrencesOfString:@","
                            withString:@",，"
                               options:NSLiteralSearch
                                 range:NSMakeRange(0, [string length])];*/
}


char *replace_numbers_by_exponents(char *string)
{
    unsigned int i, len;
    char *new_string;

    len = strlen(string);
    new_string = malloc(len * sizeof(wchar_t)); //malloc(2 * len * sizeof(char));
    strcpy(new_string, "");

    for (i = 0; i < len; i++) {
        switch(*(string + i)) {
        case '0':
            strcat(new_string, "⁰");
            break;
        case '1':
            strcat(new_string, "¹");
            break;
        case '2':
            strcat(new_string, "²");
            break;
        case '3':
            strcat(new_string, "³");
            break;
        case '4':
            strcat(new_string, "⁴");
            break;
        case '5':
            strcat(new_string, "⁵");
            break;
        case '6':
            strcat(new_string, "⁶");
            break;
        case '7':
            strcat(new_string, "⁷");
            break;
        case '8':
            strcat(new_string, "⁸");
            break;
        case '9':
            strcat(new_string, "⁹");
            break;
        case '-':
            strcat(new_string, "⁻");
            break;
        default:
            strncat(new_string, string + i, 1);
        }
    }
    return new_string;
}


gboolean insensitive_g_string_replace(GString *g_origString, char *stringToReplace, char *replaceWith, GString *g_output)
{
    char *found;
    char *copy;
    char *p;

    copy = malloc(g_origString->len + 1);
    copy = strcpy(copy, g_origString->str);
    p = copy;

    if (strlen(replaceWith) > 0) {
        if((found = (strstr(copy, stringToReplace)))) {
            if((found - copy) > 0) {
                g_string_overwrite_len(g_output, 0, copy, found - copy);
                g_string_truncate(g_output, found - copy);
                p += (found - copy);
            } else {
                g_string_assign(g_output, "");
                //g_string_truncate(g_output, 0);
            }
            g_string_append(g_output, replaceWith);
            g_string_append(g_output, p + strlen(stringToReplace));
            free(copy);
            return TRUE;
        } else
            g_string_assign(g_output, g_origString->str);
    } else
        g_string_assign(g_output, g_origString->str);
    free(copy);
    return FALSE;
}


gchar *substring_for_keyword_in_string(gchar *keyword, gchar *text, gsize length)
{
	gchar *return_string = NULL;
	gchar *pos = NULL;
	size_t len;

	if (text == NULL)
		goto search_failed;
	if (length < 1)
		goto search_failed;
	pos = g_strstr_len(text, length, keyword);
	if (pos == NULL)
		goto search_failed;
	pos += strlen(keyword);
	while (*pos == ' ')
		pos++;
	len = 0;
	for (len = 0; (*(pos + len) != '\n') && (*(pos + len) != '\0'); len++);
	return_string = calloc(len + 1, sizeof(char));
	if (return_string == NULL)
		goto search_failed;
	strncpy(return_string, pos, len);

	return return_string;

search_failed:
	return NULL;
}


void coherence_orders_for_matrix(DSPComplex *matrix, int *order_vector, int spins)
{
	int i, m, n, size, order;
	DSPComplex *order_matrix, *aux_matrix, *xmag, *identity;

	if (order_vector != NULL) {
		size = pow2(spins);
		identity = malloc(4 * sizeof(DSPComplex));
		set_complex_identity_matrix(identity, 2);
		xmag = malloc(4 * sizeof(DSPComplex));
		xmag[0] = complex_rect(0.0, 0.0);
		xmag[1] = complex_rect(0.5, 0.0);
		xmag[2] = complex_rect(0.5, 0.0);
		xmag[3] = complex_rect(0.0, 0.0);
		for (i = 0; i <= 2 * spins; i++)
			order_vector[i] = 0;
		/* Check for zero order coherences */
		for (i = 0; i < size; i++) {
			if (matrix[i * size + i].real >= 0.01
			    || matrix[i * size + i].imag >= 0.01)
				order_vector[spins] = 1;
		}
		for (i = 1; i < (int)pow2(spins); i++) {
			order = 0;
			order_matrix = malloc(sizeof(DSPComplex));
			*order_matrix = complex_rect(1.0, 0.0);
			/* Construct matrix with active operators */
			for (n = 0; n < spins; n++) {
				if ((pow2(n) & i) == 0) {
					aux_matrix = kronecker_multiply(order_matrix, pow2(n), identity, 2);
				} else {
					aux_matrix = kronecker_multiply(order_matrix, pow2(n), xmag, 2);
					order++;
				}
				free(order_matrix);
				order_matrix = aux_matrix;
			}
			/* Check if there are elements in matrix of that order that are non zero */
			for (m = 0; m < size; m++) {
				for (n = 0; n < size; n++) {
					if ((fabsf(matrix[n * size + m].real) >= 0.01 || fabsf(matrix[n * size + m].imag) >= 0.01)
					    && order_matrix[n * size + m].real != 0) {
						if (n < m)
							order_vector[spins + order] = 1;
						else if (n > m)
							order_vector[spins - order] = 1;
					}
				}
			}
			free(order_matrix);
		}
		free(xmag);
		free(identity);
	}
}


VectorCoordinates *alloc_vector_coordinates(int size)
{
	int i;
	VectorCoordinates *structure;

	structure = malloc(sizeof(VectorCoordinates));
	if (structure != NULL) {
		structure->size = size;
		structure->x = malloc(size * sizeof(float));
		structure->y = malloc(size * sizeof(float));
		structure->z = malloc(size * sizeof(float));
		structure->spinType = malloc(size * sizeof(int));
		structure->selected = malloc(size * sizeof(char));
		for (i = 0; i < size; i++) {
			structure->x[i] = 0;
			structure->y[i] = 0;
			structure->z[i] = 0;
			structure->spinType[i] = spinTypeI;
			structure->selected[i] = 0;
		}
	}
	return structure;
}


void free_vector_coordinates(VectorCoordinates *structure)
{
	if (structure != NULL) {
		free(structure->x);
		free(structure->y);
		free(structure->z);
		free(structure->spinType);
		free(structure->selected);
		free(structure);
	}
}


         //////  /////// //////      /////// /////// ////////
//    // //   // //      //   //     //      //         //
//    // //   // /////// //////      /////   /////      //
 //  //  //   //      // //          //      //         //
  ////   //////  /////// //          //      //         //

void vDSP_fft_zip(fftw_plan __Plan, const DSPSplitComplex *__C, unsigned int __stride, unsigned int __Log2N, int __Direction)
{
	fftw_complex *fftw_data;
	unsigned int i, size = pow2(__Log2N);

    fftw_data = fftw_malloc(size * sizeof(fftw_complex));
	for (i = 0; i < size; i++)
    	fftw_data[i] = __C->realp[i * __stride] + I * __C->imagp[i * __stride];
	__Plan = fftw_plan_dft_1d(size, fftw_data, fftw_data, __Direction, FFTW_ESTIMATE);
	fftw_execute(__Plan);
	for (i = 0; i < size; i++) {
		__C->realp[i * __stride] = creal(fftw_data[i]);
		__C->imagp[i * __stride] = cimag(fftw_data[i]);
	}
	fftw_free(fftw_data);
	fftw_destroy_plan(__Plan);
}


void vDSP_fft2d_zip(fftw_plan __Plan, const DSPSplitComplex *__C, unsigned int __IC0, unsigned int __IC1, unsigned int __Log2N0, unsigned int __Log2N1, int __Direction)
{
    fftw_complex *fftw_data;
    unsigned int i;
    unsigned int size0 = pow2(__Log2N0);
    unsigned int size1 = pow2(__Log2N1);
    unsigned int size = size0 * size1;

    fftw_data = fftw_malloc(size * sizeof(fftw_complex));
	for (i = 0; i < size; i++)
		fftw_data[i] = __C->realp[i] + I * __C->imagp[i];
	__Plan = fftw_plan_dft_2d(size1, size0, fftw_data, fftw_data, __Direction, FFTW_ESTIMATE);
    fftw_execute(__Plan);
	for (i = 0; i < size; i++) {
		__C->realp[i] = creal(fftw_data[i]);
		__C->imagp[i] = cimag(fftw_data[i]);
	}
	fftw_free(fftw_data);
	fftw_destroy_plan(__Plan);
}


//       /////  //////   /////   ////// //   //     //     //  //////  //////  //   //  /////  //////   //////  //    // ///    // //////
//      //   // //   // //   // //      //  //      //     // //    // //   // //  //  //   // //   // //    // //    // ////   // //   //
//      /////// //////  /////// //      /////       //  /  // //    // //////  /////   /////// //////  //    // //    // // //  // //   //
//      //   // //      //   // //      //  //      // /// // //    // //   // //  //  //   // //   // //    // //    // //  // // //   //
/////// //   // //      //   //  ////// //   //      /// ///   //////  //   // //   // //   // //   //  //////   //////  //   //// //////

int eigen_symmv(float *matrix, float *eval, float *evec, int size)
{
	/* Algorithm taken from "Numerical Recipes", 3rd edition 2007 */

	int i, j, k, l, m, iter;
	float scale, hh, h, g, f, s, r, p, dd, c, b;
	float *e;

	cblas_scopy(size * size, matrix, 1, evec, 1);
	e = malloc(size * sizeof(float));

	/* Perform Householder reduction on matrix (tred2) */
	for(i = size - 1; i > 0; i--) {
		l = i - 1;
		h = 0.0;
		scale = 0.0;
		if(i > 1) {
			for(k = 0; k < i; k++)
				scale += fabs(evec[k * size + i]);
			if(scale == 0.0)
			/* Skip transformation */
				e[i] = evec[l * size + i];
			else {
				for(k = 0; k < i; k++) {
					/* User scaled a's for transformation */
					evec[k * size + i] = evec[k * size + i] / scale;
					/* form sigma in h */
					h += evec[k * size + i] * evec[k * size + i];
				}
				f = evec[l * size + i];
				g = (f > 0.0 ? -sqrt(h) : sqrt(h));
				e[i] = scale * g;
				/* make h = 1/2 |u|^2 */
				h -= f * g;
				/* store u in the ith row of evec */
				evec[l * size + i] = f - g;
				f = 0.0;
				for(j = 0; j < i; j++) {
					/* Store u/H in ith column of evec */
					evec[i * size + j] = evec[j * size + i] / h;
					/* Form an element of A * u in g */
					g = 0.0;
					for(k = 0; k <= j; k++)
						g += evec[k * size + j] * evec[k * size + i];
					for(k = j + 1; k < i; k++)
						g += evec[j * size + k] * evec[k * size + i];
					/* Form element of p in temporarily unused element of e */
					e[j] = g / h;
					f += e[j] * evec[j * size + i];
				}
				/* Form K = u^T * p / 2 H */
				hh = f / (h + h);
				/* Form q and store in e overwriting p */
				for(j = 0; j < i; j++) {
                    f = evec[j * size + i];
					g = e[j] - hh * f;
					e[j] = g;
					/* Reduce evec, A' = A - q*u^T - u*q^T */
					for(k = 0; k <= j; k++)
						evec[k * size + j] = evec[k * size + j] - (f * e[k] + g * evec[k * size + i]);
				}
			}
		} else
			e[i] = evec[l * size + i];
        eval[i] = h;
	}
    eval[0] = 0.0;
    e[0] = 0.0;
	/* Begin accumulation of transformation matrices */
	for(i = 0; i < size; i++) {
		/* This block is not skipped when i == 1 although it is in tred2.c */
		for(j = 0; j < i; j++) {
			g = 0.0;
			/* Use u and u/H stored in evec to form P*Q */
			for(k = 0; k < i; k++)
				g += evec[k * size + i] * evec[j * size + k];
			for(k = 0; k < i; k++)
				evec[j * size + k] = evec[j * size + k] - g * evec[i * size + k];
		}
		eval[i] = evec[i * size + i];
		/* Reset row and column of evec to identity matrix for next iteration */
		evec[i * size + i] = 1.0;
		for(j = 0; j < i; j++) {
			evec[i * size + j] = 0.0;
			evec[j * size + i] = 0.0;
		}
	}

	/* Perform QL algorithm with Implicit Shifts (tqli) */
	for(i = 1; i < size; i++)
		e[i - 1] = e[i];
	e[size - 1] = 0.0;
	for(l = 0; l < size; l++) {
		iter = 0;
		do {
			for(m = l; m < size - 1; m++) {
				dd = fabs(eval[m]) + fabs(eval[m + 1]);
				if(fabs(e[m] + dd) == dd)
					break;
			}
			if(m != l) {
				if(iter++ == 30) {
					printf("Too many iterations in QL algorithm!\n");
					return 10;
				}
				g = (eval[l + 1] - eval[l]) / (2.0 * e[l]);
				r = sqrt((g * g) + 1.0);
				g = eval[m] - eval[l] + e[l] / (g + SIGN(r, g));
				s = 1.0;
				c = 1.0;
				p = 0.0;
				for(i = m - 1; i >= l; i--) {
					f = s * e[i];
					b = c * e[i];
					if(fabs(f) >= fabs(g)) {
						c = g / f;
						r = sqrt((c * c) + 1.0);
						e[i + 1] = f * r;
						s = 1.0 / r;
						c *= s;
					} else {
						s = f / g;
						r = sqrt((s * s) + 1.0);
						e[i + 1] = g * r;
						c = 1.0 / r;
						s *= c;
					}
					g = eval[i + 1] - p;
					r = (eval[i] - g) * s + 2.0 * c * b;
					p = s * r;
					eval[i + 1] = g + p;
					g = c * r - b;
					for(k = 0; k < size; k++) {
						f = evec[(i + 1) * size + k];
                        evec[(i + 1) * size + k] = s * evec[i * size + k] + c * f;
                        evec[i * size + k] = c * evec[i * size + k] - s * f;
                    }
                }
                eval[l] = eval[l] - p;
                e[l] = g;
                e[m] = 0.0;
            }
        } while(m != l);
    }
    free(e);

    return 0;
}


int linalg_LU_decomp(float *LU, size_t *p, int *signum, int size)
{
    int	i, j, k;
    int	max;
    size_t tmp;
    double	c, c1;

    *signum = 1;
    for(i = 0; i < size; i++) {
        p[i] = i;
    }
    for(k = 0; k < size; k++){
        /* partial pivoting */
        max = k;
        c = 0;
        for(i = k; i < size; i++) {
            c1 = fabs(LU[k * size + p[i]]);
            if(c1 > c) {
                c = c1;
                max = i;
            }
        }
        /* row exchange, update permutation vector */
        if(k != max) {
            tmp = p[k];
            p[k] = p[max];
            p[max] = tmp;
            *signum *= -1;
        }
        /* suspected singular matrix */
        if(LU[k * size + p[k]] == 0)
            return -1;
        for(i = k + 1; i < size; i++) {
            /* calculate m(i,j) */
            LU[k * size + p[i]] = LU[k * size + p[i]] / LU[k * size + p[k]];
            /* elimination */
            for(j = k + 1; j < size; j++) {
                LU[j * size + p[i]] = LU[j * size + p[i]] - LU[k * size + p[i]] * LU[j * size + p[k]];
            }
        }
    }

    return 0;
}


int linalg_LU_invert(float *LU, size_t *p, float *inverse, int size)
{
    float *copy_matrix;
    float *B;
    int	i, j, k, m, vec_size;
    double sum;

    vec_size = size * size;
    copy_matrix = malloc(vec_size * sizeof(float));
    cblas_scopy(vec_size, LU, 1, copy_matrix, 1);
    B = malloc(size * sizeof(float));

    /* Missing: check for singular matrix !!! */

    for(i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            if(j == i)
                B[j] = 1;
            else
                B[j] = 0;
        }
        for(k = 0; k < size; k++)
            for(m = k + 1; m < size; m++)
                B[p[m]] = B[p[m]] - copy_matrix[k * size + p[m]] * B[p[k]];
        inverse[i * size + size - 1] = B[p[size - 1]] / copy_matrix[(size - 1) * size + p[size - 1]];
        for(k = size - 2; k >= 0; k--) {
            sum = 0;
            for(j = k + 1; j < size; j++)
                sum += copy_matrix[j * size + p[k]] * inverse[i * size + j];
            inverse[i * size + k] = (B[p[k]] - sum) / copy_matrix[k * size + p[k]];
        }
    }
    free(copy_matrix);
    free(B);

    return 0;
}


DSPComplex *strong_coupling_operator_workaround(unsigned int spins, unsigned int array1, unsigned int array2, float angle)
{
    int m, n, size, vec_size, signum;
    DSPComplex *operator, *complex_eigenvectors, *inverse_complex_eigenvectors;
    DSPComplex *aux_matrix, zalpha, zbeta, zangle;
    float *eigenvalues;
    float *real_operator, *eigenvectors, *inverse_eigenvectors, *LU_matrix;
    size_t *permutation_matrix;

    // Create strong coupling operator
    size = pow2(spins);
    vec_size = size * size;
    operator = I1I2(array1, array2, spins);

    // Convert to real matrix
    real_operator = malloc(vec_size * sizeof(float));
    for(n = 0; n < size; n++)
        for(m = 0; m < size; m++)
            real_operator[n * size + m] = operator[n * size + m].real;

    // Solve eigenvalue equation and return eigenvector matrix
    eigenvalues = malloc(size * sizeof(float));
    eigenvectors = malloc(vec_size * sizeof(float));
    eigen_symmv(real_operator, eigenvalues, eigenvectors, size);

    // Invert eigenvector matrix
    LU_matrix = malloc(vec_size * sizeof(float));
    cblas_scopy(vec_size, eigenvectors, 1, LU_matrix, 1);
    inverse_eigenvectors = malloc(vec_size * sizeof(float));
    permutation_matrix = malloc(size * sizeof(size_t));
    linalg_LU_decomp(LU_matrix, permutation_matrix, &signum, size);
    linalg_LU_invert(LU_matrix, permutation_matrix, inverse_eigenvectors, size);
    free(permutation_matrix);

    // Revert to complex matrices
    complex_eigenvectors = malloc(vec_size * sizeof(DSPComplex));
    inverse_complex_eigenvectors = malloc(vec_size * sizeof(DSPComplex));
    for(n = 0; n < size; n++) {
        for(m = 0; m < size; m++) {
            complex_eigenvectors[n * size + m] = complex_rect(eigenvectors[n * size + m], 0);
            inverse_complex_eigenvectors[n * size + m] = complex_rect(inverse_eigenvectors[n * size + m], 0);
        }
    }
    free(eigenvalues);
    free(eigenvectors);
    free(inverse_eigenvectors);
    free(real_operator);
    free(LU_matrix);

    // Perform matrix transformation D = X^-1.A.X
    zalpha = complex_rect(1, 0);
    zbeta =complex_rect(0, 0);
    zangle = complex_rect(0, 0.5 * angle);
    aux_matrix = malloc(vec_size * sizeof(DSPComplex));
    // aux_matrix = operator.eigenvectors
    cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size, size,
                &zalpha, operator, size, complex_eigenvectors, size, &zbeta, aux_matrix, size);
    // operator = zangle * inverse_eigenvectors.aux_matrix
    cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size, size,
                &zangle, inverse_complex_eigenvectors, size, aux_matrix, size, &zbeta, operator, size);

    // Calculate exp(D)
    for(m = 0; m < size; m++)
        operator[m * size + m] = complex_rect(exp(operator[m * size + m].real) * cos(operator[m * size + m].imag),
                                              exp(operator[m * size + m].real) * sin(operator[m * size + m].imag));

    // Perform matrix transformation A = X D X^-1
    // aux_matrix = operator.inverse_eigenvectors
    cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size, size,
                &zalpha, operator, size, inverse_complex_eigenvectors, size, &zbeta, aux_matrix, size);
    // operator = eigenvectors.aux_matrix
    cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size, size,
                &zalpha, complex_eigenvectors, size, aux_matrix, size, &zbeta, operator, size);

    free(aux_matrix); free(complex_eigenvectors); free(inverse_complex_eigenvectors);

    return operator;
}
