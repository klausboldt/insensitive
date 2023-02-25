/* insensitive-controller.h
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

#ifndef __INSENSITIVE_CONTROLLER_H__
#define __INSENSITIVE_CONTROLLER_H__

#pragma once

#include <glib-object.h>

#include "insensitive-library.h"
#include "insensitive-spinsystem.h"
#include "insensitive-settings.h"
#include "insensitive-pulsesequence.h"
#include "insensitive-pulseshaper.h"
#include "insensitive-composer.h"


struct _InsensitiveController {
	GObject parent_instance;

	InsensitiveSpinSystem *spinSystem;
	InsensitiveSettings *settings;
	gpointer displayController;

	guint stepwiseOperationTimerNr, animationTimerNr;
	unsigned int stepsPerformedForOperation, stepsToBePerformedForOperation, dwellTimeFraction, danteCounter;
	gboolean operationIsInProgress, animationIsInProgress, acquisitionIsInProgress, interruptAcquisition;
	gboolean haltAnimation, relaxationWasIncludedBefore, iSpinsWereDecoupledBefore, sSpinsWereDecoupledBefore;
	gboolean interruptCoherencePathwayCalculation;
    InsensitivePulseShaper *pulseShaperController;
    InsensitiveComposer *matrixComposerController;

	unsigned int selectedSpin, expno, selectivePrecessionArray;

	DSPSplitComplex fid, spectrum1D, spectrum2D, spectrumSymmetrized;
	DSPSplitComplex fidStates, spectrum1DStates;
	DSPSplitComplex noiseTime, noiseFrequency, noiseAbs, pulseShape, pulsePowerSpectrum;
	unsigned int totalDataPointsInSER, recordedDataPointsInFID, firstDataPointInFID;
	fftw_plan fftsetup;
	enum WindowFunctionType windowFunction;
	float gaussianWidth, gaussianShift;
	float *apodizationT2, *apodizationT1;
	gboolean realDataSetsForStatesMethod, spectrumDataAvailable;
	enum PurePhaseDetectionMethod detectionMethodOfCurrentSpectrum;
    float acquisitionProgress;

	gboolean isRecordingPulseSequence, acquisitionAfterPulseSequence;
	gboolean currentSpectrumIsTwoDimensional;
	unsigned int phaseCycles;
	InsensitivePulseSequence *pulseSequence;
	GPtrArray *phaseCyclingArray, *pulseList;
    GtkListStore *phaseCyclingTable;
	int indexForVariableEvolutionTime;
	gchar *pulseSequenceName;
    GString *spectrumReport;
	double acquisitionTime;
	FittedSpectrum *dosyParameters;
	unsigned int fittedPeaks;
	unsigned int currentStepInPulseSequence, previousStepInPulseSequence;

	gchar **selectableDelayNames;
	float *selectableDelayValues;
	GPtrArray *grapefruitPath;

	unsigned int previousPhaseCycles, previousPulseArray;
	InsensitiveSpinSystem *previousSpinSystem;
	InsensitivePulseSequence *previousPulseSequence;
	GPtrArray *previousPhaseCyclingArray, *previousPulseList;
};


G_BEGIN_DECLS

#define INSENSITIVE_TYPE_CONTROLLER (insensitive_controller_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveController, insensitive_controller, INSENSITIVE, CONTROLLER, GObject)

/* Initialisation */
InsensitiveController *insensitive_controller_new(void);
void insensitive_controller_setup(InsensitiveController *self, InsensitiveSpinSystem *aSpinSystem, InsensitiveSettings *aSetting, gpointer aDisplayController);
void insensitive_controller_connect_pulseShaperController(InsensitiveController *self, InsensitivePulseShaper *pulseShaper);
void insensitive_controller_load_and_display_settings(InsensitiveController *self);

/* Spin System State Requests */
gchar *insensitive_controller_get_productOperatorString(InsensitiveController *self);
VectorCoordinates *insensitive_controller_get_vectorCoordinates(InsensitiveController *self);
void insensitive_controller_initialise_grapefruit_path(InsensitiveController *self);
void insensitive_controller_add_to_grapefruit_path(InsensitiveController *self);
GPtrArray *insensitive_controller_get_grapefruit_path(InsensitiveController *self);
gboolean insensitive_controller_no_larmorFrequency_set(InsensitiveController *self);
gboolean insensitive_controller_no_coupling_set(InsensitiveController *self);
gboolean insensitive_controller_allow_separate_shift_and_coupling(InsensitiveController *self);

/* Spin System Changes */
unsigned int insensitive_controller_get_selected_spin(InsensitiveController *self);
void insensitive_controller_set_selected_spin(InsensitiveController *self, unsigned int value);
void insensitive_controller_set_gyromagnetic_ratios(InsensitiveController *self, unsigned int codeForI, unsigned int codeForS);
void insensitive_controller_calculate_gyromagnetic_ratios(InsensitiveController *self);
void insensitive_controller_reset_couplingMatrix(InsensitiveController *self);
void insensitive_controller_set_spinType_of_spin(InsensitiveController *self, unsigned int spin, int type);
void insensitive_controller_set_larmorFrequency_for_spin(InsensitiveController *self, unsigned int spin, float value);
void insensitive_controller_set_jCouplingConstant_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float J);
void insensitive_controller_set_dipolarCouplingConstant_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float b);
void insensitive_controller_set_distance_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float r);
void insensitive_controller_spin_number_changed(InsensitiveController *self, unsigned int newNumberOfSpins);
float insensitive_controller_get_unitConversion(InsensitiveController *self);
void insensitive_controller_update_matrix_with(InsensitiveController *self, int numberOfOperators, int *base4Index, float *coefficient);
void insensitive_controller_rotate_spinsystem(InsensitiveController *self, gboolean backwards);
void insensitive_controller_redraw_current_spinsystem_state(InsensitiveController *self);

/* Energy Level */
void insensitive_controller_calculate_energy_levels(InsensitiveController *self);

/* NMR Operations */
void insensitive_controller_save_previous_state(InsensitiveController *self);
void insensitive_controller_save_previous_phaseCyclingTable(InsensitiveController *self);
void insensitive_controller_restore_previous_state(InsensitiveController *self);
void insensitive_controller_return_to_thermal_equilibrium(InsensitiveController *self);
void insensitive_controller_perform_pulse(InsensitiveController *self, float angle, float phase, gboolean animated);
void insensitive_controller_perform_pulse_animated(InsensitiveController *self, gboolean animated);
gboolean pulseTimerEvent(gpointer user_data);
gboolean pulseWithOffResonanceEffectsTimerEvent(gpointer user_data);
void insensitive_controller_perform_chemicalShift_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_chemicalShift_on_ISpins_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_chemicalShift_on_SSpins_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_chemicalShift_on_spinArray(InsensitiveController *self, int array, gboolean animated);
void insensitive_controller_perform_chemicalShift_with_array_animated(InsensitiveController *self, gboolean animated);
gboolean chemicalShiftTimerEvent(gpointer user_data);
void insensitive_controller_perform_coupling_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_coupling_on_ISpins_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_coupling_on_SSpins_animated(InsensitiveController *self, gboolean animated);
void insensitive_controller_perform_coupling_on_spinArray(InsensitiveController *self, int array, gboolean animated);
void insensitive_controller_perform_coupling_with_array_animated(InsensitiveController *self, gboolean animated);
gboolean couplingTimerEvent(gpointer user_data);
void insensitive_controller_perform_relaxation_animated(InsensitiveController *self, gboolean animated);
gboolean relaxationTimerEvent(gpointer user_data);
void insensitive_controller_perform_freeEvolution_animated(InsensitiveController *self, gboolean animated);
gboolean freeEvolutionTimerEvent(gpointer user_data);
void insensitive_controller_perform_gradient(InsensitiveController *self);
void gradientEvent(gpointer user_data);
gboolean animationTimerEvent(gpointer user_data);

/* Pulse Sequence */
gboolean insensitive_controller_get_isRecordingPulseSequence(InsensitiveController *self);
void insensitive_controller_set_isRecordingPulseSequence(InsensitiveController *self, gboolean value);
unsigned int insensitive_controller_get_numberOfPulseSequenceElements(InsensitiveController *self);
unsigned int insensitive_controller_get_numberOfPhaseCycles(InsensitiveController *self);
void insensitive_controller_add_columns_to_phaseCyclingTable(InsensitiveController *self, unsigned int added_columns);
void insensitive_controller_add_number_of_phase_cycles(InsensitiveController *self, int number);
InsensitivePulseSequence *insensitive_controller_get_pulseSequence(InsensitiveController *self);
void insensitive_controller_substitute_pulseSequence(InsensitiveController *self, GPtrArray *array);
GPtrArray *insensitive_controller_get_phaseCyclingArray(InsensitiveController *self);
GPtrArray *insensitive_controller_get_pulseList(InsensitiveController *self);
void insensitive_controller_substitute_phaseCyclingArray(InsensitiveController *self, GPtrArray *array, unsigned int numberOfCycles);
void insensitive_controller_toggle_recordingPulseSequence(InsensitiveController *self);
int insensitive_controller_get_variableEvolutionTime(InsensitiveController *self);
void insensitive_controller_set_variableEvolutionTime(InsensitiveController *self, int value);
gboolean insensitive_controller_get_pulseSequence_ends_with_acquisition(InsensitiveController *self);
void insensitive_controller_erase_pulseSequence(InsensitiveController *self);
void insensitive_controller_perform_pulseSequence(InsensitiveController *self);
gpointer insensitive_controller_perform_pulseSequence_in_background(gpointer data);
gboolean insensitive_controller_finish_perform_pulseSequence(gpointer data);
unsigned int insensitive_controller_get_currentStepInPulseSequence(InsensitiveController *self);
void insensitive_controller_set_currentStepInPulseSequence(InsensitiveController *self, int value);
void insensitive_controller_perform_next_step_of_pulseSequence(InsensitiveController *self);
GString *insensitive_controller_export_pulseSequence(InsensitiveController *self, gchar *name);
void insensitive_controller_perform_threaded_coherencePathway_calculation(InsensitiveController * self);
void insensitive_controller_interrupt_coherencePathway_calculation(InsensitiveController * self);
gpointer insensitive_controller_calculate_coherencePathway(gpointer data);
gchar *insensitive_controller_get_pulseSequence_name(InsensitiveController *self);
void insensitive_controller_set_name_for_pulseSequence(InsensitiveController *self, gchar *name);
GString *insensitive_controller_create_spectrumReport(InsensitiveController *self, gboolean takeDataFromPulseSequence);
GString *insensitive_controller_get_spectrumReport(InsensitiveController *self);
void insensitive_controller_set_spectrumReport(InsensitiveController *self, gchar *report);

/* Acquisition */
gboolean insensitive_controller_get_acquisitionIsInProgress(InsensitiveController *self);
void insensitive_controller_reset_acquisition_for_dataPoints(InsensitiveController *self, unsigned int number);
void insensitive_controller_perform_acquisition(InsensitiveController *self);
void insensitive_controller_perform_2D_acquisition(InsensitiveController *self);
gpointer insensitive_controller_perform_2D_acquisition_in_background(gpointer data);
void insensitive_controller_initiate_interface_for_2D_acquisition(gpointer data);
gboolean insensitive_controller_update_interface_during_2D_acquisition(gpointer data);
gboolean insensitive_controller_finish_perform_2D_pulseSequence(gpointer data);
void insensitive_controller_get_first_trace_of_2D_spectrum(InsensitiveController *self, gboolean state);
void insensitive_controller_acquire_dataPoint(InsensitiveController *self);
void insensitive_controller_interrupt_acquisition(InsensitiveController *self);
void insensitive_controller_stop_acquisition(InsensitiveController *self);
void insensitive_controller_inject_spectrum(InsensitiveController *self, float *real, float *imag,
                                            float *realStates, float *imagStates,
                                            unsigned int size, unsigned int stride, int domain);
void insensitive_controller_show_FID(InsensitiveController *self);
void insensitive_controller_show_SER(InsensitiveController *self);
void insensitive_controller_fourier_transform_1D_spectrum(InsensitiveController *self);
void insensitive_controller_absolute_value_1D_spectrum(InsensitiveController *self, gboolean complexData);
void insensitive_controller_perform_single_dimension_fourier_transform(InsensitiveController *self,
                                                                       DSPSplitComplex source,
                                                                       DSPSplitComplex destination,
                                                                       enum SpectrumDimension domain);
void insensitive_controller_swap_states_spectra(gboolean realDataSet, DSPSplitComplex *sinModulated, DSPSplitComplex *cosModulated);
void insensitive_controller_fourier_transform_2D_spectrum_along_T2(InsensitiveController *self);
void insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(InsensitiveController *self);
void insensitive_controller_absolute_value_spectrum(InsensitiveController *self);
void insensitive_controller_spectrum_symmetrization(InsensitiveController *self, enum Symmetrization symmetrize, unsigned int spectrumDomain);

enum WindowFunctionType insensitive_controller_get_windowFunction(InsensitiveController *self);
void insensitive_controller_set_windowFunction(InsensitiveController *self, enum WindowFunctionType type);
float insensitive_controller_get_gaussianWidth(InsensitiveController *self);
float insensitive_controller_get_gaussianShift(InsensitiveController *self);
void insensitive_controller_set_gaussianWidth(InsensitiveController *self, float value);
void insensitive_controller_set_gaussianShift(InsensitiveController *self, float value);
gboolean insensitive_controller_get_realDataSetsForStatesMethod(InsensitiveController *self);
void insensitive_controller_set_realDataSetsForStatesMethod(InsensitiveController *self, gboolean value);
gboolean insensitive_controller_tilt_2D_spectrum(InsensitiveController *self, unsigned int dataset, unsigned int coordinate);
int insensitive_controller_determine_peak_list(InsensitiveController *self, GPtrArray *peaks, int dataset);
int insensitive_controller_fit_lorentzians(InsensitiveController *self, int dataset);
void insensitive_controller_dosy_fit(InsensitiveController *self, int dataset);
void insensitive_controller_dosy_spectrum(InsensitiveController *self, int dataset);
gboolean insensitive_controller_get_spectrumDataAvailable(InsensitiveController *self);
DSPSplitComplex *insensitive_controller_get_rawFID(InsensitiveController *self);
DSPSplitComplex *insensitive_controller_get_rawFIDStates(InsensitiveController *self);
float insensitive_controller_get_acquisitionTime(InsensitiveController *self);
void insensitive_controller_calculate_first_derivative_of_1D_spectrum(InsensitiveController *self);

/* Pulse Shaper */
float insensitive_controller_get_pulseLength(InsensitiveController *self);
void insensitive_controller_set_pulseLength(InsensitiveController *self, float value);
float insensitive_controller_get_pulseFrequency(InsensitiveController *self);
void insensitive_controller_set_pulseFrequency(InsensitiveController *self, float value);
void insensitive_controller_set_pulseEnvelope(InsensitiveController *self, enum PulseEnvelope value);
void insensitive_controller_create_pulse_powerspectrum(InsensitiveController *self);
DSPSplitComplex insensitive_controller_get_pulseShape(InsensitiveController *self);
DSPSplitComplex insensitive_controller_get_pulsePowerSpectrum(InsensitiveController *self);

/* Settings Input and Output */
void insensitive_controller_set_flipAngle(InsensitiveController *self, float value);
void insensitive_controller_set_pulseDuration(InsensitiveController *self, float value);
void insensitive_controller_set_pulseStrength(InsensitiveController *self, float value);
void insensitive_controller_make_hard_pulse(InsensitiveController *self);
void insensitive_controller_make_selective_pulse(InsensitiveController *self);
void insensitive_controller_make_soft_pulse(InsensitiveController *self);
void insensitive_controller_make_softer_pulse(InsensitiveController *self);
void insensitive_controller_update_display_with_pulse_settings(InsensitiveController *self);
void insensitive_controller_set_phase(InsensitiveController *self, float value);
void insensitive_controller_change_pulseArray_for_spin(InsensitiveController *self, int spin, gboolean value);
void insensitive_controller_set_all_iSpins_active(InsensitiveController *self, gboolean value);
void insensitive_controller_set_all_sSpins_active(InsensitiveController *self, gboolean value);
void insensitive_controller_set_all_spins_active(InsensitiveController *self, gboolean value);
void insensitive_controller_set_strongCoupling(InsensitiveController *self, gboolean value);
void insensitive_controller_set_dipolarRelaxation(InsensitiveController *self, gboolean value);
void insensitive_controller_set_animates(InsensitiveController *self, gboolean value);
void insensitive_controller_halt_animation(InsensitiveController *self, gboolean value);
void insensitive_controller_set_relaxation_with_evolution(InsensitiveController *self, gboolean value);
void insensitive_controller_restore_relaxation_with_evolution(InsensitiveController *self);
void insensitive_controller_set_T1(InsensitiveController *self, float value);
void insensitive_controller_set_T2(InsensitiveController *self, float value);
void insensitive_controller_set_correlationTime(InsensitiveController *self, float value);
void insensitive_controller_set_delay(InsensitiveController *self, float value);
void insensitive_controller_set_delay_from_menuEntry(InsensitiveController *self, unsigned int tag);
void insensitive_controller_calculate_selectable_delays(InsensitiveController *self);
void insensitive_controller_set_dephasingJitter(InsensitiveController *self, gboolean value);
void insensitive_controller_set_IDecoupling(InsensitiveController *self, gboolean value);
void insensitive_controller_set_SDecoupling(InsensitiveController *self, gboolean value);
void insensitive_controller_save_decoupling(InsensitiveController *self);
void insensitive_controller_restore_decoupling(InsensitiveController *self);
void insensitive_controller_set_spinlock(InsensitiveController *self, gboolean value);
void insensitive_controller_set_gradientStrength(InsensitiveController *self, float value);
void insensitive_controller_set_gradientDuration(InsensitiveController *self, float value);
void insensitive_controller_set_diffusion(InsensitiveController *self, gboolean value);
void insensitive_controller_set_dataPoints(InsensitiveController *self, unsigned int value);
void insensitive_controller_set_dwellTime(InsensitiveController *self, float value);
void insensitive_controller_set_noiseLevel(InsensitiveController *self, float value);
void insensitive_controller_set_showRealPart(InsensitiveController *self, gboolean value);
void insensitive_controller_set_showImaginaryPart(InsensitiveController *self, gboolean value);
void insensitive_controller_set_showIntegral(InsensitiveController *self, gboolean value);
void insensitive_controller_set_showWindowFunction(InsensitiveController *self, gboolean value);
void insensitive_controller_set_zeroFilling(InsensitiveController *self, gboolean value);
void insensitive_controller_set_acquisitionAfterNextPulse(InsensitiveController *self, gboolean value);
void insensitive_controller_set_pulseBeforeAcquisition(InsensitiveController *self, gboolean value);
void insensitive_controller_set_detectISignal(InsensitiveController *self, gboolean value);
void insensitive_controller_set_detectSSignal(InsensitiveController *self, gboolean value);
void insensitive_controller_set_vectorDisplayType(InsensitiveController *self, enum VectorDisplayType value);
void insensitive_controller_set_operatorBasis(InsensitiveController *self, enum OperatorBasis value);
void insensitive_controller_set_color1stOrderCoherences(InsensitiveController *self, gboolean value);
void insensitive_controller_set_matrixDisplayType(InsensitiveController *self, enum MatrixDisplayType value);
void insensitive_controller_set_vectorDiagramType(InsensitiveController *self, enum VectorDiagramType value);
void insensitive_controller_set_vectorDiagramType(InsensitiveController *self, enum VectorDiagramType value);
void insensitive_controller_set_chemicalShiftUnitsToDegreesPerSecond(InsensitiveController *self, gboolean value);
void insensitive_controller_set_allowShiftAndCouplingButtons(InsensitiveController *self, gboolean value);
void insensitive_controller_set_playSound(InsensitiveController *self, gboolean value);
void insensitive_controller_set_detectionMethod(InsensitiveController *self, enum PurePhaseDetectionMethod value);
enum PurePhaseDetectionMethod insensitive_controller_get_detectionMethod(InsensitiveController *self);

G_END_DECLS

#endif /* __INSENSITIVE_CONTROLLER_H__ */
