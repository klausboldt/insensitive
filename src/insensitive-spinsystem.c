/* insensitive-spinsystem.c
 *
 * Copyright 2009-2021 Klaus Boldt
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written
 * authorization.
 */

#include "insensitive-config.h"
#include "insensitive-spinsystem.h"
#include "insensitive-settings.h"
#include "insensitive-controller.h"


G_DEFINE_TYPE(InsensitiveSpinSystem, insensitive_spinsystem, G_TYPE_OBJECT)


InsensitiveSpinSystem* insensitive_spinsystem_new()
{
	return (InsensitiveSpinSystem *)g_object_new(INSENSITIVE_TYPE_SPINSYSTEM, NULL);
}


static void insensitive_spinsystem_dispose(GObject *gobject)
{
	InsensitiveSpinSystem *self = (InsensitiveSpinSystem *)gobject;

	/* In dispose(), you are supposed to free all types referenced from this
	 * object which might themselves hold a reference to self. Generally,
	 * the most simple solution is to unref all members on which you own a
	 * reference.
	 */

	/* dispose() might be called multiple times, so we must guard against
	 * calling g_object_unref() on an invalid GObject by setting the member
	 * NULL; g_clear_object() does this for us.
	 */
	insensitive_spinsystem_free_gradient_array(self);
    g_object_unref(self->actionsSinceLastGradient);

	G_OBJECT_CLASS(insensitive_spinsystem_parent_class)->dispose(gobject);
}


static void insensitive_spinsystem_finalize(GObject *gobject)
{
	InsensitiveSpinSystem *self = (InsensitiveSpinSystem *)gobject;

	if (self->matrix !=  NULL)
		free(self->matrix);
	if (self->couplingMatrix !=  NULL)
		free(self->couplingMatrix);

	if (self->pulsePropagator !=  NULL)
		free(self->pulsePropagator);
	if (self->shiftPropagator != NULL)
		free(self->shiftPropagator);
	if (self->couplingPropagator != NULL)
		free(self->couplingPropagator);
	if (self->relaxationPropagator != NULL)
		free(self->relaxationPropagator);

	G_OBJECT_CLASS(insensitive_spinsystem_parent_class)->finalize(gobject);
}


static void insensitive_spinsystem_class_init(InsensitiveSpinSystemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = insensitive_spinsystem_dispose;
	object_class->finalize = insensitive_spinsystem_finalize;
}


static void insensitive_spinsystem_init(InsensitiveSpinSystem *self)
{
	insensitive_spinsystem_init_with_spins(self, 2);
}


void insensitive_spinsystem_init_with_spins(InsensitiveSpinSystem *self, unsigned int number)
{
	self->spinTypeArray = 0;

	self->pulsePropagator = NULL;
	self->shiftPropagator = NULL;
	self->couplingPropagator = NULL;
	self->relaxationPropagator = NULL;
	self->gyroI = 1.0;
	self->gyroS = gyro_13C / gyro_1H;

	self->gradientArray = NULL;
	self->firstGradientPulseIssued = FALSE;
	self->actionsSinceLastGradient = insensitive_pulsesequence_new();

	insensitive_spinsystem_set_spins(self, number);
}


InsensitiveSpinSystem *insensitive_spinsystem_copy(InsensitiveSpinSystem *self)
{
    InsensitiveSpinSystem *copy;

    copy = insensitive_spinsystem_new();
    insensitive_spinsystem_init_with_spins(copy, self->spins);
    insensitive_spinsystem_set_spintypearray(copy, self->spinTypeArray);
    insensitive_spinsystem_substitute_matrix(copy, self->matrix);
    insensitive_spinsystem_substitute_couplingmatrix(copy, self->couplingMatrix);
    insensitive_spinsystem_set_firstgradientpulseissued(copy, self->firstGradientPulseIssued);
    if (self->firstGradientPulseIssued) {
        insensitive_spinsystem_create_gradientarray(copy);
        insensitive_spinsystem_set_gradientarray(copy, self->gradientArray);
        insensitive_spinsystem_set_actions_since_last_gradient(copy, self->actionsSinceLastGradient);
    }
    copy->gyroI = self->gyroI;
    copy->gyroS = self->gyroS;
    copy->absGyroI = self->absGyroI;
    copy->absGyroS = self->absGyroS;

    return copy;
}


/////// //////  // ///    //     ///    // //    // ///    /// //////  /////// //////       /////  ///    // //////      //////// //    // //////  ///////
//      //   // // ////   //     ////   // //    // ////  //// //   // //      //   //     //   // ////   // //   //        //     //  //  //   // //
/////// //////  // // //  //     // //  // //    // // //// // //////  /////   //////      /////// // //  // //   //        //      ////   //////  /////
     // //      // //  // //     //  // // //    // //  //  // //   // //      //   //     //   // //  // // //   //        //       //    //      //
/////// //      // //   ////     //   ////  //////  //      // //////  /////// //   //     //   // //   //// //////         //       //    //      ///////

unsigned int insensitive_spinsystem_get_spins(InsensitiveSpinSystem *self)
{
	return self->spins;
}


void insensitive_spinsystem_set_spins(InsensitiveSpinSystem *self, unsigned int number)
{
	if ((number != self->spins) && (number <= maxNumberOfSpins)) {
		insensitive_spinsystem_resize_couplingmatrix_for_spins(self, number);
		self->spins = number;
		self->spinsxspins = number * number;
		self->size = pow2(self->spins);
		g_free(self->matrix);
		self->matrix = malloc(self->size * self->size * sizeof(DSPComplex));
		insensitive_spinsystem_return_to_thermal_equilibrium(self);
		insensitive_spinsystem_reset_relaxationpropagator(self);
	}
}


void insensitive_spinsystem_add_spin(InsensitiveSpinSystem *self)
{
	if (self->spins < maxNumberOfSpins)
		insensitive_spinsystem_set_spins(self, self->spins + 1);
}


void insensitive_spinsystem_remove_spin(InsensitiveSpinSystem *self)
{
	if (self->spins > 1)
		insensitive_spinsystem_set_spins(self, self->spins - 1);
}


void insensitive_spinsystem_remove_spin_number(InsensitiveSpinSystem *self, unsigned int number)
{
	unsigned int row, column;

	// Delete marked row by shifting rows > number
	for (row = number + 1; row < self->spins; row++) {
		for (column = 0; column < self->spins; column++) {
			self->couplingMatrix[(row - 1) * self->spins + column] = self->couplingMatrix[row * self->spins + column];
		}
	}

	// Delete marked column by shifting columns > number
	for (column = number + 1; column < self->spins; column++) {
		for (row = 0; row < self->spins; row++) {
			self->couplingMatrix[row * self->spins + column - 1] = self->couplingMatrix[row * self->spins + column];
		}
	}

	// Remove marked spin type from array
	for (row = number + 1; row < self->spins; row++)
		insensitive_spinsystem_set_spintype_for_spin(self,
							     insensitive_spinsystem_get_spintype_for_spin(self, row),
							     row - 1);

	// Remove last spin
	insensitive_spinsystem_set_spins(self, self->spins - 1);
}


unsigned int insensitive_spinsystem_get_size(InsensitiveSpinSystem *self)
{
	return self->size;
}


unsigned int insensitive_spinsystem_get_spintypearray(InsensitiveSpinSystem *self)
{
	return self->spinTypeArray;
}


unsigned int insensitive_spinsystem_get_pulsearray_for_spintype(InsensitiveSpinSystem *self, unsigned int spinType)
{
	unsigned int i, returnArray = 0;

	if (spinType <= 1)
		for (i = 0; i < self->spins; i++)
			returnArray += (insensitive_spinsystem_get_spintype_for_spin(self, i) == spinType) ? pow2(i) : 0;

	return returnArray;
}


unsigned int insensitive_spinsystem_get_spintype_for_spin(InsensitiveSpinSystem *self, unsigned int number)
{
	return (self->spinTypeArray & pow2(number)) >> number;
}


void insensitive_spinsystem_set_spintypearray(InsensitiveSpinSystem *self, unsigned int value)
{
	if (self->spinTypeArray != value) {
		self->spinTypeArray = value;
		insensitive_spinsystem_constants_were_changed(self);
	}
}


void insensitive_spinsystem_set_spintype_for_spin(InsensitiveSpinSystem *self, unsigned int type, unsigned int number)
{
	if (type == spinTypeS) {
		self->spinTypeArray |= pow2(number);
	} else if (type == spinTypeI) {
		self->spinTypeArray |= pow2(number);
		self->spinTypeArray -= pow2(number);
	}
	insensitive_spinsystem_constants_were_changed(self);
}


unsigned int insensitive_spinsystem_get_number_of_ispins(InsensitiveSpinSystem *self)
{
	unsigned int i, number, array;

	number = 0;
	array = self->spinTypeArray;
	for (i = 0; i < self->spins; i++) {
		if (!(array & 1))
			number++;
		array >>= 1;
	}
	return number;
}


unsigned int insensitive_spinsystem_get_number_of_sspins(InsensitiveSpinSystem *self)
{
	unsigned int i, number, array;

	number = 0;
	array = self->spinTypeArray;
	for (i = 0; i < self->spins; i++) {
		if (array & 1)
			number++;
		array >>= 1;
	}
	return number;
}


 //////  //////  //    // //////  //      // ///    //  //////      ///    ///  /////  //////// //////  // //   //
//      //    // //    // //   // //      // ////   // //           ////  //// //   //    //    //   // //  // //
//      //    // //    // //////  //      // // //  // //   ///     // //// // ///////    //    //////  //   ///
//      //    // //    // //      //      // //  // // //    //     //  //  // //   //    //    //   // //  // //
 //////  //////   //////  //      /////// // //   ////  //////      //      // //   //    //    //   // // //   //

float *insensitive_spinsystem_get_raw_couplingmatrix(InsensitiveSpinSystem *self)
{
	return self->couplingMatrix;
}


void insensitive_spinsystem_init_couplingmatrix(InsensitiveSpinSystem *self)
{
	unsigned int i, j;

	self->couplingMatrix = malloc(self->spinsxspins * sizeof(float));

	// Initialize chemical shifts
	for (i = 0; i < self->spins; i++)
		self->couplingMatrix[i * self->spins + i] = initial_chemical_shift;

	// Initialize coupling constants
	for (j = 0; j < self->spins; j++)
		for (i = j + 1; i < self->spins; i++) {
			// J-Coupling constant in Hz
			self->couplingMatrix[j * self->spins + i] = initial_scalar_coupling_constant;
			// Distance in nm
			self->couplingMatrix[i * self->spins + j] = initial_distance;
		}
}


void insensitive_spinsystem_resize_couplingmatrix_for_spins(InsensitiveSpinSystem *self, unsigned int number)
{
	float *newCouplingMatrix;
	unsigned int i, j;

	// Tensor cannot be smaller than 1 x 1
	if (number > 0) {
		newCouplingMatrix = malloc(number * number * sizeof(float));
		for (j = 0; j < number; j++) {
			for (i = 0; i < number; i++) {
				if ((i < self->spins) && (j < self->spins))
					newCouplingMatrix[j * number + i] = self->couplingMatrix[j * self->spins + i];
				else {
					if (i > j)       // Lower triangular matrix: Distance in nm
						newCouplingMatrix[i * number + j] = initial_distance;
					else if (i == j) // Diagonal elements: Chemical shift in Hz
						newCouplingMatrix[i * number + j] = initial_chemical_shift;
					else             // Upper triangular matrix: J-Coupling constant in Hz
						newCouplingMatrix[i * number + j] = initial_scalar_coupling_constant;
				}
			}
		}
		free(self->couplingMatrix);
		self->couplingMatrix = newCouplingMatrix;
		insensitive_spinsystem_constants_were_changed(self);
	}
}


void insensitive_spinsystem_substitute_couplingmatrix(InsensitiveSpinSystem *self, float *newCouplingMatrix)
{
	cblas_scopy(self->spinsxspins, newCouplingMatrix, 1, self->couplingMatrix, 1);
	insensitive_spinsystem_constants_were_changed(self);
}


float insensitive_spinsystem_get_larmorfrequency_for_spin(InsensitiveSpinSystem *self, unsigned int spinNumber)
{
	return self->couplingMatrix[spinNumber * self->spins + spinNumber];
}


void insensitive_spinsystem_set_larmorfrequency_for_spin(InsensitiveSpinSystem *self, unsigned int spinNumber, float chemicalShift)
{
	self->couplingMatrix[spinNumber * self->spins + spinNumber] = chemicalShift;

	if (self->pulsePropagator != NULL) {
		free(self->pulsePropagator);
		self->pulsePropagator = NULL;
	}
	if (self->shiftPropagator != NULL) {
		free(self->shiftPropagator);
		self->shiftPropagator = NULL;
	}
}


float insensitive_spinsystem_get_jcouplingconstant_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2)
{
	if (spinNumber1 > spinNumber2)
		return self->couplingMatrix[spinNumber2 * self->spins + spinNumber1];
	else
		return self->couplingMatrix[spinNumber1 * self->spins + spinNumber2];
}


void insensitive_spinsystem_set_jcouplingconstant_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2, float couplingConstant)
{
	if (spinNumber1 > spinNumber2)
		self->couplingMatrix[spinNumber2 * self->spins + spinNumber1] = couplingConstant;
	else
		self->couplingMatrix[spinNumber1 * self->spins + spinNumber2] = couplingConstant;

	if (self->couplingPropagator != NULL) {
		free(self->couplingPropagator);
		self->couplingPropagator = NULL;
	}
}


float insensitive_spinsystem_get_dipolarcouplingconstant_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2)
{
	float value, gyro1, gyro2;

	gyro1 = (insensitive_spinsystem_get_spintype_for_spin(self, spinNumber1) == spinTypeI) ? self->absGyroI : self->absGyroS;
	gyro2 = (insensitive_spinsystem_get_spintype_for_spin(self, spinNumber2) == spinTypeI) ? self->absGyroI : self->absGyroS;

	if (spinNumber1 > spinNumber2)
		value = self->couplingMatrix[spinNumber1 * self->spins + spinNumber2];
	else
		value = self->couplingMatrix[spinNumber2 * self->spins + spinNumber1];

	return dipolar_coupling_constant(value, gyro1, gyro2) / (2 * M_PI);
}


float insensitive_spinsystem_get_distance_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2)
{
	if (spinNumber1 > spinNumber2)
		return self->couplingMatrix[spinNumber1 * self->spins + spinNumber2];
	else
		return self->couplingMatrix[spinNumber2 * self->spins + spinNumber1];
}


void insensitive_spinsystem_set_dipolarcouplingconstant_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2, float b)
{
	float r, gyro1, gyro2;

	gyro1 = (insensitive_spinsystem_get_spintype_for_spin(self, spinNumber1) == spinTypeI) ? self->absGyroI : self->absGyroS;
	gyro2 = (insensitive_spinsystem_get_spintype_for_spin(self, spinNumber2) == spinTypeI) ? self->absGyroI : self->absGyroS;
	r = distance_between_spins(b, gyro1, gyro2);
	insensitive_spinsystem_set_distance_between_spins(self, spinNumber1, spinNumber2, r);
}


void insensitive_spinsystem_set_distance_between_spins(InsensitiveSpinSystem *self, unsigned int spinNumber1, unsigned int spinNumber2, float r)
{
	if (spinNumber1 > spinNumber2)
		self->couplingMatrix[spinNumber1 * self->spins + spinNumber2] = r;
	else
		self->couplingMatrix[spinNumber2 * self->spins + spinNumber1] = r;

	insensitive_spinsystem_reset_relaxationpropagator(self);
}


void insensitive_spinsystem_constants_were_changed(InsensitiveSpinSystem *self)
{
	if (self->pulsePropagator != NULL) {
		free(self->pulsePropagator);
		self->pulsePropagator = NULL;
	}
	if (self->shiftPropagator != NULL) {
		free(self->shiftPropagator);
		self->shiftPropagator = NULL;
	}
	if (self->couplingPropagator != NULL) {
		free(self->couplingPropagator);
		self->couplingPropagator = NULL;
	}
	insensitive_spinsystem_reset_relaxationpropagator(self);
}


//////  /////// ///    // /////// // //////// //    //     ///    ///  /////  //////// //////  // //   //
//   // //      ////   // //      //    //     //  //      ////  //// //   //    //    //   // //  // //
//   // /////   // //  // /////// //    //      ////       // //// // ///////    //    //////  //   ///
//   // //      //  // //      // //    //       //        //  //  // //   //    //    //   // //  // //
//////  /////// //   //// /////// //    //       //        //      // //   //    //    //   // // //   //

DSPComplex *insensitive_spinsystem_get_raw_densitymatrix(InsensitiveSpinSystem *self)
{
	return self->matrix;
}


DSPComplex insensitive_spinsystem_get_matrixelement(InsensitiveSpinSystem *self, unsigned int row, unsigned int col)
{
	return self->matrix[row * self->size + col];
}


void insensitive_spinsystem_substitute_matrix(InsensitiveSpinSystem *self, DSPComplex *newMatrix)
{
	cblas_ccopy(self->size * self->size, newMatrix, 1, self->matrix, 1);
}


float insensitive_spinsystem_get_expectationvalue_x_for_spin(InsensitiveSpinSystem *self, unsigned int spin)
{
	float expectationValue;
	DSPComplex *xmag, z;

	xmag = Ix(spin, self->spins);
	z = expectation_value(self->matrix, xmag, self->size);
	expectationValue = self->size * z.real;
	free(xmag);

	return expectationValue;
}


float insensitive_spinsystem_get_expectationvalue_y_for_spin(InsensitiveSpinSystem *self, unsigned int spin)
{
	float expectationValue;
	DSPComplex *ymag, z;

	ymag = Iy(spin, self->spins);
	z = expectation_value(self->matrix, ymag, self->size);
	expectationValue = self->size * z.real;
	free(ymag);

	return expectationValue;
}


float insensitive_spinsystem_get_expectationvalue_z_for_spin(InsensitiveSpinSystem *self, unsigned int spin)
{
	float expectationValue;
	DSPComplex *zmag, z;

	zmag = Iz(spin, self->spins);
	z = expectation_value(self->matrix, zmag, self->size);
	expectationValue = self->size * z.real;
	free(zmag);

	return expectationValue;
}


gchar *insensitive_spinsystem_get_cartesianOperatorString(InsensitiveSpinSystem *self)
{
	gchar *productOperatorCString, *returnString;

	productOperatorCString = (gchar *)cartesian_product_operators(self->matrix, self->size, self->spinTypeArray);

	if (!strcmp(productOperatorCString, "E/2\0")) {
		returnString = malloc(4 * sizeof(gchar));
		strcpy(returnString, productOperatorCString);
	} else {
		if (productOperatorCString[4] == '+') {
			returnString = malloc((strlen(productOperatorCString) - 5) * sizeof(gchar));
			strcpy(returnString, productOperatorCString + 6);
		} else {
			returnString = malloc((strlen(productOperatorCString) - 3) * sizeof(gchar));
			strcpy(returnString, productOperatorCString + 4);
		}
		free(productOperatorCString);
	}

	return returnString;
}


gchar *insensitive_spinsystem_get_sphericalOperatorString(InsensitiveSpinSystem *self)
{
	gchar *productOperatorCString, *returnString, **split;

	productOperatorCString = (gchar *)spherical_product_operators(self->matrix, self->size, self->spinTypeArray);

	if (!strcmp(productOperatorCString, "E/2\0")) {
		returnString = malloc(4 * sizeof(gchar));
		strcpy(returnString, productOperatorCString);
	} else {
		/* Replace placeholders for ⁺, ⁻, and π */
		split = g_strsplit(productOperatorCString, "[+]", -1);
		g_free(productOperatorCString);
		productOperatorCString = g_strjoinv("⁺", split);
		g_strfreev(split);
		split = g_strsplit(productOperatorCString, "[-]", -1);
		g_free(productOperatorCString);
		productOperatorCString = g_strjoinv("⁻", split);
		g_strfreev(split);
		split = g_strsplit(productOperatorCString, "pi", -1);
		g_free(productOperatorCString);
		productOperatorCString = g_strjoinv("π", split);
		g_strfreev(split);
		if (productOperatorCString[4] == '+') {
			returnString = malloc((strlen(productOperatorCString) - 6) * sizeof(gchar));
			strcpy(returnString, productOperatorCString + 6);
		} else {
			returnString = malloc((strlen(productOperatorCString) - 4) * sizeof(gchar));
			strcpy(returnString, productOperatorCString + 4);
		}
		free(productOperatorCString);
	}

	return returnString;
}


VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_coherences(InsensitiveSpinSystem *self, unsigned int highlighted)
{
	VectorCoordinates *xyzVector;
	unsigned int i, j, spinNumber, numberOfVectors, currentVector;

	// Determine number of vectors
	numberOfVectors = pow2(self->spins - 1) * self->spins;
	xyzVector = alloc_vector_coordinates(numberOfVectors);
	currentVector = 0;
	// Scan all matrix elements for observable magnetisation
	for (j = 0; j < self->size; j++) {
		for (i = 0; i < self->size; i++) {
			// Select simple (+1)-coherences
			if (test_for_simple_coherence(i, j) > 0) {
				spinNumber = self->spins - lb(i ^ j) - 1;
				xyzVector->spinType[currentVector] = insensitive_spinsystem_get_spintype_for_spin(self, spinNumber);
				xyzVector->z[currentVector] = insensitive_spinsystem_get_expectationvalue_z_for_spin(self, spinNumber);
				xyzVector->x[currentVector] = pow2(self->spins) * self->matrix[j * self->size + i].real;
				xyzVector->y[currentVector] = pow2(self->spins) * self->matrix[j * self->size + i].imag;
				xyzVector->selected[currentVector] = (spinNumber == highlighted) ? TRUE : FALSE;
				currentVector++;
			}
		}
	}

	return xyzVector;
}


VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_moments(InsensitiveSpinSystem *self, unsigned int highlighted)
{
	unsigned int i;
	VectorCoordinates *xyzVector;

	xyzVector = alloc_vector_coordinates(self->spins);
	for (i = 0; i < self->spins; i++) {
		xyzVector->x[i] = insensitive_spinsystem_get_expectationvalue_x_for_spin(self, i);
		xyzVector->y[i] = insensitive_spinsystem_get_expectationvalue_y_for_spin(self, i);
		xyzVector->z[i] = insensitive_spinsystem_get_expectationvalue_z_for_spin(self, i);
		xyzVector->spinType[i] = insensitive_spinsystem_get_spintype_for_spin(self, i);
		xyzVector->selected[i] = (i == highlighted) ? TRUE : FALSE;
	}

	return xyzVector;
}


VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_fid(InsensitiveSpinSystem *self)
{
	unsigned int i;
	VectorCoordinates *xyzVector;
	float vectorLength;

	xyzVector = alloc_vector_coordinates(2);
	xyzVector->spinType[0] = spinTypeI;
	xyzVector->spinType[1] = spinTypeS;
	xyzVector->selected[0] = FALSE;
	xyzVector->selected[1] = FALSE;
	for (i = 0; i < self->spins; i++) {
		xyzVector->x[insensitive_spinsystem_get_spintype_for_spin(self, i)] += insensitive_spinsystem_get_expectationvalue_x_for_spin(self, i);
		xyzVector->y[insensitive_spinsystem_get_spintype_for_spin(self, i)] += insensitive_spinsystem_get_expectationvalue_y_for_spin(self, i);
		xyzVector->z[insensitive_spinsystem_get_spintype_for_spin(self, i)] += insensitive_spinsystem_get_expectationvalue_z_for_spin(self, i);
	}
	// Show relative magnitude with I spin vector length = 1 for thermal equilibrium
	vectorLength = insensitive_spinsystem_get_number_of_ispins(self);
	xyzVector->x[0] /= vectorLength;
	xyzVector->y[0] /= vectorLength;
	xyzVector->z[0] /= vectorLength;
	vectorLength = (insensitive_spinsystem_get_number_of_ispins(self) == 0) ? insensitive_spinsystem_get_number_of_sspins(self) * self->gyroS : insensitive_spinsystem_get_number_of_ispins(self) * self->gyroI;
	xyzVector->x[1] /= vectorLength;
	xyzVector->y[1] /= vectorLength;
	xyzVector->z[1] /= vectorLength;

	return xyzVector;
}


///    // ///    /// //////       //////  //////  /////// //////   /////  //////// //  //////  ///    // ///////
////   // ////  //// //   //     //    // //   // //      //   // //   //    //    // //    // ////   // //
// //  // // //// // //////      //    // //////  /////   //////  ///////    //    // //    // // //  // ///////
//  // // //  //  // //   //     //    // //      //      //   // //   //    //    // //    // //  // //      //
//   //// //      // //   //      //////  //      /////// //   // //   //    //    //  //////  //   //// ///////

void insensitive_spinsystem_return_to_thermal_equilibrium(InsensitiveSpinSystem *self)
{
	set_thermal_equilibrium(self->matrix, self->spins, self->spinTypeArray, self->gyroI, self->gyroS);
    insensitive_spinsystem_free_gradient_array(self);
}


void insensitive_spinsystem_pulse(InsensitiveSpinSystem *self, float angle, float phase, int array, float *powerSpectrum)
{
	unsigned int i;
	int frequency;
	float currentAngle;

	for (i = 1; i < pow2(self->spins); i *= 2)
		if (i & array) {
			frequency = ROUND(insensitive_spinsystem_get_larmorfrequency_for_spin(self, lb(i)));
			currentAngle = angle;
			if (powerSpectrum != NULL) {
				if (abs(frequency) > 127)
					currentAngle = 0;
				else
					currentAngle *= powerSpectrum[(int)frequency + 128];
			}
			self->pulsePropagator = xy_rotation(self->spins, i, currentAngle, phase);
			equation_of_motion(self->matrix, self->pulsePropagator, self->size);
			free(self->pulsePropagator);
		}
	self->pulsePropagator = NULL;
}


void insensitive_spinsystem_offresonancepulse(InsensitiveSpinSystem *self, float gammaB1, float tp, float phase, float resFreq, unsigned int array)
{
	// The method expects strength to include the pulse shape factor at time tp.
	// The phase needs to include the phase from the pulse shape at time tp.

	unsigned int i, spin;
	float Omega_larmor;     // Larnor frequency Ω of the spin
	float omega_nut;        // Nutation frequency 1/2 γ B₁ * shape factor
	float omega_eff;        // Effective rotation frequency √(omega_nut² + Ω²)
	float theta;            // Tilt angle of the rotation axis (π/2 = on resonance)
	float flipAngle;        // Flip angle is omega_eff * tp

	for (i = 1; i < pow2(self->spins); i *= 2) {
		if (i & array) {
			spin = lb(i);
			Omega_larmor = self->couplingMatrix[spin * self->spins + spin] - resFreq;
			omega_nut = 0.5 * gammaB1;
			omega_eff = sqrtf(pow(omega_nut, 2) + pow(Omega_larmor, 2));
			theta = atan2f(omega_nut, Omega_larmor);
			flipAngle = omega_eff * tp; // Unit in rad or deg???
			self->pulsePropagator = arbitrary_rotation(self->spins, i, flipAngle, phase, theta);
			equation_of_motion(self->matrix, self->pulsePropagator, self->size);
			free(self->pulsePropagator);
		}
	}
	self->pulsePropagator = NULL;
}


void insensitive_spinsystem_chemicalshift(InsensitiveSpinSystem *self, float time, gboolean jitter)
{
	insensitive_spinsystem_chemicalshift_for_spins(self, time, jitter, self->size - 1);
}


void insensitive_spinsystem_chemicalshift_for_spins(InsensitiveSpinSystem *self, float time, gboolean jitter, unsigned int activeSpins)
{
	unsigned int i, j, index, array = self->size - 1;
	float noise = 0;

	/* spinlist: convert position X from 1 to 0 if chemical shift has been performed */
	for (i = 0; i < self->spins; i++) {
		index = pow2(i);
		if (array & index & activeSpins) {
			/* Test of chemical shift occurs more than once */
			if (!jitter) {
				for (j = i + 1; j < self->spins; j++) {
					if (self->couplingMatrix[i * self->spins + i] == self->couplingMatrix[j * self->spins + j]) {
						/* Add equivalent spin to array... */
						index += pow2(j);
						/* ...and delete it from the list */
						array -= pow2(j);
					}
				}
			} else {
				noise = random_noise(100);
			}
			if (self->couplingMatrix[i * self->spins + i] != 0) {
				self->shiftPropagator = z_rotation(self->spins, index, (self->couplingMatrix[i * self->spins + i] + noise) * 360 * time);
				equation_of_motion(self->matrix, self->shiftPropagator, self->size);
				free(self->shiftPropagator);
			}
		}
	}
	self->shiftPropagator = NULL;
}


void insensitive_spinsystem_jcoupling(InsensitiveSpinSystem *self, float time, enum CouplingMode strongCoupling)
{
	insensitive_spinsystem_jcoupling_for_spins(self, time, strongCoupling, self->size - 1);
}


void insensitive_spinsystem_jcoupling_for_spins(InsensitiveSpinSystem *self, float time, enum CouplingMode strongCoupling, unsigned int array)
{
	unsigned int i, n, m;
	float angle;
	DSPComplex z;

	for (n = 0; n < self->spins - 1; n++)
		for (m = n + 1; m < self->spins; m++) {
			if ((pow2(n) & array) && (pow2(m) & array)) {
				angle = self->couplingMatrix[n * self->spins + m] * time * M_PI;
				if (strongCoupling == SpinLockMode || (strongCoupling == StrongCouplingMode && (insensitive_spinsystem_get_spintype_for_spin(self, m) == insensitive_spinsystem_get_spintype_for_spin(self, n) || self->gyroI == self->gyroS))) {
					// Workaround for ssyevr-bug: Use eigenvalue calculation from Numerical Recipes (tred2 & tqli)
					self->couplingPropagator = strong_coupling_operator_workaround(self->spins, pow2(n), pow2(m), angle);
				} else {
					self->couplingPropagator = IzSz(pow2(n), pow2(m), self->spins);
					z = complex_rect(0, 0.5 * angle);
					cblas_cscal(self->size * self->size, &z, self->couplingPropagator, 1);
					for (i = 0; i < self->size; i++)
						self->couplingPropagator[i * self->size + i] = complex_rect(exp(self->couplingPropagator[i * self->size + i].real) * cos(self->couplingPropagator[i * self->size + i].imag),
													    exp(self->couplingPropagator[i * self->size + i].real) * sin(self->couplingPropagator[i * self->size + i].imag));
				}
				equation_of_motion(self->matrix, self->couplingPropagator, self->size);
				if (self->couplingPropagator != NULL)
					free(self->couplingPropagator);
			}
		}
	self->couplingPropagator = NULL;
}


void insensitive_spinsystem_simplerelaxation(InsensitiveSpinSystem *self, float time, float T1, float T2, gboolean spinlock)
{
	if (spinlock) {
		transverse_relaxation(self->matrix, self->size, T2, time);
		transverse_longitudinal_relaxation(self->matrix, self->size, T2, time);
	} else {
		transverse_relaxation(self->matrix, self->size, T2, time);
		longitudinal_relaxation(self->matrix, self->size, self->spinTypeArray, T1, time, self->gyroI, self->gyroS);
	}
}


void insensitive_spinsystem_dipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau)
{
	if (self->spins == 1)
		g_print("dipolar_relaxation: Only one spin present!\n");
	else {
		if (self->relaxationPropagator == NULL)
			self->relaxationPropagator = relaxation_matrix(self->spins, self->spinTypeArray, self->couplingMatrix, tau, self->absGyroI, self->absGyroS, 0);
		dipolar_relaxation(self->matrix, self->size, self->spinTypeArray, self->relaxationPropagator, time, self->gyroI, self->gyroS);
	}
}


void insensitive_spinsystem_transversedipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau)
{
	if (self->spins == 1)
		g_print("dipolar_relaxation: Only one spin present!\n");
	else {
		if (self->relaxationPropagator == NULL)
			self->relaxationPropagator = relaxation_matrix(self->spins, self->spinTypeArray, self->couplingMatrix, tau, self->absGyroI, self->absGyroS, 1);
		transverse_dipolar_relaxation(self->matrix, self->size, self->relaxationPropagator, time);
	}
}


void insensitive_spinsystem_switchtospinlockmode(InsensitiveSpinSystem *self, gboolean on)
{
	// Rotate coordinate system 90° around y-axis before relaxation, then rotate back
	if (on)
		self->pulsePropagator = xy_rotation(self->spins, pow2(self->spins) - 1, 90, 0);
	else
		self->pulsePropagator = xy_rotation(self->spins, pow2(self->spins) - 1, 90, 180);
	equation_of_motion(self->matrix, self->pulsePropagator, self->size);
	free(self->pulsePropagator);
	self->pulsePropagator = NULL;
}


void insensitive_spinsystem_reset_relaxationpropagator(InsensitiveSpinSystem *self)
{
	if (self->relaxationPropagator != NULL) {
		free(self->relaxationPropagator);
		self->relaxationPropagator = NULL;
	}
}


void insensitive_spinsystem_perform_decoupling(InsensitiveSpinSystem *self, gboolean iDecoupling, gboolean sDecoupling, float phase)
{
	unsigned int array = 0;

	if (iDecoupling)
		array += insensitive_spinsystem_get_pulsearray_for_spintype(self, spinTypeI);
	if (sDecoupling)
		array += insensitive_spinsystem_get_pulsearray_for_spintype(self, spinTypeS);

	insensitive_spinsystem_pulse(self, 180, phase + 90, array, NULL);
}


 //////  //////   /////  //////  // /////// ///    // //////// ///////
//       //   // //   // //   // // //      ////   //    //    //
//   /// //////  /////// //   // // /////   // //  //    //    ///////
//    // //   // //   // //   // // //      //  // //    //         //
 //////  //   // //   // //////  // /////// //   ////    //    ///////

gboolean insensitive_spinsystem_get_firstgradientpulseissued(InsensitiveSpinSystem *self)
{
	return self->firstGradientPulseIssued;
}


void insensitive_spinsystem_set_firstgradientpulseissued(InsensitiveSpinSystem *self, gboolean value)
{
	self->firstGradientPulseIssued = value;
}


void insensitive_spinsystem_create_gradientarray(InsensitiveSpinSystem *self)
{
	self->gradientArray = create_matrix_array_for_gradients(self->matrix, self->size, gradientSlices);
}


DSPComplex **insensitive_spinsystem_get_gradientarray(InsensitiveSpinSystem *self)
{
	return self->gradientArray;
}


void insensitive_spinsystem_set_gradientarray(InsensitiveSpinSystem *self, DSPComplex **array)
{
	unsigned int i;

	if (array != NULL && self->firstGradientPulseIssued)
		for (i = 0; i < gradientSlices; i++)
			cblas_ccopy(self->size * self->size, array[i], 1, self->gradientArray[i], 1);
}


void insensitive_spinsystem_set_actions_since_last_gradient(InsensitiveSpinSystem *self, InsensitivePulseSequence *sequence)
{
    g_object_unref(self->actionsSinceLastGradient);
    self->actionsSinceLastGradient = insensitive_pulsesequence_copy(sequence);
}


void insensitive_spinsystem_add_gradient_action(InsensitiveSpinSystem *self, enum SequenceType type, InsensitiveSettings *settings)
{
	// If there are more than one evolution times in a row, combine them into one
	if (type == SequenceTypeEvolution && insensitive_pulsesequence_get_number_of_elements(self->actionsSinceLastGradient) != 0) {
		SequenceElement *lastRecordedAction = insensitive_pulsesequence_get_last_element(self->actionsSinceLastGradient);
		if ((lastRecordedAction->type == SequenceTypeEvolution)
		    && (lastRecordedAction->iDecoupling == insensitive_settings_get_iDecoupling(settings))
		    && (lastRecordedAction->sDecoupling == insensitive_settings_get_sDecoupling(settings))) {
			lastRecordedAction->time += insensitive_settings_get_delay(settings);
		} else {
            insensitive_pulsesequence_add_element(self->actionsSinceLastGradient, type, settings);
		}
	} else {
		insensitive_pulsesequence_add_element(self->actionsSinceLastGradient, type, settings);
	}
}


void insensitive_spinsystem_gradient(InsensitiveSpinSystem *self, float strength, float time, InsensitiveSettings *settings)
{
	unsigned int z, i;
	float diffusionLosses, Delta = 0;
	DSPComplex *slice;

	if (!self->firstGradientPulseIssued) {
        insensitive_spinsystem_create_gradientarray(self);
	} else {
		Delta = insensitive_pulsesequence_gradient_perform_actions_on_spinsystem(self->actionsSinceLastGradient, self, settings);
        insensitive_pulsesequence_erase_sequence(self->actionsSinceLastGradient);
	}
	gradient(self->gradientArray, self->couplingMatrix, self->spins, self->spinTypeArray, strength, time, gradientSlices);
	// Perform diffusion dampening
	if (insensitive_settings_get_diffusion(settings) && self->firstGradientPulseIssued) {
		// Signal change: S(t) = S₀⋅exp(-Dγ²G²δ²(Δ-δ/3))
		// Gyromagnetic ratios of contributing spins?
		diffusionLosses = exp(-insensitive_settings_get_correlationTime(settings) * pow(strength, 2) * pow(time, 2) * (Delta - time / 3));
		for (z = 0; z < gradientSlices; z++) {
			slice = self->gradientArray[z];
			for (i = 0; i < self->size * self->size; i++) {
				slice[i].real *= diffusionLosses;
				slice[i].imag *= diffusionLosses;
			}
		}
	}
	average_matrix_array(self->matrix, self->gradientArray, self->size, gradientSlices);
	self->firstGradientPulseIssued = TRUE;
}


void insensitive_spinsystem_free_gradient_array(InsensitiveSpinSystem *self)
{
    unsigned int i;

    if(self->gradientArray != NULL) {
        for(i = 0; i < gradientSlices; i++)
            if(self->gradientArray[i] != NULL)
                free(self->gradientArray[i]);
        free(self->gradientArray);
        self->gradientArray = NULL;
        self->firstGradientPulseIssued = FALSE;
        insensitive_pulsesequence_erase_sequence(self->actionsSinceLastGradient);
    }
}


void insensitive_spinsystem_gradient_pulse(InsensitiveSpinSystem *self, float angle, float phase, int array, float *powerSpectrum)
{
    unsigned int i;
    int frequency;
    float currentAngle;

    for(i = 1; i < pow2(self->spins); i *= 2)
		if(i & array) {
			frequency = ROUND(insensitive_spinsystem_get_larmorfrequency_for_spin(self, lb(i)));
            currentAngle = angle;
            if(powerSpectrum != NULL)
                currentAngle *= powerSpectrum[128 + frequency];
            self->pulsePropagator = xy_rotation(self->spins, i, currentAngle, phase);
            for(i = 0; i < gradientSlices; i++)
                equation_of_motion(self->gradientArray[i], self->pulsePropagator, self->size);
            free(self->pulsePropagator);
		}
    self->pulsePropagator = NULL;
}


void insensitive_spinsystem_gradient_offresonancepulse(InsensitiveSpinSystem *self, float gammaB1, float tp, float phase, float resFreq, unsigned int array)
{
	// The method expects strength to include the pulse shape factor at time tp.
	// The phase needs to include the phase from the pulse shape at time tp.

	unsigned int i, j, spin;
	float Omega_larmor;     // Larmor frequency Ω of the spin
	float omega_nut;        // Nutation frequency 1/2 γ B₁ * shape factor
	float omega_eff;        // Effective rotation frequency √(omega_nut² + Ω²)
	float theta;            // Tilt angle of the rotation axis (π/2 = on resonance)
	float flipAngle;        // Flip angle is omega_eff * tp

	for (i = 1; i < pow2(self->spins); i *= 2) {
		if (i & array) {
			spin = lb(i);
			Omega_larmor = self->couplingMatrix[spin * self->spins + spin] - resFreq;
			omega_nut = 0.5 * gammaB1;
			omega_eff = sqrtf(pow(omega_nut, 2) + pow(Omega_larmor, 2));
			theta = atan2f(omega_nut, Omega_larmor);
			flipAngle = omega_eff * tp; // Unit in rad or deg???
			self->pulsePropagator = arbitrary_rotation(self->spins, i, flipAngle, phase, theta);
			for (j = 0; j < gradientSlices; j++)
				equation_of_motion(self->gradientArray[j], self->pulsePropagator, self->size);
			free(self->pulsePropagator);
		}
	}
	self->pulsePropagator = NULL;
}


void insensitive_spinsystem_gradient_chemicalshift(InsensitiveSpinSystem *self, float time)
{
	unsigned int i, j, index, array = self->size - 1;

    /* spinlist: convert position X from 1 to 0 if chemical shift has been performed */
	for(i = 0; i < self->spins; i++) {
		index = pow2(i);
		if(array & index) {
            // Test of chemical shift occurs more than once
			for(j = i + 1; j < self->spins; j++)
				if(self->couplingMatrix[i * self->spins + i] == self->couplingMatrix[j * self->spins + j]) {
                    // Add equivalent spin to array...
					index += pow2(j);
                    // ...and delete it from the list
					array -= pow2(j);
				}
            if(self->couplingMatrix[i * self->spins + i] != 0) {
                self->shiftPropagator = z_rotation(self->spins, index, self->couplingMatrix[i * self->spins + i] * 360 * time);
                for(i = 0; i < gradientSlices; i++)
                    equation_of_motion(self->gradientArray[i], self->shiftPropagator, self->size);
                free(self->shiftPropagator);
            }
		}
	}
    self->shiftPropagator = NULL;
}


void insensitive_spinsystem_gradient_jcoupling(InsensitiveSpinSystem *self, float time, enum CouplingMode strongCoupling)
{
    unsigned int i, n, m;
	float angle;
	DSPComplex z;

	for (n = 0; n < self->spins - 1; n++)
		for (m = n + 1; m < self->spins; m++) {
			angle = self->couplingMatrix[n * self->spins + m] * time * M_PI;
			if (strongCoupling && (insensitive_spinsystem_get_spintype_for_spin(self, m) == insensitive_spinsystem_get_spintype_for_spin(self, n)))
				// Workaround for ssyevr-bug: Use eigenvalue calculation from Numerical Recipes (tred2 & tqli)
				self->couplingPropagator = strong_coupling_operator(self->spins, pow2(n), pow2(m), angle);
			else {
				self->couplingPropagator = IzSz(pow2(n), pow2(m), self->spins);
				z = complex_rect(0, 0.5 * angle);
				cblas_cscal(self->size * self->size, &z, self->couplingPropagator, 1);
				for (i = 0; i <self-> size; i++)
                    self->couplingPropagator[i * self->size + i] = complex_rect(exp(self->couplingPropagator[i * self->size + i].real) * cos(self->couplingPropagator[i * self->size + i].imag),
													    exp(self->couplingPropagator[i * self->size + i].real) * sin(self->couplingPropagator[i * self->size + i].imag));
			}
			for (i = 0; i < gradientSlices; i++)
				equation_of_motion(self->gradientArray[i], self->couplingPropagator, self->size);
			free(self->couplingPropagator);
		}
	self->couplingPropagator = NULL;
}


void insensitive_spinsystem_gradient_simplerelaxation(InsensitiveSpinSystem *self, float time, float T1, float T2)
{
    unsigned int i;

    for(i = 0; i < gradientSlices; i++) {
        transverse_relaxation(self->gradientArray[i], self->size, T2, time);
        longitudinal_relaxation(self->gradientArray[i], self->size, self->spinTypeArray, T1, time, self->gyroI, self->gyroS);
    }
}


void insensitive_spinsystem_gradient_dipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau)
{
	unsigned int i, j, relaxationIterations;

    if(self->spins == 1)
        puts("dipolar_relaxation: Only one spin present!");
    else {
        if(self->relaxationPropagator == NULL)
			self->relaxationPropagator = relaxation_matrix(self->spins, self->spinTypeArray, self->couplingMatrix, tau, self->absGyroI, self->absGyroS, 0);
        relaxationIterations = time * 1e3;
        for(j = 0; j < gradientSlices; j++)
            for(i = 0; i < relaxationIterations; i++)
                dipolar_relaxation(self->gradientArray[i], self->size, self->spinTypeArray, self->relaxationPropagator, time / relaxationIterations, self->gyroI, self->gyroS);
	}
}


void insensitive_spinsystem_gradient_transversedipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau)
{
	unsigned int i, j, relaxationIterations;

    if(self->spins == 1)
        puts("dipolar_relaxation: Only one spin present!");
    else {
        if(self->relaxationPropagator == NULL)
			self->relaxationPropagator = relaxation_matrix(self->spins, self->spinTypeArray, self->couplingMatrix, tau, self->absGyroI, self->absGyroS, 1);
        relaxationIterations = time * 1e3;
        for(j = 0; j < gradientSlices; j++)
            for(i = 0; i < relaxationIterations; i++) {
                // Rotate coordinate system 90° around y-axis before relaxation, then rotate back
                self->pulsePropagator = xy_rotation(self->spins, pow2(self->spins) - 1, 90, 0);
                equation_of_motion(self->matrix, self->pulsePropagator, self->size);
                free(self->pulsePropagator);
                dipolar_relaxation(self->gradientArray[i], self->size, self->spinTypeArray, self->relaxationPropagator, time / relaxationIterations, self->gyroI, self->gyroS);
                self->pulsePropagator = xy_rotation(self->spins, pow2(self->spins) - 1, 90, 180);
                equation_of_motion(self->matrix, self->pulsePropagator, self->size);
                free(self->pulsePropagator);
                self->pulsePropagator = NULL;
            }
	}
}


void insensitive_spinsystem_gradient_perform_decoupling(InsensitiveSpinSystem *self, gboolean iDecoupling, gboolean sDecoupling, float phase)
{
	unsigned int array = 0;

	if (iDecoupling)
		array += insensitive_spinsystem_get_pulsearray_for_spintype(self, spinTypeI);
	if (sDecoupling)
		array += insensitive_spinsystem_get_pulsearray_for_spintype(self, spinTypeS);

	insensitive_spinsystem_gradient_pulse(self, 180, phase + 90, array, NULL);
}
