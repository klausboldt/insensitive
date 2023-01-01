/* insensitive-settings.h
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

#ifndef __INSENSITIVE_SETTINGS_H__
#define __INSENSITIVE_SETTINGS_H__

#pragma once

#include <glib-object.h>

#include "insensitive.h"
#include "insensitive-library.h"


struct _InsensitiveSettings
{
	GObject parent_instance;

	// Pulse settings
	float flipAngle, phase, pulseDuration, pulseStrength;
	unsigned int pulseArray;
	gboolean allISpinsSelected, allSSpinsSelected, allSpinsSelected;
	gboolean someISpinsSelected, someSSpinsSelected;
	float pulseLength, pulseFrequency;
	float *pulsePowerSpectrum;
	enum PulseEnvelope pulseEnvelope;
	DSPSplitComplex pulseShape;
	enum ExcitationProfile excitationProfile;

	// Free evolution settings
	gboolean strongCoupling;
	gboolean dipolarRelaxation;
	gboolean animates;
	gboolean relaxationWithEvolution;
	float T1, T2, correlationTime;
	float delay;
	gboolean dephasingJitter;
	gboolean iDecoupling, sDecoupling, spinlock;

	// Gradient settings;
	float gradientStrength;
	float gradientDuration;
	gboolean diffusion;

	// Spectrum settings
	int dataPoints;
	float dwellTime;
	float noiseLevel;
	gboolean zeroFilling;
	gboolean showRealPart, showImaginaryPart, showIntegral, showWindowFunction;
	gboolean pulseBeforeAcquisition, acquisitionAfterNextPulse;
	gboolean detectISpins, detectSSpins;
	enum PurePhaseDetectionMethod detectionMethod;

	// Display settings
	enum VectorDisplayType vectorDisplayType;
	enum OperatorBasis operatorBasis;
	gboolean color1stOrderCoherences;
	enum MatrixDisplayType matrixDisplayType;

    // Spectrum settings
    gboolean showGrid;

	// More options
	gboolean allowShiftAndCouplingButtons;
	gboolean playSoundAfterAcquisition;
	enum VectorDiagramType vectorDiagramType;
	gboolean larmorFrequencyInDegreesPerSeconds;
	gboolean showMatrix;
	enum ExportFormat exportFormat;
	gboolean ignoreOffResonanceEffectsForPulses;
	float gyroI, gyroS;
	float signalToNoiseThreshold;
	unsigned int gyroCodeI, gyroCodeS;
    float maxCoherenceCalculations;
    float spectrumLineWidth;
    gchar *matrixFont;

	gboolean saveSettings;

    GKeyFile *defaultSettings;
};


G_BEGIN_DECLS

#define INSENSITIVE_TYPE_SETTINGS (insensitive_settings_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveSettings, insensitive_settings, INSENSITIVE, SETTINGS, GObject)

InsensitiveSettings *insensitive_settings_new(void);

void insensitive_settings_save_defaults(InsensitiveSettings *self);
void insensitive_settings_load_defaults(InsensitiveSettings *self);
gchar* insensitive_settings_defaults_as_string(InsensitiveSettings *self);
void insensitive_settings_save_spinsystem(InsensitiveSettings *self, gpointer spinsystem);
void insensitive_settings_load_spinsystem(InsensitiveSettings *self, gpointer spinsystem);
void insensitive_settings_save_pulsesequence(InsensitiveSettings *self, gpointer source);
void insensitive_settings_load_pulsesequence(InsensitiveSettings *self, gpointer source);

/* Pulse Settings */
float insensitive_settings_get_flipAngle(InsensitiveSettings *self);
void insensitive_settings_set_flipAngle(InsensitiveSettings *self, float value);
float insensitive_settings_get_pulseDuration(InsensitiveSettings *self);
void insensitive_settings_set_pulseDuration(InsensitiveSettings *self, float value);
float insensitive_settings_get_pulseStrength(InsensitiveSettings *self);
void insensitive_settings_set_pulseStrength(InsensitiveSettings *self, float value);
float insensitive_settings_get_phase(InsensitiveSettings *self);
void insensitive_settings_set_phase(InsensitiveSettings *self, float value);
unsigned int insensitive_settings_get_pulseArray(InsensitiveSettings *self);
gboolean insensitive_settings_get_allISpinsSelected(InsensitiveSettings *self);
gboolean insensitive_settings_get_allSSpinsSelected(InsensitiveSettings *self);
gboolean insensitive_settings_get_someISpinsSelected(InsensitiveSettings *self);
gboolean insensitive_settings_get_someSSpinsSelected(InsensitiveSettings *self);
gboolean insensitive_settings_get_allSpinsSelected(InsensitiveSettings *self);
void insensitive_settings_set_pulseArray(InsensitiveSettings *self, unsigned int value, unsigned int number_of_spins, unsigned int spinTypeArray);
void insensitive_settings_set_pulseArray_for_spinnumber(InsensitiveSettings *self, unsigned int number, gboolean value, unsigned int number_of_spins, unsigned int spinTypeArray);
void insensitive_settings_calculate_selected_spins(InsensitiveSettings *self, unsigned int number_of_spins, unsigned int spinTypeArray);
float insensitive_settings_get_pulseLength(InsensitiveSettings *self);
void insensitive_settings_set_pulseLength(InsensitiveSettings *self, float value);
float insensitive_settings_get_pulseFrequency(InsensitiveSettings *self);
void insensitive_settings_set_pulseFrequency(InsensitiveSettings *self, float value);
enum PulseEnvelope insensitive_settings_get_pulseEnvelope(InsensitiveSettings *self);
void insensitive_settings_set_pulseEnvelope(InsensitiveSettings *self, enum PulseEnvelope value);
DSPSplitComplex insensitive_settings_get_pulseShape(InsensitiveSettings *self);
void insensitive_settings_create_pulseShape(InsensitiveSettings *self);
float *insensitive_settings_get_pulsePowerSpectrum(InsensitiveSettings *self);
void insensitive_settings_set_pulsePowerSpectrum(InsensitiveSettings *self, float *powerSpectrum);
enum ExcitationProfile insensitive_settings_get_excitationProfile(InsensitiveSettings *self);
void insensitive_settings_set_excitationProfile(InsensitiveSettings *self, enum ExcitationProfile value);

/* Free Evolution Settings */
gboolean insensitive_settings_get_strongCoupling(InsensitiveSettings *self);
void insensitive_settings_set_strongCoupling(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_dipolarRelaxation(InsensitiveSettings *self);
void insensitive_settings_set_dipolarRelaxation(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_animates(InsensitiveSettings *self);
void insensitive_settings_set_animates(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_relaxationWithEvolution(InsensitiveSettings *self);
void insensitive_settings_set_relaxationWithEvolution(InsensitiveSettings *self, gboolean value);
float insensitive_settings_get_T1(InsensitiveSettings *self);
void insensitive_settings_set_T1(InsensitiveSettings *self, float value);
float insensitive_settings_get_T2(InsensitiveSettings *self);
void insensitive_settings_set_T2(InsensitiveSettings *self, float value);
float insensitive_settings_get_correlationTime(InsensitiveSettings *self);
void insensitive_settings_set_correlationTime(InsensitiveSettings *self, float value);
float insensitive_settings_get_delay(InsensitiveSettings *self);
void insensitive_settings_set_delay(InsensitiveSettings *self, float value);
void insensitive_settings_set_delay(InsensitiveSettings *self, float value);
gboolean insensitive_settings_get_dephasingJitter(InsensitiveSettings *self);
void insensitive_settings_set_dephasingJitter(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_iDecoupling(InsensitiveSettings *self);
void insensitive_settings_set_iDecoupling(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_sDecoupling(InsensitiveSettings *self);
void insensitive_settings_set_sDecoupling(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_spinlock(InsensitiveSettings *self);
void insensitive_settings_set_spinlock(InsensitiveSettings *self, gboolean value);

/* Gradient Settings */
float insensitive_settings_get_gradientStrength(InsensitiveSettings *self);
void insensitive_settings_set_gradientStrength(InsensitiveSettings *self, float value);
float insensitive_settings_get_gradientDuration(InsensitiveSettings *self);
void insensitive_settings_set_gradientDuration(InsensitiveSettings *self, float value);
gboolean insensitive_settings_get_diffusion(InsensitiveSettings *self);
void insensitive_settings_set_diffusion(InsensitiveSettings *self, gboolean value);

/* Specrum Settings */
unsigned int insensitive_settings_get_dataPoints(InsensitiveSettings *self);
void insensitive_settings_set_dataPoints(InsensitiveSettings *self, unsigned int value);
void insensitive_settings_set_logDataPoints(InsensitiveSettings *self, unsigned int value);
float insensitive_settings_get_dwellTime(InsensitiveSettings *self);
void insensitive_settings_set_dwellTime(InsensitiveSettings *self, float value);
float insensitive_settings_get_noiseLevel(InsensitiveSettings *self);
void insensitive_settings_set_noiseLevel(InsensitiveSettings *self, float value);
gboolean insensitive_settings_get_zeroFilling(InsensitiveSettings *self);
void insensitive_settings_set_zeroFilling(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_showRealPart(InsensitiveSettings *self);
void insensitive_settings_set_showRealPart(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_showImaginaryPart(InsensitiveSettings *self);
void insensitive_settings_set_showImaginaryPart(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_showIntegral(InsensitiveSettings *self);
void insensitive_settings_set_showIntegral(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_showWindowFunction(InsensitiveSettings *self);
void insensitive_settings_set_showWindowFunction(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_pulseBeforeAcquisition(InsensitiveSettings *self);
void insensitive_settings_set_pulseBeforeAcquisition(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_acquisitionAfterNextPulse(InsensitiveSettings *self);
void insensitive_settings_set_acquisitionAfterNextPulse(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_detectISpins(InsensitiveSettings *self);
void insensitive_settings_set_detectISpins(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_detectSSpins(InsensitiveSettings *self);
void insensitive_settings_set_detectSSpins(InsensitiveSettings *self, gboolean value);

/* Display Settings */
enum VectorDisplayType insensitive_settings_get_vectorDisplayType(InsensitiveSettings *self);
void insensitive_settings_set_vectorDisplayType(InsensitiveSettings *self, enum VectorDisplayType value);
enum OperatorBasis insensitive_settings_get_operatorBasis(InsensitiveSettings *self);
void insensitive_settings_set_operatorBasis(InsensitiveSettings *self, enum OperatorBasis value);
gboolean insensitive_settings_get_color1stOrderCoherences(InsensitiveSettings *self);
void insensitive_settings_set_color1stOrderCoherences(InsensitiveSettings *self, gboolean value);
enum MatrixDisplayType insensitive_settings_get_matrixDisplayType(InsensitiveSettings *self);
void insensitive_settings_set_matrixDisplayType(InsensitiveSettings *self, enum MatrixDisplayType value);

/* Spectrum settings */
gboolean insensitive_settings_get_showGrid(InsensitiveSettings *self);
void insensitive_settings_set_showGrid(InsensitiveSettings *self, gboolean value);

/* Other Options */
gboolean insensitive_settings_get_allowShiftAndCouplingButtons(InsensitiveSettings *self);
void insensitive_settings_set_allowShiftAndCouplingButtons(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_playSoundAfterAcquisition(InsensitiveSettings *self);
void insensitive_settings_set_playSoundAfterAcquisition(InsensitiveSettings *self, gboolean value);
enum VectorDiagramType insensitive_settings_get_vectorDiagramType(InsensitiveSettings *self);
void insensitive_settings_set_vectorDiagramType(InsensitiveSettings *self, enum VectorDiagramType value);
gboolean insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(InsensitiveSettings *self);
void insensitive_settings_set_larmorFrequencyInDegreesPerSeconds(InsensitiveSettings *self, gboolean value);
gboolean insensitive_settings_get_showMatrix(InsensitiveSettings *self);
void insensitive_settings_set_showMatrix(InsensitiveSettings *self, gboolean value);
gchar *insensitive_settings_get_matrixFont(InsensitiveSettings *self);
void insensitive_settings_set_matrixFont(InsensitiveSettings *self, const gchar *fontname);
enum PurePhaseDetectionMethod insensitive_settings_get_detectionMethod(InsensitiveSettings *self);
void insensitive_settings_set_detectionMethod(InsensitiveSettings *self, enum PurePhaseDetectionMethod value);
enum ExportFormat insensitive_settings_get_exportFormat(InsensitiveSettings *self);
void insensitive_settings_set_exportFormat(InsensitiveSettings *self, enum ExportFormat value);
gboolean insensitive_settings_get_ignoreOffResonanceEffectsForPulses(InsensitiveSettings *self);
void insensitive_settings_set_ignoreOffResonanceEffectsForPulses(InsensitiveSettings *self, gboolean value);
unsigned int insensitive_settings_get_gyroCodeI(InsensitiveSettings *self);
void insensitive_settings_set_gyroCodeI(InsensitiveSettings *self, unsigned int value);
unsigned int insensitive_settings_get_gyroCodeS(InsensitiveSettings *self);
void insensitive_settings_set_gyroCodeS(InsensitiveSettings *self, unsigned int value);
float insensitive_settings_get_signalToNoiseThreshold(InsensitiveSettings *self);
void insensitive_settings_set_signalToNoiseThreshold(InsensitiveSettings *self, float value);
float insensitive_settings_get_maxCoherenceCalculations(InsensitiveSettings *self);
void insensitive_settings_set_maxCoherenceCalculations(InsensitiveSettings *self, float value);
float insensitive_settings_get_spectrumLineWidth(InsensitiveSettings *self);
void insensitive_settings_set_spectrumLineWidth(InsensitiveSettings *self, float value);

G_END_DECLS

#endif /* __INSENSITIVE_SETTINGS_H__ */
