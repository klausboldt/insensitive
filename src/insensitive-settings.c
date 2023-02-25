/* insensitive-settings.c
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
#include "insensitive-library.h"
#include "insensitive-settings.h"
#include "insensitive-spinsystem.h"
#include "insensitive-pulsesequence.h"
#include "insensitive-controller.h"
#include "insensitive-window.h"


const gchar *group = "Settings";


G_DEFINE_TYPE(InsensitiveSettings, insensitive_settings, G_TYPE_OBJECT)


InsensitiveSettings* insensitive_settings_new()
{
	return (InsensitiveSettings *)g_object_new(INSENSITIVE_TYPE_SETTINGS, NULL);
}


static void insensitive_settings_dispose(GObject *gobject)
{
	InsensitiveSettings *self = (InsensitiveSettings *)gobject;

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

	insensitive_settings_save_defaults(self);
    g_key_file_free(self->defaultSettings);

	/* g_object_unref(self->sequence); */

	/* [self freeGradientArray]; */
	/* [actionsSinceLastGradient release]; */

	G_OBJECT_CLASS(insensitive_settings_parent_class)->dispose(gobject);
}


static void insensitive_settings_finalize(GObject *gobject)
{
	InsensitiveSettings *self = (InsensitiveSettings *)gobject;

	g_free(self->pulsePowerSpectrum);
	g_free(self->pulseShape.realp);
	g_free(self->pulseShape.imagp);

	G_OBJECT_CLASS(insensitive_settings_parent_class)->finalize(gobject);
}


static void insensitive_settings_class_init(InsensitiveSettingsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = insensitive_settings_dispose;
	object_class->finalize = insensitive_settings_finalize;
}


static void insensitive_settings_init(InsensitiveSettings *self)
{
    gchar *filename = g_build_filename(g_get_user_config_dir(), "insensitive.conf", NULL);

	self->saveSettings = TRUE;
	self->defaultSettings = g_key_file_new();
    g_key_file_load_from_file(self->defaultSettings, filename, G_KEY_FILE_KEEP_COMMENTS, NULL);
	insensitive_settings_load_defaults(self);
}


/*-(id)copyWithZone: (NSZone *)zone
{
	InsensitiveSettings *copy;

	[[NSUserDefaults standardUserDefaults] synchronize];
	copy = [[InsensitiveSettings alloc] init];

	return copy;
}*/


/*void insensitive_settings_use_spinsystem(InsensitiveSettings *self, InsensitiveSpinSystem *aSpinSystem)
{
	self->spinSystem = aSpinSystem;
	insensitive_settings_calculate_selected_spins(self);
}*/


void insensitive_settings_save_defaults(InsensitiveSettings *self)
{
    gchar *filename = g_build_filename(g_get_user_config_dir(), "insensitive.conf", NULL);

    g_key_file_save_to_file(self->defaultSettings, filename, NULL);
    free(filename);
}


void insensitive_settings_load_defaults(InsensitiveSettings *self)
{
	GKeyFile *keyfile = self->defaultSettings;
	GError *err = NULL;

	self->flipAngle = g_key_file_get_double(keyfile, group, "FlipAngle", &err);
	if (err != NULL) {
		self->flipAngle = standardFlipAngle;
		g_clear_error(&err);
	}
	self->pulseDuration = g_key_file_get_double(keyfile, group, "PulseDuration", &err);
	if (err != NULL) {
		self->pulseDuration = standardPulseDuration;
		g_clear_error(&err);
	}
	self->pulseStrength = g_key_file_get_double(keyfile, group, "PulseStrength", &err);
	if (err != NULL) {
		self->pulseStrength = standardPulseStrength;
		g_clear_error(&err);
	}
	self->phase = g_key_file_get_double(keyfile, group, "Phase", &err);
	if (err != NULL) {
		self->phase = standardPhase;
		g_clear_error(&err);
	}
	self->pulseArray = g_key_file_get_integer(keyfile, group, "PulseArray", &err);
	if (err != NULL) {
		self->pulseArray = standardPulseArray;
		g_clear_error(&err);
	}
	self->pulseLength = g_key_file_get_double(keyfile, group, "PulseLength", &err);
	if (err != NULL) {
		self->pulseLength = standardPulseLength;
		g_clear_error(&err);
	}
	self->pulseFrequency = g_key_file_get_double(keyfile, group, "PulseFrequency", &err);
	if (err != NULL) {
		self->pulseFrequency = standardPulseFrequency;
		g_clear_error(&err);
	}
	self->pulseEnvelope = (enum PulseEnvelope)g_key_file_get_integer(keyfile, group, "PulseEnvelope", &err);
	if (err != NULL) {
		self->pulseEnvelope = Rectangle;
		g_clear_error(&err);
	}
	self->excitationProfile = (enum ExcitationProfile)g_key_file_get_double(keyfile, group, "ExcitationProfile", &err);
	if (err != NULL) {
		self->excitationProfile = Mxy_Phase;
		g_clear_error(&err);
	}
	self->pulsePowerSpectrum = malloc(0.5 * pulsePowerSpectrumResolution * sizeof(float));
	self->pulseShape.realp = malloc(pulseShapeResolution * sizeof(float));
	self->pulseShape.imagp = malloc(pulseShapeResolution * sizeof(float));
	insensitive_settings_create_pulseShape(self);
	self->strongCoupling = g_key_file_get_boolean(keyfile, group, "StrongCoupling", NULL);
	if (err != NULL) {
		self->strongCoupling = FALSE;
		g_clear_error(&err);
	}
	self->dipolarRelaxation = g_key_file_get_boolean(keyfile, group, "DipolarRelaxation", NULL);
	if (err != NULL) {
		self->dipolarRelaxation = FALSE;
		g_clear_error(&err);
	}
	self->animates = FALSE;
	self->relaxationWithEvolution = g_key_file_get_boolean(keyfile, group, "RelaxationWithEvolution", NULL);
	if (err != NULL) {
		self->relaxationWithEvolution = FALSE;
		g_clear_error(&err);
	}
	self->T1 = g_key_file_get_double(keyfile, group, "T1", &err);
	if (err != NULL) {
		self->T1 = standardT1;
		g_clear_error(&err);
	}
	self->T2 = g_key_file_get_double(keyfile, group, "T2", &err);
	if (err != NULL) {
		self->T2 = standardT2;
		g_clear_error(&err);
	}
	self->correlationTime = g_key_file_get_double(keyfile, group, "CorrelationTime", &err);
	if (err != NULL) {
		self->correlationTime = standardCorrelationTime;
		g_clear_error(&err);
	}
	self->delay = g_key_file_get_double(keyfile, group, "Delay", &err);
	if (err != NULL) {
		self->delay = standardDelay;
		g_clear_error(&err);
	}
	self->dephasingJitter = g_key_file_get_boolean(keyfile, group, "DephasingJitter", NULL);
	if (err != NULL) {
		self->dephasingJitter = FALSE;
		g_clear_error(&err);
	}
	self->iDecoupling = g_key_file_get_boolean(keyfile, group, "IDecoupling", NULL);
	if (err != NULL) {
		self->iDecoupling = FALSE;
		g_clear_error(&err);
	}
	self->sDecoupling = g_key_file_get_boolean(keyfile, group, "SDecoupling", NULL);
	if (err != NULL) {
		self->sDecoupling = FALSE;
		g_clear_error(&err);
	}
	self->spinlock = g_key_file_get_boolean(keyfile, group, "Spinlock", NULL);
	if (err != NULL) {
		self->spinlock = FALSE;
		g_clear_error(&err);
	}
	self->gradientStrength = g_key_file_get_double(keyfile, group, "GradientStrength", &err);
	if (err != NULL) {
		self->gradientStrength = standardGradientStrength;
		g_clear_error(&err);
	}
	self->gradientDuration = g_key_file_get_double(keyfile, group, "GradientDuration", &err);
	if (err != NULL) {
		self->gradientDuration = standardGradientDuration;
		g_clear_error(&err);
	}
	self->diffusion = g_key_file_get_boolean(keyfile, group, "Diffusion", NULL);
	if (err != NULL) {
		self->diffusion = FALSE;
		g_clear_error(&err);
	}
	self->dataPoints = g_key_file_get_double(keyfile, group, "DataPoints", &err);
	if (err != NULL) {
		self->dataPoints = standardDataPoints;
		g_clear_error(&err);
	}
	self->dwellTime = g_key_file_get_double(keyfile, group, "DwellTime", &err);
	if (err != NULL) {
		self->dwellTime = standardDwellTime;
		g_clear_error(&err);
	}
	self->noiseLevel = g_key_file_get_double(keyfile, group, "NoiseLevel", &err);
	if (err != NULL) {
		self->noiseLevel = standardNoiseLevel;
		g_clear_error(&err);
	}
	self->zeroFilling = g_key_file_get_boolean(keyfile, group, "ZeroFilling", NULL);
	if (err != NULL) {
		self->zeroFilling = FALSE;
		g_clear_error(&err);
	}
	self->showRealPart = g_key_file_get_boolean(keyfile, group, "ShowRealSpectrum", &err);
	if (err != NULL) {
		self->showRealPart = TRUE;
		g_clear_error(&err);
	}
	self->showImaginaryPart = g_key_file_get_boolean(keyfile, group, "ShowImaginarySpectrum", NULL);
	if (err != NULL) {
		self->showImaginaryPart = FALSE;
		g_clear_error(&err);
	}
	self->showIntegral = g_key_file_get_boolean(keyfile, group, "Integral", NULL);
	if (err != NULL) {
		self->showIntegral = FALSE;
		g_clear_error(&err);
	}
	self->showWindowFunction = g_key_file_get_boolean(keyfile, group, "ShowWindowFunction", NULL);
	if (err != NULL) {
		self->showWindowFunction = FALSE;
		g_clear_error(&err);
	}
	self->detectISpins = g_key_file_get_boolean(keyfile, group, "DetectISpins", &err);
	if (err != NULL) {
		self->detectISpins = TRUE;
		g_clear_error(&err);
	}
	self->detectSSpins = g_key_file_get_boolean(keyfile, group, "DetectSSpins", NULL);
	if (err != NULL) {
		self->detectSSpins = FALSE;
		g_clear_error(&err);
	}
	self->vectorDisplayType = (enum VectorDisplayType)g_key_file_get_integer(keyfile, group, "VectorDisplayType", &err);
	if (err != NULL) {
		self->vectorDisplayType = standardVectorDisplayType;
		g_clear_error(&err);
	}
	self->operatorBasis = (enum OperatorBasis)g_key_file_get_double(keyfile, group, "OperatorBasis", &err);
	if (err != NULL) {
		self->operatorBasis = standardOperatorBasis;
		g_clear_error(&err);
	}
	self->color1stOrderCoherences = g_key_file_get_boolean(keyfile, group, "ColoredMatrix", &err);
	if (err != NULL) {
		self->color1stOrderCoherences = TRUE;
		g_clear_error(&err);
	}
	self->matrixDisplayType = (enum MatrixDisplayType)g_key_file_get_double(keyfile, group, "MatrixDisplayType", &err);
	if (err != NULL) {
		self->matrixDisplayType = standardMatrixDisplayType;
		g_clear_error(&err);
	}
	self->allowShiftAndCouplingButtons = g_key_file_get_boolean(keyfile, group, "AllowShiftAndCouplingButtons", NULL);
	if (err != NULL) {
		self->allowShiftAndCouplingButtons = FALSE;
		g_clear_error(&err);
	}
	self->playSoundAfterAcquisition = g_key_file_get_boolean(keyfile, group, "PlaySound", NULL);
	if (err != NULL) {
		self->playSoundAfterAcquisition = FALSE;
		g_clear_error(&err);
	}
	self->vectorDiagramType = (enum VectorDiagramType)g_key_file_get_integer(keyfile, group, "2DVectorMode", NULL);
	if (err != NULL) {
		self->vectorDiagramType = VectorDiagram3D;
		g_clear_error(&err);
	}
	self->larmorFrequencyInDegreesPerSeconds = g_key_file_get_boolean(keyfile, group, "LarmorFrequencyInDegreesPerSeconds", NULL);
	if (err != NULL) {
		self->larmorFrequencyInDegreesPerSeconds = FALSE;
		g_clear_error(&err);
	}
	self->showMatrix = g_key_file_get_boolean(keyfile, group, "ShowMatrix", &err);
	if (err != NULL) {
		self->showMatrix = TRUE;
		g_clear_error(&err);
	}
    self->matrixFont = g_key_file_get_string(keyfile, group, "MatrixFont", &err);
	if (err != NULL) {
        self->matrixFont = malloc(6 * sizeof(gchar));
		strcpy(self->matrixFont, "Sans");
		g_clear_error(&err);
	}
	self->detectionMethod = (enum PurePhaseDetectionMethod)g_key_file_get_integer(keyfile, group, "PurePhaseDetectionMethod", &err);
	if (err != NULL) {
		self->detectionMethod = None;
		g_clear_error(&err);
	}
	self->exportFormat = (enum ExportFormat)g_key_file_get_integer(keyfile, group, "ExportFormat", &err);
	if (err != NULL) {
		self->exportFormat = CSV;
		g_clear_error(&err);
	}
	self->ignoreOffResonanceEffectsForPulses = g_key_file_get_boolean(keyfile, group, "OffResonancePulses", &err);
    if (err != NULL) {
		self->ignoreOffResonanceEffectsForPulses = FALSE;
		g_clear_error(&err);
	}
	self->gyroCodeI = g_key_file_get_integer(keyfile, group, "GyroICode", &err);
	if (err != NULL) {
		self->gyroCodeI = 0;
		g_clear_error(&err);
	}
	self->gyroCodeS = g_key_file_get_integer(keyfile, group, "GyroSCode", &err);
	if (err != NULL) {
		self->gyroCodeS = 1;
		g_clear_error(&err);
	}
	self->signalToNoiseThreshold = g_key_file_get_double(keyfile, group, "SignalToNoiseThreshold", &err);
	if (err != NULL) {
		self->signalToNoiseThreshold = standardSignalToNoiseThreshold;
		g_clear_error(&err);
	}
    self->showGrid = g_key_file_get_boolean(keyfile, group, "DisplayGridForSpectrum", &err);
	if (err != NULL) {
		self->showGrid = TRUE;
		g_clear_error(&err);
	}
    self->maxCoherenceCalculations = g_key_file_get_double(keyfile, group, "MaxCoherenceCalculations", &err);
	if (err != NULL) {
		self->maxCoherenceCalculations = standardMaxCoherenceCalculations;
		g_clear_error(&err);
	}
    self->spectrumLineWidth = g_key_file_get_double(keyfile, group, "SpectrumLineWidth", &err);
	if (err != NULL) {
		self->spectrumLineWidth = 1.0;
		g_clear_error(&err);
	}
}


gchar* insensitive_settings_defaults_as_string(InsensitiveSettings *self)
{
	return g_key_file_to_data(self->defaultSettings, NULL, NULL);
}


void insensitive_settings_save_spinsystem(InsensitiveSettings *self, gpointer spinsystem)
{
    InsensitiveSpinSystem *ss = (InsensitiveSpinSystem *)spinsystem;
    const gchar *ss_group = "Spin System";
    gdouble *couplingMatrix_double;
    unsigned int i, size = pow(insensitive_spinsystem_get_spins(ss), 2);

    g_key_file_set_integer(self->defaultSettings, ss_group, "Spins", ss->spins);
    g_key_file_set_integer(self->defaultSettings, ss_group, "SpinTypeArray", ss->spinTypeArray);
    couplingMatrix_double = malloc(size * sizeof(double));
    for (i = 0; i < size; i++)
        couplingMatrix_double[i] = ss->couplingMatrix[i];
    g_key_file_set_double_list(self->defaultSettings, ss_group, "CouplingMatrix", couplingMatrix_double, size);
}


void insensitive_settings_load_spinsystem(InsensitiveSettings *self, gpointer spinsystem)
{
    InsensitiveSpinSystem *ss = (InsensitiveSpinSystem *)spinsystem;
    const gchar *ss_group = "Spin System";
    gdouble *couplingMatrix_double;
    float *couplingMatrix;
    gsize size;
    unsigned int i, spins, spinTypeArray;
    GError *err = NULL;

	spins = g_key_file_get_integer(self->defaultSettings, ss_group, "Spins", &err);
    if (err == NULL)
		insensitive_spinsystem_set_spins(ss, spins);
    else
        g_clear_error(&err);
    spinTypeArray = g_key_file_get_integer(self->defaultSettings, ss_group, "SpinTypeArray", &err);
	if (err == NULL)
		insensitive_spinsystem_set_spintypearray(ss, spinTypeArray);
    else
		g_clear_error(&err);
    couplingMatrix_double = g_key_file_get_double_list(self->defaultSettings, ss_group, "CouplingMatrix", &size, &err);
    if (err == NULL && size > 0) {
        couplingMatrix = malloc(size * sizeof(float));
		for (i = 0; i < size; i++)
            couplingMatrix[i] = couplingMatrix_double[i];
        insensitive_spinsystem_substitute_couplingmatrix(ss, couplingMatrix);
        free(couplingMatrix);
		free(couplingMatrix_double);
    } else
        g_clear_error(&err);
}


void insensitive_settings_save_pulsesequence(InsensitiveSettings *self, gpointer source)
{
	InsensitiveController *controller = (InsensitiveController *)source;
	InsensitivePulseSequence *pp = (InsensitivePulseSequence *)controller->pulseSequence;
	const gchar *pp_group = "Pulse Sequence";
	unsigned int i, size;
	gchar **pp_code, **phase_list;
	GPtrArray *phaseCyclingArray;

	size = insensitive_pulsesequence_get_number_of_elements(pp);
	if (size > 0) {
		g_key_file_set_string(self->defaultSettings, pp_group, "Name", controller->pulseSequenceName);
		g_key_file_set_integer(self->defaultSettings, pp_group, "PhaseCycles",
				       insensitive_controller_get_numberOfPhaseCycles(controller));
		phaseCyclingArray = insensitive_controller_get_phaseCyclingArray(controller);
		phase_list = malloc(phaseCyclingArray->len * sizeof(gchar *));
		for (i = 0; i < phaseCyclingArray->len; i++)
			phase_list[i] = g_ptr_array_index(phaseCyclingArray, i);
		g_key_file_set_string_list(self->defaultSettings, pp_group, "PhaseCyclingArray",
								   phase_list, phaseCyclingArray->len);
		free(phase_list);
		pp_code = malloc(size * sizeof(guchar *));
		for (i = 0; i < size; i++)
			pp_code[i] = g_base64_encode((const guchar *)insensitive_pulsesequence_get_element_at_index(pp, i),
						 				 sizeof(SequenceElement));
		g_key_file_set_string_list(self->defaultSettings, pp_group, "PulseSequence", pp_code, size);
		for (i = 0; i < size; i++)
			g_free(pp_code[i]);
		free(pp_code);
		g_key_file_set_integer(self->defaultSettings, pp_group, "PurePhaseDetectionMethod",
				       insensitive_controller_get_detectionMethod(controller));
		g_key_file_set_integer(self->defaultSettings, pp_group, "VariableEvolutionTime",
				       insensitive_controller_get_variableEvolutionTime(controller));
	}
}


void insensitive_settings_load_pulsesequence(InsensitiveSettings *self, gpointer source)
{
    InsensitiveWindow *window = (InsensitiveWindow *)source;
	InsensitiveController *controller = window->controller;
    GPtrArray *pp, *phaseCyclingArray;
    const gchar *pp_group = "Pulse Sequence";
    unsigned int i;
    gchar **pp_code, **phase_list, *name;
    gsize size, data_len;
    GError *err = NULL;
    SequenceElement *element;
    guint phaseCycles;
    gint variableEvolutionTime;
    enum PurePhaseDetectionMethod detectionMethod;

    pp_code = g_key_file_get_string_list(self->defaultSettings, pp_group, "PulseSequence", &size, &err);
    if (err == NULL && size > 0) {
        pp = g_ptr_array_new();
        for (i = 0; i < size; i++) {
            element = (SequenceElement *)g_base64_decode(pp_code[i], &data_len);
            g_ptr_array_add(pp, element);
        }
        insensitive_controller_substitute_pulseSequence(controller, pp);
        phaseCycles = g_key_file_get_integer(self->defaultSettings, pp_group, "PhaseCycles", &err);
        if (err == NULL) {
            phase_list = g_key_file_get_string_list(self->defaultSettings, pp_group, "PhaseCyclingArray", &size, &err);
            if (err == NULL) {
                phaseCyclingArray = g_ptr_array_new();
                for (i = 0; i < size; i++)
                    g_ptr_array_add(phaseCyclingArray, phase_list[i]);
                insensitive_controller_substitute_phaseCyclingArray(controller, phaseCyclingArray, phaseCycles);
            } else {
				g_clear_error(&err);
			}
	    } else {
	        g_clear_error(&err);
	    }
        variableEvolutionTime = g_key_file_get_integer(self->defaultSettings, pp_group, "VariableEvolutionTime", &err);
        if (err == NULL)
            insensitive_controller_set_variableEvolutionTime(controller, variableEvolutionTime);
        else
		    g_clear_error(&err);
        name = g_key_file_get_string(self->defaultSettings, pp_group, "Name", &err);
        if (err == NULL)
            insensitive_controller_set_name_for_pulseSequence(controller, name);
        else
		    g_clear_error(&err);
        detectionMethod = g_key_file_get_integer(self->defaultSettings, pp_group, "PurePhaseDetectionMethod", &err);
        if (err == NULL) {
            insensitive_controller_set_detectionMethod(controller, detectionMethod);
            if (insensitive_controller_get_variableEvolutionTime(controller) == 0) {
                gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, FALSE);
                insensitive_controller_set_detectionMethod(window->controller, None);
            } else {
                gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, TRUE);
                insensitive_controller_set_detectionMethod(window->controller, detectionMethod);
            }
        } else {
			g_clear_error(&err);
		}
    } else {
        g_clear_error(&err);
    }
    set_recording_button_clicked(window, FALSE);
    erase_coherencePathway(window);
    close_coherencePathway(window);
    update_pulseSequence(window);
}


/* Pulse Settings */

float insensitive_settings_get_flipAngle(InsensitiveSettings *self)
{
	return self->flipAngle;
}


void insensitive_settings_set_flipAngle(InsensitiveSettings *self, float value)
{
	if (value < 0.1)
		self->flipAngle = 0;
	else if (value > 360)
		self->flipAngle = 360;
	else
		self->flipAngle = value;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "FlipAngle", self->flipAngle);
}


float insensitive_settings_get_pulseDuration(InsensitiveSettings *self)
{
	return self->pulseDuration;
}


void insensitive_settings_set_pulseDuration(InsensitiveSettings *self, float value)
{
	if (value < 0.001)
		self->pulseDuration = 0.0;
	else if (value > 1000.0)
		self->pulseDuration = 1000.0;
	else
		self->pulseDuration = value;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "PulseDuration", self->pulseDuration);
}


float insensitive_settings_get_pulseStrength(InsensitiveSettings *self)
{
	return self->pulseStrength;
}


void insensitive_settings_set_pulseStrength(InsensitiveSettings *self, float value)
{
	if (value < 0.001)
		self->pulseStrength = 0.0;
	else if (value > 1000.0)
		self->pulseStrength = 1000.0;
	else
		self->pulseStrength = value;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "PulseStrength", self->pulseStrength);
}


float insensitive_settings_get_phase(InsensitiveSettings *self)
{
	return self->phase;
}


void insensitive_settings_set_phase(InsensitiveSettings *self, float value)
{
	if (value < 0)
		self->phase = 0;
	else if (value > 360)
		self->phase = 360;
	else
		self->phase = value;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "Phase", self->phase);
}


unsigned int insensitive_settings_get_pulseArray(InsensitiveSettings *self)
{
	return self->pulseArray;
}


gboolean insensitive_settings_get_allISpinsSelected(InsensitiveSettings *self)
{
	return self->allISpinsSelected;
}


gboolean insensitive_settings_get_allSSpinsSelected(InsensitiveSettings *self)
{
	return self->allSSpinsSelected;
}


gboolean insensitive_settings_get_someISpinsSelected(InsensitiveSettings *self)
{
	return self->someISpinsSelected;
}


gboolean insensitive_settings_get_someSSpinsSelected(InsensitiveSettings *self)
{
	return self->someSSpinsSelected;
}


gboolean insensitive_settings_get_allSpinsSelected(InsensitiveSettings *self)
{
	return self->allSpinsSelected;
}


void insensitive_settings_set_pulseArray(InsensitiveSettings *self, unsigned int value, unsigned int number_of_spins, unsigned int spinTypeArray)
{
	if (value < pow2(number_of_spins)) {
		self->pulseArray = value;
		insensitive_settings_calculate_selected_spins(self, number_of_spins, spinTypeArray);
		if (self->saveSettings)
			g_key_file_set_integer(self->defaultSettings, group, "PulseArray", self->pulseArray);
	}
}


void insensitive_settings_set_pulseArray_for_spinnumber(InsensitiveSettings *self, unsigned int number, gboolean value, unsigned int number_of_spins, unsigned int spinTypeArray)
{
	if (number < number_of_spins) {
		if (value) {
			self->pulseArray |= pow2(number);
		} else {
			self->pulseArray |= pow2(number);
			self->pulseArray -= pow2(number);
		}
		insensitive_settings_calculate_selected_spins(self, number_of_spins, spinTypeArray);
		if (self->saveSettings)
			g_key_file_set_integer(self->defaultSettings, group, "PulseArray", self->pulseArray);
	}
}


void insensitive_settings_calculate_selected_spins(InsensitiveSettings *self, unsigned int number_of_spins, unsigned int spinTypeArray)
{
	unsigned int mask, size;

	size = pow2(number_of_spins);
	self->allSpinsSelected = (self->pulseArray == size - 1);
	mask = spinTypeArray;
	self->allSSpinsSelected = ((self->pulseArray & mask) == mask) && (mask != 0);
	mask = spinTypeArray ^ (size - 1);
	self->allISpinsSelected = ((self->pulseArray & mask) == mask) && (mask != 0);
	self->someSSpinsSelected = (spinTypeArray & self->pulseArray) != 0;
	self->someISpinsSelected = (~spinTypeArray & self->pulseArray) != 0;
}


float insensitive_settings_get_pulseLength(InsensitiveSettings *self)
{
	self->pulseLength = roundf(pulseDurationToSliderScale(self->pulseDuration, self->flipAngle));
	if (self->pulseLength < 1) {
		g_print("Error: pulse length (%.3f) is smaller than 1 -> truncated", self->pulseLength);
		self->pulseLength = 1;
	} else if (self->pulseLength > 512) {
		g_print("Error: pulse length (%.3f) is larger than 512 -> truncated", self->pulseLength);
		self->pulseLength = 512;
	}

	return self->pulseLength;
}


void insensitive_settings_set_pulseLength(InsensitiveSettings *self, float value)
{
	if ((value >= 1) && (value <= 512))
		self->pulseLength = fabs(value);
	else if (value < 1)
		self->pulseLength = 1;
	else
		self->pulseLength = 512;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "PulseLength", self->pulseLength);
}


float insensitive_settings_get_pulseFrequency(InsensitiveSettings *self)
{
	return self->pulseFrequency;
}


void insensitive_settings_set_pulseFrequency(InsensitiveSettings *self, float value)
{
	if ((value >= -127) && (value <= 127))
		self->pulseFrequency = value;
	else if (value < -127)
		self->pulseFrequency = -127;
	else
		self->pulseFrequency = 127;

	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "PulseFrequency", self->pulseFrequency);
}


enum PulseEnvelope insensitive_settings_get_pulseEnvelope(InsensitiveSettings *self)
{
	return self->pulseEnvelope;
}


void insensitive_settings_set_pulseEnvelope(InsensitiveSettings *self, enum PulseEnvelope value)
{
	self->pulseEnvelope = value;
	insensitive_settings_create_pulseShape(self);

	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "PulseEnvelope", self->pulseEnvelope);
}


DSPSplitComplex insensitive_settings_get_pulseShape(InsensitiveSettings *self)
{
	return self->pulseShape;
}


void insensitive_settings_create_pulseShape(InsensitiveSettings *self)
{
	unsigned int i, n, centre;
	float A, B, factor, frequency, width;

	switch (self->pulseEnvelope) {
	case Rectangle:
		for (i = 0; i < pulseShapeResolution; i++) {
			self->pulseShape.realp[i] = 1.0;
			self->pulseShape.imagp[i] = 0.0;
		}
		break;
	case Gaussian:
        width = 3530;
		centre = pulseShapeResolution / 2;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 2.4365 * exp(-pow(i - centre + 0.5, 2) / width);
			self->pulseShape.realp[i] = factor;
			self->pulseShape.imagp[i] = 0.0;
		}
		break;
	case Sinc:
		width = 0.0739198; //18.92347 / pulseShapeResolution;
		centre = pulseShapeResolution / 2;
		for (i = 0; i < pulseShapeResolution; i++) {
			if (i == centre)
				factor = 1;
			else
				factor = 5.7545 * sincf((i - centre + 0.5) * width);
			self->pulseShape.realp[i] = fabsf(factor);
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case HypSec:
		break;
	case EBURP_1:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.23;
			for (n = 1; n <= 8; n++) {
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
				}
				factor += (A * cosf(n * frequency * i) + B * sinf(n * frequency * i));
			}
			self->pulseShape.realp[i] = fabsf(factor) * 4.348;
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case EBURP_2:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.26;
			for (n = 1; n <= 9; n++) {
				switch (n) {
				case 1:
					A = 0.91;
					B = -0.16;
					break;
				case 2:
					A = 0.29;
					B = -1.82;
					break;
				case 3:
					A = -1.28;
					B = 0.18;
					break;
				case 4:
					A = -0.05;
					B = 0.42;
					break;
				case 5:
					A = 0.04;
					B = 0.07;
					break;
				case 6:
					A = 0.02;
					B = 0.07;
					break;
				case 7:
					A = 0.06;
					B = -0.01;
					break;
				case 8:
					A = 0.00;
					B = -0.04;
					break;
				case 9:
					A = -0.02;
					B = 0.00;
					break;
				}
				factor += (A * cosf(n * frequency * i) + B * sinf(n * frequency * i));
			}
			self->pulseShape.realp[i] = fabsf(factor) * 3.855;
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case IBURP_1:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.50;
			for (n = 1; n <= 9; n++) {
				switch (n) {
				case 1:
					A = 0.70;
					B = -1.54;
					break;
				case 2:
					A = -0.15;
					B = 1.01;
					break;
				case 3:
					A = -0.94;
					B = -0.24;
					break;
				case 4:
					A = 0.11;
					B = -0.04;
					break;
				case 5:
					A = -0.02;
					B = 0.08;
					break;
				case 6:
					A = -0.04;
					B = -0.04;
					break;
				case 7:
					A = 0.01;
					B = -0.01;
					break;
				case 8:
					A = -0.02;
					B = 0.01;
					break;
				case 9:
					A = -0.01;
					B = -0.01;
					break;
				}
				factor += (A * cosf(n * frequency * i) + B * sinf(n * frequency * i));
			}
			self->pulseShape.realp[i] = fabsf(factor) * 2;
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case IBURP_2:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.50;
			for (n = 1; n <= 11; n++) {
				switch (n) {
				case 1:
					A = 0.81;
					B = -0.68;
					break;
				case 2:
					A = 0.07;
					B = -1.38;
					break;
				case 3:
					A = -1.25;
					B = 0.20;
					break;
				case 4:
					A = -0.24;
					B = 0.45;
					break;
				case 5:
					A = 0.07;
					B = 0.23;
					break;
				case 6:
					A = 0.11;
					B = 0.05;
					break;
				case 7:
					A = 0.05;
					B = -0.04;
					break;
				case 8:
					A = -0.02;
					B = -0.04;
					break;
				case 9:
					A = -0.03;
					B = 0.00;
					break;
				case 10:
					A = -0.02;
					B = 0.01;
					break;
				case 11:
					A = 0.00;
					B = 0.01;
					break;
				}
				factor += (A * cosf(n * frequency * i) + B * sinf(n * frequency * i));
			}
			self->pulseShape.realp[i] = fabsf(factor) * 2;
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case UBURP:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.27;
			for (n = 1; n <= 20; n++) {
				switch (n) {
				case 1:
					A = -1.42;
					break;
				case 2:
					A = -0.37;
					break;
				case 3:
					A = -1.84;
					break;
				case 4:
					A = 4.40;
					break;
				case 5:
					A = -1.19;
					break;
				case 6:
					A = 0.00;
					break;
				case 7:
					A = -0.37;
					break;
				case 8:
					A = 0.50;
					break;
				case 9:
					A = -0.31;
					break;
				case 10:
					A = 0.18;
					break;
				case 11:
					A = -0.21;
					break;
				case 12:
					A = 0.23;
					break;
				case 13:
					A = -0.12;
					break;
				case 14:
					A = 0.07;
					break;
				case 15:
					A = -0.06;
					break;
				case 16:
					A = 0.06;
					break;
				case 17:
					A = -0.04;
					break;
				case 18:
					A = 0.03;
					break;
				case 19:
					A = -0.02;
					break;
				case 20:
					A = 0.02;
					break;
				}
				factor += A * cosf(n * frequency * i);
			}
			self->pulseShape.realp[i] = fabsf(factor);
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case REBURP:
		frequency = 2 * M_PI / pulseShapeResolution;
		for (i = 0; i < pulseShapeResolution; i++) {
			factor = 0.49;
			for (n = 1; n <= 15; n++) {
				switch (n) {
				case 1:
					A = -1.02;
					break;
				case 2:
					A = 1.11;
					break;
				case 3:
					A = -1.57;
					break;
				case 4:
					A = 0.83;
					break;
				case 5:
					A = -0.42;
					break;
				case 6:
					A = 0.26;
					break;
				case 7:
					A = -0.16;
					break;
				case 8:
					A = 0.10;
					break;
				case 9:
					A = -0.07;
					break;
				case 10:
					A = 0.04;
					break;
				case 11:
					A = -0.03;
					break;
				case 12:
					A = 0.01;
					break;
				case 13:
					A = -0.02;
					break;
				case 14:
					A = 0.00;
					break;
				case 15:
					A = -0.01;
					break;
				}
				factor += A * cosf(n * frequency * i);
			}
			self->pulseShape.realp[i] = fabsf(factor);
			self->pulseShape.imagp[i] = (factor >= 0) ? 0.0 : 180.0;
		}
		break;
	case DANTE:
		for (i = 0; i < (int)pulseShapeResolution / (int)maxDanteCycles; i++) {
			self->pulseShape.realp[i] = 1.0;
			self->pulseShape.imagp[i] = 0.0;
		}
		for (i = pulseShapeResolution / maxDanteCycles; i < pulseShapeResolution; i++) {
			self->pulseShape.realp[i] = 0.0;
			self->pulseShape.imagp[i] = 0.0;
		}
		break;
	}
}


float *insensitive_settings_get_pulsePowerSpectrum(InsensitiveSettings *self)
{
	return self->pulsePowerSpectrum;
}


void insensitive_settings_set_pulsePowerSpectrum(InsensitiveSettings *self, float *powerSpectrum)
{
	int i;

	for (i = 0; i < pulsePowerSpectrumResolution / 2; i++)
		self->pulsePowerSpectrum[i] = powerSpectrum[i];
}


enum ExcitationProfile insensitive_settings_get_excitationProfile(InsensitiveSettings *self)
{
	return self->excitationProfile;
}


void insensitive_settings_set_excitationProfile(InsensitiveSettings *self, enum ExcitationProfile value)
{
	self->excitationProfile = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "ExcitationProfile", self->excitationProfile);
}


/* Free Evolution Settings */

gboolean insensitive_settings_get_strongCoupling(InsensitiveSettings *self)
{
	return self->strongCoupling;
}


void insensitive_settings_set_strongCoupling(InsensitiveSettings *self, gboolean value)
{
	self->strongCoupling = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "StrongCoupling", self->strongCoupling);
}


gboolean insensitive_settings_get_dipolarRelaxation(InsensitiveSettings *self)
{
	return self->dipolarRelaxation;
}


void insensitive_settings_set_dipolarRelaxation(InsensitiveSettings *self, gboolean value)
{
	self->dipolarRelaxation = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "DipolarRelaxation", self->dipolarRelaxation);
}


gboolean insensitive_settings_get_animates(InsensitiveSettings *self)
{
	return self->animates;
}


void insensitive_settings_set_animates(InsensitiveSettings *self, gboolean value)
{
	self->animates = value;
}


gboolean insensitive_settings_get_relaxationWithEvolution(InsensitiveSettings *self)
{
	return self->relaxationWithEvolution;
}


void insensitive_settings_set_relaxationWithEvolution(InsensitiveSettings *self, gboolean value)
{
	self->relaxationWithEvolution = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "RelaxationWithEvolution", self->relaxationWithEvolution);
}


float insensitive_settings_get_T1(InsensitiveSettings *self)
{
	return self->T1;
}


void insensitive_settings_set_T1(InsensitiveSettings *self, float value)
{
	if (value < 1)
		value = 1;

	if (value > 99)
		value = 99;

	self->T1 = value;
	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "T1", self->T1);

	if (self->T2 > self->T1) {
		self->T2 = self->T1;
		if (self->saveSettings)
			g_key_file_set_double(self->defaultSettings, group, "T2", self->T2);
	}
}


float insensitive_settings_get_T2(InsensitiveSettings *self)
{
	return self->T2;
}


void insensitive_settings_set_T2(InsensitiveSettings *self, float value)
{
	if (value < 1)
		value = 1;

	if (value > 99)
		value = 99;

	if (value > self->T1)
		self->T2 = self->T1;
	else
		self->T2 = value;

	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "T2", self->T2);
}


float insensitive_settings_get_correlationTime(InsensitiveSettings *self)
{
	return self->correlationTime;
}


void insensitive_settings_set_correlationTime(InsensitiveSettings *self, float value)
{
	if ((value > 0) && (value <= 50))
		self->correlationTime = value;
	else
		self->correlationTime = standardCorrelationTime;

	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "CorrelationTime", self->correlationTime);
}


float insensitive_settings_get_delay(InsensitiveSettings *self)
{
	return self->delay;
}


void insensitive_settings_set_delay(InsensitiveSettings *self, float value)
{
	if (value > 0) {
		self->delay = value;
		if (self->saveSettings)
			g_key_file_set_boolean(self->defaultSettings, group, "Delay", self->delay);
	}
}


gboolean insensitive_settings_get_dephasingJitter(InsensitiveSettings *self)
{
	return self->dephasingJitter;
}


void insensitive_settings_set_dephasingJitter(InsensitiveSettings *self, gboolean value)
{
	self->dephasingJitter = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "DephasingJitter", self->dephasingJitter);
}


gboolean insensitive_settings_get_iDecoupling(InsensitiveSettings *self)
{
	return self->iDecoupling;
}


void insensitive_settings_set_iDecoupling(InsensitiveSettings *self, gboolean value)
{
	self->iDecoupling = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "IDecoupling", self->iDecoupling);
}


gboolean insensitive_settings_get_sDecoupling(InsensitiveSettings *self)
{
	return self->sDecoupling;
}


void insensitive_settings_set_sDecoupling(InsensitiveSettings *self, gboolean value)
{
	self->sDecoupling = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "SDecoupling", self->sDecoupling);
}


gboolean insensitive_settings_get_spinlock(InsensitiveSettings *self)
{
	return self->spinlock;
}


void insensitive_settings_set_spinlock(InsensitiveSettings *self, gboolean value)
{
	self->spinlock = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "Spinlock", self->spinlock);
}


/* Gradient Settings */

float insensitive_settings_get_gradientStrength(InsensitiveSettings *self)
{
	return self->gradientStrength;
}


void insensitive_settings_set_gradientStrength(InsensitiveSettings *self, float value)
{
	if (value > 32000)
		self->gradientStrength = 32000;
	else if (value < -32000)
		self->gradientStrength = -32000;
	else
		self->gradientStrength = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "GradientStrength", self->gradientStrength);
}


float insensitive_settings_get_gradientDuration(InsensitiveSettings *self)
{
	return self->gradientDuration;
}


void insensitive_settings_set_gradientDuration(InsensitiveSettings *self, float value)
{
	if (value > 0)
		self->gradientDuration = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "GradientDuration", self->gradientDuration);
}


gboolean insensitive_settings_get_diffusion(InsensitiveSettings *self)
{
	return self->diffusion;
}


void insensitive_settings_set_diffusion(InsensitiveSettings *self, gboolean value)
{
	self->diffusion = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "Diffusion", self->diffusion);
}


/* Specrum Settings */

unsigned int insensitive_settings_get_dataPoints(InsensitiveSettings *self)
{
	return self->zeroFilling ? 2 * self->dataPoints : self->dataPoints;
}


void insensitive_settings_set_dataPoints(InsensitiveSettings *self, unsigned int value)
{
	self->dataPoints = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "DataPoints", self->dataPoints);
}


void insensitive_settings_set_logDataPoints(InsensitiveSettings *self, unsigned int value)
{
	insensitive_settings_set_dataPoints(self, pow2(value));
}


float insensitive_settings_get_dwellTime(InsensitiveSettings *self)
{
	return self->dwellTime;
}


void insensitive_settings_set_dwellTime(InsensitiveSettings *self, float value)
{
	if (value > 0)
		self->dwellTime = value;
	else
		self->dwellTime = standardDwellTime;
	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "DwellTime", self->dwellTime);
}


float insensitive_settings_get_noiseLevel(InsensitiveSettings *self)
{
	return self->noiseLevel;
}


void insensitive_settings_set_noiseLevel(InsensitiveSettings *self, float value)
{
	if ((value >= 0) && (value <= 100))
		self->noiseLevel = value;
	if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "NoiseLevel", self->noiseLevel);
}


gboolean insensitive_settings_get_zeroFilling(InsensitiveSettings *self)
{
	return self->zeroFilling;
}


void insensitive_settings_set_zeroFilling(InsensitiveSettings *self, gboolean value)
{
	self->zeroFilling = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ZeroFilling", self->zeroFilling);
}


gboolean insensitive_settings_get_showRealPart(InsensitiveSettings *self)
{
	return self->showRealPart;
}


void insensitive_settings_set_showRealPart(InsensitiveSettings *self, gboolean value)
{
	self->showRealPart = value;
	if (!self->showRealPart && !self->showImaginaryPart) {
		self->showImaginaryPart = TRUE;
		if (self->saveSettings)
			g_key_file_set_boolean(self->defaultSettings, group, "ShowImaginarySpectrum", self->showImaginaryPart);
	}
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ShowRealSpectrum", self->showRealPart);
}


gboolean insensitive_settings_get_showImaginaryPart(InsensitiveSettings *self)
{
	return self->showImaginaryPart;
}


void insensitive_settings_set_showImaginaryPart(InsensitiveSettings *self, gboolean value)
{
	self->showImaginaryPart = value;
	if (!self->showRealPart && !self->showImaginaryPart) {
		self->showRealPart = TRUE;
		if (self->saveSettings)
			g_key_file_set_boolean(self->defaultSettings, group, "ShowRealSpectrum", self->showRealPart);
	}
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ShowImaginarySpectrum", self->showImaginaryPart);
}


gboolean insensitive_settings_get_showIntegral(InsensitiveSettings *self)
{
	return self->showIntegral;
}


void insensitive_settings_set_showIntegral(InsensitiveSettings *self, gboolean value)
{
	self->showIntegral = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "Integral", self->showIntegral);
}


gboolean insensitive_settings_get_showWindowFunction(InsensitiveSettings *self)
{
	return self->showWindowFunction;
}


void insensitive_settings_set_showWindowFunction(InsensitiveSettings *self, gboolean value)
{
	self->showWindowFunction = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ShowWindowFunction", self->showWindowFunction);
}


gboolean insensitive_settings_get_pulseBeforeAcquisition(InsensitiveSettings *self)
{
	return self->pulseBeforeAcquisition;
}


void insensitive_settings_set_pulseBeforeAcquisition(InsensitiveSettings *self, gboolean value)
{
	self->pulseBeforeAcquisition = value;
}


gboolean insensitive_settings_get_acquisitionAfterNextPulse(InsensitiveSettings *self)
{
	return self->acquisitionAfterNextPulse;
}


void insensitive_settings_set_acquisitionAfterNextPulse(InsensitiveSettings *self, gboolean value)
{
	self->acquisitionAfterNextPulse = value;
}


gboolean insensitive_settings_get_detectISpins(InsensitiveSettings *self)
{
	return self->detectISpins;
}


void insensitive_settings_set_detectISpins(InsensitiveSettings *self, gboolean value)
{
	self->detectISpins = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "DetectISpins", self->detectISpins);
}


gboolean insensitive_settings_get_detectSSpins(InsensitiveSettings *self)
{
	return self->detectSSpins;
}


void insensitive_settings_set_detectSSpins(InsensitiveSettings *self, gboolean value)
{
	self->detectSSpins = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "DetectSSpins", self->detectSSpins);
}


/* Display Settings */

enum VectorDisplayType insensitive_settings_get_vectorDisplayType(InsensitiveSettings *self)
{
	return self->vectorDisplayType;
}


void insensitive_settings_set_vectorDisplayType(InsensitiveSettings *self, enum VectorDisplayType value)
{
	self->vectorDisplayType = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "VectorDisplayType", self->vectorDisplayType);
}


enum OperatorBasis insensitive_settings_get_operatorBasis(InsensitiveSettings *self)
{
	return self->operatorBasis;
}


void insensitive_settings_set_operatorBasis(InsensitiveSettings *self, enum OperatorBasis value)
{
	self->operatorBasis = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "OperatorBasis", self->operatorBasis);
}


gboolean insensitive_settings_get_color1stOrderCoherences(InsensitiveSettings *self)
{
	return self->color1stOrderCoherences;
}


void insensitive_settings_set_color1stOrderCoherences(InsensitiveSettings *self, gboolean value)
{
	self->color1stOrderCoherences = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ColoredMatrix", self->color1stOrderCoherences);
}


enum MatrixDisplayType insensitive_settings_get_matrixDisplayType(InsensitiveSettings *self)
{
	return self->matrixDisplayType;
}


void insensitive_settings_set_matrixDisplayType(InsensitiveSettings *self, enum MatrixDisplayType value)
{
	self->matrixDisplayType = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "MatrixDisplayType", self->matrixDisplayType);
}


/* Other Options */

gboolean insensitive_settings_get_allowShiftAndCouplingButtons(InsensitiveSettings *self)
{
	return self->allowShiftAndCouplingButtons;
}


void insensitive_settings_set_allowShiftAndCouplingButtons(InsensitiveSettings *self, gboolean value)
{
	self->allowShiftAndCouplingButtons = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "AllowShiftAndCouplingButtons", self->allowShiftAndCouplingButtons);
}


gboolean insensitive_settings_get_playSoundAfterAcquisition(InsensitiveSettings *self)
{
	return self->playSoundAfterAcquisition;
}


void insensitive_settings_set_playSoundAfterAcquisition(InsensitiveSettings *self, gboolean value)
{
	self->playSoundAfterAcquisition = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "PlaySound", self->playSoundAfterAcquisition);
}


enum VectorDiagramType insensitive_settings_get_vectorDiagramType(InsensitiveSettings *self)
{
	return self->vectorDiagramType;
}


void insensitive_settings_set_vectorDiagramType(InsensitiveSettings *self, enum VectorDiagramType value)
{
	self->vectorDiagramType = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "2DVectorMode", self->vectorDiagramType);
}


gboolean insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(InsensitiveSettings *self)
{
	return self->larmorFrequencyInDegreesPerSeconds;
}


void insensitive_settings_set_larmorFrequencyInDegreesPerSeconds(InsensitiveSettings *self, gboolean value)
{
	self->larmorFrequencyInDegreesPerSeconds = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "LarmorFrequencyInDegreesPerSeconds", self->larmorFrequencyInDegreesPerSeconds);
}


gboolean insensitive_settings_get_showMatrix(InsensitiveSettings *self)
{
	return self->showMatrix;
}


void insensitive_settings_set_showMatrix(InsensitiveSettings *self, gboolean value)
{
	self->showMatrix = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "ShowMatrix", self->showMatrix);
}


gchar *insensitive_settings_get_matrixFont(InsensitiveSettings *self)
{
    return self->matrixFont;
}


void insensitive_settings_set_matrixFont(InsensitiveSettings *self, const gchar *fontname)
{
    if (fontname != NULL) {
        if (self->matrixFont == NULL)
            self->matrixFont = malloc((strlen(fontname) + 1) * sizeof(gchar));
        strcpy(self->matrixFont, fontname);
        if (self->saveSettings)
		    g_key_file_set_string(self->defaultSettings, group, "MatrixFont", self->matrixFont);
    }
}


enum PurePhaseDetectionMethod insensitive_settings_get_detectionMethod(InsensitiveSettings *self)
{
	return self->detectionMethod;
}


void insensitive_settings_set_detectionMethod(InsensitiveSettings *self, enum PurePhaseDetectionMethod value)
{
	self->detectionMethod = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "PurePhaseDetectionMethod", self->detectionMethod);
}


gboolean insensitive_settings_get_showGrid(InsensitiveSettings *self)
{
	return self->showGrid;
}


void insensitive_settings_set_showGrid(InsensitiveSettings *self, gboolean value)
{
	self->showGrid = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "DisplayGridForSpectrum", self->showGrid);
}


enum ExportFormat insensitive_settings_get_exportFormat(InsensitiveSettings *self)
{
	return self->exportFormat;
}


void insensitive_settings_set_exportFormat(InsensitiveSettings *self, enum ExportFormat value)
{
	self->exportFormat = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "ExportFormat", self->exportFormat);
}


gboolean insensitive_settings_get_ignoreOffResonanceEffectsForPulses(InsensitiveSettings *self)
{
	return self->ignoreOffResonanceEffectsForPulses;
}


void insensitive_settings_set_ignoreOffResonanceEffectsForPulses(InsensitiveSettings *self, gboolean value)
{
	self->ignoreOffResonanceEffectsForPulses = value;
	if (self->saveSettings)
		g_key_file_set_boolean(self->defaultSettings, group, "OffResonancePulses", self->ignoreOffResonanceEffectsForPulses);
}


unsigned int insensitive_settings_get_gyroCodeI(InsensitiveSettings *self)
{
	return self->gyroCodeI;
}


void insensitive_settings_set_gyroCodeI(InsensitiveSettings *self, unsigned int value)
{
	self->gyroCodeI = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "GyroCodeI", self->gyroCodeI);
}


unsigned int insensitive_settings_get_gyroCodeS(InsensitiveSettings *self)
{
	return self->gyroCodeS;
}


void insensitive_settings_set_gyroCodeS(InsensitiveSettings *self, unsigned int value)
{
	self->gyroCodeS = value;
	if (self->saveSettings)
		g_key_file_set_integer(self->defaultSettings, group, "GyroCodeS", self->gyroCodeS);
}


float insensitive_settings_get_signalToNoiseThreshold(InsensitiveSettings *self)
{
	return self->signalToNoiseThreshold;
}


void insensitive_settings_set_signalToNoiseThreshold(InsensitiveSettings *self, float value)
{
	if (value >= 0) {
		self->signalToNoiseThreshold = value;
		if (self->saveSettings)
			g_key_file_set_double(self->defaultSettings, group, "SignalToNoiseThreshold", self->signalToNoiseThreshold);
	}
}


float insensitive_settings_get_maxCoherenceCalculations(InsensitiveSettings *self)
{
	return self->maxCoherenceCalculations;
}


void insensitive_settings_set_maxCoherenceCalculations(InsensitiveSettings *self, float value)
{
	if (value >= 0) {
		self->maxCoherenceCalculations = value;
		if (self->saveSettings)
			g_key_file_set_double(self->defaultSettings, group, "MaxCoherenceCalculations", self->maxCoherenceCalculations);
	}
}


float insensitive_settings_get_spectrumLineWidth(InsensitiveSettings *self)
{
	return self->spectrumLineWidth;
}


void insensitive_settings_set_spectrumLineWidth(InsensitiveSettings *self, float value)
{
    if (value >= 0.5) {
        self->spectrumLineWidth = value;
    } else {
        self->spectrumLineWidth = 0.5;
    }
    if (self->saveSettings)
		g_key_file_set_double(self->defaultSettings, group, "SpectrumLineWidth", self->spectrumLineWidth);
}
