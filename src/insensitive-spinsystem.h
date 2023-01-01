/* insensitive-spinsystem.h
 *
 * Copyright 2009-2023 Klaus Boldt
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

#ifndef __INSENSITIVE_SPINSYSTEM_H__
#define __INSENSITIVE_SPINSYSTEM_H__

#pragma once

#include <glib-object.h>

#include "insensitive.h"
#include "insensitive-library.h"
#include "insensitive-pulsesequence.h"

struct _InsensitiveSpinSystem {
	GObject parent_instance;

	unsigned int spins, spinsxspins;
	unsigned int size;
	unsigned int spinTypeArray;
	float gyroI, gyroS, absGyroI, absGyroS;

	DSPComplex *matrix;
	DSPComplex *pulsePropagator, *shiftPropagator, *couplingPropagator, *relaxationPropagator;
	float *couplingMatrix;

	DSPComplex **gradientArray;
	gboolean firstGradientPulseIssued;
	InsensitivePulseSequence *actionsSinceLastGradient;
};


G_BEGIN_DECLS

#define INSENSITIVE_TYPE_SPINSYSTEM (insensitive_spinsystem_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveSpinSystem, insensitive_spinsystem, INSENSITIVE, SPINSYSTEM, GObject)

InsensitiveSpinSystem *insensitive_spinsystem_new(void);
void insensitive_spinsystem_init_with_spins(InsensitiveSpinSystem *self, unsigned int number);
InsensitiveSpinSystem *insensitive_spinsystem_copy(InsensitiveSpinSystem *self);

/* Spin Number and Type */
unsigned int insensitive_spinsystem_get_spins(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_set_spins(InsensitiveSpinSystem *spinsystem, unsigned int number);
void insensitive_spinsystem_add_spin(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_remove_spin(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_remove_spin_number(InsensitiveSpinSystem *spinsystem, unsigned int number);
unsigned int insensitive_spinsystem_get_size(InsensitiveSpinSystem *spinsystem);
unsigned int insensitive_spinsystem_get_spintypearray(InsensitiveSpinSystem *spinsystem);
unsigned int insensitive_spinsystem_get_pulsearray_for_spintype(InsensitiveSpinSystem *spinsystem, unsigned int spinType);
unsigned int insensitive_spinsystem_get_spintype_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int number);
void insensitive_spinsystem_set_spintypearray(InsensitiveSpinSystem *spinsystem, unsigned int value);
void insensitive_spinsystem_set_spintype_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int type, unsigned int number);
unsigned int insensitive_spinsystem_get_number_of_ispins(InsensitiveSpinSystem *spinsystem);
unsigned int insensitive_spinsystem_get_number_of_sspins(InsensitiveSpinSystem *spinsystem);

/* Coupling Matrix */
float *insensitive_spinsystem_get_raw_couplingmatrix(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_init_couplingmatrix(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_resize_couplingmatrix_for_spins(InsensitiveSpinSystem *spinsystem, unsigned int number);
void insensitive_spinsystem_substitute_couplingmatrix(InsensitiveSpinSystem *spinsystem, float *newCouplingMatrix);
float insensitive_spinsystem_get_larmorfrequency_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber);
void insensitive_spinsystem_set_larmorfrequency_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber, float chemicalShift);
float insensitive_spinsystem_get_jcouplingconstant_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2);
void insensitive_spinsystem_set_jcouplingconstant_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2, float couplingConstant);
float insensitive_spinsystem_get_dipolarcouplingconstant_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2);
float insensitive_spinsystem_get_distance_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2);
void insensitive_spinsystem_set_dipolarcouplingconstant_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2, float b);
void insensitive_spinsystem_set_distance_between_spins(InsensitiveSpinSystem *spinsystem, unsigned int spinNumber1, unsigned int spinNumber2, float r);
void insensitive_spinsystem_constants_were_changed(InsensitiveSpinSystem *spinsystem);

/* Density Matrix */
DSPComplex *insensitive_spinsystem_get_raw_densitymatrix(InsensitiveSpinSystem *spinsystem);
DSPComplex insensitive_spinsystem_get_matrixelement(InsensitiveSpinSystem *spinsystem, unsigned int row, unsigned int col);
void insensitive_spinsystem_substitute_matrix(InsensitiveSpinSystem *spinsystem, DSPComplex *newMatrix);
float insensitive_spinsystem_get_expectationvalue_x_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int spin);
float insensitive_spinsystem_get_expectationvalue_y_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int spin);
float insensitive_spinsystem_get_expectationvalue_z_for_spin(InsensitiveSpinSystem *spinsystem, unsigned int spin);
gchar *insensitive_spinsystem_get_cartesianOperatorString(InsensitiveSpinSystem *spinsystem);
gchar *insensitive_spinsystem_get_sphericalOperatorString(InsensitiveSpinSystem *spinsystem);
VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_coherences(InsensitiveSpinSystem *spinsystem, unsigned int highlighted);
VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_moments(InsensitiveSpinSystem *spinsystem, unsigned int highlighted);
VectorCoordinates *insensitive_spinsystem_get_vectorrepresentation_fid(InsensitiveSpinSystem *spinsystem);

/* NMR Operations */
void insensitive_spinsystem_return_to_thermal_equilibrium(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_pulse(InsensitiveSpinSystem *spinsystem, float angle, float phase, int array, float *powerSpectrum);
void insensitive_spinsystem_offresonancepulse(InsensitiveSpinSystem *spinsystem, float gammaB1, float tp, float phase, float resFreq, unsigned int array);
void insensitive_spinsystem_chemicalshift(InsensitiveSpinSystem *spinsystem, float time, gboolean jitter);
void insensitive_spinsystem_chemicalshift_for_spins(InsensitiveSpinSystem *spinsystem, float time, gboolean jitter, unsigned int activeSpins);
void insensitive_spinsystem_jcoupling(InsensitiveSpinSystem *spinsystem, float time, enum CouplingMode strongCoupling);
void insensitive_spinsystem_jcoupling_for_spins(InsensitiveSpinSystem *spinsystem, float time, enum CouplingMode strongCoupling, unsigned int array);
void insensitive_spinsystem_simplerelaxation(InsensitiveSpinSystem *spinsystem, float time, float T1, float T2, gboolean spinlock);
void insensitive_spinsystem_dipolarrelaxation(InsensitiveSpinSystem *spinsystem, float time, float tau);
void insensitive_spinsystem_transversedipolarrelaxation(InsensitiveSpinSystem *spinsystem, float time, float tau);
void insensitive_spinsystem_switchtospinlockmode(InsensitiveSpinSystem *spinsystem, gboolean on);
void insensitive_spinsystem_reset_relaxationpropagator(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_perform_decoupling(InsensitiveSpinSystem *spinsystem, gboolean iDecoupling, gboolean sDecoupling, float phase);

/* Gradients */
gboolean insensitive_spinsystem_get_firstgradientpulseissued(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_set_firstgradientpulseissued(InsensitiveSpinSystem *spinsystem, gboolean value);
void insensitive_spinsystem_create_gradientarray(InsensitiveSpinSystem *spinsystem);
DSPComplex **insensitive_spinsystem_get_gradientarray(InsensitiveSpinSystem *spinsystem);
void insensitive_spinsystem_set_gradientarray(InsensitiveSpinSystem *spinsystem, DSPComplex **array);
void insensitive_spinsystem_set_actions_since_last_gradient(InsensitiveSpinSystem *self, InsensitivePulseSequence *sequence);
void insensitive_spinsystem_add_gradient_action(InsensitiveSpinSystem *self, enum SequenceType type, InsensitiveSettings *settings);
void insensitive_spinsystem_gradient(InsensitiveSpinSystem *self, float strength, float time, InsensitiveSettings *settings);
void insensitive_spinsystem_free_gradient_array(InsensitiveSpinSystem *self);
void insensitive_spinsystem_gradient_pulse(InsensitiveSpinSystem *self, float angle, float phase, int array, float *powerSpectrum);
void insensitive_spinsystem_gradient_offresonancepulse(InsensitiveSpinSystem *self, float gammaB1, float tp, float phase, float resFreq, unsigned int array);
void insensitive_spinsystem_gradient_chemicalshift(InsensitiveSpinSystem *self, float time);
void insensitive_spinsystem_gradient_jcoupling(InsensitiveSpinSystem *self, float time, enum CouplingMode strongCoupling);
void insensitive_spinsystem_gradient_simplerelaxation(InsensitiveSpinSystem *self, float time, float T1, float T2);
void insensitive_spinsystem_gradient_dipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau);
void insensitive_spinsystem_gradient_transversedipolarrelaxation(InsensitiveSpinSystem *self, float time, float tau);
void insensitive_spinsystem_gradient_perform_decoupling(InsensitiveSpinSystem *self, gboolean iDecoupling, gboolean sDecoupling, float phase);

G_END_DECLS

#endif /* __INSENSITIVE_SPINSYSTEM_H__ */
