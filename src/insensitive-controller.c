/* insensitive-controller.c
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

#include "insensitive.h"
#include "insensitive-config.h"
#include "insensitive-controller.h"
#include "insensitive-window.h"
#include "insensitive-pulseshaper.h"


G_DEFINE_TYPE(InsensitiveController, insensitive_controller, G_TYPE_OBJECT)


InsensitiveController* insensitive_controller_new()
{
	return (InsensitiveController *)g_object_new(INSENSITIVE_TYPE_CONTROLLER, NULL);
}


static void insensitive_controller_dispose(GObject *gobject)
{
	InsensitiveController *self = (InsensitiveController *)gobject;

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

    self->haltAnimation = TRUE;
    self->interruptAcquisition = TRUE;
    self->interruptCoherencePathwayCalculation = TRUE;

	g_object_unref(self->spinSystem);
	g_object_unref(self->settings);
	g_object_unref(self->pulseSequence);

	/* Always chain up to the parent class; there is no need to check if
	 * the parent class implements the dispose() virtual function: it is
	 * always guaranteed to do so
	 */
	G_OBJECT_CLASS(insensitive_controller_parent_class)->dispose(gobject);
}


static void insensitive_controller_finalize(GObject *gobject)
{
	InsensitiveController *self = (InsensitiveController *)gobject;

	g_free(self->spinSystem);
	g_free(self->settings);
	g_free(self->pulseSequence);

	g_free(self->selectableDelayNames);
	g_free(self->selectableDelayValues);
	g_ptr_array_free(self->grapefruitPath, TRUE);
    g_ptr_array_free(self->pulseList, TRUE);
    g_ptr_array_free(self->previousPulseList, TRUE);
    g_ptr_array_free(self->phaseCyclingArray, TRUE);
    g_ptr_array_free(self->previousPhaseCyclingArray, TRUE);

	if (self->fid.realp != NULL)
		g_free(self->fid.realp);
	if (self->fid.imagp != NULL)
		g_free(self->fid.imagp);
	if (self->spectrum1D.realp != NULL)
		g_free(self->spectrum1D.realp);
	if (self->spectrum1D.imagp != NULL)
		g_free(self->spectrum1D.imagp);
	if (self->spectrum2D.realp != NULL)
		g_free(self->spectrum2D.realp);
	if (self->spectrum2D.imagp != NULL)
		g_free(self->spectrum2D.imagp);
	if (self->spectrumSymmetrized.realp != NULL)
		g_free(self->spectrumSymmetrized.realp);
	if (self->spectrumSymmetrized.imagp != NULL)
		g_free(self->spectrumSymmetrized.imagp);
	if (self->apodizationT2 != NULL)
		g_free(self->apodizationT2);
	if (self->apodizationT1 != NULL)
		g_free(self->apodizationT1);
	if (self->noiseTime.realp != NULL)
		g_free(self->noiseTime.realp);
	if (self->noiseTime.imagp != NULL)
		g_free(self->noiseTime.imagp);
	if (self->noiseFrequency.realp != NULL)
		g_free(self->noiseFrequency.realp);
	if (self->noiseFrequency.imagp != NULL)
		g_free(self->noiseFrequency.imagp);
	if (self->noiseAbs.realp != NULL)
		g_free(self->noiseAbs.realp);
	if (self->noiseAbs.imagp != NULL)
		g_free(self->noiseAbs.imagp);
	if (self->dosyParameters != NULL)
		g_free(self->dosyParameters);
	fftw_destroy_plan(self->fftsetup);
	if (self->pulseSequenceName != NULL)
		g_free(self->pulseSequenceName);
	if (self->spectrumReport != NULL)
        g_string_free(self->spectrumReport, TRUE);

	G_OBJECT_CLASS(insensitive_controller_parent_class)->finalize(gobject);
}


static void insensitive_controller_class_init(InsensitiveControllerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = insensitive_controller_dispose;
	object_class->finalize = insensitive_controller_finalize;
}


static void insensitive_controller_init(InsensitiveController *self)
{
    gchar *firstPhaseCycleString;

	self->spinSystem = NULL;
	self->settings = NULL;

	self->previousSpinSystem = NULL;

	self->operationIsInProgress = FALSE;
	self->animationIsInProgress = FALSE;
	self->acquisitionIsInProgress = FALSE;
	self->isRecordingPulseSequence = FALSE;
	self->haltAnimation = FALSE;

	self->fid.realp = NULL;
	self->fid.imagp = NULL;
	self->spectrum1D.realp = NULL;
	self->spectrum1D.imagp = NULL;
	self->spectrum2D.realp = NULL;
	self->spectrum2D.imagp = NULL;
	self->spectrumSymmetrized.realp = NULL;
	self->spectrumSymmetrized.imagp = NULL;
	self->fidStates.realp = NULL;
	self->fidStates.imagp = NULL;
	self->spectrum1DStates.realp = NULL;
	self->spectrum1DStates.imagp = NULL;
    self->realDataSetsForStatesMethod = TRUE;
	self->spectrumDataAvailable = FALSE;
	self->totalDataPointsInSER = 0;
	self->recordedDataPointsInFID = 0;
	self->firstDataPointInFID = 0;
	self->gaussianWidth = 10.0;
	self->gaussianShift = 0.0;
	self->apodizationT2 = NULL;
	self->apodizationT1 = NULL;
	self->dosyParameters = NULL;
	self->windowFunction = WFNone;

	self->selectableDelayNames = calloc(4 * 6, sizeof(gchar *));
	self->selectableDelayValues = malloc(4 * 6 * sizeof(float));

	self->isRecordingPulseSequence = FALSE;
	self->phaseCycles = 1;
	self->pulseSequence = insensitive_pulsesequence_new();
	self->pulseList = g_ptr_array_new();

    firstPhaseCycleString = malloc(5 * sizeof(gchar));
    sprintf(firstPhaseCycleString, "0");
    self->phaseCyclingArray = g_ptr_array_new_with_free_func(g_free); // g_ptr_array_sized_new(self->phaseCycles);
    g_ptr_array_add(self->phaseCyclingArray, firstPhaseCycleString);

	self->indexForVariableEvolutionTime = 0;
	self->pulseSequenceName = NULL;
	self->spectrumReport = NULL;
	self->interruptCoherencePathwayCalculation = TRUE;
	self->detectionMethodOfCurrentSpectrum = None;
	self->acquisitionAfterPulseSequence = FALSE;

	self->pulseShape.realp = malloc(pulsePowerSpectrumResolution * sizeof(float));
	self->pulseShape.imagp = malloc(pulsePowerSpectrumResolution * sizeof(float));
	self->pulsePowerSpectrum.realp = malloc(pulsePowerSpectrumResolution * sizeof(float));
	self->pulsePowerSpectrum.imagp = malloc(pulsePowerSpectrumResolution * sizeof(float));
    self->pulseShaperController = NULL;

	self->grapefruitPath = g_ptr_array_sized_new(1024);

	self->previousPulseList = g_ptr_array_new();
	self->previousPhaseCyclingArray = g_ptr_array_sized_new(self->phaseCycles);

	self->selectedSpin = 0;
	self->expno = 0;
}


void insensitive_controller_load_and_display_settings(InsensitiveController *self)
{
    InsensitiveWindow *displayController = (InsensitiveWindow *)self->displayController;

    set_pulseEnvelope(displayController, insensitive_settings_get_pulseEnvelope(self->settings));
    set_flipAngle(displayController, insensitive_settings_get_flipAngle(self->settings));
    set_phase(displayController, insensitive_settings_get_phase(self->settings));
    set_pulseFrequency(displayController, insensitive_settings_get_pulseFrequency(self->settings));
    set_pulseDuration(displayController, insensitive_settings_get_pulseDuration(self->settings));
    set_pulseStrength(displayController, insensitive_settings_get_pulseStrength(self->settings));
    insensitive_settings_calculate_selected_spins(self->settings,
                                                  insensitive_spinsystem_get_spins(self->spinSystem),
                                                  insensitive_spinsystem_get_spintypearray(self->spinSystem));
    set_iSpins_checkbox(displayController, insensitive_settings_get_allISpinsSelected(self->settings));
    set_sSpins_checkbox(displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
    set_allSpins_checkbox(displayController, insensitive_settings_get_allSpinsSelected(self->settings));
    set_spin_checkboxes(displayController, insensitive_settings_get_pulseArray(self->settings));
    set_strongCoupling_checkbox(displayController, insensitive_settings_get_strongCoupling(self->settings));
    set_dipolarRelaxation_checkbox(displayController, insensitive_settings_get_dipolarRelaxation(self->settings));
    set_animation_checkbox(displayController, insensitive_settings_get_animates(self->settings));
    set_include_relaxation_checkbox(displayController, insensitive_settings_get_relaxationWithEvolution(self->settings));
    set_T1(displayController, insensitive_settings_get_T1(self->settings));
    set_T2(displayController, insensitive_settings_get_T2(self->settings));
    set_correlationTime(displayController, insensitive_settings_get_correlationTime(self->settings));
    set_delay(displayController, insensitive_settings_get_delay(self->settings));
    insensitive_controller_calculate_selectable_delays(self);
    set_dephasingJitter_checkbox(displayController, insensitive_settings_get_dephasingJitter(self->settings));
    set_spinlock(displayController, insensitive_settings_get_spinlock(self->settings));
    if(insensitive_settings_get_spinlock(self->settings)) {
        insensitive_controller_set_spinlock(self, TRUE);
    } else {
        set_iDecoupling_checkbox(displayController, insensitive_settings_get_iDecoupling(self->settings));
        set_sDecoupling_checkbox(displayController, insensitive_settings_get_sDecoupling(self->settings));
    }
    set_gradient_strength(displayController, insensitive_settings_get_gradientStrength(self->settings));
    set_gradient_duration(displayController, insensitive_settings_get_gradientDuration(self->settings));
    set_diffusion(displayController, insensitive_settings_get_diffusion(self->settings));
    set_dataPoints(displayController, insensitive_settings_get_zeroFilling(self->settings) ? insensitive_settings_get_dataPoints(self->settings) / 2 : insensitive_settings_get_dataPoints(self->settings));
    set_dwellTime(displayController, insensitive_settings_get_dwellTime(self->settings));
    set_acquisitionAfterNextPulse(displayController, insensitive_settings_get_acquisitionAfterNextPulse(self->settings));
    set_detectSSignal(displayController, insensitive_settings_get_detectSSpins(self->settings));
    set_detectISignal(displayController, insensitive_settings_get_detectISpins(self->settings));
    set_zeroFilling(displayController, insensitive_settings_get_zeroFilling(self->settings));
    set_vectorDisplayType(displayController, insensitive_settings_get_vectorDisplayType(self->settings));
    set_operatorBasis(displayController, insensitive_settings_get_operatorBasis(self->settings));
    set_color1stOrderCoherences(displayController, insensitive_settings_get_color1stOrderCoherences(self->settings));
    set_matrixDisplayType(displayController, insensitive_settings_get_matrixDisplayType(self->settings));
    set_vectorDiagramType(displayController, insensitive_settings_get_vectorDiagramType(self->settings));

    spin_number_was_changed(displayController);
}


void insensitive_controller_setup(InsensitiveController *self, InsensitiveSpinSystem *aSpinSystem, InsensitiveSettings *aSetting, gpointer aDisplayController)
{
	self->spinSystem = aSpinSystem;
	self->settings = aSetting;
	self->displayController = aDisplayController;
	insensitive_controller_set_windowFunction(self, self->windowFunction);

    set_spin_number((InsensitiveWindow *)self->displayController, insensitive_spinsystem_get_spins(self->spinSystem));
    set_chemicalShift_units_to_degreesPerSecond((InsensitiveWindow *)self->displayController,
                                                insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(self->settings));
    set_gyromagneticRatio_comboboxes((InsensitiveWindow *)self->displayController,
                                     insensitive_settings_get_gyroCodeI(self->settings),
                                     insensitive_settings_get_gyroCodeS(self->settings));
    insensitive_controller_create_pulse_powerspectrum(self);
    set_current_step_in_pulseSequence((InsensitiveWindow *)self->displayController, 0);
    set_showRealSpectrum((InsensitiveWindow *)self->displayController,
                         insensitive_settings_get_showRealPart(self->settings));
    set_showImaginarySpectrum((InsensitiveWindow *)self->displayController,
                              insensitive_settings_get_showImaginaryPart(self->settings));
    set_showIntegral((InsensitiveWindow *)self->displayController,
                     insensitive_settings_get_showIntegral(self->settings));
    insensitive_controller_set_noiseLevel(self, insensitive_settings_get_noiseLevel(self->settings));

    insensitive_controller_load_and_display_settings(self);
    insensitive_settings_load_spinsystem(self->settings, self->spinSystem);
    set_spin_number((InsensitiveWindow *)self->displayController, insensitive_spinsystem_get_spins(self->spinSystem));
    insensitive_controller_calculate_gyromagnetic_ratios(self);
    insensitive_controller_calculate_selectable_delays(self);
    insensitive_controller_return_to_thermal_equilibrium(self);
    insensitive_controller_save_previous_state(self);
    insensitive_controller_calculate_energy_levels(self);
    //init_settings((InsensitiveWindow *)self->displayController, self->settings);
}


void insensitive_controller_connect_pulseShaperController(InsensitiveController *self, InsensitivePulseShaper *pulseShaper)
{
    self->pulseShaperController = pulseShaper;
    insensitive_pulse_shaper_set_controller(pulseShaper, self);
    insensitive_pulse_shaper_set_pulseLength(pulseShaper, insensitive_settings_get_pulseDuration(self->settings));
    insensitive_pulse_shaper_set_pulseFrequency(pulseShaper, insensitive_settings_get_pulseFrequency(self->settings));
    insensitive_pulse_shaper_set_pulseEnvelope(pulseShaper, insensitive_settings_get_pulseEnvelope(self->settings));
}


/////// ////////  /////  //////// ///////     //////  ///////  //////  //    // /////// /////// //////// ///////
//         //    //   //    //    //          //   // //      //    // //    // //      //         //    //
///////    //    ///////    //    /////       //////  /////   //    // //    // /////   ///////    //    ///////
     //    //    //   //    //    //          //   // //      // // // //    // //           //    //         //
///////    //    //   //    //    ///////     //   // ///////  //////   //////  /////// ///////    //    ///////
                                                                  //
gchar *insensitive_controller_get_productOperatorString(InsensitiveController *self)
{
    switch (insensitive_settings_get_operatorBasis(self->settings)) {
	case CartesianOperatorBasis:
		return insensitive_spinsystem_get_cartesianOperatorString(self->spinSystem);
	case SphericalOperatorBasis:
		return insensitive_spinsystem_get_sphericalOperatorString(self->spinSystem);
	}
	return NULL;
}


VectorCoordinates *insensitive_controller_get_vectorCoordinates(InsensitiveController *self)
{
	switch (insensitive_settings_get_vectorDisplayType(self->settings)) {
	case VectorDisplayTypeCoherences:
		return insensitive_spinsystem_get_vectorrepresentation_coherences(self->spinSystem, self->selectedSpin);
	case VectorDisplayTypeMoments:
		return insensitive_spinsystem_get_vectorrepresentation_moments(self->spinSystem, self->selectedSpin);
	case VectorDisplayTypeFID:
		return insensitive_spinsystem_get_vectorrepresentation_fid(self->spinSystem);
	}
	return NULL;
}


void insensitive_controller_initialise_grapefruit_path(InsensitiveController *self)
{
    g_ptr_array_remove_range(self->grapefruitPath, 0, self->grapefruitPath->len);
	insensitive_controller_add_to_grapefruit_path(self);
}


void insensitive_controller_add_to_grapefruit_path(InsensitiveController *self)
{
	float diff, value;
	VectorCoordinates *newVectors, *lastVectors;
	int i, size;

	diff = 0.0;
	newVectors = insensitive_controller_get_vectorCoordinates(self);
	if (self->grapefruitPath->len > 0) {
		lastVectors = g_ptr_array_index(self->grapefruitPath, self->grapefruitPath->len - 1);
		size = newVectors->size;
		for (i = 0; i < size; i++) {
			value = fabsf(lastVectors->x[i] - newVectors->x[i]);
			if (value > diff)
				diff = value;
			value = fabsf(lastVectors->y[i] - newVectors->y[i]);
			if (value > diff)
				diff = value;
			value = fabsf(lastVectors->z[i] - newVectors->z[i]);
			if (value > diff)
				diff = value;
		}
	}
	if (diff > 0.01 || self->grapefruitPath->len == 0) {
		g_ptr_array_add(self->grapefruitPath, newVectors);
		if (self->grapefruitPath->len == 1024) {
			g_ptr_array_remove_index(self->grapefruitPath, 0);
		}
	}
}


GPtrArray *insensitive_controller_get_grapefruit_path(InsensitiveController *self)
{
	return self->grapefruitPath;
}


gboolean insensitive_controller_no_larmorFrequency_set(InsensitiveController *self)
{
	unsigned int i;
	gboolean noLarmorFrequencySet = TRUE;

	for (i = 0; i < insensitive_spinsystem_get_spins(self->spinSystem); i++)
		if (insensitive_spinsystem_get_larmorfrequency_for_spin(self->spinSystem, i) != 0)
			noLarmorFrequencySet = FALSE;
	return noLarmorFrequencySet;
}


gboolean insensitive_controller_no_coupling_set(InsensitiveController *self)
{
	unsigned int i, j;
	gboolean noCouplingSet = TRUE;

	for (j = 0; j < insensitive_spinsystem_get_spins(self->spinSystem); j++)
		for (i = j + 1; i < insensitive_spinsystem_get_spins(self->spinSystem); i++) {
			if (insensitive_spinsystem_get_jcouplingconstant_between_spins(self->spinSystem, i, j) != 0)
				noCouplingSet = FALSE;
		}
	return noCouplingSet;
}


gboolean insensitive_controller_allow_separate_shift_and_coupling(InsensitiveController *self)
{
	if (self->isRecordingPulseSequence)
		return FALSE;
	else if (insensitive_settings_get_strongCoupling(self->settings)
		 && !insensitive_settings_get_allowShiftAndCouplingButtons(self->settings))
		return FALSE;
	else
		return TRUE;
}


/////// //////  // ///    //      ////// //   //  /////  ///    //  //////  /////// ///////
//      //   // // ////   //     //      //   // //   // ////   // //       //      //
/////// //////  // // //  //     //      /////// /////// // //  // //   /// /////   ///////
     // //      // //  // //     //      //   // //   // //  // // //    // //           //
/////// //      // //   ////      ////// //   // //   // //   ////  //////  /////// ///////

unsigned int insensitive_controller_get_selected_spin(InsensitiveController *self)
{
    return self->selectedSpin;
}


void insensitive_controller_set_selected_spin(InsensitiveController *self, unsigned int value)
{
    self->selectedSpin = value;
    //spin_number_was_changed((InsensitiveWindow *)self->displayController);
    spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_gyromagnetic_ratios(InsensitiveController *self, unsigned int codeForI, unsigned int codeForS)
{
    insensitive_settings_set_gyroCodeI(self->settings, codeForI);
    insensitive_settings_set_gyroCodeS(self->settings, codeForS);
    insensitive_controller_calculate_gyromagnetic_ratios(self);
    insensitive_controller_return_to_thermal_equilibrium(self);
    insensitive_controller_calculate_energy_levels(self);
    self->expno = 0;
}


void insensitive_controller_calculate_gyromagnetic_ratios(InsensitiveController *self)
{
    float gyro1, gyro2;

    gyro1 = gyro_for_code(insensitive_settings_get_gyroCodeI(self->settings));
    gyro2 = gyro_for_code(insensitive_settings_get_gyroCodeS(self->settings));

    self->spinSystem->absGyroI = gyro1;
    self->spinSystem->absGyroS = gyro2;
    if(fabsf(gyro1) > fabsf(gyro2)) {
        self->spinSystem->gyroI = (gyro1 > 0) ? 1.0 : -1.0;
        self->spinSystem->gyroS = fabsf(gyro2 / gyro1) * ((gyro2 > 0) ? 1.0 : -1.0);
    } else {
        self->spinSystem->gyroI = fabsf(gyro1 / gyro2) * ((gyro1 > 0) ? 1.0 : -1.0);
        self->spinSystem->gyroS = (gyro2 > 0) ? 1.0 : -1.0;
    }
    insensitive_spinsystem_constants_were_changed(self->spinSystem);
    //[spinEditorController updateCouplingConstantLabels];
}


void insensitive_controller_reset_couplingMatrix(InsensitiveController *self)
{
    insensitive_spinsystem_init_couplingmatrix(self->spinSystem);
    // [spinEditorController updateCouplingConstantLabels];
    insensitive_controller_calculate_energy_levels(self);
    insensitive_controller_calculate_selectable_delays(self);
    insensitive_controller_interrupt_coherencePathway_calculation(self);
    erase_coherencePathway((InsensitiveWindow *)self->displayController);
    self->expno = 0;
}


void insensitive_controller_set_spinType_of_spin(InsensitiveController *self, unsigned int spin, int type)
{
    insensitive_spinsystem_set_spintype_for_spin(self->spinSystem, type, spin);
    insensitive_settings_calculate_selected_spins(self->settings,
                                                  insensitive_spinsystem_get_spins(self->spinSystem),
                                                  insensitive_spinsystem_get_spintypearray(self->spinSystem));
    set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
    set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
    spin_number_was_changed((InsensitiveWindow *)self->displayController);
    erase_coherencePathway((InsensitiveWindow *)self->displayController);
    insensitive_controller_return_to_thermal_equilibrium(self);
    insensitive_controller_calculate_energy_levels(self);

    if(insensitive_settings_get_spinlock(self->settings)) {
        insensitive_controller_set_IDecoupling(self, insensitive_spinsystem_get_number_of_ispins(self->spinSystem) > 0);
        insensitive_controller_set_SDecoupling(self, insensitive_spinsystem_get_number_of_sspins(self->spinSystem) > 0);
    }
    self->expno = 0;
}


void insensitive_controller_set_larmorFrequency_for_spin(InsensitiveController *self, unsigned int spin, float value)
{
    if(fabsf(value) > 127.0)
        value = 127.0;
    insensitive_spinsystem_set_larmorfrequency_for_spin(self->spinSystem, spin, value);
    // [spinEditorController updateCouplingConstantLabels];
    set_larmorFrequency((InsensitiveWindow *)self->displayController,
                        insensitive_spinsystem_get_larmorfrequency_for_spin(self->spinSystem, spin));
    erase_coherencePathway((InsensitiveWindow *)self->displayController);
    insensitive_controller_calculate_energy_levels(self);
    self->expno = 0;
}


void insensitive_controller_set_jCouplingConstant_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float J)
{
    insensitive_spinsystem_set_jcouplingconstant_between_spins(self->spinSystem, spinNumber1, spinNumber2, J);
    // [spinEditorController updateCouplingConstantLabels];
    set_scalarConstant((InsensitiveWindow *)self->displayController,
                        insensitive_spinsystem_get_jcouplingconstant_between_spins(self->spinSystem, spinNumber1, spinNumber2));
    erase_coherencePathway((InsensitiveWindow *)self->displayController);
    insensitive_controller_calculate_selectable_delays(self);
    insensitive_controller_calculate_energy_levels(self);
    self->expno = 0;
}


void insensitive_controller_set_dipolarCouplingConstant_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float b)
{
    insensitive_spinsystem_set_dipolarcouplingconstant_between_spins(self->spinSystem, spinNumber1, spinNumber2, 2 * M_PI * b);
    // [spinEditorController updateCouplingConstantLabels];
    set_dipolarConstant((InsensitiveWindow *)self->displayController,
                         insensitive_spinsystem_get_dipolarcouplingconstant_between_spins(self->spinSystem, spinNumber1, spinNumber2));
    set_distanceConstant((InsensitiveWindow *)self->displayController,
                         insensitive_spinsystem_get_distance_between_spins(self->spinSystem, spinNumber1, spinNumber2));
    self->expno = 0;
}


void insensitive_controller_set_distance_between_spins(InsensitiveController *self, int spinNumber1, int spinNumber2, float r)
{
    insensitive_spinsystem_set_distance_between_spins(self->spinSystem, spinNumber1, spinNumber2, r);
    // [spinEditorController updateCouplingConstantLabels];
    set_dipolarConstant((InsensitiveWindow *)self->displayController,
                         insensitive_spinsystem_get_dipolarcouplingconstant_between_spins(self->spinSystem, spinNumber1, spinNumber2));
    set_distanceConstant((InsensitiveWindow *)self->displayController,
                         insensitive_spinsystem_get_distance_between_spins(self->spinSystem, spinNumber1, spinNumber2));
    self->expno = 0;
}


void insensitive_controller_spin_number_changed(InsensitiveController *self, unsigned int newNumberOfSpins)
{
    unsigned int i, oldNumberOfSpins = insensitive_spinsystem_get_spins(self->spinSystem);

    if ((newNumberOfSpins > 0) && (newNumberOfSpins < 5)) {
        // Resize spin system
        if (newNumberOfSpins == oldNumberOfSpins - 1) {
            for (i = self->selectedSpin + 1; i < oldNumberOfSpins; i++)
                insensitive_settings_set_pulseArray_for_spinnumber(self->settings,
                                                                   i - 1,
                                                                   insensitive_settings_get_pulseArray(self->settings) & pow2(i),
                                                                   oldNumberOfSpins,
                                                                   insensitive_spinsystem_get_spintypearray(self->spinSystem));
            insensitive_spinsystem_remove_spin_number(self->spinSystem, self->selectedSpin);
        } else {
            insensitive_spinsystem_set_spins(self->spinSystem, newNumberOfSpins);
        }
        insensitive_spinsystem_resize_couplingmatrix_for_spins(self->spinSystem, newNumberOfSpins);
        insensitive_spinsystem_set_spintypearray(self->spinSystem,
                                                 insensitive_spinsystem_get_spintypearray(self->spinSystem) & (pow2(newNumberOfSpins) - 1));

        // Deactivate all removed spins
        insensitive_settings_set_pulseArray(self->settings,
                                            insensitive_settings_get_pulseArray(self->settings) & (pow2(newNumberOfSpins) - 1),
                                            newNumberOfSpins,
                                            insensitive_spinsystem_get_spintypearray(self->spinSystem));
        // Activate newly created spins
        if(oldNumberOfSpins < newNumberOfSpins)
            insensitive_settings_set_pulseArray(self->settings,
                                            insensitive_settings_get_pulseArray(self->settings) | ((pow2(newNumberOfSpins) - 1) - (pow2(oldNumberOfSpins) - 1)),
                                            newNumberOfSpins,
                                            insensitive_spinsystem_get_spintypearray(self->spinSystem));

        // Reset selected spin if necessary
        if(self->selectedSpin >= newNumberOfSpins) {
            self->selectedSpin = 0;
            spin_was_selected((InsensitiveWindow *)self->displayController, 0);
        }

        // Update user interface
        //[spinEditorController updateCouplingConstantLabels];
        spin_number_was_changed((InsensitiveWindow *)self->displayController);
        spin_state_was_changed((InsensitiveWindow *)self->displayController);
        if(self->operationIsInProgress) {
            g_source_remove(self->stepwiseOperationTimerNr);
            self->operationIsInProgress = FALSE;
		    stop_progress_indicator((InsensitiveWindow *)self->displayController);
        }
        if(insensitive_settings_get_spinlock(self->settings)) {
            insensitive_controller_set_IDecoupling(self, insensitive_spinsystem_get_number_of_ispins(self->spinSystem) > 0);
            insensitive_controller_set_SDecoupling(self, insensitive_spinsystem_get_number_of_sspins(self->spinSystem) > 0);
        }
        set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
        set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
        set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
        set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
        set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
        insensitive_controller_calculate_selectable_delays(self);
        insensitive_controller_calculate_energy_levels(self);
        insensitive_controller_interrupt_coherencePathway_calculation(self);
        erase_coherencePathway((InsensitiveWindow *)self->displayController);
        self->expno = 0;
    }
    set_spin_number((InsensitiveWindow *)self->displayController, newNumberOfSpins);
    insensitive_controller_save_previous_state(self);
    insensitive_controller_initialise_grapefruit_path(self);
}


float insensitive_controller_get_unitConversion(InsensitiveController *self)
{
    return insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(self->settings) ? 360 : 1;
}


void insensitive_controller_update_matrix_with(InsensitiveController *self, int numberOfOperators, int *base4Index, float *coefficient)
{
    DSPComplex *newMatrix;

    newMatrix = matrix_from_base4_array(base4Index,
                                        insensitive_spinsystem_get_spins(self->spinSystem),
                                        numberOfOperators,
                                        insensitive_spinsystem_get_spintypearray(self->spinSystem),
                                        coefficient,
                                        self->spinSystem->gyroI,
                                        self->spinSystem->gyroI);
    insensitive_spinsystem_substitute_matrix(self->spinSystem, newMatrix);
    free(newMatrix);

    insensitive_controller_initialise_grapefruit_path(self);
    spin_state_was_changed((InsensitiveWindow *)self->displayController);

    // Stop saving actions after gradients
    insensitive_spinsystem_set_firstgradientpulseissued(self->spinSystem, FALSE);
    insensitive_spinsystem_free_gradient_array(self->spinSystem);
}


void insensitive_controller_rotate_spinsystem(InsensitiveController *self, gboolean backwards)
{
	unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);
	unsigned int i, j, oldI, oldJ, newSpinTypeArray = 0;
	float *oldCouplingMatrix = insensitive_spinsystem_get_raw_couplingmatrix(self->spinSystem);
	float *newCouplingMatrix = malloc(spins * spins * sizeof(float));

	for (j = 0; j < spins; j++) {
		// backwards: 2 <- 0 <- 1 <- 2
		// forwards : 2 -> 0 -> 1 -> 2
		// Consider case for four spins: make rotation intuitive
		// backwards: 1 <- 0 <- 2 <- 3 <- 1
		// forwards:  0 -> 1 -> 3 -> 2 -> 0
		if (backwards) {
			if (spins == 4) {
				switch (j) {
				case 0:
					oldJ = 1;
					break;
				case 1:
					oldJ = 3;
					break;
				case 3:
					oldJ = 2;
					break;
				default:
					oldJ = 0;
				}
			} else
				oldJ = (j == spins - 1) ? 0 : j + 1;
		} else {
			if (spins == 4) {
				switch (j) {
				case 0:
					oldJ = 2;
					break;
				case 3:
					oldJ = 1;
					break;
				case 2:
					oldJ = 3;
					break;
				default:
					oldJ = 0;
				}
			} else
				oldJ = (j == 0) ? spins - 1 : j - 1;
		}
		// Rearrange spin types
		if (insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, oldJ) == spinTypeS)
			newSpinTypeArray += pow2(j);
		// Rearrange coupling matrix
		for (i = 0; i < spins; i++) {
			if (backwards) {
				if (spins == 4) {
					switch (i) {
					case 0:
						oldI = 1;
						break;
					case 1:
						oldI = 3;
						break;
					case 3:
						oldI = 2;
						break;
					default:
						oldI = 0;
					}
				} else
					oldI = (i == spins - 1) ? 0 : i + 1;
			} else {
				if (spins == 4) {
					switch (i) {
					case 0:
						oldI = 2;
						break;
					case 3:
						oldI = 1;
						break;
					case 2:
						oldI = 3;
						break;
					default:
						oldI = 0;
					}
				} else
					oldI = (i == 0) ? spins - 1 : i - 1;
			}
			// Make sure J and r won't get scrambled up
			if ((i < j && oldI > oldJ) || (i > j && oldI < oldJ))
				newCouplingMatrix[i * spins + j] = oldCouplingMatrix[oldJ * spins + oldI];
			else
				newCouplingMatrix[i * spins + j] = oldCouplingMatrix[oldI * spins + oldJ];
		}
	}
    insensitive_spinsystem_set_spintypearray(self->spinSystem, newSpinTypeArray);
	insensitive_spinsystem_substitute_couplingmatrix(self->spinSystem, newCouplingMatrix);
	free(newCouplingMatrix);
    insensitive_controller_spin_number_changed(self,spins);
    insensitive_controller_return_to_thermal_equilibrium(self);
	// Stop saving actions after gradients
    insensitive_spinsystem_set_firstgradientpulseissued(self->spinSystem, FALSE);
	insensitive_spinsystem_free_gradient_array(self->spinSystem);
}


void insensitive_controller_redraw_current_spinsystem_state(InsensitiveController *self)
{
    spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


///    // ///    /// //////       //////  //////  /////// //////   /////  //////// //  //////  ///    // ///////
////   // ////  //// //   //     //    // //   // //      //   // //   //    //    // //    // ////   // //
// //  // // //// // //////      //    // //////  /////   //////  ///////    //    // //    // // //  // ///////
//  // // //  //  // //   //     //    // //      //      //   // //   //    //    // //    // //  // //      //
//   //// //      // //   //      //////  //      /////// //   // //   //    //    //  //////  //   //// ///////

void insensitive_controller_save_previous_state(InsensitiveController *self)
{
    unsigned int i;
    SequenceElement *element, *original;

    g_clear_object(&self->previousSpinSystem);
    self->previousSpinSystem = insensitive_spinsystem_copy(self->spinSystem);

    g_clear_object(&self->previousPulseSequence);
    self->previousPulseSequence = insensitive_pulsesequence_copy(self->pulseSequence);
    g_ptr_array_remove_range(self->previousPulseList, 0, self->previousPulseList->len);
    for (i = 0; i < self->pulseList->len; i++) {
        element = malloc(sizeof(SequenceElement));
        original = g_ptr_array_index(self->pulseList, i);
        element->type = original->type;
        element->time = original->time;
        element->secondParameter = original->secondParameter;
        element->pulseArray = original->pulseArray;
        element->iDecoupling = original->iDecoupling;
        element->sDecoupling = original->sDecoupling;
        element->activeISpins = original->activeISpins;
        element->activeSSpins = original->activeSSpins;
        element->selectiveIPulse = original->selectiveIPulse;
        element->selectiveSPulse = original->selectiveSPulse;
        element->spinlock = original->spinlock;
        element->pulseDuration = original->pulseDuration;
        element->pulseStrength = original->pulseStrength;
        element->pulseFrequency = original->pulseFrequency;
        element->pulseEnvelope = original->pulseEnvelope;
        g_ptr_array_add(self->previousPulseList, element);
    }
    self->previousStepInPulseSequence = self->currentStepInPulseSequence;
    insensitive_controller_save_previous_phaseCyclingTable(self);
}


void insensitive_controller_save_previous_phaseCyclingTable(InsensitiveController *self)
{
    unsigned int i;
    gchar *str;

    g_ptr_array_free(self->previousPhaseCyclingArray, TRUE);
    self->previousPhaseCyclingArray = g_ptr_array_new();
    for (i = 0; i < self->phaseCyclingArray->len; i++) {
        str = malloc(5 * sizeof(gchar));
        strcpy(str, g_ptr_array_index(self->phaseCyclingArray, i));
        g_ptr_array_add(self->previousPhaseCyclingArray, str);
    }
    self->previousPhaseCycles = self->phaseCycles;
}


void insensitive_controller_restore_previous_state(InsensitiveController *self)
{
    unsigned int tempPhaseCycles, tempStepInPulseSequence;
    InsensitiveSpinSystem *tempSpinSystem;
    GPtrArray *tempArray;
    InsensitivePulseSequence *tempSequence;

    tempSpinSystem = self->spinSystem;
	self->spinSystem = self->previousSpinSystem;
	self->previousSpinSystem = tempSpinSystem;
    if(self->isRecordingPulseSequence) {
        tempSequence = self->pulseSequence;
        self->pulseSequence = self->previousPulseSequence;
        self->previousPulseSequence = tempSequence;

        tempArray = self->pulseList;
        self->pulseList = self->previousPulseList;
        self->previousPulseList = tempArray;
        tempPhaseCycles = self->phaseCycles;
        self->phaseCycles = self->previousPhaseCycles;
        self->previousPhaseCycles = tempPhaseCycles;
        tempArray = self->phaseCyclingArray;
        self->phaseCyclingArray = self->previousPhaseCyclingArray;
        self->previousPhaseCyclingArray = tempArray;
        if ((int)self->previousPulseList->len - (int)self->pulseList->len == 1)
            remove_last_column_from_phaseCyclingTable((InsensitiveWindow *)self->displayController);
        else if (self->previousPulseList->len - self->pulseList->len == -1)
            insert_column_into_phaseCyclingTable((InsensitiveWindow *)self->displayController, self->pulseList->len + 1, 1);
    }
    insensitive_controller_initialise_grapefruit_path(self);
    tempStepInPulseSequence = self->currentStepInPulseSequence;
    insensitive_controller_set_currentStepInPulseSequence(self, self->previousStepInPulseSequence);
    self->previousStepInPulseSequence = tempStepInPulseSequence;
    spin_state_was_changed((InsensitiveWindow *)self->displayController);
    update_pulseSequence((InsensitiveWindow *)self->displayController);
    update_phaseCyclingTable((InsensitiveWindow *)self->displayController, self->pulseList->len + 2);
}


void insensitive_controller_return_to_thermal_equilibrium(InsensitiveController *self)
{
	insensitive_controller_save_previous_state(self);
	insensitive_spinsystem_return_to_thermal_equilibrium(self->spinSystem);
	insensitive_controller_initialise_grapefruit_path(self);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
}


void insensitive_controller_perform_pulse(InsensitiveController *self, float angle, float phase, gboolean animated)
{
	insensitive_controller_save_previous_state(self);

	insensitive_controller_set_flipAngle(self, angle);
	insensitive_controller_set_phase(self, phase);

	insensitive_controller_perform_pulse_animated(self, animated);
}


void insensitive_controller_perform_pulse_animated(InsensitiveController *self, gboolean animated)
{
	unsigned int i, step;
	float stepDuration, phase, strength, rest, timerFrequency;
    gchar *phaseString;

	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);

	// Gradient & Recording operations
	if (insensitive_spinsystem_get_firstgradientpulseissued(self->spinSystem))
		insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypePulse, self->settings);

	if (self->isRecordingPulseSequence) {
        insensitive_pulsesequence_add_element(self->pulseSequence, SequenceTypePulse, self->settings);
        g_ptr_array_add(self->pulseList, insensitive_pulsesequence_get_last_element(self->pulseSequence));
		update_pulseSequence((InsensitiveWindow *)self->displayController);
		for (step = 0; step < self->phaseCycles; step++) {
            phaseString = malloc(5 * sizeof(gchar));
            sprintf(phaseString, "%.0f", insensitive_settings_get_phase(self->settings));
            g_ptr_array_insert(self->phaseCyclingArray, step * (self->pulseList->len + 1) + self->pulseList->len, phaseString);
		}
        insert_column_into_phaseCyclingTable((InsensitiveWindow *)self->displayController, self->pulseList->len + 1, 1);
	}

	if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings)) {
		// Perform pulse while ignoring off-resonance effects
		if (!animated) {
			// Perform pulse in one step
			insensitive_spinsystem_pulse(self->spinSystem,
						     insensitive_settings_get_flipAngle(self->settings),
						     insensitive_settings_get_phase(self->settings),
						     insensitive_settings_get_pulseArray(self->settings),
						     insensitive_settings_get_pulsePowerSpectrum(self->settings));
			spin_state_was_changed((InsensitiveWindow *)self->displayController);
			if (insensitive_settings_get_acquisitionAfterNextPulse(self->settings)) {
				insensitive_settings_set_acquisitionAfterNextPulse(self->settings, FALSE);
				disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
				insensitive_controller_perform_acquisition(self);
			}
		} else {
			// Calculate number of steps and account for angles smaller than one step
			self->stepsToBePerformedForOperation = (int)(insensitive_settings_get_flipAngle(self->settings) / 3);
			rest = (insensitive_settings_get_flipAngle(self->settings) / 3 - self->stepsToBePerformedForOperation) * 3;
			if (rest != 0) {
				insensitive_spinsystem_pulse(self->spinSystem,
							     insensitive_settings_get_flipAngle(self->settings),
							     insensitive_settings_get_phase(self->settings),
							     insensitive_settings_get_pulseArray(self->settings),
							     insensitive_settings_get_pulsePowerSpectrum(self->settings));
				insensitive_controller_add_to_grapefruit_path(self);
				spin_state_was_changed((InsensitiveWindow *)self->displayController);
			}
			// Perform the pulse in steps
			if (insensitive_settings_get_flipAngle(self->settings) > 3) {
				if (self->operationIsInProgress)
					g_source_remove(self->stepwiseOperationTimerNr);
				set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
				self->stepsPerformedForOperation = 0;
				self->operationIsInProgress = TRUE;
				self->stepwiseOperationTimerNr = g_timeout_add(25, pulseTimerEvent, self);
			} else {
				stop_progress_indicator((InsensitiveWindow *)self->displayController);
				if (insensitive_settings_get_acquisitionAfterNextPulse(self->settings)) {
					insensitive_settings_set_acquisitionAfterNextPulse(self->settings, FALSE);
					disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
					show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);
					insensitive_controller_perform_acquisition(self);
				}
			}
		}
	} else {
		// Perform pulse with off-resonance effects
		stepDuration = insensitive_settings_get_pulseDuration(self->settings) / pulseShapeResolution;
		self->danteCounter = (insensitive_settings_get_pulseEnvelope(self->settings) == DANTE) ? maxDanteCycles : 1;

		if (!animated) {
			for ( ; self->danteCounter > 0; self->danteCounter--) {
				for (i = 0; i < pulseShapeResolution; i++) {
					strength = insensitive_settings_get_pulseShape(self->settings).realp[i] * insensitive_settings_get_pulseStrength(self->settings);
					phase = insensitive_settings_get_pulseShape(self->settings).imagp[i] + insensitive_settings_get_phase(self->settings);
					while (phase > 360) {
						phase -= 360;
					}
					insensitive_spinsystem_offresonancepulse(self->spinSystem,
										 strength * 720,
										 stepDuration,
										 phase,
										 insensitive_settings_get_pulseFrequency(self->settings),
										 insensitive_settings_get_pulseArray(self->settings));
					insensitive_controller_add_to_grapefruit_path(self);
				}
			}
			spin_state_was_changed((InsensitiveWindow *)self->displayController);
			if (insensitive_settings_get_acquisitionAfterNextPulse(self->settings)) {
				insensitive_settings_set_acquisitionAfterNextPulse(self->settings, FALSE);
				disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
				insensitive_controller_perform_acquisition(self);
			}
			stop_progress_indicator((InsensitiveWindow *)self->displayController);
		} else {
			if (self->operationIsInProgress)
				g_source_remove(self->stepwiseOperationTimerNr);
			set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
			self->stepsPerformedForOperation = 0;
			timerFrequency = (insensitive_settings_get_pulseEnvelope(self->settings) == DANTE) ? 1 : 5;
			self->operationIsInProgress = TRUE;
			self->stepwiseOperationTimerNr = g_timeout_add(insensitive_settings_get_flipAngle(self->settings) / 90 * timerFrequency, pulseWithOffResonanceEffectsTimerEvent, self);
		}
	}
}


gboolean pulseTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;

	insensitive_spinsystem_pulse(self->spinSystem,
				     3.0,
				     insensitive_settings_get_phase(self->settings),
				     insensitive_settings_get_pulseArray(self->settings),
				     insensitive_settings_get_pulsePowerSpectrum(self->settings));
	insensitive_controller_add_to_grapefruit_path(self);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	self->stepsPerformedForOperation++;
	if (self->stepsPerformedForOperation >= self->stepsToBePerformedForOperation) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		if (insensitive_settings_get_acquisitionAfterNextPulse(self->settings)) {
			insensitive_settings_set_acquisitionAfterNextPulse(self->settings, FALSE);
			disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
			show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);
			insensitive_controller_perform_acquisition(self);
		}
		if (insensitive_settings_get_animates(self->settings))
			insensitive_controller_initialise_grapefruit_path(self);
		return FALSE;
	}
	return TRUE;
}


gboolean pulseWithOffResonanceEffectsTimerEvent(gpointer user_data)
{
	float strength, phase;
	InsensitiveController *self = (InsensitiveController *)user_data;

	strength = insensitive_settings_get_pulseShape(self->settings).realp[self->stepsPerformedForOperation] * insensitive_settings_get_pulseStrength(self->settings);
	phase = insensitive_settings_get_pulseShape(self->settings).imagp[self->stepsPerformedForOperation] + insensitive_settings_get_phase(self->settings);

	while (phase > 360) {
		phase -= 360;
	}
	insensitive_spinsystem_offresonancepulse(self->spinSystem,
						 strength * 720,
						 insensitive_settings_get_pulseDuration(self->settings) / pulseShapeResolution,
						 phase,
						 insensitive_settings_get_pulseFrequency(self->settings),
						 insensitive_settings_get_pulseArray(self->settings));
	insensitive_controller_add_to_grapefruit_path(self);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	self->stepsPerformedForOperation++;
	if (self->stepsPerformedForOperation >= pulseShapeResolution) {
		self->danteCounter--;
		self->stepsPerformedForOperation = 0;
	}
	if (self->danteCounter == 0) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		if (insensitive_settings_get_acquisitionAfterNextPulse(self->settings)) {
			insensitive_settings_set_acquisitionAfterNextPulse(self->settings, FALSE);
			disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
			show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);
			insensitive_controller_perform_acquisition(self);
		}
		if (insensitive_settings_get_animates(self->settings))
			insensitive_controller_initialise_grapefruit_path(self);
		return FALSE;
	}
	return TRUE;
}


void insensitive_controller_perform_chemicalShift_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_size(self->spinSystem) - 1;
	insensitive_controller_perform_chemicalShift_with_array_animated(self, animated);
}


void insensitive_controller_perform_chemicalShift_on_ISpins_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_pulsearray_for_spintype(self->spinSystem, spinTypeI);
	insensitive_controller_perform_chemicalShift_with_array_animated(self, animated);
}


void insensitive_controller_perform_chemicalShift_on_SSpins_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_pulsearray_for_spintype(self->spinSystem, spinTypeS);
	insensitive_controller_perform_chemicalShift_with_array_animated(self, animated);
}


void insensitive_controller_perform_chemicalShift_on_spinArray(InsensitiveController *self, int array, gboolean animated)
{
	self->selectivePrecessionArray = array;
	insensitive_controller_perform_chemicalShift_with_array_animated(self, animated);
}


void insensitive_controller_perform_chemicalShift_with_array_animated(InsensitiveController *self, gboolean animated)
{
	float rest;
	float delay = insensitive_settings_get_delay(self->settings);
	float dwellTime = insensitive_settings_get_dwellTime(self->settings);

	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);

	// Gradient operations
	if (insensitive_spinsystem_get_firstgradientpulseissued(self->spinSystem))
		insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypeShift, self->settings);

	if (!animated || (delay < dwellTime)) {
		// Perform chemical shift in one step
		if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
		insensitive_spinsystem_chemicalshift_for_spins(self->spinSystem,
							       delay,
							       insensitive_settings_get_dephasingJitter(self->settings),
							       self->selectivePrecessionArray);
		if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
		spin_state_was_changed((InsensitiveWindow *)self->displayController);
	} else {
		// Calculate number of steps and account for delay times smaller than one step
		rest = fmod(delay, dwellTime);
		self->stepsToBePerformedForOperation = (delay - rest) / dwellTime;
		if (rest != 0) {
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
			insensitive_spinsystem_chemicalshift_for_spins(self->spinSystem,
								       rest,
								       insensitive_settings_get_dephasingJitter(self->settings),
								       self->selectivePrecessionArray);
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
			spin_state_was_changed((InsensitiveWindow *)self->displayController);
		}
		// Perform the chemical shift in steps
		if (self->operationIsInProgress)
			g_source_remove(self->stepwiseOperationTimerNr);
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
		self->stepsPerformedForOperation = 0;
		self->operationIsInProgress = TRUE;
		self->stepwiseOperationTimerNr = g_timeout_add(25, chemicalShiftTimerEvent, self);
	}
}


gboolean chemicalShiftTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;

	if (insensitive_settings_get_spinlock(self->settings))
		insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
	insensitive_spinsystem_chemicalshift_for_spins(self->spinSystem,
						       insensitive_settings_get_dwellTime(self->settings),
						       insensitive_settings_get_dephasingJitter(self->settings),
						       self->selectivePrecessionArray);
	if (insensitive_settings_get_spinlock(self->settings))
		insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	self->stepsPerformedForOperation++;
	if (self->stepsPerformedForOperation >= self->stepsToBePerformedForOperation) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		return FALSE;
	}
	return TRUE;
}


void insensitive_controller_perform_coupling_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_size(self->spinSystem) - 1;
	insensitive_controller_perform_coupling_with_array_animated(self, animated);
}


void insensitive_controller_perform_coupling_on_ISpins_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_pulsearray_for_spintype(self->spinSystem, spinTypeI);
	insensitive_controller_perform_coupling_with_array_animated(self, animated);
}


void insensitive_controller_perform_coupling_on_SSpins_animated(InsensitiveController *self, gboolean animated)
{
	self->selectivePrecessionArray = insensitive_spinsystem_get_pulsearray_for_spintype(self->spinSystem, spinTypeS);
	insensitive_controller_perform_coupling_with_array_animated(self, animated);
}


void insensitive_controller_perform_coupling_on_spinArray(InsensitiveController *self, int array, gboolean animated)
{
	self->selectivePrecessionArray = array;
	insensitive_controller_perform_coupling_with_array_animated(self, animated);
}


void insensitive_controller_perform_coupling_with_array_animated(InsensitiveController *self, gboolean animated)
{
	float rest;
	float delay = insensitive_settings_get_delay(self->settings);
	float dwellTime = insensitive_settings_get_dwellTime(self->settings);

	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);

	// Gradient operations
	if (insensitive_spinsystem_get_firstgradientpulseissued(self->spinSystem))
        insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypeCoupling, self->settings);

	if ((!animated && !insensitive_settings_get_strongCoupling(self->settings)) || (delay < dwellTime)) {
		// Perform coupling in one step
		if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
		    && !insensitive_settings_get_spinlock(self->settings)) {
			insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
								   delay / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
								   self->selectivePrecessionArray);
			insensitive_spinsystem_perform_decoupling(self->spinSystem,
								  insensitive_settings_get_iDecoupling(self->settings),
								  insensitive_settings_get_sDecoupling(self->settings),
								  insensitive_settings_get_phase(self->settings));
			insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
								   delay / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
								   self->selectivePrecessionArray);
		} else {
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
			insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
								   delay,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
								   self->selectivePrecessionArray);
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
		}
		spin_state_was_changed((InsensitiveWindow *)self->displayController);
	} else {
		// Calculate number of steps and account for delay times smaller than one step
		rest = fmodf(delay, dwellTime);
		self->stepsToBePerformedForOperation = (delay - rest) / dwellTime;
		if (rest != 0) {
			if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
			    && !insensitive_settings_get_spinlock(self->settings)) {
				insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
									   rest / 2,
									   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
									   self->selectivePrecessionArray);
				insensitive_spinsystem_perform_decoupling(self->spinSystem,
									  insensitive_settings_get_iDecoupling(self->settings),
									  insensitive_settings_get_sDecoupling(self->settings),
									  insensitive_settings_get_phase(self->settings));
				insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
									   rest / 2,
									   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
									   self->selectivePrecessionArray);
			} else {
				if (insensitive_settings_get_spinlock(self->settings))
					insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
				insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
									   rest,
									   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
									   self->selectivePrecessionArray);
				if (insensitive_settings_get_spinlock(self->settings))
					insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
			}
			spin_state_was_changed((InsensitiveWindow *)self->displayController);
		}
		// Perform the coupling in steps
		if (self->operationIsInProgress)
			g_source_remove(self->stepwiseOperationTimerNr);
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
		self->stepsPerformedForOperation = 0;
		self->operationIsInProgress = TRUE;
		self->stepwiseOperationTimerNr = g_timeout_add(25, couplingTimerEvent, self);
	}
}


gboolean couplingTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;

	if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
	    && !insensitive_settings_get_spinlock(self->settings)) {
		insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings) / 2,
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
							   self->selectivePrecessionArray);
		insensitive_spinsystem_perform_decoupling(self->spinSystem,
							  insensitive_settings_get_iDecoupling(self->settings),
							  insensitive_settings_get_sDecoupling(self->settings),
							  insensitive_settings_get_phase(self->settings));
		insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings) / 2,
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
							   self->selectivePrecessionArray);
	} else {
		if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
		insensitive_spinsystem_jcoupling_for_spins(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings),
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode,
							   self->selectivePrecessionArray);
        if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
	}
	self->stepsPerformedForOperation++;
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	if (self->stepsPerformedForOperation >= self->stepsToBePerformedForOperation) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		return FALSE;
	}
	return TRUE;
}


void insensitive_controller_perform_relaxation_animated(InsensitiveController *self, gboolean animated)
{
	float rest;
	float delay = insensitive_settings_get_delay(self->settings);
	float dwellTime = insensitive_settings_get_dwellTime(self->settings);
	float correlationTime = insensitive_settings_get_correlationTime(self->settings);

	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);

	// Gradient operations
	if (insensitive_spinsystem_get_firstgradientpulseissued(self->spinSystem))
		insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypeRelaxation, self->settings);

	if (!animated) {
		// Perform relaxation in one step
		if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem, delay, correlationTime);
			else
				insensitive_spinsystem_dipolarrelaxation(self->spinSystem, delay, correlationTime);
		} else
			insensitive_spinsystem_simplerelaxation(self->spinSystem,
								delay,
								insensitive_settings_get_T1(self->settings),
								insensitive_settings_get_T2(self->settings),
								insensitive_settings_get_spinlock(self->settings));
		spin_state_was_changed((InsensitiveWindow *)self->displayController);
	} else {
		// Calculate number of steps and account for delay times smaller than one step
		rest = fmodf(delay, dwellTime);
		self->stepsToBePerformedForOperation = (delay - rest) / dwellTime;
		if (rest != 0) {
			if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
				if (insensitive_settings_get_spinlock(self->settings)) {
					insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
					insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem, rest, correlationTime);
					insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
				} else
					insensitive_spinsystem_dipolarrelaxation(self->spinSystem, rest, correlationTime);
			} else
				insensitive_spinsystem_simplerelaxation(self->spinSystem,
									rest,
									insensitive_settings_get_T1(self->settings),
									insensitive_settings_get_T2(self->settings),
									insensitive_settings_get_spinlock(self->settings));
			spin_state_was_changed((InsensitiveWindow *)self->displayController);
		}
		// Perform the chemical shift in steps
		if (self->operationIsInProgress)
			g_source_remove(self->stepwiseOperationTimerNr);
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
		self->stepsPerformedForOperation = 0;
		self->operationIsInProgress = TRUE;
		self->stepwiseOperationTimerNr = g_timeout_add(25, relaxationTimerEvent, self);
	}
}


gboolean relaxationTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;

	if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
		if (insensitive_settings_get_spinlock(self->settings)) {
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
			insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem,
									   insensitive_settings_get_dwellTime(self->settings),
									   insensitive_settings_get_correlationTime(self->settings));
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
		} else
			insensitive_spinsystem_dipolarrelaxation(self->spinSystem,
								 insensitive_settings_get_dwellTime(self->settings),
								 insensitive_settings_get_correlationTime(self->settings));
	} else
		insensitive_spinsystem_simplerelaxation(self->spinSystem,
							insensitive_settings_get_dwellTime(self->settings),
							insensitive_settings_get_T1(self->settings),
							insensitive_settings_get_T2(self->settings),
							insensitive_settings_get_spinlock(self->settings));
	self->stepsPerformedForOperation++;
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	if (self->stepsPerformedForOperation >= self->stepsToBePerformedForOperation) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		return FALSE;
	}
	return TRUE;
}


void insensitive_controller_perform_freeEvolution_animated(InsensitiveController *self, gboolean animated)
{
	float rest;
    float delay = insensitive_settings_get_delay(self->settings);
	float dwellTime = insensitive_settings_get_dwellTime(self->settings);
	float correlationTime = insensitive_settings_get_correlationTime(self->settings);

	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);

	// Gradient & Recording operations
	if (insensitive_spinsystem_get_firstgradientpulseissued(self->spinSystem))
		insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypeEvolution, self->settings);

	if (self->isRecordingPulseSequence) {
		insensitive_pulsesequence_add_element(self->pulseSequence, SequenceTypeEvolution, self->settings);
		update_pulseSequence((InsensitiveWindow *)self->displayController);
		update_evolutionTimes_combobox((InsensitiveWindow *)self->displayController);
	}

	if ((!animated && !insensitive_settings_get_strongCoupling(self->settings)) || (delay < dwellTime)) {
		// Perform coupling in one step
		if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
		insensitive_spinsystem_chemicalshift(self->spinSystem,
							       delay,
							       insensitive_settings_get_dephasingJitter(self->settings));
		if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
		    && !insensitive_settings_get_spinlock(self->settings)) {
			insensitive_spinsystem_jcoupling(self->spinSystem,
								   delay / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
			insensitive_spinsystem_perform_decoupling(self->spinSystem,
								  insensitive_settings_get_iDecoupling(self->settings),
								  insensitive_settings_get_sDecoupling(self->settings),
								  insensitive_settings_get_phase(self->settings));
			insensitive_spinsystem_jcoupling(self->spinSystem,
								   delay / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
		} else
			insensitive_spinsystem_jcoupling(self->spinSystem,
								   delay,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
		if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
            if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			    if (insensitive_settings_get_spinlock(self->settings))
				    insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem, delay, correlationTime);
			    else
				    insensitive_spinsystem_dipolarrelaxation(self->spinSystem, delay, correlationTime);
		    } else
			    insensitive_spinsystem_simplerelaxation(self->spinSystem,
							                        	delay,
								                        insensitive_settings_get_T1(self->settings),
							                        	insensitive_settings_get_T2(self->settings),
							                        	insensitive_settings_get_spinlock(self->settings));
        }
		if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
		spin_state_was_changed((InsensitiveWindow *)self->displayController);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
	} else {
		// Calculate number of steps and account for delay times smaller than one step
		rest = fmodf(delay, dwellTime);
		self->stepsToBePerformedForOperation = (delay - rest) / dwellTime;
		if (rest != 0) {
			if (insensitive_settings_get_spinlock(self->settings))
			    insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
			insensitive_spinsystem_chemicalshift(self->spinSystem,
							       rest,
							       insensitive_settings_get_dephasingJitter(self->settings));
			if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
		        && !insensitive_settings_get_spinlock(self->settings)) {
			insensitive_spinsystem_jcoupling(self->spinSystem,
								   rest / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
			insensitive_spinsystem_perform_decoupling(self->spinSystem,
								  insensitive_settings_get_iDecoupling(self->settings),
								  insensitive_settings_get_sDecoupling(self->settings),
								  insensitive_settings_get_phase(self->settings));
			insensitive_spinsystem_jcoupling(self->spinSystem,
								   rest / 2,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
			} else
				insensitive_spinsystem_jcoupling(self->spinSystem,
								   rest,
								   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
			if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
            if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			    if (insensitive_settings_get_spinlock(self->settings))
				    insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem, rest, correlationTime);
			    else
				    insensitive_spinsystem_dipolarrelaxation(self->spinSystem, rest, correlationTime);
		    } else
			    insensitive_spinsystem_simplerelaxation(self->spinSystem,
							                        	rest,
								                        insensitive_settings_get_T1(self->settings),
							                        	insensitive_settings_get_T2(self->settings),
							                        	insensitive_settings_get_spinlock(self->settings));
        }
		if (insensitive_settings_get_spinlock(self->settings))
			insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
		}
		spin_state_was_changed((InsensitiveWindow *)self->displayController);
		// Perform the coupling in steps
        if (self->operationIsInProgress)
			g_source_remove(self->stepwiseOperationTimerNr);
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
		self->stepsPerformedForOperation = 0;
		self->operationIsInProgress = TRUE;
		self->stepwiseOperationTimerNr = g_timeout_add(25, freeEvolutionTimerEvent, self);
	}
}


gboolean freeEvolutionTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;

	if (insensitive_settings_get_spinlock(self->settings))
		insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
	insensitive_spinsystem_chemicalshift(self->spinSystem,
						       insensitive_settings_get_dwellTime(self->settings),
						       insensitive_settings_get_dephasingJitter(self->settings));
	if ((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings))
	    && !insensitive_settings_get_spinlock(self->settings)) {
		insensitive_spinsystem_jcoupling(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings) / 2,
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
		insensitive_spinsystem_perform_decoupling(self->spinSystem,
							  insensitive_settings_get_iDecoupling(self->settings),
							  insensitive_settings_get_sDecoupling(self->settings),
							  insensitive_settings_get_phase(self->settings));
		insensitive_spinsystem_jcoupling(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings) / 2,
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
	} else
		insensitive_spinsystem_jcoupling(self->spinSystem,
							   insensitive_settings_get_dwellTime(self->settings),
							   insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
	if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
		if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			if (insensitive_settings_get_spinlock(self->settings))
				insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem,
										   insensitive_settings_get_dwellTime(self->settings),
										   insensitive_settings_get_correlationTime(self->settings));
			else
				insensitive_spinsystem_dipolarrelaxation(self->spinSystem,
									 insensitive_settings_get_dwellTime(self->settings),
									 insensitive_settings_get_correlationTime(self->settings));
		} else
			insensitive_spinsystem_simplerelaxation(self->spinSystem,
								insensitive_settings_get_dwellTime(self->settings),
								insensitive_settings_get_T1(self->settings),
								insensitive_settings_get_T2(self->settings),
								insensitive_settings_get_spinlock(self->settings));
	}
	if (insensitive_settings_get_spinlock(self->settings))
		insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
	self->stepsPerformedForOperation++;
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
	if (self->stepsPerformedForOperation >= self->stepsToBePerformedForOperation) {
		self->operationIsInProgress = FALSE;
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
		stop_progress_indicator((InsensitiveWindow *)self->displayController);
		return FALSE;
	}
	return TRUE;
}


void insensitive_controller_perform_gradient(InsensitiveController *self)
{
	insensitive_controller_save_previous_state(self);
	insensitive_controller_initialise_grapefruit_path(self);
	start_progress_indicator((InsensitiveWindow *)self->displayController);
	//[self performSelector: @selector(gradientEvent:) withObject : NULL afterDelay : 0];
    gradientEvent(self);
}


void gradientEvent(gpointer user_data)
{
    InsensitiveController *self = (InsensitiveController *)user_data;

	// Recording operations
    if (self->isRecordingPulseSequence) {
		insensitive_pulsesequence_add_element(self->pulseSequence, SequenceTypeGradient, self->settings);
		update_pulseSequence((InsensitiveWindow *)self->displayController);
	}
    insensitive_spinsystem_gradient(self->spinSystem,
                                    insensitive_settings_get_gradientStrength(self->settings),
                                    insensitive_settings_get_gradientDuration(self->settings),
                                    self->settings);
    set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
	stop_progress_indicator((InsensitiveWindow *)self->displayController);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


gboolean animationTimerEvent(gpointer user_data)
{
	InsensitiveController *self = (InsensitiveController *)user_data;
	InsensitiveSpinSystem *spinsystem = self->spinSystem;
	InsensitiveSettings *settings = self->settings;
	unsigned int dataPoints = insensitive_settings_get_dataPoints(settings);
	unsigned int steps = self->acquisitionIsInProgress ? 1 : 6;
	float delay = insensitive_settings_get_delay(settings);
	float time = insensitive_settings_get_dwellTime(settings) / steps;
	gboolean spinlock = insensitive_settings_get_spinlock(settings);

	if (!self->operationIsInProgress && (!self->haltAnimation || self->acquisitionIsInProgress)) {
		if (self->acquisitionIsInProgress) {
		    if (self->dwellTimeFraction == 0) {
			    insensitive_controller_acquire_dataPoint(self);
			    set_complex_spectrum((InsensitiveWindow *)self->displayController,
					                  self->fid,
					                  self->recordedDataPointsInFID,
					                  dataPoints);
			}
			if (self->recordedDataPointsInFID == dataPoints) {
				self->acquisitionTime = (float)g_get_monotonic_time() - self->acquisitionTime;
				if (self->spectrumReport != NULL)
					g_string_free(self->spectrumReport, TRUE);
				self->spectrumReport = insensitive_controller_create_spectrumReport(self, self->acquisitionAfterPulseSequence);
				self->acquisitionAfterPulseSequence = FALSE;
			    if (!self->acquisitionIsInProgress)
				    show_spectrumParameters_textview((InsensitiveWindow *)self->displayController, TRUE);
			}
		} else if (insensitive_spinsystem_get_firstgradientpulseissued(spinsystem)) {
			// Save the gradient action for the dwell time only
			insensitive_settings_set_delay(settings, time);
			insensitive_spinsystem_add_gradient_action(self->spinSystem, SequenceTypeEvolution, self->settings);
			insensitive_settings_set_delay(settings, delay);
		}
		if (spinlock)
			insensitive_spinsystem_switchtospinlockmode(spinsystem, TRUE);
		insensitive_spinsystem_chemicalshift(spinsystem, time, insensitive_settings_get_dephasingJitter(settings));
		if ((insensitive_settings_get_iDecoupling(settings) || insensitive_settings_get_sDecoupling(settings)) && !spinlock) {
			insensitive_spinsystem_jcoupling(spinsystem,
							                 time / 2,
							                 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
			insensitive_spinsystem_perform_decoupling(spinsystem,
								                      insensitive_settings_get_iDecoupling(settings),
								                      insensitive_settings_get_sDecoupling(settings),
								                      insensitive_settings_get_phase(settings));
			insensitive_spinsystem_jcoupling(spinsystem,
							                 time / 2,
							                 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
		} else
			insensitive_spinsystem_jcoupling(spinsystem,
							                 time,
							                 insensitive_settings_get_strongCoupling(settings) ? StrongCouplingMode : WeakCouplingMode);
		if (insensitive_settings_get_relaxationWithEvolution(settings)) {
			if (insensitive_settings_get_dipolarRelaxation(settings) && (insensitive_spinsystem_get_spins(spinsystem) > 1)) {
				if (spinlock)
					insensitive_spinsystem_transversedipolarrelaxation(spinsystem, time, insensitive_settings_get_correlationTime(settings));
				else
					insensitive_spinsystem_dipolarrelaxation(spinsystem, time, insensitive_settings_get_correlationTime(settings));
			} else {
				insensitive_spinsystem_simplerelaxation(spinsystem, time, insensitive_settings_get_T1(settings), insensitive_settings_get_T2(settings), spinlock);
			}
		}
		if (spinlock)
			insensitive_spinsystem_switchtospinlockmode(spinsystem, FALSE);
		if (!self->acquisitionIsInProgress)
			insensitive_controller_add_to_grapefruit_path(self);
		else if (insensitive_controller_get_grapefruit_path(self)->len > 0)
			insensitive_controller_initialise_grapefruit_path(self);
		g_idle_add((GSourceFunc)spin_state_was_changed, (InsensitiveWindow *)self->displayController);
	    if (++(self->dwellTimeFraction) == steps)
		    self->dwellTimeFraction = 0;
	}
	return G_SOURCE_CONTINUE;
}


/////// ///    // /////// //////   //////  //    //     //      /////// //    // /////// //      ///////
//      ////   // //      //   // //        //  //      //      //      //    // //      //      //
/////   // //  // /////   //////  //   ///   ////       //      /////   //    // /////   //      ///////
//      //  // // //      //   // //    //    //        //      //       //  //  //      //           //
/////// //   //// /////// //   //  //////     //        /////// ///////   ////   /////// /////// ///////

void insensitive_controller_calculate_energy_levels(InsensitiveController *self)
{
    InsensitiveSpinSystem *spinsystem = self->spinSystem;
    InsensitiveSettings *settings = self->settings;
    float *energyLevel;
    int *transition;
    unsigned int maxNumberOfTransitions;
    float *transitionProbability, maxTransitionProbability;
    GPtrArray *energyLevelNames;
    unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);
    unsigned int size = pow2(spins);
    unsigned int spinsInTheSystem, sizeOfTheSystem, sizeSquare;
    int levelsInCurrentSystem = 0;
    int levelsInPreviousSystems = 0;
    int eigenvalueIndex = 0;
    int *oldNumbering, *newNumbering;
    float referenceFrequency, P;
    DSPComplex *hamiltonian, z;
    DSPComplex *singleOperator;
    DSPComplex *complex_eigenvectors;
    unsigned int i, j, k, n, m, system, Ispin, Sspin;
    int numberOfISpins, numberOfSSpins;
    int numberOfMixedSystems = 0;
    unsigned int numberOfMagneticallyEquivalentSystems = 0;
    gboolean stronglyCoupled;

    // Initialize energy level vector and transition array
    maxNumberOfTransitions = size * lb(size); //size * (size - 1) / 2;
    energyLevel = malloc(size * sizeof(float));
    transition = malloc(maxNumberOfTransitions * sizeof(int));
    transitionProbability = malloc(maxNumberOfTransitions * sizeof(float));
    for (i = 0; i < maxNumberOfTransitions; i++)
        transitionProbability[i] = 1.0;
    maxTransitionProbability = 0;

    // FLT_MAX indicates an unused entry that will be ignored when drawing the level diagram
    for(i = 0; i < size; i++) {
        energyLevel[i] = FLT_MAX;
    }
    // Initialize the transition array to no transitions
    for(i = 0; i < maxNumberOfTransitions /*size * spins*/; i++)
        transition[i] = -1;

    // Determine spin systems
    GPtrArray *spinSystem1, *spinSystem2, *spinSystem3, *spinSystem4;
    int *spin1, *spin2, *spin3, *spin4;
    spin1 = malloc(sizeof(int));
    *spin1 = 1;
    spin2 = malloc(sizeof(int));
    *spin2 = 2;
    spin3 = malloc(sizeof(int));
    *spin3 = 3;
    spin4 = malloc(sizeof(int));
    *spin4 = 4;
    GPtrArray *spinArray = g_ptr_array_sized_new(spins);
    GPtrArray *spinSystemArray = g_ptr_array_sized_new(spins);
    // Initialize every spin as a one-spin system
    spinSystem1 = g_ptr_array_new();
    g_ptr_array_add(spinSystem1, spin1);
    g_ptr_array_add(spinArray, spin1);
    g_ptr_array_add(spinSystemArray, spinSystem1);
    if(spins > 1) {
        spinSystem2 = g_ptr_array_new();
        g_ptr_array_add(spinSystem2, spin2);
        g_ptr_array_add(spinArray, spin2);
        g_ptr_array_add(spinSystemArray, spinSystem2);
    } else
	  free(spin2);
    if(spins > 2) {
        spinSystem3 = g_ptr_array_new();
        g_ptr_array_add(spinSystem3, spin3);
        g_ptr_array_add(spinArray, spin3);
        g_ptr_array_add(spinSystemArray, spinSystem3);
    } else
	  free(spin3);
    if(spins > 3) {
        spinSystem4 = g_ptr_array_new();
        g_ptr_array_add(spinSystem4, spin4);
        g_ptr_array_add(spinArray, spin4);
        g_ptr_array_add(spinSystemArray, spinSystem4);
    } else
	  free(spin4);
    // Group spins according to coupling information
    // I: spin that is currently considered
    // S: coupling partner to spin I
    guint *index = malloc(sizeof(guint));
    GPtrArray *systemThatIncludesI, *systemThatIncludesS;
    for (Ispin = 0; Ispin < spins; Ispin++) {
        for (Sspin = Ispin + 1; Sspin < spins; Sspin++) {
            // If I couples to S
            if (insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, Ispin, Sspin) != 0.0) {
                // Determine the spin system that includes S
                systemThatIncludesI = NULL;
                systemThatIncludesS = NULL;
                for (k = 0; k < spins; k++) {
                    if (g_ptr_array_find(g_ptr_array_index(spinSystemArray, k), g_ptr_array_index(spinArray, Ispin), index))
                        systemThatIncludesI = g_ptr_array_index(spinSystemArray, k);
                    if (g_ptr_array_find(g_ptr_array_index(spinSystemArray, k), g_ptr_array_index(spinArray, Sspin), index))
                        systemThatIncludesS = g_ptr_array_index(spinSystemArray, k);
                }
                // If it is not the same system that also includes I
                if (systemThatIncludesI != systemThatIncludesS) {
                    // Copy all spins into one set
                    for (i = 0; i < systemThatIncludesS->len; i++)
                        g_ptr_array_add(systemThatIncludesI, g_ptr_array_index(systemThatIncludesS, i));
                    //g_ptr_array_extend(systemThatIncludesI, systemThatIncludesS, NULL, NULL); // only for glib >= 2.62
                    // And remove all spins from the second set
                    g_ptr_array_remove_range(systemThatIncludesS, 0, systemThatIncludesS->len);
                }
            }
        }
    }
    free(index);
    energyLevelNames = g_ptr_array_new();
    // The number of separate spin systems lies within 1 and spins
    // The energy eigenvalues for each spin system must be calculated separately
    GPtrArray *currentSystem;
    for (system = 0; system < spins; system++) {
        // Determine number of spins in this system and the systems before
        levelsInPreviousSystems += levelsInCurrentSystem;
        levelsInCurrentSystem = 0;
        currentSystem = g_ptr_array_index(spinSystemArray, system);
        spinsInTheSystem = currentSystem->len;
        sizeOfTheSystem = pow2(spinsInTheSystem);
        sizeSquare = sizeOfTheSystem * sizeOfTheSystem;
        // Extract spins into separate spin systems
        if (spinsInTheSystem > 0) {
            hamiltonian = malloc(sizeSquare * sizeof(DSPComplex));
            set_complex_zero_matrix(hamiltonian, sizeOfTheSystem);
            // Determine which spins belong to the system
            oldNumbering = malloc(spinsInTheSystem * sizeof(int));
            newNumbering = malloc(spinsInTheSystem * sizeof(int));
            for (i = 0; i < spinsInTheSystem; i++) {
                newNumbering[i] = i;
                oldNumbering[i] = *(int *)g_ptr_array_index(currentSystem, i);
            }
            // Determine of any two spins are strongly coupled
            numberOfISpins = 0;
            numberOfSSpins = 0;
            for(i = 0; i < spinsInTheSystem; i++) {
                if (insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[i] - 1) == spinTypeI)
                    numberOfISpins++;
                else if(insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[i] - 1) == spinTypeS)
                    numberOfSSpins++;
            }
            stronglyCoupled = FALSE;
            if (insensitive_settings_get_strongCoupling(settings) && spinsInTheSystem > 1
                && !(numberOfISpins == 1 && numberOfSSpins == 1 && spinsystem->gyroI != spinsystem->gyroS)) {
                for (i = 0; i < spinsInTheSystem; i++) {
                    for (j = i + 1; j < spinsInTheSystem; j++) {
                        if (insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[i] - 1, oldNumbering[j] - 1) != 0.0
                           && (insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[i] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[j] - 1) || spinsystem->gyroI == spinsystem->gyroS))
                            stronglyCoupled = TRUE;
                    }
                }
            }
            // Create Hamiltonian
            for (i = 0; i < spinsInTheSystem; i++) {
                // Make sure to account for I and S spins
                singleOperator = Iz(i, spinsInTheSystem);
                referenceFrequency = 500 / gyro_1H;
                referenceFrequency *= (insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[i] - 1) == spinTypeS) ? spinsystem->absGyroS : spinsystem->absGyroI;
                z = complex_rect(pow2(spinsInTheSystem - 1) * (referenceFrequency + insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[i] - 1)), 0.0);
                cblas_caxpy(sizeSquare, &z, singleOperator, 1, hamiltonian, 1);
                free(singleOperator);
                for (k = i + 1; k < spinsInTheSystem; k++) {
                    if ((insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[i] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[k] - 1) || spinsystem->gyroI == spinsystem->gyroS) && insensitive_settings_get_strongCoupling(settings))
                        singleOperator = I1I2(pow2(i), pow2(k), spinsInTheSystem);
                    else
                        singleOperator = IzSz(pow2(i), pow2(k), spinsInTheSystem);
                    z = complex_rect(M_PI * insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[i] - 1, oldNumbering[k] - 1), 0);
                    cblas_caxpy(sizeSquare, &z, singleOperator, 1, hamiltonian, 1);
                    free(singleOperator);
                }
            }
            // Diagonalize Hamiltonian
            if (stronglyCoupled) {
                __CLPK_integer size, vec_size, numberOfEigenvalues, *ISUPPZ;
                __CLPK_integer *IWORK, sizeWORK, sizeIWORK, info, zeroInt = 0;
                __CLPK_real abstol, *WORK, zeroFloat = 0.0;
                __CLPK_real *colMajorMatrix, *eigenvalues, *eigenvectors;
                char jobz = 'V', range = 'A', uplo = 'U', cmach = 'S';
                DSPComplex *aux_matrix, zalpha, zbeta;

                size = sizeOfTheSystem;
                vec_size = size * size;
                // Convert matrix to column major form
                colMajorMatrix = malloc(vec_size * sizeof(__CLPK_real));
                for (m = 0; m < size; m++)
                    for (n = 0; n < size; n++)
                        colMajorMatrix[n * size + m] = hamiltonian[n * size + m].real;
                // Solve eigenvalue equation
                eigenvalues = malloc(size * sizeof(__CLPK_real));
                eigenvectors = malloc(vec_size * sizeof(__CLPK_real));
                complex_eigenvectors = malloc(vec_size * sizeof(DSPComplex));
                abstol = slamch_(&cmach);
                ISUPPZ = malloc(2 * size * sizeof(__CLPK_integer));
                WORK = malloc(26 * size * sizeof(__CLPK_real));
                IWORK = malloc(10 * size * sizeof(__CLPK_integer));
                sizeWORK = -1; sizeIWORK = -1;
                ssyevr_(&jobz, &range, &uplo, &size, colMajorMatrix, &size,
                        &zeroFloat, &zeroFloat, &zeroInt, &zeroInt, &abstol,
                        &numberOfEigenvalues, eigenvalues, eigenvectors, &size,
                        ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
                sizeWORK = (__CLPK_integer)WORK[0]; sizeIWORK = IWORK[0];
                free(WORK); free(IWORK);
                WORK = malloc(sizeWORK * sizeof(__CLPK_real));
                IWORK = malloc(sizeIWORK * sizeof(__CLPK_integer));
                ssyevr_(&jobz, &range, &uplo, &size, colMajorMatrix, &size,
                        &zeroFloat, &zeroFloat, &zeroInt, &zeroInt, &abstol,
                        &numberOfEigenvalues, eigenvalues, eigenvectors, &size,
                        ISUPPZ, WORK, &sizeWORK, IWORK, &sizeIWORK, &info);
                free(ISUPPZ); free(WORK); free(IWORK);
                /* eigen_symmv(colMajorMatrix, eigenvalues, eigenvectors, size); */
                free(colMajorMatrix); free(eigenvalues);
                // Mirror eigenvectors at horizontal axis and revert to complex values
                for (m = 0; m < (unsigned int)size; m++)
                    for (n = 0; n < (unsigned int)size; n++)
                        complex_eigenvectors[n * size + m] = complex_rect(eigenvectors[(size - 1 - n) * size + m], 0);
                free(eigenvectors);
                // Perform matrix transformation D = X^-1.A.X for hermitian A (inverse equals transpose)
                zalpha = complex_rect(1, 0);
                zbeta = complex_rect(0, 0);
                aux_matrix = malloc(vec_size * sizeof(DSPComplex));
                // aux_matrix = operator.eigenvectors
                cblas_cgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, size, size, size,
                            &zalpha, hamiltonian, size, complex_eigenvectors, size, &zbeta, aux_matrix, size);
                // hamiltonian = transpose_eigenvectors.aux_matrix
                cblas_cgemm(CblasColMajor, CblasConjTrans, CblasNoTrans, size, size, size,
                            &zalpha, complex_eigenvectors, size, aux_matrix, size, &zbeta, hamiltonian, size);
                free(aux_matrix);
            }
            // Organize levels and transitions
            for (i = 0; i < sizeOfTheSystem; i++) {
                // Save energy level eigenvalues
                energyLevel[eigenvalueIndex] = hamiltonian[i * sizeOfTheSystem + i].real;
                if (energyLevel[eigenvalueIndex] != FLT_MAX)
                    levelsInCurrentSystem++;

                // Save transitions
                int t = 0.0;
                for (j = i + 1; j < sizeOfTheSystem; j++) {
                    if (stronglyCoupled) {
                        P = 0.0;
                        // Transition probability is proportional to |<φ|Ix|ψ>|²
                        // Signal intensity is proportional to 1 ± sin(2ξ)
                        for (m = 0; m < sizeOfTheSystem; m++) {
                            for (n = 0; n < sizeOfTheSystem; n++) {
                                if (test_for_simple_coherence(n, m))
                                    P += complex_eigenvectors[j * sizeOfTheSystem + n].real * complex_eigenvectors[i * sizeOfTheSystem + m].real;
                            }
                        }
                        P = fabsf(P);
                        if (P > maxTransitionProbability)
                            maxTransitionProbability = P;
                        if (P >= 0.0001) {
                            transition[eigenvalueIndex * spins + t] = j + levelsInPreviousSystems;
                            transitionProbability[eigenvalueIndex * spins + t] = P;
                            // Increment counter for transition
                            t++;
                        }
                    } else {
                        if (test_for_simple_coherence(i, j)) {
                            transition[eigenvalueIndex * spins + t] = j + levelsInPreviousSystems;
                            transitionProbability[eigenvalueIndex * spins + t] = 1.0;
                            // Increment counter for transition
                            t++;
                        }
                    }
                }
                // Increment counter for the next eigenvalue
                eigenvalueIndex++;
            }
            // Organize labels
            gchar *levelNameString, *tempStr;
            // Two magnetically equivalent spins
            if(insensitive_settings_get_strongCoupling(settings) && spinsInTheSystem == 2
               && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[1] - 1)
               && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[1] - 1)) {
                // Identify largest and second largest level
                float highestLevel = 0;
                float secondHighestLevel = 0;
                gboolean lowerTripletStateFound = FALSE;
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > highestLevel)
                        highestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > secondHighestLevel
                       && hamiltonian[i * sizeOfTheSystem + i].real < highestLevel)
                        secondHighestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                // Add labels for singlet/triplet basis
                for (i = 0; i < sizeOfTheSystem; i++) {
                    levelNameString = malloc(56 * sizeof(gchar));
                    if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - highestLevel) < 0.1) {
                        strcpy(levelNameString, "T₋₁");
                    } else if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - secondHighestLevel) < 0.1) {
                        strcpy(levelNameString, "T₀");
                    } else if (!lowerTripletStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (2 * secondHighestLevel - highestLevel)) < 0.1) {
                        strcpy(levelNameString, "T₊₁");
                        lowerTripletStateFound = TRUE;
                    } else {
                        strcpy(levelNameString, "S₀");
                    }
                    for (n = 0; n < numberOfMagneticallyEquivalentSystems; n++)
                        strcat(levelNameString, "'");
                    g_ptr_array_add(energyLevelNames, levelNameString);
                    /*[levelNameString release];*/
                }
                numberOfMagneticallyEquivalentSystems++;
            // Three magnetically equivalent spins
            } else if (insensitive_settings_get_strongCoupling(settings) && spinsInTheSystem == 3
                       && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[1] - 1)
                       && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[1] - 1)
                       && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[1] - 1, oldNumbering[2] - 1)) {
                // Identify largest and second largest level
                float highestLevel = 0;
                float secondHighestLevel = 0;
                gboolean thirdQuartetStateFound = FALSE;
                gboolean fourthQuartetStateFound = FALSE;
                unsigned int doubletStatesFound = 0;
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > highestLevel)
                        highestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > secondHighestLevel
                       && hamiltonian[i * sizeOfTheSystem + i].real < highestLevel)
                        secondHighestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                // Add labels for doublet/quartet basis
                for (i = 0; i < sizeOfTheSystem; i++) {
                    levelNameString = malloc(56 * sizeof(gchar));
                    if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - highestLevel) < 0.1) {
                        strcpy(levelNameString, "|³⁄₂, −³⁄₂⟩");
                    } else if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - secondHighestLevel) < 0.1) {
                        strcpy(levelNameString, "|³⁄₂, −¹⁄₂⟩");
                    } else if (!thirdQuartetStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (2 * secondHighestLevel - highestLevel)) < 0.1) {
                        strcpy(levelNameString, "|³⁄₂, ¹⁄₂⟩");
                        thirdQuartetStateFound = TRUE;
                    } else if (!fourthQuartetStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (-2 * highestLevel + 3 * secondHighestLevel)) < 0.1) {
                        strcpy(levelNameString, "|³⁄₂, ³⁄₂⟩");
                        fourthQuartetStateFound = TRUE;
                    } else {
                        if (doubletStatesFound < 2)
                            strcpy(levelNameString, "|¹⁄₂, −¹⁄₂⟩");
                        else
                            strcpy(levelNameString, "|¹⁄₂, ¹⁄₂⟩");
                        doubletStatesFound++;
                    }
                    g_ptr_array_add(energyLevelNames, levelNameString);
                    /*[levelNameString release];*/
                }
                numberOfMagneticallyEquivalentSystems++;
            // Four magnetically equivalent spins
            } else if (insensitive_settings_get_strongCoupling(settings) && spinsInTheSystem == 4
                       && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[1] - 1)
                       && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, oldNumbering[3] - 1)
                       && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[1] - 1)
                       && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[0] - 1) == insensitive_spinsystem_get_spintype_for_spin(spinsystem, oldNumbering[3] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[3] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[1] - 1, oldNumbering[2] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[1] - 1, oldNumbering[3] - 1)
                       && insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[0] - 1, oldNumbering[1] - 1) == insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, oldNumbering[2] - 1, oldNumbering[3] - 1)) {
                // Identify largest and second largest level
                float highestLevel = 0;
                float secondHighestLevel = 0;
                gboolean thirdQuintetStateFound = FALSE;
                gboolean fourthQuintetStateFound = FALSE;
                gboolean fifthQuintetStateFound = FALSE;
                unsigned int otherStatesFound = 0;
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > highestLevel)
                        highestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                for (i = 0; i < sizeOfTheSystem; i++) {
                    if (hamiltonian[i * sizeOfTheSystem + i].real > secondHighestLevel
                       && hamiltonian[i * sizeOfTheSystem + i].real < highestLevel)
                        secondHighestLevel = hamiltonian[i * sizeOfTheSystem + i].real;
                }
                // Add labels for quintet/triplet/singlet basis
                for (i = 0; i < sizeOfTheSystem; i++) {
                    levelNameString = malloc(56 * sizeof(gchar));
                    if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - highestLevel) < 0.1) {
                        strcpy(levelNameString, "|2, −2⟩");
                    } else if (fabsf(hamiltonian[i * sizeOfTheSystem + i].real - secondHighestLevel) < 0.1) {
                        strcpy(levelNameString, "|2, −1⟩");
                    } else if (!thirdQuintetStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (2 * secondHighestLevel - highestLevel)) < 0.1) {
                        strcpy(levelNameString, "|2, 0⟩");
                        thirdQuintetStateFound = TRUE;
                    } else if(!fourthQuintetStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (-2 * highestLevel + 3 * secondHighestLevel)) < 0.1) {
                        strcpy(levelNameString, "|2, 1⟩");
                        fourthQuintetStateFound = TRUE;
                    } else if(!fifthQuintetStateFound && fabsf(hamiltonian[i * sizeOfTheSystem + i].real - (-3 * highestLevel + 4 * secondHighestLevel)) < 0.1) {
                        strcpy(levelNameString, "|2, 2⟩");
                        fifthQuintetStateFound = TRUE;
                    } else {
                        if(otherStatesFound < 3)
                            strcpy(levelNameString, "|1, −1⟩");
                        else if(otherStatesFound < 6)
                            strcpy(levelNameString, "|1, 0⟩");
                        else if(otherStatesFound < 8)
                            strcpy(levelNameString, "|0, 0⟩");
                        else
                            strcpy(levelNameString, "|1, 1⟩");
                       otherStatesFound++;
                    }
                    g_ptr_array_add(energyLevelNames, levelNameString);
                    /*[levelNameString release];*/
                }
                numberOfMagneticallyEquivalentSystems++;
            // Magnetically inequivalent spins
            } else {
                int character;
                levelNameString = malloc(56 * sizeof(gchar));
                tempStr = malloc(5 * sizeof(gchar));
                for (i = 0; i < sizeOfTheSystem; i++) {
                    // Save energy level names
                    strcpy(levelNameString, "");
                    if (stronglyCoupled) {
                        // Mixed levels are cos(ξ/2)|αβ> + sin(ξ/2)|βα> and cos(ξ/2)|βα> - sin(ξ/2)|αβ>
                        if (numberOfMixedSystems == 0) {
                            sprintf(tempStr, "ψ%d", sizeOfTheSystem - i);
                            strcat(levelNameString, tempStr);
                        } else {
                            sprintf(tempStr, "φ%d", sizeOfTheSystem - i);
                            strcat(levelNameString, tempStr);
                        }
                    } else {
                        for (character = spinsInTheSystem - 1; character >= 0; character--)
                            if ((i & pow2(character)) >> character == 1) {
                                sprintf(tempStr, "α%d", oldNumbering[-1 * (character - spinsInTheSystem + 1)]);
                                strcat(levelNameString, tempStr);
                            } else {
                                sprintf(tempStr, "β%d", oldNumbering[-1 * (character - spinsInTheSystem + 1)]);
                                strcat(levelNameString, tempStr);
                            }
                    }
                    g_ptr_array_add(energyLevelNames, replace_numbers_by_indices(levelNameString));
                }
                free(levelNameString);
                free(tempStr);
                if(stronglyCoupled)
                    numberOfMixedSystems++;
            }
            free(hamiltonian);
            free(oldNumbering);
            free(newNumbering);
            if(stronglyCoupled) {
                free(complex_eigenvectors);
            }
        }
    }
    for (i = 0; i < maxNumberOfTransitions; i++)
        transitionProbability[i] /= maxTransitionProbability;

    set_energy_values((InsensitiveWindow *)self->displayController,
                      energyLevel, size, energyLevelNames, transition, transitionProbability);

    free(transition);
    free(transitionProbability);
    free(energyLevel);
}


/* - (void)replaceNumbersByExponentsInString:(NSMutableString *)string */
/* { */
/*     [string replaceOccurrencesOfString:@"0" */
/*                             withString:@"⁰" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"1" */
/*                             withString:@"¹" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"2" */
/*                             withString:@"²" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"3" */
/*                             withString:@"³" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"4" */
/*                             withString:@"⁴" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"5" */
/*                             withString:@"⁵" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"6" */
/*                             withString:@"⁶" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"7" */
/*                             withString:@"⁷" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"8" */
/*                             withString:@"⁸" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/*     [string replaceOccurrencesOfString:@"9" */
/*                             withString:@"⁹" */
/*                                options:NSLiteralSearch */
/*                                  range:NSMakeRange(0, [string length])]; */
/* } */


/* - (void)formatIrreducibleSphericalTensorString:(NSMutableString *)string */
/* { */
/*     // T[l,m] {spins,...} */
/*     // Set script for the brackets as follows: */
/*     // [l,m]       in subscript w/o brackets */
/*     // {spin,...}  leave as is but add space */

/*     NSCharacterSet *brackets = [NSCharacterSet characterSetWithCharactersInString:@"[]{}"]; */
/*     NSArray *components = [string componentsSeparatedByCharactersInSet:brackets]; */
/*     NSMutableString *tempString; */

/*     if([components count] == 1) { */
/*         [string setString:[NSMutableString stringWithString:[components objectAtIndex:0]]]; */
/*     } else { */
/*         [string setString:[NSMutableString stringWithString:@"T"]]; */
/*     } */
/*     if([components count] == 3) { */
/*         tempString = [NSMutableString stringWithString:[components objectAtIndex:1]]; */
/*         [self replaceNumbersByIndicesInString:tempString]; */
/*         [string appendFormat:@"%s", tempString]; */
/*     } */
/*     if([components count] == 5) { */
/*         tempString = [NSMutableString stringWithString:[components objectAtIndex:1]]; */
/*         [self replaceNumbersByIndicesInString:tempString]; */
/*         [string appendFormat:@"%s", tempString]; */
/*         [string appendFormat:@"{%s}", [components objectAtIndex:3]]; */
/*     } */
/* } */


/* - (NSString *)irreducibleSphericalTensorStringForNumberOfSpins:(unsigned int)spins array:(unsigned int)array */
/*                                                           rank:(unsigned int)l order:(int)m */
/* { */
/*     NSMutableString *output = [NSMutableString stringWithUTF8String:irreducible_spherical_tensor_label(spins, array, l, m)]; */
/*     [self formatIrreducibleSphericalTensorString:output]; */

/*     return [NSString stringWithString:output]; */
/* } */



//////  //    // //      /////// ///////     /////// ///////  //////  //    // /////// ///    //  ////// ///////
//   // //    // //      //      //          //      //      //    // //    // //      ////   // //      //
//////  //    // //      /////// /////       /////// /////   //    // //    // /////   // //  // //      /////
//      //    // //           // //               // //      // // // //    // //      //  // // //      //
//       //////  /////// /////// ///////     /////// ///////  //////   //////  /////// //   ////  ////// ///////
                                                                 //

gboolean insensitive_controller_get_isRecordingPulseSequence(InsensitiveController *self)
{
    return self->isRecordingPulseSequence;
}


void insensitive_controller_set_isRecordingPulseSequence(InsensitiveController *self, gboolean value)
{
    self->isRecordingPulseSequence = value;
}


unsigned int insensitive_controller_get_numberOfPulseSequenceElements(InsensitiveController *self)
{
    return insensitive_pulsesequence_get_number_of_elements(self->pulseSequence);
}


unsigned int insensitive_controller_get_numberOfPhaseCycles(InsensitiveController *self)
{
    return self->phaseCycles;
}


void insensitive_controller_add_number_of_phase_cycles(InsensitiveController *self, int number)
{
	int i, j, len, number_of_columns;
	gchar *phaseString;

    number_of_columns = self->pulseList->len + 1;
	// Number positive? Then append rows
	if (number > 0) {
        // Should fill with phase shifts of the last row
        for (i = 0; i < number * number_of_columns; i++) {
            phaseString = malloc(5 * sizeof(gchar));
            strcpy(phaseString, "0");
			g_ptr_array_add(self->phaseCyclingArray, phaseString);
        }
	// Number negative? Then delete rows
	} else if (number < 0) {
		len = -number * number_of_columns;
        g_ptr_array_remove_range(self->phaseCyclingArray, self->phaseCyclingArray->len - len, len);
	}
    self->phaseCycles += number;
	update_phaseCyclingTable((InsensitiveWindow *)self->displayController, number_of_columns);
}


InsensitivePulseSequence *insensitive_controller_get_pulseSequence(InsensitiveController *self)
{
    return self->pulseSequence;
}


void insensitive_controller_substitute_pulseSequence(InsensitiveController *self, GPtrArray *array)
{
    unsigned int i;
    SequenceElement *element;

    insensitive_pulsesequence_erase_sequence(self->pulseSequence);
    g_ptr_array_remove_range(self->pulseList, 0, self->pulseList->len);
    for (i = 0; i < array->len; i++) {
        element = g_ptr_array_index(array, i);
        insensitive_pulsesequence_add_sequence_element(self->pulseSequence, element);
        if (element->type == SequenceTypePulse)
            g_ptr_array_add(self->pulseList, insensitive_pulsesequence_get_last_element(self->pulseSequence));
    }
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
    update_evolutionTimes_combobox((InsensitiveWindow *)self->displayController);
    update_pulseSequence((InsensitiveWindow *)self->displayController); // is commented out in Xcode
}


GPtrArray *insensitive_controller_get_phaseCyclingArray(InsensitiveController *self)
{
    return self->phaseCyclingArray;
}


GPtrArray *insensitive_controller_get_pulseList(InsensitiveController *self)
{
    return self->pulseList;
}


void insensitive_controller_substitute_phaseCyclingArray(InsensitiveController *self, GPtrArray *array, unsigned int numberOfCycles)
{
    unsigned int i;
    gchar *phaseString;

    reset_phaseCyclingTable((InsensitiveWindow *)self->displayController);
    for (i = 0; i < self->pulseList->len; i++)
        insert_column_into_phaseCyclingTable((InsensitiveWindow *)self->displayController, i + 2, 1);
    self->phaseCycles = numberOfCycles;
    g_ptr_array_remove_range(self->phaseCyclingArray, 0, self->phaseCyclingArray->len);
    for (i = 0; i < array->len; i++) {
        phaseString = malloc(5 * sizeof(gchar));
        strcpy(phaseString, g_ptr_array_index(array, i));
        g_ptr_array_add(self->phaseCyclingArray, phaseString);
    }
    set_phaseCycling_combobox((InsensitiveWindow *)self->displayController, numberOfCycles);
    update_phaseCyclingTable((InsensitiveWindow *)self->displayController, self->pulseList->len + 1/*2*/);
    // Update Phase Cycling ComboBox
    // Does not really work with phase cycling
}


void insensitive_controller_toggle_recordingPulseSequence(InsensitiveController *self)
{
    self->isRecordingPulseSequence = !self->isRecordingPulseSequence;
    set_recording_button_clicked((InsensitiveWindow *)self->displayController, self->isRecordingPulseSequence);
    if (self->isRecordingPulseSequence) {
        insensitive_controller_erase_pulseSequence(self);
        insensitive_controller_set_animates(self, FALSE);
        enable_animation_checkbox((InsensitiveWindow *)self->displayController, FALSE);
    } else
        enable_animation_checkbox((InsensitiveWindow *)self->displayController, TRUE);
    if (!self->operationIsInProgress)
        set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
}


int insensitive_controller_get_variableEvolutionTime(InsensitiveController *self)
{
    return self->indexForVariableEvolutionTime;
}


void insensitive_controller_set_variableEvolutionTime(InsensitiveController *self, int value)
{
    self->indexForVariableEvolutionTime = value;
    set_variable_evolution_time((InsensitiveWindow *)self->displayController, self->indexForVariableEvolutionTime);
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
}


gboolean insensitive_controller_get_pulseSequence_ends_with_acquisition(InsensitiveController *self)
{
    SequenceElement *fidElement;

    if(insensitive_pulsesequence_get_number_of_elements(self->pulseSequence) > 0) {
        fidElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
        if(fidElement->type == SequenceTypeFID)
            return TRUE;
    }
    return FALSE;
}


void insensitive_controller_erase_pulseSequence(InsensitiveController *self)
{
    unsigned int step;
    gchar *phaseString;

    // Pulse sequence
    insensitive_pulsesequence_erase_sequence(self->pulseSequence);
    g_ptr_array_remove_range(self->pulseList, 0, self->pulseList->len);
    self->indexForVariableEvolutionTime = 0;
    set_variable_evolution_time((InsensitiveWindow *)self->displayController, self->indexForVariableEvolutionTime);
    update_evolutionTimes_combobox((InsensitiveWindow *)self->displayController);
    update_pulseSequence((InsensitiveWindow *)self->displayController);
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
    // Phase cycling
    g_ptr_array_remove_range(self->phaseCyclingArray, 0, self->phaseCyclingArray->len);
    for (step = 0; step < self->phaseCycles; step++) {
        phaseString = malloc(5 * sizeof(gchar));
        strcpy(phaseString, "0");
        g_ptr_array_add(self->phaseCyclingArray, phaseString);
    }
    reset_phaseCyclingTable((InsensitiveWindow *)self->displayController);
    if (self->pulseSequenceName != NULL) {
        g_free(self->pulseSequenceName);
        self->pulseSequenceName = NULL;
    }
}


void insensitive_controller_perform_pulseSequence(InsensitiveController *self)
{
    SequenceElement *element;
    SequenceElement *fidElement;
    unsigned int i, phaseCyclingIndex = 0;

    if (insensitive_pulsesequence_get_number_of_elements(self->pulseSequence) > 0 && !self->acquisitionIsInProgress) {
        self->currentSpectrumIsTwoDimensional = FALSE;
        insensitive_controller_set_currentStepInPulseSequence(self, 0);
        self->expno++;
        set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
        start_progress_indicator((InsensitiveWindow *)self->displayController);
        insensitive_controller_save_previous_state(self);
        disable_acquireAfterNextPulse((InsensitiveWindow *)self->displayController);
        // Do not show acquisition if phase cycling must be performed
        fidElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
        if (fidElement->type == SequenceTypeFID) {
            set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
            set_display_frequency_domain((InsensitiveWindow *)self->displayController, FALSE);
            go_to_fft_panel((InsensitiveWindow *)self->displayController);
            insensitive_controller_reset_acquisition_for_dataPoints(self, insensitive_settings_get_dataPoints(self->settings));
            reset_window_function((InsensitiveWindow *)self->displayController);
            self->relaxationWasIncludedBefore = insensitive_settings_get_relaxationWithEvolution(self->settings);
            self->previousPulseArray = insensitive_settings_get_pulseArray(self->settings);
            insensitive_controller_save_decoupling(self);
            insensitive_controller_set_IDecoupling(self, fidElement->iDecoupling);
            insensitive_controller_set_SDecoupling(self, fidElement->sDecoupling);
            insensitive_controller_set_relaxation_with_evolution(self, TRUE);
            insensitive_controller_set_animates(self, FALSE);
            enable_animation_checkbox((InsensitiveWindow *)self->displayController, FALSE);
            set_acquisition_is_running((InsensitiveWindow *)self->displayController, TRUE);
            self->acquisitionIsInProgress = TRUE;
            self->acquisitionAfterPulseSequence = TRUE;
            self->acquisitionTime = (float)g_get_monotonic_time();
            // Perform Phase cycling for 1D spectrum silently
            if (self->phaseCycles > 1) {
                enable_acquisition_button((InsensitiveWindow *)self->displayController, FALSE);
                set_acquisition_in_background((InsensitiveWindow *)self->displayController, TRUE);
                g_thread_new("PulseSequenceThread", insensitive_controller_perform_pulseSequence_in_background, self);
            // Perform single acquisition with animation
            } else {
                phaseCyclingIndex++;
                for (i = 0; i < self->pulseList->len; i++) {
                    element = g_ptr_array_index(self->pulseList, i);
                    element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex++));
                }
                insensitive_spinsystem_return_to_thermal_equilibrium(self->spinSystem);
                insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence, self->spinSystem, 0, 0, self->settings, self);
                set_complex_spectrum((InsensitiveWindow *)self->displayController,
                                     self->fid,
                                     insensitive_settings_get_dataPoints(self->settings),
                                     insensitive_settings_get_dataPoints(self->settings));
                spin_state_was_changed((InsensitiveWindow *)self->displayController);
                stop_progress_indicator((InsensitiveWindow *)self->displayController);
                set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);

                // Make sure that the correct pulse settings are displayed
                set_flipAngle((InsensitiveWindow *)self->displayController, insensitive_settings_get_flipAngle(self->settings));
                set_phase((InsensitiveWindow *)self->displayController, insensitive_settings_get_phase(self->settings));
                insensitive_settings_set_pulseArray(self->settings,
                                                    self->previousPulseArray,
                                                    insensitive_spinsystem_get_spins(self->spinSystem),
                                                    insensitive_spinsystem_get_spintypearray(self->spinSystem));
                set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
                set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
                set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
                set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
            }
        } else {
            insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence, self->spinSystem, 0, 0, self->settings, self);
            spin_state_was_changed((InsensitiveWindow *)self->displayController);
            stop_progress_indicator((InsensitiveWindow *)self->displayController);
            set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
            show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 1);
        }
    }
}


gboolean insensitive_controller_perform_pulseSequence_in_background(gpointer data)
{
    InsensitiveController *self = (InsensitiveController *)data;
    SequenceElement *element, *fidElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
    float factor = 2 * insensitive_spinsystem_get_spins(self->spinSystem);
    unsigned int i, pulse, spin, dataPoint, dataPoints, phaseCyclingIndex = 0;
    DSPComplex z, receiverPhase;
    DSPSplitComplex accumulatedFID;

    self->currentSpectrumIsTwoDimensional = FALSE;
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
    self->previousPulseArray = insensitive_settings_get_pulseArray(self->settings);
    dataPoints = insensitive_settings_get_dataPoints(self->settings);
    accumulatedFID.realp = malloc(dataPoints * sizeof(float));
    accumulatedFID.imagp = malloc(dataPoints * sizeof(float));
    for (i = 0; i < dataPoints; i++) {
        accumulatedFID.realp[i] = 0;
        accumulatedFID.imagp[i] = 0;
    }
    for (i = 0; i < self->phaseCycles; i++) {
        insensitive_spinsystem_return_to_thermal_equilibrium(self->spinSystem);
        receiverPhase = complex_rect(cos(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI),
                                     -sin(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI));
        phaseCyclingIndex++;
        for (pulse = 0; pulse < self->pulseList->len; pulse++) {
            element = g_ptr_array_index(self->pulseList, pulse);
            element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex++));
        }
        insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence, self->spinSystem, 0, 0, self->settings, NULL);
        for (dataPoint = 0; dataPoint < dataPoints; dataPoint++) {
            if (!insensitive_settings_get_zeroFilling(self->settings) || dataPoint <= dataPoints / 2) {
                // Acquire data point
                z.real = 0;
                z.imag = 0;
                for (spin = 0; spin < insensitive_spinsystem_get_spins(self->spinSystem); spin++)
                    if (((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin) == spinTypeI) && fidElement->activeISpins)
                       || ((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin) == spinTypeS) && fidElement->activeSSpins)) {
                        z.real -= insensitive_spinsystem_get_expectationvalue_y_for_spin(self->spinSystem, spin) / factor;
                        z.imag += insensitive_spinsystem_get_expectationvalue_x_for_spin(self->spinSystem, spin) / factor;
                    }
                z = complex_mul(z, receiverPhase);
                accumulatedFID.realp[dataPoint] += z.real;
                accumulatedFID.imagp[dataPoint] += z.imag;
                // Free evolution
                if (insensitive_settings_get_spinlock(self->settings))
                    insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
                insensitive_spinsystem_chemicalshift(self->spinSystem,
                                                     insensitive_settings_get_dwellTime(self->settings),
                                                     insensitive_settings_get_dephasingJitter(self->settings));
                if (insensitive_spinsystem_get_spins(self->spinSystem) > 1) {
                    if((insensitive_settings_get_iDecoupling(self->settings) || insensitive_settings_get_sDecoupling(self->settings)) && !insensitive_settings_get_spinlock(self->settings)) {
                        insensitive_spinsystem_jcoupling(self->spinSystem,
                                                         insensitive_settings_get_dwellTime(self->settings) / 2,
                                                         insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                        insensitive_spinsystem_perform_decoupling(self->spinSystem,
                                                                  insensitive_settings_get_iDecoupling(self->settings),
                                                                  insensitive_settings_get_sDecoupling(self->settings),
                                                                  insensitive_settings_get_phase(self->settings));
                        insensitive_spinsystem_jcoupling(self->spinSystem,
                                                         insensitive_settings_get_dwellTime(self->settings) / 2,
                                                         insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                    } else
                        insensitive_spinsystem_jcoupling(self->spinSystem,
                                                         insensitive_settings_get_dwellTime(self->settings),
                                                         insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                }
                if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
		            if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			            if (insensitive_settings_get_spinlock(self->settings))
				            insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem,
										                                       insensitive_settings_get_dwellTime(self->settings),
										                                       insensitive_settings_get_correlationTime(self->settings));
			            else
				            insensitive_spinsystem_dipolarrelaxation(self->spinSystem,
									                                 insensitive_settings_get_dwellTime(self->settings),
									                                 insensitive_settings_get_correlationTime(self->settings));
		            } else
			            insensitive_spinsystem_simplerelaxation(self->spinSystem,
								                                insensitive_settings_get_dwellTime(self->settings),
							                                	insensitive_settings_get_T1(self->settings),
							                                	insensitive_settings_get_T2(self->settings),
							                                	insensitive_settings_get_spinlock(self->settings));
	            }
                if (insensitive_settings_get_spinlock(self->settings))
                    insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
            }
        }
    }
    // Reset pulse sequence phases to first phase cycle
    phaseCyclingIndex = 1;
    for (pulse = 0; pulse < self->pulseList->len; pulse++) {
        element = g_ptr_array_index(self->pulseList, pulse);
        element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex++));
    }
    self->recordedDataPointsInFID = dataPoints;
    free(self->fid.realp);
    free(self->fid.imagp);
    self->fid.realp = accumulatedFID.realp;
    self->fid.imagp = accumulatedFID.imagp;
    gdk_threads_add_idle((GSourceFunc)insensitive_controller_finish_perform_pulseSequence, self);

	return FALSE;
}


gboolean insensitive_controller_finish_perform_pulseSequence(gpointer data)
{
    InsensitiveController *self = (InsensitiveController *)data;

    self->acquisitionTime = (float)g_get_monotonic_time() - self->acquisitionTime;
    if(self->spectrumReport != NULL)
        g_string_free(self->spectrumReport, FALSE);
    self->spectrumReport = insensitive_controller_create_spectrumReport(self, TRUE);

    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->fid,
                         insensitive_settings_get_dataPoints(self->settings),
                         insensitive_settings_get_dataPoints(self->settings));
    insensitive_controller_stop_acquisition(self);
    if (insensitive_settings_get_playSoundAfterAcquisition(self->settings))
        play_sound((InsensitiveWindow *)self->displayController);
    enable_fft_along_t1((InsensitiveWindow *)self->displayController, TRUE);
    insensitive_controller_restore_relaxation_with_evolution(self);
    insensitive_controller_restore_decoupling(self);
    show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);

    spin_state_was_changed((InsensitiveWindow *)self->displayController);
    stop_progress_indicator((InsensitiveWindow *)self->displayController);
    set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
    set_acquisition_is_running((InsensitiveWindow *)self->displayController, FALSE);
    enable_acquisition_button((InsensitiveWindow *)self->displayController, TRUE);
    set_acquisition_in_background((InsensitiveWindow *)self->displayController, FALSE);

    // Make sure that the correct pulse settings are displayed
    set_flipAngle((InsensitiveWindow *)self->displayController, insensitive_settings_get_flipAngle(self->settings));
    set_phase((InsensitiveWindow *)self->displayController, insensitive_settings_get_phase(self->settings));
    insensitive_settings_set_pulseArray(self->settings,
                                        self->previousPulseArray,
                                        insensitive_spinsystem_get_spins(self->spinSystem),
                                        insensitive_spinsystem_get_spintypearray(self->spinSystem));
    set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
    set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
    set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
    set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));

    show_spectrumParameters_textview((InsensitiveWindow *)self->displayController, TRUE);

	return FALSE;
}


unsigned int insensitive_controller_get_currentStepInPulseSequence(InsensitiveController *self)
{
    return self->currentStepInPulseSequence;
}


void insensitive_controller_set_currentStepInPulseSequence(InsensitiveController *self, int value)
{
    self->currentStepInPulseSequence = (value >= insensitive_pulsesequence_get_number_of_elements(self->pulseSequence)) ? 0 : value;
    set_current_step_in_pulseSequence((InsensitiveWindow *)self->displayController, self->currentStepInPulseSequence);
}


void insensitive_controller_perform_next_step_of_pulseSequence(InsensitiveController *self)
{
    unsigned int i, phaseCyclingIndex;
    SequenceElement *element;

    if (insensitive_pulsesequence_get_number_of_elements(self->pulseSequence) > 0 && !self->acquisitionIsInProgress) {
        insensitive_controller_save_previous_state(self);
        phaseCyclingIndex = 1;
        for( i = 0; i < self->pulseList->len; i++) {
            element = g_ptr_array_index(self->pulseList, i);
            element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex++));
        }
        element = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, self->currentStepInPulseSequence);
        if(element->type != SequenceTypeFID) {
            insensitive_controller_initialise_grapefruit_path(self);

            insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence,
                                                                    self->spinSystem,
                                                                    self->currentStepInPulseSequence,
                                                                    1,
                                                                    self->settings,
                                                                    self);
            spin_state_was_changed((InsensitiveWindow *)self->displayController);
            stop_progress_indicator((InsensitiveWindow *)self->displayController);
            set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
            insensitive_controller_set_currentStepInPulseSequence(self, self->currentStepInPulseSequence + 1);
            show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 1);
        } else {
            insensitive_controller_set_currentStepInPulseSequence(self, 0);
            insensitive_controller_set_pulseBeforeAcquisition(self, FALSE);
            if(element->spinlock) {
                insensitive_controller_set_spinlock(self, TRUE);
            } else {
                insensitive_controller_set_spinlock(self, FALSE);
                insensitive_controller_set_IDecoupling(self, element->iDecoupling);
                insensitive_controller_set_SDecoupling(self, element->sDecoupling);
            }
            insensitive_controller_set_detectISignal(self, element->activeISpins);
            insensitive_controller_set_detectSSignal(self, element->activeSSpins);
            insensitive_controller_perform_acquisition(self);
        }
    }
}


GString *insensitive_controller_export_pulseSequence(InsensitiveController *self, gchar *name)
{
	unsigned int i, j, cycle, pulse;
	unsigned int pulseNumber = 1, delayNumber = 2, gradientNumber = 1, numberOfDelays = 0;
	int delayOccurredBefore, gradientOccurredBefore, delayIndex = 1;
	GString *pp = g_string_sized_new(710);
	GString *pulseCode = g_string_sized_new(64);
	gchar *shapedPulseString, *shapedPulseFile, *delayLabel, *newLabel;
	gchar *shapedPulsePrefix, *shapedPulseSuffix;
	SequenceElement *firstElement, *lastElement, *secondElement, *thirdElement, *element, *nextDelay;
	gboolean includeGradients = FALSE, variableDelayIsEcho = FALSE, identifiedNOESY = FALSE;
	gboolean f1Decoupling = FALSE, f2Decoupling = FALSE, spinlock = FALSE, f2_is_used = FALSE;
	gboolean longF1Decoupling = FALSE, longF2Decoupling = FALSE, nextElementDecouples;
	gboolean d2Present = FALSE, d3Present = FALSE, d4Present = FALSE, d11Present = FALSE;
	unsigned int d6Present = 0;
	gboolean checkForD2 = TRUE, checkForD4 = TRUE, checkForD3 = TRUE;//, checkForD6 = TRUE;
	gboolean cnstHH = FALSE, cnstHX = FALSE, cnstXX = FALSE, identifiedInversionRecovery = FALSE;
	gboolean contains180Pulse_f1 = FALSE, contains180Pulse_f2 = FALSE, shapedPulse90 = FALSE, shapedPulse180 = FALSE, detectsISpins;
	GPtrArray *delayList = g_ptr_array_new();
	GHashTable *delayNames = g_hash_table_new(g_str_hash, g_str_equal);
	GPtrArray *gradientList = g_ptr_array_new();
	unsigned int index;
	gboolean p11issued = FALSE, p12issued = FALSE, p13issued = FALSE, p14issued = FALSE;
	gboolean lastElementWasSelectiveF1Pulse = FALSE, lastElementWasSelectiveF2Pulse = FALSE;
	gboolean lastElementWasF1Pulse = FALSE, lastElementWasF2Pulse = FALSE, d13Present = FALSE;
	float p11duration, p12duration, p13duration, p14duration, dante = FALSE, d14Present = FALSE;
	int spin1type, spin2type, numberOfGradientsLeft;
	gchar *d2Value = NULL, *d3Value = NULL, *d4Value = NULL;
	gchar *sp1file, *sp2file, *sp3file, *sp4file;
	unsigned int numberOfReplacements;
	float *float_ptr1, *float_ptr2, *delayValue, *delayValueInMenu;
    gchar *char_ptr1, *char_ptr2;
	unsigned int numberOfElements = insensitive_pulsesequence_get_number_of_elements(self->pulseSequence);

	// At the moment when several pulses with different shapes are used the later ones are ignored!

	// Check whether sequence includes acquisition
	firstElement = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, 0);
	secondElement = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, 1);
	thirdElement = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, 2);
	lastElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
	if (lastElement == NULL)
		return NULL;
	if (lastElement->type != SequenceTypeFID)
		return NULL;
	else if (lastElement->activeSSpins)
		detectsISpins = FALSE;
	else
		detectsISpins = TRUE;
	g_string_append_printf(pp, ";%s\n;insensitive-version %s\n;\n;DISCLAIMER: This pulse program was created automatically for\n;educational purposes only. It may be incomplete or contain\n;errors. Do not run it on real spectrometer hardware without\n;checking the code with an experienced and authorised person.\n;The author takes absolutely no warranty for damaged equipment!\n;\n;$CLASS=\n;$DIM=%s\n;$TYPE=\n;$SUBTYPE=\n;$COMMENT=\n\n\n#include <Avance.incl>",
			       name, insensitive_version, (self->indexForVariableEvolutionTime == 0) ? "1D" : "2D");
	// Build the pulse sequence code
	g_string_append(pulseCode, "1 ze\n2 d1");
	// Switch off decoupler if it was left on at the end of the sequence, but it starts without decoupling
	if (detectsISpins && lastElement->iDecoupling) {
		if (!firstElement->iDecoupling)
			g_string_append(pulseCode, " do:f1");
	} else if (!detectsISpins && lastElement->sDecoupling) {
		if (!firstElement->sDecoupling)
			g_string_append(pulseCode, " do:f1");
	}
	if (detectsISpins && lastElement->sDecoupling) {
		if (!firstElement->sDecoupling)
			g_string_append(pulseCode, " do:f2");
	} else if (!detectsISpins && lastElement->iDecoupling) {
		if (!firstElement->iDecoupling)
			g_string_append(pulseCode, " do:f2");
	}
	g_string_append(pulseCode, "\n3 ");
	for (i = 0; i < numberOfElements; i++) {
		element = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, i);
		switch (element->type) {
		case SequenceTypePulse:
			switch (element->pulseEnvelope) {
			case Rectangle:
				shapedPulseFile = "(Squa100.1000)";
				break;
			case Gaussian:
				shapedPulseFile = "(Gaus1.1000)";
				break;
			case Sinc:
				shapedPulseFile = "(sinc3.1000)";
				break;
			case EBURP_1:
				shapedPulseFile = "(eburp1.64)";
				break;
			case EBURP_2:
				shapedPulseFile = "(Eburp2.1000)";
				break;
			case IBURP_2:
				shapedPulseFile = "(Iburp2.1000)";
				break;
			case REBURP:
				shapedPulseFile = "(Reburp.1000)";
				break;
			case DANTE:
				dante = TRUE;
			default:
				shapedPulseFile = "";
			}
			shapedPulsePrefix = "";
			shapedPulseSuffix = "";
			if ((lastElementWasF1Pulse && ((element->activeISpins && detectsISpins) || (element->activeSSpins && !detectsISpins)))
			    || (lastElementWasF2Pulse && ((element->activeSSpins && detectsISpins) || (element->activeISpins && !detectsISpins)))) {
				g_string_append(pulseCode, "d13\n  ");
				d13Present = TRUE;
			}
			if ((element->activeSSpins && detectsISpins) || (element->activeISpins && !detectsISpins)) {
				f2_is_used = TRUE;
				lastElementWasF2Pulse = TRUE;
				if (element->pulseEnvelope != Rectangle) {
					shapedPulsePrefix = "4u pl0:f2\n  ";
					shapedPulseSuffix = "\n  4u";
					if (element->time == 180.0) {
						shapedPulseString = "p14:sp4";
						shapedPulse180 = TRUE;
						if (!p14issued) {
							p14issued = TRUE;
							p14duration = element->pulseDuration;
							sp4file = malloc(strlen(shapedPulseFile) * sizeof(gchar));
							strcpy(sp4file, shapedPulseFile);
						}
					} else {
						shapedPulseString = "p13:sp3";
						shapedPulse90 = TRUE;
						if (!p13issued) {
							p13issued = TRUE;
							p13duration = element->pulseDuration;
							sp3file = malloc(strlen(shapedPulseFile) * sizeof(gchar));
							strcpy(sp3file, shapedPulseFile);
						}
					}
					lastElementWasSelectiveF1Pulse = FALSE;
					lastElementWasSelectiveF2Pulse = TRUE;
				} else {
					if (element->time == 180.0)
						shapedPulseString = "p4";
					else
						shapedPulseString = "p3";
					lastElementWasSelectiveF1Pulse = FALSE;
					lastElementWasSelectiveF2Pulse = FALSE;
				}
				g_string_append(pulseCode, shapedPulsePrefix);
				if (element->activeISpins && element->activeSSpins)
					g_string_append(pulseCode, "(center ");
				if (strlen(shapedPulseString) > 3) {
					if (element->activeISpins && element->activeSSpins)
						g_string_append(pulseCode, "(");
					if (element->time == 90.0)
						g_string_append_printf(pulseCode, "%s:f2 ph%d:r", shapedPulseString, pulseNumber);
					else if (element->time == 180.0) {
						g_string_append_printf(pulseCode, "%s:f2 ph%d:r", shapedPulseString, pulseNumber);
						contains180Pulse_f2 = TRUE;
					} else
						g_string_append_printf(pulseCode, "%s*%.2f:f2 ph%d:r", shapedPulseString, element->time / 90, pulseNumber);
					if (element->activeISpins && element->activeSSpins)
						g_string_append(pulseCode, ")");
				} else {
					if (element->time == 90.0)
						g_string_append_printf(pulseCode, "(%s ph%d):f2", shapedPulseString, pulseNumber);
					else if (element->time == 180.0) {
						g_string_append_printf(pulseCode, "(%s ph%d):f2", shapedPulseString, pulseNumber);
						contains180Pulse_f2 = TRUE;
					} else
						g_string_append_printf(pulseCode, "(%s*%.2f ph%d):f2", shapedPulseString, element->time / 90, pulseNumber);
				}
				g_string_append(pulseCode, " ");
			}
			if ((element->activeISpins && detectsISpins) || (element->activeSSpins && !detectsISpins)) {
				lastElementWasF1Pulse = TRUE;
				if (element->pulseEnvelope != Rectangle) {
					shapedPulsePrefix = "4u pl0:f1\n  ";
					shapedPulseSuffix = "\n  4u";
					if (element->time == 180.0) {
						shapedPulseString = "p12:s";
						shapedPulse180 = TRUE;
						if (!p12issued) {
							p12issued = TRUE;
							p12duration = element->pulseDuration;
							sp2file = malloc(strlen(shapedPulseFile) * sizeof(gchar));
							strcpy(sp2file, shapedPulseFile);
						}
					} else {
						shapedPulseString = "p11:s";
						shapedPulse90 = TRUE;
						if (!p11issued) {
							p11issued = TRUE;
							p11duration = element->pulseDuration;
							sp1file = malloc(strlen(shapedPulseFile) * sizeof(gchar));
							strcpy(sp1file, shapedPulseFile);
						}
					}
					lastElementWasSelectiveF1Pulse = TRUE;
					lastElementWasSelectiveF2Pulse = FALSE;
				} else {
					shapedPulseString = "";
					lastElementWasSelectiveF1Pulse = FALSE;
					lastElementWasSelectiveF2Pulse = FALSE;
				}
				if (!(element->activeSSpins && detectsISpins) && !(element->activeISpins && !detectsISpins))
					g_string_append(pulseCode, shapedPulsePrefix);
				if (element->activeISpins && element->activeSSpins)
					g_string_append(pulseCode, "(");
				if (strlen(shapedPulseString) > 3) {
					if (element->time == 90.0)
						g_string_append_printf(pulseCode, "%sp1:f1 ph%d:r", shapedPulseString, pulseNumber);
					else if (element->time == 180.0) {
						g_string_append_printf(pulseCode, "%sp2:f1 ph%d:r", shapedPulseString, pulseNumber);
						contains180Pulse_f1 = TRUE;
					} else
						g_string_append_printf(pulseCode, "%sp1*%.2f:f1 ph%d:r", shapedPulseString, element->time / 90, pulseNumber);
				} else {
					if (element->time == 90.0)
						g_string_append_printf(pulseCode, "%sp1 ph%d", shapedPulseString, pulseNumber);
					else if (element->time == 180.0) {
						g_string_append_printf(pulseCode, "%sp2 ph%d", shapedPulseString, pulseNumber);
						contains180Pulse_f1 = TRUE;
					} else
						g_string_append_printf(pulseCode, "%sp1*%.2f ph%d", shapedPulseString, element->time / 90, pulseNumber);
				}
				if (element->activeISpins && element->activeSSpins)
					g_string_append(pulseCode, ")");
				if (element->activeISpins && element->activeSSpins)
					g_string_append(pulseCode, " )");
			}
			g_string_append(pulseCode, shapedPulseSuffix);
			g_string_append(pulseCode, "\n  ");
			pulseNumber++;
			break;
		case SequenceTypeEvolution:
			numberOfDelays++;
			if (i + 1 == (unsigned int)abs(self->indexForVariableEvolutionTime)) {
				g_string_append(pulseCode, "d0");
				if (self->indexForVariableEvolutionTime < 0) {
					variableDelayIsEcho = TRUE;
				}
			} else if (lastElementWasSelectiveF1Pulse || lastElementWasSelectiveF2Pulse) {
				g_string_append(pulseCode, "d14 pl");
				if (lastElementWasSelectiveF1Pulse)
					g_string_append(pulseCode, "1:f1");
				if (lastElementWasSelectiveF2Pulse)
					g_string_append(pulseCode, "3:f2");
				d14Present = TRUE;
			} else if (variableDelayIsEcho) {
				g_string_append(pulseCode, "d0");
				variableDelayIsEcho = FALSE;
			} else {
				delayOccurredBefore = -1;
				for (index = 0; index < delayList->len; index++) {
                    delayValue = g_ptr_array_index(delayList, index);
                    if (*delayValue == element->time) {
						delayOccurredBefore = index;
						index = (unsigned int)delayList->len;
					}
				}
				if (delayOccurredBefore >= 0) {
					delayLabel = malloc(11 * sizeof(gchar));
					sprintf(delayLabel, "REPLACE%d", delayOccurredBefore + 1);
				} else {
					delayLabel = malloc(11 * sizeof(gchar));
					sprintf(delayLabel, "REPLACE%d", delayNumber++ - 1);
					g_ptr_array_add(delayList, &element->time);
				}
				g_string_append(pulseCode, delayLabel);
				g_hash_table_insert(delayNames, delayLabel, &element->time);
			}
			nextDelay = NULL;
			for (j = i + 1; j < numberOfElements; j++) {
				if ((insensitive_pulsesequence_get_element_at_index(self->pulseSequence, j)->type == SequenceTypeEvolution)
				    || (insensitive_pulsesequence_get_element_at_index(self->pulseSequence, j)->type == SequenceTypeFID)) {
					nextDelay = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, j);
					break;
				}
			}
			if (element->spinlock) {
				g_string_append_printf(pulseCode, " pl11:f1\n  p15"); // ph#
				spinlock = TRUE;
			} else {
				if ((element->iDecoupling && detectsISpins) || (element->sDecoupling && !detectsISpins)) {
					if (element->iDecoupling) {
						nextElementDecouples = nextDelay->iDecoupling;
					} else {
						nextElementDecouples = nextDelay->sDecoupling;
					}
					if (!longF1Decoupling) {
						if (nextElementDecouples) {
							g_string_append(pulseCode, " cpd2:f1"); //cw
							longF1Decoupling = TRUE;
						} else {
							g_string_append(pulseCode, " pl12");
						}
					} else {
						if (!nextElementDecouples) {
							g_string_append(pulseCode, " do:f1");
							longF1Decoupling = FALSE;
						}
					}
					f1Decoupling = TRUE;
				}
				if ((element->sDecoupling && detectsISpins) || (element->iDecoupling && !detectsISpins)) {
					if (element->iDecoupling) {
						nextElementDecouples = nextDelay->iDecoupling;
					} else {
						nextElementDecouples = nextDelay->sDecoupling;
					}
					if (!longF2Decoupling) {
						if (nextElementDecouples) {
							g_string_append(pulseCode, " cpd2:f2"); //cw
							longF2Decoupling = TRUE;
						} else {
							g_string_append(pulseCode, " pl12:f2");
						}
					} else {
						if (!nextElementDecouples) {
							g_string_append(pulseCode, " do:f2");
							longF2Decoupling = FALSE;
						}
					}
					f2Decoupling = TRUE;
				}
			}
			g_string_append(pulseCode, "\n  ");
			lastElementWasSelectiveF1Pulse = FALSE;
			lastElementWasSelectiveF2Pulse = FALSE;
			lastElementWasF1Pulse = FALSE;
			lastElementWasF2Pulse = FALSE;
			break;
		case SequenceTypeGradient:
			gradientOccurredBefore = -1;
			for (index = 0; index < gradientList->len; index += 2) {
				float_ptr1 = g_ptr_array_index(gradientList, index);
				float_ptr2 = g_ptr_array_index(gradientList, index + 1);
				if ((*float_ptr1 == element->time) && (*float_ptr2 == fabsf(element->secondParameter))) {
					gradientOccurredBefore = index;
					index = (unsigned int)delayList->len;
				}
			}
			if (!includeGradients)
				g_string_append(pulseCode, "50u UNBLKGRAD\n  ");
			else
				g_string_append(pulseCode, "d13\n  ");
			if (gradientOccurredBefore >= 0) {
				g_string_append_printf(pulseCode, "p16:gp%d", (gradientOccurredBefore / 2) + 1);
			} else {
				g_string_append_printf(pulseCode, "p16:gp%d", gradientNumber++);
				float_ptr1 = malloc(sizeof(float));
				*float_ptr1 = fabsf(element->time);
				g_ptr_array_add(gradientList, float_ptr1);
				float_ptr2 = malloc(sizeof(float));
				*float_ptr2 = fabsf(element->secondParameter);
				g_ptr_array_add(gradientList, float_ptr2);
			}
			if (element->secondParameter < 0)
				g_string_append(pulseCode, "*-1");
			g_string_append(pulseCode, "\n  d16\n  ");
			numberOfGradientsLeft = 0;
			for (j = i + 1; j < numberOfElements; j++) {
				if (insensitive_pulsesequence_get_element_at_index(self->pulseSequence, j)->type == SequenceTypeGradient)
					numberOfGradientsLeft++;
			}
			if (numberOfGradientsLeft == 0)
				g_string_append_printf(pulseCode, "4u BLKGRAD\n  ");
			includeGradients = TRUE;
			lastElementWasSelectiveF1Pulse = FALSE;
			lastElementWasSelectiveF2Pulse = FALSE;
			lastElementWasF1Pulse = FALSE;
			lastElementWasF2Pulse = FALSE;
			break;
		case SequenceTypeFID:
			g_string_append_printf(pulseCode, "go=2 ph31");
			if (((element->sDecoupling && detectsISpins) || (element->iDecoupling && !detectsISpins)) && !longF2Decoupling) {
				f2_is_used = TRUE;
				g_string_append_printf(pulseCode, " cpd2:f2");
				f2Decoupling = TRUE;
				longF2Decoupling = TRUE;
			} else if (((element->iDecoupling && detectsISpins) || (element->sDecoupling && !detectsISpins)) && !longF1Decoupling) {
				g_string_append_printf(pulseCode, " cpd2");
				f1Decoupling = TRUE;
				longF1Decoupling = TRUE;
			}
			g_string_append_printf(pulseCode, "\n  d1");
			if (longF1Decoupling || longF2Decoupling) {
				g_string_append(pulseCode, "1");
				d11Present = TRUE;
			}
			if (longF1Decoupling)
				g_string_append(pulseCode, " do:f1");
			if (longF2Decoupling)
				g_string_append(pulseCode, " do:f2");
			g_string_append_printf(pulseCode, " mc #0 to 2 ");
			if (self->indexForVariableEvolutionTime == 0)
				g_string_append(pulseCode, "F0(zd)");
			else {
				if (insensitive_settings_get_detectionMethod(self->settings) != None)
					g_string_append(pulseCode, "F1PH(calph(ph1, +90), caldel(d0, +in0))");
				//else if(includeGradients)
				//  g_string_append(pulseCode, "F1EA(calph(ph1, +90), caldel(d0, +in0))");
				else
					g_string_append(pulseCode, "F1QF(caldel(d0, +in0))");
			}
			g_string_append(pulseCode, "\nexit\n\n");
			lastElementWasSelectiveF1Pulse = FALSE;
			lastElementWasSelectiveF2Pulse = FALSE;
			lastElementWasF1Pulse = FALSE;
			lastElementWasF2Pulse = FALSE;
			break;
		case SequenceTypeShift:
		case SequenceTypeCoupling:
		case SequenceTypeRelaxation:
			break;
		}
	}
	g_string_append(pp, "\n#include <Delay.incl>");
	if (includeGradients)
		g_string_append(pp, "\n#include <Grad.incl>");
	g_string_append(pp, "\n\n\n");
	if (contains180Pulse_f1)
		g_string_append(pp, "\"p2=p1*2\"\n");
	if (contains180Pulse_f2)
		g_string_append(pp, "\"p4=p3*2\"\n");
	//if(indexForVariableEvolutionTime != 0)
	//  g_string_append(pp, "\"d0=3u\"\n"]);
	for (i = 0; i < delayList->len; i++) {
		float_ptr1 = g_ptr_array_index(delayList, i);
		// Recognise an NOESY pulse program: 3 pulses, only I spins, no or first delay is incremented
		if (i == delayList->len - 1 && (self->indexForVariableEvolutionTime == 0 || self->indexForVariableEvolutionTime == 2)
		    && self->pulseList->len == 3 && detectsISpins && !f2_is_used) {
			delayLabel = "d8";
			identifiedNOESY = TRUE;
		}
		// Recognise an Inversion Recovery pulse program: 2 pulses, only I spins, no delay is incremented
		else if (i == 0 && self->indexForVariableEvolutionTime == 0 && pulseNumber == 3 && numberOfElements == 4) {
			if (firstElement->type == SequenceTypePulse && firstElement->time == 180
			    && secondElement->type == SequenceTypeEvolution
			    && thirdElement->type == SequenceTypePulse && thirdElement->time == 90) {
				delayLabel = "d7";
				identifiedInversionRecovery = TRUE;
			}
		} else if (!d6Present && *float_ptr1 > 2 && !identifiedNOESY && !spinlock) {
			delayLabel = "d6";
			d6Present = i + 1;
			//NSString *d6Label = [NSString stringWithFormat:@"REPLACE%d", d6Present];
			//NSLog(@"%s = %f", d6Label, [[delayNames valueForKey:d6Label] floatValue]);
		} else  {
			delayLabel = malloc(11 * sizeof(gchar));
			sprintf(delayLabel, "REPLACE%d", i + 1);
		}
		if (checkForD2) {
			for (j = 0; j < 4 * 6; j += 4) {
				delayValue = self->selectableDelayValues + j;
				delayValueInMenu = g_hash_table_lookup(delayNames, delayLabel);
				if (delayValue != NULL && delayValueInMenu != NULL) {
					if (*delayValue == *delayValueInMenu) {
						d2Present = TRUE;
						coupling_partners_from_index(&spin1type, &spin2type, j / 4, insensitive_spinsystem_get_spins(self->spinSystem));
						spin1type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin1type - 1);
						spin2type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin2type - 1);
						if (spin1type == spinTypeI && spin2type == spinTypeI) {
							cnstHH = TRUE;
							if (d2Value == NULL)
								d2Value = "1s/cnst1*2";
						} else if (spin1type == spinTypeS && spin2type == spinTypeS) {
							cnstXX = TRUE;
							if (d2Value == NULL)
								d2Value = "1s/cnst3*2";
						} else {
							cnstHX = TRUE;
							if (d2Value == NULL)
								d2Value = "1s/cnst2*2";
						}
						break;
					}
				}
			}
		}
		if (checkForD3) {
			for (j = 1; j < 4 * 6; j += 4) {
				delayValue = self->selectableDelayValues + j;
				delayValueInMenu = g_hash_table_lookup(delayNames, delayLabel);
				if (delayValue != NULL && delayValueInMenu != NULL) {
					if (*delayValue == *delayValueInMenu) {
						d3Present = TRUE;
						coupling_partners_from_index(&spin1type, &spin2type, (j - 1) / 4, insensitive_spinsystem_get_spins(self->spinSystem));
						spin1type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin1type - 1);
						spin2type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin2type - 1);
						if (spin1type == spinTypeI && spin2type == spinTypeI) {
							cnstHH = TRUE;
							if (d3Value == NULL)
								d3Value = "1s/cnst1*3";
						} else if (spin1type == spinTypeS && spin2type == spinTypeS) {
							cnstXX = TRUE;
							if (d3Value == NULL)
								d3Value = "1s/cnst3*3";
						} else {
							cnstHX = TRUE;
							if (d3Value == NULL)
								d3Value = "1s/cnst2*3";
						}
						break;
					}
				}
			}
		}
		if (checkForD4 && 4 * 6 /*[selectableDelayValues count]*/ > 0) {
			for (j = 2; j < 4 * 6; j += 4) {
				delayValue = self->selectableDelayValues + j;
				delayValueInMenu = g_hash_table_lookup(delayNames, delayLabel);
				if (delayValue != NULL && delayValueInMenu != NULL) {
					if (*delayValue == *delayValueInMenu) {
						d4Present = TRUE;
						coupling_partners_from_index(&spin1type, &spin2type, (j - 2) / 4, insensitive_spinsystem_get_spins(self->spinSystem));
						spin1type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin1type - 1);
						spin2type = insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, spin2type - 1);
						if (spin1type == spinTypeI && spin2type == spinTypeI) {
							cnstHH = TRUE;
							if (d4Value == NULL)
								d4Value = "1s/cnst1*4";
						} else if (spin1type == spinTypeS && spin2type == spinTypeS) {
							cnstXX = TRUE;
							if (d4Value == NULL)
								d4Value = "1s/cnst3*4";
						} else {
							cnstHX = TRUE;
							if (d4Value == NULL)
								d4Value = "1s/cnst2*4";
						}
						break;
					}
				}
			}
		}
		if (d2Present && checkForD2) {
			if (d2Value == NULL) {
				d2Value = malloc(11 * sizeof(gchar));
				sprintf(d2Value, "%.2fu", *float_ptr1);
			}
			g_string_append_printf(pp, "\"d2=%s\"\n", d2Value);
			while (insensitive_g_string_replace(pulseCode, delayLabel, "d2", pulseCode));
			checkForD2 = FALSE;
		} else if (d3Present && checkForD3) {
			if (d3Value == NULL) {
				d3Value = malloc(11 * sizeof(gchar));
				sprintf(d3Value, "%.2fu", *float_ptr1);
			}
			g_string_append_printf(pp, "\"d3=%s\"\n", d3Value);
			while (insensitive_g_string_replace(pulseCode, delayLabel, "d3", pulseCode));
			checkForD3 = FALSE;
		} else if (d4Present && checkForD4) {
			if (d4Value == NULL) {
				d4Value = malloc(11 * sizeof(gchar));
				sprintf(d4Value, "%.2fu", *float_ptr1);
			}
			g_string_append_printf(pp, "\"d4=%s\"\n", d4Value);
			while (insensitive_g_string_replace(pulseCode, delayLabel, "d4", pulseCode));
			checkForD4 = FALSE;
			//} else if(d6Present && checkForD6) {
			//  g_string_append_printf(pp, "\"d6=%.2fm\"\n", *float_ptr1);
			//  while(insensitive_g_string_replace(pulseCode, delayLabel, "d6", pulseCode));
		} else {
			g_string_append_printf(pp, "\"%s=%.2fu\"\n", delayLabel, *float_ptr1);
		}
	}
	if (gradientNumber > 1)
		g_string_append_printf(pp, "\"d13=4u\"\n");
	if (self->indexForVariableEvolutionTime > 0)
		g_string_append_printf(pp, "\n\n\"in0=inf1\"\n\n\"d0=3u\"\n");
	else if (self->indexForVariableEvolutionTime < 0)
		g_string_append_printf(pp, "\n\n\"in0=inf1/2\"\n\n\"d0=3u\"\n");
	if (contains180Pulse_f1 || contains180Pulse_f2 || self->indexForVariableEvolutionTime != 0 || delayList->len > 0)
		g_string_append_printf(pp, "\n\n");
	if (p11issued || p12issued || p13issued || p14issued)
		g_string_append_printf(pp, "#ifdef CALC_SPOFFS\n\"spoff1=bf1*(cnst21/1000000)-o1\"\n#else\n#endif /*CALC_SPOFFS*/\n\n\n");

	// Center consecutive 180° pulses using regex substitution
	// Should also be implemented for consecutive 90° pulses
	// Cannot currently decide whether centering makes sense and is not what Insensitive does!
	/*NSString *regex;
	   regex = @"p2 ph[0-9]+\n  \\(p4 ph[0-9]+\\):f2";
	   [pulseCode replaceOccurrencesOfRegex:regex
	                             options:RKLCaseless
	                             inRange:NSMakeRange(0, [pulseCode length])
	                               error:NULL
	                  enumerationOptions:RKLRegexEnumerationNoOptions
	                          usingBlock:^NSString *(NSInteger captureCount,
	                                                 NSString *const *capturedStrings,
	                                                 const NSRange capturedRanges[captureCount],
	                                                 volatile gboolean *const stop) {
	                              NSCharacterSet *numberCharset = [NSCharacterSet characterSetWithCharactersInString:@"0123456789"];
	                              NSScanner *numberScanner = [NSScanner scannerWithString:capturedStrings[0]];
	                              int i, integer, ph1 = 0, ph2 = 0;
	                              for(i = 0; i < 4 && ![numberScanner isAtEnd]; i++) {
	                                  [numberScanner scanUpToCharactersFromSet:numberCharset intoString:NULL];
	                                  if([numberScanner scanInt:&integer]) {
	                                      switch(i) {
	                                          case 1:
	                                              ph1 = integer;
	                                              break;
	                                          case 3:
	                                              ph2 = integer;
	                                              break;
	                                          default:
	                                              break;
	                                      }
	                                  }
	                              }
	                              return [NSString stringWithFormat:@"(center (p2 ph%d) (p4 ph%d):f2 )", ph1, ph2];
	                          }];
	   regex = @"\\(p4 ph[0-9]+\\):f2 \n  p2 ph[0-9]+";
	   [pulseCode replaceOccurrencesOfRegex:regex
	                             options:RKLCaseless
	                             inRange:NSMakeRange(0, [pulseCode length])
	                               error:NULL
	                  enumerationOptions:RKLRegexEnumerationNoOptions
	                          usingBlock:^NSString *(NSInteger captureCount,
	                                                 NSString *const *capturedStrings,
	                                                 const NSRange capturedRanges[captureCount],
	                                                 volatile gboolean *const stop) {
	                              NSCharacterSet *numberCharset = [NSCharacterSet characterSetWithCharactersInString:@"0123456789"];
	                              NSScanner *numberScanner = [NSScanner scannerWithString:capturedStrings[0]];
	                              int i, integer, ph1 = 0, ph2 = 0;
	                              for(i = 0; i < 5 && ![numberScanner isAtEnd]; i++) {
	                                  [numberScanner scanUpToCharactersFromSet:numberCharset intoString:NULL];
	                                  if([numberScanner scanInt:&integer]) {
	                                      switch(i) {
	                                          case 1:
	                                              ph1 = integer;
	                                              break;
	                                          case 4:
	                                              ph2 = integer;
	                                              break;
	                                          default:
	                                              break;
	                                      }
	                                  }
	                              }
	                              return [NSString stringWithFormat:@"(center (p2 ph%d) (p4 ph%d):f2 )", ph2, ph1];
	                          }];*/

	// Insert pulse program code
	g_string_append(pp, pulseCode->str);
	g_string_free(pulseCode, TRUE);

	// Replace "REPLACE#" with "DELTA#" with new index
	for (i = 1; i <= numberOfDelays; i++) {
		delayLabel = malloc(11 * sizeof(gchar));
		sprintf(delayLabel, "REPLACE%d", i);
		if (identifiedNOESY && ((i == numberOfDelays && self->indexForVariableEvolutionTime == 0) || (i == numberOfDelays - 1 && self->indexForVariableEvolutionTime != 0)))
			newLabel = "d8";
		else if (identifiedInversionRecovery && i == 1 && self->indexForVariableEvolutionTime == 0)
			newLabel = "d7";
		else if (i == d6Present)
			newLabel = "d6";
		else {
			newLabel = malloc(16 * sizeof(gchar));
			sprintf(newLabel, "DELTA%d", delayIndex);
		}
		for (numberOfReplacements = 0; insensitive_g_string_replace(pp, delayLabel, newLabel, pp); numberOfReplacements++);
		if (numberOfReplacements > 0)
			delayIndex++;
	}
	if (g_strstr_len(pp->str, pp->len, "DELTA") == NULL)
		insensitive_g_string_replace(pp, "\n#include <Delay.incl>", " ", pp);

	// Create phase cycling table
	if (self->phaseCyclingArray->len >= (self->phaseCycles / 2) * (self->pulseList->len + 1) + self->pulseList->len + 1) {
		unsigned int cycleSize;
		gboolean half;
		for (pulse = 0; pulse < self->pulseList->len; pulse++) {
			// ph31 is reserved for receiver phase cycle
			g_string_append_printf(pp, "\nph%d=", pulse + ((pulse < 30) ? 1 : 2));
			// Check if phase cycling table can be shortened
			cycleSize = self->phaseCycles;
			if (cycleSize % 2 == 0) {
				half = TRUE;
				while (cycleSize > 1 && half) {
					half = TRUE;
					for (cycle = 0; cycle < cycleSize / 2; cycle++) {
						char_ptr1 = g_ptr_array_index(self->phaseCyclingArray, cycle * (self->pulseList->len + 1) + pulse + 1);
						char_ptr2 = g_ptr_array_index(self->phaseCyclingArray, (cycle + cycleSize / 2) * (self->pulseList->len + 1) + pulse + 1);
						if (atof(char_ptr1) != atof(char_ptr2))
							half = FALSE;
					}
					if (half)
						cycleSize /= 2;
				}
			}
			// Append pulse phase list
			for (cycle = 0; cycle < cycleSize; cycle++) {
				char_ptr1 = g_ptr_array_index(self->phaseCyclingArray, cycle * (self->pulseList->len + 1) + pulse + 1);
				g_string_append_printf(pp, "%.0f ", atof(char_ptr1) / 90);
			}
		}
		// Receiver phase
		g_string_append(pp, "\nph31=");
		// Check if phase cycling table can be shortened
		cycleSize = self->phaseCycles;
		if (cycleSize % 2 == 0) {
			half = TRUE;
			while (cycleSize > 1 && half) {
				half = TRUE;
				for (cycle = 0; cycle < cycleSize / 2; cycle++) {
					char_ptr1 = g_ptr_array_index(self->phaseCyclingArray, cycle * (self->pulseList->len + 1));
					char_ptr2 = g_ptr_array_index(self->phaseCyclingArray, (cycle + cycleSize / 2) * (self->pulseList->len + 1));
					if (atof(char_ptr1) != atof(char_ptr2))
						half = FALSE;
				}
				if (half)
					cycleSize /= 2;
			}
		}
		// Append receiver phase cycling table
		for (cycle = 0; cycle < cycleSize; cycle++) {
			char_ptr1 = g_ptr_array_index(self->phaseCyclingArray, cycle * (self->pulseList->len + 1));
			g_string_append_printf(pp, "%.0f ", atof(char_ptr1) / 90);
		}
	} else {
		g_string_append(pp, "\n;Error creating phase cycling table");
	}

	// Create comment section
	g_string_append(pp, "\n\n\n");
	if (p11issued || p12issued || p13issued || p14issued)
		g_string_append(pp, ";pl0 : 0W\n");
	g_string_append(pp, ";pl1 : f1 channel - power level for pulse (default)\n");
	if (f2_is_used)
		g_string_append(pp, ";pl2 : f2 channel - power level for pulse (default)\n");
	//g_string_append(pp, ";pl9 : f1 channel - power level for presaturation\n"];
	//g_string_append(pp, ";pl10: f1 channel - power level for TOCSY-spinlock\n"];
	if (spinlock)
		g_string_append(pp, ";pl11: f1 channel - power level for ROESY-spinlock\n");
	if (f2Decoupling)
		g_string_append(pp, ";pl12: f2 channel - power level for CPD/BB decoupling\n");
	//g_string_append(pp, ";pl14: f2 channel - power level for cw saturation\n"];
	if (f1Decoupling)
		g_string_append(pp, ";pl19: f1 channel - power level for CPD/BB decoupling\n");
	//g_string_append(pp, ";pl15: f2 channel - power level for TOCSY-spinlock\n"];
	if (p11issued)
		g_string_append_printf(pp, ";sp1 : f1 channel - shaped pulse  90 degree %s\n", sp1file);
	if (p12issued)
		g_string_append_printf(pp, ";sp2 : f1 channel - shaped pulse 180 degree %s\n", sp2file);
	if (p13issued)
		g_string_append_printf(pp, ";sp3 : f2 channel - shaped pulse  90 degree %s\n", sp3file);
	if (p14issued)
		g_string_append_printf(pp, ";sp4 : f2 channel - shaped pulse 180 degree %s\n", sp4file);
	//g_string_append(pp, ";sp7 : f2 channel - shaped pulse 180 degree (off resonance2) or f2 channel - shaped pulse 180 degree (adiabatic) or f1 channel - shaped pulse for wet\n");
	//g_string_append(pp, ";p0 : for different applications i.e. f1 channel - variable flip angle high power pulse in DEPT\n");
	g_string_append(pp, ";p1 : f1 channel - 90 degree high power pulse\n");
	if (contains180Pulse_f1)
		g_string_append(pp, ";p2 : f1 channel - 180 degree high power pulse\n");
	if (f2_is_used)
		g_string_append(pp, ";p3 : f2 channel - 90 degree high power pulse\n");
	if (contains180Pulse_f2)
		g_string_append(pp, ";p4 : f2 channel - 180 degree high power pulse\n");
	//g_string_append(pp, ";p6 : f1 channel - 90 degree low power pulse\n"];
	if (p11issued)
		g_string_append_printf(pp, ";p11: f1 channel -  90 degree shaped pulse for excitation [%.1f msec]\n", p11duration);
	if (p12issued)
		g_string_append_printf(pp, ";p12: f1 channel - 180 degree shaped pulse for refocussing [%.1f msec]\n", p12duration);
	if (p13issued)
		g_string_append_printf(pp, ";p13: f2 channel -  90 degree shaped pulse for excitation [%.1f msec]\n", p13duration);
	if (p14issued)
		g_string_append_printf(pp, ";p14: f2 channel - 180 degree shaped pulse for refocussing [%.1f msec]\n", p14duration);
	if (spinlock)
		g_string_append(pp, ";p15: f1 channel - pulse for ROESY spinlock\n");
	if (includeGradients)
		g_string_append(pp, ";p16: homospoil/gradient pulse\n");
	//g_string_append(pp, ";p17: f1 channel - trim pulse at pl10 or pl15\n");
	//g_string_append(pp, ";p18: f1 channel - shaped pulse (off resonance presaturation)\n");
	if (self->indexForVariableEvolutionTime != 0)
		g_string_append(pp, ";d0 : incremented delay (2D)                         [3 usec]\n");
	g_string_append(pp, ";d1 : relaxation delay 1-5 * T1\n");
	if (d2Present)
		g_string_append(pp, ";d2 : 1/(2J)\n");
	if (d3Present)
		g_string_append(pp, ";d3 : 1/(3J)\n");
	if (d4Present)
		g_string_append(pp, ";d4 : 1/(4J)\n");
	if (d6Present)
		g_string_append(pp, ";d6 : delay for evolution of long range couplings\n");
	if (identifiedInversionRecovery)
		g_string_append(pp, ";d7 : delay for inversion recovery\n");
	if (identifiedNOESY)
		g_string_append(pp, ";d8 : mixing time\n");
	//g_string_append(pp, ";d9 : TOCSY mixing time\n");
	if (d11Present)
		g_string_append(pp, ";d11: delay for disk I/O                             [30 msec]\n");
	//g_string_append(pp, ";d12: delay for power switching                      [20 usec]\n"];
	if (gradientNumber > 1 || d13Present)
		g_string_append(pp, ";d13: short delay                                    [4 usec]\n");
	if (d14Present)
		g_string_append(pp, ";d14: delay for evolution after shaped pulse\n");
	if (includeGradients)
		g_string_append(pp, ";d16: delay for homospoil/gradient recovery\n");
	if (dante)
		g_string_append(pp, ";d17: delay for DANTE pulse-train\n");
	//g_string_append(pp, ";d18: delay for evolution of long range couplings\n");
	//g_string_append(pp, ";d19: delay for binomial water suppression\n");
	//g_string_append(pp, ";d20: for different applications\n");
	if (cnstHH)
		g_string_append(pp, ";cnst1: = J(HH)\n");
	if (cnstHX)
		g_string_append(pp, ";cnst2: = J(XH)\n");
	if (cnstXX)
		g_string_append(pp, ";cnst3: = J(XX)\n");
	//if(FALSE)
	//    g_string_append(pp, ";cnst11: = for multiplicity selection\n"];
	if (p11issued || p12issued || p13issued || p14issued)
		g_string_append(pp, ";cnst21: chemical shift for selective pulse (offset, in ppm)\n");
	if (self->indexForVariableEvolutionTime > 0) {
		g_string_append(pp, ";nd0 = 1\n;inf1: 1/SW = 2 * DW\n;in0: 1/(1 * SW) = 2 * DW\n");
	} else if (self->indexForVariableEvolutionTime < 0) {
		g_string_append(pp, ";nd0 = 2\n;inf1: 1/SW = 2 * DW\n;in0: 1/(2 * SW) = DW\n");
	}
	g_string_append(pp, ";ns: 1 * n, total number of scans: NS * TD0\n;ds: 16\n");
	if (self->indexForVariableEvolutionTime != 0) {
		g_string_append(pp, ";td1: number of experiments\n");
		if (insensitive_settings_get_detectionMethod(self->settings) != None)
			g_string_append(pp, ";FnMODE: States-TPPI, TPPI, States or QSEQ\n");
		//else if(includeGradients)
		//  g_string_append(pp, ";FnMODE: echo-antiecho\n");
		else
			g_string_append(pp, ";FnMODE: QF\n");
	}
	if ((lastElement->sDecoupling && detectsISpins) || (lastElement->iDecoupling && !detectsISpins))
		g_string_append(pp, ";cpd2: decoupling according to sequence defined by cpdprg2\n;pcpd2: f2 channel - 90 degree pulse for decoupling sequence\n");

	if (includeGradients) {
		g_string_append(pp, "\n\n;use gradient ratio:    ");
		for (i = 1; i < gradientList->len; i += 2) {
			g_string_append_printf(pp, "gp %d", (i + 1) / 2);
			if (i + 2 < gradientList->len)
				g_string_append(pp, " : ");
		}
		g_string_append(pp, "\n;                       ");
		for (i = 1; i < gradientList->len; i += 2) {
			float_ptr1 = g_ptr_array_index(gradientList, i);
			g_string_append_printf(pp, "%4.4g", *float_ptr1 / 320);
			if (i + 2 < gradientList->len)
				g_string_append(pp, " : ");
		}
		g_string_append(pp, "\n\n;for z-only gradients:\n");
		for (i = 1; i < gradientList->len; i += 2) {
			float_ptr1 = g_ptr_array_index(gradientList, i);
			g_string_append_printf(pp, ";gpz%d: %.4g%%\n", (i + 1) / 2, *float_ptr1 / 320);
		}
		g_string_append(pp, "\n;use gradient files:\n");
		for (i = 1; i < gradientList->len; i += 2) {
			g_string_append_printf(pp, ";gpnam%d: SMSQ10.100\n", (i + 1) / 2);
		}
	}

	GDateTime *date = g_date_time_new_now_local();
	g_string_append_printf(pp, "\n\n\n;$Id: %s,v 1.0 %d/%d/%d %d:%d:%.2d ber Exp $",
                           name,
                           g_date_time_get_year(date),
                           g_date_time_get_month(date),
                           g_date_time_get_day_of_month(date),
                           g_date_time_get_hour(date),
                           g_date_time_get_minute(date),
                           g_date_time_get_second(date));
    g_date_time_unref(date);

	return pp;
}


void insensitive_controller_perform_threaded_coherencePathway_calculation(InsensitiveController * self)
{
    self->interruptCoherencePathwayCalculation = FALSE;
    g_thread_new("CoherencePathwayThread", insensitive_controller_calculate_coherencePathway, self);
}


void insensitive_controller_interrupt_coherencePathway_calculation(InsensitiveController * self)
{
    self->interruptCoherencePathwayCalculation = TRUE;
}


gboolean insensitive_controller_calculate_coherencePathway(gpointer data)
{
    /*
     *  for p spins and n pulses in the sequence there are (2p+1)ⁿ possible coherence pathways
     *  The coefficient for each pathway is the product of coefficients split at each pulse
     *  Coherence coefficients are the sum of all expectation values of the same coherence order
     *  Calculate for each phase cycle and sum up the coefficients
     *  Then plot the surviving pathways
     *
     *   2              ,----   can be saved as {0, -1, 2} and is assigned a coefficient
     *   1             /        c = Tr((I1z + I2z + 2I1zI2z + 2I1-I2+ + 2I1+I2-).ρ)
     *   0 ----,      /             × Tr((I1- + I2- + 2I1-I2z + 2I1zI2-).ρ) × Tr((2I1+I2+).ρ)
     *  -1      `----´          c is complex and needs to be turned into its absolute value
     *  -2
     *
     *  Create 2p+1 operators by summing over all product operators with the same coherence order
     *  each base-4 digit in operator code is either 1 (+), 0 (z, 0) or -1 (-), sum up over all.
     *  Save operators, then check before and after each pulse for all phase cycles.
     *  Do not count identity operator (index == 0)
     */

    InsensitiveController *self = (InsensitiveController *)data;

    unsigned int i, j, cycle, type, step, index, mask, order;
    unsigned int spins, size, sizeSquare, numberOfPulses, numberOfOrders;
    unsigned long numberOfPathways;
    int **pathway;
    gchar *char_ptr;
    float receiverPhase;
    DSPComplex *zmag, *shiftplus, *shiftminus, *identity, *coefficient;
    DSPComplex **orderOperator, *operator, *temp, z, c;
    InsensitiveSpinSystem *densityMatrix;
    SequenceElement *element;
    InsensitiveSettings *simpleSettings;
    unsigned int numberOfSpins = insensitive_spinsystem_get_spins(self->spinSystem);

    // Use simpler settings that do not influence the transfer pathway for faster computation
    insensitive_settings_save_defaults(self->settings);
    simpleSettings = insensitive_settings_new();
    simpleSettings->saveSettings = FALSE;
    insensitive_settings_set_ignoreOffResonanceEffectsForPulses(simpleSettings, TRUE);
    insensitive_settings_set_dipolarRelaxation(simpleSettings,FALSE);
    insensitive_settings_set_dephasingJitter(simpleSettings,FALSE);

    zmag = Iz(0, 1);
    shiftplus = Iplus(0, 1);
    shiftminus = Iminus(0, 1);
    identity = malloc(4 * sizeof(DSPComplex));
    set_complex_identity_matrix(identity, 2);
    numberOfPulses = (int)self->pulseList->len;
    for(type = 0; type < 2 && !self->interruptCoherencePathwayCalculation; type++) {
        if(type == spinTypeI) {
            spins = insensitive_spinsystem_get_number_of_ispins(self->spinSystem);
        } else if(type == spinTypeS) {
            spins = insensitive_spinsystem_get_number_of_sspins(self->spinSystem);
        }
        if (spins > 0) {
            z = complex_rect(1.0, 0.0);
            size = pow2(numberOfSpins);
            sizeSquare = size * size;
            numberOfOrders = 2 * spins + 1;
            numberOfPathways = pow(numberOfOrders, numberOfPulses);
            if (numberOfPathways * self->phaseCycles <= pow(10, insensitive_settings_get_maxCoherenceCalculations(self->settings))
               && numberOfPathways * self->phaseCycles > 0) {
                pathway = malloc(numberOfPathways * sizeof(int *));
                coefficient = malloc(numberOfPathways * sizeof(DSPComplex));
                for (i = 0; i < numberOfPathways; i++) {
                    pathway[i] = malloc(numberOfPulses * sizeof(int));
                    coefficient[i] = complex_rect(0.0, 0.0);
                }

                // Create operators
                orderOperator = malloc(numberOfOrders * sizeof(DSPComplex *));
                for(i = 0; i < numberOfOrders; i++) {
                    orderOperator[i] = malloc(sizeSquare * sizeof(DSPComplex));
                    set_complex_zero_matrix(orderOperator[i], size);
                }
                for(index = 0; index < pow(4, numberOfSpins) && !self->interruptCoherencePathwayCalculation; index++) {
                    operator = malloc(sizeof(DSPComplex));
                    set_complex_identity_matrix(operator, 1);
                    order = 0;
                    for(i = 0; i < numberOfSpins && !self->interruptCoherencePathwayCalculation; i++) {
                        if(insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == type) {
                            mask = component_from_base4_coded_product_operator(index, i);
                            switch(mask) {
                                case 1:
                                    temp = kronecker_multiply(operator, pow2(i), zmag, 2);
                                    break;
                                case 2:
                                    temp = kronecker_multiply(operator, pow2(i), shiftminus, 2);
                                    order--;
                                    break;
                                case 3:
                                    temp = kronecker_multiply(operator, pow2(i), shiftplus, 2);
                                    order++;
                                    break;
                                default:
                                    temp = kronecker_multiply(operator, pow2(i), identity, 2);
                            }
                        } else {
                            temp = kronecker_multiply(operator, pow2(i), identity, 2);
                        }
                        free(operator);
                        operator = temp;
                    }
                    cblas_caxpy(sizeSquare, &z, operator, 1, orderOperator[order + spins], 1);
                    free(operator);
                }

                // Create pathway table
                for(step = 0; step < numberOfPulses && !self->interruptCoherencePathwayCalculation; step++) {
                    index = 0;
                    for(j = pow(numberOfOrders, numberOfPulses - step - 1); j > 0; j--) {
                        for(order = 0; order < numberOfOrders; order++) {
                            for(i = 0; i < pow(numberOfOrders, step); i++) {
                                pathway[index++][numberOfPulses - 1 - step] = order;
                            }
                        }
                    }
                }

                // Calculate coefficients for each pathway
                densityMatrix = insensitive_spinsystem_copy(self->spinSystem);
                // Loop through phase cycles
                for(cycle = 0; cycle < self->phaseCycles && !self->interruptCoherencePathwayCalculation; cycle++) {
                    // Fill in pulse phases from phase cycling table
                    char_ptr = g_ptr_array_index(self->phaseCyclingArray, cycle * numberOfPulses);
                    receiverPhase = atof(char_ptr);
                    for(step = 0; step < numberOfPulses; step++) {
                        element = g_ptr_array_index(self->pulseList, step);
                        char_ptr = g_ptr_array_index(self->phaseCyclingArray, (cycle * numberOfPulses) + step + 1);
                        element->secondParameter = atof(char_ptr) - receiverPhase;
                        if(element->secondParameter < 0)
                            element->secondParameter += 360;
                    }
                    // Loop through pathways
                    for(i = 0; i < numberOfPathways && !self->interruptCoherencePathwayCalculation; i++) {
                        index = 0;
                        insensitive_spinsystem_return_to_thermal_equilibrium(densityMatrix);
                        c = complex_rect(1.0, 0.0);
                        // Loop through pulse sequence and update coefficient after each pulse
                        for (step = 0; step < (unsigned int)insensitive_pulsesequence_get_number_of_elements(self->pulseSequence) && !self->interruptCoherencePathwayCalculation; step++) {
                            if (hypotf(c.real, c.imag) >= 0.000001) {
                                insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence,
                                                                                        densityMatrix,
                                                                                        step,
                                                                                        1,
                                                                                        simpleSettings,
                                                                                        NULL);
                                element = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, step);
                                if(!self->interruptCoherencePathwayCalculation && element->type == SequenceTypePulse) {
                                    // multiply coefficient with expectation value for step n of coherence pathway
                                    z = expectation_value(insensitive_spinsystem_get_raw_densitymatrix(densityMatrix), orderOperator[pathway[i][index++]], size);
                                    c = complex_mul(c, z);
                                    // Remove pathways that do not change after 90° and 180° pulses
                                    if(step + 1 <= (unsigned int)insensitive_pulsesequence_get_number_of_elements(self->pulseSequence) && index > 1) { // <= is < if next line is not commented out
                                        // Only  allow pulses on the current spin type
                                        if((type == spinTypeI && element->activeISpins)
                                           || (type == spinTypeS && element->activeSSpins)) {
                                            if((element->time == 90 || element->time == 180) &&
                                               (pathway[i][index - 1] == pathway[i][index - 2] && pathway[i][index - 1] != (int)spins)) {
                                                c = complex_rect(0.0, 0.0);
                                            }
                                        } else {
                                            if(pathway[i][index - 1] != pathway[i][index - 2] && pathway[i][index - 1] != (int)spins) {
                                                c = complex_rect(0.0, 0.0);
                                            }
                                        }
                                    }
                                }
                            } else {
                                step = insensitive_pulsesequence_get_number_of_elements(self->pulseSequence);
                            }
                        }
                        coefficient[i].real += c.real;
                        coefficient[i].imag += c.imag;
                    }
                }
            } else {
                coefficient = NULL;
                pathway = NULL;
                orderOperator = NULL;
            }

            // Display the pathways via pulseSequenceController
            if(type == spinTypeI && !self->interruptCoherencePathwayCalculation)
                set_iSpin_coherencePathway_coefficients((InsensitiveWindow *)self->displayController, coefficient);
            else if(type == spinTypeS && !self->interruptCoherencePathwayCalculation)
                set_sSpin_coherencePathway_coefficients((InsensitiveWindow *)self->displayController, coefficient);

            // Free memory
            if(pathway != NULL) {
                for(i = 0; i < numberOfPathways; i++)
                    free(pathway[i]);
                free(pathway);
            }
            if(coefficient != NULL)
                free(coefficient);
            if(orderOperator != NULL) {
                for(i = 0; i < numberOfOrders; i++)
                    free(orderOperator[i]);
                free(orderOperator);
            }
        } else {
            if(type == spinTypeI)
                set_iSpin_coherencePathway_coefficients((InsensitiveWindow *)self->displayController, NULL);
            else if(type == spinTypeS)
                set_sSpin_coherencePathway_coefficients((InsensitiveWindow *)self->displayController, NULL);
        }
        // Tell the pulse sequence controller not to redraw only if this loop has reached the final round
        if(type == 1) {
            set_needsToRecalculateCoherencePathways((InsensitiveWindow *)self->displayController, FALSE);
        }
    }
    // Restore phase cycling table
    for(step = 0; step < (unsigned int)numberOfPulses && !self->interruptCoherencePathwayCalculation; step++) {
        element = g_ptr_array_index(self->pulseList, step);
        char_ptr = g_ptr_array_index(self->phaseCyclingArray, step + 1);
        element->secondParameter = atoi(char_ptr);
    }
    draw_coherencePathway((InsensitiveWindow *)self->displayController);
    //insensitive_settings_set_ignoreOffResonanceEffectsForPulses(simpleSettings, insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings));
    g_object_unref(simpleSettings);
    free(zmag);
    free(shiftminus);
    free(shiftplus);
    free(identity);

    self->interruptCoherencePathwayCalculation = TRUE;

	return FALSE;
}


gboolean insensitive_controller_coherencePathwayCalculation_is_running(InsensitiveController *self)
{
    return !self->interruptCoherencePathwayCalculation;
}


gchar *insensitive_controller_get_pulseSequence_name(InsensitiveController *self)
{
    return self->pulseSequenceName;
}


void insensitive_controller_set_name_for_pulseSequence(InsensitiveController *self, gchar *name)
{
    if(self->pulseSequenceName != NULL) {
        g_free(self->pulseSequenceName);
        self->pulseSequenceName = NULL;
    }
    if(name != NULL)
        self->pulseSequenceName = name;
}


GString *insensitive_controller_create_spectrumReport(InsensitiveController *self, gboolean takeDataFromPulseSequence)
{
    unsigned int i, j, index, numberOfChannels;
    float f1gyro, f2gyro;
    gboolean channelI = FALSE, channelS = FALSE;
    gboolean iDecoupling = FALSE, sDecoupling = FALSE;
    gboolean f1Decoupling = FALSE, f2Decoupling = FALSE;
	gboolean spinlock = FALSE;
    gboolean skip;
    unsigned int gradients = 0;
    float *gradientStrength;
    float freq1, freq2;
    unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);
    float dwellTime = insensitive_settings_get_dwellTime(self->settings);
    gchar *nuc, *pulprog, *fnmode;
    GString *parameterString;
    unsigned int numberOfElements = insensitive_pulsesequence_get_number_of_elements(self->pulseSequence);
    SequenceElement *lastElement, *element;
    GDateTime *date = g_date_time_new_now_local();

    // Check if a pulse sequence with FID acquisition is loaded
    numberOfChannels = 0;
    if(takeDataFromPulseSequence && numberOfElements > 0) {
        gradientStrength = malloc(numberOfElements * sizeof(float));
        lastElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
        if(lastElement->type == SequenceTypeFID) {
            // Check for the number of channels necessary
            for(i = 0; i < numberOfElements; i++) {
                element = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, i);
				if(element->spinlock)
                    spinlock = TRUE;
                if(element->iDecoupling)// && !spinlock)
                    iDecoupling = TRUE;
                if(element->sDecoupling)// && !spinlock)
                    sDecoupling = TRUE;
                if(element->type == SequenceTypePulse
                   || (element->type == SequenceTypeEvolution && (element->iDecoupling || element->sDecoupling))
                   || (element->type == SequenceTypeFID && (element->iDecoupling || element->sDecoupling))) {
                    if((element->activeISpins || element->selectiveIPulse) && !channelI && !spinlock && numberOfChannels < 2) {
                        channelI = TRUE;
                        numberOfChannels++;
                    }
                    if((element->activeSSpins || element->selectiveSPulse) && !channelS && !spinlock && numberOfChannels < 2) {
                        channelS = TRUE;
                        numberOfChannels++;
                    }
                } else if(element->type == SequenceTypeGradient) {
                    gradientStrength[gradients] = element->secondParameter;
                    gradients++;
                }
                /*if(numberOfChannels == 2)
                    break;*/
				spinlock = FALSE;
            }
            // Determine frequency for f1
            if(lastElement->activeISpins) {
                f1gyro = self->spinSystem->absGyroI;
                f2gyro = self->spinSystem->absGyroS;
                f1Decoupling = iDecoupling;
                f2Decoupling = sDecoupling;
            } else if(lastElement->activeSSpins) {
                f1gyro = self->spinSystem->absGyroS;
                f2gyro = self->spinSystem->absGyroI;
                f1Decoupling = sDecoupling;
                f2Decoupling = iDecoupling;
            } else {
                f1gyro = -1;
            }
        }
    }
    // If no pulse sequence with acquisition is present, use current settings
    if(numberOfChannels == 0) {
        if(insensitive_settings_get_detectISpins(self->settings) && insensitive_settings_get_detectSSpins(self->settings)) {
            f1gyro = self->spinSystem->absGyroI;
            f2gyro = self->spinSystem->absGyroS;
            f1Decoupling = insensitive_settings_get_iDecoupling(self->settings);
            f2Decoupling = insensitive_settings_get_sDecoupling(self->settings);
            numberOfChannels = 2;
        } else if(insensitive_settings_get_detectISpins(self->settings)) {
            f1gyro = self->spinSystem->absGyroI;
            f2gyro = self->spinSystem->absGyroS;
            f1Decoupling = insensitive_settings_get_iDecoupling(self->settings);
            f2Decoupling = insensitive_settings_get_sDecoupling(self->settings);
            numberOfChannels = 1;
        } else if(insensitive_settings_get_detectSSpins(self->settings)) {
            f1gyro = self->spinSystem->absGyroS;
            f2gyro = self->spinSystem->absGyroI;
            f1Decoupling = insensitive_settings_get_sDecoupling(self->settings);
            f2Decoupling = insensitive_settings_get_iDecoupling(self->settings);
            numberOfChannels = 1;
        } else {
            f1gyro = -1;
            numberOfChannels = 0;
        }
    }
    pulprog = malloc(250 * sizeof(gchar));
    nuc = malloc(19 * sizeof(gchar));
    fnmode = malloc(15 * sizeof(gchar));
    strcpy(pulprog, (self->pulseSequenceName == NULL || !takeDataFromPulseSequence) ? "no name" : self->pulseSequenceName);
    parameterString = g_string_new("Current Data Parameters     \n");
    g_string_append(parameterString, "NAME        insensitive\n");
    g_string_append_printf(parameterString, "EXPNO %17d\n", self->expno);
    g_string_append_printf(parameterString, "PROCNO %16d\n", 1);
    self->acquisitionTime /= 1e6;
    if(self->acquisitionTime > 3600)
        g_string_append_printf(parameterString, "EXPT %18.6f min\n", self->acquisitionTime / 60);
    else
        g_string_append_printf(parameterString, "EXPT %18.6f sec\n", self->acquisitionTime);
    g_string_append_printf(parameterString, "\n");
    //if(!currentSpectrumIsTwoDimensional)
    //    g_string_append_printf(parameterString, "F1"];
    //else
        g_string_append(parameterString, "F2");
    g_string_append(parameterString, " - Acquisition Parameters\n");
    g_string_append_printf(parameterString, "Date_          %4d%02d%02d\n", g_date_time_get_year(date), g_date_time_get_month(date), g_date_time_get_day_of_month(date));
    g_string_append_printf(parameterString, "Time              %02d.%02d\n", g_date_time_get_hour(date), g_date_time_get_minute(date));
    g_string_append(parameterString, "INSTRUM           spect\n");
    g_string_append(parameterString, "PROBHD    BBFO BB Z-GRD\n");
    g_string_append_printf(parameterString, "PULPROG %15.20s\n", pulprog);
    g_string_append_printf(parameterString, "TD %20d\n", dataPoints * 2);
    g_string_append(parameterString, "SOLVENT            none\n");
    g_string_append_printf(parameterString, "NS %20d\n", takeDataFromPulseSequence ? insensitive_controller_get_numberOfPhaseCycles(self) : 1);
    g_string_append_printf(parameterString, "DS %20d\n", 0);
    g_string_append_printf(parameterString, "SWH %19.3g Hz\n", 1 / dwellTime);
    g_string_append_printf(parameterString, "FIDRES %16.6f Hz\n", 1 / ((float)dataPoints * dwellTime));
    g_string_append_printf(parameterString, "AQ %20.6f sec\n", dwellTime * dataPoints * 2);
    g_string_append(parameterString, "RG                    1\n");
    g_string_append_printf(parameterString, "DW %20.3g usec\n", dwellTime * 1000);
    g_string_append_printf(parameterString, "DE %20.2f usec\n", 0.0);
    g_string_append_printf(parameterString, "TE %20.1f K\n", 300.0);
    g_string_append_printf(parameterString, "D1 %20.8f sec\n", 5 * insensitive_settings_get_T1(self->settings));
    g_string_append(parameterString, "TD0                   1\n");
    g_string_append(parameterString, "\n");
    // Pulse channel f1: the nucleus that is detected
    g_string_append(parameterString, "======== CHANNEL f1 ========\n");
    if(f1Decoupling)
        g_string_append(parameterString, "CPDPRG1         waltz16\n");
    freq1 = f1gyro / gyro_1H * spectrometer_frequency / 1e6;
    if(f1gyro == gyro_1H)
        strcpy(nuc, "                1H");
    else if(f1gyro == gyro_13C)
        strcpy(nuc, "               13C");
    else if(f1gyro == gyro_15N)
        strcpy(nuc, "               15N");
    else if(f1gyro == gyro_19F)
        strcpy(nuc, "               19F");
    else if(f1gyro == gyro_29Si)
        strcpy(nuc, "              29Si");
    else if(f1gyro == gyro_31P)
        strcpy(nuc, "               31P");
    else if(f1gyro == gyro_57Fe)
        strcpy(nuc, "              57Fe");
    else if(f1gyro == gyro_77Se)
        strcpy(nuc, "              77Se");
    else if(f1gyro == gyro_113Cd)
        strcpy(nuc, "             113Cd");
    else if(f1gyro == gyro_119Sn)
        strcpy(nuc, "             119Sn");
    else if(f1gyro == gyro_129Xe)
        strcpy(nuc, "             129Xe");
    else if(f1gyro == gyro_183W)
        strcpy(nuc, "              183W");
    else if(f1gyro == gyro_195Pt)
        strcpy(nuc, "             195Pt");
    else
        strcpy(nuc, "                ??");
    g_string_append_printf(parameterString, "NUC1 %s\n", nuc);
    if (takeDataFromPulseSequence) {
        for (i = 0; i < self->pulseList->len; i++) {
            element = g_ptr_array_index(self->pulseList, i);
            if (element->time != 180) {
                if ((f1gyro == self->spinSystem->absGyroS && element->activeSSpins) || (f1gyro == self->spinSystem->absGyroI && element->activeISpins)) {
                    g_string_append_printf(parameterString, "P1 %20.1f usec\n", 100.0 / gyro_1H * f1gyro);
                    break;
                }
            }
        }
        for (i = 0; i < self->pulseList->len; i++) {
            element = g_ptr_array_index(self->pulseList, i);
            if (element->time == 180) {
                if ((f1gyro == self->spinSystem->absGyroS && element->activeSSpins) || (f1gyro == self->spinSystem->absGyroI && element->activeISpins)) {
                    g_string_append_printf(parameterString, "P2 %20.1f usec\n", 200.0 / gyro_1H * f1gyro);
                    break;
                }
            }
        }
    }
    //if(f1gyro == [spinSystem absGyroS]) {
    //    g_string_append_printf(parameterString, "PLW3 %18.8f W\n", 17.0];
    //} else {
    //    g_string_append_printf(parameterString, "PLW1 %18.8f W\n", 17.0];
    //}
    g_string_append_printf(parameterString, "SFO1 %18.2f MHz\n", freq1);
    g_string_append(parameterString, "\n");
    // Pulse channel f2: if the pulse sequence has a second nucleus show it here
    freq2 = f2gyro / gyro_1H * spectrometer_frequency / 1e6;
    if(numberOfChannels == 2 || (!takeDataFromPulseSequence && ((iDecoupling && insensitive_settings_get_detectSSpins(self->settings))
                                                                || (sDecoupling && insensitive_settings_get_detectISpins(self->settings))))) {
        g_string_append_printf(parameterString, "======== CHANNEL f2 ========\n");
        if(f2Decoupling)
            g_string_append_printf(parameterString, "CPDPRG2         waltz16\n");
        if(f2gyro == gyro_1H)
            strcpy(nuc, "                1H");
        else if(f2gyro == gyro_13C)
            strcpy(nuc, "               13C");
        else if(f2gyro == gyro_15N)
            strcpy(nuc, "               15N");
        else if(f2gyro == gyro_19F)
            strcpy(nuc, "               19F");
        else if(f2gyro == gyro_29Si)
            strcpy(nuc, "              29Si");
        else if(f2gyro == gyro_31P)
            strcpy(nuc, "               31P");
        else if(f2gyro == gyro_57Fe)
            strcpy(nuc, "              57Fe");
        else if(f2gyro == gyro_77Se)
            strcpy(nuc, "              77Se");
        else if(f2gyro == gyro_113Cd)
            strcpy(nuc, "             113Cd");
        else if(f2gyro == gyro_119Sn)
            strcpy(nuc, "             119Sn");
        else if(f2gyro == gyro_129Xe)
            strcpy(nuc, "             129Xe");
        else if(f2gyro == gyro_183W)
            strcpy(nuc, "              183W");
        else if(f2gyro == gyro_195Pt)
            strcpy(nuc, "             195Pt");
        else
            strcpy(nuc, "                ??");
        g_string_append_printf(parameterString, "NUC2 %s\n", nuc);
        if (takeDataFromPulseSequence) {
            for (i = 0; i < self->pulseList->len; i++) {
                element = g_ptr_array_index(self->pulseList, i);
                if (element->time != 180) {
                    if ((f2gyro == self->spinSystem->absGyroS && element->activeSSpins) || (f2gyro == self->spinSystem->absGyroI && element->activeISpins)) {
                        g_string_append_printf(parameterString, "P3 %20.1f usec\n", 100.0 / gyro_1H * f2gyro);
                        break;
                    }
                }
            }
            for (i = 0; i < self->pulseList->len; i++) {
                element = g_ptr_array_index(self->pulseList, i);
                if (element->time == 180) {
                    if ((f2gyro == self->spinSystem->absGyroS && element->activeSSpins) || (f2gyro == self->spinSystem->absGyroI && element->activeISpins)) {
                        g_string_append_printf(parameterString, "P4 %20.1f usec\n", 200.0 / gyro_1H * f2gyro);
                        break;
                    }
                }
            }
        }
        g_string_append_printf(parameterString, "SFO2 %18.2f MHz\n", freq2);
        g_string_append(parameterString, "\n");
    }
    if(gradients) {
        g_string_append(parameterString, "===== GRADIENT CHANNEL =====\n");
        index = 0;
        for (i = 0; i < gradients; i++) {
            skip = FALSE;
            for (j = 0; j < i; j++) {
                if (gradientStrength[j] == gradientStrength[i]) {
                    skip = TRUE;
                    break;
                }
            }
            if (!skip) {
                index++;
                if (i < 9)
                    g_string_append_printf(parameterString, "GPNAM%d         SINE.100\n", index);
                else
                    g_string_append_printf(parameterString, "GPNAM%d        SINE.100\n", index);
            }
        }
        index = 0;
        for(i = 0; i < gradients; i++) {
            skip = FALSE;
            for(j = 0; j < i; j++) {
                if(gradientStrength[j] == gradientStrength[i]) {
                    skip = TRUE;
                    break;
                }
            }
            if(!skip) {
                index++;
                if(i < 9)
                    g_string_append_printf(parameterString, "GPZ%d %18.2f %%\n", index, gradientStrength[i] / standardGradientStrength * 100);
                else
                    g_string_append_printf(parameterString, "GPZ%d %17.2f %%\n", index, gradientStrength[i] / standardGradientStrength * 100);
            }
        }
        g_string_append(parameterString, "P16             1000.00 usec\n\n");
    }
    if(self->currentSpectrumIsTwoDimensional) {
        g_string_append(parameterString, "F1 - Acquisition Parameters\n");
        i = 128;
        if(dataPoints < i)
            i = dataPoints;
        if(self->detectionMethodOfCurrentSpectrum == TPPI  || self->detectionMethodOfCurrentSpectrum == StatesTPPI)
            i *= 2;
        g_string_append_printf(parameterString, "TD %20d\n", i * 2);
        g_string_append_printf(parameterString, "FIDRES %16.6f Hz\n", 1 / (indirect_datapoints(self->detectionMethodOfCurrentSpectrum, dataPoints) * dwellTime));
        switch(self->detectionMethodOfCurrentSpectrum) {
            case QSEQ:
                strcpy(fnmode, "QSEQ");
                break;
            case States:
                strcpy(fnmode, "States");
                break;
            case TPPI:
                strcpy(fnmode, "TPPI");
                break;
            case StatesTPPI:
                strcpy(fnmode, "States-TPPI");
                break;
            case EchoAntiecho:
                strcpy(fnmode, "Echo-Antiecho");
                break;
            default:
                strcpy(fnmode, "QF");
        }
        g_string_append_printf(parameterString, "FnMode %16.16s\n", fnmode);
        g_string_append_printf(parameterString, "\n");
    }
    free(nuc);
    free(pulprog);
    free(fnmode);
	if(takeDataFromPulseSequence && numberOfElements > 0)
		free(gradientStrength);
    g_date_time_unref(date);

    return parameterString;
}


GString *insensitive_controller_get_spectrumReport(InsensitiveController *self)
{
    return self->spectrumReport;
}


void insensitive_controller_set_spectrumReport(InsensitiveController *self, gchar *report)
{
    if(self->spectrumReport != NULL) {
        g_string_free(self->spectrumReport, TRUE);
        self->spectrumReport = NULL;
    }
    if(report != NULL)
        self->spectrumReport = g_string_new(report);
}


 /////   //////  //////  //    // // /////// // //////// //  //////  ///    //
//   // //      //    // //    // // //      //    //    // //    // ////   //
/////// //      //    // //    // // /////// //    //    // //    // // //  //
//   // //      // // // //    // //      // //    //    // //    // //  // //
//   //  //////  //////   //////  // /////// //    //    //  //////  //   ////
                    //

gboolean insensitive_controller_get_acquisitionIsInProgress(InsensitiveController *self)
{
    return self->acquisitionIsInProgress;
}


void insensitive_controller_reset_acquisition_for_dataPoints(InsensitiveController *self, unsigned int number)
{
    if (self->fid.realp != NULL)
        free(self->fid.realp);
    if (self->fid.imagp != NULL)
        free(self->fid.imagp);
    if (self->spectrum1D.realp != NULL)
        free(self->spectrum1D.realp);
    if (self->spectrum1D.imagp != NULL)
        free(self->spectrum1D.imagp);
    if (self->spectrum2D.realp != NULL)
        free(self->spectrum2D.realp);
    if (self->spectrum2D.imagp != NULL)
        free(self->spectrum2D.imagp);
    if (self->spectrumSymmetrized.realp != NULL)
        free(self->spectrumSymmetrized.realp);
    if (self->spectrumSymmetrized.imagp != NULL)
        free(self->spectrumSymmetrized.imagp);
    self->fid.realp = malloc(number * sizeof(float));
    self->fid.imagp = malloc(number * sizeof(float));
    self->spectrum1D.realp = malloc(number * sizeof(float));
    self->spectrum1D.imagp = malloc(number * sizeof(float));
    self->spectrum2D.realp = malloc(number * sizeof(float));
    self->spectrum2D.imagp = malloc(number * sizeof(float));
    self->spectrumSymmetrized.realp = malloc(number * sizeof(float));
    self->spectrumSymmetrized.imagp = malloc(number * sizeof(float));
    // Begin only needed for Stated method
    if(insensitive_settings_get_detectionMethod(self->settings) == States || insensitive_settings_get_detectionMethod(self->settings) == StatesTPPI
       || self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
        if (self->fidStates.realp != NULL)
            free(self->fidStates.realp);
        if (self->fidStates.imagp != NULL)
            free(self->fidStates.imagp);
        if (self->spectrum1DStates.realp != NULL)
            free(self->spectrum1DStates.realp);
        if (self->spectrum1DStates.imagp != NULL)
            free(self->spectrum1DStates.imagp);
        self->fidStates.realp = malloc(number * sizeof(float));
        self->fidStates.imagp = malloc(number * sizeof(float));
        self->spectrum1DStates.realp = malloc(number * sizeof(float));
        self->spectrum1DStates.imagp = malloc(number * sizeof(float));
    }
    // End only needed by States method
    self->totalDataPointsInSER = number;
    self->recordedDataPointsInFID = 0;
    self->firstDataPointInFID = 0;
    self->acquisitionTime = 0.0;
    // Should call this on main thread, but messes up perform_open_spectrum
    //g_idle_add((GSourceFunc)reset_spectrum_display, (InsensitiveWindow *)self->displayController);
    reset_spectrum_display((InsensitiveWindow *)self->displayController);
    enable_fft_along_t1((InsensitiveWindow *)self->displayController, FALSE);
    enable_fft_along_t2((InsensitiveWindow *)self->displayController, FALSE);
    enable_symmerization((InsensitiveWindow *)self->displayController, FALSE);
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->fid,
                         self->recordedDataPointsInFID,
                         insensitive_settings_get_dataPoints(self->settings));
    insensitive_controller_set_noiseLevel(self, insensitive_settings_get_noiseLevel(self->settings));
    insensitive_controller_set_windowFunction(self, self->windowFunction);
}


void insensitive_controller_perform_acquisition(InsensitiveController *self)
{
    if (insensitive_settings_get_pulseBeforeAcquisition(self->settings)) {
        insensitive_controller_set_pulseBeforeAcquisition(self, FALSE);
        insensitive_controller_set_acquisitionAfterNextPulse(self, TRUE);
        insensitive_controller_perform_pulse_animated(self, FALSE);
    } else {
        self->currentSpectrumIsTwoDimensional = FALSE;
        self->expno++;
        insensitive_controller_save_previous_state(self);
        self->previousPulseArray = insensitive_settings_get_pulseArray(self->settings);
        insensitive_controller_reset_acquisition_for_dataPoints(self, insensitive_settings_get_dataPoints(self->settings));
        set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
        self->acquisitionTime = (float)g_get_monotonic_time();
        if (self->spectrumReport != NULL) {
            g_string_free(self->spectrumReport, TRUE);
            self->spectrumReport = NULL;
        }
        reset_window_function((InsensitiveWindow *)self->displayController);
        show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);
        // Recording operations
        if (self->isRecordingPulseSequence) {
            insensitive_pulsesequence_add_element(self->pulseSequence, SequenceTypeFID, self->settings);
            update_pulseSequence((InsensitiveWindow *)self->displayController);
            insensitive_controller_toggle_recordingPulseSequence(self);
        }
        self->relaxationWasIncludedBefore = insensitive_settings_get_relaxationWithEvolution(self->settings);
        insensitive_controller_set_relaxation_with_evolution(self, TRUE);
        self->acquisitionIsInProgress = TRUE;
        set_acquisition_is_running((InsensitiveWindow *)self->displayController, TRUE);
        set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
        enable_animation_checkbox((InsensitiveWindow *)self->displayController, FALSE);
        set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
        set_display_frequency_domain((InsensitiveWindow *)self->displayController, FALSE);
        go_to_fft_panel((InsensitiveWindow *)self->displayController);
        allow_spectrum_acquisition((InsensitiveWindow *)self->displayController, FALSE);
        update_phaseCyclingTable((InsensitiveWindow *)self->displayController, self->pulseList->len + 1);
        if (!self->animationIsInProgress)
            insensitive_controller_set_animates(self, TRUE);
    }
}


void insensitive_controller_perform_2D_acquisition(InsensitiveController *self)
{
    unsigned int i, t1DataPoints, t2DataPoints, totalDataPoints;

	self->expno++;
    self->acquisitionProgress = 0;
    self->spectrumDataAvailable = FALSE;
	self->interruptAcquisition = FALSE;
	self->currentSpectrumIsTwoDimensional = TRUE;
    self->acquisitionIsInProgress = TRUE;
	self->detectionMethodOfCurrentSpectrum = insensitive_settings_get_detectionMethod(self->settings);
    self->acquisitionTime = (float)g_get_monotonic_time();
    self->relaxationWasIncludedBefore = insensitive_settings_get_relaxationWithEvolution(self->settings);
    self->previousPulseArray = insensitive_settings_get_pulseArray(self->settings);
    insensitive_controller_save_decoupling(self);
    insensitive_controller_set_relaxation_with_evolution(self, TRUE);
    insensitive_controller_set_animates(self, FALSE);
	insensitive_controller_save_previous_state(self);
    insensitive_controller_set_currentStepInPulseSequence(self, 0);
    t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
    t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
    totalDataPoints = t1DataPoints * t2DataPoints;
    insensitive_controller_reset_acquisition_for_dataPoints(self, totalDataPoints);
    for (i = 0; i < totalDataPoints; i++) {
        self->fid.realp[i] = 0;
        self->fid.imagp[i] = 0;
        if(self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
            self->fidStates.realp[i] = 0;
            self->fidStates.imagp[i] = 0;
        }
    }
	reset_window_function((InsensitiveWindow *)self->displayController);
	set_2D_mode((InsensitiveWindow *)self->displayController, TRUE);
	enable_fft_along_t1((InsensitiveWindow *)self->displayController, FALSE);
	enable_fft_along_t2((InsensitiveWindow *)self->displayController, FALSE);
	enable_symmerization((InsensitiveWindow *)self->displayController, FALSE);
	set_acquisition_is_running((InsensitiveWindow *)self->displayController, TRUE);
	set_user_controls_enabled((InsensitiveWindow *)self->displayController, FALSE);
	enable_animation_checkbox((InsensitiveWindow *)self->displayController, FALSE);
	if (insensitive_settings_get_showRealPart(self->settings) && insensitive_settings_get_showImaginaryPart(self->settings))
		insensitive_controller_set_showImaginaryPart(self, FALSE);
	show_spectrum_progressbar((InsensitiveWindow *)self->displayController, TRUE);
    set_spectrum_progressbar_maximum((InsensitiveWindow *)self->displayController, self->phaseCycles * totalDataPoints);
    set_dataPoints_label((InsensitiveWindow *)self->displayController, t2DataPoints, totalDataPoints);

	g_thread_new("2DAcquisitionThread", insensitive_controller_perform_2D_acquisition_in_background, self);
}


gboolean insensitive_controller_perform_2D_acquisition_in_background(gpointer data)
{
	InsensitiveController *self = (InsensitiveController *)data;

	unsigned int i, j, t1, t2, t1DataPoints, t2DataPoints, totalDataPoints, offset;
	unsigned int position, rows, numberOfPulsesBeforeT1 = 0;
	int phaseCyclingIndex, cycle;
	float originalEvolutionTime, factor, shiftedPhase, dwellTime;
	DSPComplex z, receiverPhase;
	SequenceElement *element, *fidElement, *variableElement1, *variableElement2;
	gboolean detectISpins, detectSSpins;

	// Initiate the parameters
    self->acquisitionTime = (float)g_get_monotonic_time();
	factor = 2 * insensitive_spinsystem_get_spins(self->spinSystem);
	t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
	t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
	totalDataPoints = t1DataPoints * t2DataPoints;
	dwellTime = insensitive_settings_get_dwellTime(self->settings);

	// Check for FID in pulse sequence (This is chcked before and never happens.)
	fidElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
	if (fidElement->type != SequenceTypeFID) {
		insensitive_pulsesequence_add_element(self->pulseSequence, SequenceTypeFID, self->settings);
		gdk_threads_add_timeout(10, (GSourceFunc)redraw_pulseSequence, (InsensitiveWindow *)self->displayController);
		if (self->isRecordingPulseSequence)
			insensitive_controller_toggle_recordingPulseSequence(self);
		detectISpins = insensitive_settings_get_detectISpins(self->settings);
		detectSSpins = insensitive_settings_get_detectSSpins(self->settings);
		fidElement = insensitive_pulsesequence_get_last_element(self->pulseSequence);
	} else {
		detectISpins = fidElement->activeISpins;
		detectSSpins = fidElement->activeSSpins;
	}
	// Identify the variable evolution time
	if (self->indexForVariableEvolutionTime > 0) {
		variableElement1 = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, self->indexForVariableEvolutionTime - 1);
		variableElement2 = NULL;
		originalEvolutionTime = variableElement1->time;
	} else if (insensitive_pulsesequence_get_element_at_index(self->pulseSequence, -self->indexForVariableEvolutionTime - 1)->type == SequenceTypeEvolution) {
		variableElement1 = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, -self->indexForVariableEvolutionTime - 1);
		if (insensitive_pulsesequence_get_element_at_index(self->pulseSequence, -self->indexForVariableEvolutionTime)->type == SequenceTypeGradient) {
			variableElement2 = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, -self->indexForVariableEvolutionTime + 3);
		} else {
			variableElement2 = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, -self->indexForVariableEvolutionTime + 1);
		}
		originalEvolutionTime = variableElement1->time;
	}
	// Determine number of pulses before t1 for States or TPPI
	if (self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == TPPI || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
		SequenceElement *element;
		for (i = 0; i < (unsigned int)abs(self->indexForVariableEvolutionTime) - 1; i++) {
			element = insensitive_pulsesequence_get_element_at_index(self->pulseSequence, i);
			if (element->type == SequenceTypePulse)
				numberOfPulsesBeforeT1++;
		}
	}
	// If States method is used the spectrum has to be samples again with 90° phase before t1
    if (self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
        // Phase shift all pulses before t1 +90°
        for (cycle = 0; cycle < (int)self->phaseCycles; cycle++) {
            rows = self->pulseList->len + 1;
            for (i = 1; i <= numberOfPulsesBeforeT1; i++) {
                position = cycle * rows + i;
                shiftedPhase = atof(g_ptr_array_index(self->phaseCyclingArray, position)) + 90;
                sprintf(g_ptr_array_index(self->phaseCyclingArray, position), "%.0f", shiftedPhase);
            }
        }
        // Cosine modulated 2D acquisition
        // Loop through phase cycles backwards to end with the first position
        for (cycle = self->phaseCycles - 1; cycle >= 0 && !self->interruptAcquisition; cycle--) {
            phaseCyclingIndex = cycle * (self->pulseList->len + 1);
            for (i = 1; i <= self->pulseList->len && !self->interruptAcquisition; i++) {
                element = g_ptr_array_index(self->pulseList, i - 1);
                element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex + i));
            }
            receiverPhase = complex_rect(cos(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI),
                                         -sin(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI));
            // Loop through variable delay times (t1)
            for (t1 = 0; t1 < t1DataPoints && !self->interruptAcquisition; t1++) {
	            insensitive_spinsystem_return_to_thermal_equilibrium(self->spinSystem);
                insensitive_controller_set_currentStepInPulseSequence(self, 0);
                // Set phase shifts for States-TPPI method
                if(self->detectionMethodOfCurrentSpectrum == StatesTPPI ) {
                    // Phase shift all pulses before t1 by 90°
                    for (j = 0; j < self->phaseCycles && !self->interruptAcquisition; j++) {
                        rows = self->pulseList->len + 1;
                        for (i = 0; i <= numberOfPulsesBeforeT1; i++) {
                            position = j * rows + i;
                            shiftedPhase = atof(g_ptr_array_index(self->phaseCyclingArray, position)) - 180;
                            if(shiftedPhase < 0)
                                shiftedPhase += 360;
                            sprintf(g_ptr_array_index(self->phaseCyclingArray, position), "%.0f", shiftedPhase);
                        }
                    }
                    for (i = 1; i <= numberOfPulsesBeforeT1 && !self->interruptAcquisition; i++) {
                        element = g_ptr_array_index(self->pulseList, i - 1);
                        element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex + i));
                    }
                    if(self->indexForVariableEvolutionTime > 0) {
                        variableElement1->time = t1 * dwellTime * 0.5;
                    } else {
                        variableElement1->time = t1 * dwellTime * 0.25;
                        variableElement2->time = t1 * dwellTime * 0.25;
                    }
                // End preparation for States-TPPI
                } else {
                    if(self->indexForVariableEvolutionTime > 0) {
                        variableElement1->time = t1 * dwellTime;
                    } else {
                        variableElement1->time = t1 * dwellTime * 0.5;
                        variableElement2->time = t1 * dwellTime * 0.5;
                    }
                }
                insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence, self->spinSystem, 0, 0, self->settings, NULL);
                insensitive_settings_set_spinlock(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->spinlock);
                insensitive_settings_set_iDecoupling(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->iDecoupling);
                insensitive_settings_set_sDecoupling(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->sDecoupling);
                offset = t1 * t2DataPoints;
                // Collect FID (t2)
                for (t2 = 0; t2 < t2DataPoints && !self->interruptAcquisition; t2++) {
                    if (insensitive_settings_get_zeroFilling(self->settings) && (t2 > t2DataPoints / 2)) {
                        self->fidStates.realp[t2 + offset] += 0;
                        self->fidStates.imagp[t2 + offset] += 0;
                    } else {
                        z.real = 0;
                        z.imag = 0;
                        for (i = 0; i < insensitive_spinsystem_get_spins(self->spinSystem) && !self->interruptAcquisition; i++)
                            if (((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeI) && detectISpins)
                               || ((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeS) && detectSSpins)) {
                                z.real -= insensitive_spinsystem_get_expectationvalue_y_for_spin(self->spinSystem, i) / factor;
                                z.imag += insensitive_spinsystem_get_expectationvalue_x_for_spin(self->spinSystem, i) / factor;
                            }
                        z = complex_mul(z, receiverPhase);
                        self->fidStates.realp[t2 + offset] += z.real;
                        self->fidStates.imagp[t2 + offset] += z.imag;
                        // Perform free evolution
                        if (insensitive_settings_get_spinlock(self->settings))
                            insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
                        insensitive_spinsystem_chemicalshift(self->spinSystem, dwellTime, insensitive_settings_get_dephasingJitter(self->settings));
                        if (insensitive_spinsystem_get_spins(self->spinSystem) > 1) {
                            if (fidElement->iDecoupling || fidElement->sDecoupling) {
                                insensitive_spinsystem_jcoupling(self->spinSystem,
                                                                 dwellTime / 2,
                                                                 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                                insensitive_spinsystem_perform_decoupling(self->spinSystem,
                                                                          fidElement->iDecoupling,
                                                                          fidElement->sDecoupling,
                                                                          insensitive_settings_get_phase(self->settings));
                                insensitive_spinsystem_jcoupling(self->spinSystem,
                                                                 dwellTime / 2,
                                                                 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                            } else
                                insensitive_spinsystem_jcoupling(self->spinSystem,
                                                                 dwellTime,
                                                                 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
                        }
                        if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
		                    if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
			                    if (insensitive_settings_get_spinlock(self->settings))
				                    insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem,
										                                               dwellTime,
										                                               insensitive_settings_get_correlationTime(self->settings));
			                    else
				                    insensitive_spinsystem_dipolarrelaxation(self->spinSystem,
									                                         dwellTime,
									                                         insensitive_settings_get_correlationTime(self->settings));
		                    } else
			                    insensitive_spinsystem_simplerelaxation(self->spinSystem,
								                                        dwellTime,
							                                	        insensitive_settings_get_T1(self->settings),
							                                	        insensitive_settings_get_T2(self->settings),
							                                	        insensitive_settings_get_spinlock(self->settings));
	                    }
                        if(insensitive_settings_get_spinlock(self->settings))
                            insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
                    }
                    if(t2 % 2 == 0)
                        gdk_threads_add_idle((GSourceFunc)insensitive_controller_update_interface_during_2D_acquisition, self);
                }
                gdk_threads_add_timeout(10, (GSourceFunc)redraw_pulseSequence, (InsensitiveWindow *)self->displayController);
            }
        }
        // Phase shift all pulses before t1 -90°
        for (cycle = 0; cycle < (int)self->phaseCycles; cycle++) {
            rows = self->pulseList->len + 1;
            for (i = 1; i <= self->pulseList->len; i++) {
                if (i <= numberOfPulsesBeforeT1) {
                    position = cycle * rows + i;
                    shiftedPhase = atof(g_ptr_array_index(self->phaseCyclingArray, position)) - 90;
                    sprintf(g_ptr_array_index(self->phaseCyclingArray, position), "%.0f", shiftedPhase);
                }
            }
        }
    }
	// Sine modulated 2D acquisition
	// Loop through phase cycles backwards to end with the first position
	for (cycle = self->phaseCycles - 1; cycle >= 0 && !self->interruptAcquisition; cycle--) {
		phaseCyclingIndex = cycle * (self->pulseList->len + 1);
		for (i = 1; i <= self->pulseList->len && !self->interruptAcquisition; i++) {
			element = g_ptr_array_index(self->pulseList, i - 1);
			element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex + i));
		}
		receiverPhase = complex_rect(cos(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI),
					     -sin(atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex)) / 180 * M_PI));
		// Loop through variable delay times (t1)
		for (t1 = 0; t1 < t1DataPoints && !self->interruptAcquisition; t1++) {
			// Set phase shifts for TPPI method
			if (self->detectionMethodOfCurrentSpectrum == TPPI || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
				// Phase shift all pulses before t1 by 90°
				for (j = 0; j < self->phaseCycles && !self->interruptAcquisition; j++) {
					rows = self->pulseList->len + 1;
					for (i = (self->detectionMethodOfCurrentSpectrum == StatesTPPI) ? 0 : 1; i <= numberOfPulsesBeforeT1; i++) {
						position = j * rows + i;
						shiftedPhase = atof(g_ptr_array_index(self->phaseCyclingArray, position)) - ((self->detectionMethodOfCurrentSpectrum == StatesTPPI) ? 180 : 90);
						if (shiftedPhase < 0)
							shiftedPhase += 360;
						sprintf(g_ptr_array_index(self->phaseCyclingArray, position), "%.0f", shiftedPhase);
					}
				}
				for (i = 1; i <= numberOfPulsesBeforeT1 && !self->interruptAcquisition; i++) {
					element = g_ptr_array_index(self->pulseList, i - 1);
					element->secondParameter = atof(g_ptr_array_index(self->phaseCyclingArray, phaseCyclingIndex + i));
				}
				if (self->indexForVariableEvolutionTime > 0) {
					variableElement1->time = t1 * dwellTime * 0.5;
				} else {
					variableElement1->time = t1 * dwellTime * 0.25;
					variableElement2->time = t1 * dwellTime * 0.25;
				}
				// End preparation for TPPI
			} else {
				if (self->indexForVariableEvolutionTime > 0) {
					variableElement1->time = t1 * dwellTime;
				} else {
					variableElement1->time = t1 * dwellTime * 0.5;
					variableElement2->time = t1 * dwellTime * 0.5;
				}
			}
			insensitive_spinsystem_return_to_thermal_equilibrium(self->spinSystem);
			insensitive_controller_set_currentStepInPulseSequence(self, 0);
			insensitive_pulsesequence_perform_actions_on_spinsystem(self->pulseSequence, self->spinSystem, 0, 0, self->settings, NULL);
			insensitive_settings_set_spinlock(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->spinlock);
			insensitive_settings_set_iDecoupling(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->iDecoupling);
			insensitive_settings_set_sDecoupling(self->settings, insensitive_pulsesequence_get_last_element(self->pulseSequence)->sDecoupling);
			offset = t1 * t2DataPoints;
			// Collect FID (t2)
			for (t2 = 0; t2 < t2DataPoints && !self->interruptAcquisition; t2++) {
				if (insensitive_settings_get_zeroFilling(self->settings) && (t2 > t2DataPoints / 2)) {
					self->fid.realp[t2 + offset] += 0;
					self->fid.imagp[t2 + offset] += 0;
				} else {
					z.real = 0;
					z.imag = 0;
					for (i = 0; i < insensitive_spinsystem_get_spins(self->spinSystem) && !self->interruptAcquisition; i++)
						if (((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeI) && detectISpins)
						    || ((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeS) && detectSSpins)) {
							z.real -= insensitive_spinsystem_get_expectationvalue_y_for_spin(self->spinSystem, i) / factor;
							z.imag += insensitive_spinsystem_get_expectationvalue_x_for_spin(self->spinSystem, i) / factor;
						}
					z = complex_mul(z, receiverPhase);
					self->fid.realp[t2 + offset] += z.real;
					self->fid.imagp[t2 + offset] += z.imag;
					// Perform free evolution
					if (insensitive_settings_get_spinlock(self->settings))
						insensitive_spinsystem_switchtospinlockmode(self->spinSystem, TRUE);
					insensitive_spinsystem_chemicalshift(self->spinSystem, dwellTime, insensitive_settings_get_dephasingJitter(self->settings));
					if (insensitive_spinsystem_get_spins(self->spinSystem) > 1) {
						if (fidElement->iDecoupling || fidElement->sDecoupling) {
							insensitive_spinsystem_jcoupling(self->spinSystem,
											 dwellTime / 2,
											 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
							insensitive_spinsystem_perform_decoupling(self->spinSystem,
												  fidElement->iDecoupling,
												  fidElement->sDecoupling,
												  insensitive_settings_get_phase(self->settings));
							insensitive_spinsystem_jcoupling(self->spinSystem,
											 dwellTime / 2,
											 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
						} else
							insensitive_spinsystem_jcoupling(self->spinSystem,
											 dwellTime,
											 insensitive_settings_get_strongCoupling(self->settings) ? StrongCouplingMode : WeakCouplingMode);
					}
					if (insensitive_settings_get_relaxationWithEvolution(self->settings)) {
						if (insensitive_settings_get_dipolarRelaxation(self->settings) && (insensitive_spinsystem_get_spins(self->spinSystem) > 1)) {
							if (insensitive_settings_get_spinlock(self->settings))
								insensitive_spinsystem_transversedipolarrelaxation(self->spinSystem,
														   dwellTime,
														   insensitive_settings_get_correlationTime(self->settings));
							else
								insensitive_spinsystem_dipolarrelaxation(self->spinSystem,
													 dwellTime,
													 insensitive_settings_get_correlationTime(self->settings));
						} else
							insensitive_spinsystem_simplerelaxation(self->spinSystem,
												dwellTime,
												insensitive_settings_get_T1(self->settings),
												insensitive_settings_get_T2(self->settings),
												insensitive_settings_get_spinlock(self->settings));
					}
					if (insensitive_settings_get_spinlock(self->settings))
						insensitive_spinsystem_switchtospinlockmode(self->spinSystem, FALSE);
				}
				if ((self->detectionMethodOfCurrentSpectrum != States && self->detectionMethodOfCurrentSpectrum != StatesTPPI) || (t2 % 2 != 0))
					gdk_threads_add_idle((GSourceFunc)insensitive_controller_update_interface_during_2D_acquisition, self);
			}
			gdk_threads_add_timeout(10, (GSourceFunc)redraw_pulseSequence, (InsensitiveWindow *)self->displayController);
		}
	}
	// Restore original values in pulse sequence
	variableElement1->time = originalEvolutionTime;
	if (self->indexForVariableEvolutionTime < 0)
		variableElement2->time = originalEvolutionTime;
	// If the acquisition was not interrupted, submit the spectrum and report
	if (!self->interruptAcquisition) {
		self->acquisitionTime = (float)g_get_monotonic_time() - self->acquisitionTime;
		if (self->spectrumReport != NULL)
			g_string_free(self->spectrumReport, TRUE);
		self->spectrumReport = insensitive_controller_create_spectrumReport(self, TRUE);
		set_indirect_dataPoints((InsensitiveWindow *)self->displayController, t1DataPoints);
		set_complex_spectrum((InsensitiveWindow *)self->displayController,
				     self->fid, t2DataPoints, totalDataPoints);
		self->spectrumDataAvailable = TRUE;
	} else {
		self->spectrumDataAvailable = FALSE;
	}
	gdk_threads_add_idle((GSourceFunc)insensitive_controller_finish_perform_2D_pulseSequence, self);

	return FALSE;
}


gboolean insensitive_controller_update_interface_during_2D_acquisition(gpointer data)
{
    InsensitiveController *self = (InsensitiveController *)data;

    add_to_spectrum_progressbar((InsensitiveWindow *)self->displayController, ++self->acquisitionProgress);

	return FALSE;
}


gboolean insensitive_controller_finish_perform_2D_pulseSequence(gpointer data)
{
    InsensitiveController *self = (InsensitiveController *)data;

    if (self->spectrumDataAvailable) {
        enable_fft_along_t1((InsensitiveWindow *)self->displayController, TRUE);
        enable_fft_along_t2((InsensitiveWindow *)self->displayController, TRUE);
        enable_symmerization((InsensitiveWindow *)self->displayController, TRUE);
        go_to_fft_panel((InsensitiveWindow *)self->displayController);
        show_spectrumParameters_textview((InsensitiveWindow *)self->displayController, TRUE);
        if (insensitive_settings_get_playSoundAfterAcquisition(self->settings))
            play_sound((InsensitiveWindow *)self->displayController);
    }
    insensitive_controller_restore_relaxation_with_evolution(self);
    insensitive_controller_restore_decoupling(self);
    insensitive_controller_stop_acquisition(self);
    redraw_pulseSequence((InsensitiveWindow *)self->displayController);

    // Make sure that the correct pulse settings are displayed
    set_flipAngle((InsensitiveWindow *)self->displayController, insensitive_settings_get_flipAngle(self->settings));
    set_phase((InsensitiveWindow *)self->displayController, insensitive_settings_get_phase(self->settings));
    insensitive_settings_set_pulseArray(self->settings,
                                        self->previousPulseArray,
                                        insensitive_spinsystem_get_spins(self->spinSystem),
                                        insensitive_spinsystem_get_spintypearray(self->spinSystem));
    set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
    set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
    set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
    set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
    show_mainWindow_notebook_page((InsensitiveWindow *)self->displayController, 3);

	return FALSE;
}


void insensitive_controller_get_first_trace_of_2D_spectrum(InsensitiveController *self, gboolean state)
{
    int t2DataPoints, totalDataPoints;

    t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
    if(insensitive_settings_get_showRealPart(self->settings)) {
        insensitive_controller_set_showImaginaryPart(self, TRUE);
        insensitive_controller_set_showRealPart(self, FALSE);
    } else {
        insensitive_controller_set_showImaginaryPart(self, FALSE);
        insensitive_controller_set_showRealPart(self, TRUE);
    }
    if(state) {
        set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
				     self->fid, t2DataPoints, t2DataPoints);
    } else {
        set_2D_mode((InsensitiveWindow *)self->displayController, TRUE);
        totalDataPoints = t2DataPoints * indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
				     self->fid, t2DataPoints, totalDataPoints);
    }
}


void insensitive_controller_acquire_dataPoint(InsensitiveController *self)
{
    float factor;
    unsigned int currentIndex = self->firstDataPointInFID + self->recordedDataPointsInFID;
    DSPComplex z, receiverPhase;
    unsigned int i;
    unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);
    unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);

    if(currentIndex < self->totalDataPointsInSER) {
        if(self->recordedDataPointsInFID < dataPoints) {
            receiverPhase = complex_rect(cos(atof(g_ptr_array_index(self->phaseCyclingArray, 0)) / 180 * M_PI),
                                         -sin(atof(g_ptr_array_index(self->phaseCyclingArray, 0)) / 180 * M_PI));
            z.real = 0;
            z.imag = 0;
            factor = 2 * spins;
            for (i = 0; i < spins; i++)
                if (((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeI) && insensitive_settings_get_detectISpins(self->settings))
                   || ((insensitive_spinsystem_get_spintype_for_spin(self->spinSystem, i) == spinTypeS) && insensitive_settings_get_detectSSpins(self->settings))) {
                    z.real -= insensitive_spinsystem_get_expectationvalue_y_for_spin(self->spinSystem, i) / factor;
                    z.imag += insensitive_spinsystem_get_expectationvalue_x_for_spin(self->spinSystem, i) / factor;
                }
            z = complex_mul(z, receiverPhase);
            self->fid.realp[currentIndex] = z.real;
            self->fid.imagp[currentIndex] = z.imag;
            self->recordedDataPointsInFID++;
        }
    }
    if (insensitive_settings_get_zeroFilling(self->settings)) {
        if (self->recordedDataPointsInFID == dataPoints / 2) {
            for (i = self->recordedDataPointsInFID; i < dataPoints; i++) {
                self->fid.realp[i] = 0;
                self->fid.imagp[i] = 0;
            }
            self->recordedDataPointsInFID *= 2;
            insensitive_controller_stop_acquisition(self);
            if(insensitive_settings_get_playSoundAfterAcquisition(self->settings))
                play_sound((InsensitiveWindow *)self->displayController);
        }
    } else {
        if (self->recordedDataPointsInFID == dataPoints) {
            insensitive_controller_stop_acquisition(self);
            if(insensitive_settings_get_playSoundAfterAcquisition(self->settings))
                play_sound((InsensitiveWindow *)self->displayController);
        }
    }
}


void insensitive_controller_interrupt_acquisition(InsensitiveController *self)
{
    // Let threaded 2D acquisition call stopAcquisition itself
    self->interruptAcquisition = TRUE;
}


void insensitive_controller_stop_acquisition(InsensitiveController *self)
{
    self->interruptAcquisition = TRUE;
    show_spectrum_progressbar((InsensitiveWindow *)self->displayController, FALSE);
    set_acquisition_is_running((InsensitiveWindow *)self->displayController, FALSE);
    self->acquisitionIsInProgress = FALSE;
    set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
    stop_progress_indicator((InsensitiveWindow *)self->displayController);
    enable_animation_checkbox((InsensitiveWindow *)self->displayController, TRUE);
    spin_state_was_changed((InsensitiveWindow *)self->displayController);
    allow_spectrum_acquisition((InsensitiveWindow *)self->displayController, TRUE);
    enable_pulseSequence_play_button((InsensitiveWindow *)self->displayController, TRUE);
    insensitive_spinsystem_free_gradient_array(self->spinSystem);
    insensitive_controller_set_animates(self, FALSE);
    insensitive_controller_restore_relaxation_with_evolution(self);
    insensitive_controller_restore_decoupling(self);
    if (!shows_2D_spectrum((InsensitiveWindow *)self->displayController)) {
        if(self->recordedDataPointsInFID == insensitive_settings_get_dataPoints(self->settings)) {
            self->spectrumDataAvailable = TRUE;
            enable_fft_along_t1((InsensitiveWindow *)self->displayController, TRUE);
            enable_symmerization((InsensitiveWindow *)self->displayController, TRUE);
        } else {
            self->spectrumDataAvailable = FALSE;
        }
    }
}


void insensitive_controller_inject_spectrum(InsensitiveController *self, float *real, float *imag,
                                            float *realStates, float *imagStates,
                                            unsigned int size, unsigned int stride, int domain)
{
    unsigned int i;

    if(self->pulseSequenceName != NULL) {
        free(self->pulseSequenceName);
        self->pulseSequenceName = NULL;
    }
    if(self->spectrumReport != NULL) {
        g_string_free(self->spectrumReport, TRUE);
        self->spectrumReport = NULL;
    }
    insensitive_controller_set_zeroFilling(self, FALSE);
    insensitive_controller_set_dataPoints(self, stride);
    insensitive_controller_reset_acquisition_for_dataPoints(self, size);
    for(i = 0; i < size; i++) {
        self->fid.realp[i] = real[i];
        self->fid.imagp[i] = imag[i];
        if((self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == StatesTPPI)
           && realStates != NULL && imagStates != NULL) {
            self->fidStates.realp[i] = realStates[i];
            self->fidStates.imagp[i] = imagStates[i];
        }
    }
    self->spectrumDataAvailable = TRUE;
    if(size > stride) {
        set_2D_mode((InsensitiveWindow *)self->displayController, TRUE);
        enable_fft_along_t2((InsensitiveWindow *)self->displayController, TRUE);
    } else {
        set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
    }
    enable_fft_along_t1((InsensitiveWindow *)self->displayController, TRUE);
    enable_symmerization((InsensitiveWindow *)self->displayController, TRUE);
    switch(domain) {
    case 1:
        on_fft1D_button_clicked(NULL, (InsensitiveWindow *)self->displayController);
        break;
    case 2:
        on_fft2D_button_clicked(NULL, (InsensitiveWindow *)self->displayController);
        break;
    case 3:
        on_magnitude_button_clicked(NULL, (InsensitiveWindow *)self->displayController);
        break;
    default:
        on_fid_button_clicked(NULL, (InsensitiveWindow *)self->displayController);
    }
}


void insensitive_controller_show_FID(InsensitiveController *self)
{
    set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseTime);
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->fid,
                         insensitive_settings_get_dataPoints(self->settings),
                         insensitive_settings_get_dataPoints(self->settings));
}


void insensitive_controller_show_SER(InsensitiveController *self)
{
    unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);
    unsigned int totalDataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, dataPoints);
    totalDataPoints *= dataPoints;

    set_indirect_dataPoints((InsensitiveWindow *)self->displayController,
                            indirect_datapoints(self->detectionMethodOfCurrentSpectrum, dataPoints));

    if((self->detectionMethodOfCurrentSpectrum == States) && !self->realDataSetsForStatesMethod)
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->fidStates, dataPoints, totalDataPoints);
    else
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->fid, dataPoints, totalDataPoints);
}


void insensitive_controller_fourier_transform_1D_spectrum(InsensitiveController *self)
{
    unsigned int i, center;
    DSPSplitComplex foldedSpectrum;
    unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);

    foldedSpectrum.realp = malloc(dataPoints * sizeof(float));
    foldedSpectrum.imagp = malloc(dataPoints * sizeof(float));
    for (i = 0; i < dataPoints; i++) {
        foldedSpectrum.realp[i] = self->fid.realp[i] * self->apodizationT2[i];
        foldedSpectrum.imagp[i] = self->fid.imagp[i] * self->apodizationT2[i];
    }
    vDSP_fft_zip(self->fftsetup, &foldedSpectrum, 1, lb(dataPoints), FFT_FORWARD);
    center = dataPoints / 2;
    for (i = 0; i < dataPoints; i++) {
        if (i < center) {
            self->spectrum1D.realp[i + center] = foldedSpectrum.realp[i];
            self->spectrum1D.imagp[i + center] = foldedSpectrum.imagp[i];
        } else {
            self->spectrum1D.realp[i - center] = foldedSpectrum.realp[i];
            self->spectrum1D.imagp[i - center] = foldedSpectrum.imagp[i];
        }
    }
    set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseFrequency);
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->spectrum1D, dataPoints, dataPoints);
    free(foldedSpectrum.realp);
    free(foldedSpectrum.imagp);
}


void insensitive_controller_absolute_value_1D_spectrum(InsensitiveController *self, gboolean complexData)
{
    unsigned int x;
    unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);

    insensitive_controller_fourier_transform_1D_spectrum(self);
    if(complexData) {
        for(x = 0; x < dataPoints; x++) {
            self->spectrumSymmetrized.realp[x] = sqrtf(self->spectrum1D.realp[x] * self->spectrum1D.realp[x] + self->spectrum1D.imagp[x] * self->spectrum1D.imagp[x]);
            self->spectrumSymmetrized.imagp[x] = self->spectrumSymmetrized.realp[x];
        }
    } else {
        for(x = 0; x < dataPoints; x++) {
            self->spectrumSymmetrized.realp[x] = sqrtf(self->spectrum1D.realp[x] * self->spectrum1D.realp[x]);
            self->spectrumSymmetrized.imagp[x] = sqrtf(self->spectrum1D.imagp[x] * self->spectrum1D.imagp[x]);
        }
    }
    set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseAbs);
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->spectrumSymmetrized, dataPoints, dataPoints);
}


void insensitive_controller_perform_single_dimension_fourier_transform(InsensitiveController *self,
								                                       DSPSplitComplex source,
								                                       DSPSplitComplex destination,
								                                       enum SpectrumDimension domain)
{
	unsigned int x, y, t1DataPoints, t2DataPoints, totalDataPoints, center;
	DSPSplitComplex foldedSpectrum, fid_entry;

	t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
	t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
	totalDataPoints = t1DataPoints * t2DataPoints;
	foldedSpectrum.realp = malloc(totalDataPoints * sizeof(float));
	foldedSpectrum.imagp = malloc(totalDataPoints * sizeof(float));
	// Perform FFT on sine modulated spectrum
	for (y = 0; y < t1DataPoints; y++)
		for (x = 0; x < t2DataPoints; x++) {
			foldedSpectrum.realp[y * t2DataPoints + x] = source.realp[y * t2DataPoints + x];
			foldedSpectrum.imagp[y * t2DataPoints + x] = source.imagp[y * t2DataPoints + x];
			if (domain == F2) {
				foldedSpectrum.realp[y * t2DataPoints + x] *= self->apodizationT2[x] * self->apodizationT1[y];
				foldedSpectrum.imagp[y * t2DataPoints + x] *= self->apodizationT2[x] * self->apodizationT1[y];
			}
		}
	switch (domain) {
	case F2:
		center = t2DataPoints / 2;
		for (y = 0; y < t1DataPoints; y++) {
			fid_entry.realp = foldedSpectrum.realp + y * t2DataPoints;
			fid_entry.imagp = foldedSpectrum.imagp + y * t2DataPoints;
			vDSP_fft_zip(self->fftsetup, &fid_entry, 1, lb(t2DataPoints), FFT_FORWARD);
			for (x = 0; x < t2DataPoints; x++) {
				// First half of the spectrum
				if (x < center) {
					destination.realp[y * t2DataPoints + x + center] = fid_entry.imagp[x];
					destination.imagp[y * t2DataPoints + x + center] = fid_entry.realp[x];
					// Second half of the spectrum
				} else {
					destination.realp[y * t2DataPoints + x - center] = fid_entry.imagp[x];
					destination.imagp[y * t2DataPoints + x - center] = fid_entry.realp[x];
				}
			}
		}
		break;
	case F1:
		for (x = 0; x < t2DataPoints; x++) {
			fid_entry.realp = foldedSpectrum.realp + x;
			fid_entry.imagp = foldedSpectrum.imagp + x;
			vDSP_fft_zip(self->fftsetup, &fid_entry, t2DataPoints, lb(t1DataPoints), FFT_FORWARD);
		}
		// Unfold the raw FFT (Mirror at 1/4 and 3/4 axis along F1)
		int position, newPosition;
		center = t1DataPoints / 2;
		for (y = 0; y < t1DataPoints; y++) {
			for (x = 0; x < t2DataPoints; x++) {
				position = y * t2DataPoints + x;
				if (y < center)
					newPosition = (center - 1 - y) * t2DataPoints + x;
				else
					newPosition = (3 * center - 1 - y) * t2DataPoints + x;
				destination.realp[newPosition] = foldedSpectrum.imagp[position];
				destination.imagp[newPosition] = foldedSpectrum.realp[position];
			}
		}
		break;
	}
	free(foldedSpectrum.realp);
	free(foldedSpectrum.imagp);
}


void insensitive_controller_swap_states_spectra(gboolean realDataSet, DSPSplitComplex *sinModulated, DSPSplitComplex *cosModulated)
{
    float *temp;
    if(realDataSet) {
        // Sine-modulated Re (spectrum1D) becomes Im
        // Cosine-modulated Re (spectrum1DStates) becomes Re
        temp = sinModulated->imagp;
        sinModulated->imagp = sinModulated->realp;
        sinModulated->realp = cosModulated->realp;
        cosModulated->realp = temp;
    } else {
        // Sine-modulated Im (spectrum1D) becomes Im
        // Cosine-modulated Im (spectrum1DStates) becomes Re
        temp = sinModulated->realp;
        sinModulated->realp = cosModulated->imagp;
        cosModulated->imagp = temp;
    }
}


void insensitive_controller_fourier_transform_2D_spectrum_along_T2(InsensitiveController *self)
{
    int t1DataPoints, t2DataPoints, totalDataPoints;

    // Perform FFT on sine modulated spectrum
    insensitive_controller_perform_single_dimension_fourier_transform(self, self->fid, self->spectrum1D, F2);
    // Perform FFT on cosine modulated spectrum for States procedure
    if(self->detectionMethodOfCurrentSpectrum == States) {
        insensitive_controller_perform_single_dimension_fourier_transform(self, self->fidStates, self->spectrum1DStates, F2);
        // Swap spectra to form pure phase 2D spectra:
        insensitive_controller_swap_states_spectra(self->realDataSetsForStatesMethod, &self->spectrum1D, &self->spectrum1DStates);
    }
    t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
    t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
    totalDataPoints = t2DataPoints * t1DataPoints;
    set_indirect_dataPoints((InsensitiveWindow *)self->displayController, t1DataPoints);
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->spectrum1D, t2DataPoints, totalDataPoints);
}


void insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(InsensitiveController *self)
{
    unsigned int i, x, y, row, col;
    unsigned int t1DataPoints, t2DataPoints, totalDataPoints, centerF1, centerF2;
    unsigned int position, newPosition;
    DSPSplitComplex foldedSpectrum;

    t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
    t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
    totalDataPoints = t1DataPoints * t2DataPoints;
    if(self->detectionMethodOfCurrentSpectrum == TPPI || self->detectionMethodOfCurrentSpectrum == StatesTPPI)
        set_indirect_dataPoints((InsensitiveWindow *)self->displayController, t1DataPoints / 2);
    else
        set_indirect_dataPoints((InsensitiveWindow *)self->displayController, t1DataPoints);
    // With amplitude modulation
    if(self->detectionMethodOfCurrentSpectrum != None) {
        // Perform FFT on sine modulated spectra
        insensitive_controller_perform_single_dimension_fourier_transform(self, self->fid, self->spectrum1D, F2);
        // States
        if(self->detectionMethodOfCurrentSpectrum == States || self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
            // Perform FFT on cosine modulated spectra
            insensitive_controller_perform_single_dimension_fourier_transform(self, self->fidStates, self->spectrum1DStates, F2);
            // Swap spectra to form pure phase 2D spectra:
            insensitive_controller_swap_states_spectra(self->realDataSetsForStatesMethod, &self->spectrum1D, &self->spectrum1DStates);
            // Perform 2D FFT
            insensitive_controller_perform_single_dimension_fourier_transform(self, self->spectrum1D, self->spectrum2D, F1);
            // Rearrange spectrum if States-TPPI method was used
            if(self->detectionMethodOfCurrentSpectrum == StatesTPPI) {
                foldedSpectrum.realp = malloc(totalDataPoints * sizeof(float));
                foldedSpectrum.imagp = malloc(totalDataPoints * sizeof(float));
                centerF1 = totalDataPoints / 2;
                for(i = 0; i < totalDataPoints; i++) {
                    if(i < centerF1)
                        newPosition = i + centerF1;
                    else
                        newPosition = i - centerF1;
                    foldedSpectrum.realp[newPosition] = self->spectrum2D.realp[i];
                    foldedSpectrum.imagp[newPosition] = self->spectrum2D.imagp[i];
                }
                for(i = 0; i < totalDataPoints; i++) {
                    if(i < centerF1) {
                        self->spectrum2D.imagp[i] = foldedSpectrum.imagp[i + centerF1 / 2];
                        self->spectrum2D.realp[i] = foldedSpectrum.realp[i + centerF1 / 2];
                    } else {
                        self->spectrum2D.imagp[i] = 0.0;
                        self->spectrum2D.realp[i] = 0.0;
                    }
                }
                free(foldedSpectrum.realp);
                free(foldedSpectrum.imagp);
            }
            // End States-TPPI
        // TPPI
        } else if(self->detectionMethodOfCurrentSpectrum == TPPI) {
            double *realVector;
            float halfT1DataPoints = t1DataPoints / 2;
            fftw_complex *complexSpectrum;

            realVector = malloc(t1DataPoints * sizeof(double));
            complexSpectrum = fftw_malloc((halfT1DataPoints + 1) * sizeof(fftw_complex));
            foldedSpectrum.realp = malloc(totalDataPoints * sizeof(float));
            foldedSpectrum.imagp = malloc(totalDataPoints * sizeof(float));
            // Perform real FFT on F1 domain
            for(col = 0; col < t2DataPoints; col++) {
                for(row = 0; row < t1DataPoints; row++) {
                    if(self->realDataSetsForStatesMethod)
                        realVector[row] = self->spectrum1D.realp[row * t2DataPoints + col];
                    else
                        realVector[row] = self->spectrum1D.imagp[row * t2DataPoints + col];
                }
                self->fftsetup = fftw_plan_dft_r2c_1d(t1DataPoints, realVector, complexSpectrum, FFTW_ESTIMATE);
                fftw_execute(self->fftsetup);
                fftw_destroy_plan(self->fftsetup);
                for(row = 0; row < halfT1DataPoints; row++) {
                    foldedSpectrum.imagp[row * t2DataPoints + col] = creal(complexSpectrum[row]);
                    foldedSpectrum.realp[row * t2DataPoints + col] = cimag(complexSpectrum[row]);
                }
                foldedSpectrum.realp[0 * t2DataPoints + col] = 0;
            }
            // Unfold the raw FFT
            for(y = 0; y < t1DataPoints; y++) {
                for(x = 0; x < t2DataPoints; x++) {
                    position = y * t2DataPoints + x;
                    newPosition = position;
                    self->spectrum2D.realp[newPosition] = -foldedSpectrum.imagp[position];
                    self->spectrum2D.imagp[newPosition] = -foldedSpectrum.realp[position];
                }
            }
            free(foldedSpectrum.realp);
            free(foldedSpectrum.imagp);
            free(realVector);
            fftw_free(complexSpectrum);
        }
    // Without amplitude modulation
    } else {
        centerF1 = t1DataPoints / 2;
        centerF2 = t2DataPoints / 2;
        foldedSpectrum.realp = malloc(totalDataPoints * sizeof(float));
        foldedSpectrum.imagp = malloc(totalDataPoints * sizeof(float));
        for(y = 0; y < t1DataPoints; y++)
            for(x = 0; x < t2DataPoints; x++) {
                foldedSpectrum.realp[y * t2DataPoints + x] = self->fid.realp[y * t2DataPoints + x] * self->apodizationT2[x] * self->apodizationT1[y];
                foldedSpectrum.imagp[y * t2DataPoints + x] = self->fid.imagp[y * t2DataPoints + x] * self->apodizationT2[x] * self->apodizationT1[y];
            }
        vDSP_fft2d_zip(self->fftsetup, &foldedSpectrum, 1, 0, lb(t2DataPoints), lb(t1DataPoints), FFT_FORWARD);
        for(y = 0; y < t1DataPoints; y++) {
            for(x = 0; x < t2DataPoints; x++) {
                position = y * t2DataPoints + x;
                // First quadrant of the spectrum
                if((x < centerF2) && (y < centerF1))
                    newPosition = (y + centerF1) * t2DataPoints + x + centerF2;
                // Second quadrant of the spectrum
                else if((x >= centerF2) && (y < centerF1))
                    newPosition = (y + centerF1) * t2DataPoints + x - centerF2;
                // Third quadrant of the spectrum
                else if((x < centerF2) && (y >= centerF1))
                    newPosition = (y - centerF1) * t2DataPoints + x + centerF2;
                // Fourth quadrant of the spectrum
                else if((x >= centerF2) && (y >= centerF1))
                    newPosition = (y - centerF1) * t2DataPoints + x - centerF2;
                self->spectrum2D.realp[newPosition] = foldedSpectrum.realp[position];
                self->spectrum2D.imagp[newPosition] = foldedSpectrum.imagp[position];
            }
        }
        free(foldedSpectrum.realp);
        free(foldedSpectrum.imagp);
    }
    if(self->detectionMethodOfCurrentSpectrum == TPPI)
        totalDataPoints /= 2;
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->spectrum2D, t2DataPoints, totalDataPoints);
}


void insensitive_controller_absolute_value_spectrum(InsensitiveController *self)
{
	unsigned int t1DataPoints, t2DataPoints, totalDataPoints, x, y, indirectSize;

	t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
	t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
	totalDataPoints = t1DataPoints * t2DataPoints;
	insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(self);
	indirectSize = (self->detectionMethodOfCurrentSpectrum == TPPI || self->detectionMethodOfCurrentSpectrum == StatesTPPI) ? (t1DataPoints / 2) : t1DataPoints;
	for (y = 0; y < indirectSize; y++) {
		for (x = 0; x < t2DataPoints; x++) {
			self->spectrumSymmetrized.realp[y * t2DataPoints + x] = hypotf(self->spectrum2D.realp[y * t2DataPoints + x], self->spectrum2D.imagp[y * t2DataPoints + x]);
			self->spectrumSymmetrized.imagp[y * t2DataPoints + x] = hypotf(self->spectrum2D.realp[y * t2DataPoints + x], self->spectrum2D.imagp[y * t2DataPoints + x]);
		}
	}
	set_complex_spectrum((InsensitiveWindow *)self->displayController,
			     self->spectrumSymmetrized, t2DataPoints, totalDataPoints);
}


void insensitive_controller_spectrum_symmetrization(InsensitiveController *self, enum Symmetrization symmetrize, unsigned int spectrumDomain)
{
    unsigned int t1DataPoints, t2DataPoints, totalDataPoints, center;
    unsigned int i, x, y, position1, position2, denominator, indirectSize;
    float abs1, abs2, sign1, sign2;
    DSPSplitComplex symmetricSER, squareSpectrum;

    t2DataPoints = insensitive_settings_get_dataPoints(self->settings);
    t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, t2DataPoints);
    totalDataPoints = t1DataPoints * t2DataPoints;
    set_indirect_dataPoints((InsensitiveWindow *)self->displayController, t1DataPoints);
    if (symmetrize == SYMJ) {
        switch (spectrumDomain) {
        case 3:
            squareSpectrum.realp = self->spectrumSymmetrized.realp;
            squareSpectrum.imagp = self->spectrumSymmetrized.imagp;
            break;
        case 0:
        case 1:
            insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(self);
            /* Fallthrough: if no 2D FFT has been performed, do so and proceed with spectrumDomain == 2 */
        case 2:
            squareSpectrum.realp = self->spectrum2D.realp;
            squareSpectrum.imagp = self->spectrum2D.imagp;
            break;
        }
        for (y = 0; y < t1DataPoints / 2; y++) {
            for (x = 0; x < t2DataPoints; x++) {
                // lowest, most negative intensity
                position1 = y * t2DataPoints + x;
                position2 = (t1DataPoints - (y + 1)) * t2DataPoints + x;
                if (insensitive_settings_get_showImaginaryPart(self->settings)) {
                    abs1 = squareSpectrum.imagp[position1];
                    abs2 = squareSpectrum.imagp[position2];
                } else {
                    abs1 = squareSpectrum.realp[position1];
                    abs2 = squareSpectrum.realp[position2];
                }
                if (abs1 < abs2) {
                    self->spectrumSymmetrized.realp[position2] = squareSpectrum.realp[position1];
                    self->spectrumSymmetrized.imagp[position2] = squareSpectrum.imagp[position1];
                } else {
                    self->spectrumSymmetrized.realp[position1] = squareSpectrum.realp[position2];
                    self->spectrumSymmetrized.imagp[position1] = squareSpectrum.imagp[position2];
                }
            }
        }
    } else /* symmetrisation along diagonal */ {
        // Create a square spectrum
        center = t2DataPoints / 2;
        symmetricSER.realp = malloc(t2DataPoints * t2DataPoints * sizeof(float));
        symmetricSER.imagp = malloc(t2DataPoints * t2DataPoints * sizeof(float));
        squareSpectrum.realp = malloc(t2DataPoints * t2DataPoints * sizeof(float));
        squareSpectrum.imagp = malloc(t2DataPoints * t2DataPoints * sizeof(float));
        for (y = 0; y < t1DataPoints; y++)
            for (x = 0; x < t2DataPoints; x++) {
                symmetricSER.realp[y * t2DataPoints + x] = self->fid.realp[y * t2DataPoints + x] * self->apodizationT2[x] * self->apodizationT1[y];
                symmetricSER.imagp[y * t2DataPoints + x] = self->fid.imagp[y * t2DataPoints + x] * self->apodizationT2[x] * self->apodizationT1[y];
            }
        for (y = t1DataPoints; y < t2DataPoints; y++)
            for(x = 0; x < t2DataPoints; x++) {
                symmetricSER.realp[y * t2DataPoints + x] = 0;
                symmetricSER.imagp[y * t2DataPoints + x] = 0;
            }
        vDSP_fft2d_zip(self->fftsetup, &symmetricSER, 1, 0, lb(t2DataPoints), lb(t2DataPoints), FFT_FORWARD);
        // Unfold the raw FFT (Mirror at 1/2 axis along F1)
        for (y = 0; y < t2DataPoints; y++) {
            for (x = 0; x < t2DataPoints; x++) {
                position1 = y * t2DataPoints + x;
                // First quadrant of the spectrum
                if ((x < center) && (y < center))
                    position2 = (y + center) * t2DataPoints + x + center;
                // Second quadrant of the spectrum
                else if ((x >= center) && (y < center))
                    position2 = (y + center) * t2DataPoints + x - center;
                // Third quadrant of the spectrum
                else if ((x < center) && (y >= center))
                    position2 = (y - center) * t2DataPoints + x + center;
                // Fourth quadrant of the spectrum
                else if ((x >= center) && (y >= center))
                    position2 = (y - center) * t2DataPoints + x - center;
                if (spectrumDomain == 3) {
                    squareSpectrum.realp[position2] = hypotf(symmetricSER.realp[position1], symmetricSER.imagp[position1]);
                    squareSpectrum.imagp[position2] = hypotf(symmetricSER.realp[position1], symmetricSER.imagp[position1]);
                } else {
                    squareSpectrum.realp[position2] = symmetricSER.realp[position1];
                    squareSpectrum.imagp[position2] = symmetricSER.imagp[position1];
                }
            }
        }
        // Symmetrize
        if (self->detectionMethodOfCurrentSpectrum != States)
        for (y = 0; y < t2DataPoints; y++) {
            for (x = 0; x < t2DataPoints; x++) {
                // Determine mirror pair
                if (symmetrize > 0) {
                    // P-type data
                    position1 = y * t2DataPoints + x;
                    position2 = x * t2DataPoints + y;
                } else {
                    // N-type data
                    position1 = y * t2DataPoints + x;
                    position2 = (t2DataPoints - x - 1) * t2DataPoints + (t2DataPoints - y - 1);
                }
                // lowest, absolute intensity, keep sign
                if (symmetrize == SYMA || symmetrize == SYMA_P) {
                    abs1 = fabsf(squareSpectrum.realp[position1]);
                    abs2 = fabsf(squareSpectrum.realp[position2]);
                    sign1 = (squareSpectrum.realp[position1] < 0) ? -1 : 1;
                    sign2 = (squareSpectrum.realp[position2] < 0) ? -1 : 1;
                    if (abs1 < abs2) {
                        squareSpectrum.realp[position2] = sign2 * fabsf(squareSpectrum.realp[position1]);
                        squareSpectrum.imagp[position2] = sign2 * fabsf(squareSpectrum.imagp[position1]);
                    } else {
                        squareSpectrum.realp[position1] = sign1 * fabsf(squareSpectrum.realp[position2]);
                        squareSpectrum.imagp[position1] = sign1 * fabsf(squareSpectrum.imagp[position2]);
                    }
                // lowest, most negative intensity
                } else {
                    if (insensitive_settings_get_showImaginaryPart(self->settings)) {
                        abs1 = squareSpectrum.imagp[position1];
                        abs2 = squareSpectrum.imagp[position2];
                    } else {
                        abs1 = squareSpectrum.realp[position1];
                        abs2 = squareSpectrum.realp[position2];
                    }
                    if (abs1 < abs2) {
                        squareSpectrum.realp[position2] = squareSpectrum.realp[position1];
                        squareSpectrum.imagp[position2] = squareSpectrum.imagp[position1];
                    } else {
                        squareSpectrum.realp[position1] = squareSpectrum.realp[position2];
                        squareSpectrum.imagp[position1] = squareSpectrum.imagp[position2];
                    }
                }

            }
        }
        // Reduce F1 resolution to 128
        denominator = t2DataPoints / t1DataPoints;
        for (y = 0; y < t1DataPoints; y++) {
            for (x = 0; x < t2DataPoints; x++) {
                self->spectrumSymmetrized.realp[y * t2DataPoints + x] = 0;
                self->spectrumSymmetrized.imagp[y * t2DataPoints + x] = 0;
                for (i = 0; i < denominator; i++) {
                    self->spectrumSymmetrized.realp[y * t2DataPoints + x] += squareSpectrum.realp[(y * denominator + i) * t2DataPoints + x];
                    self->spectrumSymmetrized.imagp[y * t2DataPoints + x] += squareSpectrum.imagp[(y * denominator + i) * t2DataPoints + x];
                }
                self->spectrumSymmetrized.realp[y * t2DataPoints + x] /= denominator;
                self->spectrumSymmetrized.imagp[y * t2DataPoints + x] /= denominator;
            }
        }
        free(symmetricSER.realp);
        free(symmetricSER.imagp);
    }
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         self->spectrumSymmetrized, t2DataPoints, totalDataPoints);
}


enum WindowFunctionType insensitive_controller_get_windowFunction(InsensitiveController *self)
{
	return self->windowFunction;
}


void insensitive_controller_set_windowFunction(InsensitiveController *self, enum WindowFunctionType type)
{
	unsigned int i;
	float factor = insensitive_settings_get_zeroFilling(self->settings) ? 0.5 : 1.0;
	float shift = (self->detectionMethodOfCurrentSpectrum == TPPI) ? 0.5 : 1.0;
	float t1correction = (self->detectionMethodOfCurrentSpectrum == TPPI) ? 1.0 : 0.5;
	unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);
	float T2 = insensitive_settings_get_T2(self->settings);
	unsigned int t1DataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, dataPoints);

	if (self->apodizationT2 != NULL)
		free(self->apodizationT2);
	if (self->apodizationT1 != NULL)
		free(self->apodizationT1);
	self->apodizationT2 = malloc(dataPoints * sizeof(float));
	self->apodizationT1 = malloc(t1DataPoints * sizeof(float));
	self->windowFunction = type;
    for (i = 0; i < dataPoints; i++) {
		if (self->windowFunction == WFLorentzGaussTransformation) {
			self->apodizationT2[i] = lorentz_gauss_transformation(i, factor * dataPoints, T2, self->gaussianWidth, self->gaussianShift);
		} else if (self->windowFunction == WFGaussPseudoEchoTransformation) {
			self->apodizationT2[i] = lorentz_gauss_transformation(i, factor * dataPoints, T2, self->gaussianWidth, self->gaussianShift);
		} else {
			self->apodizationT2[i] = window_function(self->windowFunction, i, factor * dataPoints);
		}
	}
	for (i = 0; i < t1DataPoints; i++) {
		if (self->windowFunction == WFLorentzGaussTransformation) {
			self->apodizationT1[i] = lorentz_gauss_transformation(i, factor * t1DataPoints, T2, self->gaussianWidth * t1correction * 2, self->gaussianShift);
		} else if (self->windowFunction == WFGaussPseudoEchoTransformation) {
			self->apodizationT1[i] = lorentz_gauss_transformation(i, factor * t1DataPoints, T2, self->gaussianWidth * t1correction, shift * self->gaussianShift);
		} else {
			self->apodizationT1[i] = window_function(self->windowFunction, i, factor * dataPoints);
		}
	}
    set_apodization((InsensitiveWindow *)self->displayController, self->apodizationT2, self->apodizationT1);
	insensitive_controller_set_noiseLevel(self, insensitive_settings_get_noiseLevel(self->settings));
}


float insensitive_controller_get_gaussianWidth(InsensitiveController *self)
{
    return self->gaussianWidth;
}


float insensitive_controller_get_gaussianShift(InsensitiveController *self)
{
    return self->gaussianShift;
}


void insensitive_controller_set_gaussianWidth(InsensitiveController *self, float value)
{
    self->gaussianWidth = value;
    insensitive_controller_set_windowFunction(self, self->windowFunction);
}


void insensitive_controller_set_gaussianShift(InsensitiveController *self, float value)
{
    self->gaussianShift = value;
    insensitive_controller_set_windowFunction(self, self->windowFunction);
}


gboolean insensitive_controller_get_realDataSetsForStatesMethod(InsensitiveController *self)
{
    return self->realDataSetsForStatesMethod;
}


void insensitive_controller_set_realDataSetsForStatesMethod(InsensitiveController *self, gboolean value)
{
    self->realDataSetsForStatesMethod = value;
}


gboolean insensitive_controller_tilt_2D_spectrum(InsensitiveController *self, unsigned int dataset, unsigned int coordinate)
{
    // dataset:    0 = SER; 1 = 1D FFT; 2 = 2D FFT; 3 = Abs
    // coordinate: 0 = y;   1 = x

    int directDataPoints, indirectDataPoints, totalDataPoints;
    DSPSplitComplex sourceDataSet;

    switch (dataset) {
    case 2:
        insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(self);
        sourceDataSet.realp = self->spectrum2D.realp;
        sourceDataSet.imagp = self->spectrum2D.imagp;
        break;
    case 3:
        insensitive_controller_absolute_value_spectrum(self);
        sourceDataSet.realp = self->spectrumSymmetrized.realp;
        sourceDataSet.imagp = self->spectrumSymmetrized.imagp;
        break;
    case 0:
    case 1:
    default:
        return FALSE;
    }

    directDataPoints = insensitive_settings_get_dataPoints(self->settings);
    indirectDataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, directDataPoints);
    totalDataPoints = directDataPoints * indirectDataPoints;

    switch(coordinate) {
        case 0:
            // Tilting for J-resolved spectra
            tilt_x(sourceDataSet, directDataPoints, indirectDataPoints);
            break;
        case 1:
            // Tilting for SECSY
            tilt_stretch_y(sourceDataSet, directDataPoints, indirectDataPoints);
            break;
        case 2:
            // Tilting for FOCSY
            foldover_correction(sourceDataSet, directDataPoints, indirectDataPoints);
            break;
        default:
            return FALSE;
    }
    set_complex_spectrum((InsensitiveWindow *)self->displayController,
                         sourceDataSet, directDataPoints, totalDataPoints);

    return TRUE;
}


int insensitive_controller_determine_peak_list(InsensitiveController *self, GPtrArray *peaks, int dataset)
{
    // dataset = 0: real spectrum
    // dataset = 1: imaginary spectrum
    // dataset = 2: absolute value

    float *spectrum;
    unsigned int i;
    int *array, *number;
    int dataPoints = insensitive_settings_get_dataPoints(self->settings);

    switch (dataset) {
    case 1:
        spectrum = self->spectrum1D.imagp;
        break;
    case 2:
        spectrum = self->spectrumSymmetrized.realp;
        break;
    default:
        spectrum = self->spectrum1D.realp;
    }
    array = malloc(dataPoints * sizeof(int));
    self->fittedPeaks = peak_list(spectrum, dataPoints, array, insensitive_settings_get_signalToNoiseThreshold(self->settings));
    if (self->dosyParameters != NULL)
        free(self->dosyParameters);
    self->dosyParameters = malloc(self->fittedPeaks * sizeof(FittedSpectrum));
    for (i = 0; i < self->fittedPeaks; i++) {
        number = malloc(sizeof(int));
        *number = array[i];
        g_ptr_array_add(peaks, number);
        self->dosyParameters[i].x = array[i];
    }

    return self->fittedPeaks;
}


int insensitive_controller_fit_lorentzians(InsensitiveController *self, int dataset)
{
    // dataset = 0: real spectrum
    // dataset = 1: imaginary spectrum
    // dataset = 2: absolute value

    float *spectrum;
    int dataPoints = insensitive_settings_get_dataPoints(self->settings);

    if(self->dosyParameters != NULL) {
        switch(dataset) {
            case 1:
                spectrum = self->spectrum1D.imagp;
                break;
            case 2:
                spectrum = self->spectrumSymmetrized.realp;
                break;
            default:
                spectrum = self->spectrum1D.realp;
        }
        multiple_lorentzian_peak_fit(spectrum, dataPoints, self->fittedPeaks, self->dosyParameters);
        set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                             self->spectrum1D, dataPoints, dataPoints);
        return 1;
    } else
        return 0;
}


void insensitive_controller_dosy_fit(InsensitiveController *self, int dataset)
{
    // dataset: 0 = real; 1 = imaginary

    int directDataPoints, indirectDataPoints, totalDataPoints;

    insensitive_controller_get_first_trace_of_2D_spectrum(self, FALSE);
    if(self->dosyParameters != NULL) {
        insensitive_controller_fourier_transform_2D_spectrum_along_T2(self);
        directDataPoints = insensitive_settings_get_dataPoints(self->settings);
        indirectDataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, directDataPoints);
        totalDataPoints = directDataPoints * indirectDataPoints;

        if(dataset == 0) {
            dosy_fit(self->spectrum1D.realp, indirectDataPoints, directDataPoints, self->fittedPeaks, self->dosyParameters);
        } else if(dataset == 1) {
            dosy_fit(self->spectrum1D.imagp, indirectDataPoints, directDataPoints, self->fittedPeaks, self->dosyParameters);
        }
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                             self->spectrum1D, directDataPoints, totalDataPoints);
    }
}


void insensitive_controller_dosy_spectrum(InsensitiveController *self, int dataset)
{
    int directDataPoints, indirectDataPoints, totalDataPoints;

    if(self->dosyParameters != NULL) {
        directDataPoints = insensitive_settings_get_dataPoints(self->settings);
        indirectDataPoints = indirect_datapoints(self->detectionMethodOfCurrentSpectrum, directDataPoints);
        totalDataPoints = directDataPoints * indirectDataPoints;
        if(dataset == 0) {
            dosy_spectrum(self->spectrum1D.realp, indirectDataPoints, directDataPoints, self->fittedPeaks, self->dosyParameters);
        } else if(dataset == 1) {
            dosy_spectrum(self->spectrum1D.imagp, indirectDataPoints, directDataPoints, self->fittedPeaks, self->dosyParameters);
        }
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                             self->spectrum1D, directDataPoints, totalDataPoints);
    }
}


gboolean insensitive_controller_get_spectrumDataAvailable(InsensitiveController *self)
{
    return self->spectrumDataAvailable;
}


DSPSplitComplex *insensitive_controller_get_rawFID(InsensitiveController *self)
{
    return &self->fid;
}


DSPSplitComplex *insensitive_controller_get_rawFIDStates(InsensitiveController *self)
{
    return &self->fidStates;
}


float insensitive_controller_get_acquisitionTime(InsensitiveController *self)
{
    return self->acquisitionTime;
}


void insensitive_controller_calculate_first_derivative_of_1D_spectrum(InsensitiveController *self)
{
    unsigned int dataPoints;

    if(self->spectrumDataAvailable && !shows_2D_spectrum((InsensitiveWindow *)self->displayController)) {
        if(self->spectrum1D.realp == NULL || self->spectrum1D.imagp == NULL)
            insensitive_controller_fourier_transform_1D_spectrum(self);
        dataPoints = insensitive_settings_get_dataPoints(self->settings);
        derivative(self->spectrum1D.realp, dataPoints);
        derivative(self->spectrum1D.imagp, dataPoints);
        set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseFrequency);
        set_complex_spectrum((InsensitiveWindow *)self->displayController,
                             self->spectrum1D,
                             insensitive_settings_get_dataPoints(self->settings),
                             insensitive_settings_get_dataPoints(self->settings));
        spin_state_was_changed((InsensitiveWindow *)self->displayController);
    }
}


//////  //    // //      /////// ///////     /////// //   //  /////  //////  /////// //////
//   // //    // //      //      //          //      //   // //   // //   // //      //   //
//////  //    // //      /////// /////       /////// /////// /////// //////  /////   //////
//      //    // //           // //               // //   // //   // //      //      //   //
//       //////  /////// /////// ///////     /////// //   // //   // //      /////// //   //

float insensitive_controller_get_pulseLength(InsensitiveController *self)
{
	return insensitive_settings_get_pulseLength(self->settings);
}


void insensitive_controller_set_pulseLength(InsensitiveController *self, float value)
{
	// Value for pulse length is between 1 and 512
	// Translate that to a pulse length and strength for the set flip angle:
	//   β = 90°  -> 0.25 ... 250
	//   β = 180° -> 0.5 ... 500
	//   β = 360° -> 1.0 ... 1000
	float lengthFactor = insensitive_settings_get_flipAngle(self->settings) / 360;
	float newPulseDuration = 1.95616 * value - 1.55616;

	if (newPulseDuration > 999.9)
		newPulseDuration = 1000.0;
	float newPulseStrength = 1 / newPulseDuration;

	newPulseDuration *= lengthFactor;
	insensitive_settings_set_pulseDuration(self->settings, newPulseDuration);
	insensitive_settings_set_pulseStrength(self->settings, newPulseStrength);
    insensitive_controller_update_display_with_pulse_settings(self);
}


float insensitive_controller_get_pulseFrequency(InsensitiveController *self)
{
	return insensitive_settings_get_pulseFrequency(self->settings);
}


void insensitive_controller_set_pulseFrequency(InsensitiveController *self, float value)
{
	insensitive_settings_set_pulseFrequency(self->settings, value);
    insensitive_pulse_shaper_set_pulseFrequency(self->pulseShaperController, insensitive_settings_get_pulseFrequency(self->settings));
	set_pulseFrequency((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseFrequency(self->settings));
	insensitive_controller_create_pulse_powerspectrum(self);
}


void insensitive_controller_set_pulseEnvelope(InsensitiveController *self, enum PulseEnvelope value)
{
	insensitive_settings_set_pulseEnvelope(self->settings, value);
    insensitive_pulse_shaper_set_pulseEnvelope(self->pulseShaperController, insensitive_settings_get_pulseEnvelope(self->settings));
	set_pulseEnvelope((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseEnvelope(self->settings));
	switch (insensitive_settings_get_pulseEnvelope(self->settings)) {
	case EBURP_1:
	case EBURP_2:
		insensitive_controller_set_flipAngle(self, 90.0);
		break;
	case IBURP_1:
	case IBURP_2:
		insensitive_controller_set_flipAngle(self, 180.0);
		break;
	case UBURP:
	case REBURP:
		insensitive_controller_set_flipAngle(self, 360.0);
		break;
	default:
		break;
	}
	insensitive_controller_create_pulse_powerspectrum(self);
}


void insensitive_controller_create_pulse_powerspectrum(InsensitiveController *self)
{
	int i, j, n, onset;
	float length, strength, phase;
	float maximum, frequency, pulseFrequency, width, envelope;
	enum PulseEnvelope pulseEnvelope;
	float burp_freq, freq, factor, A, B, shift, real, imag;
	int dantePulseWidth, danteCycleWidth;
	DSPSplitComplex foldedSpectrum;

	length = insensitive_settings_get_pulseLength(self->settings);
	pulseEnvelope = insensitive_settings_get_pulseEnvelope(self->settings);
	pulseFrequency = roundf(insensitive_settings_get_pulseFrequency(self->settings));
	frequency = 4 * pulseFrequency / (2 * pulsePowerSpectrumResolution) * M_PI;
	// Create a rectangular pulse
	switch (pulseEnvelope) {
	case Rectangle:
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			if ((i >= (pulsePowerSpectrumResolution - length) / 2)
			    && (i < (pulsePowerSpectrumResolution + length) / 2)) {
				self->pulseShape.realp[i] = pow(-1, pulseFrequency) * cosf(i * frequency);
				self->pulseShape.imagp[i] = pow(-1, pulseFrequency) * sinf(i * frequency);
			} else {
				self->pulseShape.realp[i] = 0;
				self->pulseShape.imagp[i] = 0;
			}
		}
		break;
	case Gaussian:
		width =  length * length / 12;
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			envelope = exp(-pow(i - (int)pulsePowerSpectrumCenter, 2) / width);
			self->pulseShape.realp[i] = pow(-1, pulseFrequency) * cos(i * frequency) * envelope;
			self->pulseShape.imagp[i] = pow(-1, pulseFrequency) * sin(i * frequency) * envelope;
		}
		break;
	case Sinc:
		width = 10 / length;
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			envelope = (i == pulsePowerSpectrumCenter) ? 1.0 : sincf(M_PI * (i - pulsePowerSpectrumCenter) * width);
			self->pulseShape.realp[i] = pow(-1, pulseFrequency) * cos(i * frequency) * envelope;
			self->pulseShape.imagp[i] = pow(-1, pulseFrequency) * sin(i * frequency) * envelope;
		}
		break;
	case HypSec:
		/*width = 1 / [settings pulseLength];
		   for(i = 0; i < pulsePowerSpectrumResolution; i++) {
		    envelope = (i == pulsePowerSpectrumCenter) ? 1.0 : 1 / (coshf(M_PI * (i - pulsePowerSpectrumCenter) * width));
		    float phase = 5 * log2f(1 / (coshf(M_PI * (i - pulsePowerSpectrumCenter) * width)));
		    pulseShape.realp[i] = pow(-1, pulseFrequency) * cos(i * frequency) * envelope * cosf(phase);
		    pulseShape.imagp[i] = pow(-1, pulseFrequency) * sin(i * frequency) * envelope * sinf(phase);
		   }*/
		break;
	case EBURP_1:
//		shift = 0.80;
	// fall through
	case EBURP_2:
//		if (pulseEnvelope == EBURP_2)
//			shift = 0.85;
	// fall through
	case IBURP_1:
//		if (pulseEnvelope == IBURP_1)
//			shift = 0.85;
	// fall through
	case IBURP_2:
//		if (pulseEnvelope == IBURP_2)
//			shift = 0.85;
	// fall through
	case UBURP:
	case REBURP:
//		if (pulseEnvelope == UBURP || pulseEnvelope == REBURP || !insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings))
			shift = 0.50;
		burp_freq = 2 * M_PI / length;
		onset = (pulsePowerSpectrumResolution / 2 - shift * length);
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			if (i >= onset && i <= onset + length) {
				self->pulseShape.realp[i] = pow(-1, pulseFrequency) * cos(i * frequency);
				self->pulseShape.imagp[i] = pow(-1, pulseFrequency) * sin(i * frequency);
				if (pulseEnvelope == EBURP_1)
					factor = 0.23;
				else if (pulseEnvelope == EBURP_2)
					factor = 0.26;
				else if (pulseEnvelope == IBURP_1 || pulseEnvelope == IBURP_2)
					factor = 0.50;
				else if (pulseEnvelope == UBURP)
					factor = 0.27;
				else if (pulseEnvelope == REBURP)
					factor = 0.48;
				for (n = 1; n <= 11; n++) {
					if (pulseEnvelope == EBURP_1) {
						switch (n) {
						case 1:
							A = 0.89;
							B = -0.40;
							break;
						case 2:
							A = -1.02;
							B = -1.42;
							break;
						case 3:
							A = -0.25;
							B = 0.74;
							break;
						case 4:
							A = 0.14;
							B = 0.06;
							break;
						case 5:
							A = 0.03;
							B = 0.03;
							break;
						case 6:
							A = 0.04;
							B = -0.04;
							break;
						case 7:
							A = -0.03;
							B = -0.02;
							break;
						case 8:
							A = 0.00;
							B = 0.01;
							break;
						default:
							A = 0.00;
							B = 0.00;
						}
					} else if (pulseEnvelope == EBURP_2) {
						// For 64 ordinates
						switch (n) {
						case 1:
							A = 0.91;
							B = -0.12;
							break;
						case 2:
							A = 0.45;
							B = -1.79;
							break;
						case 3:
							A = -1.31;
							B = 0.01;
							break;
						case 4:
							A = -0.12;
							B = 0.41;
							break;
						case 5:
							A = 0.03;
							B = 0.08;
							break;
						case 6:
							A = 0.01;
							B = 0.07;
							break;
						case 7:
							A = 0.06;
							B = 0.01;
							break;
						case 8:
							A = 0.01;
							B = -0.04;
							break;
						case 9:
							A = -0.02;
							B = -0.01;
							break;
						case 10:
							A = -0.01;
							B = 0.0;
							break;
						default:
							A = 0.0;
							B = 0.0;
							break;
						}
					} else if (pulseEnvelope == IBURP_1) {
						// For 64 ordinates
						switch (n) {
						case 1:
							A = 0.74;
							B = -1.52;
							break;
						case 2:
							A = -0.20;
							B = +1.00;
							break;
						case 3:
							A = -0.92;
							B = -0.31;
							break;
						case 4:
							A = 0.12;
							B = -0.03;
							break;
						case 5:
							A = -0.03;
							B = 0.08;
							break;
						case 6:
							A = -0.04;
							B = -0.05;
							break;
						case 7:
							A = 0.01;
							B = 0.00;
							break;
						case 8:
							A = -0.02;
							B = 0.01;
							break;
						case 9:
							A = -0.01;
							B = -0.01;
							break;
						default:
							A = 0.0;
							B = 0.0;
							break;
						}
					} else if (pulseEnvelope == IBURP_2) {
						// For 64 ordinates
						switch (n) {
						case 1:
							A = 0.79;
							B = -0.71;
							break;
						case 2:
							A = 0.00;
							B = -1.39;
							break;
						case 3:
							A = -1.23;
							B = 0.31;
							break;
						case 4:
							A = -0.19;
							B = 0.47;
							break;
						case 5:
							A = 0.10;
							B = 0.22;
							break;
						case 6:
							A = 0.12;
							B = 0.03;
							break;
						case 7:
							A = 0.04;
							B = -0.05;
							break;
						case 8:
							A = -0.03;
							B = -0.04;
							break;
						case 9:
							A = -0.03;
							B = 0.00;
							break;
						case 10:
							A = -0.01;
							B = 0.02;
							break;
						case 11:
							A = 0.0;
							B = 0.01;
							break;
						default:
							A = 0.0;
							B = 0.0;
							break;
						}
					} else if (pulseEnvelope == UBURP) {
						// For 64 ordinates
						switch (n) {
						case 1:
							A = -1.42;
							B = 0;
							break;
						case 2:
							A = -0.33;
							break;
						case 3:
							A = -1.72;
							break;
						case 4:
							A = 4.47;
							break;
						case 5:
							A = -1.33;
							break;
						case 6:
							A = -0.04;
							break;
						case 7:
							A = -0.34;
							break;
						case 8:
							A = 0.50;
							break;
						case 9:
							A = -0.33;
							break;
						case 10:
							A = 0.18;
							break;
						case 11:
							A = -0.21;
							break;
						case 12:
							A = 0.24;
							break;
						case 13:
							A = -0.14;
							break;
						case 14:
							A = 0.07;
							break;
						case 15:
							A = -0.06;
							break;
						case 16:
							A = 0.00;
							break;
						case 17:
							A = -0.04;
							break;
						case 18:
							A = 0.03;
							break;
						case 19:
							A = -0.03;
							break;
						case 20:
							A = 0.02;
							break;
						}
					} else if (pulseEnvelope == REBURP) {
						// For 64 ordinates
						switch (n) {
						case 1:
							A = -1.03;
							B = 0;
							break;
						case 2:
							A = 1.09;
							break;
						case 3:
							A = -1.59;
							break;
						case 4:
							A = 0.86;
							break;
						case 5:
							A = -0.44;
							break;
						case 6:
							A = 0.27;
							break;
						case 7:
							A = -0.17;
							break;
						case 8:
							A = 0.10;
							break;
						case 9:
							A = -0.08;
							break;
						case 10:
							A = 0.04;
							break;
						case 11:
							A = -0.04;
							break;
						case 12:
							A = 0.01;
							break;
						case 13:
							A = -0.02;
							break;
						case 14:
							A = 0.0;
							break;
						case 15:
							A = -0.02;
							break;
						default:
							A = 0.0;
							break;
						}
					}
					freq = (float)(i - onset);
					factor += (A * cosf(n * burp_freq * freq) + B * sinf(n * burp_freq * freq));
				}
				if (pulseEnvelope == UBURP)
					factor *= 0.125;
				else if (pulseEnvelope == REBURP)
					factor *= 0.175;
				else
					factor *= 0.25;
				self->pulseShape.realp[i] *= factor;
				self->pulseShape.imagp[i] *= factor;
			} else {
				self->pulseShape.realp[i] = 0;
				self->pulseShape.imagp[i] = 0;
			}
		}
		break;
	case DANTE:
		dantePulseWidth = length / maxDanteCycles / 2;
		if (dantePulseWidth < 1)
			dantePulseWidth = 1;
		danteCycleWidth = pulsePowerSpectrumResolution / maxDanteCycles;
		for (i = pulsePowerSpectrumCenter; i < pulsePowerSpectrumResolution; i += danteCycleWidth) {
			for (j = 0; j < danteCycleWidth; j++) {
				if (j > danteCycleWidth / 2 - dantePulseWidth && j < danteCycleWidth / 2 + dantePulseWidth) {
					self->pulseShape.realp[i + j] = pow(-1, pulseFrequency) * cos((i + j) * frequency);
					self->pulseShape.imagp[i + j] = pow(-1, pulseFrequency) * sin((i + j) * frequency);
					self->pulseShape.realp[pulsePowerSpectrumResolution - i - j] = pow(-1, pulseFrequency) * cos((i + j) * frequency);
					self->pulseShape.imagp[pulsePowerSpectrumResolution - i - j] = pow(-1, pulseFrequency) * sin((i + j) * frequency);
				} else {
					self->pulseShape.realp[i + j] = 0.0;
					self->pulseShape.imagp[i + j] = 0.0;
					self->pulseShape.realp[pulsePowerSpectrumResolution - i - j] = 0.0;
					self->pulseShape.imagp[pulsePowerSpectrumResolution - i - j] = 0.0;
				}
				if (i + j > pulsePowerSpectrumResolution)
					break;
			}
		}
		break;
	}

	if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings)) {
        // If no off-resonance effects enabled: calculate Fourier transform of pulse shape
		// Copy time domain
		foldedSpectrum.realp = malloc(4 * pulsePowerSpectrumResolution * sizeof(float));
		foldedSpectrum.imagp = malloc(4 * pulsePowerSpectrumResolution * sizeof(float));
        for (i = 0; i < 4 * pulsePowerSpectrumResolution; i++) {
            if (i > 3 * pulsePowerSpectrumCenter && i < 5 * pulsePowerSpectrumCenter) {
			    foldedSpectrum.realp[i] = self->pulseShape.realp[i - 3 * pulsePowerSpectrumCenter];
			    foldedSpectrum.imagp[i] = self->pulseShape.imagp[i - 3 * pulsePowerSpectrumCenter];
            } else {
                foldedSpectrum.realp[i] = 0.0;
                foldedSpectrum.imagp[i] = 0.0;
            }
		}
		// Fourier transformation
		vDSP_fft_zip(self->fftsetup, &foldedSpectrum, 1, lb(4 * pulsePowerSpectrumResolution), FFT_FORWARD);
		// Fold and normalise wrapped frequency domain
        if (pulseFrequency >= 0)
			maximum = fabsf(foldedSpectrum.realp[(int)pulseFrequency * 4]);
		else
			maximum = fabsf(foldedSpectrum.realp[((int)pulseFrequency + pulsePowerSpectrumResolution) * 4]);
		if (pulseEnvelope == EBURP_1 || pulseEnvelope == EBURP_2)
			maximum *= 3.5;
		else if (pulseEnvelope == IBURP_1 || pulseEnvelope == IBURP_2 || pulseEnvelope == REBURP)
			maximum *= 1.75;
		else if (pulseEnvelope == UBURP)
			maximum *= 8;
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
            int j = (i > pulsePowerSpectrumCenter) ? i + 3 * pulsePowerSpectrumResolution : i;
			// Calculate envelope function (remove high frequency oscillations)
			real = pow(-1, i) * foldedSpectrum.realp[j] / maximum;
			imag = pow(-1, i) * foldedSpectrum.imagp[j] / maximum;
			phase = atan2f(real, imag) / (M_PI * 4);
			real = sqrt(pow(real, 2) + pow(imag, 2));
			imag = phase;
			if (i < pulsePowerSpectrumCenter) {
				self->pulsePowerSpectrum.realp[i + pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i + pulsePowerSpectrumCenter] = imag;
			} else {
				self->pulsePowerSpectrum.realp[i - pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i - pulsePowerSpectrumCenter] = imag;
			}
		}
        /*
        // If no off-resonance effects enabled: calculate Fourier transform of pulse shape
		// Copy time domain
		foldedSpectrum.realp = malloc(2 * pulsePowerSpectrumResolution * sizeof(float));
		foldedSpectrum.imagp = malloc(2 * pulsePowerSpectrumResolution * sizeof(float));
        for (i = 0; i < 2 * pulsePowerSpectrumResolution; i++) {
            if (i > pulsePowerSpectrumCenter && i < 3 * pulsePowerSpectrumCenter) {
			    foldedSpectrum.realp[i] = self->pulseShape.realp[i - pulsePowerSpectrumCenter];
			    foldedSpectrum.imagp[i] = self->pulseShape.imagp[i - pulsePowerSpectrumCenter];
            } else {
                foldedSpectrum.realp[i] = 0.0;
                foldedSpectrum.imagp[i] = 0.0;
            }
		}
		// Fourier transformation
		vDSP_fft_zip(self->fftsetup, &foldedSpectrum, 1, lb(2 * pulsePowerSpectrumResolution), FFT_FORWARD);
		// Fold and normalise wrapped frequency domain
        if (pulseFrequency >= 0)
			maximum = fabsf(foldedSpectrum.realp[(int)pulseFrequency * 2]);
		else
			maximum = fabsf(foldedSpectrum.realp[((int)pulseFrequency + pulsePowerSpectrumResolution) * 2]);
		if (pulseEnvelope == EBURP_1 || pulseEnvelope == EBURP_2)
			maximum *= 3.5;
		else if (pulseEnvelope == IBURP_1 || pulseEnvelope == IBURP_2 || pulseEnvelope == REBURP)
			maximum *= 1.75;
		else if (pulseEnvelope == UBURP)
			maximum *= 8;
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
            int j = (i > pulsePowerSpectrumCenter) ? i + pulsePowerSpectrumResolution : i;
			// Calculate envelope function (remove high frequency oscillations)
			real = pow(-1, i) * foldedSpectrum.realp[j] / maximum;
			imag = pow(-1, i) * foldedSpectrum.imagp[j] / maximum;
			phase = atan2f(real, imag) / (M_PI * 4);
			real = sqrt(pow(real, 2) + pow(imag, 2));
			imag = phase;
			if (i < pulsePowerSpectrumCenter) {
				self->pulsePowerSpectrum.realp[i + pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i + pulsePowerSpectrumCenter] = imag;
			} else {
				self->pulsePowerSpectrum.realp[i - pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i - pulsePowerSpectrumCenter] = imag;
			}
		}
        */
        /*
        // If no off-resonance effects enabled: calculate Fourier transform of pulse shape
		// Copy time domain
		foldedSpectrum.realp = malloc(pulsePowerSpectrumResolution * sizeof(float));
		foldedSpectrum.imagp = malloc(pulsePowerSpectrumResolution * sizeof(float));
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			foldedSpectrum.realp[i] = self->pulseShape.realp[i];
			foldedSpectrum.imagp[i] = self->pulseShape.imagp[i];
		}
		// Fourier transformation
		vDSP_fft_zip(self->fftsetup, &foldedSpectrum, 1, lb(pulsePowerSpectrumResolution), FFT_FORWARD);
		// Fold and normalise wrapped frequency domain
		if (pulseFrequency >= 0)
			maximum = fabsf(foldedSpectrum.realp[(int)pulseFrequency]);
		else
			maximum = fabsf(foldedSpectrum.realp[(int)pulseFrequency + pulsePowerSpectrumResolution]);
		if (pulseEnvelope == EBURP_1 || pulseEnvelope == EBURP_2)
			maximum *= 3.5;
		else if (pulseEnvelope == IBURP_1 || pulseEnvelope == IBURP_2 || pulseEnvelope == REBURP)
			maximum *= 1.75;
		else if (pulseEnvelope == UBURP)
			maximum *= 8;
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			// Calculate envelope function (remove high frequency oscillations)
			real = pow(-1, i) * foldedSpectrum.realp[i] / maximum;
			imag = pow(-1, i) * foldedSpectrum.imagp[i] / maximum;
			phase = atan2f(real, imag) / (M_PI * 4);
			real = sqrt(pow(real, 2) + pow(imag, 2));
			imag = phase;
			if (i < pulsePowerSpectrumCenter) {
				self->pulsePowerSpectrum.realp[i + pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i + pulsePowerSpectrumCenter] = imag;
			} else {
				self->pulsePowerSpectrum.realp[i - pulsePowerSpectrumCenter] = real;
				self->pulsePowerSpectrum.imagp[i - pulsePowerSpectrumCenter] = imag;
			}
		}
        */
	} else {
		// Simulate pulse for spectrum width
		int danteCycles;
		float steps = pulseShapeResolution;
		float stepDuration = insensitive_settings_get_pulseDuration(self->settings) / pulseShapeResolution;
		InsensitiveSpinSystem *simulatedSpinSystem = insensitive_spinsystem_new();
		insensitive_spinsystem_init_with_spins(simulatedSpinSystem, 1);
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			insensitive_spinsystem_set_larmorfrequency_for_spin(simulatedSpinSystem, 0, (i - pulsePowerSpectrumCenter) * 0.1);
			insensitive_spinsystem_return_to_thermal_equilibrium(simulatedSpinSystem);
			for (danteCycles = (pulseEnvelope == DANTE) ? maxDanteCycles : 1; danteCycles > 0; danteCycles--) {
				for (j = 0; j < steps; j++) {
					strength = insensitive_settings_get_pulseShape(self->settings).realp[j] * insensitive_settings_get_pulseStrength(self->settings);
					phase = insensitive_settings_get_pulseShape(self->settings).imagp[j] + insensitive_settings_get_phase(self->settings);
					while (phase > 360) {
						phase -= 360;
					}
					insensitive_spinsystem_offresonancepulse(simulatedSpinSystem,
										                     strength * 720,
										                     stepDuration,
										                     phase,
										                     insensitive_settings_get_pulseFrequency(self->settings),
										                     1);
				}
			}
			real = -insensitive_spinsystem_get_expectationvalue_y_for_spin(simulatedSpinSystem, 0);
			imag = insensitive_spinsystem_get_expectationvalue_x_for_spin(simulatedSpinSystem, 0);
			switch (insensitive_settings_get_excitationProfile(self->settings)) {
			case Mxy_Phase:
				self->pulsePowerSpectrum.realp[i] = sqrt(pow(real, 2) + pow(imag, 2));
				self->pulsePowerSpectrum.imagp[i] = atan2f(real, imag) / (M_PI * 4);
				break;
			case Mx_My:
				self->pulsePowerSpectrum.realp[i] = real;
				self->pulsePowerSpectrum.imagp[i] = imag;
				break;
			case Mz:
				self->pulsePowerSpectrum.realp[i] = insensitive_spinsystem_get_expectationvalue_z_for_spin(simulatedSpinSystem, 0);
				self->pulsePowerSpectrum.imagp[i] = 0.0;
			}
		}
		g_object_unref(simulatedSpinSystem);
	}

	insensitive_settings_set_pulsePowerSpectrum(self->settings, &(self->pulsePowerSpectrum.realp[pulsePowerSpectrumQuarter]));
	insensitive_pulse_shaper_refreshGraphs(self->pulseShaperController);

	if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings)) {
		free(foldedSpectrum.realp);
		free(foldedSpectrum.imagp);
	}
}


DSPSplitComplex insensitive_controller_get_pulseShape(InsensitiveController *self)
{
	return self->pulseShape;
}


DSPSplitComplex insensitive_controller_get_pulsePowerSpectrum(InsensitiveController *self)
{
	DSPSplitComplex centerPartOfSpectrum;

	centerPartOfSpectrum.realp = &(self->pulsePowerSpectrum.realp[pulsePowerSpectrumQuarter]);
	centerPartOfSpectrum.imagp = &(self->pulsePowerSpectrum.imagp[pulsePowerSpectrumQuarter]);

	return centerPartOfSpectrum;
}


/////// /////// //////// //////// // ///    //  //////  ///////
//      //         //       //    // ////   // //       //
/////// /////      //       //    // // //  // //   /// ///////
     // //         //       //    // //  // // //    //      //
/////// ///////    //       //    // //   ////  //////  ///////

void insensitive_controller_set_flipAngle(InsensitiveController *self, float value)
{
	float rescalingFactor = insensitive_settings_get_flipAngle(self->settings);
	float newPulseDuration;

	if (value < 0.0)
		insensitive_settings_set_flipAngle(self->settings, 0.0);
	else if (value > 360.0)
		insensitive_settings_set_flipAngle(self->settings, 360.0);
	else
		insensitive_settings_set_flipAngle(self->settings, value);

	// Adjust pulse duration and pulse strength
	// Assume that the user wants to change the pulse width before changing the pulse strength
	if (insensitive_settings_get_flipAngle(self->settings) < 0.1) {
		insensitive_settings_set_pulseDuration(self->settings, 0.0);
	} else {
		if (insensitive_settings_get_pulseDuration(self->settings) == 0.0) {
			newPulseDuration = insensitive_settings_get_flipAngle(self->settings) / (360 * insensitive_settings_get_pulseStrength(self->settings));
		} else {
			rescalingFactor /= insensitive_settings_get_flipAngle(self->settings);
			newPulseDuration = insensitive_settings_get_pulseDuration(self->settings) / rescalingFactor;
			if (newPulseDuration < 0.001) {
				rescalingFactor = 0.001 * newPulseDuration;
				newPulseDuration = 0.001;
				insensitive_settings_set_pulseStrength(self->settings, insensitive_settings_get_pulseStrength(self->settings) * rescalingFactor);
			}
		}
		insensitive_settings_set_pulseDuration(self->settings, newPulseDuration);
	}
	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_set_pulseDuration(InsensitiveController *self, float value)
{
	float rescalingFactor = insensitive_settings_get_pulseDuration(self->settings);

	if (value > 1000.0)
		insensitive_settings_set_pulseDuration(self->settings, 1000.0);
	else if (value < 0.001) {
		insensitive_settings_set_pulseDuration(self->settings, 0.0);
		insensitive_settings_set_flipAngle(self->settings, 0.0);
		insensitive_settings_set_pulseStrength(self->settings, 1.0);
	} else
		insensitive_settings_set_pulseDuration(self->settings, value);

	if (insensitive_settings_get_pulseDuration(self->settings) >= 0.001) {
		// Assume that the user wants to change the pulse width and keep the flip angle constant.
		// A longer pulse requires a lower pulse strength for the same flip angle.
		rescalingFactor /= insensitive_settings_get_pulseDuration(self->settings);
		insensitive_settings_set_pulseStrength(self->settings, insensitive_settings_get_pulseStrength(self->settings) * rescalingFactor);
		// If γB₁ goes below 0.001 then β needs to change as well
		if (insensitive_settings_get_pulseStrength(self->settings) < 0.001) {
			insensitive_settings_set_pulseStrength(self->settings, 0.001);
			insensitive_settings_set_flipAngle(self->settings, insensitive_settings_get_pulseDuration(self->settings) * insensitive_settings_get_pulseStrength(self->settings) * 360);
		}
	}
	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_set_pulseStrength(InsensitiveController *self, float value)
{
	float rescalingFactor = insensitive_settings_get_pulseStrength(self->settings);
	float newFlipAngle, fullNewFlipAngle, leftoverFraction;

	if (value > 1000.0)
		insensitive_settings_set_pulseStrength(self->settings, 1000.0);
	else if (value < 0.001) {
		insensitive_settings_set_pulseDuration(self->settings, 0.0);
		insensitive_settings_set_flipAngle(self->settings, 0.0);
		insensitive_settings_set_pulseStrength(self->settings, 1.0);
	} else
		insensitive_settings_set_pulseStrength(self->settings, value);

	if (insensitive_settings_get_pulseStrength(self->settings) >= 0.001 && insensitive_settings_get_flipAngle(self->settings) > 0.0) {
		// Assume that the user wants to change the flip angle and keep the pulse width constant.
		// A stronger pulse leads to a larger flip angle, but flip angle cannot be > 360° here.
		// If flip angle is > 360° the pulse duration needs to be shortened.
		rescalingFactor = insensitive_settings_get_pulseStrength(self->settings) / rescalingFactor;
		newFlipAngle = insensitive_settings_get_flipAngle(self->settings) * rescalingFactor;
		if (newFlipAngle > 360) {
			fullNewFlipAngle = newFlipAngle;
			while (newFlipAngle > 360) {
				newFlipAngle -= 360;
			}
			leftoverFraction = newFlipAngle / fullNewFlipAngle;
			insensitive_settings_set_pulseDuration(self->settings, insensitive_settings_get_pulseDuration(self->settings) * leftoverFraction);
		}
		insensitive_settings_set_flipAngle(self->settings, newFlipAngle);
		// If tp goes below 0.001 then β needs to change as well
		if (insensitive_settings_get_pulseDuration(self->settings) < 0.001) {
			insensitive_settings_set_pulseDuration(self->settings, 0.001);
			insensitive_settings_set_flipAngle(self->settings, insensitive_settings_get_pulseDuration(self->settings) * insensitive_settings_get_pulseStrength(self->settings) * 360);
		}
	}
	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_make_hard_pulse(InsensitiveController *self)
{
	float duration, strength;

	duration = insensitive_settings_get_flipAngle(self->settings) / 900;
	strength = 2.5;
	insensitive_settings_set_pulseDuration(self->settings, duration);
	insensitive_settings_set_pulseStrength(self->settings, strength);

	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_make_selective_pulse(InsensitiveController *self)
{
	float duration, strength;

	duration = insensitive_settings_get_flipAngle(self->settings) / 360 * 1000;
	strength = 0.001;
	insensitive_settings_set_pulseDuration(self->settings, duration);
	insensitive_settings_set_pulseStrength(self->settings, strength);

	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_make_soft_pulse(InsensitiveController *self)
{
	float duration, strength;

	duration = insensitive_settings_get_flipAngle(self->settings) / 360 * 100;
	strength = 0.010;
	insensitive_settings_set_pulseDuration(self->settings, duration);
	insensitive_settings_set_pulseStrength(self->settings, strength);

	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_make_softer_pulse(InsensitiveController *self)
{
	float duration, strength;

	duration = insensitive_settings_get_flipAngle(self->settings) / 360 * 336;
	strength = 0.00297619;
	insensitive_settings_set_pulseDuration(self->settings, duration);
	insensitive_settings_set_pulseStrength(self->settings, strength);

	insensitive_controller_update_display_with_pulse_settings(self);
}


void insensitive_controller_update_display_with_pulse_settings(InsensitiveController *self)
{
	set_flipAngle((InsensitiveWindow *)self->displayController, insensitive_settings_get_flipAngle(self->settings));
	set_pulseDuration((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseDuration(self->settings));
	set_pulseStrength((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseStrength(self->settings));
	insensitive_pulse_shaper_set_pulseLength(self->pulseShaperController, insensitive_settings_get_pulseDuration(self->settings));

	insensitive_controller_create_pulse_powerspectrum(self);
}


void insensitive_controller_set_phase(InsensitiveController *self, float value)
{
	if ((value >= 0) && (value <= 360))
		insensitive_settings_set_phase(self->settings, value);
	set_phase((InsensitiveWindow *)self->displayController, insensitive_settings_get_phase(self->settings));

	insensitive_controller_create_pulse_powerspectrum(self);
}


void insensitive_controller_change_pulseArray_for_spin(InsensitiveController *self, int spin, gboolean value)
{
	insensitive_settings_set_pulseArray_for_spinnumber(self->settings,
							   spin,
							   value,
							   insensitive_spinsystem_get_spins(self->spinSystem),
							   insensitive_spinsystem_get_spintypearray(self->spinSystem));
	set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
	set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
	set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
	set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
}


void insensitive_controller_set_all_iSpins_active(InsensitiveController *self, gboolean value)
{
	unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);
	unsigned int pulseArray = insensitive_settings_get_pulseArray(self->settings);
	unsigned int spinTypeArray = insensitive_spinsystem_get_spintypearray(self->spinSystem);

	if (value)
		insensitive_settings_set_pulseArray(self->settings,
						    (pulseArray | ~spinTypeArray) - ~(pow2(spins) - 1),
						    spins,
						    spinTypeArray);
	else
		insensitive_settings_set_pulseArray(self->settings,
						    pulseArray & spinTypeArray,
						    spins,
						    spinTypeArray);
	set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
	set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
	set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
	set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
}


void insensitive_controller_set_all_sSpins_active(InsensitiveController *self, gboolean value)
{
	unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);
	unsigned int pulseArray = insensitive_settings_get_pulseArray(self->settings);
	unsigned int spinTypeArray = insensitive_spinsystem_get_spintypearray(self->spinSystem);

	if (value)
		insensitive_settings_set_pulseArray(self->settings,
						    pulseArray | spinTypeArray,
						    spins,
						    spinTypeArray);
	else
		insensitive_settings_set_pulseArray(self->settings,
						    pulseArray & ~spinTypeArray,
						    spins,
						    spinTypeArray);
	set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
	set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
	set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
	set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
}


void insensitive_controller_set_all_spins_active(InsensitiveController *self, gboolean value)
{
	unsigned int spins = insensitive_spinsystem_get_spins(self->spinSystem);
	unsigned int spinTypeArray = insensitive_spinsystem_get_spintypearray(self->spinSystem);

	if (value)
		insensitive_settings_set_pulseArray(self->settings,
						    pow2(spins) - 1,
						    spins,
						    spinTypeArray);
	else
		insensitive_settings_set_pulseArray(self->settings,
						    0,
						    spins,
						    spinTypeArray);
	set_allSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSpinsSelected(self->settings));
	set_iSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allISpinsSelected(self->settings));
	set_sSpins_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_allSSpinsSelected(self->settings));
	set_spin_checkboxes((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseArray(self->settings));
}


void insensitive_controller_set_strongCoupling(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_strongCoupling(self->settings, value);
	set_strongCoupling_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_strongCoupling(self->settings));
	insensitive_controller_calculate_energy_levels(self);
	if (!self->operationIsInProgress)
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
}


void insensitive_controller_set_dipolarRelaxation(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_dipolarRelaxation(self->settings, value);
	set_dipolarRelaxation_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_dipolarRelaxation(self->settings));
}


void insensitive_controller_set_animates(InsensitiveController *self, gboolean value)
{
	if (value) {
		insensitive_controller_initialise_grapefruit_path(self);
		if (self->animationIsInProgress)
			g_source_remove(self->animationTimerNr);
		else
			self->animationIsInProgress = TRUE;
		insensitive_settings_set_animates(self->settings, TRUE);
		self->animationTimerNr = g_timeout_add(2, animationTimerEvent, self); /* was 2.5 ms */
	} else {
		if (self->animationIsInProgress)
			g_source_remove(self->animationTimerNr);
		self->animationIsInProgress = FALSE;
		insensitive_settings_set_animates(self->settings, FALSE);
	}
	self->dwellTimeFraction = 0;
	set_animation_checkbox((InsensitiveWindow *)self->displayController, self->animationIsInProgress);
}


void insensitive_controller_halt_animation(InsensitiveController *self, gboolean value)
{
	self->haltAnimation = value;
}


void insensitive_controller_set_relaxation_with_evolution(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_relaxationWithEvolution(self->settings, value);
	set_include_relaxation_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_relaxationWithEvolution(self->settings));
}


void insensitive_controller_restore_relaxation_with_evolution(InsensitiveController *self)
{
	insensitive_settings_set_relaxationWithEvolution(self->settings, self->relaxationWasIncludedBefore);
}


void insensitive_controller_set_T1(InsensitiveController *self, float value)
{
	if (insensitive_settings_get_T2(self->settings))
		insensitive_settings_set_T2(self->settings, value);
	insensitive_settings_set_T1(self->settings, value);
	set_T1((InsensitiveWindow *)self->displayController, insensitive_settings_get_T1(self->settings));
}


void insensitive_controller_set_T2(InsensitiveController *self, float value)
{
	insensitive_settings_set_T2(self->settings, value);
	set_T2((InsensitiveWindow *)self->displayController, insensitive_settings_get_T2(self->settings));

	if (self->windowFunction == WFLorentzGaussTransformation || self->windowFunction == WFGaussPseudoEchoTransformation) {
		insensitive_controller_set_windowFunction(self, self->windowFunction);
	}
}


void insensitive_controller_set_correlationTime(InsensitiveController *self, float value)
{
	insensitive_settings_set_correlationTime(self->settings, value);
	set_correlationTime((InsensitiveWindow *)self->displayController, insensitive_settings_get_correlationTime(self->settings));
	insensitive_spinsystem_reset_relaxationpropagator(self->spinSystem);
}


void insensitive_controller_set_delay(InsensitiveController *self, float value)
{
	insensitive_settings_set_delay(self->settings, value);
	set_delay((InsensitiveWindow *)self->displayController, insensitive_settings_get_delay(self->settings));
}


void insensitive_controller_set_delay_from_menuEntry(InsensitiveController *self, unsigned int tag)
{
	insensitive_controller_set_delay(self, self->selectableDelayValues[tag]);
}


void insensitive_controller_calculate_selectable_delays(InsensitiveController *self)
{
	unsigned int i, j, n, spins;
	wchar_t index[4] = { 0x2081, 0x2082, 0x2083, 0x2084 };
	float J;

	for (i = 0; i < 4 * 6; i++) {
        //g_free(self->selectableDelayNames[i]); // String seems to be freed when combobox is cleared
        self->selectableDelayValues[i] = 0.0;
	}

	spins = insensitive_spinsystem_get_spins(self->spinSystem);
	n = 0;
	for (i = 0; i < maxNumberOfSpins; i++) {
		for (j = i + 1; j < maxNumberOfSpins; j++) {
			J = insensitive_spinsystem_get_jcouplingconstant_between_spins(self->spinSystem, i, j);
			if (J != 0 && i < spins && j < spins) {
				self->selectableDelayNames[n * 4] = malloc(14 * sizeof(gchar));
				sprintf(self->selectableDelayNames[n * 4], "1/(2 J%C%C)", index[i], index[j]);
				self->selectableDelayNames[n * 4 + 1] = malloc(14 * sizeof(gchar));
				sprintf(self->selectableDelayNames[n * 4 + 1], "1/(3 J%C%C)", index[i], index[j]);
				self->selectableDelayNames[n * 4 + 2] = malloc(14 * sizeof(gchar));
				sprintf(self->selectableDelayNames[n * 4 + 2], "1/(4 J%C%C)", index[i], index[j]);
				self->selectableDelayNames[n * 4 + 3] = malloc(14 * sizeof(gchar));
				sprintf(self->selectableDelayNames[n * 4 + 3], "1/(8 J%C%C)", index[i], index[j]);
				self->selectableDelayValues[n * 4] = 1 / (2 * J);
				self->selectableDelayValues[n * 4 + 1] = 1 / (3 * J);
				self->selectableDelayValues[n * 4 + 2] = 1 / (4 * J);
				self->selectableDelayValues[n * 4 + 3] = 1 / (8 * J);
			} else {
                self->selectableDelayNames[n * 4] = NULL;
				self->selectableDelayNames[n * 4 + 1] = NULL;
				self->selectableDelayNames[n * 4 + 2] = NULL;
				self->selectableDelayNames[n * 4 + 3] = NULL;
				self->selectableDelayValues[n * 4] = 0;
				self->selectableDelayValues[n * 4 + 1] = 0;
				self->selectableDelayValues[n * 4 + 2] = 0;
				self->selectableDelayValues[n * 4 + 3] = 0;
            }
            n++;
		}
	}
	set_selectable_delay_times((InsensitiveWindow *)self->displayController, self->selectableDelayNames, 4 * 6);
	display_pulseProgram_code((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_dephasingJitter(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_dephasingJitter(self->settings, value);
	set_dephasingJitter_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_dephasingJitter(self->settings));
}


void insensitive_controller_set_IDecoupling(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_iDecoupling(self->settings, value);
	set_iDecoupling_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_iDecoupling(self->settings));
}


void insensitive_controller_set_SDecoupling(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_sDecoupling(self->settings, value);
	set_sDecoupling_checkbox((InsensitiveWindow *)self->displayController, insensitive_settings_get_sDecoupling(self->settings));
}


void insensitive_controller_save_decoupling(InsensitiveController *self)
{
	self->iSpinsWereDecoupledBefore = insensitive_settings_get_iDecoupling(self->settings);
	self->sSpinsWereDecoupledBefore = insensitive_settings_get_sDecoupling(self->settings);
}


void insensitive_controller_restore_decoupling(InsensitiveController *self)
{
	insensitive_controller_set_IDecoupling(self, self->iSpinsWereDecoupledBefore);
	insensitive_controller_set_SDecoupling(self, self->sSpinsWereDecoupledBefore);
}


void insensitive_controller_set_spinlock(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_spinlock(self->settings, value);
	if (value) {
		insensitive_controller_set_IDecoupling(self, insensitive_spinsystem_get_number_of_ispins(self->spinSystem) > 0);
		insensitive_controller_set_SDecoupling(self, insensitive_spinsystem_get_number_of_sspins(self->spinSystem) > 0);
	} else {
		insensitive_controller_set_IDecoupling(self, FALSE);
		insensitive_controller_set_SDecoupling(self, FALSE);
	}
	insensitive_controller_save_decoupling(self);
	insensitive_spinsystem_reset_relaxationpropagator(self->spinSystem);
	set_spinlock((InsensitiveWindow *)self->displayController, insensitive_settings_get_spinlock(self->settings));
}


void insensitive_controller_set_gradientStrength(InsensitiveController *self, float value)
{
	insensitive_settings_set_gradientStrength(self->settings, value);
	set_gradient_strength((InsensitiveWindow *)self->displayController, insensitive_settings_get_gradientStrength(self->settings));
}


void insensitive_controller_set_gradientDuration(InsensitiveController *self, float value)
{
	insensitive_settings_set_gradientDuration(self->settings, value);
	set_gradient_duration((InsensitiveWindow *)self->displayController, insensitive_settings_get_gradientDuration(self->settings));
}


void insensitive_controller_set_diffusion(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_diffusion(self->settings, value);
	set_diffusion((InsensitiveWindow *)self->displayController, insensitive_settings_get_diffusion(self->settings));
}


void insensitive_controller_set_dataPoints(InsensitiveController *self, unsigned int value)
{
	insensitive_settings_set_dataPoints(self->settings, value);
	value = insensitive_settings_get_dataPoints(self->settings);
	//[spectrumController set1DDataPoints:0 ofMaxDataPoints:value];
	set_dataPoints((InsensitiveWindow *)self->displayController,
		       insensitive_settings_get_zeroFilling(self->settings) ? value / 2 : value);
	//[self stopAcquisition];
	//[self resetAcquisitionForDataPoints:dataPoints];
}


void insensitive_controller_set_dwellTime(InsensitiveController *self, float value)
{
	insensitive_settings_set_dwellTime(self->settings, value);
	set_dwellTime((InsensitiveWindow *)self->displayController, insensitive_settings_get_dwellTime(self->settings));
}


void insensitive_controller_set_noiseLevel(InsensitiveController *self, float value)
{
	unsigned int i;
	unsigned int dataPoints = insensitive_settings_get_dataPoints(self->settings);
	float noiseLevel;

	insensitive_settings_set_noiseLevel(self->settings, value);
	noiseLevel = insensitive_settings_get_noiseLevel(self->settings);

	set_noiseLevel((InsensitiveWindow *)self->displayController, noiseLevel);
	if (self->noiseTime.realp != NULL)
		free(self->noiseTime.realp);
	if (self->noiseTime.imagp != NULL)
		free(self->noiseTime.imagp);
	if (self->noiseFrequency.realp != NULL)
		free(self->noiseFrequency.realp);
	if (self->noiseFrequency.imagp != NULL)
		free(self->noiseFrequency.imagp);
	if (self->noiseAbs.realp != NULL)
		free(self->noiseAbs.realp);
	if (self->noiseAbs.imagp != NULL)
		free(self->noiseAbs.imagp);
	self->noiseTime.realp = malloc(dataPoints * sizeof(float));
	self->noiseTime.imagp = malloc(dataPoints * sizeof(float));
	self->noiseFrequency.realp = malloc(dataPoints * sizeof(float));
	self->noiseFrequency.imagp = malloc(dataPoints * sizeof(float));
	self->noiseAbs.realp = malloc(dataPoints * sizeof(float));
	self->noiseAbs.imagp = malloc(dataPoints * sizeof(float));
	for (i = 0; i < dataPoints; i++) {
        if (insensitive_settings_get_zeroFilling(self->settings) && (i > dataPoints / 2)) {
			self->noiseTime.realp[i] = 0.0;
			self->noiseTime.imagp[i] = 0.0;
		} else {
			self->noiseTime.realp[i] = random_noise(noiseLevel);
			self->noiseTime.imagp[i] = random_noise(noiseLevel);
		}
		self->noiseFrequency.realp[i] = self->noiseTime.realp[i] * self->apodizationT2[i];
		self->noiseFrequency.imagp[i] = self->noiseTime.imagp[i] * self->apodizationT2[i];
	}
	vDSP_fft_zip(self->fftsetup, &self->noiseFrequency, 1, lb(dataPoints), FFT_FORWARD);
	for (i = 0; i < dataPoints; i++) {
		self->noiseAbs.realp[i] = sqrtf(self->noiseFrequency.realp[i] * self->noiseFrequency.realp[i] + self->noiseFrequency.imagp[i] * self->noiseFrequency.imagp[i]);
		self->noiseAbs.imagp[i] = self->noiseAbs.realp[i];
	}
	if(get_showsFrequencyDomain((InsensitiveWindow *)self->displayController))
        set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseFrequency);
	else
	    set_noise_spectrum((InsensitiveWindow *)self->displayController, self->noiseTime);
}


void insensitive_controller_set_showRealPart(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_showRealPart(self->settings, value);
    set_showRealSpectrum((InsensitiveWindow *)self->displayController,
                         insensitive_settings_get_showRealPart(self->settings));
    set_showImaginarySpectrum((InsensitiveWindow *)self->displayController,
                              insensitive_settings_get_showImaginaryPart(self->settings));
}


void insensitive_controller_set_showImaginaryPart(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_showImaginaryPart(self->settings, value);
	set_showRealSpectrum((InsensitiveWindow *)self->displayController,
                         insensitive_settings_get_showRealPart(self->settings));
    set_showImaginarySpectrum((InsensitiveWindow *)self->displayController,
                              insensitive_settings_get_showImaginaryPart(self->settings));
}


void insensitive_controller_set_showIntegral(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_showIntegral(self->settings, value);
    set_showIntegral((InsensitiveWindow *)self->displayController,
                     insensitive_settings_get_showIntegral(self->settings));
}


void insensitive_controller_set_showWindowFunction(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_showWindowFunction(self->settings, value);
    set_show_windowFunction((InsensitiveWindow *)self->displayController,
                            insensitive_settings_get_showWindowFunction(self->settings));
}


void insensitive_controller_set_zeroFilling(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_zeroFilling(self->settings, value);

	set_zeroFilling((InsensitiveWindow *)self->displayController, insensitive_settings_get_zeroFilling(self->settings));
	insensitive_controller_stop_acquisition(self);
    insensitive_controller_reset_acquisition_for_dataPoints(self, insensitive_settings_get_dataPoints(self->settings));
    set_2D_mode((InsensitiveWindow *)self->displayController, FALSE);
}


void insensitive_controller_set_acquisitionAfterNextPulse(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_acquisitionAfterNextPulse(self->settings, value);
	set_acquisitionAfterNextPulse((InsensitiveWindow *)self->displayController, insensitive_settings_get_acquisitionAfterNextPulse(self->settings));
}


void insensitive_controller_set_pulseBeforeAcquisition(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_pulseBeforeAcquisition(self->settings, value);
	set_pulseBeforeAcquisition((InsensitiveWindow *)self->displayController, insensitive_settings_get_pulseBeforeAcquisition(self->settings));
}


void insensitive_controller_set_detectISignal(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_detectISpins(self->settings, value);
	set_detectISignal((InsensitiveWindow *)self->displayController, insensitive_settings_get_detectISpins(self->settings));
	if (value && insensitive_settings_get_detectSSpins(self->settings))
		insensitive_controller_set_detectSSignal(self, FALSE);
}


void insensitive_controller_set_detectSSignal(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_detectSSpins(self->settings, value);
	set_detectSSignal((InsensitiveWindow *)self->displayController, insensitive_settings_get_detectSSpins(self->settings));
	if (value && insensitive_settings_get_detectISpins(self->settings))
		insensitive_controller_set_detectISignal(self, FALSE);
}


void insensitive_controller_set_vectorDisplayType(InsensitiveController *self, enum VectorDisplayType value)
{
	insensitive_settings_set_vectorDisplayType(self->settings, value);
	set_vectorDisplayType((InsensitiveWindow *)self->displayController, insensitive_settings_get_vectorDisplayType(self->settings));
	insensitive_controller_initialise_grapefruit_path(self);
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_operatorBasis(InsensitiveController *self, enum OperatorBasis value)
{
	insensitive_settings_set_operatorBasis(self->settings, value);
	set_operatorBasis((InsensitiveWindow *)self->displayController, insensitive_settings_get_operatorBasis(self->settings));
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_color1stOrderCoherences(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_color1stOrderCoherences(self->settings, value);
	set_color1stOrderCoherences((InsensitiveWindow *)self->displayController, insensitive_settings_get_color1stOrderCoherences(self->settings));
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_matrixDisplayType(InsensitiveController *self, enum MatrixDisplayType value)
{
	insensitive_settings_set_matrixDisplayType(self->settings, value);
	set_matrixDisplayType((InsensitiveWindow *)self->displayController, insensitive_settings_get_matrixDisplayType(self->settings));
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_vectorDiagramType(InsensitiveController *self, enum VectorDiagramType value)
{
	insensitive_settings_set_vectorDiagramType(self->settings, value);
	set_vectorDiagramType((InsensitiveWindow *)self->displayController, insensitive_settings_get_vectorDiagramType(self->settings));
	spin_state_was_changed((InsensitiveWindow *)self->displayController);
}


void insensitive_controller_set_chemicalShiftUnitsToDegreesPerSecond(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_larmorFrequencyInDegreesPerSeconds(self->settings, value);
    set_chemicalShift_units_to_degreesPerSecond((InsensitiveWindow *)self->displayController,
                                                insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(self->settings));
}


void insensitive_controller_set_allowShiftAndCouplingButtons(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_allowShiftAndCouplingButtons(self->settings, value);
	if (!self->operationIsInProgress && !self->animationIsInProgress && !self->acquisitionIsInProgress)
		set_user_controls_enabled((InsensitiveWindow *)self->displayController, TRUE);
}


void insensitive_controller_set_playSound(InsensitiveController *self, gboolean value)
{
	insensitive_settings_set_playSoundAfterAcquisition(self->settings, value);
}


void insensitive_controller_set_detectionMethod(InsensitiveController *self, enum PurePhaseDetectionMethod value)
{
	insensitive_settings_set_detectionMethod(self->settings, value);
    set_detectionMethod((InsensitiveWindow *)self->displayController, value);
}


enum PurePhaseDetectionMethod insensitive_controller_get_detectionMethod(InsensitiveController *self)
{
	return insensitive_settings_get_detectionMethod(self->settings);
}
