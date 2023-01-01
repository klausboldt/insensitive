/* insensitive-pulsesequence.c
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
 * EXPRESS OR IMPLIED, INCLUDING BUT FALSET LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * FALSENINFRINGEMENT. IN FALSE EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written
 * authorization.
 */

#include "insensitive.h"
#include "insensitive-config.h"
#include "insensitive-library.h"
#include "insensitive-pulsesequence.h"
#include "insensitive-spinsystem.h"
#include "insensitive-controller.h"


G_DEFINE_TYPE(InsensitivePulseSequence, insensitive_pulsesequence, G_TYPE_OBJECT)


InsensitivePulseSequence* insensitive_pulsesequence_new(){
	return (InsensitivePulseSequence *)g_object_new(INSENSITIVE_TYPE_PULSESEQUENCE, NULL);
}


static void insensitive_pulsesequence_dispose(GObject *gobject)
{
	InsensitivePulseSequence *self = (InsensitivePulseSequence *)gobject;

	/* In dispose(), you are supposed to free all types referenced from this
	 * object which might themselves hold a reference to self. Generally,
	 * the most simple solution is to unref all members on which you own a
	 * reference.
	 */

	/* dispose() might be called multiple times, so we must guard against
	 * calling g_object_unref() on an invalid GObject by setting the member
	 * NULL; g_clear_object() does this for us.
	 */
	//g_clear_object(&gobject_member);

	/* [sequence release]; */

	G_OBJECT_CLASS(insensitive_pulsesequence_parent_class)->dispose(gobject);
}


static void insensitive_pulsesequence_finalize(GObject *gobject)
{
	InsensitivePulseSequence *self = (InsensitivePulseSequence *)gobject;

	g_ptr_array_free(self->sequence, TRUE);

	G_OBJECT_CLASS(insensitive_pulsesequence_parent_class)->finalize(gobject);
}


static void insensitive_pulsesequence_class_init(InsensitivePulseSequenceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = insensitive_pulsesequence_dispose;
	object_class->finalize = insensitive_pulsesequence_finalize;
}


static void insensitive_pulsesequence_init(InsensitivePulseSequence *self)
{
	self->sequence = g_ptr_array_new();
	self->position = -1;
}


InsensitivePulseSequence *insensitive_pulsesequence_copy(InsensitivePulseSequence *self)
{
    InsensitivePulseSequence *copy;

    copy = insensitive_pulsesequence_new();
    insensitive_pulsesequence_set_position(copy, self->position);
    insensitive_pulsesequence_set_sequenceArray(copy, self->sequence);

    return copy;
}


void insensitive_pulsesequence_add_element(InsensitivePulseSequence *self, enum SequenceType type, InsensitiveSettings *settings)
{
	SequenceElement *element;

	element = calloc(1, sizeof(SequenceElement));
	element->type = type;
	element->iDecoupling = insensitive_settings_get_iDecoupling(settings);
	element->sDecoupling = insensitive_settings_get_sDecoupling(settings);
	element->spinlock = insensitive_settings_get_spinlock(settings);
	switch (type) {
	case SequenceTypePulse:
		element->time = insensitive_settings_get_flipAngle(settings);
		element->secondParameter = insensitive_settings_get_phase(settings);
		element->pulseArray = insensitive_settings_get_pulseArray(settings);
		element->activeISpins = insensitive_settings_get_allISpinsSelected(settings);
		element->activeSSpins = insensitive_settings_get_allSSpinsSelected(settings);
		element->selectiveIPulse = insensitive_settings_get_someISpinsSelected(settings);
		element->selectiveSPulse = insensitive_settings_get_someSSpinsSelected(settings);
		element->pulseDuration = insensitive_settings_get_pulseDuration(settings);
		element->pulseStrength = insensitive_settings_get_pulseStrength(settings);
		element->pulseFrequency = insensitive_settings_get_pulseFrequency(settings);
		element->pulseEnvelope = insensitive_settings_get_pulseEnvelope(settings);
		break;
	case SequenceTypeEvolution:
	case SequenceTypeShift:
	case SequenceTypeCoupling:
	case SequenceTypeRelaxation:
		element->time = insensitive_settings_get_delay(settings);
		element->secondParameter = insensitive_settings_get_phase(settings);
		element->activeISpins = FALSE;
		element->activeSSpins = FALSE;
		break;
	case SequenceTypeGradient:
		element->time = insensitive_settings_get_gradientDuration(settings);
		element->secondParameter = insensitive_settings_get_gradientStrength(settings);
		element->activeISpins = FALSE;
		element->activeSSpins = FALSE;
		break;
	case SequenceTypeFID:
		element->secondParameter = insensitive_settings_get_phase(settings);
		element->activeISpins = insensitive_settings_get_detectISpins(settings);
		element->activeSSpins = insensitive_settings_get_detectSSpins(settings);
		break;
	}
	g_ptr_array_add(self->sequence, element);
	self->position = self->sequence->len - 1;
}


void insensitive_pulsesequence_add_sequence_element(InsensitivePulseSequence *self, SequenceElement *newElement)
{
	SequenceElement *element;

	element = calloc(1, sizeof(SequenceElement));
	element->type = newElement->type;
	element->time = newElement->time;
	element->secondParameter = newElement->secondParameter;
	element->pulseArray = newElement->pulseArray;
	element->iDecoupling = newElement->iDecoupling;
	element->sDecoupling = newElement->sDecoupling;
	element->spinlock = newElement->spinlock;
	element->activeISpins = newElement->activeISpins;
	element->activeSSpins = newElement->activeSSpins;
	element->selectiveIPulse = newElement->selectiveIPulse;
	element->selectiveSPulse = newElement->selectiveSPulse;
	element->pulseDuration = newElement->pulseDuration;
	element->pulseStrength = newElement->pulseStrength;
	element->pulseFrequency = newElement->pulseFrequency;
	element->pulseEnvelope = newElement->pulseEnvelope;
	g_ptr_array_add(self->sequence, element);
	self->position = self->sequence->len - 1;
}


void insensitive_pulsesequence_erase_sequence(InsensitivePulseSequence *self)
{
	int i;

	for (i = self->sequence->len - 1; i >= 0; i--)
		g_free(g_ptr_array_index(self->sequence, i));
	g_ptr_array_remove_range(self->sequence, 0, self->sequence->len);
	self->position = -1;
}


int insensitive_pulsesequence_get_number_of_elements(InsensitivePulseSequence *self)
{
	return self->sequence->len;
}


int insensitive_pulsesequence_get_position(InsensitivePulseSequence *self)
{
	return self->position;
}


void insensitive_pulsesequence_set_position(InsensitivePulseSequence *self, int value)
{
	self->position = value;
}


GPtrArray *insensitive_pulsesequence_get_sequenceArray(InsensitivePulseSequence *self)
{
	return self->sequence;
}


void insensitive_pulsesequence_set_sequenceArray(InsensitivePulseSequence *self, GPtrArray *array)
{
	unsigned int i;
	SequenceElement *element, *originalElement;

	insensitive_pulsesequence_erase_sequence(self);
	for (i = 0; i < array->len; i++) {
		element = malloc(sizeof(SequenceElement));
		originalElement = g_ptr_array_index(array, i);
		element->type = originalElement->type;
		element->time = originalElement->time;
		element->secondParameter = originalElement->secondParameter;
		element->pulseArray = originalElement->pulseArray;
		element->iDecoupling = originalElement->iDecoupling;
		element->sDecoupling = originalElement->sDecoupling;
		element->spinlock = originalElement->spinlock;
		element->activeISpins = originalElement->activeISpins;
		element->activeSSpins = originalElement->activeSSpins;
		element->selectiveIPulse = originalElement->selectiveIPulse;
		element->selectiveSPulse = originalElement->selectiveSPulse;
		element->pulseDuration = originalElement->pulseDuration;
		element->pulseStrength = originalElement->pulseStrength;
		element->pulseFrequency = originalElement->pulseFrequency;
		element->pulseEnvelope = originalElement->pulseEnvelope;
		g_ptr_array_add(self->sequence, element);
	}
}


SequenceElement *insensitive_pulsesequence_get_element_at_index(InsensitivePulseSequence *self, unsigned int index)
{
	if (index < self->sequence->len)
		return g_ptr_array_index(self->sequence, index);
	else
		return NULL;
}


SequenceElement *insensitive_pulsesequence_get_last_element(InsensitivePulseSequence *self)
{
	if (self->sequence->len == 0)
		return NULL;
	else
		return g_ptr_array_index(self->sequence, self->sequence->len - 1);
}


float insensitive_pulsesequence_perform_actions_on_spinsystem(InsensitivePulseSequence *self,
							      gpointer spinsystem_,
							      unsigned int start,
							      unsigned int stride,
							      InsensitiveSettings *settings,
							      gpointer controller_)
{
	InsensitiveSpinSystem *spinsystem = (InsensitiveSpinSystem *)spinsystem_;
	InsensitiveController *controller = (InsensitiveController *)controller_;
	unsigned int i, j, steps, s, pulseArray, end, danteCounter;
	float stepDuration, phase, strength;
	float rest, totalEvolutionTime = 0;
	float tempTime, tempSecondParameter, tempPulseArray;
	gboolean spinlockWasOnBefore;
	SequenceElement *element;
	enum PulseEnvelope savedPulseEnvelope = insensitive_settings_get_pulseEnvelope(settings);
	unsigned int spinTypeArray = insensitive_spinsystem_get_spintypearray(spinsystem);
	unsigned int spins = insensitive_spinsystem_get_spins(spinsystem);
	float dwellTime = insensitive_settings_get_dwellTime(settings);

	spinlockWasOnBefore = FALSE;
	if (stride == 0)
		end = self->sequence->len;
	else
		end = start + stride;
	for (i = start; i < end; i++) {
		element = g_ptr_array_index(self->sequence, i);
		if (element->spinlock != spinlockWasOnBefore)
			insensitive_spinsystem_reset_relaxationpropagator(spinsystem);
		spinlockWasOnBefore = element->spinlock;
		// Make sure that the spinlock includes both spin types
		if (element->spinlock) {
			element->iDecoupling = (insensitive_spinsystem_get_number_of_ispins(spinsystem) > 0);
			element->sDecoupling = (insensitive_spinsystem_get_number_of_sspins(spinsystem) > 0);
		}
		switch (element->type) {
		case SequenceTypePulse:
			// Determine appropriate pulse array:
			// If a hard pulse is shown, pulse all spins of that type
			// If a selective pulse is shown use the saved pulse array for that spin type
			pulseArray = 0;
			if (element->activeISpins || element->selectiveIPulse)
				pulseArray += (spinTypeArray ^ (pow2(spins) - 1)) & (pow2(spins) - 1);
			if (element->activeSSpins || element->selectiveSPulse)
				pulseArray += spinTypeArray;
			if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
				tempTime = insensitive_settings_get_flipAngle(settings);
				tempSecondParameter = insensitive_settings_get_phase(settings);
				tempPulseArray = insensitive_settings_get_pulseArray(settings);
				insensitive_settings_set_flipAngle(settings, element->time);
				insensitive_settings_set_phase(settings, element->secondParameter);
				insensitive_settings_set_pulseArray(settings, pulseArray, spins, spinTypeArray);
                insensitive_spinsystem_add_gradient_action(spinsystem, SequenceTypePulse, settings);
				insensitive_settings_set_flipAngle(settings, tempTime);
				insensitive_settings_set_phase(settings, tempSecondParameter);
				insensitive_settings_set_pulseArray(settings, tempPulseArray, spins, spinTypeArray);
				insensitive_settings_set_pulseDuration(settings, element->pulseDuration);
				insensitive_settings_set_pulseStrength(settings, element->pulseStrength);
				insensitive_settings_set_pulseFrequency(settings, element->pulseFrequency);
				insensitive_settings_set_pulseEnvelope(settings, element->pulseEnvelope);
			}
			if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(settings)) {
				insensitive_spinsystem_pulse(spinsystem, element->time, element->secondParameter, pulseArray, NULL);
			} else {
				stepDuration = element->pulseDuration / pulseShapeResolution;
				insensitive_settings_set_pulseEnvelope(settings, element->pulseEnvelope);
				danteCounter = (element->pulseEnvelope == DANTE) ? maxDanteCycles : 1;
				for ( ; danteCounter > 0; danteCounter--) {
					for (j = 0; j < pulseShapeResolution; j++) {
						strength = insensitive_settings_get_pulseShape(settings).realp[j] * element->pulseStrength;
						phase = insensitive_settings_get_pulseShape(settings).imagp[j] + element->secondParameter;
						while (phase > 360) {
							phase -= 360;
						}
						insensitive_spinsystem_offresonancepulse(spinsystem, strength * 720, stepDuration, phase, element->pulseFrequency, pulseArray);
					}
				}
			}
			break;
		case SequenceTypeShift:
			if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
				tempTime = insensitive_settings_get_delay(settings);
				insensitive_settings_set_delay(settings, element->time);
                insensitive_spinsystem_add_gradient_action(spinsystem, SequenceTypeShift, settings);
				insensitive_settings_set_delay(settings, tempTime);
			}
			if (insensitive_settings_get_spinlock(settings))
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			insensitive_spinsystem_chemicalshift(spinsystem, element->time, insensitive_settings_get_dephasingJitter(settings));
			if (insensitive_settings_get_spinlock(settings))
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeCoupling:
			if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
				tempTime = insensitive_settings_get_delay(settings);
				insensitive_settings_set_delay(settings, element->time);
				insensitive_spinsystem_add_gradient_action(spinsystem, SequenceTypeCoupling, settings);
				insensitive_settings_set_delay(settings, tempTime);
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			if (rest != 0) {
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
					insensitive_spinsystem_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
					insensitive_spinsystem_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				} else
					insensitive_spinsystem_jcoupling(spinsystem, rest, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
			}
			for (s = 0; s < steps; s++) {
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
					insensitive_spinsystem_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
					insensitive_spinsystem_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				} else
					insensitive_spinsystem_jcoupling(spinsystem, dwellTime, insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeRelaxation:
			if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
				tempTime = insensitive_settings_get_delay(settings);
				insensitive_settings_set_delay(settings, element->time);
				insensitive_spinsystem_add_gradient_action(spinsystem, SequenceTypeRelaxation, settings);
				insensitive_settings_set_delay(settings, tempTime);
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
				if (rest != 0) {
					if (element->spinlock)
						insensitive_spinsystem_transversedipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
					else
						insensitive_spinsystem_dipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
				}
				for (s = 0; s < steps; s++) {
					if (element->spinlock)
						insensitive_spinsystem_transversedipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
					else
						insensitive_spinsystem_dipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
				}
			} else {
				if (rest != 0)
					insensitive_spinsystem_simplerelaxation(spinsystem, rest, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings), element->spinlock);
				for (s = 0; s < steps; s++)
					insensitive_spinsystem_simplerelaxation(spinsystem, dwellTime, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings), element->spinlock);
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeEvolution:
			totalEvolutionTime += element->time;
			if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
				tempTime = insensitive_settings_get_delay(settings);
				insensitive_settings_set_delay(settings, element->time);
				insensitive_spinsystem_add_gradient_action(spinsystem, SequenceTypeEvolution, settings);
				insensitive_settings_set_delay(settings, tempTime);
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			// Perform step of rest
			if (rest != 0) {
				insensitive_spinsystem_chemicalshift(spinsystem,
													 rest,
													 insensitive_settings_get_dephasingJitter(settings));
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_jcoupling(spinsystem,
													 rest / 2,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
					insensitive_spinsystem_perform_decoupling(spinsystem,
															  element->iDecoupling,
															  element->sDecoupling,
															  element->secondParameter);
					insensitive_spinsystem_jcoupling(spinsystem,
													 rest / 2,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				} else
					insensitive_spinsystem_jcoupling(spinsystem,
													 rest,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				if (insensitive_settings_get_relaxationWithEvolution(settings)) {
					if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
						if (element->spinlock)
							insensitive_spinsystem_transversedipolarrelaxation(spinsystem,
																			   rest,
																			   insensitive_settings_get_correlationTime(settings));
						else
							insensitive_spinsystem_dipolarrelaxation(spinsystem,
																	 rest,
																	 insensitive_settings_get_correlationTime(settings));
					} else
						insensitive_spinsystem_simplerelaxation(spinsystem,
																rest,
																insensitive_settings_get_T1(settings),
																insensitive_settings_get_T2(settings),
																element->spinlock);
				}
			}
			// Perform in steps of dwell time
			for (s = 0; s < steps; s++) {
				insensitive_spinsystem_chemicalshift(spinsystem,
													 dwellTime,
													 insensitive_settings_get_dephasingJitter(settings));
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_jcoupling(spinsystem,
													 dwellTime / 2,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
					insensitive_spinsystem_perform_decoupling(spinsystem,
															  element->iDecoupling,
															  element->sDecoupling,
															  element->secondParameter);
					insensitive_spinsystem_jcoupling(spinsystem,
													 dwellTime / 2,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				} else
					insensitive_spinsystem_jcoupling(spinsystem,
													 dwellTime,
													 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
				if (insensitive_settings_get_relaxationWithEvolution(settings)) {
					if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
						if (element->spinlock)
							insensitive_spinsystem_transversedipolarrelaxation(spinsystem,
																			   dwellTime,
																			   insensitive_settings_get_correlationTime(settings));
						else
							insensitive_spinsystem_dipolarrelaxation(spinsystem,
																	 dwellTime,
																	 insensitive_settings_get_correlationTime(settings));
					} else
						insensitive_spinsystem_simplerelaxation(spinsystem,
																dwellTime,
																insensitive_settings_get_T1(settings),
																insensitive_settings_get_T2(settings),
																element->spinlock);
				}
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeGradient:
			totalEvolutionTime += element->time;
            insensitive_spinsystem_gradient(spinsystem, element->secondParameter , element->time, settings);
			break;
		case SequenceTypeFID:
			if (controller != NULL) {
				if (element->spinlock) {
					insensitive_controller_set_spinlock(controller, TRUE);
				} else {
					insensitive_controller_set_spinlock(controller, FALSE);
					insensitive_controller_set_IDecoupling(controller, element->iDecoupling);
					insensitive_controller_set_SDecoupling(controller, element->sDecoupling);
				}
				insensitive_controller_set_detectISignal(controller, element->activeISpins);
				insensitive_controller_set_detectSSignal(controller, element->activeSSpins);
				insensitive_controller_restore_relaxation_with_evolution(controller);
				insensitive_controller_perform_acquisition(controller);
			}
			break;
		}
	}
	insensitive_settings_set_pulseEnvelope(settings, savedPulseEnvelope);

	return totalEvolutionTime;
}


float insensitive_pulsesequence_gradient_perform_actions_on_spinsystem(InsensitivePulseSequence *self,
								                                       gpointer spinsystem_,
                                                                       InsensitiveSettings *settings)
{
    InsensitiveSpinSystem *spinsystem = (InsensitiveSpinSystem *)spinsystem_;
	unsigned int i, j, steps, s, pulseArray, danteCounter;
	float stepDuration, phase, strength;
	float rest, totalEvolutionTime = 0;
	gboolean spinlockWasOnBefore;
	SequenceElement *element;
	enum PulseEnvelope savedPulseEnvelope = insensitive_settings_get_pulseEnvelope(settings);
	unsigned int spinTypeArray = insensitive_spinsystem_get_spintypearray(spinsystem);
	unsigned int spins = insensitive_spinsystem_get_spins(spinsystem);
	float dwellTime = insensitive_settings_get_dwellTime(settings);

	spinlockWasOnBefore = FALSE;
	for (i = 0; i < self->sequence->len; i++) {
        element = g_ptr_array_index(self->sequence, i);
		if (element->spinlock != spinlockWasOnBefore)
			insensitive_spinsystem_reset_relaxationpropagator(spinsystem);
		spinlockWasOnBefore = element->spinlock;
		// Make sure that the spinlock includes both spin types
        if (element->spinlock) {
			element->iDecoupling = (insensitive_spinsystem_get_number_of_ispins(spinsystem) > 0);
			element->sDecoupling = (insensitive_spinsystem_get_number_of_sspins(spinsystem) > 0);
		}
		switch (element->type) {
		case SequenceTypePulse:
			// Determine appropriate pulse array:
			// If a hard pulse is shown, pulse all spins of that type
			// If a selective pulse is shown use the saved pulse array for that spin type
			pulseArray = 0;
			if (element->activeISpins || element->selectiveIPulse)
				pulseArray += (spinTypeArray ^ (pow2(spins) - 1)) & (pow2(spins) - 1);
			if (element->activeSSpins || element->selectiveSPulse)
				pulseArray += spinTypeArray;
			if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(settings)) {
                insensitive_spinsystem_gradient_pulse(spinsystem,
                                                      element->time,
                                                      element->secondParameter,
                                                      element->pulseArray,
                                                      NULL);
			} else {
				stepDuration = element->pulseDuration / pulseShapeResolution;
                insensitive_settings_set_pulseEnvelope(settings, element->pulseEnvelope);
				danteCounter = (element->pulseEnvelope == DANTE) ? maxDanteCycles : 1;
				for ( ; danteCounter > 0; danteCounter--) {
					for (j = 0; j < pulseShapeResolution; j++) {
						strength = insensitive_settings_get_pulseShape(settings).realp[j] * element->pulseStrength;
						phase = insensitive_settings_get_pulseShape(settings).imagp[j] + element->secondParameter;
						while (phase > 360) {
							phase -= 360;
						}
                        insensitive_spinsystem_gradient_offresonancepulse(spinsystem,
                                                                          strength * 720,
                                                                          stepDuration,
                                                                          phase,
                                                                          element->pulseFrequency,
                                                                          pulseArray);
					}
				}
			}
			break;
		case SequenceTypeShift:
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
            insensitive_spinsystem_gradient_chemicalshift(spinsystem, element->time);
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeCoupling:
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			if (rest != 0) {
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings));
					insensitive_spinsystem_gradient_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings));
				} else
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, rest, insensitive_settings_get_strongCoupling(settings));
			}
			for (s = 0; s < steps; s++) {
                if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings));
					insensitive_spinsystem_gradient_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings));
				} else
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime, insensitive_settings_get_strongCoupling(settings));
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeRelaxation:
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
				if (rest != 0) {
					if (element->spinlock)
                        insensitive_spinsystem_gradient_transversedipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
					else
						insensitive_spinsystem_gradient_dipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
				}
				for (s = 0; s < steps; s++) {
					if (element->spinlock)
                        insensitive_spinsystem_gradient_transversedipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
					else
						insensitive_spinsystem_gradient_dipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
				}
			} else {
				if (rest != 0)
                    insensitive_spinsystem_gradient_simplerelaxation(spinsystem, rest, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings));

				for (s = 0; s < steps; s++)
                    insensitive_spinsystem_gradient_simplerelaxation(spinsystem, dwellTime, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings));
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeEvolution:
			totalEvolutionTime += element->time;
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
			rest = fmod(element->time, dwellTime);
			steps = (element->time - rest) / dwellTime;
			// Perform step of rest
			if (rest != 0) {
				insensitive_spinsystem_gradient_chemicalshift(spinsystem, rest);
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_gradient_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings));
					insensitive_spinsystem_gradient_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, rest / 2, insensitive_settings_get_strongCoupling(settings));
				} else
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, rest, insensitive_settings_get_strongCoupling(settings));
				if (insensitive_settings_get_relaxationWithEvolution(settings)) {
					if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
						if (element->spinlock)
                            insensitive_spinsystem_gradient_transversedipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
						else
                            insensitive_spinsystem_gradient_dipolarrelaxation(spinsystem, rest, insensitive_settings_get_correlationTime(settings));
					} else
                        insensitive_spinsystem_gradient_simplerelaxation(spinsystem, rest, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings));
				}
			}
			// Perform in steps of dwell time
			for (s = 0; s < steps; s++) {
				insensitive_spinsystem_gradient_chemicalshift(spinsystem, dwellTime);
				if ((element->iDecoupling || element->sDecoupling) && !element->spinlock) {
					insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings));
					insensitive_spinsystem_gradient_perform_decoupling(spinsystem, element->iDecoupling, element->sDecoupling, element->secondParameter);
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime / 2, insensitive_settings_get_strongCoupling(settings));
				} else
                    insensitive_spinsystem_gradient_jcoupling(spinsystem, dwellTime, insensitive_settings_get_strongCoupling(settings));
				if (insensitive_settings_get_relaxationWithEvolution(settings)) {
					if (insensitive_settings_get_dipolarRelaxation(settings) && (spins > 1)) {
						if (element->spinlock)
                            insensitive_spinsystem_gradient_transversedipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
						else
                            insensitive_spinsystem_gradient_dipolarrelaxation(spinsystem, dwellTime, insensitive_settings_get_correlationTime(settings));
					} else
                        insensitive_spinsystem_gradient_simplerelaxation(spinsystem, dwellTime, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings));
				}
			}
			if (element->spinlock)
				insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
			break;
		case SequenceTypeGradient:
			break;
		case SequenceTypeFID:
			break;
		}
	}
    insensitive_settings_set_pulseEnvelope(settings, savedPulseEnvelope);

	return totalEvolutionTime;
}
