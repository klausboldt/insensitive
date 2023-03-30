/* insensitive-preferences.c
 *
 * Copyright 2021-2023 Klaus Boldt
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

#include "insensitive-config.h"
#include "insensitive-preferences.h"
#include "insensitive-settings.h"
#include "insensitive-spinsystem.h"
#include "insensitive-controller.h"
#include "insensitive-window.h"


struct _InsensitivePreferences
{
    GtkWindow           parent_instance;

    InsensitiveWindow       *window;
	InsensitiveController   *controller;
    InsensitiveSettings     *settings;

    GtkToggleButton         *playSound_checkbox, *offResonance_checkbox, *allowShiftCoupling_checkbox;
    GtkToggleButton         *csv_radiobutton, *dat_radiobutton, *jdx_radiobutton, *txt_radiobutton, *png_radiobutton;
    GtkScale                *coherence_pathway_scale;
    GtkAdjustment           *coherence_pathway_adjustment;
    GtkLabel                *coherence_pathway_label;
    GtkFontButton           *font_button;
    GtkToggleButton         *autoUpdate_checkbox, *autoDownload_checkbox;
    GtkButton               *reset_button;
};


G_DEFINE_TYPE(InsensitivePreferences, insensitive_preferences, GTK_TYPE_WINDOW)


InsensitivePreferences *insensitive_preferences_new()
{
	return (InsensitivePreferences *)g_object_new(INSENSITIVE_TYPE_PREFERENCES, NULL);
}


static void insensitive_preferences_class_init(InsensitivePreferencesClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-preferences.ui");

	gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, playSound_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, offResonance_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, allowShiftCoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, csv_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, dat_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, jdx_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, txt_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, png_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, coherence_pathway_scale);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, coherence_pathway_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, coherence_pathway_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, font_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, autoUpdate_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, autoDownload_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePreferences, reset_button);
}


static void insensitive_preferences_dispose(GObject *gobject)
{
	InsensitivePreferences *self = (InsensitivePreferences *)gobject;

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

	G_OBJECT_CLASS(insensitive_preferences_parent_class)->dispose(gobject);
}


static void insensitive_preferences_finalize(GObject *gobject)
{
	InsensitivePreferences *self = (InsensitivePreferences *)gobject;

	G_OBJECT_CLASS(insensitive_preferences_parent_class)->finalize(gobject);
}


static void insensitive_preferences_init(InsensitivePreferences *self)
{
    float f;

    gtk_widget_init_template(GTK_WIDGET(self));

    for (f = 3; f <= 10; f += 1)
        gtk_scale_add_mark(self->coherence_pathway_scale, f, GTK_POS_BOTTOM, NULL);
}


G_MODULE_EXPORT void on_InsensitivePreferences_destroy(InsensitivePreferences *self, gpointer user_data)
{
}


void insensitive_preferences_set_controller(InsensitivePreferences *self, gpointer newController)
{
    float max_calcs;
    gchar *temp, *exponent = malloc(12 * sizeof(gchar));

    self->window = (InsensitiveWindow *)newController;
    self->controller = self->window->controller;
    self->settings = self->controller->settings;

    gtk_toggle_button_set_active(self->playSound_checkbox,
                                 insensitive_settings_get_playSoundAfterAcquisition(self->settings));
    gtk_toggle_button_set_active(self->offResonance_checkbox,
                                 insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings));
    gtk_toggle_button_set_active(self->allowShiftCoupling_checkbox,
                                 insensitive_settings_get_allowShiftAndCouplingButtons(self->settings));
    switch (insensitive_settings_get_exportFormat(self->settings)) {
    case CSV:
        gtk_toggle_button_set_active(self->csv_radiobutton, TRUE);
        break;
    case DAT:
        gtk_toggle_button_set_active(self->dat_radiobutton, TRUE);
        break;
    case JDX:
        gtk_toggle_button_set_active(self->jdx_radiobutton, TRUE);
        break;
    case TXT:
        gtk_toggle_button_set_active(self->txt_radiobutton, TRUE);
        break;
    case PNG:
        gtk_toggle_button_set_active(self->png_radiobutton, TRUE);
    }
    max_calcs = insensitive_settings_get_maxCoherenceCalculations(self->settings);
    gtk_adjustment_set_value(self->coherence_pathway_adjustment, max_calcs);
    sprintf(exponent, "%.0f", max_calcs);
    temp = replace_numbers_by_exponents(exponent);
    sprintf(exponent, "10%s", temp);
    gtk_label_set_text(self->coherence_pathway_label, exponent);
    free(temp);
    free(exponent);
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(self->font_button),
                              insensitive_settings_get_matrixFont(self->settings));

}


G_MODULE_EXPORT void on_playSound_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;

    insensitive_settings_set_playSoundAfterAcquisition(self->settings, gtk_toggle_button_get_active(checkbox));
}


G_MODULE_EXPORT void on_offResonance_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;

    insensitive_settings_set_ignoreOffResonanceEffectsForPulses(self->settings, gtk_toggle_button_get_active(checkbox));
    insensitive_controller_create_pulse_powerspectrum(self->controller);
    insensitive_pulse_shaper_set_ignoreOffResonanceEffectsForPulses(self->window->pulse_shaper_window,
                                                                    insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->settings));
}


G_MODULE_EXPORT void on_allowShiftCoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;

    insensitive_settings_set_allowShiftAndCouplingButtons(self->settings, gtk_toggle_button_get_active(checkbox));
}


G_MODULE_EXPORT void on_export_radiobutton_toggled(GtkToggleButton *radiobutton, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;

    if (radiobutton == self->csv_radiobutton)
        insensitive_settings_set_exportFormat(self->settings, CSV);
    else if (radiobutton == self->dat_radiobutton)
        insensitive_settings_set_exportFormat(self->settings, DAT);
    else if(radiobutton == self->jdx_radiobutton)
        insensitive_settings_set_exportFormat(self->settings, JDX);
    else if(radiobutton == self->txt_radiobutton)
        insensitive_settings_set_exportFormat(self->settings, TXT);
    else if(radiobutton == self->png_radiobutton)
        insensitive_settings_set_exportFormat(self->settings, PNG);
}


G_MODULE_EXPORT void on_coherence_pathway_adjustment_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;
    float max_calcs;
    gchar *temp, *exponent = malloc(12 * sizeof(gchar));

    max_calcs = ROUND(gtk_adjustment_get_value(adjustment));
    g_signal_handlers_block_by_func(G_OBJECT(adjustment), G_CALLBACK(on_coherence_pathway_adjustment_changed), (gpointer)self);
    gtk_adjustment_set_value(adjustment, max_calcs);
    g_signal_handlers_unblock_by_func(G_OBJECT(adjustment), G_CALLBACK(on_coherence_pathway_adjustment_changed), (gpointer)self);
    insensitive_settings_set_maxCoherenceCalculations(self->settings, max_calcs);
    sprintf(exponent, "%.0f", max_calcs);
    temp = replace_numbers_by_exponents(exponent);
    sprintf(exponent, "10%s", temp);
    gtk_label_set_text(self->coherence_pathway_label, exponent);
    free(temp);
    free(exponent);
}


G_MODULE_EXPORT void on_font_button_font_set(GtkFontButton *fontButton, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;
    gchar *fontname;

    fontname = g_strdup(pango_font_description_get_family(gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(fontButton))));
    insensitive_settings_set_matrixFont(self->settings, fontname);
    g_free(fontname);
}


G_MODULE_EXPORT void on_reset_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitivePreferences *self = (InsensitivePreferences *)user_data;

    insensitive_spinsystem_set_spins(self->controller->spinSystem, 2);
    insensitive_spinsystem_set_spintypearray(self->controller->spinSystem, 0);
    insensitive_settings_set_pulseEnvelope(self->settings, Rectangle);
    insensitive_settings_set_excitationProfile(self->settings, Mxy_Phase);
    insensitive_settings_create_pulseShape(self->settings);
    insensitive_settings_set_flipAngle(self->settings, standardFlipAngle);
    insensitive_settings_set_pulseDuration(self->settings, standardPulseDuration);
    insensitive_settings_set_pulseStrength(self->settings, standardPulseStrength);
    insensitive_settings_set_phase(self->settings, standardPhase);
	insensitive_settings_set_pulseArray(self->settings, standardPulseArray, 2, 0);
	insensitive_settings_set_pulseLength(self->settings, standardPulseLength);
	insensitive_settings_set_pulseFrequency(self->settings, standardPulseFrequency);
    insensitive_settings_set_strongCoupling(self->settings, FALSE);
    insensitive_settings_set_dipolarRelaxation(self->settings, FALSE);
    insensitive_settings_set_animates(self->settings, FALSE);
	insensitive_settings_set_relaxationWithEvolution(self->settings, standardRelaxationWithEvolution);
	insensitive_settings_set_T1(self->settings, standardT1);
	insensitive_settings_set_T2(self->settings, standardT2);
	insensitive_settings_set_correlationTime(self->settings, standardCorrelationTime);
	insensitive_settings_set_delay(self->settings, standardDelay);
	insensitive_settings_set_iDecoupling(self->settings, standardDecoupling);
    insensitive_settings_set_sDecoupling(self->settings, standardDecoupling);
    insensitive_settings_set_spinlock(self->settings, FALSE);
    insensitive_settings_set_dephasingJitter(self->settings, FALSE);
	insensitive_settings_set_gradientStrength(self->settings, standardGradientStrength);
	insensitive_settings_set_gradientDuration(self->settings, standardGradientDuration);
    insensitive_settings_set_diffusion(self->settings, FALSE);
	insensitive_settings_set_dataPoints(self->settings, standardDataPoints);
	insensitive_settings_set_dwellTime(self->settings, standardDwellTime);
    insensitive_settings_set_zeroFilling(self->settings, FALSE);
	insensitive_settings_set_noiseLevel(self->settings, standardNoiseLevel);
	insensitive_settings_set_detectISpins(self->settings, standardDetectISpins);
	insensitive_settings_set_detectSSpins(self->settings, standardDetectSSpins);
	insensitive_settings_set_vectorDisplayType(self->settings, standardVectorDisplayType);
	insensitive_settings_set_operatorBasis(self->settings, standardOperatorBasis);
	insensitive_settings_set_color1stOrderCoherences(self->settings, standardColor1stOrderCoherences);
	insensitive_settings_set_matrixDisplayType(self->settings, standardMatrixDisplayType);
	insensitive_settings_set_allowShiftAndCouplingButtons(self->settings, standardAllowShiftAndCouplingButtons);
	insensitive_settings_set_playSoundAfterAcquisition(self->settings, standardPlaySound);
	insensitive_settings_set_vectorDiagramType(self->settings, standardVectorDiagramType);
	insensitive_settings_set_signalToNoiseThreshold(self->settings, standardSignalToNoiseThreshold);
	insensitive_settings_set_maxCoherenceCalculations(self->settings, standardMaxCoherenceCalculations);
    insensitive_settings_set_showRealPart(self->settings, TRUE);
    insensitive_settings_set_showImaginaryPart(self->settings, FALSE);
    insensitive_settings_set_showIntegral(self->settings, FALSE);
    insensitive_settings_set_showGrid(self->settings, TRUE);
    insensitive_settings_set_showWindowFunction(self->settings, FALSE);
    insensitive_settings_set_larmorFrequencyInDegreesPerSeconds(self->settings, FALSE);
    insensitive_settings_set_showMatrix(self->settings, TRUE);
    insensitive_settings_set_matrixFont(self->settings, "Times");
    insensitive_settings_set_exportFormat(self->settings, CSV);
    insensitive_settings_set_ignoreOffResonanceEffectsForPulses(self->settings, FALSE);
    insensitive_settings_set_gyroCodeI(self->settings, gyro_1H);
    insensitive_settings_set_gyroCodeS(self->settings, gyro_13C);
    insensitive_settings_set_spectrumLineWidth(self->settings, 1.0);
    insensitive_controller_load_and_display_settings(self->controller);
    spin_number_was_changed(self->window);
    spin_state_was_changed(self->window);
    on_reset_constants_button_clicked(NULL, self->window);
    on_erase_button_clicked(NULL, self->window);
    insensitive_controller_calculate_energy_levels(self->controller);
    insensitive_controller_create_pulse_powerspectrum(self->controller);
    insensitive_controller_connect_pulseShaperController(self->controller, self->window->pulse_shaper_window);
}
