/* insensitive-window.c
 *
 * Copyright 2021-2023 Klaus Boldt
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or selexport_sl copies of the Software, and to
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
#include "insensitive-window.h"
#include "insensitive-pulsesequence.h"
#include "insensitive-settings.h"
#include "insensitive-spinsystem.h"
#include "insensitive-pulseshaper.h"
#include "insensitive-singlespins.h"
#include "insensitive-composer.h"
#include "insensitive-preferences.h"
#ifdef USE_WEBKIT_GTK
#include "insensitive-tutorial.h"
#endif /* USE_WEBKIT_GTK */


static const float icon_half_width = 32;
static const int GAP_TOP = 17;
static const int GAP_BOTTOM = 29;
static const int GAP_LEFT = 24;
static const float contourInterval = 3.25;
static const float contourCellSize = 4.0;
static const float contourBase = -1.0;
static const unsigned int countOfContours = 7;


G_DEFINE_TYPE(InsensitiveWindow, insensitive_window, GTK_TYPE_APPLICATION_WINDOW)


static void insensitive_window_class_init(InsensitiveWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-window.ui");

	/* Main window */
	//gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, header_bar);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, command_line);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinsystem_toolbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinstate_toolbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulsesequence_toolbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrum_toolbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, mainwindow_notebook);

	/* Spin system */
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinNumber_spinbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinNumber_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, chemicalShift_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, chemicalShift_unit_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, displayedConstant_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, scalarConstant_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dipolarConstant_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, distanceConstant_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, chemicalShift_units_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gyroI_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gyroS_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, addSpin_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, removeSpin_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, reset_constants_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, rotate_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, strong_coupling_checkbox_spinsystem);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinEditor_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, energyLevel_drawingarea);

	/* Spin state */
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, productoperator_textbuffer);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, chemicalShift_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, coupling_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, relaxation_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, freeEvolution_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gradient_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, acquire_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, equilibrium_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, undo_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, acquisition_spinner);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, iSpinVector_drawingarea);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, sSpinVector_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, matrix_drawingarea);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse90x_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse90y_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse90minusx_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse90minusy_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse180x_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse180y_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse180minusx_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulse180minusy_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, expandPulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, contractPulse_button);

	/* Settings */
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseEnvelope_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, flipAngle_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, flipAngle_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, flipAngle_scale);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseDuration_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseStrength_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, hardpulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, softpulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, softerpulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, selectivepulse_button);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseFrequency_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseFrequency_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseFrequency_scale);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phase_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phase_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phase_scale);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spin1_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spin2_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spin3_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spin4_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, ispins_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, sspins_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, allspins_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, strong_coupling_checkbox_spinstate);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dipolar_relaxation_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, animation_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, include_relaxation_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, T1_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, T2_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, correlationTime_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, delay_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, delay_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dephasingJitter_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, iDecoupling_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, sDecoupling_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spinlock_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gradient_strength_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gradient_duration_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gradient_strength_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, diffusion_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, datapoints_spinbutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, datapoints_adjustment);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dwelltime_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, noiseLevel_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, acquisitionAfterNextPulse_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, detectISignal_radiobutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, detectSSignal_radiobutton);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, zeroFilling_checkbox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, vectorDisplayType_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, matrixDisplayType_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, operatorBasis_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, vectorDiagramType_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, color1stOrderCoherences_checkbox);

	/* Pulse sequence */
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseSequence_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseSequenceStep_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, evolutionTimes_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, detectionMethod_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phaseCycles_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phaseCycles_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, bottomDisplay_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, bottomDisplay_notebook);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, record_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, play_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, step_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, phaseCycling_treeview);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, coherencePathway_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pulseProgram_textbuffer);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_notebook);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_shape_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_idecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_sdecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_pulse_ok_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_idecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_sdecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_spinlock_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_delay_ok_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_gradient_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_gradient_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_gradient_ok_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_idetect_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_sdetect_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_idecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_sdecoupling_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_spinlock_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pp_edit_fid_ok_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, step_window);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, stepWindow_button);

	/* Spectrum*/
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrum_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrum_tools_stack);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, fourier_stack_child);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrum_progressbar);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, fid_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, fft1D_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, fft2D_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, magnitude_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dataPoints_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, noiseLevelSpectrum_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, noiseLevelSpectrum_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, apodization_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianWidth_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianWidthUnit_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianShift_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianWidth_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianShift_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, gaussianShift_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, zeroOrder_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, firstOrder_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pivotPoint_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, zeroOrder_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, firstOrder_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, pivotPoint_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, showReal_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, showImaginary_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, integral_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, grid_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, window_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, shiftBaseline_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, plotStyle_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, plotStyle_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, lineWidth_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, lineWidth_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, lineWidth_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, signalToNoiseThreshold_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, symmetrize_menubutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, tilt_menubutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyToolbox_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, absDispReal_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, absDispImag_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dataSet_label);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrumParameters_textview);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, spectrumParameters_textbuffer);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, toggleParameters_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyToolBox_window);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyFirstTrace_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyPickPeaks_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyFitLorentzian_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyFitExponential_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveWindow, dosyPlotSpectrum_button);
}


static void insensitive_window_init(InsensitiveWindow *self)
{
	InsensitiveSpinSystem *spinsystem;
	InsensitiveSettings *settings;
    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info;
    gchar *str;
    float f;

	gtk_widget_init_template(GTK_WIDGET(self));

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, "#grey_scrollview {background-color: @theme_bg_color;}\n#grey_textview text {background-color: @theme_bg_color;}\n#grey_textview {font: 15px \"Monospace\";}\n#command_line {background-color: white; border: none; box-shadow: none;}\n#pulseSequence_toolbar {background-color: #CCCCCC;}", -1, NULL);

	self->spin_checkbox_array = malloc(4 * sizeof(GtkToggleButton *));
	self->spin_checkbox_array[0] = self->spin1_checkbox;
	self->spin_checkbox_array[1] = self->spin2_checkbox;
	self->spin_checkbox_array[2] = self->spin3_checkbox;
	self->spin_checkbox_array[3] = self->spin4_checkbox;

    self->keywordTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "plum",
                                                  "foreground", "#A80D91", NULL);
    self->commentTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "clover",
                                                  "foreground", "#1F8005", NULL);
    self->numberTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "blueberry",
                                                 "foreground", "#1C00Cf", NULL);
    self->stringTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "cayenne",
                                                 "foreground", "#C41A17", NULL);
    self->variableTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "eggplant",
                                                   "foreground", "#5C2699", NULL);
    self->preprocessorTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "mocha",
                                                       "foreground", "#633821", NULL);
    self->filenameTag = gtk_text_buffer_create_tag(self->pulseProgram_textbuffer, "sky",
                                                   "foreground", "#A6C9ff", NULL);

    icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-ispin", 48, GTK_ICON_LOOKUP_FORCE_REGULAR);
    self->ispin_image = cairo_image_surface_create_from_png(gtk_icon_info_get_filename(icon_info));
    g_clear_object(&icon_info);
    icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-ispin-selected", 48, GTK_ICON_LOOKUP_FORCE_REGULAR);
    self->ispin_selected_image = cairo_image_surface_create_from_png(gtk_icon_info_get_filename(icon_info));
    g_clear_object(&icon_info);
    icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-sspin", 48, GTK_ICON_LOOKUP_FORCE_REGULAR);
    self->sspin_image = cairo_image_surface_create_from_png(gtk_icon_info_get_filename(icon_info));
    g_clear_object(&icon_info);
    icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-sspin-selected", 48, GTK_ICON_LOOKUP_FORCE_REGULAR);
    self->sspin_selected_image = cairo_image_surface_create_from_png(gtk_icon_info_get_filename(icon_info));
    g_clear_object(&icon_info);
    self->spinEditor_drawLine = FALSE;
    gtk_widget_set_events((GtkWidget *)self->spinEditor_drawingarea,
                          gtk_widget_get_events((GtkWidget *)self->spinEditor_drawingarea) | GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_set_events((GtkWidget *)self->pulseSequence_drawingarea,
                          gtk_widget_get_events((GtkWidget *)self->pulseSequence_drawingarea) | GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_set_events((GtkWidget *)self->spectrum_drawingarea,
                          gtk_widget_get_events((GtkWidget *)self->spectrum_drawingarea) | GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events((GtkWidget *)self->spectrum_drawingarea, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

    for (f = 0; f <= 360; f += 90) {
        gtk_scale_add_mark(self->flipAngle_scale, f, GTK_POS_BOTTOM, NULL);
        gtk_scale_add_mark(self->phase_scale, f, GTK_POS_BOTTOM, NULL);
        gtk_scale_add_mark(self->zeroOrder_slider, f, GTK_POS_BOTTOM, NULL);
    }
    for (f = -127; f <= 127; f += 18)
        gtk_scale_add_mark(self->pulseFrequency_scale, f, GTK_POS_BOTTOM, NULL);
    for (f = -1.5; f <= 1.5; f += 0.5)
        gtk_scale_add_mark(self->firstOrder_slider, f, GTK_POS_BOTTOM, NULL);
    for (f = -0.5; f <= 0.5; f += 0.25)
        gtk_scale_add_mark(self->pivotPoint_slider, f, GTK_POS_BOTTOM, NULL);

	// Connect controllers
    self->controller = insensitive_controller_new();
	spinsystem = insensitive_spinsystem_new();
	settings = insensitive_settings_new();
	insensitive_controller_setup(self->controller, spinsystem, settings, self);
    spin_number_was_changed(self);
    spin_state_was_changed(self);
    reset_phaseCyclingTable(self);
    insensitive_settings_load_pulsesequence(self->controller->settings, self);

	// Create Matrix Composer and connect it to the controller
    self->matrix_composer_window = g_object_new(INSENSITIVE_TYPE_COMPOSER,
		                                     "default-width", 390,
		                                     "default-height", 390,
		                                     NULL);
    gtk_window_set_title((GtkWindow *)self->matrix_composer_window, "Matrix Composer");
    g_signal_connect(self->matrix_composer_window, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    insensitive_composer_set_controller(self->matrix_composer_window, self->controller);

    // Create Pulse Shaper and connect it to the controller
    self->pulse_shaper_window = g_object_new(INSENSITIVE_TYPE_PULSESHAPER,
		                                     "default-width", 540,
		                                     "default-height", 337,
		                                     NULL);
    gtk_window_set_title((GtkWindow *)self->pulse_shaper_window, "Pulse Shape");
    g_signal_connect(self->pulse_shaper_window, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    insensitive_controller_connect_pulseShaperController(self->controller, self->pulse_shaper_window);

	// Load and autosave main window position

	// Make command line first responder and create command history
	gtk_widget_grab_focus(GTK_WIDGET(self->command_line));
    self->commandHistory = g_ptr_array_new();
	self->commandHistoryPosition = 0;

	// Set Preferences settings
	/*if([settings playSoundAfterAcquisition]) {
	   [playSoundCheckBox setState:NSOnState];
	   [controller setPlaySound:TRUE];
	   }
	   if([settings ignoreOffResonanceEffectsForPulses]) {
	   [ignoreOffResonanceEffectsForPulsesCheckBox setState:NSOnState];
	   [pulseShaperController setIgnoreOffResonanceEffectsForPulses:TRUE];
	   }
	   if([settings allowShiftAndCouplingButtons]) {
	   [allowSeparateShiftAndCouplingCheckBox setState:NSOnState];
	   }
	   [exportRadioButtons selectCellWithTag:(NSInteger)[settings exportFormat]];
	   [enforceEnglishButton setState:[[NSUserDefaults standardUserDefaults] boolForKey:@"EnforceEnglish"] ? NSOnState : NSOffState];
	   if(![[NSUserDefaults standardUserDefaults] objectForKey:@"MaxCoherenceCalculations"])
	   [[NSUserDefaults standardUserDefaults] setFloat:6.0 forKey:@"MaxCoherenceCalculations"];
	   [maxCalculationsSlider setIntegerValue:[[NSUserDefaults standardUserDefaults] integerForKey:@"MaxCoherenceCalculations"]];
	   [exponentLabel setStringValue:[NSString stringWithFormat:@"%ld", (long)[[NSUserDefaults standardUserDefaults] integerForKey:@"MaxCoherenceCalculations"]]];*/

	// Recover last session from user defaults file.
	/*  NSData *couplingData = [[NSUserDefaults standardUserDefaults] objectForKey:@"CouplingMatrix"];
	   if(couplingData) {
	      if([[NSUserDefaults standardUserDefaults] objectForKey:@"Spins"])
	          [spinSystem setSpins:(unsigned int)[[NSUserDefaults standardUserDefaults] integerForKey:@"Spins"]];
	      if([[NSUserDefaults standardUserDefaults] objectForKey:@"SpinTypeArray"])
	          [spinSystem setSpinTypeArray:(unsigned int)[[NSUserDefaults standardUserDefaults] integerForKey:@"SpinTypeArray"]];
	      [spinSystem substituteCouplingMatrixWith:(float *)[couplingData bytes]];
	      if([spinSystem firstGradientPulseIssued])
	          [spinSystem freeGradientArray];
	      [controller spinNumberChanged:[spinSystem spins]];
	      [controller calculateSelectableDelays];
	      [controller returnToThermalEquilibrium];
	   }
	   NSDictionary *pulseProgramData = [[NSUserDefaults standardUserDefaults] objectForKey:@"CurrentPulseProgram"];
	   if(pulseProgramData) {
	      NSString *ppName = [[NSUserDefaults standardUserDefaults] objectForKey:@"CurrentPulseSequenceName"];
	      if(ppName)
	          [controller setNameForPulseSequence:ppName];
	      [pulseSequenceController performOpenPulseProgram:pulseProgramData];
	   }
	   [settings release];
	   [spinSystem release];*/

    self->coefficientsForISpins = NULL;
    self->pathwaysForISpins = NULL;
    self->coefficientsForSSpins = NULL;
    self->pathwaysForSSpins = NULL;
    self->labelArray = NULL;
    self->pathArray = NULL;
    self->alphaArray = NULL;
    self->showOnlyNCoherences = TRUE;
    self->errorCode = 0;
    self->pathway_scaling = 1.0;
    self->needsToRecalculateCoherencePathways = TRUE;

    self->abscissaCentered = TRUE;
    self->ordinateCentered = FALSE;
    self->autoMaximize = TRUE;
    self->allowResizing = TRUE;
    self->drawAbscissa = TRUE;
    self->drawOrdinate = FALSE;
    self->drawScale = TRUE;
    self->drawCursor = FALSE;
    self->drawRealPart = TRUE;
    self->drawImaginaryPart = FALSE;
    self->drawIntegral = FALSE;
    self->drawPivotPoint = FALSE;
    self->drawWindow = FALSE;
    self->windowFunctionType = WFNone;
    self->plotMode = Raster;
    self->scaling = 100;
    self->cursorX = 0.0;
    self->cursorY = 0.0;
    self->lineWidth = 1.0;
    self->gaussianWidth = 10.0;
    self->gaussianShift = 0.0;
    self->integralMaximum = 1.0;
    self->lastDataPointDisplayed = 0;
    self->maxDataPoints = 0;
    self->maxOrdinateValue = 1.0;
    self->magnification = 1.0;
    self->showsFrequencyDomain = FALSE;
    self->shows2DFrequencyDomain = FALSE;
    self->twoDimensionalSpectrum = FALSE;
    self->indirectDataPoints = 128;
    self->noCursor = FALSE;
    self->parametersVisible = TRUE;
    self->statesDataSet = 1;
    self->spectrumIsDOSY2D = FALSE;
    self->numberOfPeaks = 0;
    self->shiftedBaseline = FALSE;

    self->data.realp = NULL;
    self->data.imagp = NULL;
    self->integral = NULL;
    self->phase.realp = NULL;
    self->phase.imagp = NULL;
    self->apodizationT2 = NULL;
    self->apodizationT1 = NULL;
    self->noise.realp = NULL;
    self->noise.imagp = NULL;
    self->displayedData.realp = NULL;
    self->displayedData.imagp = NULL;
    self->peaks = NULL;

    gtk_toggle_button_set_active(self->grid_checkbox, insensitive_settings_get_showGrid(self->controller->settings));
    self->lineWidth = insensitive_settings_get_spectrumLineWidth(self->controller->settings);
    str = malloc(6 * sizeof(gchar));
    sprintf(str, "%.1f", self->lineWidth);
    gtk_entry_set_text(self->lineWidth_entry, str);
    sprintf(str, "%.2f", insensitive_settings_get_signalToNoiseThreshold(self->controller->settings));
    gtk_entry_set_text(self->signalToNoiseThreshold_entry, str);
    free(str);
    set_dataPoints_label(self, 0, insensitive_settings_get_dataPoints(self->controller->settings));
    g_idle_add((GSourceFunc)update_pulseSequence, self);

    gtk_widget_hide(GTK_WIDGET(self->step_window));
    gtk_widget_hide(GTK_WIDGET(self->dosyToolBox_window));
    set_2D_mode(self, FALSE);
    show_spectrumParameters_textview(self, FALSE);
    update_spectrum_parameter_panel(self);

	gtk_notebook_set_show_tabs(self->mainwindow_notebook, FALSE);
	show_mainWindow_notebook_page(self, 1);
}


G_MODULE_EXPORT void on_open_file_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    GtkWidget *chooser;
    GtkFileFilter *filter;

    chooser = gtk_file_chooser_dialog_new("Open Insensitive File...",
                                          (GtkWindow *)window,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "Cancel", GTK_RESPONSE_CANCEL,
                                          "Open", GTK_RESPONSE_ACCEPT,
                                          NULL);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive files (ISS, IPP, IGG)");
    gtk_file_filter_add_pattern(filter, "*.iss");
    gtk_file_filter_add_pattern(filter, "*.ipp");
    gtk_file_filter_add_pattern(filter, "*.igg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive spin systems (ISS)");
    gtk_file_filter_add_pattern(filter, "*.iss");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive pulse programs (IPP)");
    gtk_file_filter_add_pattern(filter, "*.ipp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive spectra (IGG)");
    gtk_file_filter_add_pattern(filter, "*.igg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    choose_file(chooser, window);
}


void choose_file(GtkWidget *chooser, InsensitiveWindow *window)
{
	gint result;
	gchar *filename;

	gtk_widget_show_all(chooser);
    result = gtk_dialog_run((GtkDialog *)chooser);
    if (result == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
		open_file(window, filename);
		g_free(filename);
	}
	g_object_unref(chooser);
}


void open_file(InsensitiveWindow *window, gchar *filename)
{
    xmlDoc *doc = NULL;
    xmlNode *root, *first_child, *node;
    gboolean fileTypeRecognised = FALSE;

    LIBXML_TEST_VERSION
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
        show_open_file_error(window, filename);
    else {
        root = xmlDocGetRootElement(doc);
        if (root->type == XML_ELEMENT_NODE && !strcmp((char *)root->name, "plist")) {
            first_child = root->children;
            if (first_child->type == XML_TEXT_NODE && first_child->next->type == XML_ELEMENT_NODE) {
                if (!strcmp((char *)first_child->next->name, "dict")) {
                    first_child = first_child->next->children;
                    for (node = first_child; node; node = node->next) {
                        if(node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "key")) {
                            if (!strcmp((char *)xmlNodeGetContent(node), "ISSFileVersion")) {
                                if (perform_open_spinSystem(window, first_child)) {
                                    set_openedFileState_for_spinSystem(window, FileOpened, g_path_get_basename(filename));
                                    show_mainWindow_notebook_page(window, 0);
                                    fileTypeRecognised = TRUE;
                                } else
                                    show_open_file_error(window, filename);
                                break;
                            } else if(!strcmp((char *)xmlNodeGetContent(node), "IPPFileVersion")) {
                                if (perform_open_pulseProgram(window, first_child)) {
                                    set_openedFileState_for_pulseSequence(window, FileOpened, g_path_get_basename(filename));
                                    show_mainWindow_notebook_page(window, 2);
                                    fileTypeRecognised = TRUE;
                                } else
                                   show_open_file_error(window, filename);
                               break;
                            } else if(!strcmp((char *)xmlNodeGetContent(node), "IGGFileVersion")) {
                               if (perform_open_spectrum(window, first_child)) {
                                   set_openedFileState_for_spectrum(window, FileOpened, g_path_get_basename(filename));
                                   show_mainWindow_notebook_page(window, 3);
                                   fileTypeRecognised = TRUE;
                               } else
                                   show_open_file_error(window, filename);
                               break;
                            }
                        }
                    }
                    if (!fileTypeRecognised)
                        show_open_file_error(window, filename);
                } else
                    show_open_file_error(window, filename);
            } else
                show_open_file_error(window, filename);
        } else
            show_open_file_error(window, filename);
        xmlFreeDoc(doc);
        xmlCleanupParser();
    }
}


void show_open_file_error(InsensitiveWindow *window, gchar *filename)
{
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                   GTK_DIALOG_DESTROY_WITH_PARENT,
							                   GTK_MESSAGE_ERROR,
							                   GTK_BUTTONS_OK,
							                   "The file \"%s\" was not recognised as a valid Insensitive spin system (iss), pulse program (ipp) or spectrum (igg) file.", filename);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error opening file");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


G_MODULE_EXPORT void on_preferences_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    InsensitivePreferences *preferences;

    preferences = g_object_new(INSENSITIVE_TYPE_PREFERENCES,
		                       NULL);
    gtk_window_set_title((GtkWindow *)preferences, "Preferences");
    insensitive_preferences_set_controller(preferences, window);
	gtk_window_present((GtkWindow *)preferences);
}


G_MODULE_EXPORT void quit_insensitive(InsensitiveWindow *window)
{
    insensitive_settings_save_spinsystem(window->controller->settings, window->controller->spinSystem);
    insensitive_settings_save_pulsesequence(window->controller->settings, window->controller);
    insensitive_settings_save_defaults(window->controller->settings);
    gtk_widget_destroy((GtkWidget *)window->pulse_shaper_window);
    gtk_widget_destroy((GtkWidget *)window->matrix_composer_window);
    cairo_surface_destroy(window->pulseSequence_surface);
    cairo_surface_destroy(window->spectrum_surface);
}


 //////  //    // //////// //////  //    // ////////      /////   ////// //////// //  //////  ///    // ///////
//    // //    //    //    //   // //    //    //        //   // //         //    // //    // ////   // //
//    // //    //    //    //////  //    //    //        /////// //         //    // //    // // //  // ///////
//    // //    //    //    //      //    //    //        //   // //         //    // //    // //  // //      //
 //////   //////     //    //       //////     //        //   //  //////    //    //  //////  //   //// ///////

void show_mainWindow_notebook_page(InsensitiveWindow *self, unsigned int page)
{
	InsensitiveController *controller = self->controller;

    if (page < 4) {
        gtk_notebook_set_current_page(self->mainwindow_notebook, page);

		if (page == 1 && (insensitive_controller_get_isRecordingPulseSequence(controller)
       					 || insensitive_controller_get_currentStepInPulseSequence(controller) > 0)
   		    && !insensitive_controller_get_acquisitionIsInProgress(controller)) {
    		gtk_widget_show(GTK_WIDGET(self->step_window));
			gtk_widget_set_visible(GTK_WIDGET(self->stepWindow_button), !insensitive_controller_get_isRecordingPulseSequence(controller));
		} else if (gtk_widget_is_visible(GTK_WIDGET(self->step_window))) {
    		gtk_widget_hide(GTK_WIDGET(self->step_window));
		}
	}
}


void init_settings(InsensitiveWindow *self, InsensitiveSettings *settings)
{
    unsigned int i;
    gchar *str = malloc(6 * sizeof(gchar));

    g_signal_handlers_block_by_func(G_OBJECT(self->flipAngle_entry), G_CALLBACK(on_flipAngle_entry_activate), (gpointer)self);
    sprintf(str, "%.1f", insensitive_settings_get_flipAngle(settings));
    gtk_entry_set_text(self->flipAngle_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->flipAngle_entry), G_CALLBACK(on_flipAngle_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->flipAngle_adjustment), G_CALLBACK(on_flipAngle_adjustment_changed), (gpointer)self);
    gtk_adjustment_set_value(self->flipAngle_adjustment, insensitive_settings_get_flipAngle(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->flipAngle_adjustment), G_CALLBACK(on_flipAngle_adjustment_changed), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->pulseDuration_entry), G_CALLBACK(on_pulseDuration_entry_activate), (gpointer)self);
    sprintf(str, "%.3f", insensitive_settings_get_pulseDuration(settings));
    gtk_entry_set_text(self->pulseDuration_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->pulseDuration_entry), G_CALLBACK(on_pulseDuration_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->pulseStrength_entry), G_CALLBACK(on_pulseStrength_entry_activate), (gpointer)self);
    sprintf(str, "%.1f", insensitive_settings_get_pulseStrength(settings));
    gtk_entry_set_text(self->pulseStrength_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->pulseStrength_entry), G_CALLBACK(on_pulseStrength_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->pulseFrequency_entry), G_CALLBACK(on_pulseFrequency_entry_activate), (gpointer)self);
    sprintf(str, "%.0f", insensitive_settings_get_pulseFrequency(settings));
    gtk_entry_set_text(self->pulseFrequency_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->pulseFrequency_entry), G_CALLBACK(on_pulseFrequency_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->pulseFrequency_adjustment), G_CALLBACK(on_pulseFrequency_adjustment_changed), (gpointer)self);
    gtk_adjustment_set_value(self->pulseFrequency_adjustment, insensitive_settings_get_pulseFrequency(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->pulseFrequency_adjustment), G_CALLBACK(on_pulseFrequency_adjustment_changed), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->phase_entry), G_CALLBACK(on_phase_entry_activate), (gpointer)self);
    sprintf(str, "%.1f", insensitive_settings_get_phase(settings));
    gtk_entry_set_text(self->phase_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->phase_entry), G_CALLBACK(on_phase_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->phase_adjustment), G_CALLBACK(on_phase_adjustment_changed), (gpointer)self);
    gtk_adjustment_set_value(self->phase_adjustment, insensitive_settings_get_phase(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->phase_adjustment), G_CALLBACK(on_phase_adjustment_changed), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->spin1_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_block_by_func(G_OBJECT(self->spin2_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_block_by_func(G_OBJECT(self->spin3_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_block_by_func(G_OBJECT(self->spin4_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_block_by_func(G_OBJECT(self->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)self);
	g_signal_handlers_block_by_func(G_OBJECT(self->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)self);
	g_signal_handlers_block_by_func(G_OBJECT(self->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)self);
	for (i = 0; i < maxNumberOfSpins; i++)
        gtk_toggle_button_set_active(self->spin_checkbox_array[i],
                                     insensitive_settings_get_pulseArray(settings) & pow2(i));
    gtk_toggle_button_set_active(self->allspins_checkbox, insensitive_settings_get_allSpinsSelected(settings));
    gtk_toggle_button_set_active(self->ispins_checkbox, insensitive_settings_get_allISpinsSelected(settings));
    gtk_toggle_button_set_active(self->sspins_checkbox, insensitive_settings_get_allSSpinsSelected(settings));
	g_signal_handlers_unblock_by_func(G_OBJECT(self->spin1_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->spin2_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->spin3_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->spin4_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)self);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)self);
	g_signal_handlers_unblock_by_func(G_OBJECT(self->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)self);
	g_signal_handlers_unblock_by_func(G_OBJECT(self->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->strong_coupling_checkbox_spinstate), G_CALLBACK(on_strongCoupling_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->strong_coupling_checkbox_spinstate, insensitive_settings_get_strongCoupling(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->strong_coupling_checkbox_spinstate), G_CALLBACK(on_strongCoupling_checkbox_toggled), (gpointer)self);
    g_signal_handlers_block_by_func(G_OBJECT(self->strong_coupling_checkbox_spinsystem), G_CALLBACK(on_strongCoupling_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->strong_coupling_checkbox_spinsystem, insensitive_settings_get_strongCoupling(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->strong_coupling_checkbox_spinsystem), G_CALLBACK(on_strongCoupling_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->dipolar_relaxation_checkbox), G_CALLBACK(on_dipolar_relaxation_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->dipolar_relaxation_checkbox, insensitive_settings_get_dipolarRelaxation(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->dipolar_relaxation_checkbox), G_CALLBACK(on_dipolar_relaxation_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->animation_checkbox), G_CALLBACK(on_animation_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->animation_checkbox, insensitive_settings_get_animates(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->animation_checkbox), G_CALLBACK(on_animation_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->include_relaxation_checkbox), G_CALLBACK(on_include_relaxation_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->include_relaxation_checkbox, insensitive_settings_get_relaxationWithEvolution(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->include_relaxation_checkbox), G_CALLBACK(on_include_relaxation_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->T1_entry), G_CALLBACK(on_T1_entry_activate), (gpointer)self);
    sprintf(str, "%.0f", insensitive_settings_get_T1(settings));
    gtk_entry_set_text(self->T1_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->T1_entry), G_CALLBACK(on_T1_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->T2_entry), G_CALLBACK(on_T2_entry_activate), (gpointer)self);
    sprintf(str, "%.0f", insensitive_settings_get_T2(settings));
    gtk_entry_set_text(self->T2_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->T2_entry), G_CALLBACK(on_T2_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->correlationTime_entry), G_CALLBACK(on_correlationTime_entry_activate), (gpointer)self);
    sprintf(str, "%.2f", insensitive_settings_get_correlationTime(settings));
    gtk_entry_set_text(self->correlationTime_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->correlationTime_entry), G_CALLBACK(on_correlationTime_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->delay_entry), G_CALLBACK(on_delay_entry_activate), (gpointer)self);
    sprintf(str, "%.2f", insensitive_settings_get_delay(settings));
    gtk_entry_set_text(self->delay_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->delay_entry), G_CALLBACK(on_delay_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->iDecoupling_checkbox), G_CALLBACK(on_iDecoupling_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->iDecoupling_checkbox, insensitive_settings_get_iDecoupling(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->iDecoupling_checkbox), G_CALLBACK(on_iDecoupling_checkbox_toggled), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->sDecoupling_checkbox), G_CALLBACK(on_sDecoupling_checkbox_toggled), (gpointer)self);
    gtk_toggle_button_set_active(self->sDecoupling_checkbox, insensitive_settings_get_sDecoupling(settings));
    g_signal_handlers_unblock_by_func(G_OBJECT(self->sDecoupling_checkbox), G_CALLBACK(on_sDecoupling_checkbox_toggled), (gpointer)self);


    g_signal_handlers_block_by_func(G_OBJECT(self->dwelltime_entry), G_CALLBACK(on_dwelltime_entry_activate), (gpointer)self);
    sprintf(str, "%.3f", insensitive_settings_get_dwellTime(settings));
    gtk_entry_set_text(self->dwelltime_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->dwelltime_entry), G_CALLBACK(on_dwelltime_entry_activate), (gpointer)self);

    g_signal_handlers_block_by_func(G_OBJECT(self->noiseLevel_entry), G_CALLBACK(on_noise_level_entry_activate), (gpointer)self);
    sprintf(str, "%.1f", insensitive_settings_get_noiseLevel(settings));
    gtk_entry_set_text(self->noiseLevel_entry, str);
    g_signal_handlers_unblock_by_func(G_OBJECT(self->noiseLevel_entry), G_CALLBACK(on_noise_level_entry_activate), (gpointer)self);


/*
	   [gradientStrengthComboBox setStringValue:[NSString stringWithFormat:@"%.0f", [settings gradientStrength]]];
	   [gradientDurationTextBox setStringValue:[NSString stringWithFormat:@"%.2f", [settings gradientDuration]]];
	   [dataPointsStepper setIntValue:lb([settings dataPoints])];
	   [dataPointsTextBox setStringValue:[NSString stringWithFormat:@"%d", [settings dataPoints]]];
 *
	   [acquireAfterNextPulseCheckbox setState:[settings acquisitionAfterNextPulse] ? NSOnState : NSOffState];
	   [detectSignalRadioButtons selectCellWithTag:[settings detectSSpins] ? 0 : 1];
	   [detectISignalCheckbox setState:[settings detectISpins] ? NSOnState : NSOffState];
	   [detectSSignalCheckbox setState:[settings detectSSpins] ? NSOnState : NSOffState];
	   [color1stOrderCoherencesCheckbox setState:[settings color1stOrderCoherences] ? NSOnState : NSOffState];
 * */

    free(str);
}


void set_selectable_delay_times(InsensitiveWindow *self, gchar **names, unsigned int size)
{
	unsigned int i;
	gchar tag[3];

	g_signal_handlers_block_by_func(G_OBJECT(self->delay_combobox), G_CALLBACK(on_delay_combobox_changed), (gpointer)self);
	gtk_combo_box_text_remove_all(self->delay_combobox);
	for (i = 0; i < size; i++) {
		if (names[i] != NULL) {
			sprintf(tag, "%d", i);
			gtk_combo_box_text_append(self->delay_combobox, tag, names[i]);
			//if(((i + 1) % 4 == 0) && (i + 1 != size))
			//    gtk_combo_box_text_append_text(self->delay_combobox, "--------");
		}
	}
	g_signal_handlers_unblock_by_func(G_OBJECT(self->delay_combobox), G_CALLBACK(on_delay_combobox_changed), (gpointer)self);
}


void set_user_controls_enabled(InsensitiveWindow *self, gboolean value)
{
	if (value == FALSE) {
		gtk_widget_set_sensitive((GtkWidget *)self->pulse_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90x_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90y_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90minusx_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90minusy_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180x_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180y_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180minusx_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180minusy_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->expandPulse_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->contractPulse_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->chemicalShift_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->coupling_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->relaxation_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->freeEvolution_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->gradient_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->equilibrium_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->undo_button, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)self->step_button, FALSE);
	} else {
		gtk_widget_set_sensitive((GtkWidget *)self->pulse_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90x_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90y_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90minusx_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse90minusy_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180x_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180y_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180minusx_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->pulse180minusy_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->expandPulse_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->contractPulse_button, TRUE);
		if (insensitive_controller_allow_separate_shift_and_coupling(self->controller) && !self->controller->isRecordingPulseSequence) {
			gtk_widget_set_sensitive((GtkWidget *)self->chemicalShift_button, TRUE);
			if (self->controller->spinSystem->spins > 1)
				gtk_widget_set_sensitive((GtkWidget *)self->coupling_button, TRUE);
		} else {
			gtk_widget_set_sensitive((GtkWidget *)self->chemicalShift_button, FALSE);
			gtk_widget_set_sensitive((GtkWidget *)self->coupling_button, FALSE);
		}
		gtk_widget_set_sensitive((GtkWidget *)self->relaxation_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->freeEvolution_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->gradient_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->equilibrium_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->undo_button, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)self->step_button, TRUE);
	}
}


void enable_animation_checkbox(InsensitiveWindow *self, gboolean value)
{
	gtk_widget_set_sensitive((GtkWidget *)self->animation_checkbox, value);
}


void set_acquisition_is_running(InsensitiveWindow *self, gboolean value)
{
	if (value)
		gtk_button_set_label(self->acquire_button, "Stop");
	else
		gtk_button_set_label(self->acquire_button, "Acquire");
}


void enable_acquisition_button(InsensitiveWindow *self, gboolean value)
{
	gtk_widget_set_sensitive((GtkWidget *)self->acquire_button, value);
}


void start_progress_indicator(InsensitiveWindow *self)
{
	gtk_spinner_start(self->acquisition_spinner);
}


void stop_progress_indicator(InsensitiveWindow *self)
{
	gtk_spinner_stop(self->acquisition_spinner);
}


gboolean spin_state_was_changed(InsensitiveWindow *window)
{
	InsensitiveController *controller = window->controller;
	gchar *postring;
	VectorCoordinates *vectorCoordinates;

	// Vector representation
	vectorCoordinates = insensitive_controller_get_vectorCoordinates(controller);
	if (insensitive_spinsystem_get_number_of_sspins(controller->spinSystem) == 0) {
		gtk_widget_set_visible((GtkWidget *)window->iSpinVector_drawingarea, TRUE);
		gtk_widget_set_visible((GtkWidget *)window->sSpinVector_drawingarea, FALSE);
		gtk_widget_queue_draw((GtkWidget *)window->iSpinVector_drawingarea);
	} else if (insensitive_spinsystem_get_number_of_ispins(controller->spinSystem) == 0) {
		gtk_widget_set_visible((GtkWidget *)window->iSpinVector_drawingarea, FALSE);
		gtk_widget_set_visible((GtkWidget *)window->sSpinVector_drawingarea, TRUE);
		gtk_widget_queue_draw((GtkWidget *)window->sSpinVector_drawingarea);
	} else {
		gtk_widget_set_visible((GtkWidget *)window->iSpinVector_drawingarea, TRUE);
		gtk_widget_set_visible((GtkWidget *)window->sSpinVector_drawingarea, TRUE);
		gtk_widget_queue_draw((GtkWidget *)window->iSpinVector_drawingarea);
        gtk_widget_queue_draw((GtkWidget *)window->sSpinVector_drawingarea);
	}
	free_vector_coordinates(vectorCoordinates);

	// Density matrix
    gtk_widget_queue_draw((GtkWidget *)window->matrix_drawingarea);

	// Print product operator string
	postring = insensitive_controller_get_productOperatorString(controller);
    gtk_text_buffer_set_text(window->productoperator_textbuffer, postring, strlen(postring));
    g_free(postring);

	return FALSE;
}


void spin_number_was_changed(InsensitiveWindow *window)
{
	unsigned int i;
	InsensitiveController *controller = window->controller;

	// Resize matrix
	set_matrixDisplayType(window, insensitive_settings_get_matrixDisplayType(controller->settings));
    insensitive_composer_reset(window->matrix_composer_window);

	// Disable unused checkboxes
	for (i = 0; i < maxNumberOfSpins; i++)
		if (i < insensitive_spinsystem_get_spins(controller->spinSystem)) {
			gtk_toggle_button_set_active(window->spin_checkbox_array[i], TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(window->spin_checkbox_array[i]), TRUE);
        } else {
			gtk_toggle_button_set_active(window->spin_checkbox_array[i], FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(window->spin_checkbox_array[i]), FALSE);
        }

    gtk_spin_button_set_value(window->spinNumber_spinbutton, insensitive_spinsystem_get_spins(window->controller->spinSystem));
    insensitive_controller_return_to_thermal_equilibrium(window->controller);
    insensitive_controller_calculate_energy_levels(window->controller);
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


// ///    // //////  //    // ////////      /////   ////// //////// //  //////  ///    // ///////
// ////   // //   // //    //    //        //   // //         //    // //    // ////   // //
// // //  // //////  //    //    //        /////// //         //    // //    // // //  // ///////
// //  // // //      //    //    //        //   // //         //    // //    // //  // //      //
// //   //// //       //////     //        //   //  //////    //    //  //////  //   //// ///////

G_MODULE_EXPORT void on_equilibrium_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	if (insensitive_controller_get_currentStepInPulseSequence(window->controller)) {
        insensitive_controller_set_currentStepInPulseSequence(window->controller, 0);
        gtk_widget_hide(GTK_WIDGET(window->step_window));
    }
	insensitive_controller_return_to_thermal_equilibrium(window->controller);
}


G_MODULE_EXPORT void on_pulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	/*if ([pulseButton selectedSegment] == 1) {
	        [quickPulseButtons setHidden:FALSE];
	   gtk_widget_set_sensitive(self->pulse_button, TRUE);
	        gtk_widget_set_sensitive(self->chemicalShift_button, TRUE);
	        gtk_widget_set_sensitive(self->coupling_button, TRUE);
	        gtk_widget_set_sensitive(self->relaxation_button, TRUE);
	        gtk_widget_set_sensitive(self->freeEvolution_button, TRUE);
	        gtk_widget_set_sensitive(self->gradient_button, TRUE);
	   } else {*/
	start_progress_indicator(window);
	insensitive_controller_perform_pulse_animated(window->controller, TRUE);
	/*}*/
}


G_MODULE_EXPORT void on_pulse90x_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 90, 0, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse90y_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 90, 90, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse90minusx_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 90, 180, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse90minusy_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 90, 270, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse180x_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 180, 0, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse180y_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 180, 90, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse180minusx_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 180, 180, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_pulse180minusy_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_pulse(window->controller, 180, 270, TRUE);
	on_contractPulse_button_clicked(window->contractPulse_button, window);
}


G_MODULE_EXPORT void on_expandPulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	gtk_widget_set_visible(GTK_WIDGET(window->pulse_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->chemicalShift_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->coupling_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->relaxation_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->freeEvolution_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->expandPulse_button), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(window->pulse90x_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90y_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90minusx_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90minusy_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180x_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180y_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180minusx_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180minusy_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->contractPulse_button), TRUE);
}


G_MODULE_EXPORT void on_contractPulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	gtk_widget_set_visible(GTK_WIDGET(window->pulse_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->chemicalShift_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->coupling_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->relaxation_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->freeEvolution_button), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(window->expandPulse_button), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(window->pulse90x_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90y_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90minusx_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse90minusy_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180x_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180y_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180minusx_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->pulse180minusy_button), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(window->contractPulse_button), FALSE);
}


G_MODULE_EXPORT void on_chemicalShift_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	if (insensitive_controller_no_larmorFrequency_set(window->controller)) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_OK,
						"All Larmor frequencies are set to zero. This operation has no effect on the spin system.");
		gtk_window_set_title(GTK_WINDOW(dialog), "No Chemical Shift");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	} else {
		start_progress_indicator(window);
		insensitive_controller_perform_chemicalShift_animated(window->controller, TRUE);
	}
}


G_MODULE_EXPORT void on_coupling_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	if (insensitive_controller_no_coupling_set(window->controller)) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_OK,
						"All scalar coupling constants are set to zero. This operation has no effect on the spin system.");
		gtk_window_set_title(GTK_WINDOW(dialog), "No Scalar Coupling");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	} else {
		start_progress_indicator(window);
		insensitive_controller_perform_coupling_animated(window->controller, TRUE);
	}
}


G_MODULE_EXPORT void on_relaxation_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_relaxation_animated(window->controller, TRUE);
}


G_MODULE_EXPORT void on_freeEvolution_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	insensitive_controller_perform_freeEvolution_animated(window->controller, TRUE);
}


G_MODULE_EXPORT void on_gradient_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	start_progress_indicator(window);
	set_user_controls_enabled(window, FALSE);
	insensitive_controller_perform_gradient(window->controller);
}


G_MODULE_EXPORT void on_acquire_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_controller_get_acquisitionIsInProgress(window->controller)) {
        // For 2D gradient spectra the threaded acquisition needs to halt itself by setting the interrupt flag
        if(shows_2D_spectrum(window))
            insensitive_controller_interrupt_acquisition(window->controller);
        else
            insensitive_controller_stop_acquisition(window->controller);
        enable_pulseSequence_play_button(window, TRUE);
    } else {
        start_progress_indicator(window);
        insensitive_controller_perform_acquisition(window->controller);
        enable_pulseSequence_play_button(window, FALSE);
		if (insensitive_controller_get_currentStepInPulseSequence(window->controller)) {
			insensitive_controller_set_currentStepInPulseSequence(window->controller, 0);
			gtk_widget_queue_draw((GtkWidget *)window->pulseSequence_drawingarea);
		}
        show_mainWindow_notebook_page(window, 3);
    }
}


G_MODULE_EXPORT void on_undo_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_restore_previous_state(window->controller);
}


G_MODULE_EXPORT void on_notebook_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    unsigned int page;

	if (!strcmp(gtk_tool_button_get_label(toolbutton), "Spin System\0"))
        page = 0;
	else if (!strcmp(gtk_tool_button_get_label(toolbutton), "Spin State\0"))
		page = 1;
	else if (!strcmp(gtk_tool_button_get_label(toolbutton), "Pulse Sequence\0"))
		page = 2;
	else if (!strcmp(gtk_tool_button_get_label(toolbutton), "Spectrum\0"))
		page = 3;
	show_mainWindow_notebook_page(window, page);
}


G_MODULE_EXPORT void on_matrix_composer_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	gtk_window_present((GtkWindow *)window->matrix_composer_window);
}


G_MODULE_EXPORT void on_pulse_shaper_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	gtk_window_present((GtkWindow *)window->pulse_shaper_window);
}


G_MODULE_EXPORT void on_single_spins_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveSingleSpins *single_spins_window;

    single_spins_window = g_object_new(INSENSITIVE_TYPE_SINGLESPINS,
		                           "default-width", 480,
		                           "default-height", 414,
		                           NULL);
    gtk_window_set_title((GtkWindow *)single_spins_window, "Individual spins on resonance with Ω⁰ = 0 Hz");
	gtk_window_present((GtkWindow *)single_spins_window);
}


G_MODULE_EXPORT void on_tutorial_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
#ifdef USE_WEBKIT_GTK
    InsensitiveTutorial *tutorial_window;

    tutorial_window = g_object_new(INSENSITIVE_TYPE_TUTORIAL,
		                           "default-width", 600,
		                           "default-height", 700,
		                           NULL);
    gtk_window_set_title((GtkWindow *)tutorial_window, "Tutorial");
	gtk_window_present((GtkWindow *)tutorial_window);
#else
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *filename, *url;
    const gchar * const *dirs = g_get_system_data_dirs();
    GError *error = NULL;

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", "default.html", NULL);
        printf("%s\n", filename);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
            url = malloc((strlen(filename) + 7) * sizeof(gchar));
            strcpy(url, "file://");
            strcat(url, filename);
			gtk_show_uri_on_window(GTK_WINDOW(window), url, GDK_CURRENT_TIME, &error);
            if (error != NULL) {
                fprintf (stderr, "Error: %s\n", error->message);
                g_error_free (error);
            }
            if (url != NULL)
                g_free(url);
		    if (filename != NULL)
                g_free(filename);
            break;
        }
	}
#endif /* USE_WEBKIT_GTK */
}


G_MODULE_EXPORT void on_about_menu_item_activate(GtkMenuItem *item, gpointer *user_data)
{
	GtkWidget *dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Insensitive");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), insensitive_version);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2011-2023 Klaus Boldt");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "NMR Simulation Tool");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://github.com/klausboldt/insensitive");
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "com.klausboldt.insensitive");
	gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_MIT_X11);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/////// /////// //////// //////// // ///    //  //////  ///////
//      //         //       //    // ////   // //       //
/////// /////      //       //    // // //  // //   /// ///////
     // //         //       //    // //  // // //    //      //
/////// ///////    //       //    // //   ////  //////  ///////

void set_pulseEnvelope(InsensitiveWindow *self, enum PulseEnvelope value)
{
	gtk_combo_box_set_active((GtkComboBox *)self->pulseEnvelope_combobox, (gint)value);
}


G_MODULE_EXPORT void on_pulseEnvelope_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_set_pulseEnvelope(window->controller,
						 (enum PulseEnvelope)gtk_combo_box_get_active((GtkComboBox *)combobox));
}


void set_flipAngle(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.1f", value);
    //g_signal_handlers_block_by_func(G_OBJECT(window->flipAngle_entry), G_CALLBACK(on_flipAngle_entry_activate), (gpointer)window);
	//g_signal_handlers_block_by_func(G_OBJECT(window->flipAngle_adjustment), G_CALLBACK(on_flipAngle_adjustment_changed), (gpointer)window);
	//g_signal_handlers_block_by_func(G_OBJECT(window->pulseDuration_entry), G_CALLBACK(on_pulseDuration_entry_activate), (gpointer)window);
	//g_signal_handlers_block_by_func(G_OBJECT(window->pulseStrength_entry), G_CALLBACK(on_pulseStrength_entry_activate), (gpointer)window);
	gtk_adjustment_set_value(window->flipAngle_adjustment, value);
	gtk_entry_set_text(window->flipAngle_entry, string);
    //g_signal_handlers_unblock_by_func(G_OBJECT(window->flipAngle_entry), G_CALLBACK(on_flipAngle_entry_activate), (gpointer)window);
	//g_signal_handlers_unblock_by_func(G_OBJECT(window->flipAngle_adjustment), G_CALLBACK(on_flipAngle_adjustment_changed), (gpointer)window);
	//g_signal_handlers_unblock_by_func(G_OBJECT(window->pulseDuration_entry), G_CALLBACK(on_pulseDuration_entry_activate), (gpointer)window);
	//g_signal_handlers_unblock_by_func(G_OBJECT(window->pulseStrength_entry), G_CALLBACK(on_pulseStrength_entry_activate), (gpointer)window);
	g_free(string);

    insensitive_pulse_shaper_set_pulseLengthScale_for_flipAngle(window->controller->pulseShaperController,
                                                                insensitive_settings_get_flipAngle(window->controller->settings));
}


G_MODULE_EXPORT void on_flipAngle_adjustment_changed(GtkAdjustment *slider, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = gtk_adjustment_get_value(slider);

	insensitive_controller_set_flipAngle(window->controller, value);
}


G_MODULE_EXPORT void on_flipAngle_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_flipAngle(window->controller, value);
}


void set_pulseDuration(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.3f", value);
	gtk_entry_set_text(window->pulseDuration_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_pulseDuration_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_pulseDuration(window->controller, value);
}


void set_pulseStrength(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.3f", value);
	gtk_entry_set_text(window->pulseStrength_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_pulseStrength_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_pulseStrength(window->controller, value);
}


G_MODULE_EXPORT void on_hardpulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_make_hard_pulse(window->controller);
}


G_MODULE_EXPORT void on_softpulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_make_soft_pulse(window->controller);
}


G_MODULE_EXPORT void on_softerpulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_make_softer_pulse(window->controller);
}


G_MODULE_EXPORT void on_selectivepulse_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_make_selective_pulse(window->controller);
}


void set_pulseFrequency(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_adjustment_set_value(window->pulseFrequency_adjustment, value);
	gtk_entry_set_text(window->pulseFrequency_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_pulseFrequency_adjustment_changed(GtkAdjustment *slider, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = gtk_adjustment_get_value(slider);

	insensitive_controller_set_pulseFrequency(window->controller, value);
}


G_MODULE_EXPORT void on_pulseFrequency_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_pulseFrequency(window->controller, value);
}


void set_phase(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.1f", value);
	gtk_adjustment_set_value(window->phase_adjustment, value);
	gtk_entry_set_text(window->phase_entry, string);
	g_free(string);

    insensitive_pulse_shaper_set_pulseLengthScale_for_flipAngle(window->controller->pulseShaperController,
                                                                insensitive_settings_get_flipAngle(window->controller->settings));
}


G_MODULE_EXPORT void on_phase_adjustment_changed(GtkAdjustment *slider, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = gtk_adjustment_get_value(slider);

	insensitive_controller_set_phase(window->controller, value);
}


G_MODULE_EXPORT void on_phase_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_phase(window->controller, value);
}


void set_spin_checkboxes(InsensitiveWindow *window, int pulseArray)
{
	unsigned int i;

	for (i = 0; i < maxNumberOfSpins; i++)
		gtk_toggle_button_set_active(window->spin_checkbox_array[i], pulseArray & pow2(i));
}


G_MODULE_EXPORT void on_spin_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	const gchar *label = gtk_button_get_label((GtkButton *)checkbox);
	int spin_number = atoi(label) - 1;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	g_signal_handlers_block_by_func(G_OBJECT(window->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_block_by_func(G_OBJECT(window->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_block_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
	insensitive_controller_change_pulseArray_for_spin(window->controller, spin_number, value);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
}


void set_iSpins_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->ispins_checkbox, value);
}


G_MODULE_EXPORT void on_iSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	g_signal_handlers_block_by_func(G_OBJECT(window->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_block_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
	insensitive_controller_set_all_iSpins_active(window->controller, value);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->sspins_checkbox), G_CALLBACK(on_sSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
}


void set_sSpins_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->sspins_checkbox, value);
}


G_MODULE_EXPORT void on_sSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	g_signal_handlers_block_by_func(G_OBJECT(window->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_block_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
	insensitive_controller_set_all_sSpins_active(window->controller, value);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->ispins_checkbox), G_CALLBACK(on_iSpins_checkbox_toggled), (gpointer)user_data);
	g_signal_handlers_unblock_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
}


void set_allSpins_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->allspins_checkbox, value);
}


G_MODULE_EXPORT void on_allSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_all_spins_active(window->controller, value);
}


void set_strongCoupling_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->strong_coupling_checkbox_spinstate, value);
	gtk_toggle_button_set_active(window->strong_coupling_checkbox_spinsystem, value);
}


G_MODULE_EXPORT void on_strongCoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_strongCoupling(window->controller, value);
}


void set_dipolarRelaxation_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->dipolar_relaxation_checkbox, value);
}


G_MODULE_EXPORT void on_dipolar_relaxation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_dipolarRelaxation(window->controller, value);
}


void set_animation_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->animation_checkbox, value);
}


G_MODULE_EXPORT void on_animation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_animates(window->controller, value);
}


void set_include_relaxation_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->include_relaxation_checkbox, value);
}


G_MODULE_EXPORT void on_include_relaxation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_relaxation_with_evolution(window->controller, value);
}

void set_T1(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.0f", value);
	gtk_entry_set_text(window->T1_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_T1_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_T1(window->controller, value);
}


void set_T2(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.0f", value);
	gtk_entry_set_text(window->T2_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_T2_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_T2(window->controller, value);
}


void set_correlationTime(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value * 1e9);
	gtk_entry_set_text(window->correlationTime_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_correlationTime_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_correlationTime(window->controller, value * 1e-9);
}


void set_delay(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_entry_set_text(window->delay_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_delay_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_delay(window->controller, value);
}


G_MODULE_EXPORT void on_delay_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gchar *tag_string = malloc(8 * sizeof(gchar));
	unsigned int tag;

	if (gtk_combo_box_get_active((GtkComboBox *)combobox) > -1) {
		strcpy(tag_string, gtk_combo_box_get_active_id((GtkComboBox *)combobox));
		tag = atoi(tag_string);
		insensitive_controller_set_delay_from_menuEntry(window->controller, tag);
		g_free(tag_string);
	}
}


void set_dephasingJitter_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->dephasingJitter_checkbox, value);
}


G_MODULE_EXPORT void on_dephasingJitter_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_dephasingJitter(window->controller, value);
}


void set_iDecoupling_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->iDecoupling_checkbox, value);
}


G_MODULE_EXPORT void on_iDecoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_IDecoupling(window->controller, value);
	insensitive_controller_save_decoupling(window->controller);
}


void set_sDecoupling_checkbox(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->sDecoupling_checkbox, value);
}


G_MODULE_EXPORT void on_sDecoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_SDecoupling(window->controller, value);
	insensitive_controller_save_decoupling(window->controller);
}


void set_spinlock(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->spinlock_checkbox, value);

	if (value) {
		gtk_widget_set_sensitive((GtkWidget *)window->iDecoupling_checkbox, FALSE);
		gtk_widget_set_sensitive((GtkWidget *)window->sDecoupling_checkbox, FALSE);
	} else {
		gtk_widget_set_sensitive((GtkWidget *)window->iDecoupling_checkbox, TRUE);
		gtk_widget_set_sensitive((GtkWidget *)window->sDecoupling_checkbox, TRUE);
	}
	insensitive_controller_save_decoupling(window->controller);
}


G_MODULE_EXPORT void on_spinlock_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_spinlock(window->controller, value);
}


void set_gradient_strength(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.0f", value);
	gtk_entry_set_text(window->gradient_strength_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_gradient_strength_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_combo_box_text_get_active_text(combobox));

	insensitive_controller_set_gradientStrength(window->controller, value);
}


G_MODULE_EXPORT void on_gradient_strength_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_gradientStrength(window->controller, value);
}


void set_gradient_duration(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_entry_set_text(window->gradient_duration_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_gradient_duration_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_gradientDuration(window->controller, value);
}


void set_diffusion(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->diffusion_checkbox, value);
}


G_MODULE_EXPORT void on_diffusion_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_diffusion(window->controller, value);
}


void set_dataPoints(InsensitiveWindow *window, int value)
{
	gtk_adjustment_set_value(window->datapoints_adjustment, (gdouble)value);
	gtk_spin_button_set_value(window->datapoints_spinbutton, (gdouble)value);
}


G_MODULE_EXPORT void on_datapoints_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	unsigned int value = gtk_spin_button_get_value_as_int(spinbutton);

	insensitive_controller_set_dataPoints(window->controller, pow2(value));
}


G_MODULE_EXPORT void on_datapoints_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	unsigned int temp, value = gtk_adjustment_get_value(adjustment);

	if (value % pow2(lb(value)) == 1)
		temp = pow2(lb(value) + 1);
	else
		temp = pow2(lb(value));
	insensitive_controller_set_dataPoints(window->controller, temp);
}


void set_dwellTime(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.3f", value);
	gtk_entry_set_text(window->dwelltime_entry, string);
	g_free(string);
}


G_MODULE_EXPORT void on_dwelltime_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_dwellTime(window->controller, value);
}


void set_noiseLevel(InsensitiveWindow *window, float value)
{
	gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.1f", value);
	gtk_entry_set_text(window->noiseLevel_entry, string);
    gtk_entry_set_text(window->noiseLevelSpectrum_entry, string);
    sprintf(string, "%.2f", insensitive_settings_get_signalToNoiseThreshold(window->controller->settings));
	gtk_entry_set_text(window->signalToNoiseThreshold_entry, string);
    if (window->numberOfPeaks > 0) {
       on_auto_peak_picking_button_clicked(NULL, window);
    }
	g_free(string);
}


G_MODULE_EXPORT void on_noise_level_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry));

	insensitive_controller_set_noiseLevel(window->controller, value);
}


void set_acquisitionAfterNextPulse(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->acquisitionAfterNextPulse_checkbox, value);
}


void disable_acquireAfterNextPulse(InsensitiveWindow *window)
{
	gtk_toggle_button_set_active(window->acquisitionAfterNextPulse_checkbox, FALSE);
}


G_MODULE_EXPORT void on_acquisitionAfterNextPulse_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_acquisitionAfterNextPulse(window->controller, value);
}


void set_pulseBeforeAcquisition(InsensitiveWindow *window, gboolean value)
{
	// Not available on Linux
}


void disable_pulseBeforeAcquisition(InsensitiveWindow *window)
{
	// Not available on Linux
}


G_MODULE_EXPORT void on_pulseBeforeAcquisition_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	// Not available on Linux
}


void set_detectISignal(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active((GtkToggleButton *)window->detectISignal_radiobutton, value);
	gtk_toggle_button_set_active((GtkToggleButton *)window->detectSSignal_radiobutton, !value);
}


G_MODULE_EXPORT void on_detectISignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active((GtkToggleButton *)radiobutton);

	insensitive_controller_set_detectISignal(window->controller, value);
	insensitive_controller_set_detectSSignal(window->controller, !value);
}


void set_detectSSignal(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active((GtkToggleButton *)window->detectSSignal_radiobutton, value);
	gtk_toggle_button_set_active((GtkToggleButton *)window->detectISignal_radiobutton, !value);
}


G_MODULE_EXPORT void on_detectSSignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active((GtkToggleButton *)radiobutton);

	insensitive_controller_set_detectISignal(window->controller, !value);
	insensitive_controller_set_detectSSignal(window->controller, value);
}


G_MODULE_EXPORT void detectSignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	GSList *group =  gtk_radio_button_get_group(radiobutton);
	int selected_index = g_slist_index(group, radiobutton);
	int ispin_index = g_slist_index(group, window->detectISignal_radiobutton);
	int sspin_index = g_slist_index(group, window->detectSSignal_radiobutton);

	insensitive_controller_set_detectISignal(window->controller, selected_index == ispin_index);
	insensitive_controller_set_detectSSignal(window->controller, selected_index == sspin_index);
}


void set_zeroFilling(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->zeroFilling_checkbox, value);
}


G_MODULE_EXPORT void on_zeroFilling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_zeroFilling(window->controller, value);
}


void set_vectorDisplayType(InsensitiveWindow *self, enum VectorDisplayType value)
{
	switch (value) {
	case VectorDisplayTypeCoherences:
		gtk_combo_box_set_active((GtkComboBox *)self->vectorDisplayType_combobox, 0);
		break;
	case VectorDisplayTypeMoments:
		gtk_combo_box_set_active((GtkComboBox *)self->vectorDisplayType_combobox, 1);
		break;
	case VectorDisplayTypeFID:
		gtk_combo_box_set_active((GtkComboBox *)self->vectorDisplayType_combobox, 2);
	}
}


G_MODULE_EXPORT void on_vectorDisplayType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_set_vectorDisplayType(window->controller,
						     (enum VectorDisplayType)gtk_combo_box_get_active((GtkComboBox *)combobox));
}


void set_operatorBasis(InsensitiveWindow *self, enum OperatorBasis value)
{
	switch (value) {
	case CartesianOperatorBasis:
		gtk_combo_box_set_active((GtkComboBox *)self->operatorBasis_combobox, 0);
		break;
	case SphericalOperatorBasis:
		gtk_combo_box_set_active((GtkComboBox *)self->operatorBasis_combobox, 1);
	}
}


G_MODULE_EXPORT void on_operatorBasis_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_set_operatorBasis(window->controller,
						 (enum OperatorBasis)gtk_combo_box_get_active((GtkComboBox *)combobox));
}


void set_color1stOrderCoherences(InsensitiveWindow *window, gboolean value)
{
	gtk_toggle_button_set_active(window->color1stOrderCoherences_checkbox, value);
}


G_MODULE_EXPORT void on_color1stOrderCoherences_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	gboolean value = gtk_toggle_button_get_active(checkbox);

	insensitive_controller_set_color1stOrderCoherences(window->controller, value);
	//[matrixView setColored:value];
	//[matrixView setSpins:[[controller spinSystem] spins]];
}


G_MODULE_EXPORT void on_main_window_resize(GtkWindow *main_window, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)main_window;

    set_matrixDisplayType((InsensitiveWindow *)window, insensitive_settings_get_matrixDisplayType(window->controller->settings));
}


void set_matrixDisplayType(InsensitiveWindow *window, enum MatrixDisplayType value)
{
	gint matrix_height;

	switch (value) {
	case MatrixDisplayTypeGraphical:
		gtk_combo_box_set_active((GtkComboBox *)window->matrixDisplayType_combobox, 1);
		switch (insensitive_spinsystem_get_spins(window->controller->spinSystem)) {
		case 1:
			matrix_height = 99;
			break;
		case 2:
			matrix_height = 185;
			break;
		case 3:
			matrix_height = 357;
			break;
		case 4:
			matrix_height = 701;
			break;
		}
		break;
    case MatrixDisplayTypeHidden:
        gtk_combo_box_set_active((GtkComboBox *)window->matrixDisplayType_combobox, 2);
        matrix_height = 5;
        break;
	case MatrixDisplayTypeLarge:
		/*gtk_combo_box_set_active((GtkComboBox *)window->matrixDisplayType_combobox, 1);
		matrix_height = 309;
		break;*/
	case MatrixDisplayTypeTiny:
	case MatrixDisplayTypeSmall:
	default:
		gtk_combo_box_set_active((GtkComboBox *)window->matrixDisplayType_combobox, 0);
		matrix_height = 0.24 * gtk_widget_get_allocated_width((GtkWidget *)window->matrix_drawingarea);//177;
	}
    gtk_widget_set_size_request((GtkWidget *)window->matrix_drawingarea, -1, matrix_height);
	//[matrixView setMatrixFontSize:fontSize forNewSpinNumber:[[controller spinSystem] spins]];
	//[matrixView displayMatrix:[controller spinSystem]];
}


G_MODULE_EXPORT void on_matrixDisplayType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	switch (gtk_combo_box_get_active((GtkComboBox *)combobox)) {
	case 1:
		insensitive_controller_set_matrixDisplayType(window->controller, MatrixDisplayTypeGraphical);
        insensitive_settings_set_showMatrix(window->controller->settings, TRUE);
		break;
    case 2:
		insensitive_controller_set_matrixDisplayType(window->controller, MatrixDisplayTypeHidden);
        insensitive_settings_set_showMatrix(window->controller->settings, FALSE);
		break;
	default:
		insensitive_controller_set_matrixDisplayType(window->controller, MatrixDisplayTypeSmall);
        insensitive_settings_set_showMatrix(window->controller->settings, TRUE);
	}
}


void set_vectorDiagramType(InsensitiveWindow *window, enum VectorDiagramType value)
{
	//[iSpinVectorView setShowXYPlane:value == VectorDiagramXYplane];
	//[sSpinVectorView setShowXYPlane:value == VectorDiagramXYplane];
	//[iSpinVectorView setGrapefruitDiagram:value == VectorDiagramGrapefruit];
	//[sSpinVectorView setGrapefruitDiagram:value == VectorDiagramGrapefruit];
	gtk_combo_box_set_active((GtkComboBox *)window->vectorDiagramType_combobox, value);
	// Now called from controller: spin_type_was_changed(self);
}


G_MODULE_EXPORT void on_vectorDiagramType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	insensitive_controller_set_vectorDiagramType(window->controller,
						     (enum VectorDiagramType)gtk_combo_box_get_active((GtkComboBox *)combobox));
}


/////// //////  // ///    //     /////// //////  // ////////  //////  //////
//      //   // // ////   //     //      //   // //    //    //    // //   //
/////// //////  // // //  //     /////   //   // //    //    //    // //////
     // //      // //  // //     //      //   // //    //    //    // //   //
/////// //      // //   ////     /////// //////  //    //     //////  //   //

gboolean perform_open_spinSystem(InsensitiveWindow *window, xmlNodePtr node)
{
	xmlNodePtr iter;
    gsize data_len;
	unsigned int spins, spinTypeArray, gyroICode, gyroSCode;
    float *couplingMatrix;
	DSPComplex *densityMatrix;
	xmlChar *version, *densityMatrixData, *couplingData;

	for (iter = node; iter; iter = iter->next) {
		if (iter->type == XML_ELEMENT_NODE && !strcmp((char *)iter->name, "key")) {
			if (!strcmp((char *)xmlNodeGetContent(iter), "ISSFileVersion"))
				version = xmlNodeGetContent(iter->next->next);
			else if (!strcmp((char *)xmlNodeGetContent(iter), "Spins"))
				spins = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "SpinTypeArray"))
				spinTypeArray = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "GyroForI"))
				gyroICode = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "GyroForS"))
				gyroSCode = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "CouplingMatrix"))
				couplingData = xmlNodeGetContent(iter->next->next);
			else if (!strcmp((char *)xmlNodeGetContent(iter), "DensityMatrix"))
				densityMatrixData = xmlNodeGetContent(iter->next->next);
		}
	}
    if (!strcmp((char *)version, "Insensitive Spin System 0.9.7")) {
        densityMatrixData = NULL;
        gyroICode = 0;
        gyroSCode = 1;
    } else if (!strcmp((char *)version, "Insensitive Spin System 0.9.13")) {
        densityMatrix = (DSPComplex *)g_base64_decode((char *)densityMatrixData, &data_len);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                       GTK_DIALOG_DESTROY_WITH_PARENT,
							                       GTK_MESSAGE_ERROR,
							                       GTK_BUTTONS_OK,
							                       "This file does not appear to be in the Insensitive Spin System file format.");
	    gtk_window_set_title(GTK_WINDOW(dialog), "Corrupt spin system file");
	    gtk_dialog_run(GTK_DIALOG(dialog));
	    gtk_widget_destroy(dialog);
        return FALSE;
    }
    insensitive_spinsystem_set_spins(window->controller->spinSystem, spins);
    insensitive_spinsystem_set_spintypearray(window->controller->spinSystem, spinTypeArray);
    insensitive_controller_set_gyromagnetic_ratios(window->controller, gyroICode, gyroSCode);
    set_gyromagneticRatio_comboboxes(window, gyroICode, gyroSCode);
    couplingMatrix = (float *)g_base64_decode((char *)couplingData, &data_len);
    insensitive_spinsystem_substitute_couplingmatrix(window->controller->spinSystem, couplingMatrix);
    insensitive_controller_calculate_selectable_delays(window->controller);
    if (densityMatrixData == NULL)
        insensitive_controller_return_to_thermal_equilibrium(window->controller);
    else {
        insensitive_spinsystem_substitute_matrix(window->controller->spinSystem, densityMatrix);
        insensitive_controller_redraw_current_spinsystem_state(window->controller);
    }
    spin_number_was_changed(window);
    return TRUE;
}


G_MODULE_EXPORT void perform_save_spinSystem(GtkMenuItem *menuitem, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    GtkWidget *chooser;
    char *base64, str[12];
    gint result, size;
    gchar *filename;
    xmlDoc *doc = NULL;
    xmlNode *root, *first_child;
    GtkFileFilter *filter;

    chooser = gtk_file_chooser_dialog_new("Save Insensitive Spin System...",
                                          (GtkWindow *)window,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "Cancel", GTK_RESPONSE_CANCEL,
                                          "Save", GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), "spinsystem.iss");
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive spin systems (ISS)");
    gtk_file_filter_add_pattern(filter, "*.iss");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    gtk_widget_show_all(chooser);
    result = gtk_dialog_run((GtkDialog *)chooser);
    if (result == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
        LIBXML_TEST_VERSION
        doc = xmlNewDoc(BAD_CAST "1.0");
        root = xmlNewNode(NULL, BAD_CAST "plist");
        xmlNewProp(root, (const xmlChar *)"version", (const xmlChar *)"1.0");
        xmlDocSetRootElement(doc, root);
        xmlCreateIntSubset(doc, BAD_CAST "plist", BAD_CAST "-//Apple//DTD PLIST 1.0//EN", BAD_CAST "http://www.apple.com/DTDs/PropertyList-1.0.dtd");
        first_child = xmlNewChild(root, NULL, BAD_CAST "dict", NULL);

        size = insensitive_spinsystem_get_spins(window->controller->spinSystem);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "CouplingMatrix");
        base64 = g_base64_encode((const guchar *)insensitive_spinsystem_get_raw_couplingmatrix(window->controller->spinSystem),
                                 size * size * sizeof(float));
        xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
        g_free(base64);
        size = insensitive_spinsystem_get_size(window->controller->spinSystem);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "DensityMatrix");
        base64 = g_base64_encode((const guchar *)insensitive_spinsystem_get_raw_densitymatrix(window->controller->spinSystem),
                                 size * size * sizeof(DSPComplex));
        xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
        g_free(base64);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "GyroForI");
        sprintf(str, "%d", insensitive_settings_get_gyroCodeI(window->controller->settings));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "GyroForS");
        sprintf(str, "%d", insensitive_settings_get_gyroCodeS(window->controller->settings));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ISSFileVersion");
        xmlNewChild(first_child, NULL, BAD_CAST "string", BAD_CAST "Insensitive Spin System 0.9.13");
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "SpinTypeArray");
        sprintf(str, "%d", insensitive_spinsystem_get_spintypearray(window->controller->spinSystem));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "Spins");
        sprintf(str, "%d", insensitive_spinsystem_get_spins(window->controller->spinSystem));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlMemoryDump();
        set_openedFileState_for_spinSystem(window, FileOpened, filename);
        g_free(filename);
    }
    g_object_unref(chooser);
}


void set_openedFileState_for_spinSystem(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename)
{
    if(state == FileOpened && filename != NULL) {
        //[[self window] setTitle:[NSString stringWithFormat:@"%@: %@", NSLocalizedString(@"Spin Network Editor", nil), filename]];
        window->openFileState_for_spinSystem = state;
    } else if(state == FileOpenedAndChanged && window->openFileState_for_spinSystem == FileOpened) {
        //[[self window] setTitle:[NSString stringWithFormat:NSLocalizedString(@"%@ [modified]", nil), [[self window] title]]];
        window->openFileState_for_spinSystem = state;
    } else if(state == NoFile) {
        //[[self window] setTitle:NSLocalizedString(@"Spin Network Editor", nil)];
        window->openFileState_for_spinSystem = state;
    }
}


void spin_was_selected(InsensitiveWindow *window, unsigned int spin)
{
	gchar *string;

	if (spin > maxNumberOfSpins)
        insensitive_controller_set_selected_spin(window->controller, 0);
	else
		insensitive_controller_set_selected_spin(window->controller, spin);

	/* It would be nice to have a context menu here to choose which variable is set */
    string = malloc(10 * sizeof(gchar));
    sprintf(string, "%.2f", insensitive_spinsystem_get_larmorfrequency_for_spin(window->controller->spinSystem,
                                                                                insensitive_controller_get_selected_spin(window->controller)));
    gtk_entry_set_text(window->chemicalShift_entry, string);
    free(string);
}


void set_spin_type(InsensitiveWindow *window, unsigned int spin)
{
    if(insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin) == spinTypeI)
        insensitive_spinsystem_set_spintype_for_spin(window->controller->spinSystem, spinTypeS, spin);
    else if(insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin) == spinTypeS)
        insensitive_spinsystem_set_spintype_for_spin(window->controller->spinSystem, spinTypeI, spin);

    spin_number_was_changed(window);
    spin_state_was_changed(window);

    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_chemicalShift_entry_activate(GtkEntry *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = atof(gtk_entry_get_text(entry)) / insensitive_controller_get_unitConversion(window->controller);

	insensitive_controller_set_larmorFrequency_for_spin(window->controller,
                                                        insensitive_controller_get_selected_spin(window->controller),
                                                        value);
    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_scalarConstant_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_spinsystem_get_spins(window->controller->spinSystem) == 2)
        insensitive_controller_set_jCouplingConstant_between_spins(window->controller, 0, 1, atof(gtk_entry_get_text(entry)));
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_dipolarConstant_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_spinsystem_get_spins(window->controller->spinSystem) == 2)
        insensitive_controller_set_dipolarCouplingConstant_between_spins(window->controller, 0, 1, atof(gtk_entry_get_text(entry)));
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_distanceConstant_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_spinsystem_get_spins(window->controller->spinSystem) == 2)
        insensitive_controller_set_distance_between_spins(window->controller, 0, 1, atof(gtk_entry_get_text(entry)));
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


void set_larmorFrequency(InsensitiveWindow *window, float value)
{
    gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value * insensitive_controller_get_unitConversion(window->controller));
	gtk_entry_set_text(window->chemicalShift_entry, string);
	g_free(string);
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


void set_scalarConstant(InsensitiveWindow *window, float value)
{
    gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_entry_set_text(window->scalarConstant_entry, string);
	g_free(string);
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


void set_dipolarConstant(InsensitiveWindow *window, float value)
{
    gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_entry_set_text(window->dipolarConstant_entry, string);
	g_free(string);
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


void set_distanceConstant(InsensitiveWindow *window, float value)
{
    gchar *string = malloc(10 * sizeof(gchar));

	sprintf(string, "%.2f", value);
	gtk_entry_set_text(window->distanceConstant_entry, string);
	g_free(string);
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


G_MODULE_EXPORT void on_addSpin_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_spin_number_changed(window->controller,
                                               insensitive_spinsystem_get_spins(window->controller->spinSystem) + 1);
    // Select the added spin
    spin_was_selected(window, insensitive_spinsystem_get_spins(window->controller->spinSystem) - 1);
}


G_MODULE_EXPORT void on_removeSpin_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_spin_number_changed(window->controller,
                                               insensitive_spinsystem_get_spins(window->controller->spinSystem) - 1);
}


G_MODULE_EXPORT void on_spinNumber_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	float value = gtk_adjustment_get_value(adjustment);

	insensitive_controller_spin_number_changed(window->controller, (unsigned int)value);
}


G_MODULE_EXPORT void on_spinNumber_spinbutton_activate(GtkSpinButton *entry, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	unsigned int number = (unsigned int)gtk_spin_button_get_value(entry);

	insensitive_controller_spin_number_changed(window->controller, number);
}


void set_spin_number(InsensitiveWindow *window, unsigned int number)
{
	if(number < 2)
        gtk_widget_set_sensitive((GtkWidget *)window->removeSpin_button, FALSE);
    else
        gtk_widget_set_sensitive((GtkWidget *)window->removeSpin_button, TRUE);

    if(number > 3)
        gtk_widget_set_sensitive((GtkWidget *)window->addSpin_button, FALSE);
    else
        gtk_widget_set_sensitive((GtkWidget *)window->addSpin_button, TRUE);

    gtk_adjustment_set_value(window->spinNumber_adjustment, (gdouble)number);

    set_openedFileState_for_spinSystem(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_reset_constants_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_reset_couplingMatrix(window->controller);
    //[self setOpenedFileState:NoFile withName:nil];
}


G_MODULE_EXPORT void on_chemicalShift_units_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if (gtk_combo_box_get_active(combobox) == 1)
        insensitive_controller_set_chemicalShiftUnitsToDegreesPerSecond(window->controller, TRUE);
    else
        insensitive_controller_set_chemicalShiftUnitsToDegreesPerSecond(window->controller, FALSE);

}


void set_chemicalShift_units_to_degreesPerSecond(InsensitiveWindow *window, gboolean value)
{
    if(value) {
        gtk_combo_box_set_active((GtkComboBox *)window->chemicalShift_units_combobox, 1);
        gtk_label_set_text(window->chemicalShift_unit_label, "°/s     ");
    } else {
        gtk_combo_box_set_active((GtkComboBox *)window->chemicalShift_units_combobox, 0);
        gtk_label_set_text(window->chemicalShift_unit_label, "Hz     ");
    }
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


G_MODULE_EXPORT void on_displayedConstant_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    switch(gtk_combo_box_get_active(combobox)) {
    case 1:
        window->displayedConstant = DipolarConstant;
        break;
    case 2:
        window->displayedConstant = DistanceConstant;
        break;
    default:
        window->displayedConstant = ScalarConstant;
    }
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


G_MODULE_EXPORT void on_rotate_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_rotate_spinsystem(window->controller, FALSE);
}


G_MODULE_EXPORT void on_gyro_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_gyromagnetic_ratios(window->controller,
                                                   gtk_combo_box_get_active((GtkComboBox *)window->gyroI_combobox),
                                                   gtk_combo_box_get_active((GtkComboBox *)window->gyroS_combobox));
}


void set_gyromagneticRatio_comboboxes(InsensitiveWindow *window, unsigned int codeForI, unsigned int codeForS)
{
    gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, codeForI);
    gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, codeForS);
}


 /////  //    // //      /////// ///////     /////// ///////  //////  //    // /////// ///    //  ////// ///////
//   // //    // //      //      //          //      //      //    // //    // //      ////   // //      //
//////  //    // //      /////// /////       /////// /////   //    // //    // /////   // //  // //      /////
//      //    // //           // //               // //      // // // //    // //      //  // // //      //
//       //////  /////// /////// ///////     /////// ///////  //////   //////  /////// //   ////  ////// ///////
                                                                 //

void set_recording_button_clicked(InsensitiveWindow *window, gboolean value)
{
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info;

    if (value) {
        gtk_button_set_label(window->record_button, "Stop");
        icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-stop", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
    } else {
        gtk_button_set_label(window->record_button, "Record");
        icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-record", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
    }
    gtk_button_set_image(window->record_button, gtk_image_new_from_file(gtk_icon_info_get_filename(icon_info)));
    g_clear_object(&icon_info);
}


void set_variable_evolution_time(InsensitiveWindow *window, int value)
{
    gchar *tag = malloc(5 * sizeof(gchar));

    window->variableEvolutionTime = value;
    sprintf(tag, "%d", value);
    gtk_combo_box_set_active_id((GtkComboBox *)window->evolutionTimes_combobox, tag);
    free(tag);

    /*if(value == 0)
        [acquire2DSpectrumButton setEnabled:FALSE];
    else
        [acquire2DSpectrumButton setEnabled:TRUE];*/
}


void allow_spectrum_acquisition(InsensitiveWindow *window, gboolean value)
{
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info;

    window->acquisitionIsBeingPerformed = !value;
    gtk_widget_set_sensitive((GtkWidget *)window->record_button, value);
    if (value) {
        gtk_button_set_label(window->play_button, "Play");
        icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-play", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
        gtk_button_set_image(window->play_button, gtk_image_new_from_file(gtk_icon_info_get_filename(icon_info)));
        g_clear_object(&icon_info);
    }
    gtk_widget_set_sensitive((GtkWidget *)window->evolutionTimes_combobox, value);
    if(insensitive_controller_get_variableEvolutionTime(window->controller) == 0) {
        gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, FALSE);
        insensitive_controller_set_detectionMethod(window->controller, None);
    } else {
        gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, value);
    }
    gtk_widget_set_sensitive((GtkWidget *)window->phaseCycles_combobox, value);
    //[acquire2DSpectrumButton setEnabled:value && [[evolutionTimesPopupButton selectedItem] tag] != 0];
    //[eraseButton setEnabled:value];
    gtk_widget_set_sensitive((GtkWidget *)window->phaseCycling_treeview, value);
    gtk_widget_set_sensitive((GtkWidget *)window->bottomDisplay_combobox, value);
}


void enable_pulseSequence_play_button(InsensitiveWindow *window, gboolean value)
{
    gtk_widget_set_sensitive((GtkWidget *)window->play_button, value);
    gtk_widget_set_sensitive((GtkWidget *)window->step_button, value);
}


void set_acquisition_in_background(InsensitiveWindow *window, gboolean value)
{
}


void set_phaseCycling_combobox(InsensitiveWindow *window, int value)
{
    gchar *str = malloc(5 * sizeof(gchar));

    sprintf(str, "%d", value);
    gtk_entry_set_text(window->phaseCycles_entry, str);
    //gtk_combo_box_set_active((GtkComboBox *)window->phaseCycles_combobox, value);
}


void set_detectionMethod(InsensitiveWindow *window, enum PurePhaseDetectionMethod value)
{
    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, (int)value);
}


void set_current_step_in_pulseSequence(InsensitiveWindow *window, unsigned int value)
{
    window->nextStepInPulseSequence = value;
    if(insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence) > 0)
        update_pulseSequence(window);//g_idle_add((GSourceFunc)update_pulseSequence, window);
}

gboolean update_pulseSequence(InsensitiveWindow *window)
{
    InsensitiveController *controller = window->controller;
    InsensitivePulseSequence *pulsesequence = controller->pulseSequence;
    int width, height;

    if(!window->acquisitionIsBeingPerformed && GTK_IS_WINDOW(window)) {
        width = gtk_widget_get_allocated_width(GTK_WIDGET(window->pulseSequence_drawingarea));
        height = gtk_widget_get_allocated_height(GTK_WIDGET(window->pulseSequence_drawingarea));
        if (window->pulseSequence_surface != NULL)
            cairo_surface_destroy(window->pulseSequence_surface);
        window->pulseSequence_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        create_pulseSequence_view(window, width, height);

        resize_pulseSequence_view(window);
        gtk_widget_queue_draw((GtkWidget *)window->pulseSequence_drawingarea);
        gtk_widget_queue_draw((GtkWidget *)window->pulseSequenceStep_drawingarea);
        if(insensitive_pulsesequence_get_number_of_elements(pulsesequence) > 0)
            enable_pulseSequence_play_button(window, TRUE);
        else
            enable_pulseSequence_play_button(window, FALSE);
        if(insensitive_controller_get_variableEvolutionTime(controller) == 0) {
            gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, FALSE);
            insensitive_controller_set_detectionMethod(controller, None);
        } else {
            gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, TRUE);
        }
        close_coherencePathway(window);
        display_pulseProgram_code(window);
    }

	return FALSE;
}


void resize_pulseSequence_view(InsensitiveWindow *window)
{
    int step;
    float width = 100.0;
    SequenceElement *currentElement;

    for(step = 0; step < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence); step++) {
        currentElement = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, step);
        switch(currentElement->type) {
        case SequenceTypePulse:
            width += currentElement->time / 360 * 50;
            break;
        case SequenceTypeEvolution:
            width += currentElement->time * 100;
            break;
        case SequenceTypeGradient:
            width += currentElement->time * 30;
            break;
        case SequenceTypeFID:
            width += 100;
            break;
        default:
            width += 0;
        }
    }
    gtk_widget_set_size_request(GTK_WIDGET(window->pulseSequence_drawingarea), (int)width, -1);
    gtk_widget_set_size_request(GTK_WIDGET(window->coherencePathway_drawingarea), (int)width, -1);
    gtk_widget_set_size_request(GTK_WIDGET(window->pulseSequenceStep_drawingarea), (int)(0.707 * width), -1);
}


gboolean redraw_pulseSequence(InsensitiveWindow *window)
{
    //[self finishEditingSequenceElement:self];
    resize_pulseSequence_view(window);
    gtk_widget_queue_draw((GtkWidget *)window->pulseSequence_drawingarea);
    gtk_widget_queue_draw((GtkWidget *)window->pulseSequenceStep_drawingarea);

	return FALSE;
}


G_MODULE_EXPORT void on_bottomDisplay_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gint index = gtk_combo_box_get_active(combobox);

    if (gtk_combo_box_get_active(combobox) == 1) {
        if (window->needsToRecalculateCoherencePathways) {
            //[coherenceProgressIndicator startAnimation:self];
            insensitive_controller_perform_threaded_coherencePathway_calculation(window->controller);
        }
    } else
        close_coherencePathway(window);
    gtk_notebook_set_current_page(window->bottomDisplay_notebook, index);
}


void close_coherencePathway(InsensitiveWindow *window)
{
    insensitive_controller_interrupt_coherencePathway_calculation(window->controller);
    if (gtk_notebook_get_current_page(window->bottomDisplay_notebook) == 1) {
        gtk_notebook_set_current_page(window->bottomDisplay_notebook, 0);
        gtk_combo_box_set_active((GtkComboBox *)window->bottomDisplay_combobox, 0);
    }
    //[coherenceProgressIndicator stopAnimation:self];
    calculate_coherence_paths(window);
}


void display_pulseProgram_code(InsensitiveWindow *window)
{
    gchar *filename;
    gchar alt_text[] = "A sequence ending with data acquisition is required to compute pulse program code.\0";
    GString *pp;
    GtkTextBuffer *buffer = window->pulseProgram_textbuffer;
    GtkTextIter start, end, temp, temp2, temp3;

    filename = insensitive_controller_get_pulseSequence_name(window->controller);
    if(filename == NULL || strlen(filename) < 1)
        filename = "<no name>";
    pp = insensitive_controller_export_pulseSequence(window->controller, filename);

    if(pp != NULL) {
        gtk_text_buffer_set_text(buffer, pp->str, pp->len);
        g_string_free(pp, TRUE);

        // Apply syntax highlighting
        gtk_text_buffer_get_start_iter(buffer, &start);
        while (!gtk_text_iter_starts_word(&start))
            gtk_text_iter_forward_char(&start);
        end = start;
        while (gtk_text_iter_forward_word_end(&end)) {
            const gchar *word = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
            if (!strcmp(word, "exit") || !strcmp(word, "go") || !strcmp(word, "ze") || !strcmp(word, "lo")
                || !strcmp(word, "to") || !strcmp(word, "times") || !strcmp(word, "center") || !strcmp(word, "mc")
                || !strcmp(word, "cw") || !strcmp(word, "do") || !strcmp(word, "goto") || !strcmp(word, "if")
                || !strcmp(word, "else")) {
                gtk_text_buffer_apply_tag_by_name(buffer, "plum", &start, &end);
            }
            start = end;
            for (;;) {
                if (!gtk_text_iter_forward_char (&start))
                    goto doneKeywords;
                if (gtk_text_iter_starts_word (&start))
                    break;
            }
        }
doneKeywords:
        gtk_text_buffer_get_start_iter(buffer, &start);
        do {
            end = start;
            gtk_text_iter_forward_chars(&end, 2);
            // Comments
            if (gtk_text_iter_get_char(&start) == ';') {
                gtk_text_iter_forward_to_line_end(&end);
                gtk_text_buffer_apply_tag_by_name(buffer, "clover", &start, &end);
                start = end;
            // Preprocessor commands
            } else if (!strcmp(gtk_text_buffer_get_text(buffer, &start, &end, FALSE), "#i")
                       || !strcmp(gtk_text_buffer_get_text(buffer, &start, &end, FALSE), "#e")) {
                gtk_text_iter_forward_to_line_end(&end);
                gtk_text_buffer_apply_tag_by_name(buffer, "mocha", &start, &end);
                start = end;
            // Quoted strings
            } else if (gtk_text_iter_get_char(&start) == '"') {
                temp = start;
                end = start;
                gtk_text_iter_forward_char(&temp);
                do {
                    gtk_text_iter_forward_char(&temp);
                    gtk_text_iter_forward_char(&end);
                } while (*gtk_text_buffer_get_text(buffer, &temp, &end, FALSE) != '"' && !gtk_text_iter_is_end(&temp));
                gtk_text_iter_forward_char(&end);
                gtk_text_buffer_apply_tag_by_name(buffer, "cayenne", &start, &end);
                start = end;
            // Variables (ph#)
            } else if (!strcmp(gtk_text_buffer_get_text(buffer, &start, &end, FALSE), "ph")) {
                while (g_ascii_isdigit(gtk_text_iter_get_char(&end))) {
                    gtk_text_iter_forward_char(&end);
                }
                if (strlen(gtk_text_iter_get_text(&start, &end)) > 2)
                    gtk_text_buffer_apply_tag_by_name(buffer, "eggplant", &start, &end);
            // Digits
            } else if (g_ascii_isdigit(gtk_text_iter_get_char(&start))) {
                temp = start;
                gtk_text_iter_backward_char(&temp);
                temp2 = temp;
                gtk_text_iter_backward_char(&temp2);
                temp3 = start;
                gtk_text_iter_forward_char(&temp3);
                if (!g_ascii_isalpha(gtk_text_iter_get_char(&temp)) && gtk_text_iter_get_char(&temp) != '#') {
                    if (!g_ascii_isalpha(gtk_text_iter_get_char(&temp2)) || gtk_text_iter_get_char(&temp) == '='
                        || gtk_text_iter_get_char(&temp) == '\n' || gtk_text_iter_get_char(&temp) == ' ') {
                        gtk_text_iter_backward_char(&end);
                        gtk_text_buffer_apply_tag_by_name(buffer, "blueberry", &start, &end);
                    }
                }
            }
        } while (gtk_text_iter_forward_char(&start));
        // file name
        gtk_text_buffer_get_start_iter(buffer, &start);
        do {
            end = start;
            if (gtk_text_iter_get_char(&start) == '<') {
                gtk_text_iter_forward_chars(&end, 9);
                if (!strcmp(gtk_text_iter_get_text(&start, &end), "<no name>"))
                    gtk_text_buffer_apply_tag_by_name(buffer, "sky", &start, &end);
            }
        } while (gtk_text_iter_forward_char(&start));
    } else
        gtk_text_buffer_set_text(buffer, alt_text, strlen(alt_text));
}


G_MODULE_EXPORT void export_pulse_program(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    GString *pp;
    gchar *filename, *name, *nameWithoutSuffix;
    gint result, len;
    GtkWidget *dialog, *chooser;

    if (insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence) > 0) {
        pp = insensitive_controller_export_pulseSequence(window->controller, "<no name>");
        if (pp != NULL) {
            chooser = gtk_file_chooser_dialog_new("Export Bruker Pulse Program",
                                                  (GtkWindow *)window,
                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                  "Cancel", GTK_RESPONSE_CANCEL,
                                                  "Export", GTK_RESPONSE_ACCEPT,
                                                  NULL);
            name = insensitive_controller_get_pulseSequence_name(window->controller);
            if (name != NULL && strlen(name) > 0)
                gtk_file_chooser_set_current_name((GtkFileChooser *)chooser, name);
            gtk_widget_show_all(chooser);
            result = gtk_dialog_run((GtkDialog *)chooser);
            show_mainWindow_notebook_page(window, 2);
            gtk_combo_box_set_active((GtkComboBox *)window->bottomDisplay_combobox, 2);
            if (result == GTK_RESPONSE_ACCEPT) {
                filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
                name = g_path_get_basename(filename);
                // Cut suffix from name if user entered .txt or .pp
                nameWithoutSuffix = malloc((strlen(name) + 1) * sizeof(gchar));
                strcpy(nameWithoutSuffix, name);
                len = strlen(nameWithoutSuffix);
                if (!strcmp(name + len - 4, ".txt"))
                    *(nameWithoutSuffix + len - 4) = '\0';
                else if (!strcmp(name + len - 3, ".pp"))
                    *(nameWithoutSuffix + len - 3) = '\0';
                // Replace placeholder by file name
                while (insensitive_g_string_replace(pp, "<no name>", nameWithoutSuffix, pp));
                FILE *f = fopen(filename, "w");
                if (f == NULL) {
                    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                        GTK_DIALOG_DESTROY_WITH_PARENT,
							                        GTK_MESSAGE_ERROR,
							                        GTK_BUTTONS_OK,
							                        "Could not write pulse program to file %s.", filename);
		            gtk_window_set_title(GTK_WINDOW(dialog), "Exporting pulse program failed");
		            gtk_dialog_run(GTK_DIALOG(dialog));
		            gtk_widget_destroy(dialog);
                } else {
                    fprintf(f, "%s", pp->str);
                    fclose(f);
                    insensitive_controller_set_name_for_pulseSequence(window->controller, nameWithoutSuffix);
                    update_pulseSequence(window);
                }
                g_free(nameWithoutSuffix);
                g_free(name);
                g_free(filename);
            }
            g_object_unref(chooser);
        }
        g_string_free(pp, TRUE);
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							            GTK_DIALOG_DESTROY_WITH_PARENT,
							            GTK_MESSAGE_ERROR,
							            GTK_BUTTONS_OK,
							            "There is no pulse program that can be saved.");
		gtk_window_set_title(GTK_WINDOW(dialog), "No pulse program recorded");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
    }
}


void update_evolutionTimes_combobox(InsensitiveWindow *window)
{
    GtkComboBoxText *combobox = window->evolutionTimes_combobox;
    InsensitivePulseSequence *pulseSequence = window->controller->pulseSequence;
    SequenceElement *element;
    unsigned int i, numberOfElements;
    int evolutionTimeIndex = 0;
    float firstHalfOfDelay;
    gchar *delayLabel, *delayLabelWithIndices, *tag;

    gtk_combo_box_text_remove_all(combobox);
    delayLabel = malloc(15 * sizeof(gchar));
    strcpy(delayLabel, "FID");
    tag = malloc(5 * sizeof(gchar));
    sprintf(tag, "0");
    gtk_combo_box_text_append(combobox, tag, delayLabel);
    free(delayLabel);
    numberOfElements = insensitive_pulsesequence_get_number_of_elements(pulseSequence);
    for (i = 0; i < numberOfElements; i++) {
        element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i);
        if (element->type == SequenceTypeEvolution) {
            evolutionTimeIndex++;
            delayLabel = malloc(15 * sizeof(gchar));
            sprintf(delayLabel, "τ%d", evolutionTimeIndex);
            delayLabelWithIndices = replace_numbers_by_indices(delayLabel);
            free(delayLabel);
            tag = malloc(5 * sizeof(gchar));
            sprintf(tag, "%d", i + 1);
            gtk_combo_box_text_append(combobox, tag, delayLabelWithIndices);
            firstHalfOfDelay = element->time;
            // Check for spin echo sandwich
            if ((i + 2) < numberOfElements) {
                element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 1);
                if (element->type == SequenceTypePulse) {
                    element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 2);
                    if (element->type == SequenceTypeEvolution && element->time == firstHalfOfDelay) {
                        delayLabel = malloc(28 * sizeof(gchar));
                        sprintf(delayLabel, "τ%d + τ%d", evolutionTimeIndex, evolutionTimeIndex + 1);
                        delayLabelWithIndices = replace_numbers_by_indices(delayLabel);
                        free(delayLabel);
                        tag = malloc(5 * sizeof(gchar));
                        sprintf(tag, "%d", -i - 1);
                        gtk_combo_box_text_append(combobox, tag, delayLabelWithIndices);
                    }
                }
            }
            // Check for spin echo sandwich with gradients
            if((i + 4) < numberOfElements) {
                element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 1);
                if(element->type == SequenceTypeGradient) {
                    element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 2);
                    if(element->type == SequenceTypePulse) {
                        element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 3);
                        if(element->type == SequenceTypeGradient) {
                            element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 4);
                            if(element->type == SequenceTypeEvolution && element->time == firstHalfOfDelay) {
                                delayLabel = malloc(28 * sizeof(gchar));
                                sprintf(delayLabel, "τ%d + τ%d", evolutionTimeIndex, evolutionTimeIndex + 1);
                                delayLabelWithIndices = replace_numbers_by_indices(delayLabel);
                                free(delayLabel);
                                tag = malloc(5 * sizeof(gchar));
                                sprintf(tag, "%d", -i - 1);
                                gtk_combo_box_text_append(combobox, tag, delayLabelWithIndices);
                            }
                        }
                    }
                }
            }
        }
    }
    gtk_combo_box_set_active((GtkComboBox *)combobox, 0);
}


void insert_column_into_phaseCyclingTable(InsensitiveWindow *window, unsigned int old_n_columns, unsigned int n)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gchar *str = malloc(10 * sizeof(char));
    unsigned int pos, new_n_columns;

    //old_n_columns = gtk_tree_view_get_n_columns(window->phaseCycling_treeview);
    //old_n_columns = window->controller->pulseList->len + 1;
    new_n_columns = old_n_columns + 1;
    pos = old_n_columns - 1;

    gtk_tree_view_set_model(window->phaseCycling_treeview, NULL);
    // Add column to GtkTreeView and connect new model
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", (GCallback)on_phaseCycling_treeview_edited, window);
    g_object_set_data(G_OBJECT(renderer), "column_number", GUINT_TO_POINTER(old_n_columns));
    sprintf(str, "Phase %d", pos);
    column = gtk_tree_view_column_new_with_attributes(str, renderer, "text", old_n_columns, NULL);
    gtk_tree_view_insert_column(window->phaseCycling_treeview, column, pos);
    free(str);

    update_phaseCyclingTable(window, new_n_columns - 1);
}


void remove_last_column_from_phaseCyclingTable(InsensitiveWindow *window)
{
    GtkTreeViewColumn *column;
    unsigned int old_n_columns, new_n_columns;

    old_n_columns = gtk_tree_view_get_n_columns(window->phaseCycling_treeview);
    new_n_columns = old_n_columns - 1;

    gtk_tree_view_set_model(window->phaseCycling_treeview, NULL);
    column = gtk_tree_view_get_column(window->phaseCycling_treeview, old_n_columns - 2);
    gtk_tree_view_remove_column(window->phaseCycling_treeview, column);

    update_phaseCyclingTable(window, new_n_columns);
}


void reset_phaseCyclingTable(InsensitiveWindow *window)
{
    unsigned int row;
    gchar *str;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeIter iter;

    gtk_tree_view_set_model(window->phaseCycling_treeview, NULL);
    while (gtk_tree_view_get_n_columns(window->phaseCycling_treeview)) {
        column = gtk_tree_view_get_column(window->phaseCycling_treeview, 0);
        gtk_tree_view_remove_column(window->phaseCycling_treeview, column);
    }
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Step", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(window->phaseCycling_treeview, column);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", (GCallback)on_phaseCycling_treeview_edited, window);
    g_object_set_data(G_OBJECT(renderer), "column_number", GUINT_TO_POINTER(1));
    column = gtk_tree_view_column_new_with_attributes("Receiver", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(window->phaseCycling_treeview, column);
    gtk_tree_view_set_grid_lines(window->phaseCycling_treeview, GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

    //if(GTK_IS_LIST_STORE(window->phaseCycling_liststore))
    //    gtk_list_store_clear(window->phaseCycling_liststore);
    str = malloc(5 * sizeof(gchar));
    for (row = 0; row < insensitive_controller_get_numberOfPhaseCycles(window->controller); row++) {
        window->phaseCycling_liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
        gtk_list_store_append(window->phaseCycling_liststore, &iter);
        sprintf(str, "%d", row + 1);
        gtk_list_store_set(window->phaseCycling_liststore, &iter, 0, str, 1, "0", -1);
    }
    free(str);
    gtk_tree_view_set_model(window->phaseCycling_treeview, (GtkTreeModel *)window->phaseCycling_liststore);
    update_phaseCyclingTable(window, 1);
}


void update_phaseCyclingTable(InsensitiveWindow *window, unsigned int number_of_columns)
{
    // number_of_columns = number of columns in phaseCyclingArray (controller)
    // ncol = number of columns in phaseCycling_liststore and phaseCycling_treeview (window)

    unsigned int row, col, phaseCycles;
    unsigned int ncol = number_of_columns + 1;
    int n = gtk_tree_view_get_n_columns(window->phaseCycling_treeview) - 1;
    GtkTreeIter iter;
    GType *types;
    gint *columns;
    GValue *values;
    gchar *str = malloc(10 * sizeof(char));
    GtkListStore *newPhaseCyclingTable;
    GtkTreeViewColumn *column = gtk_tree_view_get_column(window->phaseCycling_treeview, n);;
    GPtrArray *phaseCyclingArray = insensitive_controller_get_phaseCyclingArray(window->controller);

    // Create a new GtkListStore
    gtk_tree_view_set_model(window->phaseCycling_treeview, NULL);
    types = malloc(ncol * sizeof(GType));
	for (col = 0; col < ncol; col++)
		types[col] = G_TYPE_STRING;
    newPhaseCyclingTable = gtk_list_store_newv(ncol, types);
    free(types);
    phaseCycles = insensitive_controller_get_numberOfPhaseCycles(window->controller);
    columns = malloc(ncol * sizeof(gint));
    values = calloc(ncol, sizeof(GValue));
    for (row = 0; row < phaseCycles; row++) {
        for (col = 0; col < ncol; col++) {
            columns[col] = col;
            // array: rec ph1 ph2 ph3 ...
            if (col == 0)
                sprintf(str, "%d", row + 1);
            else if (col == 1) {
                if (phaseCyclingArray->len <= row * number_of_columns)
                    sprintf(str, "NaN");
                else
                    sprintf(str, "%s", (gchar *)g_ptr_array_index(phaseCyclingArray, row * number_of_columns));
            } else {
                if (phaseCyclingArray->len <= row * number_of_columns + col - 1)
                    sprintf(str, "NaN");
                else
                    sprintf(str, "%s", (gchar *)g_ptr_array_index(phaseCyclingArray, row * number_of_columns + col - 1));
            }
            if (row == 0)
                g_value_init(values + col, G_TYPE_STRING);
            g_value_set_string(values + col, str);
        }
        gtk_list_store_insert_with_valuesv(newPhaseCyclingTable, &iter, row, columns, values, ncol);
    }
    gtk_tree_view_set_model(window->phaseCycling_treeview, (GtkTreeModel *)newPhaseCyclingTable);
    g_object_unref(window->phaseCycling_liststore);
    window->phaseCycling_liststore = newPhaseCyclingTable;
    free(columns);
    free(values);
    free(str);

    // Show receiver phase only if FID present in pulse sequence
    if (insensitive_controller_get_pulseSequence_ends_with_acquisition(window->controller))
        gtk_tree_view_column_set_visible(column, TRUE);
    else
        gtk_tree_view_column_set_visible(column, FALSE);
}


G_MODULE_EXPORT void on_phaseCycling_treeview_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int phase;
    guint index;
    guint row_number = atoi(path_string);
    guint column_number = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(cell), "column_number"));
    GtkTreeIter iter;
    GPtrArray *phaseCyclingArray = insensitive_controller_get_phaseCyclingArray(window->controller);

    // Make Insensitive understand modulo 90 notation: 1 = 90°, 2 = 180°, 3 = 270°
    phase = atoi(new_text);
    if (phase == 0 || phase == 1 || phase == 2 || phase == 3)
        sprintf(new_text, "%d", 90 * phase);

    gtk_tree_model_get_iter_from_string((GtkTreeModel *)window->phaseCycling_liststore, &iter, path_string);
    gtk_list_store_set(window->phaseCycling_liststore, &iter, column_number, new_text, -1);
    index = column_number + row_number * (gtk_tree_view_get_n_columns(window->phaseCycling_treeview) - 1) - 1;
    strcpy(g_ptr_array_index(phaseCyclingArray, index), new_text);

    set_openedFileState_for_pulseSequence(window, FileOpenedAndChanged, NULL);
    display_pulseProgram_code(window);
}


G_MODULE_EXPORT void on_record_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    close_coherencePathway(window);
    insensitive_controller_toggle_recordingPulseSequence(window->controller);
    if (insensitive_controller_get_isRecordingPulseSequence(window->controller)) {
        show_mainWindow_notebook_page(window, 1);
		gtk_widget_show(GTK_WIDGET(window->step_window));
	}
}


G_MODULE_EXPORT void on_play_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info;
    //SequenceElement *lastElement = insensitive_pulsesequence_get_last_element(window->controller->pulseSequence);

    if(insensitive_controller_get_acquisitionIsInProgress(window->controller)) {
        on_acquire_button_clicked(window->acquire_button, window);
        gtk_button_set_label(button, "Play");
        icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-play", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
        gtk_button_set_image(button, gtk_image_new_from_file(gtk_icon_info_get_filename(icon_info)));
        g_clear_object(&icon_info);
    } else {
        if(insensitive_controller_get_variableEvolutionTime(window->controller) == 0
           || insensitive_pulsesequence_get_last_element(window->controller->pulseSequence)->type != SequenceTypeFID) {
            insensitive_controller_perform_pulseSequence(window->controller);
            if(insensitive_pulsesequence_get_last_element(window->controller->pulseSequence)->type == SequenceTypeFID) {
                start_progress_indicator(window);
                set_user_controls_enabled(window, FALSE);
            }
            insensitive_controller_interrupt_coherencePathway_calculation(window->controller);
        } else {
            on_acquire2DSpectrum_button_clicked(NULL, window);
        }
        if(insensitive_pulsesequence_get_last_element(window->controller->pulseSequence)->type == SequenceTypeFID) {
            gtk_button_set_label(button, "Stop");
            icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-stop", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
            gtk_button_set_image(button, gtk_image_new_from_file(gtk_icon_info_get_filename(icon_info)));
            g_clear_object(&icon_info);
        }
    }
}


G_MODULE_EXPORT void on_step_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    InsensitiveController *controller = window->controller;
    InsensitivePulseSequence *pulsesequence = controller->pulseSequence;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info;

    if(insensitive_pulsesequence_get_number_of_elements(pulsesequence) > 0) {
        if(insensitive_pulsesequence_get_element_at_index(pulsesequence, controller->currentStepInPulseSequence)->type == SequenceTypeFID) {
            gtk_button_set_label(window->play_button, "Stop");
            icon_info = gtk_icon_theme_lookup_icon(icon_theme, "insensitive-stop", 16, GTK_ICON_LOOKUP_FORCE_REGULAR);
            gtk_button_set_image(window->play_button, gtk_image_new_from_file(gtk_icon_info_get_filename(icon_info)));
            g_clear_object(&icon_info);
            start_progress_indicator(window);
            insensitive_controller_interrupt_coherencePathway_calculation(controller);
            set_user_controls_enabled(window, FALSE);
            gtk_widget_hide(GTK_WIDGET(window->step_window));
        } else {
            gtk_widget_show(GTK_WIDGET(window->step_window));
        }
        insensitive_controller_perform_next_step_of_pulseSequence(controller);
        update_pulseSequence(window);
    }
}


G_MODULE_EXPORT void on_evolutionTimes_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	InsensitiveController *controller = window->controller;
	int tag;

    if (gtk_combo_box_get_active(combobox) > -1) {
		tag = atoi(gtk_combo_box_get_active_id(combobox));
        insensitive_controller_set_variableEvolutionTime(controller, tag);
		if (tag == 0) {
			gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, FALSE);
			insensitive_controller_set_detectionMethod(controller, None);
		} else {
			gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, TRUE);
		}
		update_pulseSequence(window);
	}
}


G_MODULE_EXPORT void on_detectionMethod_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_detectionMethod(window->controller,
                                               (enum PurePhaseDetectionMethod)gtk_combo_box_get_active(combobox));
    display_pulseProgram_code(window);
}


G_MODULE_EXPORT void on_phaseCycles_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    gtk_entry_set_text(window->phaseCycles_entry, gtk_combo_box_text_get_active_text(combobox));
    on_phaseCycles_entry_activate(window->phaseCycles_entry, window);
}


G_MODULE_EXPORT void on_phaseCycles_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    unsigned int n = atoi(gtk_entry_get_text(entry));
    //gchar *str;

    if (n > 0) {
        insensitive_controller_interrupt_coherencePathway_calculation(window->controller);
        insensitive_controller_add_number_of_phase_cycles(window->controller, n - insensitive_controller_get_numberOfPhaseCycles(window->controller));
        display_pulseProgram_code(window);
    } /*else {
        str = malloc(5 * sizeof(gchar));
        sprintf(str, "%d", insensitive_controller_get_numberOfPhaseCycles(window->controller));
        gtk_entry_set_text(window->phaseCycles_entry, str);
        free(str);
    }*/
}


G_MODULE_EXPORT void on_acquire2DSpectrum_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_pulsesequence_get_last_element(window->controller->pulseSequence)->type == SequenceTypeFID) {
        start_progress_indicator(window);
        insensitive_controller_interrupt_coherencePathway_calculation(window->controller);
        allow_spectrum_acquisition(window, FALSE);
        show_mainWindow_notebook_page(window, 3);
        insensitive_controller_perform_2D_acquisition(window->controller);
    }
}


G_MODULE_EXPORT void on_erase_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    close_coherencePathway(window);
    erase_coherencePathway(window);
    insensitive_controller_erase_pulseSequence(window->controller);
    window->openFileState_for_pulseSequence = NoFile;
    set_openedFileState_for_pulseSequence(window, NoFile, NULL);
}


gboolean perform_open_pulseProgram(InsensitiveWindow *window, xmlNodePtr node)
{
    int variableEvolutionTime;
    SequenceElement *element, *elementFromFile;
    GPtrArray *pulseSequenceArray, *tempArray, *phaseCyclingArray;
    xmlNodePtr iter, array;
    gsize data_len;
	unsigned int i, phaseCycles, purePhaseDetectionMethod;
    gchar *name, *phase;
	xmlChar *version;

    name = NULL;
	for (iter = node; iter; iter = iter->next) {
		if (iter->type == XML_ELEMENT_NODE && !strcmp((char *)iter->name, "key")) {
			if (!strcmp((char *)xmlNodeGetContent(iter), "IPPFileVersion"))
				version = xmlNodeGetContent(iter->next->next);
			else if (!strcmp((char *)xmlNodeGetContent(iter), "Name"))
				name = (char *)xmlNodeGetContent(iter->next->next);
			else if (!strcmp((char *)xmlNodeGetContent(iter), "PulseSequence")) {
                tempArray = g_ptr_array_new();
                array = iter->next->next;
                if (array->type == XML_ELEMENT_NODE && !strcmp((char *)array->name, "array")) {
                    for (array = array->children->next; array; array = array->next->next) {
                        elementFromFile = (SequenceElement *)g_base64_decode((char *)xmlNodeGetContent(array), &data_len);
                        g_ptr_array_add(tempArray, elementFromFile);
                    }
                }
            } else if (!strcmp((char *)xmlNodeGetContent(iter), "VariableEvolutionTime"))
				variableEvolutionTime = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "PhaseCyclingArray")) {
				phaseCyclingArray = g_ptr_array_new();
                array = iter->next->next;
                if (array->type == XML_ELEMENT_NODE && !strcmp((char *)array->name, "array")) {
                    for (array = array->children->next; array; array = array->next->next) {
                        phase = malloc(5 * sizeof(gchar));
                        sprintf(phase, "%.0f", atof((char *)xmlNodeGetContent(array)));
                        g_ptr_array_add(phaseCyclingArray, phase);
                    }
                }
            } else if (!strcmp((char *)xmlNodeGetContent(iter), "PhaseCycles"))
				phaseCycles = atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "PurePhaseDetectionMethod"))
				purePhaseDetectionMethod = atoi((char *)xmlNodeGetContent(iter->next->next));
		}
	}
    if (strcmp((char *)version, "Insensitive Pulse Program 0.9.7") && strcmp((char *)version, "Insensitive Pulse Program 0.9.24")) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                       GTK_DIALOG_DESTROY_WITH_PARENT,
							                       GTK_MESSAGE_ERROR,
							                       GTK_BUTTONS_OK,
							                       "This file does not appear to be in the Insensitive Pulse Program file format.");
	    gtk_window_set_title(GTK_WINDOW(dialog), "Corrupt pulse program file");
	    gtk_dialog_run(GTK_DIALOG(dialog));
	    gtk_widget_destroy(dialog);
        return FALSE;
    }
    insensitive_controller_stop_acquisition(window->controller);
    insensitive_controller_set_isRecordingPulseSequence(window->controller, FALSE);
    //finish_editing_seqence_element(window);)
    erase_coherencePathway(window);
    // Get sequence name if available
    if (name != NULL)
        insensitive_controller_set_name_for_pulseSequence(window->controller, name);
    // Open pulse sequence
    pulseSequenceArray = g_ptr_array_new();
    for (i = 0; i < tempArray->len; i++) {
        elementFromFile = g_ptr_array_index(tempArray, i);
        element = malloc(sizeof(SequenceElement));
        element->type = elementFromFile->type;
        element->time = elementFromFile->time;
        element->secondParameter = elementFromFile->secondParameter;
        element->pulseArray = elementFromFile->pulseArray;
        element->iDecoupling = elementFromFile->iDecoupling;
        element->sDecoupling = elementFromFile->sDecoupling;
        element->activeISpins = elementFromFile->activeISpins;
        element->activeSSpins = elementFromFile->activeSSpins;
        element->selectiveIPulse = elementFromFile->selectiveIPulse;
        element->selectiveSPulse = elementFromFile->selectiveSPulse;
        element->spinlock = elementFromFile->spinlock;
        if (!strcmp((char *)version, "Insensitive Pulse Program 0.9.24")) {
            element->pulseDuration = elementFromFile->pulseDuration;
            element->pulseStrength = elementFromFile->pulseStrength;
            element->pulseFrequency = elementFromFile->pulseFrequency;
            element->pulseEnvelope = elementFromFile->pulseEnvelope;
        } else {
            element->pulseDuration = element->time / 360;
            element->pulseStrength = 1.0;
            element->pulseFrequency = 0.0;
            element->pulseEnvelope = Rectangle;
        }
        g_ptr_array_add(pulseSequenceArray, element);
    }
    window->controller->phaseCycles = 0;
    insensitive_controller_substitute_pulseSequence(window->controller, pulseSequenceArray);
    // Open phase cycling table
    insensitive_controller_substitute_phaseCyclingArray(window->controller, phaseCyclingArray, phaseCycles);
    insensitive_controller_set_variableEvolutionTime(window->controller, variableEvolutionTime);
    if (insensitive_controller_get_variableEvolutionTime(window->controller) == 0) {
        gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, FALSE);
        insensitive_controller_set_detectionMethod(window->controller, None);
    } else {
        gtk_widget_set_sensitive((GtkWidget *)window->detectionMethod_combobox, TRUE);
        insensitive_controller_set_detectionMethod(window->controller, purePhaseDetectionMethod);
    }
    set_recording_button_clicked(window, FALSE);
    close_coherencePathway(window);
    g_ptr_array_free(pulseSequenceArray, TRUE);
    g_ptr_array_free(phaseCyclingArray, TRUE);
    update_pulseSequence(window);

    return TRUE;
}


G_MODULE_EXPORT void perform_save_pulseProgram(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    GtkWidget *chooser;
    char str[12];
    unsigned int i, len;
    gint result;
    gchar *filename, *name, *base64;
    GPtrArray *phaseCyclingArray;
    xmlDoc *doc = NULL;
    xmlNode *root, *first_child, *node;
    GtkFileFilter *filter;

    chooser = gtk_file_chooser_dialog_new("Save Insensitive Pulse Program...",
                                          (GtkWindow *)window,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "Cancel", GTK_RESPONSE_CANCEL,
                                          "Save", GTK_RESPONSE_ACCEPT,
                                          NULL);
    name = insensitive_controller_get_pulseSequence_name(window->controller);
    if(name != NULL && strlen(name) > 0) {
        filename = malloc((strlen(name) + 5) * sizeof(gchar));
        sprintf(filename, "%s.ipp", name);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), filename);
        free(filename);
    } else
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), "pulseprogram.ipp");
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Insensitive pulse programs (IPP)");
    gtk_file_filter_add_pattern(filter, "*.ipp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    gtk_widget_show_all(chooser);
    result = gtk_dialog_run((GtkDialog *)chooser);
    if (result == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
        name = g_path_get_basename(filename);
        len = strlen(name);
        if (!strcmp(name + len - 4, ".ipp"))
            *(name + len - 4) = '\0';
        LIBXML_TEST_VERSION
        doc = xmlNewDoc(BAD_CAST "1.0");
        root = xmlNewNode(NULL, BAD_CAST "plist");
        xmlNewProp(root, (const xmlChar *)"version", (const xmlChar *)"1.0");
        xmlDocSetRootElement(doc, root);
        xmlCreateIntSubset(doc, BAD_CAST "plist", BAD_CAST "-//Apple//DTD PLIST 1.0//EN", BAD_CAST "http://www.apple.com/DTDs/PropertyList-1.0.dtd");
        first_child = xmlNewChild(root, NULL, BAD_CAST "dict", NULL);

        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "IPPFileVersion");
        xmlNewChild(first_child, NULL, BAD_CAST "string", BAD_CAST "Insensitive Pulse Program 0.9.24");
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "Name");
        xmlNewChild(first_child, NULL, BAD_CAST "string", BAD_CAST name);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PhaseCycles");
        sprintf(str, "%d", insensitive_controller_get_numberOfPhaseCycles(window->controller));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PhaseCyclingArray");
        node = xmlNewChild(first_child, NULL, BAD_CAST "array", NULL);
        phaseCyclingArray = insensitive_controller_get_phaseCyclingArray(window->controller);
        for (i = 0; i < phaseCyclingArray->len; i++)
            xmlNewChild(node, NULL, BAD_CAST "real", g_ptr_array_index(phaseCyclingArray, i));
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PulseSequence");
        node = xmlNewChild(first_child, NULL, BAD_CAST "array", NULL);
        for (i = 0; i < (unsigned int)insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence); i++) {
            base64 = g_base64_encode((const guchar *)insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i),
                                     sizeof(SequenceElement));
            xmlNewChild(node, NULL, BAD_CAST "data", BAD_CAST base64);
            g_free(base64);
        }
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PurePhaseDetectionMethod");
        sprintf(str, "%d", insensitive_controller_get_detectionMethod(window->controller));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "VariableEvolutionTime");
        sprintf(str, "%d", insensitive_controller_get_variableEvolutionTime(window->controller));
        xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
        xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlMemoryDump();
        set_openedFileState_for_pulseSequence(window, FileOpened, name);
        g_free(filename);
    }
    g_object_unref(chooser);
}


void set_openedFileState_for_pulseSequence(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename)
{
    if (state == FileOpened && filename != NULL) {
        window->openFileState_for_pulseSequence = state;
    } else if (state == FileOpenedAndChanged && window->openFileState_for_pulseSequence == FileOpened) {
        window->openFileState_for_pulseSequence = state;
        insensitive_controller_set_name_for_pulseSequence(window->controller, NULL);
    } else if (state == NoFile) {
        window->openFileState_for_pulseSequence = state;
        insensitive_controller_set_name_for_pulseSequence(window->controller, NULL);
    }
}


void edit_sequence_element(InsensitiveWindow *window, int index)
{
    InsensitivePulseSequence *pulseSequence = window->controller->pulseSequence;
    gchar *str;
    SequenceElement *elementShift1, *elementShift2, *elementShift3, *elementShift4;

    if(index >= 0 && index < insensitive_pulsesequence_get_number_of_elements(pulseSequence)) {
        window->editedElement = insensitive_pulsesequence_get_element_at_index(pulseSequence, index);
        window->secondHalfOfSandwichElement = NULL;
        switch(window->editedElement->type) {
        case SequenceTypePulse:
            gtk_notebook_set_current_page(window->pp_edit_notebook, 1);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
            gtk_combo_box_set_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox), 1);
            gtk_combo_box_set_active(GTK_COMBO_BOX(window->pp_edit_pulse_shape_combobox), window->editedElement->pulseEnvelope);
            str = malloc(12 * sizeof(gchar));
            sprintf(str, "%.3f", window->editedElement->time);
            gtk_entry_set_text(window->pp_edit_pulse_entry, str);
            free(str);
            break;
        case SequenceTypeShift:
        case SequenceTypeCoupling:
        case SequenceTypeRelaxation:
        case SequenceTypeEvolution:
            gtk_notebook_set_current_page(window->pp_edit_notebook, 2);
            // Check for spin echo sandwich (selected delay before center pulse)
            if(index + 1 == -window->variableEvolutionTime) {
                if((index + 2) < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence)) {
                    elementShift1 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 1);
                    elementShift2 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 2);
                    if(elementShift1->type == SequenceTypePulse && elementShift2->type == SequenceTypeEvolution
                       && elementShift2->time == window->editedElement->time) {
                        window->secondHalfOfSandwichElement = elementShift2;
                    }
                }
                if((index + 4) < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence)) {
                    elementShift1 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 1);
                    elementShift2 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 2);
                    elementShift3 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 3);
                    elementShift4 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index + 4);
                    if(elementShift1->type == SequenceTypeGradient && elementShift2->type == SequenceTypePulse
                       && elementShift3->type == SequenceTypeGradient && elementShift4->type == SequenceTypeEvolution
                       && elementShift4->time == window->editedElement->time) {
                        window->secondHalfOfSandwichElement = elementShift4;
                    }
                }
            // Check for spin echo sandwich (selected delay after center pulse)
            } else if(index - 1 == -window->variableEvolutionTime) {
                if((index - 2) >= 0) {
                    elementShift1 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 1);
                    elementShift2 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 2);
                    if(elementShift1->type == SequenceTypePulse && elementShift2->type == SequenceTypeEvolution
                       && elementShift2->time == window->editedElement->time) {
                        window->secondHalfOfSandwichElement = elementShift2;
                    }
                }
            } else if(index - 3 == -window->variableEvolutionTime) {
                if((index - 4) >= 0) {
                    elementShift1 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 1);
                    elementShift2 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 2);
                    elementShift3 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 3);
                    elementShift4 = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index - 4);
                    if(elementShift1->type == SequenceTypeGradient && elementShift2->type == SequenceTypePulse
                       && elementShift3->type == SequenceTypeGradient && elementShift4->type == SequenceTypeEvolution
                       && elementShift4->time == window->editedElement->time) {
                        window->secondHalfOfSandwichElement = elementShift4;
                    }
                }
            }
            str = malloc(12 * sizeof(gchar));
            sprintf(str, "%.4f", window->editedElement->time);
            gtk_entry_set_text(window->pp_edit_delay_entry, str);
            free(str);
            break;
        case SequenceTypeGradient:
            gtk_notebook_set_current_page(window->pp_edit_notebook, 3);
            gtk_combo_box_set_active(GTK_COMBO_BOX(window->pp_edit_gradient_combobox), 0);
            str = malloc(12 * sizeof(gchar));
            sprintf(str, "%.0f", window->editedElement->secondParameter);
            gtk_entry_set_text(window->pp_edit_gradient_entry, str);
            free(str);
            break;
        case SequenceTypeFID:
            gtk_notebook_set_current_page(window->pp_edit_notebook, 4);
            gtk_combo_box_set_active(GTK_COMBO_BOX(window->pp_edit_fid_combobox), 0);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idetect_radiobutton), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdetect_radiobutton), TRUE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idecoupling_checkbox), FALSE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdecoupling_checkbox), FALSE);
            gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_spinlock_checkbox), FALSE);
            if (window->editedElement->activeSSpins)
                gtk_toggle_button_set_active(window->pp_edit_fid_sdetect_radiobutton, TRUE);
            else
                gtk_toggle_button_set_active(window->pp_edit_fid_idetect_radiobutton, TRUE);
            break;
        }
        update_pulseSequence(window);
    }
}


G_MODULE_EXPORT void on_pp_edit_pulse_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *str;

    switch(gtk_combo_box_get_active(combobox)) {
    case 0: // Shape
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        gtk_combo_box_set_active(GTK_COMBO_BOX(window->pp_edit_pulse_shape_combobox), window->editedElement->pulseEnvelope);
        break;
    case 2: // Strength
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.3f", window->editedElement->pulseStrength);
        gtk_entry_set_text(window->pp_edit_pulse_entry, str);
        free(str);
        break;
    case 3: // Duration
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.3f", window->editedElement->pulseDuration);
        gtk_entry_set_text(window->pp_edit_pulse_entry, str);
        free(str);
        break;
    case 4: // Frequency
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.3f", window->editedElement->pulseFrequency);
        gtk_entry_set_text(window->pp_edit_pulse_entry, str);
        free(str);
        break;
    case 5: // Decoupling
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), TRUE);
        if (window->editedElement->activeISpins) {
            gtk_widget_set_sensitive(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), TRUE);
        } else if (window->editedElement->activeSSpins) {
            gtk_widget_set_sensitive(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        }
        gtk_toggle_button_set_active(window->pp_edit_pulse_idecoupling_checkbox, window->editedElement->iDecoupling);
        gtk_toggle_button_set_active(window->pp_edit_pulse_sdecoupling_checkbox, window->editedElement->sDecoupling);
        break;
    default: // Flip angle
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_shape_combobox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_pulse_sdecoupling_checkbox), FALSE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.3f", window->editedElement->time);
        gtk_entry_set_text(window->pp_edit_pulse_entry, str);
        free(str);
    }
}


G_MODULE_EXPORT void on_pp_edit_delay_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *str;

    switch(gtk_combo_box_get_active(combobox)) {
    case 1: // Decoupling
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_entry), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_idecoupling_checkbox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_sdecoupling_checkbox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_spinlock_checkbox), TRUE);
        gtk_toggle_button_set_active(window->pp_edit_delay_idecoupling_checkbox, window->editedElement->iDecoupling);
        gtk_toggle_button_set_active(window->pp_edit_delay_sdecoupling_checkbox, window->editedElement->sDecoupling);
        gtk_toggle_button_set_active(window->pp_edit_delay_spinlock_checkbox, window->editedElement->spinlock);
        break;
    default: // Delay time
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_entry), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_sdecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_delay_spinlock_checkbox), FALSE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.4f", window->editedElement->time);
        gtk_entry_set_text(window->pp_edit_delay_entry, str);
        free(str);
    }
}


G_MODULE_EXPORT void on_pp_edit_gradient_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *str;

    switch(gtk_combo_box_get_active(combobox)) {
    case 1: // Duration
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_gradient_entry), TRUE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.2f", window->editedElement->time);
        gtk_entry_set_text(window->pp_edit_gradient_entry, str);
        free(str);
        break;
    default: // Strength
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_gradient_entry), TRUE);
        str = malloc(12 * sizeof(gchar));
        sprintf(str, "%.0f", window->editedElement->secondParameter);
        gtk_entry_set_text(window->pp_edit_gradient_entry, str);
        free(str);
    }
}


G_MODULE_EXPORT void on_pp_edit_fid_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    switch(gtk_combo_box_get_active(combobox)) {
    case 1: // Decoupling
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idetect_radiobutton), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdetect_radiobutton), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idecoupling_checkbox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdecoupling_checkbox), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_spinlock_checkbox), TRUE);
        gtk_toggle_button_set_active(window->pp_edit_fid_idecoupling_checkbox, window->editedElement->iDecoupling);
        gtk_toggle_button_set_active(window->pp_edit_fid_sdecoupling_checkbox, window->editedElement->sDecoupling);
        gtk_toggle_button_set_active(window->pp_edit_fid_spinlock_checkbox, window->editedElement->spinlock);
        break;
    default: // Detection
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idetect_radiobutton), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdetect_radiobutton), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_idecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_sdecoupling_checkbox), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(window->pp_edit_fid_spinlock_checkbox), FALSE);
        gtk_toggle_button_set_active(window->pp_edit_fid_idetect_radiobutton, window->editedElement->activeISpins);
        gtk_toggle_button_set_active(window->pp_edit_fid_sdetect_radiobutton, window->editedElement->activeSSpins);
    }
}


G_MODULE_EXPORT void on_editing_pulsesequence_finished(gpointer sender, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    float value, temp;
    int i;
    SequenceElement *currentElement;

    if (window->editedElement != NULL && gtk_notebook_get_current_page(window->pp_edit_notebook) > 0) {
        switch(window->editedElement->type) {
        case SequenceTypePulse:
            // Flip angle
            if(gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 1) {
                value = atof(gtk_entry_get_text(window->pp_edit_pulse_entry));
                if (value <= 0 ) {
                    window->editedElement->time = 0.0;
                    window->editedElement->pulseDuration = 0.0;
                } else {
                    temp = window->editedElement->time;
                    window->editedElement->time = value;
                    if (window->editedElement->time >= 360.0)
                        window->editedElement->time = 360.0;
                    if (window->editedElement->pulseDuration == 0.0)
                        window->editedElement->pulseDuration = window->editedElement->time / (360 * window->editedElement->pulseStrength);
                    else {
                        temp /= window->editedElement->time;
                        window->editedElement->pulseDuration /= temp;
                        if (window->editedElement->pulseDuration < 0.001) {
                            temp = 0.001 * window->editedElement->pulseDuration;
                            window->editedElement->pulseDuration = 0.001;
                            window->editedElement->pulseStrength *= temp;
                        }
                    }
                }
            // Duration
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 3) {
                value = atof(gtk_entry_get_text(window->pp_edit_pulse_entry));
                temp = window->editedElement->pulseDuration;
                if (value > 1000.0)
                    window->editedElement->pulseDuration = 1000.0;
                else if (value < 0.001) {
                    window->editedElement->pulseDuration = 0.0;
                    window->editedElement->time = 0.0;
                    window->editedElement->pulseStrength = 1.0;
                } else
                    window->editedElement->pulseDuration = value;
                if (window->editedElement->pulseDuration >= 0.001) {
                    temp /= window->editedElement->pulseDuration;
                    window->editedElement->time /= temp;
                    if (window->editedElement->time > 360) {
                        temp = window->editedElement->time;
                        while (window->editedElement->time > 360) {
                            window->editedElement->time -= 360;
                        }
                        temp = window->editedElement->time / temp;
                        window->editedElement->pulseStrength *= temp;
                    }
                }
            // Strength
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 2) {
                value = atof(gtk_entry_get_text(window->pp_edit_pulse_entry));
                temp = window->editedElement->pulseStrength;
                if (value > 1000.0)
                    window->editedElement->pulseStrength = 1000.0;
                else if (value < 0.001) {
                    window->editedElement->pulseDuration = 0.0;
                    window->editedElement->time = 0.0;
                    window->editedElement->pulseStrength = 1.0;
                } else
                    window->editedElement->pulseStrength = value;
                if (window->editedElement->pulseStrength >= 0.001 && window->editedElement->time > 0.0) {
                    temp = window->editedElement->pulseStrength / temp;
                    window->editedElement->time *= temp;
                    if (window->editedElement->time > 360) {
                        temp = window->editedElement->time;
                        while (window->editedElement->time > 360) {
                            window->editedElement->time -= 360;
                        }
                        temp = window->editedElement->time / temp;
                        window->editedElement->pulseDuration *= temp;
                    }
                    if (window->editedElement->pulseDuration < 0.002) {
                        window->editedElement->pulseDuration = 0.001;
                        window->editedElement->time = window->editedElement->pulseDuration * window->editedElement->pulseStrength * 360;
                    }
                }
            // Frequency
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 4) {
                value = atof(gtk_entry_get_text(window->pp_edit_pulse_entry));
                window->editedElement->pulseFrequency = value;
                if (window->editedElement->pulseFrequency > 127.0)
                    window->editedElement->pulseFrequency = 127.0;
                else if (window->editedElement->pulseFrequency < -127.0)
                    window->editedElement->pulseFrequency = -127.0;
            // Shape
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 0) {
                window->editedElement->pulseEnvelope = (enum PulseEnvelope)gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_shape_combobox));
            // Decoupling
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_pulse_combobox)) == 5) {
                window->editedElement->iDecoupling = gtk_toggle_button_get_active(window->pp_edit_pulse_idecoupling_checkbox);
                window->editedElement->sDecoupling = gtk_toggle_button_get_active(window->pp_edit_pulse_sdecoupling_checkbox);
                if (window->editedElement->spinlock) {
                    for (i = 0; i < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence); i++) {
                        currentElement = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i);
                        if (currentElement->activeISpins || currentElement->iDecoupling)
                            window->editedElement->iDecoupling = TRUE;
                        if (currentElement->activeSSpins || currentElement->sDecoupling)
                            window->editedElement->sDecoupling = TRUE;
                    }
                }
            }
            break;
        case SequenceTypeShift:
        case SequenceTypeCoupling:
        case SequenceTypeRelaxation:
        case SequenceTypeEvolution:
            // Delay time
            if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_delay_combobox)) == 0) {
                value = atof(gtk_entry_get_text(window->pp_edit_delay_entry));
                if (window->editedElement->time > 0) {
                    window->editedElement->time = value;
                    if (window->secondHalfOfSandwichElement != NULL) {
                        window->secondHalfOfSandwichElement->time = window->editedElement->time;
                        window->secondHalfOfSandwichElement = NULL;
                    }
                    if (window->variableEvolutionTime >= 0) {
                        update_evolutionTimes_combobox(window);
                        //set_variable_evolution_time(window, window->variableEvolutionTime);
                    }
                }
            // Decoupling
            } else if (gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_delay_combobox)) == 1) {
                window->editedElement->iDecoupling = gtk_toggle_button_get_active(window->pp_edit_delay_idecoupling_checkbox);
                window->editedElement->sDecoupling = gtk_toggle_button_get_active(window->pp_edit_delay_sdecoupling_checkbox);
                window->editedElement->spinlock = gtk_toggle_button_get_active(window->pp_edit_delay_spinlock_checkbox);
                if (window->editedElement->spinlock) {
                    for (i = 0; i < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence); i++) {
                        currentElement = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i);
                        if (currentElement->activeISpins || currentElement->iDecoupling)
                            window->editedElement->iDecoupling = TRUE;
                        if (currentElement->activeSSpins || currentElement->sDecoupling)
                            window->editedElement->sDecoupling = TRUE;
                    }
                }
            }
            break;
        case SequenceTypeGradient:
            value = atof(gtk_entry_get_text(window->pp_edit_gradient_entry));
            // Strength
            if(gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_gradient_combobox)) == 0) {
                if(value < -32000)
                    window->editedElement->secondParameter = -32000;
                else if(value > 32000)
                    window->editedElement->secondParameter = 32000;
                else
                    window->editedElement->secondParameter = value;
            // Duration
            } else if(gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_gradient_combobox)) == 1) {
                if(value <= 0)
                    window->editedElement->time = 1;
                else
                    window->editedElement->time = value;
            }
            break;
        case SequenceTypeFID:
            // Detection
            if(gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_fid_combobox)) == 0) {
                if(gtk_toggle_button_get_active(window->pp_edit_fid_sdetect_radiobutton)) {
                    window->editedElement->activeISpins = FALSE;
                    window->editedElement->activeSSpins = TRUE;
                } else {
                    window->editedElement->activeISpins = TRUE;
                    window->editedElement->activeSSpins = FALSE;
                }
            // Decoupling
            } else if(gtk_combo_box_get_active(GTK_COMBO_BOX(window->pp_edit_fid_combobox)) == 1) {
                window->editedElement->iDecoupling = gtk_toggle_button_get_active(window->pp_edit_fid_idecoupling_checkbox);
                window->editedElement->sDecoupling = gtk_toggle_button_get_active(window->pp_edit_fid_sdecoupling_checkbox);
                window->editedElement->spinlock = gtk_toggle_button_get_active(window->pp_edit_fid_spinlock_checkbox);
                if (window->editedElement->spinlock) {
                    for (i = 0; i < insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence); i++) {
                        currentElement = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i);
                        if (currentElement->activeISpins || currentElement->iDecoupling)
                            window->editedElement->iDecoupling = TRUE;
                        if (currentElement->activeSSpins || currentElement->sDecoupling)
                            window->editedElement->sDecoupling = TRUE;
                    }
                }
            }
            break;
        }
        set_openedFileState_for_pulseSequence(window, FileOpenedAndChanged, NULL);
        cancel_editing_sequence_element(window);
        display_pulseProgram_code(window);
        erase_coherencePathway(window);
        close_coherencePathway(window);
    }
}



void cancel_editing_sequence_element(InsensitiveWindow *window)
{
    if(window->editedElement != NULL && gtk_notebook_get_current_page(window->pp_edit_notebook) != 0) {
        gtk_notebook_set_current_page(window->pp_edit_notebook, 0);
        window->editedElement = NULL;
        update_pulseSequence(window);
    }
}


void erase_coherencePathway(InsensitiveWindow *window)
{
    close_coherencePathway(window);
    window->needsToRecalculateCoherencePathways = TRUE;
    set_iSpin_coherencePathway_coefficients(window, NULL); // Should set numberOfPulses to 0 as well.
    set_sSpin_coherencePathway_coefficients(window, NULL); // Should set numberOfPulses to 0 as well.
}


void set_iSpin_coherencePathway_coefficients(InsensitiveWindow *window, DSPComplex *intensities)
{
    int i, j, orders, order, step, index;
    unsigned int spins = insensitive_spinsystem_get_number_of_ispins(window->controller->spinSystem);
    unsigned int pulses = window->controller->pulseList->len;

    if(spins == 0)
        orders = 1;
    else if(intensities == NULL)
        orders = 0;
    else
        orders = 2 * spins + 1;

    if(window->coefficientsForISpins != NULL) {
        free(window->coefficientsForISpins);
        window->coefficientsForISpins = NULL;
    }
    if(window->pathwaysForISpins != NULL) {
        for(i = 0; i < window->numberOfISpinPathways; i++)
            free(window->pathwaysForISpins[i]);
        free(window->pathwaysForISpins);
        window->pathwaysForISpins = NULL;
    }
    window->numberOfISpinOrders = orders;
    window->numberOfPulses = pulses;
    if(pulses == 0)
        window->errorCode |= 1;
    else
        window->errorCode &= ~1;
    window->numberOfISpinPathways = pow(orders, pulses);
    if (intensities != NULL && orders > 1) {
        window->maxCoefficientForISpins = 0.000001;
        window->pathwaysForISpins = malloc(window->numberOfISpinPathways * sizeof(int *));
        window->coefficientsForISpins = malloc(window->numberOfISpinPathways * sizeof(float));
        for (i = 0; i < window->numberOfISpinPathways; i++) {
            window->pathwaysForISpins[i] = malloc(pulses * sizeof(int));
            window->coefficientsForISpins[i] = fabsf(hypotf(intensities[i].real, intensities[i].imag));
            if (window->coefficientsForISpins[i] > window->maxCoefficientForISpins)
                window->maxCoefficientForISpins = window->coefficientsForISpins[i];
        }
        // Create pathway table
        for (step = 0; step < window->numberOfPulses; step++) {
            index = 0;
            for (j = pow(window->numberOfISpinOrders, window->numberOfPulses - step - 1); j > 0; j--) {
                for (order = 0; order < window->numberOfISpinOrders; order++) {
                    for (i = 0; i < pow(window->numberOfISpinOrders, step); i++) {
                        window->pathwaysForISpins[index++][window->numberOfPulses - 1 - step] = order;
                    }
                }
            }
        }
        window->errorCode &= ~2;
    } else {
        window->errorCode |= 2;
    }
}


void set_sSpin_coherencePathway_coefficients(InsensitiveWindow *window, DSPComplex *intensities)
{
    int i, j, orders, order, step, index;
    unsigned int spins = insensitive_spinsystem_get_number_of_sspins(window->controller->spinSystem);
    unsigned int pulses = window->controller->pulseList->len;

    if(spins == 0)
        orders = 1;
    else if(intensities == NULL)
        orders = 0;
    else
        orders = 2 * spins + 1;

    if (window->coefficientsForSSpins != NULL) {
        free(window->coefficientsForSSpins);
        window->coefficientsForSSpins = NULL;
    }
    if (window->pathwaysForSSpins != NULL) {
        for (i = 0; i < window->numberOfSSpinPathways; i++)
            free(window->pathwaysForSSpins[i]);
        free(window->pathwaysForSSpins);
        window->pathwaysForSSpins = NULL;
    }
    window->numberOfSSpinOrders = orders;
    window->numberOfPulses = pulses;
    if (pulses == 0)
        window->errorCode |= 4;
    else
        window->errorCode &= ~4;
    window->numberOfSSpinPathways = pow(orders, pulses);
    if (intensities != NULL && orders > 1) {
        window->maxCoefficientForSSpins = 0.000001;
        window->pathwaysForSSpins = malloc(window->numberOfSSpinPathways * sizeof(int *));
        window->coefficientsForSSpins = malloc(window->numberOfSSpinPathways * sizeof(float));
        for (i = 0; i < window->numberOfSSpinPathways; i++) {
            window->pathwaysForSSpins[i] = malloc(pulses * sizeof(int));
            window->coefficientsForSSpins[i] = fabsf(hypotf(intensities[i].real, intensities[i].imag));
            if(window->coefficientsForSSpins[i] > window->maxCoefficientForSSpins)
                window->maxCoefficientForSSpins = window->coefficientsForSSpins[i];
        }
        // Create pathway table
        for (step = 0; step < window->numberOfPulses; step++) {
            index = 0;
            for (j = pow(window->numberOfSSpinOrders, window->numberOfPulses - step - 1); j > 0; j--) {
                for (order = 0; order < window->numberOfSSpinOrders; order++) {
                    for (i = 0; i < pow(window->numberOfSSpinOrders, step); i++) {
                        window->pathwaysForSSpins[index++][window->numberOfPulses - 1 - step] = order;
                    }
                }
            }
        }
        window->errorCode &= ~8;
    } else {
        window->errorCode |= 8;
    }
}


void set_needsToRecalculateCoherencePathways(InsensitiveWindow *window, gboolean value)
{
    window->needsToRecalculateCoherencePathways = value;
}


void draw_coherencePathway(InsensitiveWindow *window)
{
    g_thread_new("CoherencePathwayThread", calculate_coherence_paths, window);
    //[coherenceProgressIndicator performSelectorOnMainThread:@selector(stopAnimation:) withObject:self waitUntilDone:FALSE];
}


gpointer calculate_coherence_paths(gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int i, spins, pulseIndex, step, p;
	unsigned int type = 0;
    int **pathways, numberOfOrders;
    long numberOfPathways;
    float current_Y, current_X, *coefficients, *alpha, maxCoefficient, offset, shiftFromText;
    InsensitivePulseSequence *pulseSequence = window->controller->pulseSequence;
    int numberOfElements = insensitive_pulsesequence_get_number_of_elements(pulseSequence);
    SequenceElement *currentElement;
    float complex *tuple;
    GPtrArray *path;
    gchar *label;

    if (window->labelArray != NULL) {
        g_ptr_array_free(window->labelArray, TRUE);
    }
    window->labelArray = g_ptr_array_new();
    //g_ptr_array_remove_range(window->labelArray, 0, window->labelArray->len);
    if (window->alphaArray != NULL) {
        g_ptr_array_free(window->alphaArray, TRUE);
    }
    window->pathArray = g_ptr_array_new();
    //g_ptr_array_remove_range(window->alphaArray, 0, window->labelArray->len);
    if (window->pathArray != NULL)
        while (window->pathArray->len > 0) {
            path = g_ptr_array_remove_index_fast(window->pathArray, 0);
            g_ptr_array_free(path, TRUE);
        }
    window->alphaArray = g_ptr_array_new();

    shiftFromText = 27.0;
    for (type = 0; type < 2; type++) {
        current_X = 60 * window->pathway_scaling;
        if (type == spinTypeS) {
            coefficients = window->coefficientsForSSpins;
            pathways = window->pathwaysForSSpins;
            numberOfPathways = window->numberOfSSpinPathways;
            numberOfOrders = window->numberOfSSpinOrders;
            maxCoefficient = window->maxCoefficientForSSpins;
            if (window->numberOfISpinOrders == 0) // Memory overflow (coefficients are NULL pointer)
                offset = 32;
            else if (window->numberOfISpinOrders == 1) // No I spins present
                offset = 0.0;
            else // I spins present and pathway calculated
                offset = 32 + window->numberOfISpinOrders * 15;
        } else if (type == spinTypeI) {
            coefficients = window->coefficientsForISpins;
            pathways = window->pathwaysForISpins;
            numberOfPathways = window->numberOfISpinPathways;
            numberOfOrders = window->numberOfISpinOrders;
            maxCoefficient = window->maxCoefficientForISpins;
            offset = 0.0;
        }
        if (coefficients != NULL && numberOfOrders > 1 && window->numberOfPulses > 0) {
            spins = (numberOfOrders - 1) / 2;
            for (i = 0; i < numberOfOrders; i++) {
                current_Y = ((offset + shiftFromText + i * 15) * window->pathway_scaling) + 4 * window->pathway_scaling;
                tuple = malloc(sizeof(float complex));
                *tuple = 25 + I * current_Y;
                g_ptr_array_add(window->labelArray, tuple);
                label = malloc(11 * sizeof(gchar));
                sprintf(label, "%d", -i + spins);
                g_ptr_array_add(window->labelArray, label);
            }
            current_Y = (offset + shiftFromText + spins * 15) * window->pathway_scaling;
            for (p = 0; p < numberOfPathways; p++) {
                path = g_ptr_array_new();
                tuple = malloc(sizeof(float complex));
                *tuple = (30 * window->pathway_scaling) + I * current_Y;
                g_ptr_array_add(path, tuple);
                tuple = malloc(sizeof(float complex));
                *tuple = current_X + I * current_Y;
                g_ptr_array_add(path, tuple);
                g_ptr_array_add(window->pathArray, path);
            }

            pulseIndex = -1;
            if (pulseSequence != NULL && numberOfElements > 0) {
                window->showOnlyNCoherences = FALSE;
                currentElement = insensitive_pulsesequence_get_last_element(pulseSequence);
                if(currentElement->type == SequenceTypeFID) {
                    if((currentElement->activeISpins && type == spinTypeI)
                       || (currentElement->activeSSpins && type == spinTypeS))
                        window->showOnlyNCoherences = TRUE;
                }
                // For each step of the pulse sequence
                for (step = 0; step < numberOfElements; step++) {
                    currentElement = insensitive_pulsesequence_get_element_at_index(pulseSequence, step);
                    switch (currentElement->type) {
                    case SequenceTypePulse:
                        current_X += currentElement->time / 360 * 50 * window->pathway_scaling;
                        pulseIndex++;
                        break;
                    case SequenceTypeEvolution:
                        current_X += currentElement->time * 100 * window->pathway_scaling;
                        break;
                    case SequenceTypeGradient:
                        current_X += currentElement->time * 30 * window->pathway_scaling;
                        break;
                    case SequenceTypeFID:
                        current_X += 100 * window->pathway_scaling;
                        break;
                    default:
                        current_X += 0;
                    }
                    // For each pathway with a coefficient ≠ 0 (and last coherence order = -1)
                    for (p = 0; p < numberOfPathways; p++) {
                        if (coefficients[p] > 0 && (pathways[p][window->numberOfPulses - 1] == spins - 1 || !window->showOnlyNCoherences)) {
                            for (i = 0; i < numberOfOrders; i++) {
                                // Determine whether it's a pulse
                                if (currentElement->type == SequenceTypePulse) {
                                    current_Y = offset + shiftFromText + (numberOfOrders- 1 - pathways[p][pulseIndex]) * 15;
                                } else {
                                    current_Y = offset + shiftFromText + (numberOfOrders - 1 - ((pulseIndex == -1) ? spins : pathways[p][pulseIndex])) * 15;
                                }
                                current_Y *= window->pathway_scaling;
                                if(type == spinTypeS && window->pathwaysForISpins != NULL) {
                                    path = g_ptr_array_index(window->pathArray, p + window->numberOfISpinPathways + window->numberOfISpinOrders);
                                    tuple = malloc(sizeof(float complex));
                                    *tuple = current_X + I * current_Y;
                                    g_ptr_array_add(path, tuple);
                                } else {
                                    path = g_ptr_array_index(window->pathArray, p);
                                    tuple = malloc(sizeof(float complex));
                                    *tuple = current_X + I * current_Y;
                                    g_ptr_array_add(path, tuple);
                                }
                            }
                        }
                        if (step == numberOfElements - 1) {
                            alpha = malloc(sizeof(float));
                            if(coefficients[p] / maxCoefficient < 1e-2)
                                *alpha = 0.0;
                            else
                                *alpha = coefficients[p] / maxCoefficient;
                            g_ptr_array_add(window->alphaArray, alpha);
                        }
                    }
                }
            }
            // Draw line guides in gray for step width determined from pulse sequence
            for (i = 0; i < numberOfOrders; i++) {
                current_Y = (offset + shiftFromText + i * 15) * window->pathway_scaling;
                path = g_ptr_array_new();
                tuple = malloc(sizeof(float complex));
                *tuple = (30 * window->pathway_scaling) + I * current_Y;
                g_ptr_array_add(path, tuple);
                tuple = malloc(sizeof(float complex));
                *tuple = current_X + I * current_Y;
                g_ptr_array_add(path, tuple);
                g_ptr_array_add(window->pathArray, path);
                alpha = malloc(sizeof(float));
                *alpha = 0.25;
                g_ptr_array_add(window->alphaArray, alpha);
            }
        } else {
            if (numberOfOrders == 0 && !(window->errorCode & 1) && !(window->errorCode & 4)) {
                spins = (numberOfOrders - 1) / 2;
                /*label = [[NSTextField alloc] initWithFrame:NSMakeRect(15 * pathway_scaling, (offset + 16 + numberOfOrders * 15) * pathway_scaling, 500 * pathway_scaling, 17 * pathway_scaling)];
                [label setBordered:NO];
                [label setSelectable:NO];
                [label setTextColor:[NSColor blackColor]];
                [label setBackgroundColor:[NSColor windowBackgroundColor]];
                [label setDrawsBackground:NO];
                [label setAlignment:NSLeftTextAlignment];
                [label setFont:[NSFont fontWithName:@"LucidaGrande-Bold" size:12 * pathway_scaling]];
                [label setStringValue:[NSString stringWithFormat:NSLocalizedString(@"Too many calculations required for %c Spins!", nil), (type == 1) ? 'S' : 'I']];
                [labelArray addObject:label];
                [self addSubview:label];*/
            }
        }
    }
    // Delete all pathways with zero coefficient to speed up the display
    if(window->alphaArray != NULL && window->pathArray != NULL)
        if(window->alphaArray->len == window->pathArray->len) {
            maxCoefficient = window->maxCoefficientForISpins;
            for (i = window->alphaArray->len - 1; i >= 0; i--) {
                if(i == window->numberOfISpinPathways + window->numberOfISpinOrders - 1)
                    maxCoefficient = window->maxCoefficientForISpins;
                alpha = g_ptr_array_index(window->alphaArray, i);
                if(*alpha < maxCoefficient * 1e-4) {
                    g_ptr_array_remove_index(window->alphaArray, i);
                    g_ptr_array_remove_index(window->pathArray, i);
                }
            }
        }
    gtk_widget_queue_draw((GtkWidget *)window->coherencePathway_drawingarea);

	return user_data;
}


/////// //////  ///////  ////// //////// //////  //    // ///    ///
//      //   // //      //         //    //   // //    // ////  ////
/////// //////  /////   //         //    //////  //    // // //// //
     // //      //      //         //    //   // //    // //  //  //
/////// //      ///////  //////    //    //   //  //////  //      //

/*- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSPreferredScrollerStyleDidChangeNotification
                                                  object:nil];

    [controller release];
    [super dealloc];
}*/


gboolean shows_2D_spectrum(InsensitiveWindow *window)
{
    return window->twoDimensionalSpectrum;
}


gboolean get_showsFrequencyDomain(InsensitiveWindow *window)
{
    return window->showsFrequencyDomain;
}


/*- (IBAction)showPanel:(id)sender
{
    [fftPanel setHidden:[sender tag] != 0];
    [apodisationPanel setHidden:([sender tag] != 1)];
    [displayPanel setHidden:[sender tag] != 2];
    [phasePanel setHidden:[sender tag] != 3];
}*/


void set_2D_mode(InsensitiveWindow *window, gboolean value)
{
    if (value) {
        window->twoDimensionalSpectrum = TRUE;
        gtk_button_set_label(window->fid_button, "SER");
        gtk_button_set_label(window->fft1D_button, "FFT along t₂");
        gtk_widget_set_visible((GtkWidget *)window->fft2D_button, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->noiseLevelSpectrum_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->noiseLevelSpectrum_entry, FALSE);
        gtk_widget_set_sensitive((GtkWidget *)window->integral_checkbox, FALSE);
        gtk_toggle_button_set_active(window->integral_checkbox, FALSE);
        gtk_widget_set_sensitive((GtkWidget *)window->window_checkbox, FALSE);
        gtk_widget_set_sensitive((GtkWidget *)window->shiftBaseline_checkbox, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->plotStyle_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->plotStyle_combobox, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->lineWidth_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->lineWidth_combobox, FALSE);
        gtk_widget_set_sensitive((GtkWidget *)window->lineWidth_combobox, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->tilt_menubutton, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->symmetrize_menubutton, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->dosyToolbox_button, TRUE);
        if(insensitive_controller_get_detectionMethod(window->controller) != None) {
            gtk_widget_set_visible((GtkWidget *)window->absDispReal_radiobutton, TRUE);
            gtk_widget_set_visible((GtkWidget *)window->absDispImag_radiobutton, TRUE);
            gtk_widget_set_visible((GtkWidget *)window->dataSet_label, TRUE);
        } else {
            gtk_widget_set_visible((GtkWidget *)window->absDispReal_radiobutton, FALSE);
            gtk_widget_set_visible((GtkWidget *)window->absDispImag_radiobutton, FALSE);
            gtk_widget_set_visible((GtkWidget *)window->dataSet_label, FALSE);
        }
        window->scaling = 100;
    } else {
        window->twoDimensionalSpectrum = FALSE;
        window->shows2DFrequencyDomain = FALSE;
        gtk_button_set_label(window->fid_button, "FID");
        gtk_button_set_label(window->fft1D_button, "FFT");
        gtk_widget_set_visible((GtkWidget *)window->fft2D_button, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->noiseLevelSpectrum_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->noiseLevelSpectrum_entry, TRUE);
        gtk_widget_set_sensitive((GtkWidget *)window->integral_checkbox, TRUE);
        gtk_toggle_button_set_active(window->integral_checkbox,
                                     insensitive_settings_get_showIntegral(window->controller->settings));
        gtk_widget_set_sensitive((GtkWidget *)window->window_checkbox, TRUE);
        gtk_widget_set_sensitive((GtkWidget *)window->shiftBaseline_checkbox, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->absDispReal_radiobutton, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->absDispImag_radiobutton, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->dataSet_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->plotStyle_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->plotStyle_combobox, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->lineWidth_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->lineWidth_combobox, TRUE);
        gtk_widget_set_sensitive((GtkWidget *)window->lineWidth_combobox, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->tilt_menubutton, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->symmetrize_menubutton, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->dosyToolbox_button, FALSE);
        gtk_widget_hide((GtkWidget *)window->dosyToolBox_window);
    }
}


void go_to_fft_panel(InsensitiveWindow *window)
{
    gtk_stack_set_visible_child(window->spectrum_tools_stack, window->fourier_stack_child);
}


void set_indirect_dataPoints(InsensitiveWindow *window, int value)
{
    window->indirectDataPoints = value;
}


void enable_fft_along_t1(InsensitiveWindow *window, gboolean value)
{
    gtk_widget_set_sensitive((GtkWidget *)window->fid_button, value);
    gtk_widget_set_sensitive((GtkWidget *)window->fft1D_button, value);
}


void enable_fft_along_t2(InsensitiveWindow *window, gboolean value)
{
    gtk_widget_set_sensitive((GtkWidget *)window->fft2D_button, value);
}


void enable_symmerization(InsensitiveWindow *window, gboolean value)
{
    gtk_widget_set_sensitive((GtkWidget *)window->magnitude_button, value);
}


void reset_spectrum_display(InsensitiveWindow *window)
{
    if(!window->twoDimensionalSpectrum)
        set_phase_correction(window, 0.0, 0.0, 0.0);
    reset_magnification(window);
    window->spectrumIsDOSY2D = FALSE;
    gtk_adjustment_set_value(window->zeroOrder_adjustment, 0.0);
    gtk_adjustment_set_value(window->firstOrder_adjustment, 0.0);
    gtk_adjustment_set_value(window->pivotPoint_adjustment, 0.0);
    //[self setOpenedFileState:NoFile withName:nil];
    set_display_frequency_domain(window, FALSE);
    reset_dosy_panel(window);
    show_spectrumParameters_textview(window, FALSE);
}


void reset_window_function(InsensitiveWindow *window)
{
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, FALSE);
    gtk_combo_box_set_active(window->apodization_combobox, 0);
    insensitive_controller_set_windowFunction(window->controller, WFNone);
}


void set_1D_dataPoints(InsensitiveWindow *window, unsigned int points, unsigned int max)
{

    window->lastDataPointDisplayed = 0;
    set_maxDataPoints(window, max);
}


void set_complex_spectrum(InsensitiveWindow *window, DSPSplitComplex data, unsigned int points, unsigned int max)
{
    unsigned int i;
    unsigned int total = window->twoDimensionalSpectrum ? max : points;
    float integralMaximum;
    int spins = insensitive_spinsystem_get_spins(window->controller->spinSystem);

    integralMaximum = (spins - (0.8 * insensitive_spinsystem_get_number_of_sspins(window->controller->spinSystem))) / spins;
    window->integralMaximum = 1 / integralMaximum;
    window->spectrumIsDOSY2D = FALSE;
    set_maxDataPoints(window, max);
    window->lastDataPointDisplayed = points;
    if(window->autoMaximize)
        window->maxOrdinateValue = 0.0;
    for (i = 0; i < total; i++) {
        window->data.realp[i] = data.realp[i];
        window->data.imagp[i] = data.imagp[i];
        if(window->autoMaximize) {
            if(fabsf(window->data.realp[i]) > window->maxOrdinateValue)
                window->maxOrdinateValue = fabsf(window->data.realp[i]);
            if(fabsf(window->data.imagp[i]) > window->maxOrdinateValue)
                window->maxOrdinateValue = fabsf(window->data.imagp[i]);
        }
    }
    // Hide very small signals of not visible in displayed density matrix
    if((window->maxOrdinateValue < 0.02 && window->showsFrequencyDomain) || window->maxOrdinateValue < 0.002)
        window->maxOrdinateValue = 10;
    recalculate_graph(window);
    set_dataPoints_label(window, points, max);
    /* window->domainOf2DSpectrum = 0; */ // Commented out, because it prevents sym/tilt to show cursor
    if (window->numberOfPeaks > 0) {
        window->numberOfPeaks = 0;
        g_ptr_array_free(window->peaks, TRUE);
    }
    if(insensitive_controller_get_windowFunction(window->controller) == WFLorentzGaussTransformation
       || insensitive_controller_get_windowFunction(window->controller) == WFGaussPseudoEchoTransformation) {
        window->gaussianWidth = insensitive_controller_get_gaussianWidth(window->controller);
        window->gaussianShift = insensitive_controller_get_gaussianShift(window->controller);
    }
    window->windowFunctionType = insensitive_controller_get_windowFunction(window->controller);
    g_idle_add((GSourceFunc)update_spectrum_parameter_panel, window); /* wait until done */
}


void set_dataPoints_label(InsensitiveWindow *window, unsigned int points, unsigned int max)
{
    gchar *str = malloc(40 * sizeof(gchar));

    if(window->twoDimensionalSpectrum && points != 0)
        sprintf(str, "Data points: %d × %d", points, max / points);
    else
        sprintf(str, "Data points: %d", max);
    gtk_label_set_text(window->dataPoints_label, str);
}


G_MODULE_EXPORT void on_lineWidth_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    float value = atof(gtk_entry_get_text(entry));

    if(value < 0.5)
        window->lineWidth = 0.5;
    else
        window->lineWidth = value;
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
    insensitive_settings_set_spectrumLineWidth(window->controller->settings, window->lineWidth);
}


G_MODULE_EXPORT void on_lineWidth_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    gtk_entry_set_text(window->lineWidth_entry, gtk_combo_box_text_get_active_text(combobox));
    on_lineWidth_entry_activate(window->lineWidth_entry, window);
}


void phase_slider_changed(InsensitiveWindow *window)
{
    set_phase_correction(window, window->zeroOrderPhaseShift, window->firstOrderPhaseShift, window->pivotPoint);
    set_openedFileState_for_pulseSequence(window, FileOpenedAndChanged, NULL);
    update_spectrum_parameter_panel(window);
}


float get_zero_order_phase(InsensitiveWindow *window)
{
    return window->zeroOrderPhaseShift;
}


G_MODULE_EXPORT void on_zeroOrder_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->zeroOrderPhaseShift = gtk_adjustment_get_value(adjustment) / 180 * M_PI;
    phase_slider_changed(window);
}


float get_first_order_phase(InsensitiveWindow *window)
{
    return window->firstOrderPhaseShift;
}


G_MODULE_EXPORT void on_firstOrder_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->firstOrderPhaseShift = gtk_adjustment_get_value(adjustment) / 180 * M_PI;
    phase_slider_changed(window);
}


float get_pivot_point(InsensitiveWindow *window)
{
    return window->pivotPoint;
}


G_MODULE_EXPORT void on_pivotPoint_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->pivotPoint = gtk_adjustment_get_value(adjustment);
    phase_slider_changed(window);

    if (window->showsFrequencyDomain) {
        window->hideYCursor = TRUE;
        if(window->drawPivotPoint)
            g_source_remove(window->removePivotPointTimerNr);
        else
            window->drawPivotPoint = TRUE;
        window->removePivotPointTimerNr = g_timeout_add(1000, removePivotPointTimerEvent, window);
    }
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


gboolean removePivotPointTimerEvent(gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->drawPivotPoint = FALSE;
    window->hideYCursor = FALSE;
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);

    return FALSE;
}


G_MODULE_EXPORT void on_resetPhase_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->zeroOrderPhaseShift = 0.0;
    gtk_adjustment_set_value(window->zeroOrder_adjustment, window->zeroOrderPhaseShift);
    window->firstOrderPhaseShift = 0.0;
    gtk_adjustment_set_value(window->firstOrder_adjustment, window->firstOrderPhaseShift);
    window->pivotPoint = 0.0;
    gtk_adjustment_set_value(window->pivotPoint_adjustment, window->pivotPoint);
    phase_slider_changed(window);
}


gboolean get_showRealSpectrum(InsensitiveWindow *window)
{
    return insensitive_settings_get_showRealPart(window->controller->settings);
}


void set_showRealSpectrum(InsensitiveWindow *window, gboolean value)
{
    gtk_toggle_button_set_active(window->showReal_checkbox, value);
    window->drawRealPart = value;
    recalculate_graph(window);
}


G_MODULE_EXPORT void on_showReal_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_showRealPart(window->controller,
                                                 gtk_toggle_button_get_active(checkbox));
    if(window->plotMode != Stacked && insensitive_settings_get_showRealPart(window->controller->settings)
       && insensitive_settings_get_showImaginaryPart(window->controller->settings) && window->twoDimensionalSpectrum)
        insensitive_controller_set_showImaginaryPart(window->controller, FALSE);
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
}


gboolean get_showImaginarySpectrum(InsensitiveWindow *window)
{
    return insensitive_settings_get_showImaginaryPart(window->controller->settings);
}


void set_showImaginarySpectrum(InsensitiveWindow *window, gboolean value)
{
    gtk_toggle_button_set_active(window->showImaginary_checkbox, value);
    window->drawImaginaryPart = value;
    recalculate_graph(window);
}


G_MODULE_EXPORT void on_showImaginary_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_showImaginaryPart(window->controller,
                                                 gtk_toggle_button_get_active(checkbox));
    if(window->plotMode != Stacked && insensitive_settings_get_showRealPart(window->controller->settings)
       && insensitive_settings_get_showImaginaryPart(window->controller->settings) && window->twoDimensionalSpectrum)
        insensitive_controller_set_showRealPart(window->controller, FALSE);
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
}


gboolean get_showIntegral(InsensitiveWindow *window)
{
    return insensitive_settings_get_showIntegral(window->controller->settings);
}


void set_showIntegral(InsensitiveWindow *window, gboolean value)
{
    gtk_toggle_button_set_active(window->integral_checkbox, value);
    window->drawIntegral = value;
    recalculate_graph(window);
}


G_MODULE_EXPORT void on_integral_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_showIntegral(window->controller,
                                            gtk_toggle_button_get_active(checkbox));
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
}


gboolean get_show_windowFunction(InsensitiveWindow *window)
{
    return insensitive_settings_get_showWindowFunction(window->controller->settings);
}


void set_show_windowFunction(InsensitiveWindow *window, gboolean value)
{
    gtk_toggle_button_set_active((GtkToggleButton *)window->window_checkbox, value);
    window->drawWindow = value;
    window->windowFunctionType = insensitive_controller_get_windowFunction(window->controller);
    if (window->windowFunctionType == WFLorentzGaussTransformation
        || window->windowFunctionType == WFGaussPseudoEchoTransformation) {
        window->gaussianWidth = insensitive_controller_get_gaussianWidth(window->controller);
        window->gaussianShift = insensitive_controller_get_gaussianShift(window->controller);
    }
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


G_MODULE_EXPORT void on_window_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_showWindowFunction(window->controller, gtk_toggle_button_get_active(checkbox));
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_shiftBaseline_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->shiftedBaseline = gtk_toggle_button_get_active(window->shiftBaseline_checkbox);
    if (!window->twoDimensionalSpectrum)
        gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


void set_display_frequency_domain(InsensitiveWindow *window, gboolean value)
{
    window->ordinateCentered = value;
    window->showsFrequencyDomain = value;
}


enum PlotMode get_plotMode(InsensitiveWindow *window)
{
    return window->plotMode;
}


G_MODULE_EXPORT void on_grid_checkbox_toggled(GtkToggleButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_settings_set_showGrid(window->controller->settings, gtk_toggle_button_get_active(button));
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


G_MODULE_EXPORT void on_plotStyle_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    switch (gtk_combo_box_get_active(combobox)) {
    case 1:
        window->plotMode = Stacked;
        break;
    case 2:
        window->plotMode = Contours;
        break;
    default:
        window->plotMode = Raster;
    }
    if(window->plotMode != Stacked
       && insensitive_settings_get_showRealPart(window->controller->settings)
       && insensitive_settings_get_showImaginaryPart(window->controller->settings))
        insensitive_controller_set_showImaginaryPart(window->controller, FALSE);
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


int get_current_spectrum_domain(InsensitiveWindow *window)
{
    return window->domainOf2DSpectrum;
}


G_MODULE_EXPORT void on_fid_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    set_display_frequency_domain(window, FALSE);
    window->spectrumIsDOSY2D = FALSE;
    if (window->twoDimensionalSpectrum) {
        window->scaling = 100;
        window->shows2DFrequencyDomain = FALSE;
        insensitive_controller_show_SER(window->controller);
    } else {
        insensitive_controller_show_FID(window->controller);
    }
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
    window->domainOf2DSpectrum = 0;
}


G_MODULE_EXPORT void on_fft1D_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    set_display_frequency_domain(window, TRUE);
    window->spectrumIsDOSY2D = FALSE;
    if(window->twoDimensionalSpectrum){
        window->scaling = 10;
        window->shows2DFrequencyDomain = FALSE;
        insensitive_controller_fourier_transform_2D_spectrum_along_T2(window->controller);
    } else {
        insensitive_controller_fourier_transform_1D_spectrum(window->controller);
    }
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
    window->domainOf2DSpectrum = 1;
}


G_MODULE_EXPORT void on_fft2D_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    set_display_frequency_domain(window, TRUE);
    window->spectrumIsDOSY2D = FALSE;
    window->scaling = 1;
    window->shows2DFrequencyDomain = TRUE;
    insensitive_controller_fourier_transform_2D_spectrum_along_T2_and_T1(window->controller);
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
    window->domainOf2DSpectrum = 2;
}


G_MODULE_EXPORT void on_magnitude_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    set_display_frequency_domain(window, TRUE);
    window->spectrumIsDOSY2D = FALSE;
    if(window->twoDimensionalSpectrum) {
        window->scaling = 1;
        window->shows2DFrequencyDomain = TRUE;
        insensitive_controller_absolute_value_spectrum(window->controller);
    } else {
        insensitive_controller_absolute_value_1D_spectrum(window->controller, TRUE);
    }
    set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
    window->domainOf2DSpectrum = 3;
    update_spectrum_parameter_panel(window);
}


int get_statesDataSet(InsensitiveWindow *window)
{
    return window->statesDataSet;
}


G_MODULE_EXPORT void on_absDisp_radtiobutton_toggled(GtkToggleButton *radiobutton, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->statesDataSet = (radiobutton == window->absDispReal_radiobutton);
    insensitive_controller_set_realDataSetsForStatesMethod(window->controller, window->statesDataSet);
    switch(window->domainOf2DSpectrum) {
        case 0:
            on_fid_button_clicked(window->fid_button, window);
            break;
        case 1:
            on_fft1D_button_clicked(window->fft1D_button, window);
            break;
        case 2:
            on_fft2D_button_clicked(window->fft2D_button, window);
            break;
        default:
            on_magnitude_button_clicked(window->magnitude_button, window);
    }
}


G_MODULE_EXPORT void on_gaussian_window_function_adjusted(gpointer widget, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_set_gaussianWidth(window->controller, 0.042 / (atof(gtk_entry_get_text(window->gaussianWidth_entry)) * insensitive_settings_get_dwellTime(window->controller->settings)));
    insensitive_controller_set_gaussianShift(window->controller, gtk_adjustment_get_value(window->gaussianShift_adjustment));
    window->windowFunctionType = insensitive_controller_get_windowFunction(window->controller);
    if(window->windowFunctionType == WFLorentzGaussTransformation
       || window->windowFunctionType == WFGaussPseudoEchoTransformation) {
        window->gaussianWidth = insensitive_controller_get_gaussianWidth(window->controller);
        window->gaussianShift = insensitive_controller_get_gaussianShift(window->controller);
    }
    if(window->domainOf2DSpectrum == 1)
        on_fft1D_button_clicked(window->fft1D_button, window);
    else if(window->domainOf2DSpectrum == 2)
        on_fft2D_button_clicked(window->fft2D_button, window);
    else if(window->domainOf2DSpectrum == 3)
        on_magnitude_button_clicked(window->fft1D_button, window);
}


G_MODULE_EXPORT void on_apodization_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	enum WindowFunctionType windowFunction;
    gchar *str;

    windowFunction = (enum WindowFunctionType)gtk_combo_box_get_active(combobox);
	if (windowFunction == WFLorentzGaussTransformation) {
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, TRUE);
        gtk_adjustment_set_value(window->gaussianShift_adjustment, 0.0);
		insensitive_controller_set_gaussianShift(window->controller, 0.0);
		str = malloc(8 * sizeof(gchar));
        sprintf(str, "%.2f", 0.042 / (insensitive_settings_get_T2(window->controller->settings) * insensitive_settings_get_dwellTime(window->controller->settings)));
		gtk_entry_set_text(window->gaussianWidth_entry, str);
        insensitive_controller_set_gaussianWidth(window->controller, insensitive_settings_get_T2(window->controller->settings));
	} else if (windowFunction == WFGaussPseudoEchoTransformation) {
		gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, TRUE);
		gtk_adjustment_set_value(window->gaussianShift_adjustment, 0.5);
		insensitive_controller_set_gaussianShift(window->controller, 0.5);
		str = malloc(8 * sizeof(gchar));
        sprintf(str, "%.2f", 0.042 / (insensitive_settings_get_T2(window->controller->settings) * insensitive_settings_get_dwellTime(window->controller->settings)));
		gtk_entry_set_text(window->gaussianWidth_entry, str);
        insensitive_controller_set_gaussianWidth(window->controller, insensitive_settings_get_T2(window->controller->settings));
	} else {
		gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, FALSE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, FALSE);
	}
    insensitive_controller_set_windowFunction(window->controller, windowFunction);
	if (windowFunction == WFLorentzGaussTransformation || windowFunction == WFGaussPseudoEchoTransformation) {
        window->gaussianWidth = insensitive_controller_get_gaussianWidth(window->controller);
        window->gaussianShift = insensitive_controller_get_gaussianShift(window->controller);
	}
    window->windowFunctionType = windowFunction;
	if (window->twoDimensionalSpectrum) {
		set_2D_mode(window, TRUE);
	}
	switch (window->domainOf2DSpectrum) {
	case 1:
        on_fft1D_button_clicked(window->fft1D_button, window);
		break;
	case 2:
		if (window->twoDimensionalSpectrum)
			on_fft2D_button_clicked(window->fft1D_button, window);
		break;
	case 3:
		on_magnitude_button_clicked(window->fft1D_button, window);
		break;
	}
    update_spectrum_parameter_panel(window);
	set_openedFileState_for_spectrum(window, FileOpenedAndChanged, NULL);
}


G_MODULE_EXPORT void on_sym_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
        if(shows_2D_spectrum(window)) {
            window->scaling = 1;
            window->shows2DFrequencyDomain = TRUE;
            insensitive_controller_spectrum_symmetrization(window->controller, SYM, window->domainOf2DSpectrum);
        } else {
            alert_for_invalid_fourier_transform(window, 2);
        }
    } else {
        gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
    }
}


G_MODULE_EXPORT void on_syma_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
        if(shows_2D_spectrum(window)) {
            window->scaling = 1;
            window->shows2DFrequencyDomain = TRUE;
            insensitive_controller_spectrum_symmetrization(window->controller, SYMA, window->domainOf2DSpectrum);
        } else {
            alert_for_invalid_fourier_transform(window, 2);
        }
    } else {
        gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
    }
}


G_MODULE_EXPORT void on_symj_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
        if(shows_2D_spectrum(window)) {
            window->scaling = 1;
            window->shows2DFrequencyDomain = TRUE;
            insensitive_controller_spectrum_symmetrization(window->controller, SYMJ, window->domainOf2DSpectrum);
        } else {
            alert_for_invalid_fourier_transform(window, 2);
        }
    } else {
        gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
    }
}


G_MODULE_EXPORT void on_tilt_jres_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(window->twoDimensionalSpectrum)
        insensitive_controller_tilt_2D_spectrum(window->controller, window->domainOf2DSpectrum, 0);
}


G_MODULE_EXPORT void on_tilt_secsy_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if(window->twoDimensionalSpectrum)
        insensitive_controller_tilt_2D_spectrum(window->controller, window->domainOf2DSpectrum, 1);
}


// For DOSY tool box
G_MODULE_EXPORT void on_dosyToolbox_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    gtk_widget_show(GTK_WIDGET(window->dosyToolBox_window));
}


G_MODULE_EXPORT void on_dosy_show_1D_trace_only_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_get_first_trace_of_2D_spectrum(window->controller, TRUE);
    on_fft1D_button_clicked(NULL, window);
    set_magnification(window, 1.0);

    if(button == window->dosyFirstTrace_button) {
        gtk_widget_set_sensitive((GtkWidget *)window->dosyPickPeaks_button, TRUE);
        gtk_widget_show((GtkWidget *)window->dosyToolBox_window);
    }
}


G_MODULE_EXPORT void on_dosy_fit_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->spectrumIsDOSY2D = FALSE;
    insensitive_controller_dosy_fit(window->controller, insensitive_settings_get_showRealPart(window->controller->settings) ? 1 : 0);
    window->domainOf2DSpectrum = 1;
    set_magnification(window, 0.01);

    if(button == window->dosyFitExponential_button) {
        gtk_widget_set_sensitive((GtkWidget *)window->dosyPlotSpectrum_button, TRUE);
        gtk_widget_show((GtkWidget *)window->dosyToolBox_window);
    }
}


G_MODULE_EXPORT void on_dosy_spectrum_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    insensitive_controller_dosy_spectrum(window->controller, insensitive_settings_get_showRealPart(window->controller->settings) ? 0 : 1);
    /*[verticalScaleBar setUnit:@"cm²/s"];
    [verticalScaleBar setNeedsDisplay:YES];*/
    window->domainOf2DSpectrum = 1;
    set_magnification(window, 0.1);
    window->spectrumIsDOSY2D = TRUE;
}


G_MODULE_EXPORT void on_auto_peak_picking_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int dataset;

    if(!window->twoDimensionalSpectrum && insensitive_controller_get_spectrumDataAvailable(window->controller)
       && window->domainOf2DSpectrum > 0) {
        window->peaks = g_ptr_array_new();
        if(window->domainOf2DSpectrum == 3)
            dataset = 2;
        else
            dataset = !insensitive_settings_get_showRealPart(window->controller->settings) ? 1 : 0;
        window->numberOfPeaks = insensitive_controller_determine_peak_list(window->controller, window->peaks, dataset);
        gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        update_spectrum_parameter_panel(window);

        if(button == window->dosyPickPeaks_button) {
            gtk_widget_set_sensitive((GtkWidget *)window->dosyFitLorentzian_button, TRUE);
            gtk_widget_show((GtkWidget *)window->dosyToolBox_window);
        }
    }
}


G_MODULE_EXPORT void on_dosy_fit_lorentzian_peaks_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int dataset;

    window->spectrumIsDOSY2D = FALSE;
    if(!window->twoDimensionalSpectrum && insensitive_controller_get_spectrumDataAvailable(window->controller)
       && window->domainOf2DSpectrum > 0) {
        if(window->domainOf2DSpectrum == 3)
            dataset = 2;
        else
            dataset = !insensitive_settings_get_showRealPart(window->controller->settings) ? 1 : 0;
        insensitive_controller_fit_lorentzians(window->controller, dataset);

        if(button == window->dosyFitLorentzian_button) {
            gtk_widget_set_sensitive((GtkWidget *)window->dosyFitExponential_button, TRUE);
            gtk_widget_show((GtkWidget *)window->dosyToolBox_window);
        }
    }
}


void reset_dosy_panel(InsensitiveWindow *window)
{
    window->spectrumIsDOSY2D = FALSE;
    gtk_widget_set_sensitive((GtkWidget *)window->dosyFirstTrace_button, TRUE);
    gtk_widget_set_sensitive((GtkWidget *)window->dosyPickPeaks_button, FALSE);
    gtk_widget_set_sensitive((GtkWidget *)window->dosyFitLorentzian_button, FALSE);
    gtk_widget_set_sensitive((GtkWidget *)window->dosyFitExponential_button, FALSE);
    gtk_widget_set_sensitive((GtkWidget *)window->dosyPlotSpectrum_button, FALSE);
}


G_MODULE_EXPORT void on_signalToNoiseThreshold_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *str = malloc(6 * sizeof(gchar));

    insensitive_settings_set_signalToNoiseThreshold(window->controller->settings,
                                                    atof(gtk_entry_get_text(entry)));
    sprintf(str, "%.2f", insensitive_settings_get_signalToNoiseThreshold(window->controller->settings));
    gtk_entry_set_text(entry, str);
    if (window->numberOfPeaks > 0) {
        on_auto_peak_picking_button_clicked(NULL, window);
    }
}


void set_apodization(InsensitiveWindow *window, float *pointerT2, float *pointerT1)
{
    window->apodizationT2 = pointerT2;
    window->apodizationT1 = pointerT1;
    recalculate_graph(window);
}


void show_spectrum_progressbar(InsensitiveWindow *window, gboolean value)
{
    gtk_widget_set_visible((GtkWidget *)window->spectrum_progressbar, value);
}


void set_spectrum_progressbar_maximum(InsensitiveWindow *window, double value)
{
    window->spectrumProgressBarMaximum = value;
}


void add_to_spectrum_progressbar(InsensitiveWindow *window, double value)
{
    gtk_progress_bar_set_fraction(window->spectrum_progressbar, value / window->spectrumProgressBarMaximum);
}


void play_sound(InsensitiveWindow *window)
{
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "fid.wav", NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
            g_print("I should be playing %s now!\n", filename);
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}


/*- (enum OpenFileState)openFileState
{
    return openFileState;
}*/


void set_openedFileState_for_spectrum(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename)
{
    if(state == FileOpened && filename != NULL) {
        //[[self window] setTitle:[NSString stringWithFormat:@"%@: %@", NSLocalizedString(@"Spectrum", nil), filename]];
        window->openFileState_for_spectrum = state;
    } else if(state == FileOpenedAndChanged && window->openFileState_for_spectrum == FileOpened) {
        //[[self window] setTitle:[NSString stringWithFormat:NSLocalizedString(@"%@ [modified]", nil), [[self window] title]]];
        window->openFileState_for_spectrum = state;
    } else if(state == NoFile) {
        //[[self window] setTitle:NSLocalizedString(@"Spectrum", nil)];
        window->openFileState_for_spectrum = state;
    }
}


gboolean perform_open_spectrum(InsensitiveWindow *window, xmlNodePtr node)
{
    xmlNodePtr iter;
    gsize data_len;
    enum PurePhaseDetectionMethod detectionMethod;
	unsigned int dimensions, maxDataPoints, spectrumDomain;
    unsigned int statesDataSet, totalDataPoints, windowFunction;
    enum PlotMode plotStyle;
    float dwellTime, firstOrderPhase, zeroOrderPhase, pivotPoint, magnification;
    gboolean showRealSpectrum, showImaginarySpectrum, showIntegral;
    gchar *str, *report = NULL;
	xmlChar *version, *realFID, *imaginaryFID, *realFIDStates, *imaginaryFIDStates;
    float *realFIDData, *imaginaryFIDData, *realFIDStatesData, *imaginaryFIDStatesData;

    realFID = NULL;
    realFIDStates = NULL;
    imaginaryFID = NULL;
    imaginaryFIDStates = NULL;
    realFIDData = NULL;
    realFIDStatesData = NULL;
    imaginaryFIDData = NULL;
    imaginaryFIDStatesData = NULL;
    showRealSpectrum = TRUE;
    showImaginarySpectrum = FALSE;
    showIntegral = FALSE;
    plotStyle = Raster;
	for (iter = node; iter; iter = iter->next) {
		if (iter->type == XML_ELEMENT_NODE && !strcmp((char *)iter->name, "key")) {
            if (!strcmp((char *)xmlNodeGetContent(iter), "IGGFileVersion"))
				version = xmlNodeGetContent(iter->next->next);
			else if (!strcmp((char *)xmlNodeGetContent(iter), "DetectionMethod"))
				detectionMethod = (enum PurePhaseDetectionMethod)atoi((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "Dimensions"))
                dimensions = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "DwellTime"))
				dwellTime = atof((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "FirstOrderPhase"))
                firstOrderPhase = atof((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ImaginaryFID"))
                imaginaryFID = xmlNodeGetContent(iter->next->next);
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ImaginaryFIDStates"))
                imaginaryFIDStates = xmlNodeGetContent(iter->next->next);
            else if (!strcmp((char *)xmlNodeGetContent(iter), "Magnification"))
				magnification = atof((char *)xmlNodeGetContent(iter->next->next));
			else if (!strcmp((char *)xmlNodeGetContent(iter), "MaxDataPoints"))
				maxDataPoints = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "PivotPoint"))
                pivotPoint = atof((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "PlotStyle"))
                plotStyle = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "RealFID"))
                realFID = xmlNodeGetContent(iter->next->next);
            else if (!strcmp((char *)xmlNodeGetContent(iter), "RealFIDStates"))
                realFIDStates = xmlNodeGetContent(iter->next->next);
            else if (!strcmp((char *)xmlNodeGetContent(iter), "Report"))
				report = (gchar *)xmlNodeGetContent(iter->next->next);
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ShowIntegral"))
                showIntegral = !strcmp((char *)iter->next->next->name, "true");
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ShowImaginarySpectrum"))
                showImaginarySpectrum = !strcmp((char *)iter->next->next->name, "true");
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ShowRealSpectrum"))
                showRealSpectrum = !strcmp((char *)iter->next->next->name, "true");
            else if (!strcmp((char *)xmlNodeGetContent(iter), "SpectrumDomain"))
				spectrumDomain = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "StatesDataSet"))
				statesDataSet = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "TotalDataPoints"))
				totalDataPoints = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "WindowFunction"))
				windowFunction = atoi((char *)xmlNodeGetContent(iter->next->next));
            else if (!strcmp((char *)xmlNodeGetContent(iter), "ZeroOrderPhase"))
				zeroOrderPhase = atof((char *)xmlNodeGetContent(iter->next->next));
		}
	}
    if (strcmp((char *)version, "Insensitive Spectrum Data 0.9.12") && strcmp((char *)version, "Insensitive Spectrum Data 0.9.22")) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                       GTK_DIALOG_DESTROY_WITH_PARENT,
							                       GTK_MESSAGE_ERROR,
							                       GTK_BUTTONS_OK,
							                       "This file does not appear to be in the Insensitive Spectrum file format.");
	    gtk_window_set_title(GTK_WINDOW(dialog), "Corrupt spectrum file");
	    gtk_dialog_run(GTK_DIALOG(dialog));
	    gtk_widget_destroy(dialog);
        return FALSE;
    }
    insensitive_controller_stop_acquisition(window->controller);
    insensitive_controller_reset_acquisition_for_dataPoints(window->controller, maxDataPoints);
    insensitive_controller_set_dwellTime(window->controller, dwellTime);
    window->controller->detectionMethodOfCurrentSpectrum = detectionMethod;
    window->controller->settings->detectionMethod = detectionMethod;
    set_showRealSpectrum(window, showRealSpectrum);
    insensitive_controller_set_showRealPart(window->controller, showRealSpectrum);
    set_showImaginarySpectrum(window, showImaginarySpectrum);
    insensitive_controller_set_showImaginaryPart(window->controller, showImaginarySpectrum);
    set_showIntegral(window, showIntegral);
    insensitive_controller_set_showIntegral(window->controller, showIntegral);
    window->statesDataSet = statesDataSet;
    gtk_toggle_button_set_active(window->absDispReal_radiobutton, statesDataSet);
    insensitive_controller_set_realDataSetsForStatesMethod(window->controller, statesDataSet);
    window->plotMode = plotStyle;
    switch (plotStyle) {
    case Stacked:
        gtk_combo_box_set_active(window->plotStyle_combobox, 1);
        break;
    case Contours:
        gtk_combo_box_set_active(window->plotStyle_combobox, 2);
        break;
    default:
        gtk_combo_box_set_active(window->plotStyle_combobox, 0);
    }
    realFIDData = (float *)g_base64_decode((char *)realFID, &data_len);
    imaginaryFIDData = (float *)g_base64_decode((char *)imaginaryFID, &data_len);
    if (realFIDStates != NULL)
        realFIDStatesData = (float *)g_base64_decode((char *)realFIDStates, &data_len);
    if (imaginaryFIDStates != NULL)
        imaginaryFIDStatesData = (float *)g_base64_decode((char *)imaginaryFIDStates, &data_len);
    insensitive_controller_inject_spectrum(window->controller,
                                           realFIDData,
                                           imaginaryFIDData,
                                           realFIDStatesData,
                                           imaginaryFIDStatesData,
                                           totalDataPoints,
                                           maxDataPoints,
                                           spectrumDomain);
    free(realFIDData);
    free(imaginaryFIDData);
    free(realFIDStatesData);
    free(imaginaryFIDStatesData);
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, FALSE);
    gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, FALSE);
    switch (windowFunction) {
    case 1:
        insensitive_controller_set_windowFunction(window->controller, WFExp);
        gtk_combo_box_set_active(window->apodization_combobox, 1);
        break;
    case 2:
        insensitive_controller_set_windowFunction(window->controller, WFCosine);
        gtk_combo_box_set_active(window->apodization_combobox, 2);
        break;
    case 3:
        insensitive_controller_set_windowFunction(window->controller, WFTriangle);
        gtk_combo_box_set_active(window->apodization_combobox, 3);
        break;
    case 4:
        insensitive_controller_set_windowFunction(window->controller, WFHann);
        gtk_combo_box_set_active(window->apodization_combobox, 4);
        break;
    case 5:
        insensitive_controller_set_windowFunction(window->controller, WFWeightedHann);
        gtk_combo_box_set_active(window->apodization_combobox, 6);
        break;
    case 6:
        insensitive_controller_set_windowFunction(window->controller, WFSineBell);
        gtk_combo_box_set_active(window->apodization_combobox, 5);
        break;
    case 7:
        insensitive_controller_set_windowFunction(window->controller, WFLorentzGaussTransformation);
        gtk_combo_box_set_active(window->apodization_combobox, 9);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, TRUE);
		gtk_adjustment_set_value(window->gaussianShift_adjustment, 0.0);
		insensitive_controller_set_gaussianShift(window->controller, 0.0);
		str = malloc(8 * sizeof(gchar));
        sprintf(str, "%.2f", 0.042 / (insensitive_settings_get_T2(window->controller->settings) * insensitive_settings_get_dwellTime(window->controller->settings)));
		gtk_entry_set_text(window->gaussianWidth_entry, str);
        insensitive_controller_set_gaussianWidth(window->controller, insensitive_settings_get_T2(window->controller->settings));
        free(str);
        break;
    case 8:
        insensitive_controller_set_windowFunction(window->controller, WFGaussPseudoEchoTransformation);
        gtk_combo_box_set_active(window->apodization_combobox, 10);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidth_entry, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianWidthUnit_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_label, TRUE);
        gtk_widget_set_visible((GtkWidget *)window->gaussianShift_slider, TRUE);
		gtk_adjustment_set_value(window->gaussianShift_adjustment, 0.5);
		insensitive_controller_set_gaussianShift(window->controller, 0.5);
		str = malloc(8 * sizeof(gchar));
        sprintf(str, "%.2f", 0.042 / (insensitive_settings_get_T2(window->controller->settings) * insensitive_settings_get_dwellTime(window->controller->settings)));
		gtk_entry_set_text(window->gaussianWidth_entry, str);
        insensitive_controller_set_gaussianWidth(window->controller, insensitive_settings_get_T2(window->controller->settings));
        free(str);
        break;
    case 9:
        insensitive_controller_set_windowFunction(window->controller, WFTraficante);
        gtk_combo_box_set_active(window->apodization_combobox, 7);
        break;
    case 10:
        insensitive_controller_set_windowFunction(window->controller, WFTraficanteSN);
        gtk_combo_box_set_active(window->apodization_combobox, 8);
        break;
    default:
        insensitive_controller_set_windowFunction(window->controller, WFNone);
        gtk_combo_box_set_active(window->apodization_combobox, 0);
    }
    gtk_adjustment_set_value(window->zeroOrder_adjustment, zeroOrderPhase * 180 / M_PI);
    gtk_adjustment_set_value(window->firstOrder_adjustment, firstOrderPhase * 180 / M_PI);
    gtk_adjustment_set_value(window->pivotPoint_adjustment, pivotPoint);
    insensitive_controller_set_detectionMethod(window->controller, detectionMethod);
    gtk_widget_set_visible(GTK_WIDGET(window->absDispReal_radiobutton), detectionMethod != None);
    gtk_widget_set_visible(GTK_WIDGET(window->absDispImag_radiobutton), detectionMethod != None);
    gtk_widget_set_visible(GTK_WIDGET(window->dataSet_label), detectionMethod != None);
    set_magnification(window, magnification);
    insensitive_controller_set_spectrumReport(window->controller, report);
    update_spectrum_parameter_panel(window);
    reset_dosy_panel(window);
    show_spectrumParameters_textview(window, report != NULL);

    return TRUE;
}


G_MODULE_EXPORT void perform_save_spectrum(GtkMenuItem *menuitem, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	GtkWidget *chooser, *dialog;
	char str[20];
	gint result;
	gchar *filename, *base64;
	GString *report;
	unsigned int dataPoints, totalDataPoints;
	DSPSplitComplex *fid;
	xmlDoc *doc = NULL;
	xmlNode *root, *first_child;
	GtkFileFilter *filter;

	if (insensitive_controller_get_spectrumDataAvailable(window->controller)) {
		chooser = gtk_file_chooser_dialog_new("Save Insensitive Spectrum...",
						                      (GtkWindow *)window,
						                      GTK_FILE_CHOOSER_ACTION_SAVE,
						                      "Cancel", GTK_RESPONSE_CANCEL,
						                      "Save", GTK_RESPONSE_ACCEPT,
						                      NULL);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), "spectrum.igg");
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Insensitive spectra (IGG)");
		gtk_file_filter_add_pattern(filter, "*.igg");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
		gtk_widget_show_all(chooser);
		result = gtk_dialog_run((GtkDialog *)chooser);
		if (result == GTK_RESPONSE_ACCEPT) {
			filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
			LIBXML_TEST_VERSION
				doc = xmlNewDoc(BAD_CAST "1.0");
			root = xmlNewNode(NULL, BAD_CAST "plist");
			xmlNewProp(root, (const xmlChar *)"version", (const xmlChar *)"1.0");
			xmlDocSetRootElement(doc, root);
			xmlCreateIntSubset(doc, BAD_CAST "plist", BAD_CAST "-//Apple//DTD PLIST 1.0//EN", BAD_CAST "http://www.apple.com/DTDs/PropertyList-1.0.dtd");
			first_child = xmlNewChild(root, NULL, BAD_CAST "dict", NULL);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "IGGFileVersion");
			xmlNewChild(first_child, NULL, BAD_CAST "string", BAD_CAST "Insensitive Spectrum Data 0.9.22");
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "MaxDataPoints");
			dataPoints = insensitive_settings_get_dataPoints(window->controller->settings);
			sprintf(str, "%d", dataPoints);
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "DwellTime");
			sprintf(str, "%.17f", insensitive_settings_get_dwellTime(window->controller->settings));
			xmlNewChild(first_child, NULL, BAD_CAST "real", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "Dimensions");
			if (window->twoDimensionalSpectrum) {
				xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST "2");
				totalDataPoints = indirect_datapoints(insensitive_controller_get_detectionMethod(window->controller), dataPoints);
				totalDataPoints *= dataPoints;
			} else {
				xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST "1");
				totalDataPoints = dataPoints;
			}
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "TotalDataPoints");
			sprintf(str, "%d", totalDataPoints);
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ZeroOrderPhase");
			sprintf(str, "%.17f", window->zeroOrderPhaseShift);
			xmlNewChild(first_child, NULL, BAD_CAST "real", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "FirstOrderPhase");
			sprintf(str, "%.17f", window->firstOrderPhaseShift);
			xmlNewChild(first_child, NULL, BAD_CAST "real", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PivotPoint");
			sprintf(str, "%.17f", window->pivotPoint);
			xmlNewChild(first_child, NULL, BAD_CAST "real", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "WindowFunction");
            switch (insensitive_controller_get_windowFunction(window->controller)) {
            case WFExp:
                strcpy(str, "1");
                break;
            case WFCosine:
                strcpy(str, "2");
                break;
            case WFTriangle:
                strcpy(str, "3");
                break;
            case WFHann:
                strcpy(str, "4");
                break;
            case WFWeightedHann:
                strcpy(str, "5");
                break;
            case WFSineBell:
                strcpy(str, "6");
                break;
            case WFLorentzGaussTransformation:
                strcpy(str, "7");
                break;
            case WFGaussPseudoEchoTransformation:
                strcpy(str, "8");
                break;
            case WFTraficante:
                strcpy(str, "9");
                break;
            case WFTraficanteSN:
                strcpy(str, "10");
                break;
            default:
                strcpy(str, "0");
            }
            xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ShowRealSpectrum");
			if (insensitive_settings_get_showRealPart(window->controller->settings))
				xmlNewChild(first_child, NULL, BAD_CAST "true", NULL);
			else
				xmlNewChild(first_child, NULL, BAD_CAST "false", NULL);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ShowImaginarySpectrum");
			if (insensitive_settings_get_showImaginaryPart(window->controller->settings))
				xmlNewChild(first_child, NULL, BAD_CAST "true", NULL);
			else
				xmlNewChild(first_child, NULL, BAD_CAST "false", NULL);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ShowIntegral");
			if (insensitive_settings_get_showIntegral(window->controller->settings))
				xmlNewChild(first_child, NULL, BAD_CAST "true", NULL);
			else
				xmlNewChild(first_child, NULL, BAD_CAST "false", NULL);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "SpectrumDomain");
			sprintf(str, "%d", window->domainOf2DSpectrum);
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "Magnification");
			sprintf(str, "%.17f", window->magnification);
			xmlNewChild(first_child, NULL, BAD_CAST "real", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "PlotStyle");
			sprintf(str, "%d", window->plotMode);
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "StatesDataSet");
			sprintf(str, "%d", window->statesDataSet);
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "DetectionMethod");
			sprintf(str, "%d", insensitive_controller_get_detectionMethod(window->controller));
			xmlNewChild(first_child, NULL, BAD_CAST "integer", BAD_CAST str);
			report = insensitive_controller_get_spectrumReport(window->controller);
			if (report != NULL) {
				xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "Report");
				xmlNewChild(first_child, NULL, BAD_CAST "string", (const xmlChar *)report->str);
			}
			fid = insensitive_controller_get_rawFID(window->controller);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "RealFID");
			base64 = g_base64_encode((const guchar *)fid->realp, totalDataPoints * sizeof(float));
			xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
			g_free(base64);
			xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ImaginaryFID");
			base64 = g_base64_encode((const guchar *)fid->imagp, totalDataPoints * sizeof(float));
			xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
			g_free(base64);
			if (insensitive_controller_get_detectionMethod(window->controller) == States
			    || insensitive_controller_get_detectionMethod(window->controller) == StatesTPPI) {
				fid = insensitive_controller_get_rawFIDStates(window->controller);
				xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "RealFIDStates");
				base64 = g_base64_encode((const guchar *)fid->realp, totalDataPoints * sizeof(float));
				xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
				g_free(base64);
				xmlNewChild(first_child, NULL, BAD_CAST "key", BAD_CAST "ImaginaryFIDStates");
				base64 = g_base64_encode((const guchar *)fid->imagp, totalDataPoints * sizeof(float));
				xmlNewChild(first_child, NULL, BAD_CAST "data", BAD_CAST base64);
				g_free(base64);
			}
			xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
			xmlFreeDoc(doc);
			xmlCleanupParser();
			xmlMemoryDump();
			set_openedFileState_for_pulseSequence(window, FileOpened, filename);
			g_free(filename);
		}
		g_object_unref(chooser);
	} else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							"There is no spectrum data that can be saved.");
		gtk_window_set_title(GTK_WINDOW(dialog), "No spectrum acquired");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}


G_MODULE_EXPORT void export_spectrum(GtkMenuItem *menuitem, InsensitiveWindow *window)
{
	float DW, begin1, step1, begin2, step2;
	float freq1, freq2;
	float min_y1, min_y2, max_y1, max_y2, min_x1, min_x2, max_x1, max_x2;
	float factor_x1, factor_x2, factor_y1, factor_y2, newPhase0;
	//float first_x1, first_x2, first_y1, first_y2, last_x1, last_x2;
	float *dataset;
	gchar *nuc1, *nuc2, *fnmode, *pulprog, *datatype, *x_unit, *y_unit, *x_scale, *sqz;
	signed long current;
	char *z;
	unsigned int i, j, p, t1DataPoints, t2DataPoints;
	DSPSplitComplex data;
	GString *csv, *spectrum_report;
	gint result;
	gchar *filename, *separator, *date_str, *freq_str;
	GtkWidget *dialog, *chooser;
	GtkFileFilter *filter, *current_filter;
	GDateTime *date = g_date_time_new_now_local();

	if (insensitive_controller_get_spectrumDataAvailable(window->controller)) {
		separator = malloc(2 * sizeof(gchar));
		chooser = gtk_file_chooser_dialog_new("Export spectrum",
						      (GtkWindow *)window,
						      GTK_FILE_CHOOSER_ACTION_SAVE,
						      "Cancel", GTK_RESPONSE_CANCEL,
						      "Export", GTK_RESPONSE_ACCEPT,
						      NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.csv");
		gtk_file_filter_set_name(filter, "Comma Delimited (CSV)");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
		current_filter = filter;
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.dat");
		gtk_file_filter_add_pattern(filter, "*.asc");
		gtk_file_filter_set_name(filter, "Tab Delimited (DAT, ASC)");
		if (insensitive_settings_get_exportFormat(window->controller->settings) == DAT)
			current_filter = filter;
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.jdx");
		gtk_file_filter_add_pattern(filter, "*.dx");
		gtk_file_filter_set_name(filter, "JCAMP-DX files (JDX)");
		if (insensitive_settings_get_exportFormat(window->controller->settings) == JDX)
			current_filter = filter;
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.txt");
		gtk_file_filter_set_name(filter, "Bruker-style export (TXT)");
		if (insensitive_settings_get_exportFormat(window->controller->settings) == TXT)
			current_filter = filter;
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
        filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern(filter, "*.png");
		gtk_file_filter_set_name(filter, "Portable Network Graphic (PNG)");
		if (insensitive_settings_get_exportFormat(window->controller->settings) == PNG)
			current_filter = filter;
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser), current_filter);
		gtk_widget_show_all(chooser);
		result = gtk_dialog_run((GtkDialog *)chooser);
		if (result == GTK_RESPONSE_ACCEPT) {
			filter = gtk_file_chooser_get_filter((GtkFileChooser *)chooser);
			if (!strcmp(gtk_file_filter_get_name(filter), "Comma Delimited (CSV)")) {
				strcpy(separator, ",");
				insensitive_settings_set_exportFormat(window->controller->settings, CSV);
			} else if (!strcmp(gtk_file_filter_get_name(filter), "Tab Delimited (DAT, ASC)")) {
				strcpy(separator, "\t");
				insensitive_settings_set_exportFormat(window->controller->settings, DAT);
			} else if (!strcmp(gtk_file_filter_get_name(filter), "JCAMP-DX files (JDX)"))
				insensitive_settings_set_exportFormat(window->controller->settings, JDX);
			else if (!strcmp(gtk_file_filter_get_name(filter), "Bruker-style export (TXT)"))
				insensitive_settings_set_exportFormat(window->controller->settings, TXT);
            else if (!strcmp(gtk_file_filter_get_name(filter), "Portable Network Graphic (PNG)"))
				insensitive_settings_set_exportFormat(window->controller->settings, PNG);
			filename = gtk_file_chooser_get_filename((GtkFileChooser *)chooser);
			t2DataPoints = insensitive_settings_get_dataPoints(window->controller->settings);
			data = window->displayedData;
			csv = g_string_new("");
			DW = insensitive_settings_get_dwellTime(window->controller->settings);
			if (window->twoDimensionalSpectrum) {
				t1DataPoints = indirect_datapoints(insensitive_controller_get_detectionMethod(window->controller), t2DataPoints);
				switch (window->domainOf2DSpectrum) {
				case FID:
					step1 = DW;
					step2 = DW;
					begin1 = 0;
					begin2 = 0;
					break;
				case FFT1D:
					step1 = DW;
					step2 = 1 / (DW * t2DataPoints);
					begin1 = 0;
					begin2 = -0.5 / DW;
					break;
				case FFT2D:
				case AbsValue:
					step1 = 1 / (DW * t1DataPoints);
					step2 = 1 / (DW * t2DataPoints);
					begin1 = -0.5 / DW;
					begin2 = -0.5 / DW;
				}
			} else {
				if (!window->showsFrequencyDomain) {
					step1 = DW;
					begin1 = 0;
				} else {
					step1 = 1 / (DW * t2DataPoints);
					begin1 = -0.5 / DW;
				}
			}
			switch (insensitive_settings_get_exportFormat(window->controller->settings)) {
			case PNG:
				cairo_surface_write_to_png(window->spectrum_surface, filename);
				break;
			case JDX:
				spectrum_report = insensitive_controller_get_spectrumReport(window->controller);
				sqz = malloc(12 * sizeof(gchar));
				z = malloc(10 * sizeof(gchar));
				datatype = malloc(9 * sizeof(gchar));
				x_scale = malloc(10 * sizeof(gchar));
				x_unit = malloc(8 * sizeof(gchar));
				y_unit = malloc(16 * sizeof(gchar));
				strcpy(y_unit, "ARBITRARY UNITS");
				if (window->showsFrequencyDomain) {
					strcpy(datatype, "SPECTRUM");
					strcpy(x_scale, "FREQUENCY");
					strcpy(x_unit, "HZ");
				} else {
					strcpy(datatype, "FID");
					strcpy(x_scale, "TIME");
					strcpy(x_unit, "SECONDS");
				}
				if (window->twoDimensionalSpectrum) {
					freq_str = substring_for_keyword_in_string("SFO1", spectrum_report->str, spectrum_report->len);
					if (freq_str == NULL)
						freq2 = spectrometer_frequency / 1e6;
					else {
						freq_str[strlen(freq_str) - 4] = '\0';
						freq2 = atof(freq_str);
						free(freq_str);
					}
					freq_str = substring_for_keyword_in_string("SFO2", spectrum_report->str, spectrum_report->len);
					if (freq_str == NULL)
						freq1 = freq2;
					else {
						freq_str[strlen(freq_str) - 4] = '\0';
						freq1 = atof(freq_str);
						free(freq_str);
					}
					nuc2 = substring_for_keyword_in_string("NUC1", spectrum_report->str, spectrum_report->len);
					if (nuc2 == NULL) {
						nuc2 = malloc(3 * sizeof(gchar));
						strcpy(nuc2, "1H");
					}
				    nuc1 = substring_for_keyword_in_string("NUC2", spectrum_report->str, spectrum_report->len);
				    if (nuc1 == NULL) {
					    nuc1 = malloc(strlen(nuc2) * sizeof(gchar));
					    strcpy(nuc1, nuc2);
				    }
					fnmode = substring_for_keyword_in_string("FnMode", spectrum_report->str, spectrum_report->len);
					if (fnmode == NULL) {
						fnmode = malloc(10 * sizeof(gchar));
						strcpy(fnmode, "undefined");
					}
					pulprog = substring_for_keyword_in_string("PULPROG", spectrum_report->str, spectrum_report->len);
					if (fnmode == NULL) {
						fnmode = malloc(10 * sizeof(gchar));
						strcpy(fnmode, "undefined");
					}
					g_string_append_printf(csv, "##TITLE= %s\n##JCAMPDX= 6.0  $$ Insensitive NMR JCAMP-DX v%s\n",
							   			   g_path_get_basename(filename), insensitive_version);
					if (window->showsFrequencyDomain) {
						if (insensitive_settings_get_showRealPart(window->controller->settings))
							dataset = window->displayedData.realp;
						else
							dataset = data.imagp;
					}
					g_string_append_printf(csv, "##DATA TYPE= nD NMR %s\n##DATA CLASS= NTUPLES\n##NUM DIM= 2\n", datatype);
					g_string_append(csv, "##ORIGIN= Insensitive\n##OWNER= Insensitive\n");
					date_str = g_date_time_format(date, "%Y/%m/%d %H:%M:%S%z");
					g_string_append_printf(csv, "##LONG DATE= %s\n", date_str);
					free(date_str);
					g_string_append_printf(csv, "##.OBSERVE FREQUENCY= %.2f\n##.OBSERVE NUCLEUS= ^%s\n", freq2, nuc2);
			        g_string_append_printf(csv, "##.DELAY= (0.0, 0.0)\n##.ACQUISITION MODE= SIMULTANEOUS (DQD)\n##.ACQUISITION SCHEME= %s\n", fnmode);
					g_string_append(csv, "##SPECTROMETER/DATA SYSTEM= Insensitive simulation\n");
				    g_string_append_printf(csv, "##.PULSE SEQUENCE= %s\n", pulprog);
					g_string_append(csv, "##.SOLVENT NAME= none\n");
					g_string_append_printf(csv, "##NTUPLES= nD NMR %s\n", datatype);
					g_string_append_printf(csv, "##VAR_NAME=  %s1, %12s2, %13s", x_scale, x_scale, datatype);
					if (window->showsFrequencyDomain) {
						g_string_append_printf(csv, "\n");
						g_string_append(csv, "##SYMBOL=    F1,            F2,              Y\n");
					} else {
					  	g_string_append_printf(csv, "/REAL, %10s/IMAG\n", datatype);
					  	g_string_append(csv, "##SYMBOL=    T1,            T2,              R,               I\n");
					}
					g_string_append_printf(csv, "##.NUCLEUS=  %s, %13s\n", nuc1, nuc2);
					g_string_append(csv, "##VAR_TYPE=  INDEPENDENT,   INDEPENDENT,     DEPENDENT");
					if (!window->showsFrequencyDomain)
						g_string_append(csv, ",       DEPENDENT");
					g_string_append(csv, "\n##VAR_FORM=  AFFN,          AFFN,            ASDF");
					if (!window->showsFrequencyDomain) {
						g_string_append(csv, ",            ASDF");
						g_string_append_printf(csv, "\n##VAR_DIM=   %d,%15d,%16d,%16d\n", t1DataPoints, t2DataPoints, t2DataPoints, t2DataPoints);
					} else {
						g_string_append_printf(csv, "\n##VAR_DIM=   %d,%14d,%16d\n", t1DataPoints, t2DataPoints, t2DataPoints);
					}
					g_string_append_printf(csv, "##UNITS=     %s, %13s, %23s", x_unit, x_unit, y_unit);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, ", %15s", y_unit);
					// Determine Min and Max values
					if (window->showsFrequencyDomain) {
						min_y1 = dataset[0];
						max_y1 = dataset[0];
						for (i = 1; i < t2DataPoints * t1DataPoints; i++) {
							if (min_y1 > dataset[i]) {
								min_y1 = dataset[i];
								if (fabsf(min_y1) > fabsf(max_y1))
									max_x1 = i;
							}
							if (max_y1 < dataset[i]) {
								max_y1 = dataset[i];
								if (fabsf(max_y1) > fabsf(min_y1))
									max_x1 = i;
							}
						}
						min_x1 = begin1 + freq1;
						min_x2 = begin2 + freq2;
					} else {
						dataset = data.realp;
						min_y1 = data.realp[0];
						min_y2 = data.imagp[0];
						max_y1 = data.realp[0];
						max_y2 = data.imagp[0];
						for (i = 1; i < t2DataPoints * t1DataPoints; i++) {
							if (min_y1 > data.realp[i]) {
								min_y1 = data.realp[i];
								if (fabsf(min_y1) > fabsf(max_y1))
									max_x1 = i;
							}
							if (max_y1 < data.realp[i]) {
								max_y1 = data.realp[i];
								if (fabsf(max_y1) > fabsf(min_y1))
									max_x1 = i;
							}
							if (min_y2 > data.imagp[i]) {
								min_y2 = data.imagp[i];
								if (fabsf(min_y2) > fabsf(max_y2))
									max_x2 = i;
							}
							if (max_y2 < data.imagp[i]) {
								max_y2 = data.imagp[i];
								if (fabsf(max_y2) > fabsf(min_y2))
									max_x2 = i;
							}
						}
						min_x1 = begin1;
						min_x2 = begin2;
					}
					max_x1 = begin1 + max_x1 * step1;
					max_x2 = begin2 + max_x2 * step2;
					factor_x1 = 1 / step1;
					factor_x2 = 1 / step2;
					factor_y1 = 1000000.0;
					if (!window->showsFrequencyDomain)
						factor_y2 = 1000000.0;
					g_string_append_printf(csv, "\n##FACTOR=    %.9f,%14.9f,%15.9f", factor_x1, factor_x2, 1 / factor_y1);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, ", %15.9f", 1 / factor_y2);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, "\n##FIRST=     %.9f,%14.9f,%17.9f,%16.9f", min_x1, min_x2, data.realp[0], data.imagp[0]);
					else
						g_string_append_printf(csv, "\n##FIRST=     %.9f,%14.9f,%17.9f", min_x1, min_x2, data.realp[0]);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, "\n##LAST=      %.9f,%15.9f,%15.9f,%16.9f", (t1DataPoints - 1) * step1, (t2DataPoints - 1) * step2, data.realp[t1DataPoints * t2DataPoints - 1], data.imagp[t1DataPoints * t2DataPoints - 1]);
					else
						g_string_append_printf(csv, "\n##LAST=      %.9f,%14.9f,%15.9f", freq1 + begin1 + (t1DataPoints - 1) * step1, freq2 + begin2 + (t2DataPoints - 1) * step2, dataset[t1DataPoints * t2DataPoints - 1]);
					g_string_append_printf(csv, "\n##MIN=       %.9f,%14.9f,%18.9f", min_x1, min_x2, min_y1);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, ",%16.9f", min_y2);
					g_string_append_printf(csv, "\n##MAX=       %.9f,%15.9f,%15.9f", (t1DataPoints - 1) * step1, (t2DataPoints - 1) * step2, max_y1);
					if (!window->showsFrequencyDomain)
						g_string_append_printf(csv, ",%16.9f", max_y2);
					g_string_append(csv, "\n");
					if (!window->showsFrequencyDomain) {
						for (p = 0; p < t1DataPoints; p++) {
							strcpy(z, "Real");
							g_string_append_printf(csv, "$$ %s data points\n##PAGE= T1=%f\n##FIRST=\t%f,\t%f,\t%f,\t%f\n##DATA TABLE= (T2++(%c..%c)), PROFILE\n", z, begin1 + p * step1, min_x1, min_x2, data.realp[p], data.imagp[p], *z, *z);
							for (i = 0; i < t2DataPoints; i += 10) {
								g_string_append_printf(csv, "%.0f", begin1 + i);
								for (j = 0; j < 10; j++) {
									if (i + j < t2DataPoints) {
										current = lroundf(data.realp[i + j + (p * t2DataPoints)] * factor_y1);
										sprintf(sqz, "%ld", current);
										if (current < 0) {
											// negative numbers: replace first digit by a, b, c, ... for -1, -2, -3, ...
											g_string_append_printf(csv, "%c%s", *(sqz + 1) + 48, sqz + 2);
										} else {
											// positive numbers: replace first digit by @, A, B, C, ... for 0, 1, 2, 3, ...
											g_string_append_printf(csv, "%c%s", *sqz + 16, sqz + 1);
										}
									}
								}
								g_string_append(csv, "\n");
							}
							strcpy(z, "Imaginary");
							g_string_append_printf(csv, "$$ %s data points\n##PAGE= T1=%f\n##FIRST=\t%f,\t%f,\t%f,\t%f\n##DATA TABLE= (T2++(%c..%c)), PROFILE\n", z, begin2 + p * step2, begin1, begin2, data.realp[p], data.imagp[p], *z, *z);
							for (i = 0; i < t2DataPoints; i += 10) {
								g_string_append_printf(csv, "%.0f", begin2 + i);
								for (j = 0; j < 10; j++) {
									if (i + j < t2DataPoints) {
										current = lroundf(data.imagp[i + j + (p * t2DataPoints)] * factor_y2);
										sprintf(sqz, "%ld", current);
										if (current < 0) {
											// negative numbers: replace first digit by a, b, c, ... for -1, -2, -3, ...
											g_string_append_printf(csv, "%c%s", *(sqz + 1) + 48, sqz + 2);
										} else {
											// positive numbers: replace first digit by @, A, B, C, ... for 0, 1, 2, 3, ...
											g_string_append_printf(csv, "%c%s", *sqz + 16, sqz + 1);
										}
									}
								}
								g_string_append(csv, "\n");
							}
						}
					} else {
						*z = 'Y';
						for (p = 0; p < t1DataPoints; p++) {
							g_string_append_printf(csv, "##PAGE= F1=%f\n##FIRST=\t%f,\t%f,\t%f\n##DATA TABLE= (F2++(%c..%c)), PROFILE\n", (begin1 + p * step1) + freq1, min_x1, min_x2, dataset[p], *z, *z);
							for (i = 0; i < t2DataPoints; i += 10) {
								g_string_append_printf(csv, "%.0f", ((begin1 + i * step2) + freq2) * factor_x2);
								for (j = 0; j < 10; j++) {
									if (i + j < t2DataPoints) {
									    // SQZ: Difference to previous value (first digit: @ = 0, A-I > 0, a-i < 0)
										current = lroundf(dataset[i + j + (p * t2DataPoints)] * factor_y1);
									    sprintf(sqz, "%ld", current);
										if (current < 0) {
										    // negative numbers: replace first digit by a, b, c, ... for -1, -2, -3, ...
											g_string_append_printf(csv, "%c%s", *(sqz + 1) + 48, sqz + 2);
										} else {
										    // positive numbers: replace first digit by @, A, B, C, ... for 0, 1, 2, 3, ...
											g_string_append_printf(csv, "%c%s", *sqz + 16, sqz + 1);
										}
									}
								}
								g_string_append(csv, "\n");
							}
						}
					}
					g_string_append_printf(csv, "##END TUPLES=nD NMR %s\n##END=", datatype);
					free(nuc2);
					free(fnmode);
					free(pulprog);
				} else {
					freq_str = substring_for_keyword_in_string("SFO1", spectrum_report->str, spectrum_report->len);
					if (freq_str == NULL)
						freq1 = spectrometer_frequency / 1e6;
					else {
						freq_str[strlen(freq_str) - 4] = '\0';
						freq1 = atof(freq_str);
						free(freq_str);
					}
				    nuc1 = substring_for_keyword_in_string("NUC1", spectrum_report->str, spectrum_report->len);
				    if (nuc1 == NULL) {
						nuc1 = malloc(3 * sizeof(gchar));
						strcpy(nuc1, "1H");
				    }
					g_string_append_printf(csv, "##TITLE= %s\n##JCAMPDX= 5.0  $$ Insensitive NMR JCAMP-DX v%s\n",
							       		   g_path_get_basename(filename), insensitive_version);
					g_string_append_printf(csv, "##DATA TYPE= NMR %s\n", datatype);
					if (window->showsFrequencyDomain)
						g_string_append(csv, "##DATA CLASS= XYDATA\n");
					else
						g_string_append(csv, "##DATA CLASS= NTUPLES\n");
					g_string_append(csv, "##ORIGIN= Insensitive\n##OWNER= Insensitive\n");
					g_string_append_printf(csv, "##.OBSERVE FREQUENCY= %.9f\n##.OBSERVE NUCLEUS= ^%s\n", freq1, nuc1);
					// Determine Min and Max values
					min_y1 = data.realp[0];
					min_y2 = data.imagp[0];
					max_y1 = data.realp[0];
					max_y2 = data.imagp[0];
					for (i = 1; i < t2DataPoints; i++) {
						if (min_y1 > data.realp[i]) {
							min_y1 = data.realp[i];
							if (fabsf(min_y1) > fabsf(max_y1))
								max_x1 = i;
						}
						if (max_y1 < data.realp[i]) {
							max_y1 = data.realp[i];
							if (fabsf(max_y1) > fabsf(min_y1))
								max_x1 = i;
						}
						if (!window->showsFrequencyDomain) {
							if (min_y2 > data.imagp[i])
								min_y2 = data.imagp[i];
							if (max_y2 < data.imagp[i])
								max_y2 = data.imagp[i];
						}
					}
					factor_x1 = 1 / step1;
					factor_y1 = 1000000000.0;
					// Pivot point for first order phase correction should be at the hightest peak in JCAMP-DX format!
					max_x1 = begin1 + max_x1 * step1;
					newPhase0 = window->zeroOrderPhaseShift + 0.5 * t2DataPoints * window->firstOrderPhaseShift * (0.1 * max_x1 - window->pivotPoint);
					g_string_append_printf(csv, "##.PHASE 0= %f\n##.PHASE 1= %f\n", newPhase0 * 180 / M_PI, window->firstOrderPhaseShift * 180 / M_PI);
					if (!window->showsFrequencyDomain) {
						// Export FID as complex Data...
						g_string_append(csv, "##.DELAY= (0.0, 0.0)\n##.ACQUISITION MODE= SIMULTANEOUS (DQD)\n");
						g_string_append(csv, "##SPECTROMETER/DATA SYSTEM= Insensitive simulation\n");
						g_string_append_printf(csv, "##NTUPLES= NMR %s\n", datatype);
						g_string_append_printf(csv, "##VAR_NAME=  %s, %12s/REAL, %10s/IMAG, %18s\n", x_scale, datatype, datatype, "PAGE NUMBER");
						g_string_append(csv, "##SYMBOL=    X,             R,               I,               N\n");
						g_string_append(csv, "##VAR_TYPE=  INDEPENDENT,   DEPENDENT,       DEPENDENT,       PAGE\n");
						g_string_append(csv, "##VAR_FORM=  AFFN,          ASDF,            ASDF,            AFFN\n");
						g_string_append_printf(csv, "##VAR_DIM=   %d, %13d,%16d,%13d\n", t2DataPoints, t2DataPoints, t2DataPoints, 2);
						g_string_append_printf(csv, "##UNITS=     %s, %21s, %15s,\n", x_unit, y_unit, y_unit);
						g_string_append_printf(csv, "##FIRST=     %.9f,%15.9f,%15.9f, %5d\n", begin1, data.realp[0], data.imagp[0], 1);
						g_string_append_printf(csv, "##LAST=      %.9f,%13.9f,%15.9f, %5d\n", begin1 + (t2DataPoints - 1) * step1, data.realp[t2DataPoints - 1], data.imagp[t2DataPoints - 1], 2);
						g_string_append_printf(csv, "##MIN=       %.9f,%15.9f,%15.9f, %5d\n", begin1, min_y1, min_y2, 1);
						g_string_append_printf(csv, "##MAX=       %.9f,%13.9f,%15.9f, %5d\n", begin1 + (t2DataPoints - 1) * step1, max_y1, max_y2, 2);
						g_string_append_printf(csv, "##FACTOR=    %.9f,%14.9f,%15.9f, %5d\n", factor_x1, 1 / factor_y1, 1 / factor_y1, 1);
						for (p = 1; p <= 2; p++) {
							if (p == 1) {
								strcpy(z, "Real");
								dataset = data.realp;
							} else {
								strcpy(z, "Imaginary");
								dataset = data.imagp;
							}
							g_string_append_printf(csv, "$$ %s data points\n##PAGE= N=%d\n##DATA TABLE= (X++(%c..%c)), XYDATA\n", z, p, *z, *z);
							if (dataset != NULL) {
								for (i = 0; i < t2DataPoints; i += 10) {
									g_string_append_printf(csv, "%.0f", begin1 + i);
									for (j = 0; j < 10; j++) {
										if (i + j < t2DataPoints) {
											current = lroundf(dataset[i + j] * factor_y1);
											sprintf(sqz, "%ld", current);
											if (current < 0) {
												// negative numbers: replace first digit by a, b, c, ... for -1, -2, -3, ...
												g_string_append_printf(csv, "%c%s", *(sqz + 1) + 48, sqz + 2);
											} else {
												// positive numbers: replace first digit by @, A, B, C, ... for 0, 1, 2, 3, ...
												g_string_append_printf(csv, "%c%s", *sqz + 16, sqz + 1);
											}
										}
									}
									g_string_append(csv, "\n");
								}
							}
						}
						g_string_append_printf(csv, "##END NTUPLES= NMR %s\n##END=", datatype);
					} else {
						// Export only real spectrum data
						g_string_append(csv, "##.ACQUISITION MODE= SIMULTANEOUS (DQD)\n");
						g_string_append(csv, "##SPECTROMETER/DATA SYSTEM= Insensitive simulation\n");
						g_string_append_printf(csv, "##RESOLUTION= %f\n", 1 / (step1 * (t2DataPoints - 1)));
						g_string_append_printf(csv, "##XUNITS= %s\n##YUNITS= %s\n", x_unit, y_unit);
						g_string_append_printf(csv, "##XFACTOR= %.9f\n##YFACTOR= %.9f\n", factor_x1, 1 / factor_y1);
						g_string_append_printf(csv, "##FIRSTX= %.9f\n##LASTX= %.9f\n", begin1 + freq1, (begin1 + (t2DataPoints - 1) * step1) + freq1);
						g_string_append_printf(csv, "##MAXY= %.9f\n##MINY= %.9f\n", max_y1, min_y1);
						g_string_append_printf(csv, "##DELTAX= %.9f\n##NPOINTS= %d\n##FIRSTY= %.9f\n", step1, t2DataPoints, data.realp[0]);
						g_string_append(csv, "##XYDATA= (X++(Y..Y))\n");
						if (data.realp != NULL) {
							for (i = 0; i < t2DataPoints; i += 10) {
								g_string_append_printf(csv, "%.0f", ((begin1 + i * step1) + freq1) * factor_x1);
								for (j = 0; j < 10; j++) {
									if (i + j < t2DataPoints) {
									    // SQZ: Difference to previous value (first digit: @ = 0, A-I > 0, a-i < 0)
										// DIF: Difference to previous value (first digit: % = 0, J-R > 0, j-r < 0)
										current = lroundf(data.realp[i + j] * factor_y1 /*+ 0.5*/);
										sprintf(sqz, "%ld", current);
										if (current < 0) {
										    // negative numbers: replace first digit by a, b, c, ... for -1, -2, -3, ...
											g_string_append_printf(csv, "%c%s", *(sqz + 1) + 48, sqz + 2);
										} else {
										    // positive numbers: replace first digit by @, A, B, C, ... for 0, 1, 2, 3, ...
											g_string_append_printf(csv, "%c%s", *sqz + 16, sqz + 1);
										}
									}
								}
								g_string_append(csv, "\n");
							}
						}
						g_string_append(csv, "##END=");
					}
				}
				free(nuc1);
				free(datatype);
				free(x_scale);
				free(x_unit);
				free(y_unit);
				free(sqz);
				free(z);
				break;
			case TXT:
				date_str = g_date_time_format(date, "%A, %B %e, %Y %l:%M:%S %p %Z");
				g_string_append_printf(csv, "# File created = %s\n", date_str);
				free(date_str);
				g_date_time_unref(date);
				g_string_append_printf(csv, "# Data set = insensitive  %d  1  %s\n# Spectral Region:\n", window->controller->expno, filename);
				if (window->twoDimensionalSpectrum) {
					if (window->domainOf2DSpectrum == FID) {
						g_string_append_printf(csv, "# T1LEFT = 0.0 s. T1RIGHT = %.15f s.\n",
								       begin1 + (t1DataPoints - 1) * step1);
						g_string_append_printf(csv, "# T2LEFT = 0.0 s. T2RIGHT = %.15f Hz.\n#\n",
								       begin2 + (t2DataPoints - 1) * step2);
					} else if (window->domainOf2DSpectrum == FFT1D) {
						g_string_append_printf(csv, "# T1LEFT = 0.0 s. T1RIGHT = %.15f s.\n",
								       begin1 + (t1DataPoints - 1) * step1);
						g_string_append_printf(csv, "# F2LEFT = %.15f Hz. F2RIGHT = %.15f Hz.\n#\n",
								       begin1,
								       begin2 + (t2DataPoints - 1) * step2);
					} else {
						g_string_append_printf(csv, "# F1LEFT = %.15f Hz. F1RIGHT = %.15f Hz.\n",
								       begin1,
								       begin1 + (t1DataPoints - 1) * step1);
						g_string_append_printf(csv, "# F2LEFT = %.15f Hz. F2RIGHT = %.15f Hz.\n#\n",
								       begin1,
								       begin2 + (t2DataPoints - 1) * step2);
					}
					g_string_append_printf(csv, "# NROWS = %d ( = number of points along the F1 axis)\n", t1DataPoints);
					g_string_append_printf(csv, "# NCOLS = %d ( = number of points along the F2 axis)\n#\n", t2DataPoints);
					g_string_append(csv, "# In the following ordering is from the 'left' to the 'right' limits!\n# Lines beginning with '#' must be considered as comment lines.\n#");
					if ((data.realp != NULL) && (data.imagp != NULL)) {
						for (j = 0; j < t1DataPoints; j++) {
							g_string_append_printf(csv, "\n# row = %d", j);
							for (i = 0; i < t2DataPoints; i++) {
								if (insensitive_settings_get_showImaginaryPart(window->controller->settings))
									g_string_append_printf(csv, "\n%f", data.imagp[j * t2DataPoints + i]);
								else
									g_string_append_printf(csv, "\n%f", data.realp[j * t2DataPoints + i]);
							}
						}
					}
				} else {
					if (window->showsFrequencyDomain)
						g_string_append_printf(csv, "# LEFT = %.15f Hz. RIGHT = %.15f Hz.\n#\n# SIZE = %d ( = number of points)\n",
								       		   begin1,
								               begin1 + (t2DataPoints - 1) * step1,
								               t2DataPoints);
					else
						g_string_append_printf(csv, "# LEFT = 0.0 s. RIGHT = %.15f s.\n#\n# SIZE = %d ( = number of points)\n",
							       			   begin1 + (t2DataPoints - 1) * step1,
							       			   t2DataPoints);
					g_string_append(csv, "#\n# In the following ordering is from the 'left' to the 'right' limits!\n# Lines beginning with '#' must be considered as comment lines.\n#");
					if ((data.realp != NULL) && (data.imagp != NULL)) {
						for (i = 0; i < t2DataPoints; i++) {
							g_string_append_printf(csv, "\n%f", data.realp[i]);
							if (data.imagp[i] >= 0)
								g_string_append(csv, "+");
							g_string_append_printf(csv, "%fi", data.imagp[i]);
						}
					}
				}
				break;
			default:
				if (window->twoDimensionalSpectrum) {
					if ((data.realp != NULL) && (data.imagp != NULL)) {
						g_string_append(csv, "2D spectrum");
						for (j = 0; j < t1DataPoints; j++)
							g_string_append_printf(csv, "%s%f", separator, begin1 + j * step1);
						for (i = 0; i < t2DataPoints; i++) {
							g_string_append_printf(csv, "\n%f", begin2 + i * step2);
							for (j = 0; j < t1DataPoints; j++) {
								if (insensitive_settings_get_showImaginaryPart(window->controller->settings))
									g_string_append_printf(csv, "%s%f", separator, data.imagp[j * t2DataPoints + i]);
								else
									g_string_append_printf(csv, "%s%f", separator, data.realp[j * t2DataPoints + i]);
							}
						}
					}
				} else {
					if (window->showsFrequencyDomain) {
						g_string_append_printf(csv, "frequency/Hz%sRe%sIm", separator, separator);
					} else {
						g_string_append_printf(csv, "time/s%sRe%sIm", separator, separator);
					}
					if ((data.realp != NULL) && (data.imagp != NULL)) {
						for (i = 0; i < t2DataPoints; i++)
							g_string_append_printf(csv, "\n%f%s%f%s%f", begin1 + i * step1, separator, data.realp[i], separator, data.imagp[i]);
					}
				}
			}
			if (csv->len > 0) {
				FILE *f = fopen(filename, "w");
				if (f == NULL) {
					dialog = gtk_message_dialog_new(GTK_WINDOW(window),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_OK,
									"Could not wrote spectrum to file %s.", filename);
					gtk_window_set_title(GTK_WINDOW(dialog), "Exporting spectrum data failed");
					gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);
				}
				fprintf(f, "%s", csv->str);
				fclose(f);
				g_string_free(csv, TRUE);
			}
		}
		g_object_unref(chooser);
	} else {
		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						"There is no spectrum data that can be saved.");
		gtk_window_set_title(GTK_WINDOW(dialog), "No spectrum acquired");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}


gboolean update_spectrum_parameter_panel(InsensitiveWindow *window)
{
	int i, channels, dataPoints;
	float LB;//, freq;
	GString *stringBeforeProcessing;
	gchar *wdw;
	GString *parameterString;
	enum WindowFunctionType windowFunction = insensitive_controller_get_windowFunction(window->controller);

	stringBeforeProcessing = insensitive_controller_get_spectrumReport(window->controller);
	if (stringBeforeProcessing != NULL) {
		parameterString = g_string_new(stringBeforeProcessing->str);
		wdw = malloc(6 * sizeof(gchar));
		switch (windowFunction) {
		case WFNone:
			strcpy(wdw, "   NO");
			break;
		case WFExp:
			strcpy(wdw, "   EM");
            break;
		case WFCosine:
		case WFSineBell:
		case WFHann:
		case WFWeightedHann:
			strcpy(wdw, " SINM");
			break;
		case WFTriangle:
			strcpy(wdw, " TRAP");
			break;
		case WFTraficante:
			strcpy(wdw, " TRAF");
			break;
		case WFTraficanteSN:
			strcpy(wdw, "TRAFS");
			break;
		case WFLorentzGaussTransformation:
		case WFGaussPseudoEchoTransformation:
			strcpy(wdw, "   GM");
			break;
		}
		if (window->twoDimensionalSpectrum)
			channels = 2;
		else
			channels = 1;
		for (i = 2; i > 2 - channels; i--) {
            if (i < 2)
                g_string_append(parameterString, "\n\n");
			g_string_append_printf(parameterString, "F%d - Processing parameters", i);
			if (window->twoDimensionalSpectrum && i == 1) {
				dataPoints = indirect_datapoints(insensitive_controller_get_detectionMethod(window->controller), insensitive_settings_get_dataPoints(window->controller->settings));
				//freq = 0.0;
			} else {
				dataPoints = insensitive_settings_get_dataPoints(window->controller->settings);
				//freq = 0.0;
			}
			g_string_append_printf(parameterString, "\nSI %20d", dataPoints);
			//g_string_append_printf(parameterString, "SF %20.2f MHz\n", freq);
			g_string_append_printf(parameterString, "\nWDW               %s", wdw);
			if (windowFunction == WFSineBell)
				g_string_append_printf(parameterString, "\nSSB %19.1g", 0.0);
			if (windowFunction == WFCosine)
				g_string_append_printf(parameterString, "\nSSB %19.1g", 2.0);
			if (windowFunction == WFTriangle) {
				g_string_append_printf(parameterString, "\nTM1 %19.1g", 0.0);
				g_string_append_printf(parameterString, "\nTM2 %19.1g", 1.0);
			}
			if (windowFunction == WFLorentzGaussTransformation || windowFunction == WFGaussPseudoEchoTransformation) {
				LB = 0.042 / (insensitive_controller_get_gaussianWidth(window->controller) * insensitive_settings_get_dwellTime(window->controller->settings));
				g_string_append_printf(parameterString, "\nLB %20.2f Hz", LB);
				g_string_append_printf(parameterString, "\nGB %20.2g", insensitive_controller_get_gaussianShift(window->controller));
			}
			if (windowFunction == WFTraficante || windowFunction == WFTraficanteSN) {
				LB = 1 / (2 * insensitive_settings_get_T2(window->controller->settings));
				g_string_append_printf(parameterString, "\nLB %20.2f Hz", LB);
			}
			if (!(window->twoDimensionalSpectrum && i == 1)) {
				if (window->domainOf2DSpectrum == 3)
					g_string_append(parameterString, "\nPH_mod               mc");
				else {
					g_string_append(parameterString, "\nPH_mod               pk");
					g_string_append_printf(parameterString, "\nPHC0 %18.2f", window->zeroOrderPhaseShift);
					g_string_append_printf(parameterString, "\nPHC1 %18.2f", window->firstOrderPhaseShift * 100);
				}
			}
			if (window->numberOfPeaks > 0 && !window->twoDimensionalSpectrum) {
				g_string_append_printf(parameterString, "\nPC %20.2f", insensitive_settings_get_signalToNoiseThreshold(window->controller->settings));
				g_string_append(parameterString, "\nPSIGN              both"); // pos, neg, both
			}
		}
		free(wdw);
	} else {
		parameterString = g_string_new("No spectrum report available");
	}
	gtk_text_buffer_set_text(window->spectrumParameters_textbuffer, parameterString->str, parameterString->len);
	g_string_free(parameterString, TRUE);

	return FALSE;
}


void show_spectrumParameters_textview(InsensitiveWindow *window, gboolean value)
{
	window->parametersVisible = !value;
	on_toggleParameters_button_clicked(window->toggleParameters_button, window);
}


G_MODULE_EXPORT void on_toggleParameters_button_clicked(GtkButton *button, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;

	if (window->parametersVisible) {
		window->parametersVisible = FALSE;
		gtk_widget_set_visible((GtkWidget *)window->spectrumParameters_textview, FALSE);
		gtk_button_set_label(button, "Show Parameters");
	} else {
		window->parametersVisible = TRUE;
		gtk_widget_set_visible((GtkWidget *)window->spectrumParameters_textview, TRUE);
		gtk_button_set_label(button, "Hide Parameters");
	}
}


void set_maxDataPoints(InsensitiveWindow *window, unsigned int value)
{
    unsigned int i;

    if(window->maxDataPoints != value) {
        if(window->phase.realp != NULL) {
            free(window->phase.realp);
            window->phase.realp = NULL;
        }
        if(window->phase.imagp != NULL) {
            free(window->phase.imagp);
            window->phase.imagp = NULL;
        }
        if(window->data.realp != NULL) {
            free(window->data.realp);
            window->data.realp = NULL;
        }
        if(window->data.imagp != NULL) {
            free(window->data.imagp);
            window->data.imagp = NULL;
        }
        if(window->displayedData.realp != NULL) {
            free(window->displayedData.realp);
            window->displayedData.realp = NULL;
        }
        if(window->displayedData.imagp != NULL) {
            free(window->displayedData.imagp);
            window->displayedData.imagp = NULL;
        }
        if(window->integral != NULL) {
            free(window->integral);
            window->integral = NULL;
        }
        window->maxDataPoints = value;
        window->data.realp = malloc(window->maxDataPoints * sizeof(float));
        window->data.imagp = malloc(window->maxDataPoints * sizeof(float));
        window->displayedData.realp = malloc(window->maxDataPoints * sizeof(float));
        window->displayedData.imagp = malloc(window->maxDataPoints * sizeof(float));
        window->integral = malloc(window->maxDataPoints * sizeof(float));
    }
    for(i = 0; i < window->maxDataPoints; i++) {
        window->data.realp[i] = 0;
        window->data.imagp[i] = 0;
        window->displayedData.realp[i] = 0;
        window->displayedData.imagp[i] = 0;
		window->integral[i] = 0;
    }
    if((window->phase.realp == NULL) || (window->phase.imagp == NULL))
        set_phase_correction(window, 0.0, 0.0, 0.0);
}


void set_phase_correction(InsensitiveWindow *window, float phase0, float phase1, float pivot)
{
    float angle;
    unsigned int i, j, directDataPoints;

    if(window->phase.realp != NULL)
        free(window->phase.realp);
    if(window->phase.imagp != NULL)
        free(window->phase.imagp);
    window->phase.realp = malloc(window->maxDataPoints * sizeof(float));
    window->phase.imagp = malloc(window->maxDataPoints * sizeof(float));
    if(window->twoDimensionalSpectrum) {
        directDataPoints = window->maxDataPoints / window->indirectDataPoints;
        for(j = 0; j < window->indirectDataPoints; j++)
            for(i = 0; i < directDataPoints; i++) {
                angle = phase0 + (i - directDataPoints * (0.5 + pivot)) * phase1;
                window->phase.realp[j * directDataPoints + i] = cos(angle);
                window->phase.imagp[j * directDataPoints + i] = sin(angle);
            }
    } else {
        for(i = 0; i < window->maxDataPoints; i++) {
            angle = phase0 + (i - window->maxDataPoints * (0.5 + pivot)) * phase1;
            window->phase.realp[i] = cos(angle);
            window->phase.imagp[i] = sin(angle);
        }
    }
    window->cursorX = pivot;
    g_idle_add((GSourceFunc)recalculate_graph, window);
}


void set_noise_spectrum(InsensitiveWindow *window, DSPSplitComplex splitComplex)
{
    window->noise.realp = splitComplex.realp;
    window->noise.imagp = splitComplex.imagp;
    recalculate_graph(window);
}


float magnification(InsensitiveWindow *window)
{
    return window->magnification;
}


void set_magnification(InsensitiveWindow *window, float value)
{
    window->magnification = value;
}


void reset_magnification(InsensitiveWindow *window)
{
    window->magnification = 1.0;
}


gboolean recalculate_graph(InsensitiveWindow *window)
{
    unsigned int i, x, y;
    float integralSizingFactor;
    DSPComplex z;

    // 2D spectra
    if(window->twoDimensionalSpectrum) {
        for(y = 0; y < window->indirectDataPoints; y++) {
            for(x = 0; x < window->lastDataPointDisplayed; x++) {
                z = complex_rect(window->data.realp[y * window->lastDataPointDisplayed + x],
                                 window->data.imagp[y * window->lastDataPointDisplayed + x]);
                z = complex_mul(z, complex_rect(window->phase.realp[x], window->phase.imagp[x]));
                if((window->apodizationT2 != NULL) && (window->apodizationT1 != NULL) && !window->showsFrequencyDomain) {
                    z.real *= window->apodizationT2[x] * window->apodizationT1[y];
                    z.imag *= window->apodizationT2[x] * window->apodizationT1[y];
                }
                window->displayedData.realp[y * window->lastDataPointDisplayed + x] = window->scaling * z.real;
                window->displayedData.imagp[y * window->lastDataPointDisplayed + x] = window->scaling * z.imag;
            }
        }
    // 1D-Spectra
    } else {
        if(window->maxDataPoints > 0) {
            window->baselineRe = 0.0;
            window->baselineIm = 0.0;
            for (i = 0; i < window->lastDataPointDisplayed; i++) {
                if ((window->noise.realp != NULL) && (window->noise.imagp != NULL))
                    z = complex_rect(window->data.realp[i] + window->noise.realp[i], window->data.imagp[i] + window->noise.imagp[i]);
                else
                    z = complex_rect(window->data.realp[i], window->data.imagp[i]);
                z = complex_mul(z, complex_rect(window->phase.realp[i], window->phase.imagp[i]));
                if ((window->apodizationT2 != NULL) && !window->showsFrequencyDomain) {
                    z.real *= window->apodizationT2[i];
                    z.imag *= window->apodizationT2[i];
                }
                window->displayedData.realp[i] = z.real;
                window->displayedData.imagp[i] = z.imag;
                // Prepare integral
                if(i == 0) {
                    window->integral[i] = 0;
                } else {
                    window->integral[i] = window->integral[i - 1] + z.real;
                }
                // Determine baseline
                if((window->noise.realp != NULL) && (window->noise.imagp != NULL)) {
                    if((fabsf(z.real) - window->noise.realp[i]) < window->baselineRe || window->baselineRe == 0)
                        window->baselineRe = z.real - window->noise.realp[i];
                    if((fabsf(z.imag) - window->noise.imagp[i]) < window->baselineIm || window->baselineIm == 0)
                        window->baselineIm = z.imag - window->noise.imagp[i];
                }
            }
            // Calculate integral
            integralSizingFactor = (window->integral[window->maxDataPoints - 1] <= window->maxDataPoints) ? 1 : 0.5 * window->maxDataPoints / window->integral[window->maxDataPoints - 1];
            for (i = 0; i < window->lastDataPointDisplayed; i++) {
                window->integral[i] -= (i + M_PI) * window->baselineRe;
                window->integral[i] *= integralSizingFactor;
            }
        }
    }
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);

	return FALSE;
}


DSPSplitComplex displayed_graph(InsensitiveWindow *window)
{
    return window->displayedData;
}


 //////  //////  ///    /// ///    ///  /////  ///    // //////      //      // ///    // ///////
//      //    // ////  //// ////  //// //   // ////   // //   //     //      // ////   // //
//      //    // // //// // // //// // /////// // //  // //   //     //      // // //  // /////
//      //    // //  //  // //  //  // //   // //  // // //   //     //      // //  // // //
 //////  //////  //      // //      // //   // //   //// //////      /////// // //   //// ///////

G_MODULE_EXPORT void execute_command(GtkEntry *entry, gpointer user_data)
{
	gchar *command, **word, *commandCopy;
	GRegex *regex1spin_xy, *regex1spin_z, *regex2spins, *regex3spins, *regex4spins;
	gboolean commandMatches1SpinOperator_xy, commandMatches1SpinOperator_z;
	gboolean commandMatches2SpinOperator, commandMatches3SpinOperator, commandMatches4SpinOperator;
	GError *err = NULL;
	GMatchInfo *matchInfo;
	unsigned int number_of_words, str_len;
	InsensitiveWindow *window = user_data;
	GtkWidget *dialog;

	// Tokenise the command string into
    commandCopy = malloc(strlen(gtk_entry_get_text(entry)) * sizeof(gchar));
    strcpy(commandCopy, gtk_entry_get_text(entry));
    g_strstrip(commandCopy);
    if (strlen(commandCopy) == 0)
		return;
    g_ptr_array_add(window->commandHistory, commandCopy);
    if (window->commandHistory->len > 250)
        free(g_ptr_array_remove_index(window->commandHistory, 0));
    command = g_ascii_strdown(gtk_entry_get_text(entry), -1);
    g_strstrip(command);
	word = g_strsplit_set(command, " \t", 4);
	for (number_of_words = 0; word[number_of_words] != NULL; number_of_words++);
	if (number_of_words == 4) {
		show_command_error((GtkWidget *)entry, window);
		gtk_entry_set_text(entry, "");
		return;
	}

	// Check if first word matches a product operator keyword
	regex1spin_xy = g_regex_new("\\b(i|s)[1-4]?(x|y)\\b", 0, 0, &err);
	g_regex_match(regex1spin_xy, word[0], 0, &matchInfo);
	commandMatches1SpinOperator_xy = g_match_info_matches(matchInfo) ? TRUE : FALSE;
	regex1spin_z = g_regex_new("\\b(i|s)[1-4]?z\\b", 0, 0, &err);
	g_regex_match(regex1spin_z, word[0], 0, &matchInfo);
	commandMatches1SpinOperator_z = g_match_info_matches(matchInfo) ? TRUE : FALSE;
	regex2spins = g_regex_new("\\b2(i|s)[1-4]z(i|s)[1-4]z\\b", 0, 0, &err);
	g_regex_match(regex2spins, word[0], 0, &matchInfo);
	commandMatches2SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;
	regex3spins = g_regex_new("\\b4(i|s)[1-4]z(i|s)[1-4]z(i|s)[1-4]z\\b", 0, 0, &err);
	g_regex_match(regex3spins, word[0], 0, &matchInfo);
	commandMatches3SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;
	regex4spins = g_regex_new("\\b8(i|s)[1-4]z(i|s)[1-4]z(i|s)[1-4]z(i|s)[1-4]z\\b", 0, 0, &err);
	g_regex_match(regex4spins, word[0], 0, &matchInfo);
	commandMatches4SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;

	// Translate pi and pi/2 keywords to 180 and 90
	if (number_of_words > 1) {
		str_len = sizeof word[0];
		g_regex_match(regex1spin_xy, word[1], 0, &matchInfo);
		if (g_match_info_matches(matchInfo)) {
			if (!g_strcmp0(word[0], "pi/4") || !g_strcmp0(word[0], "π/4"))
				g_strlcpy(word[0], "45", str_len);
			if (!g_strcmp0(word[0], "-pi/4") || !g_strcmp0(word[0], "-π/4"))
				g_strlcpy(word[0], "-45", str_len);
			if (!g_strcmp0(word[0], "pi/2") || !g_strcmp0(word[0], "π/2"))
				g_strlcpy(word[0], "90", str_len);
			if (!g_strcmp0(word[0], "-pi/2") || !g_strcmp0(word[0], "-π/2"))
				g_strlcpy(word[0], "-90", str_len);
			if (!g_strcmp0(word[0], "pi") || !g_strcmp0(word[0], "π"))
				g_strlcpy(word[0], "180", str_len);
			if (!g_strcmp0(word[0], "-pi") || !g_strcmp0(word[0], "-π"))
				g_strlcpy(word[0], "-180", str_len);
		}
	}

	// Execute command

	// ABOUT
	if (!g_strcmp0(word[0], "about") && number_of_words == 1) {
		on_about_menu_item_activate(NULL, NULL);
	}
	// ANIM
	else if (!g_strcmp0(word[0], "anim") && number_of_words <= 2) {
		if (number_of_words == 2) {
			if (!strcmp(word[1], "stop") || !strcmp(word[1], "halt")) {
				gtk_toggle_button_set_active(window->animation_checkbox, FALSE);
			} else if (!strcmp(word[1], "start")) {
				gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
			} else {
				gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
			}
		} else {
			gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
		}
	}
	// ANIMNR
	else if (!g_strcmp0(word[0], "animnr") && number_of_words <= 2) {
		if (number_of_words == 2) {
			if (!strcmp(word[1], "stop") || !strcmp(word[1], "halt")) {
				gtk_toggle_button_set_active(window->include_relaxation_checkbox, FALSE);
				gtk_toggle_button_set_active(window->animation_checkbox, FALSE);
			} else if (!strcmp(word[1], "start")) {
				gtk_toggle_button_set_active(window->include_relaxation_checkbox, FALSE);
				gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
			} else {
				gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
			}
		} else {
			gtk_toggle_button_set_active(window->include_relaxation_checkbox, FALSE);
			gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
		}
	}
	// ANIMR
	else if (!g_strcmp0(word[0], "animr") && number_of_words <= 2) {
		if (number_of_words == 2) {
			if (!strcmp(word[1], "stop") || !strcmp(word[1], "halt")) {
				gtk_toggle_button_set_active(window->include_relaxation_checkbox, TRUE);
				gtk_toggle_button_set_active(window->animation_checkbox, FALSE);
			} else if (!strcmp(word[1], "start")) {
				gtk_toggle_button_set_active(window->include_relaxation_checkbox, TRUE);
				gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
			} else {
				gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
			}
		} else {
			gtk_toggle_button_set_active(window->include_relaxation_checkbox, TRUE);
			gtk_toggle_button_set_active(window->animation_checkbox, TRUE);
		}
	}
	// APK
	else if (!g_strcmp0(word[0], "apk") && number_of_words == 1) {
        // automatic phase correction
	}
	// CS
	else if (!g_strcmp0(word[0], "cs") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		on_chemicalShift_button_clicked(window->chemicalShift_button, window);
	}
	// D
	else if (!g_strcmp0(word[0], "d") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"D [s] = %.2f", insensitive_settings_get_delay(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Delay time");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
	}
	// DOSY2D
	else if (!g_strcmp0(word[0], "dosy2d") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller) && shows_2D_spectrum(window)) {
            on_dosy_show_1D_trace_only_button_clicked(NULL, window);
            on_auto_peak_picking_button_clicked(NULL, window);
            on_dosy_fit_lorentzian_peaks_button_clicked(NULL, window);
            on_dosy_fit_button_clicked(NULL, window);
            on_dosy_spectrum_button_clicked(NULL, window);
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// DT
	else if (!g_strcmp0(word[0], "dt") && number_of_words == 1) {
        if(!shows_2D_spectrum(window)) {
            if(window->showsFrequencyDomain && insensitive_controller_get_spectrumDataAvailable(window->controller)) {
                insensitive_controller_calculate_first_derivative_of_1D_spectrum(window->controller);
            } else {
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                    GTK_DIALOG_DESTROY_WITH_PARENT,
							                    GTK_MESSAGE_INFO,
							                    GTK_BUTTONS_OK,
							                    "Process FID before calculating the first derivative of the spectral data.");
			    gtk_window_set_title(GTK_WINDOW(dialog), "1st Derivative of 1D Spectra");
			    gtk_dialog_run(GTK_DIALOG(dialog));
			    gtk_widget_destroy(dialog);
            }
        } else {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                GTK_DIALOG_DESTROY_WITH_PARENT,
							                GTK_MESSAGE_INFO,
							                GTK_BUTTONS_OK,
							                "The command \"%s\" can only be applied to 1D spectra.", word[0]);
			gtk_window_set_title(GTK_WINDOW(dialog), "1st Derivative of 2D Spectra");
		    gtk_dialog_run(GTK_DIALOG(dialog));
		    gtk_widget_destroy(dialog);
        }
	}
	// DW
	else if (!g_strcmp0(word[0], "dw") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"DW [s] = %.3f", insensitive_settings_get_dwellTime(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Dwell time");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->dwelltime_entry, word[1]);
			on_dwelltime_entry_activate(window->dwelltime_entry, window);
		}
	}
	// EDA
	else if (!g_strcmp0(word[0], "eda") && number_of_words == 1) {
		show_mainWindow_notebook_page(window, 1);
	}
	// EDC and NEW
	else if ((!g_strcmp0(word[0], "edc") || !g_strcmp0(word[0], "new")) && number_of_words == 1) {
        insensitive_controller_reset_acquisition_for_dataPoints(window->controller,
                                                                insensitive_settings_get_dataPoints(window->controller->settings));
        insensitive_controller_set_spectrumReport(window->controller, NULL);
        update_spectrum_parameter_panel(window);
	}
	// EDCPUL
	else if (!g_strcmp0(word[0], "edcpul") && number_of_words == 1) {
        show_mainWindow_notebook_page(window, 2);
        gtk_notebook_set_current_page(window->bottomDisplay_notebook, 2);
        gtk_combo_box_set_active((GtkComboBox *)window->bottomDisplay_combobox, 2);
	}
    // EDDOSY
    else if (!g_strcmp0(word[0], "eddosy") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller) && shows_2D_spectrum(window)) {
            gtk_widget_show((GtkWidget *)window->dosyToolBox_window);
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// EDPUL
	else if (!g_strcmp0(word[0], "edpul") && number_of_words == 1) {
        on_record_button_clicked(window->record_button, window);
	}
	// EF
	else if (!g_strcmp0(word[0], "ef") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
                gtk_combo_box_set_active(window->apodization_combobox, 1);
            } else
                alert_for_invalid_fourier_transform(window, 1);
        } else
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
	}
	// EFP
	else if (!g_strcmp0(word[0], "efp") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
                gtk_combo_box_set_active(window->apodization_combobox, 1);
            } else
                alert_for_invalid_fourier_transform(window, 1);
        } else
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
	}
	// EJ
	else if (!g_strcmp0(word[0], "ej") && number_of_words == 1) {
        on_reset_constants_button_clicked(window->reset_constants_button, window);
	}
	// EDSTRUC and ELDISP
	else if ((!g_strcmp0(word[0], "edstruc") || !g_strcmp0(word[0], "eldisp")) && number_of_words == 1) {
		show_mainWindow_notebook_page(window, 0);
	}
	// EM
	else if (!g_strcmp0(word[0], "em") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 1);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
	// EQ
	else if (!g_strcmp0(word[0], "eq") && number_of_words == 1) {
		insensitive_controller_return_to_thermal_equilibrium(window->controller);
	}
	// EXIT
	else if ((!g_strcmp0(word[0], "exit") || !g_strcmp0(word[0], "quit")) && number_of_words == 1) {
        gtk_window_close((GtkWindow *)window);
	}
    // EXPORTFILE or TOPNG
	else if ((!g_strcmp0(word[0], "exportfile") || !g_strcmp0(word[0], "topng")) && number_of_words == 1) {
        insensitive_settings_set_exportFormat(window->controller->settings, PNG);
        export_spectrum(NULL, window);
    }
	// EXPT
	else if (!g_strcmp0(word[0], "expt") && number_of_words == 1) {
		float seconds = fmodf(insensitive_controller_get_acquisitionTime(window->controller), 60);
		float minutes = (insensitive_controller_get_acquisitionTime(window->controller) - seconds) / 60;
		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_OK,
						"experiment time = %.0f min %.0f sec", minutes, seconds);
		gtk_window_set_title(GTK_WINDOW(dialog), "Experiment time");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	// FID and IFT
	else if ((!g_strcmp0(word[0], "fid") || !g_strcmp0(word[0], "ift")) && number_of_words == 1) {
        if (insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            on_fid_button_clicked(window->fid_button, window);
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// FMC
	else if (!g_strcmp0(word[0], "fmc") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window))
                on_magnitude_button_clicked(window->magnitude_button, window);
            else
                alert_for_invalid_fourier_transform(window, 1);
        } else
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
	}
	// FnMODE
	else if (!g_strcmp0(word[0], "fnmode") && number_of_words <= 2) {
        if(insensitive_controller_get_variableEvolutionTime(window->controller) != 0) {
            if(number_of_words == 1) {
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_OK,
						"FnMode = %s", gtk_combo_box_text_get_active_text(window->detectionMethod_combobox));
	            gtk_window_set_title(GTK_WINDOW(dialog), "Acquisition mode of the indirect dimension");
		        gtk_dialog_run(GTK_DIALOG(dialog));
		        gtk_widget_destroy(dialog);
            } else {
                if(strcmp(word[1], "none"))
                    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, 0);
                else if(strcmp(word[1], "states"))
                    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, 1);
                else if(strcmp(word[1], "tppi"))
                    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, 2);
                else if(strcmp(word[1], "states-tppi"))
                    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, 3);
                else
                    gtk_combo_box_set_active((GtkComboBox *)window->detectionMethod_combobox, 0);
            }
        }
	}
	// FP
	else if (!g_strcmp0(word[0], "fp") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
                // phase correction missing!
            } else {
                alert_for_invalid_fourier_transform(window, 1);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// FT
	else if (!g_strcmp0(word[0], "ft") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
            } else {
                alert_for_invalid_fourier_transform(window, 1);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// GB
	else if (!g_strcmp0(word[0], "gb") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_INFO,
						GTK_BUTTONS_OK,
						"GB = %-3f", gtk_adjustment_get_value(window->gaussianShift_adjustment));
	            gtk_window_set_title(GTK_WINDOW(dialog), "Gaussian max. position for gm");
		        gtk_dialog_run(GTK_DIALOG(dialog));
		        gtk_widget_destroy(dialog);
        } else {
            gtk_adjustment_set_value(window->gaussianShift_adjustment, atof(word[1]));
        }
	}
	// GF
	else if (!g_strcmp0(word[0], "gf") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
                gtk_combo_box_set_active(window->apodization_combobox, 9);
            } else {
                alert_for_invalid_fourier_transform(window, 1);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// GFP
	else if (!g_strcmp0(word[0], "gfp") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                on_fft1D_button_clicked(window->fft1D_button, window);
                gtk_combo_box_set_active(window->apodization_combobox, 9);
                // missing phase correction
            } else {
                alert_for_invalid_fourier_transform(window, 1);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// GM
	else if (!g_strcmp0(word[0], "gm") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 9);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
	// GPL
	else if (!g_strcmp0(word[0], "gpl") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"GPL = %.3f", insensitive_settings_get_gradientStrength(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Gradient pulse power level");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->gradient_strength_entry, word[1]);
			on_gradient_strength_entry_activate(window->gradient_strength_entry, window);
		}
	}
	// GPT
	else if (!g_strcmp0(word[0], "gpt") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"GPT [ms] = %.3f", insensitive_settings_get_gradientDuration(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Gradient pulse duration");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->gradient_duration_entry, word[1]);
			on_gradient_duration_entry_activate(window->gradient_duration_entry, window);
		}
	}
	// HELP
	else if (!g_strcmp0(word[0], "help")) {
		if (number_of_words < 2)
			on_tutorial_toolbutton_clicked(NULL, window);
		else {
			const gchar * const *dirs = g_get_system_data_dirs();
			gchar *filename, **keyword, *html_file = NULL;
			gchar *line_buffer, *next_line, *text_buffer = NULL;
			gsize text_buffer_len;
			GError *err = NULL;
			//size_t n = 0;
			unsigned int i, keyword_index;
			GString *search_string;
			gboolean keyword_found = FALSE;

			search_string = g_string_new(word[1]);
			for (i = 2; i < number_of_words; i++)
				g_string_append_printf(search_string, "_%s", word[i]);
			while (insensitive_g_string_replace(search_string, "-", "_", search_string));
			g_string_ascii_down(search_string);
			while (*dirs != NULL && !keyword_found) {
    			filename = g_build_filename(*dirs++, "insensitive", "doc", "help_keywords", NULL);
				if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
					if (g_file_get_contents(filename, &text_buffer, &text_buffer_len, &err)) {
						line_buffer = text_buffer;
						while (line_buffer && !keyword_found) {
  							next_line = strchr(line_buffer, '\n');
  							if (next_line)
							  *next_line = '\0';  // temporarily terminate the current line
							keyword = g_strsplit_set(line_buffer, " \t\n", 20);
							for (keyword_index = 1; keyword[keyword_index] != NULL; keyword_index++) {
								if (!strcmp(keyword[keyword_index], search_string->str)) {
									keyword_found = TRUE;
									html_file = malloc(256 * sizeof(gchar));
									strcpy(html_file, keyword[0]);
									break;
								}
							}
							g_strfreev(keyword);
  							if (next_line)
							  *next_line = '\n';  // then restore newline-char, just to be tidy
  							line_buffer = next_line ? (next_line + 1) : NULL;
						}
						g_free(text_buffer);
					}
				}
				g_free(filename);
			}
			g_string_free(search_string, TRUE);

			if (keyword_found) {
#ifdef USE_WEBKIT_GTK
				InsensitiveTutorial *tutorial_window = g_object_new(INSENSITIVE_TYPE_TUTORIAL,
		                       										"default-width", 600,
		                       										"default-height", 700,
		                       										NULL);

				gtk_window_set_title((GtkWindow *)tutorial_window, "Tutorial");
				gtk_window_present((GtkWindow *)tutorial_window);
				load_arbitrary_page(html_file, tutorial_window);
#else
                gchar *filename, *url;
                const gchar * const *dirs = g_get_system_data_dirs();
                GError *error = NULL;

                while (*dirs != NULL) {
                    filename = g_build_filename(*dirs++, "insensitive", "doc", html_file, NULL);
                    if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
                        url = malloc((strlen(filename) + 7) * sizeof(gchar));
                        strcpy(url, "file://");
                        strcat(url, filename);
			            gtk_show_uri_on_window(GTK_WINDOW(window), url, GDK_CURRENT_TIME, &error);
                        if (error != NULL) {
                            fprintf (stderr, "Error: %s\n", error->message);
                            g_error_free (error);
                        }
                        if (url != NULL)
                            g_free(url);
		                if (filename != NULL)
                            g_free(filename);
                        break;
                    }
	            }
#endif /* USE_WEBKIT_GTK */
                if (html_file != NULL)
                    g_free(html_file);
			} else {
				dialog = gtk_message_dialog_new(GTK_WINDOW(window),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_INFO,
												GTK_BUTTONS_OK,
												"The entered keyword or phrase was not found in the help index database.");
	    		gtk_window_set_title(GTK_WINDOW(dialog), "Unknown keyword");
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
			}
		}
	}
	// IJ
	else if (!g_strcmp0(word[0], "ij") && number_of_words == 1) {
        GtkWidget *chooser;
        GtkFileFilter *filter;

        chooser = gtk_file_chooser_dialog_new("Open Spin System...",
                                              (GtkWindow *)window,
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              "Cancel", GTK_RESPONSE_CANCEL,
                                              "Open", GTK_RESPONSE_ACCEPT,
                                              NULL);
        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Insensitive spin system (ISS)");
        gtk_file_filter_add_pattern(filter, "*.iss");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
        choose_file(chooser, window);
	}
	// INT
	else if (!g_strcmp0(word[0], "int") && number_of_words == 1) {
        if(!shows_2D_spectrum(window))
            gtk_toggle_button_set_active(window->integral_checkbox, !gtk_toggle_button_get_active(window->integral_checkbox));
	}
	// JC
	else if (!g_strcmp0(word[0], "jc") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		insensitive_controller_perform_coupling_animated(window->controller, TRUE);
	}
	// LB
	else if (!g_strcmp0(word[0], "lb") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						    GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    "LB = %.3f", atof(gtk_entry_get_text(window->gaussianWidth_entry)));
	        gtk_window_set_title(GTK_WINDOW(dialog), "Gaussian width for gm");
		    gtk_dialog_run(GTK_DIALOG(dialog));
		    gtk_widget_destroy(dialog);
        } else {
            gtk_entry_set_text(window->gaussianWidth_entry, word[1]);
            on_gaussian_window_function_adjusted(window->gaussianWidth_entry, window);
        }

	}
	// MCDISP
	else if (!g_strcmp0(word[0], "mcdisp") && number_of_words == 1) {
        on_matrix_composer_toolbutton_clicked(NULL, window);
	}
	// NS
	else if (!g_strcmp0(word[0], "ns") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						                    GTK_DIALOG_DESTROY_WITH_PARENT,
						                    GTK_MESSAGE_INFO,
						                    GTK_BUTTONS_OK,
						                    "NS = %d", insensitive_controller_get_numberOfPhaseCycles(window->controller));
	        gtk_window_set_title(GTK_WINDOW(dialog), "Number of scans");
		    gtk_dialog_run(GTK_DIALOG(dialog));
		    gtk_widget_destroy(dialog);
        } else {
            int ns = atoi(word[1]);
            if(ns > 0) {
                gchar *str = malloc(11 * sizeof(gchar));
                sprintf(str, "%d", ns);
                insensitive_controller_interrupt_coherencePathway_calculation(window->controller);
                insensitive_controller_add_number_of_phase_cycles(window->controller, ns - insensitive_controller_get_numberOfPhaseCycles(window->controller));
                display_pulseProgram_code(window);
                gtk_entry_set_text(window->phaseCycles_entry, str);
                free(str);
            }
        }
	}
	// NUC1
	else if (!g_strcmp0(word[0], "nuc1") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"NUC1 = %s", gtk_combo_box_text_get_active_text(window->gyroI_combobox));
			gtk_window_set_title(GTK_WINDOW(dialog), "Nucleus for I-spins");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            if(!strcmp(word[1], "1h"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 0);
            else if(!strcmp(word[1], "13c"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 1);
            else if(!strcmp(word[1], "15n"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 2);
            else if(!strcmp(word[1], "19f"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 3);
            else if(!strcmp(word[1], "29si"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 4);
            else if(!strcmp(word[1], "31p"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 5);
            else if(!strcmp(word[1], "57fe"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 6);
            else if(!strcmp(word[1], "77se"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 7);
            else if(!strcmp(word[1], "113cd"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 8);
            else if(!strcmp(word[1], "119sn"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 9);
            else if(!strcmp(word[1], "129xe"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 10);
            else if(!strcmp(word[1], "183w"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 11);
            else if(!strcmp(word[1], "195pt"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 12);
            else
                gtk_combo_box_set_active((GtkComboBox *)window->gyroI_combobox, 0);
            on_gyro_combobox_changed(window->gyroI_combobox, window);
        }
	}
	// NUC2
	else if (!g_strcmp0(word[0], "nuc2") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"NUC2 = %s", gtk_combo_box_text_get_active_text(window->gyroS_combobox));
			gtk_window_set_title(GTK_WINDOW(dialog), "Nucleus for S-spins");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            if(!strcmp(word[1], "1h"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 0);
            else if(!strcmp(word[1], "13c"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 1);
            else if(!strcmp(word[1], "15n"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 2);
            else if(!strcmp(word[1], "19f"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 3);
            else if(!strcmp(word[1], "29si"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 4);
            else if(!strcmp(word[1], "31p"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 5);
            else if(!strcmp(word[1], "57fe"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 6);
            else if(!strcmp(word[1], "77se"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 7);
            else if(!strcmp(word[1], "113cd"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 8);
            else if(!strcmp(word[1], "119sn"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 9);
            else if(!strcmp(word[1], "129xe"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 10);
            else if(!strcmp(word[1], "183w"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 11);
            else if(!strcmp(word[1], "195pt"))
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 12);
            else
                gtk_combo_box_set_active((GtkComboBox *)window->gyroS_combobox, 0);
            on_gyro_combobox_changed(window->gyroS_combobox, window);
        }
	}
	// O1
	else if (!g_strcmp0(word[0], "o1") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"O1 [Hz] = %.3f", insensitive_settings_get_pulseFrequency(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Transmitter frequency offset");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->pulseFrequency_entry, word[1]);
			on_pulseFrequency_entry_activate(window->pulseFrequency_entry, window);
		}
	}
	// OPEN
	else if (!g_strcmp0(word[0], "open") && number_of_words == 1) {
        on_open_file_menuitem_activate(NULL, window);
        // used to be open spectrum only!
	}
	// PA
	else if (!g_strcmp0(word[0], "pa") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"PA [degree] = %.3f", insensitive_settings_get_flipAngle(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Pulse flip angle");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->flipAngle_entry, word[1]);
			on_flipAngle_entry_activate(window->flipAngle_entry, window);
		}
	}
	// PFG
	else if (!g_strcmp0(word[0], "pfg") && number_of_words == 1) {
		on_gradient_button_clicked(window->gradient_button, window);
	}
	// PH
	else if (!g_strcmp0(word[0], "ph") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"PH [Hz] = %.3f", insensitive_settings_get_phase(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Transmitter phase shift");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->phase_entry, word[1]);
			on_phase_entry_activate(window->phase_entry, window);
		}
	}
	// PHC0
	else if (!g_strcmp0(word[0], "phc0") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                GTK_DIALOG_DESTROY_WITH_PARENT,
							                GTK_MESSAGE_INFO,
							                GTK_BUTTONS_OK,
							                "PHC0 [degrees] = %.3f", get_zero_order_phase(window) / M_PI * 180);
			gtk_window_set_title(GTK_WINDOW(dialog), "0th order phase correction for pk");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            gtk_adjustment_set_value(window->zeroOrder_adjustment, atof(word[1]));
        }
	}
	// PHC1
	else if (!g_strcmp0(word[0], "phc1") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                GTK_DIALOG_DESTROY_WITH_PARENT,
							                GTK_MESSAGE_INFO,
							                GTK_BUTTONS_OK,
							                "PHC1 [degrees] = %.3f", get_first_order_phase(window) / M_PI * 180);
			gtk_window_set_title(GTK_WINDOW(dialog), "1st order phase correction for pk");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            gtk_adjustment_set_value(window->firstOrder_adjustment, atof(word[1]));
        }
	}
	// PK, PHASE
	else if ((!g_strcmp0(word[0], "pk") || !g_strcmp0(word[0], "phase")) && number_of_words == 1) {
		// phase correction
	}
	// PL
	else if (!g_strcmp0(word[0], "pl") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"PL [Hz] = %.3f", insensitive_settings_get_pulseStrength(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Pulse power level");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->pulseStrength_entry, word[1]);
			on_pulseStrength_entry_activate(window->pulseStrength_entry, window);
		}
	}
	// PLOT, XWINPLOT
	else if ((!g_strcmp0(word[0], "plot") || !g_strcmp0(word[0], "xwinplot") || !g_strcmp0(word[0], "xwp")) && number_of_words == 1) {
		show_mainWindow_notebook_page(window, 3);
	}
	// PP, PPD
	else if ((!g_strcmp0(word[0], "pp") || !g_strcmp0(word[0], "ppd")) && number_of_words == 1) {
        on_auto_peak_picking_button_clicked(NULL, window);
	}
	// PREC
	else if (!g_strcmp0(word[0], "prec") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		on_freeEvolution_button_clicked(window->freeEvolution_button, window);
	}
	// PRECNR
	else if (!g_strcmp0(word[0], "precnr") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		gtk_toggle_button_set_active(window->include_relaxation_checkbox, FALSE);
		on_freeEvolution_button_clicked(window->freeEvolution_button, window);
	}
	// PRECR
	else if (!g_strcmp0(word[0], "precr") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		gtk_toggle_button_set_active(window->include_relaxation_checkbox, TRUE);
		on_freeEvolution_button_clicked(window->freeEvolution_button, window);
	}
	// PT
	else if (!g_strcmp0(word[0], "pt") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"PT [ms] = %.3f", insensitive_settings_get_pulseDuration(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Pulse duration");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->pulseDuration_entry, word[1]);
			on_pulseDuration_entry_activate(window->pulseDuration_entry, window);
		}
	}
	// PTILT, TILT
	else if ((!g_strcmp0(word[0], "ptilt") || !g_strcmp0(word[0], "tilt")) && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller) && shows_2D_spectrum(window)) {
            insensitive_controller_tilt_2D_spectrum(window->controller, window->domainOf2DSpectrum, 0);
        }
	}
	// PTILT1
	else if (!g_strcmp0(word[0], "ptilt1") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller) && shows_2D_spectrum(window)) {
            insensitive_controller_tilt_2D_spectrum(window->controller, window->domainOf2DSpectrum, 1);
        }
	}
	// PULPROG
	else if (!g_strcmp0(word[0], "pulprog") && number_of_words == 1) {
        GtkWidget *chooser;
        GtkFileFilter *filter;

        chooser = gtk_file_chooser_dialog_new("Open Pulse Program...",
                                              (GtkWindow *)window,
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              "Cancel", GTK_RESPONSE_CANCEL,
                                              "Open", GTK_RESPONSE_ACCEPT,
                                              NULL);
        filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Insensitive pulse programs (IPP)");
        gtk_file_filter_add_pattern(filter, "*.ipp");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
        choose_file(chooser, window);
	}
	// PULSE
	else if (!g_strcmp0(word[0], "pulse") && number_of_words <= 3) {
		if (number_of_words == 3) {
			if (!g_strcmp0(word[2], "x"))
				set_phase(window, 0.0);
			else if (!g_strcmp0(word[2], "y"))
				set_phase(window, 90.0);
			else if (!g_strcmp0(word[2], "-x"))
				set_phase(window, 180.0);
			else if (!g_strcmp0(word[2], "-y"))
				set_phase(window, 270.0);
			else
				set_phase(window, atof(word[2]));
			on_phase_entry_activate(window->phase_entry, window);
		}
		if (number_of_words >= 2) {
			if (!g_strcmp0(word[1], "pi") || !g_strcmp0(word[1], "π"))
				set_flipAngle(window, 180.0);
			else if (!g_strcmp0(word[1], "pi/2") || !g_strcmp0(word[1], "π/2"))
				set_flipAngle(window, 90.0);
			else if (!g_strcmp0(word[1], "pi/4") || !g_strcmp0(word[1], "π/4"))
				set_flipAngle(window, 45.0);
			else
				set_flipAngle(window, atof(word[1]));
			on_flipAngle_entry_activate(window->flipAngle_entry, window);
		}
		on_pulse_button_clicked(window->pulse_button, user_data);
	}
	// RECORD
	else if (!g_strcmp0(word[0], "record") && number_of_words == 1) {
        on_record_button_clicked(window->record_button, window);
	}
	// RELAX
	else if (!g_strcmp0(word[0], "relax") && number_of_words <= 2) {
		if (number_of_words == 2) {
			gtk_entry_set_text(window->delay_entry, word[1]);
			on_delay_entry_activate(window->delay_entry, window);
		}
		on_relaxation_button_clicked(window->relaxation_button, window);
	}
    // RESET
	else if (!g_strcmp0(word[0], "reset") && number_of_words == 1) {
	    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                            		    GTK_MESSAGE_QUESTION,
                            			GTK_BUTTONS_NONE,
                            			"Do you want to reset all parameters and settings to the default values?");
		gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                  			   "Cancel", 0,
							   "Reset", 1, NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), "Reset all settings?");
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == 1) {
		    InsensitivePreferences *preferences;
		    preferences = g_object_new(INSENSITIVE_TYPE_PREFERENCES, NULL);
			insensitive_preferences_set_controller(preferences, window);
			on_reset_button_clicked(NULL, preferences);
		    gtk_widget_destroy(GTK_WIDGET(preferences));
		}
		gtk_widget_destroy(dialog);
	}
	// SETRES
	else if (!g_strcmp0(word[0], "setres") && number_of_words == 1) {
        on_preferences_menuitem_activate(NULL, window);
	}
	// SF
	else if (!g_strcmp0(word[0], "sf") && number_of_words == 1) {
        unsigned int i;
        unsigned int numberOfElements = insensitive_pulsesequence_get_number_of_elements(window->controller->pulseSequence);
        float freq1, freq2;
        SequenceElement *element;
        gboolean pulseSequenceLoaded = FALSE, twoChannelsActive = FALSE;

        if(numberOfElements > 0) {
            element = insensitive_pulsesequence_get_last_element(window->controller->pulseSequence);
            if(element->type == SequenceTypeFID)
                pulseSequenceLoaded = TRUE;
        }
        if(!pulseSequenceLoaded) {
            if(insensitive_settings_get_detectSSpins(window->controller->settings)) {
                freq1 = window->controller->spinSystem->absGyroS / gyro_1H * spectrometer_frequency / 1e6;
            } else {
                freq1 = window->controller->spinSystem->absGyroI / gyro_1H * spectrometer_frequency / 1e6;
            }
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"SF [MHz] = %.5f", freq1);
			gtk_window_set_title(GTK_WINDOW(dialog), "Spectrometer frequency");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            if(element->activeSSpins) {
                freq1 = window->controller->spinSystem->absGyroS / gyro_1H * spectrometer_frequency / 1e6;
                for(i = 0; i < numberOfElements; i++) {
                    element = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i);
                    if((element->type == SequenceTypePulse && element->activeISpins)) {
                        twoChannelsActive = TRUE;
                        break;
                    }
                }
                freq2 = (twoChannelsActive) ? (window->controller->spinSystem->absGyroI / gyro_1H * spectrometer_frequency / 1e6) : freq1;
            } else {
                freq1 = window->controller->spinSystem->absGyroI / gyro_1H * spectrometer_frequency / 1e6;
                for (i = 0; i < numberOfElements; i++) {
                    element = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, i);
                    if((element->type == SequenceTypePulse && element->activeSSpins)) {
                        twoChannelsActive = TRUE;
                        break;
                    }
                }
                freq2 = (twoChannelsActive) ? (window->controller->spinSystem->absGyroS / gyro_1H * spectrometer_frequency / 1e6) : freq1;
            }
            if(twoChannelsActive) {
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                    GTK_DIALOG_DESTROY_WITH_PARENT,
							                    GTK_MESSAGE_INFO,
							                    GTK_BUTTONS_OK,
							                    "SFO1 [MHz] = %.5f\nSFO2 [MHz] = %.5f", freq1, freq2);
			    gtk_window_set_title(GTK_WINDOW(dialog), "Spectrometer frequency (F1, F2)");
            } else {
                dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                    GTK_DIALOG_DESTROY_WITH_PARENT,
							                    GTK_MESSAGE_INFO,
							                    GTK_BUTTONS_OK,
							                    "SF [MHz] = %.5f", freq1);
			    gtk_window_set_title(GTK_WINDOW(dialog), "Spectrometer frequency");
            }
            gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        }
	}
	// SI
	else if (!g_strcmp0(word[0], "si") && number_of_words <= 2) {
        if(number_of_words == 1) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"SI = %d", insensitive_settings_get_dataPoints(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Size of real spectrum");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
        } else {
            int value = atoi(word[1]);
            gtk_spin_button_set_value(window->datapoints_spinbutton, (gdouble)value);
        }
	}
	// SINM
	else if (!g_strcmp0(word[0], "sinm") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 5);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
	// SP
	else if (!g_strcmp0(word[0], "sp") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							                GTK_DIALOG_DESTROY_WITH_PARENT,
							                GTK_MESSAGE_INFO,
							                GTK_BUTTONS_OK,
							                "SP = %s", gtk_combo_box_text_get_active_text(window->pulseEnvelope_combobox));
			gtk_window_set_title(GTK_WINDOW(dialog), "Shaped pulse waveform");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			// RECT, SQUA, GAUSS, SINC, EBURP1, EBURP2, IBURP1, IBURP2, UBURP, REBURP, and DANTE
			if (!strcmp(word[1], "gaus") || !strcmp(word[1], "gauss"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 1);
			else if (!strcmp(word[1], "sinc"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 2);
			else if (!strcmp(word[1], "eburp1") || !strcmp(word[1], "eburp-1") || !strcmp(word[1], "e-burp1"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 3);
			else if (!strcmp(word[1], "eburp2") || !strcmp(word[1], "eburp-2") || !strcmp(word[1], "e-burp2"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 4);
			else if (!strcmp(word[1], "iburp1") || !strcmp(word[1], "iburp-1") || !strcmp(word[1], "i-burp1"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 5);
			else if (!strcmp(word[1], "iburp2") || !strcmp(word[1], "iburp-2") || !strcmp(word[1], "i-burp2"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 6);
			else if (!strcmp(word[1], "uburp") || !strcmp(word[1], "u-burp"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 7);
			else if (!strcmp(word[1], "reburp") || !strcmp(word[1], "re-burp"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 8);
			else if (!strcmp(word[1], "dante"))
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 9);
			else {
				if (strcmp(word[1], "rect") || strcmp(word[1], "squa") || strcmp(word[1], "rectangle"))
					gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
				gtk_combo_box_set_active((GtkComboBox *)window->pulseEnvelope_combobox, 0);
			}
			//on_pulseEnvelope_combobox_changed(window->pulseEnvelope_combobox, window);
		}
	}
	// SPDISP, PPG
	else if ((!g_strcmp0(word[0], "spdisp") || !g_strcmp0(word[0], "ppg")) && number_of_words == 1) {
		show_mainWindow_notebook_page(window, 2);
	}
	// SSDISP
	else if (!g_strcmp0(word[0], "ssdisp") && number_of_words == 1) {
        on_single_spins_toolbutton_clicked(NULL, window);
	}
	// STEP
	else if (!g_strcmp0(word[0], "step") && number_of_words == 1) {
        on_step_button_clicked(window->step_button, window);
	}
	// STDISP
	else if (!g_strcmp0(word[0], "stdisp") && number_of_words == 1) {
        on_pulse_shaper_toolbutton_clicked(NULL, window);
	}
	// STOP, HALT
	else if ((!g_strcmp0(word[0], "halt") || !g_strcmp0(word[0], "stop")) && number_of_words == 1) {
		if (insensitive_controller_get_acquisitionTime(window->controller)) {
			on_acquire_button_clicked(window->acquire_button, window);
		} else {
			gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
		}
	}
	// SW, SWH
	else if ((!g_strcmp0(word[0], "sw") || !g_strcmp0(word[0], "swh")) && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"SW [Hz] = %.3f", 1 / insensitive_settings_get_dwellTime(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Sweep width");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			insensitive_controller_set_dwellTime(window->controller, 1 / atof(word[1]));
		}
	}
	// SYM
	else if (!g_strcmp0(word[0], "sym") && number_of_words == 1) {
        on_sym_menuitem_activate(NULL, window);
	}
	// SYMA
	else if (!g_strcmp0(word[0], "syma") && number_of_words == 1) {
        on_syma_menuitem_activate(NULL, window);
	}
    // SYMJ
    else if (!g_strcmp0(word[0], "symj") && number_of_words == 1) {
        on_symj_menuitem_activate(NULL, window);
    }
	// T1
	else if (!g_strcmp0(word[0], "t1") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"T1 [s] = %.3f", insensitive_settings_get_T1(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "T1 relaxation time");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->T1_entry, word[1]);
			on_T1_entry_activate(window->T1_entry, window);
		}
	}
	// T2
	else if (!g_strcmp0(word[0], "t2") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"T2 [s] = %.3f", insensitive_settings_get_T2(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "T2 relaxation time");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->T2_entry, word[1]);
			on_T2_entry_activate(window->T2_entry, window);
		}
	}
	// TC
	else if (!g_strcmp0(word[0], "tc") && number_of_words <= 2) {
		if (number_of_words == 1) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"TC [ns] = %.3f", insensitive_settings_get_correlationTime(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Correlation time");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			gtk_entry_set_text(window->correlationTime_entry, word[1]);
			on_correlationTime_entry_activate(window->correlationTime_entry, window);
		}
	}
	// TD
	else if (!g_strcmp0(word[0], "td") && number_of_words == 1) {
        if(insensitive_controller_get_variableEvolutionTime(window->controller) == 0) {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						                    GTK_DIALOG_DESTROY_WITH_PARENT,
						                    GTK_MESSAGE_INFO,
						                    GTK_BUTTONS_OK,
						                    "TD = %d", 2 * insensitive_settings_get_dataPoints(window->controller->settings));
			gtk_window_set_title(GTK_WINDOW(dialog), "Size of fid (time domain)");
        } else {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window),
						                    GTK_DIALOG_DESTROY_WITH_PARENT,
						                    GTK_MESSAGE_INFO,
						                    GTK_BUTTONS_OK,
						                    "TD = %d × %d", 2 * insensitive_settings_get_dataPoints(window->controller->settings),
                                            2 * indirect_datapoints(insensitive_controller_get_detectionMethod(window->controller), insensitive_settings_get_dataPoints(window->controller->settings)));
			gtk_window_set_title(GTK_WINDOW(dialog), "Size of fid (F1, F2)");
        }
        gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	// TM
	else if (!g_strcmp0(word[0], "tm") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 3);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
    // TOCSV
	else if (!g_strcmp0(word[0], "tocsv") && number_of_words == 1) {
        insensitive_settings_set_exportFormat(window->controller->settings, CSV);
        export_spectrum(NULL, window);
    }
    // TODAT or TOASC
	else if ((!g_strcmp0(word[0], "todat") || !g_strcmp0(word[0], "toasc")) && number_of_words == 1) {
        insensitive_settings_set_exportFormat(window->controller->settings, DAT);
        export_spectrum(NULL, window);
    }
    // TOJDX
	else if (!g_strcmp0(word[0], "tojdx") && number_of_words == 1) {
        insensitive_settings_set_exportFormat(window->controller->settings, JDX);
        export_spectrum(NULL, window);
    }
    // TOTXT
	else if (!g_strcmp0(word[0], "totxt") && number_of_words == 1) {
        insensitive_settings_set_exportFormat(window->controller->settings, TXT);
        export_spectrum(NULL, window);
    }
	// TRAF
	else if (!g_strcmp0(word[0], "traf") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 7);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
	// TRAFS
	else if (!g_strcmp0(word[0], "trafs") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            gtk_combo_box_set_active(window->apodization_combobox, 8);
            on_fid_button_clicked(window->fid_button, window);
        }
	}
	// UNDO
	else if (!g_strcmp0(word[0], "undo") && number_of_words == 1) {
		on_undo_button_clicked(window->undo_button, window);
	}
	// XF1
	else if (!g_strcmp0(word[0], "xf1") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                alert_for_invalid_fourier_transform(window, 2);
            } else {
                on_fft1D_button_clicked(window->fft1D_button, window);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// XFB
	else if (!g_strcmp0(word[0], "xfb") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                alert_for_invalid_fourier_transform(window, 2);
            } else {
                on_fft2D_button_clicked(window->fft1D_button, window);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// XFBM
	else if (!g_strcmp0(word[0], "xfbm") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            if(!shows_2D_spectrum(window)) {
                alert_for_invalid_fourier_transform(window, 2);
            } else {
                on_magnitude_button_clicked(window->fft1D_button, window);
            }
        } else {
            gdk_window_beep(gtk_widget_get_window((GtkWidget *)window));
        }
	}
	// ZG, GO
	else if ((!g_strcmp0(word[0], "zg") || !g_strcmp0(word[0], "go") || !g_strcmp0(word[0], "play")) && number_of_words == 1) {
        if(!insensitive_controller_get_pulseSequence_ends_with_acquisition(window->controller) || !g_strcmp0(word[0], "go"))
            on_acquire_button_clicked(window->acquire_button, window);
        else {
            if(abs(insensitive_controller_get_variableEvolutionTime(window->controller)) > 0)
                on_acquire2DSpectrum_button_clicked(NULL, window);
            else
                on_play_button_clicked(window->play_button, window);
        }
	}
	// .CO
	else if (!g_strcmp0(word[0], ".co") && number_of_words == 1) {
        gtk_combo_box_set_active(window->plotStyle_combobox, 0);
	}
	// .GR
	else if (!g_strcmp0(word[0], ".gr") && number_of_words == 1) {
        gtk_toggle_button_set_active(window->grid_checkbox, !insensitive_settings_get_showGrid(window->controller->settings));
    }
	// .IM
	else if (!g_strcmp0(word[0], ".im") && number_of_words == 1) {
        gtk_combo_box_set_active(window->plotStyle_combobox, 2);
	}
	// .IMAG
	else if (!g_strcmp0(word[0], ".imag") && number_of_words == 1) {
        gtk_toggle_button_set_active(window->showImaginary_checkbox, TRUE);
        gtk_toggle_button_set_active(window->showReal_checkbox, FALSE);
	}
	// .REAL
	else if (!g_strcmp0(word[0], ".real") && number_of_words == 1) {
        gtk_toggle_button_set_active(window->showReal_checkbox, TRUE);
        gtk_toggle_button_set_active(window->showImaginary_checkbox, FALSE);
	}
	// .SD
	else if (!g_strcmp0(word[0], ".sd") && number_of_words == 1) {
        gtk_toggle_button_set_active(window->shiftBaseline_checkbox, TRUE);
	}
	// .ST
	else if (!g_strcmp0(word[0], ".st") && number_of_words == 1) {
        gtk_combo_box_set_active(window->plotStyle_combobox, 1);
	}
	// .SU
	else if (!g_strcmp0(word[0], ".su") && number_of_words == 1) {
        gtk_toggle_button_set_active(window->shiftBaseline_checkbox, FALSE);
	}
	// .VR
	else if (!g_strcmp0(word[0], ".vr") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            reset_magnification(window);
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
	}
	// *2
	else if (!g_strcmp0(word[0], "*2") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            set_magnification(window, window->magnification * 2);
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
	}
	// *8
	else if (!g_strcmp0(word[0], "*8") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            set_magnification(window, window->magnification * 8);
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
	}
	// /2
	else if (!g_strcmp0(word[0], "/2") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            set_magnification(window, window->magnification / 2);
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
	}
	// /8
	else if (!g_strcmp0(word[0], "/8") && number_of_words == 1) {
        if(insensitive_controller_get_spectrumDataAvailable(window->controller)) {
            set_magnification(window, window->magnification / 8);
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
	}
	// Ix
	else if (!g_strcmp0(word[0], "ix") && number_of_words == 1) {
		gtk_toggle_button_set_active(window->ispins_checkbox, TRUE);
		gtk_toggle_button_set_active(window->sspins_checkbox, FALSE);
		gtk_entry_set_text(window->phase_entry, "0.0");
		on_phase_entry_activate(window->phase_entry, window);
		gtk_entry_set_text(window->flipAngle_entry, "90.0");
		on_flipAngle_entry_activate(window->flipAngle_entry, window);
		on_pulse_button_clicked(window->pulse_button, window);
	}
	// Iy
	else if (!g_strcmp0(word[0], "iy") && number_of_words == 1) {
		gtk_toggle_button_set_active(window->ispins_checkbox, TRUE);
		gtk_toggle_button_set_active(window->sspins_checkbox, FALSE);
		gtk_entry_set_text(window->phase_entry, "90.0");
		on_phase_entry_activate(window->phase_entry, window);
		gtk_entry_set_text(window->flipAngle_entry, "90.0");
		on_flipAngle_entry_activate(window->flipAngle_entry, window);
		on_pulse_button_clicked(window->pulse_button, window);
	}
	// Iz
	else if (!g_strcmp0(word[0], "iz") && number_of_words == 1) {
		start_progress_indicator(window);
		insensitive_controller_perform_chemicalShift_on_ISpins_animated(window->controller, TRUE);
	}
	// Sx
	else if (!g_strcmp0(word[0], "sx") && number_of_words == 1) {
		insensitive_controller_set_all_iSpins_active(window->controller, FALSE);
		insensitive_controller_set_all_sSpins_active(window->controller, TRUE);
		gtk_entry_set_text(window->phase_entry, "0.0");
		on_phase_entry_activate(window->phase_entry, window);
		gtk_entry_set_text(window->flipAngle_entry, "90.0");
		on_flipAngle_entry_activate(window->flipAngle_entry, window);
		on_pulse_button_clicked(window->pulse_button, window);
	}
	// Sy
	else if (!g_strcmp0(word[0], "sy") && number_of_words == 1) {
		insensitive_controller_set_all_iSpins_active(window->controller, FALSE);
		insensitive_controller_set_all_sSpins_active(window->controller, TRUE);
		gtk_entry_set_text(window->phase_entry, "90.0");
		on_phase_entry_activate(window->phase_entry, window);
		gtk_entry_set_text(window->flipAngle_entry, "90.0");
		on_flipAngle_entry_activate(window->flipAngle_entry, window);
		on_pulse_button_clicked(window->pulse_button, window);
	}
	// Sz
	else if (!g_strcmp0(word[0], "sz") && number_of_words == 1) {
		start_progress_indicator(window);
		insensitive_controller_perform_chemicalShift_on_SSpins_animated(window->controller, TRUE);
	}
	// iziz
	else if (!g_strcmp0(word[0], "iziz") && number_of_words == 1) {
		start_progress_indicator(window);
		insensitive_controller_perform_coupling_on_ISpins_animated(window->controller, TRUE);
	}
	// izsz
	else if (!g_strcmp0(word[0], "izsz") && number_of_words == 1) {
		start_progress_indicator(window);
		insensitive_controller_perform_coupling_animated(window->controller, TRUE);
	}
	// szsz
	else if (!g_strcmp0(word[0], "szsz") && number_of_words == 1) {
		start_progress_indicator(window);
		insensitive_controller_perform_coupling_on_SSpins_animated(window->controller, TRUE);
	}
	// I#x, I#y, S#x, S#y
	else if (commandMatches1SpinOperator_xy && number_of_words == 1) {
		unsigned int spin = (unsigned int)word[0][1] - 48;
		GtkToggleButton *spin_checkbox;
		if (spin <= insensitive_spinsystem_get_spins(window->controller->spinSystem)) {
			if ((word[0][0] == 'i' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeI) ||
			    (word[0][0] == 's' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeS)) {
				g_signal_handlers_block_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
				gtk_toggle_button_set_active(window->allspins_checkbox, FALSE);
				on_allSpins_checkbox_toggled(window->allspins_checkbox, window);
				g_signal_handlers_unblock_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
				switch (spin) {
				case 1:
					spin_checkbox = window->spin1_checkbox;
					break;
				case 2:
					spin_checkbox = window->spin2_checkbox;
					break;
				case 3:
					spin_checkbox = window->spin3_checkbox;
					break;
				case 4:
					spin_checkbox = window->spin4_checkbox;
				}
				g_signal_handlers_block_by_func(G_OBJECT(spin_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)user_data);
				gtk_toggle_button_set_active(spin_checkbox, TRUE);
				on_spin_checkbox_toggled(spin_checkbox, window);
				g_signal_handlers_unblock_by_func(G_OBJECT(spin_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)user_data);
				if (word[0][2] == 'x')
					gtk_entry_set_text(window->phase_entry, "0.0");
				else if (word[0][2] == 'y')
					gtk_entry_set_text(window->phase_entry, "90.0");
				on_phase_entry_activate(window->phase_entry, window);
				gtk_entry_set_text(window->flipAngle_entry, "90.0");
				on_flipAngle_entry_activate(window->flipAngle_entry, window);
				on_pulse_button_clicked(window->pulse_button, window);
			}
		}
	}
	// I#z, S#z
	else if (commandMatches1SpinOperator_z && number_of_words == 1) {
		unsigned int spin = (unsigned int)word[0][1] - 48;
		if (spin <= insensitive_spinsystem_get_spins(window->controller->spinSystem)) {
			if ((word[0][0] == 'i' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeI) ||
			    (word[0][0] == 's' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeS)) {
				start_progress_indicator(window);
				insensitive_controller_perform_chemicalShift_on_spinArray(window->controller, pow2(spin - 1), TRUE);
			}
		}
	}
	// #(Iz)_n
	else if (commandMatches2SpinOperator || commandMatches3SpinOperator || commandMatches4SpinOperator) {
		unsigned int i, spin, type, array = 0;
		unsigned int numberOfSpins = lb(word[0][0] - 48) + 1;

		for (i = 3; i <= 3 * numberOfSpins; i += 3) {
			spin = word[0][i - 1] - 49;
			type = (word[0][i - 2] == 'i') ? spinTypeI : spinTypeS;
			if (array & pow2(spin) || insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin) != type) {
				array = 0;
				break;
			} else {
				array += pow2(spin);
			}
		}
		if (array == 0) {
			dialog = gtk_message_dialog_new(GTK_WINDOW(window),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_WARNING,
							GTK_BUTTONS_OK,
							"The same spin my occur twice or a spin may not match its spin type in the product operator %s", word[0]);
			gtk_window_set_title(GTK_WINDOW(dialog), "Syntax error");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		} else {
			start_progress_indicator(window);
			insensitive_controller_perform_coupling_on_spinArray(window->controller, array, TRUE);
		}
	}
	// First word is real number: apply product operator
	else if (atof(word[0]) != 0.0) {
		int numberOfMinusSigns, numberOfDecimalPoints;
		gchar *string;

		string = command;
		for (numberOfMinusSigns = 0; string[numberOfMinusSigns]; string[numberOfMinusSigns] == '-' ? numberOfMinusSigns++ : *(string++));
		string = command;
		for (numberOfDecimalPoints = 0; string[numberOfDecimalPoints]; string[numberOfDecimalPoints] == '.' ? numberOfDecimalPoints++ : *(string++));

		if (numberOfMinusSigns > 1 || numberOfDecimalPoints > 1) {
			show_command_error((GtkWidget *)entry, window);
		} else if (numberOfMinusSigns == 1 && (*word[0] != '-' || strlen(word[0]) < 2)) {
			show_command_error((GtkWidget *)entry, window);
		} else {
			g_regex_match(regex1spin_xy, word[1], 0, &matchInfo);
			commandMatches1SpinOperator_xy = g_match_info_matches(matchInfo) ? TRUE : FALSE;
			g_regex_match(regex1spin_z, word[1], 0, &matchInfo);
			commandMatches1SpinOperator_z = g_match_info_matches(matchInfo) ? TRUE : FALSE;
			g_regex_match(regex2spins, word[1], 0, &matchInfo);
			commandMatches2SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;
			g_regex_match(regex3spins, word[1], 0, &matchInfo);
			commandMatches3SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;
			g_regex_match(regex4spins, word[1], 0, &matchInfo);
			commandMatches4SpinOperator = g_match_info_matches(matchInfo) ? TRUE : FALSE;

			if (commandMatches1SpinOperator_z) {
				gtk_entry_set_text(window->delay_entry, word[0]);
				on_delay_entry_activate(window->delay_entry, window);
				if (strlen(word[1]) == 2) { // This means precession of either all I or all S spins
					if (*word[1] == 'i') {
						start_progress_indicator(window);
						insensitive_controller_perform_chemicalShift_on_ISpins_animated(window->controller, TRUE);
					} else if (*word[1] == 's') {
						start_progress_indicator(window);
						insensitive_controller_perform_chemicalShift_on_SSpins_animated(window->controller, TRUE);
					}
				} else if (strlen(word[1]) == 3) { // This means spin-specific precession
					unsigned int spin = word[1][1] - 48;
					if (spin <= insensitive_spinsystem_get_spins(window->controller->spinSystem)) {
						if ((*word[1] == 'i' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeI) ||
						    (*word[1] == 's' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeS)) {
							int spin = word[1][1] - 48;
							int array = pow2(spin - 1);
							start_progress_indicator(window);
							insensitive_controller_perform_chemicalShift_on_spinArray(window->controller, array, TRUE);
						}
					}
				}
			} else if (commandMatches1SpinOperator_xy) {
				int coordinateIndex;
				gboolean noActiveSpin = FALSE;
				if (strlen(word[1]) == 2) { // This means pulses on either all I or all S spins
					coordinateIndex = 1;
					if (*word[1] == 'i') {
						insensitive_controller_set_all_sSpins_active(window->controller, FALSE);
						insensitive_controller_set_all_iSpins_active(window->controller, TRUE);
					} else if (*word[1] == 's') {
						insensitive_controller_set_all_iSpins_active(window->controller, FALSE);
						insensitive_controller_set_all_sSpins_active(window->controller, TRUE);
					}
				} else if (strlen(word[1]) == 3) { // This means spin-specific pulses
					coordinateIndex = 2;
					unsigned int spin = word[1][1] - 48;
					GtkToggleButton *spin_checkbox;

					if (spin <= insensitive_spinsystem_get_spins(window->controller->spinSystem)) {
						if ((*word[1] == 'i' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeI) ||
						    (*word[1] == 's' && insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin - 1) == spinTypeS)) {
							g_signal_handlers_block_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
							gtk_toggle_button_set_active(window->allspins_checkbox, FALSE);
							on_allSpins_checkbox_toggled(window->allspins_checkbox, window);
							g_signal_handlers_unblock_by_func(G_OBJECT(window->allspins_checkbox), G_CALLBACK(on_allSpins_checkbox_toggled), (gpointer)user_data);
							switch (spin) {
							case 1:
								spin_checkbox = window->spin1_checkbox;
								break;
							case 2:
								spin_checkbox = window->spin2_checkbox;
								break;
							case 3:
								spin_checkbox = window->spin3_checkbox;
								break;
							case 4:
								spin_checkbox = window->spin4_checkbox;
							}
							g_signal_handlers_block_by_func(G_OBJECT(spin_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)user_data);
							gtk_toggle_button_set_active(spin_checkbox, TRUE);
							on_spin_checkbox_toggled(spin_checkbox, window);
							g_signal_handlers_unblock_by_func(G_OBJECT(spin_checkbox), G_CALLBACK(on_spin_checkbox_toggled), (gpointer)user_data);
						} else {
							noActiveSpin = TRUE;
						}
					} else {
						noActiveSpin = TRUE;
					}
				}
				if (!noActiveSpin) {
					if (atof(word[0]) < 0 && word[1][coordinateIndex] == 'x')
						gtk_entry_set_text(window->phase_entry, "180.0");
					else if (atof(word[0]) >= 0 && word[1][coordinateIndex] == 'x')
						gtk_entry_set_text(window->phase_entry, "0.0");
					else if (atof(word[0]) < 0 && word[1][coordinateIndex] == 'y')
						gtk_entry_set_text(window->phase_entry, "270.0");
					else if (atof(word[0]) >= 0 && word[1][coordinateIndex] == 'y')
						gtk_entry_set_text(window->phase_entry, "90.0");
					on_phase_entry_activate(window->phase_entry, window);
					// Apply pulse operator in units of 1°
					float flipAngle = fabs(atof(word[0]));
					while (flipAngle > 360)
						flipAngle -= 360;
					gtk_entry_set_text(window->flipAngle_entry, word[0]);
					on_flipAngle_entry_activate(window->flipAngle_entry, window);
					on_pulse_button_clicked(window->pulse_button, window);
				}
			} else if (!g_strcmp0(word[1], "iziz")) {
				gtk_entry_set_text(window->delay_entry, word[0]);
				on_delay_entry_activate(window->delay_entry, window);
				start_progress_indicator(window);
				insensitive_controller_perform_coupling_on_ISpins_animated(window->controller, TRUE);
			} else if (!g_strcmp0(word[1], "izsz")) {
				gtk_entry_set_text(window->delay_entry, word[0]);
				on_delay_entry_activate(window->delay_entry, window);
				start_progress_indicator(window);
				insensitive_controller_perform_coupling_animated(window->controller, TRUE);
			} else if (!g_strcmp0(word[1], "szsz")) {
				gtk_entry_set_text(window->delay_entry, word[0]);
				on_delay_entry_activate(window->delay_entry, window);
				start_progress_indicator(window);
				insensitive_controller_perform_coupling_on_SSpins_animated(window->controller, TRUE);
			} else if (commandMatches2SpinOperator || commandMatches3SpinOperator || commandMatches4SpinOperator) {
				unsigned int i, spin, type, array = 0;
				unsigned int numberOfSpins = lb(*word[1]  - 48) + 1;

				gtk_entry_set_text(window->delay_entry, word[0]);
				on_delay_entry_activate(window->delay_entry, window);
				for (i = 3; i <= 3 * numberOfSpins; i += 3) {
					spin = word[1][i - 1] - 49;
					type = (word[1][i - 2] == 'i') ? spinTypeI : spinTypeS;
					if (array & pow2(spin) || insensitive_spinsystem_get_spintype_for_spin(window->controller->spinSystem, spin) != type) {
						array = 0;
						break;
					} else {
						array += pow2(spin);
					}
				}
				if (array == 0) {
					dialog = gtk_message_dialog_new(GTK_WINDOW(window),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_WARNING,
									GTK_BUTTONS_OK,
									"The same spin my occur twice or a spin may not match its spin type in the product operator %s", word[0]);
					gtk_window_set_title(GTK_WINDOW(dialog), "Syntax error");
					gtk_dialog_run(GTK_DIALOG(dialog));
					gtk_widget_destroy(dialog);
				} else {
					start_progress_indicator(window);
					insensitive_controller_perform_coupling_on_spinArray(window->controller, array, TRUE);
				}
			} else
				show_command_error((GtkWidget *)entry, window);
		}
	}
	// Unknown command
	else
		show_command_error((GtkWidget *)entry, window);

	gtk_entry_set_text(entry, "");
    window->commandHistoryPosition = 0;
	g_strfreev(word);
	g_free(regex1spin_xy);
	g_free(regex1spin_z);
	g_free(regex2spins);
	g_free(regex3spins);
	g_free(regex4spins);
	g_free(matchInfo);
	g_free(command);
}


void show_command_error(GtkWidget *widget, gpointer window)
{
	GtkWidget *dialog;
	gchar *command = malloc(gtk_entry_get_text_length(GTK_ENTRY(widget)) * sizeof(gchar));

	strcpy(command, gtk_entry_get_text(GTK_ENTRY(widget)));
	g_strstrip(command);
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
					"Command not implemented or wrong number of arguments:\n\"%s\"", command);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


void alert_for_invalid_fourier_transform(InsensitiveWindow *window, unsigned int dimension)
{
	GtkWidget *dialog;
	gchar *title, *message;

    title = malloc(26 * sizeof(gchar));
    sprintf(title, "%dD Fourier transformation", dimension);
    if (dimension == 1) {
        message = malloc(73 * sizeof(gchar));
        strcpy(message, "You cannot perform 2D Fourier transformation on 1D data. Use ft instead.\0");
    }
    if (dimension == 2) {
        message = malloc(102 * sizeof(gchar));
        strcpy(message, "You cannot perform 1D Fourier transformation on 2D data. Use xf1 and xfb or derived commands instead.\0");
    }
    if (dimension == 1 || dimension == 2) {
	    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
		    			                GTK_DIALOG_DESTROY_WITH_PARENT,
		    			                GTK_MESSAGE_WARNING,
		    			                GTK_BUTTONS_OK,
		    			                "%s", message);
    	gtk_window_set_title(GTK_WINDOW(dialog), title);
	    gtk_dialog_run(GTK_DIALOG(dialog));
	    gtk_widget_destroy(dialog);
        free(title);
        free(message);
    }
}


G_MODULE_EXPORT gboolean on_command_line_key_press_event(GtkEntry *entry, GdkEventKey *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    gchar *command;

    if (event->keyval == GDK_KEY_Up) {
        if (window->commandHistoryPosition < window->commandHistory->len)
            window->commandHistoryPosition++;
        if (window->commandHistoryPosition == 0)
            gtk_entry_set_text(window->command_line, "");
        else {
            command = g_ptr_array_index(window->commandHistory, window->commandHistory->len - window->commandHistoryPosition);
            gtk_entry_set_text(window->command_line, command);
            gtk_editable_set_position(GTK_EDITABLE(window->command_line), -1);
        }
        return TRUE;
    } else if (event->keyval == GDK_KEY_Down) {
        if (window->commandHistoryPosition > 0)
            window->commandHistoryPosition--;
        if (window->commandHistoryPosition == 0)
            gtk_entry_set_text(window->command_line, "");
        else {
            command = g_ptr_array_index(window->commandHistory, window->commandHistory->len - window->commandHistoryPosition);
            gtk_entry_set_text(window->command_line, command);
            gtk_editable_set_position(GTK_EDITABLE(window->command_line), -1);
        }
        return TRUE;
    }
    return FALSE;
}


//////  //////   /////  //     // // ///    //  //////
//   // //   // //   // //     // // ////   // //
//   // //////  /////// //  /  // // // //  // //   ///
//   // //   // //   // // /// // // //  // // //    //
//////  //   // //   //  /// ///  // //   ////  //////

G_MODULE_EXPORT gboolean draw_matrix_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	InsensitiveController *controller = window->controller;
	InsensitiveSettings *settings = controller->settings;
    InsensitiveSpinSystem *spinsystem = controller->spinSystem;
    unsigned int spins = insensitive_spinsystem_get_spins(spinsystem);
    unsigned int dimension = insensitive_spinsystem_get_size(spinsystem);
    float width = (float)gtk_widget_get_allocated_width(widget);
    float height = (float)gtk_widget_get_allocated_height(widget);
    unsigned int i, m, n;
    float origin_x, origin_y, x, y, spacing, factor, alignment, abs;
    float element_width, element_height, matrix_width, matrix_height;
    float radius, angle, bracket_center;
    float center_x, center_y, maxDiagValue = 0;
    gboolean isLargerThanOne;
    gchar matrixElementString[20];
    gchar *rho = "ρ =";
    DSPComplex matrixElement;
    cairo_text_extents_t extents;
    cairo_matrix_t matrix;

    origin_x = 7.0;
    origin_y = 10.0;
    width -= 10.0;
    height -= 10.0;

    switch (insensitive_settings_get_matrixDisplayType(settings)) {
    case MatrixDisplayTypeHidden:
        return FALSE;
        break;
    case MatrixDisplayTypeTiny:
    case MatrixDisplayTypeSmall:
    case MatrixDisplayTypeLarge:
        origin_x += 30.0;
        bracket_center = 2;
        element_height = height / dimension * 2/3;
        spacing = element_height / 2;
        origin_x += spacing;
        cairo_set_font_size(cr, element_height);
        strcpy(matrixElementString, "-8.88+8.88i");
        cairo_select_font_face(cr, insensitive_settings_get_matrixFont(window->controller->settings),
                               CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_text_extents (cr, matrixElementString, &extents);
        element_width = extents.width - extents.x_bearing;
        origin_x += (width - (dimension * (element_width + spacing) + spacing)) / 2;
        for (m = 0; m < dimension; m++)
		    for (n = 0; n < dimension; n++) {
                matrixElement = insensitive_spinsystem_get_matrixelement(spinsystem, m, n);
                factor = dimension;
                matrixElement.real *= factor;
                matrixElement.imag *= factor;
                strcpy(matrixElementString, "\0");
                string_for_complex_number(matrixElementString, matrixElement);
                cairo_text_extents (cr, matrixElementString, &extents);
                alignment = element_width - extents.width - extents.x_bearing;
                x = origin_x + m * element_width + m * spacing;
                y = origin_y + (n + 1) * element_height + n * spacing;
                if (insensitive_settings_get_color1stOrderCoherences(settings)) {
                    if (test_for_simple_coherence(m, n) < 0) {
                        cairo_set_source_rgba(cr, 0.6, 1.0, 0.2, 1.0);
                        cairo_rectangle(cr, x, y - 0.9 * element_height, element_width, element_height);
                        cairo_fill(cr);
                    } else if(test_for_simple_coherence(m, n) > 0) {
                        cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
                        cairo_rectangle(cr, x, y - 0.9 * element_height, element_width, element_height);
                        cairo_fill(cr);
                    }
                }
                cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	            cairo_move_to(cr, x + alignment, y);
		        cairo_show_text(cr, matrixElementString);
		    }
        break;
    case MatrixDisplayTypeGraphical:
        spacing = 5.0;
        bracket_center = 0.65;//0.66;
        //origin_x += spacing;
        origin_x += (width - dimension * 43) / 2;
        for (i = 0; i < spins; i++)
            maxDiagValue += (insensitive_spinsystem_get_spintype_for_spin(spinsystem, i) == spinTypeS) ? spinsystem->gyroS : spinsystem->gyroI;
        for (m = 0; m < dimension; m++)
		    for (n = 0; n < dimension; n++) {
                element_width = 38.0;
                element_height = 38.0;
                center_x = origin_x + m * element_width + m * spacing + ((m == n) ? 4 : 1) + 18;
                center_y = origin_y + n * element_height + n * spacing + 19;
                matrixElement = insensitive_spinsystem_get_matrixelement(spinsystem, m, n);
                if(m == n)
                    factor = dimension / maxDiagValue;
                else
                    factor = dimension;
                matrixElement.real *= factor;
                matrixElement.imag *= factor;
                if(m == n) {
                    x = matrixElement.real * element_width / 2;
                    // Draw line
                    cairo_set_line_width(cr, 0.5);
                    cairo_move_to(cr, center_x - 0.5 * element_width, center_y);
                    cairo_line_to(cr, center_x + 0.5 * element_width - 6, center_y);
                    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
                    cairo_stroke(cr);
                    // Draw rectangle
                    cairo_set_line_width(cr, 1.0);
                    cairo_rectangle(cr, center_x - 0.5 * element_width, center_y - 0.5 * element_height, element_width - 6, element_height);
                    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
                    cairo_stroke(cr);
                    // Draw vector
                    cairo_set_line_width(cr, 9.0);
                    cairo_move_to(cr, center_x - 3, center_y);
                    cairo_line_to(cr, center_x - 3, center_y - x);
                    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
                    cairo_stroke(cr);
                } else {
                    x = -matrixElement.real * cos45 + matrixElement.imag * sin45;
                    y = -matrixElement.imag * cos45 - matrixElement.real * sin45;
                    // Normalize vectors longer than unity
                    abs = sqrt(x * x + y * y);
                    if(abs > 1.00004) {
                        x /= abs;
                        y /= abs;
                        isLargerThanOne = TRUE;
                    } else
                        isLargerThanOne = FALSE;
                    x *= element_width / 2;
                    y *= element_height / 2;
                    // Draw axes
                    cairo_set_line_width(cr, 0.5);
                    cairo_move_to(cr, center_x - 0.3535 * element_width, center_y - 0.3535 * element_height);
                    cairo_line_to(cr, center_x + 0.3535 * element_width, center_y + 0.3535 * element_height);
                    cairo_move_to(cr, center_x - 0.3535 * element_width, center_y + 0.3535 * element_height);
                    cairo_line_to(cr, center_x + 0.3535 * element_width, center_y - 0.3535 * element_height);
                    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
                    cairo_stroke(cr);
                    // Draw circle
                    cairo_set_line_width(cr, 1.0);
                    cairo_arc(cr, center_x, center_y, element_width / 2, 0.0, 2 * M_PI);
                    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
                    cairo_stroke(cr);
                    // Draw vector
                    cairo_set_line_width(cr, isLargerThanOne ? 4 : 2);
                    cairo_move_to(cr, center_x, center_y);
                    cairo_line_to(cr, center_x + x, center_y + y);
                    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
                    cairo_stroke(cr);
                }
            }
    }
    matrix_width = dimension * (element_width + spacing);
    matrix_height = dimension * (element_height + spacing) - 0.5 * spacing;
    x = origin_x + (matrix_width - spacing) / 2;
    y = origin_y + matrix_height / 2;
    radius = hypotf(matrix_width / bracket_center, matrix_height / 2);
    angle = acosf(matrix_width / (bracket_center * radius));
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 3.0);
    if (insensitive_settings_get_matrixDisplayType(settings) == MatrixDisplayTypeGraphical) {
        cairo_save(cr);
        cairo_matrix_init(&matrix, 1.0, 0.0, 0.0, 3.0, 0.0, -1.0 * (matrix_height + 20));
        cairo_transform(cr, &matrix);
    }
    cairo_new_path(cr);
    cairo_arc(cr, x, y, matrix_width / 2 + 5 * spins, -angle, angle);
    cairo_stroke(cr);
    cairo_arc(cr, x, y, matrix_width / 2 + 5 * spins, -angle + M_PI, angle + M_PI);
    cairo_stroke(cr);
    if (insensitive_settings_get_matrixDisplayType(settings) == MatrixDisplayTypeGraphical)
        cairo_restore(cr);
    cairo_set_font_size(cr, 30); //height / 5);
    strcpy(matrixElementString, rho);
    cairo_select_font_face(cr, "default", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_text_extents (cr, matrixElementString, &extents);
	cairo_move_to(cr, x - matrix_width / 2 - extents.width - (22 + 3 * spins), y + extents.height / 2);
	cairo_show_text(cr, rho);

    return FALSE;
}


G_MODULE_EXPORT gboolean draw_vector_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	unsigned int i, spinTypeIsS;
	float x, y, z, temp, slimness;
	float thickness_scaling;
	static const float circleShift = 2.54;
	float width, height, origin_x, origin_y;
	float xyPlane_origin_x, xyPlane_origin_y;
	GtkStyleContext *context;
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	InsensitiveController *controller = window->controller;
	InsensitiveSettings *settings = controller->settings;
	VectorCoordinates *spinVector = insensitive_controller_get_vectorCoordinates(controller);
	gboolean showXYPlane = (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramXYplane);

	if (!strcmp(gtk_widget_get_name(widget), "iSpinVectorView"))
		spinTypeIsS = !spinTypeS;
	else if (!strcmp(gtk_widget_get_name(widget), "sSpinVectorView"))
		spinTypeIsS = spinTypeS;

	context = gtk_widget_get_style_context(widget);
	width = MIN(gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget)) - 6;
	height = width; //gtk_widget_get_allocated_height(widget);
	origin_x = 0.5 * gtk_widget_get_allocated_width(widget);
	origin_y = 0.5 * gtk_widget_get_allocated_height(widget);
	slimness = (width < 300) ? 1.0 : 0.0;
	thickness_scaling = (width < 350) ? 1.0 : width / 325;

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	gtk_render_background(context, cr, 0, 0, width, height);

	// Create sphere for grapefruit diagram (back half)
	if (!showXYPlane) {
		cairo_move_to(cr, origin_x, origin_y + 0.5 * height);
		for (z = 1; z >= -1; z -= 0.01) {
			x = -sqrtf(1 - pow(z, 2));
			y = x * sin45;
			x *= cos45;
			cairo_line_to(cr, origin_x + 0.5 * y * width, origin_y + 0.1 * x * height + 0.5 * z * height);
		}
		cairo_move_to(cr, origin_x, origin_y + 0.5 * height);
		for (z = 1; z >= -1; z -= 0.01) {
			y = -sqrtf(1 - pow(z, 2));
			y = y * cos45;
			x = -y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * y * width, origin_y + 0.1 * x * height + 0.5 * z * height);
		}
		cairo_move_to(cr, origin_x + 0.5 * -1 * width, origin_y + 0.1 * 0 * height);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * temp * width, origin_y - 0.1 * x * height);
		}
		cairo_move_to(cr, origin_x + 0.5 * 1 * width, origin_y + 0.1 * 0 * height);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * temp * width, origin_y - 0.1 * x * height);
		}
		cairo_set_line_width(cr, thickness_scaling);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
		cairo_stroke(cr);
	}
	// Draw Vectors in lower hind quarter
	if (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramGrapefruit) {
		draw_grapefruit_paths(controller, FALSE, FALSE, cr, width, height, origin_x, origin_y, spinTypeIsS);
	}
    for (i = 0; i < spinVector->size; i++) {
		x = spinVector->x[i];
		y = spinVector->y[i];
		z = spinVector->z[i];
		if (z < -0.005 && x < -0.005 && y < -0.005) {
			// If z-axis hides vectors in 3D-view
			if (((x < y + 0.01) && (x > y - 0.01) && !showXYPlane)
			    // Or if x- or y-axis hides vectors in 2D-view
			    || (((x < 0.01) && (x > -0.01)) || (((y < 0.01) && (y > -0.01)) && showXYPlane)))
				cairo_set_line_width(cr,  (4 - slimness) * thickness_scaling);
			else
				cairo_set_line_width(cr,  (2 - slimness) * thickness_scaling);
			draw_vector_to_context(settings, cr, width, height, origin_x, origin_y, x, y, z,
					       spinTypeIsS, spinVector->spinType[i], spinVector->selected[i]);
		}
	}
	// Create -z-axis
	if (!showXYPlane) {
		cairo_set_line_width(cr, thickness_scaling);
		cairo_move_to(cr, origin_x, origin_y);
		cairo_line_to(cr, origin_x, origin_y + 0.5 * height);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	}
	// Draw Vectors in lower front quarter
	for (i = 0; i < spinVector->size; i++) {
		x = spinVector->x[i];
		y = spinVector->y[i];
		z = spinVector->z[i];
		if (!showXYPlane)
			cairo_set_line_width(cr,  (3 - slimness) * thickness_scaling);
		else {
			// If x- or y-axis hides vectors in 2D-view
			if (((x < 0.01) && (x > -0.01)) || ((y < 0.01) && (y > -0.01)))
				cairo_set_line_width(cr,  (4 - slimness) * thickness_scaling);
			else
				cairo_set_line_width(cr,  (2 - slimness) * thickness_scaling);
		}
		if (z < -0.005 && (x >= -0.005 || y >= -0.005))
			draw_vector_to_context(settings, cr, width, height, origin_x, origin_y, x, y, z,
					       spinTypeIsS, spinVector->spinType[i], spinVector->selected[i]);
	}
	if (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramGrapefruit) {
		draw_grapefruit_paths(controller, FALSE, TRUE, cr, width, height, origin_x, origin_y, spinTypeIsS);
	}
	// Create x,y-plane
	cairo_set_line_width(cr, thickness_scaling);
	xyPlane_origin_x = origin_x;
	xyPlane_origin_y = 0.4 * height + 5.0;
	if (showXYPlane) {
		xyPlane_origin_y -= height / circleShift; // Not fully centred?
		cairo_arc(cr, origin_x, origin_y, MIN(width, height) / 2.0, 0, 2.0 * G_PI);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	}
	cairo_set_line_width(cr,  (4 - slimness) * thickness_scaling);
	if (showXYPlane) {
		// Create x-axis
		cairo_move_to(cr, origin_x - 0.352 * width, origin_y - 0.352 * height);
		cairo_line_to(cr, origin_x + 0.352 * width, origin_y + 0.352 * height);
		// Create y-axis
		cairo_move_to(cr, origin_x + 0.352 * width, origin_y - 0.352 * height);
		cairo_line_to(cr, origin_x - 0.352 * width, origin_y + 0.352 * height);
		cairo_set_line_width(cr, thickness_scaling);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	} else {
		cairo_set_line_width(cr, thickness_scaling);
		// Create x-axis
		cairo_move_to(cr, origin_x - 0.35 * width, origin_y - 0.35 * 0.2 * height);
		cairo_line_to(cr, origin_x, origin_y);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
		cairo_stroke(cr);
		cairo_move_to(cr, origin_x, origin_y);
		cairo_line_to(cr, origin_x + 0.35 * width, origin_y + 0.35 * 0.2 * height);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
		// Create y-axis
		cairo_move_to(cr, origin_x + 0.35 * width, origin_y - 0.35 * 0.2 * height);
		cairo_line_to(cr, origin_x, origin_y);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
		cairo_stroke(cr);
		cairo_move_to(cr, origin_x, origin_y);
		cairo_line_to(cr, origin_x - 0.35 * width, origin_y + 0.35 * 0.2 * height);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	}
	// Draw Vectors in upper hind quarter
	if (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramGrapefruit) {
		draw_grapefruit_paths(controller, TRUE, FALSE, cr, width, height, origin_x, origin_y, spinTypeIsS);
	}
	for (i = 0; i < spinVector->size; i++) {
		x = spinVector->x[i];
		y = spinVector->y[i];
		z = spinVector->z[i];
		if (z >= -0.005 && x < -0.005 && y < -0.005) {
			// Draw thick line if vector is hidden behind z-axis
			if (showXYPlane)
				cairo_set_line_width(cr,  (3 - slimness) * thickness_scaling);
			else {
				if ((x < y + 0.01) && (x > y - 0.01))
					cairo_set_line_width(cr,  (4 - slimness) * thickness_scaling);
				else
					cairo_set_line_width(cr,  (2 - slimness) * thickness_scaling);
			}
			draw_vector_to_context(settings, cr, width, height, origin_x, origin_y, x, y, z,
					       spinTypeIsS, spinVector->spinType[i], spinVector->selected[i]);
		}
	}
	// Create +z-axis
	if (!showXYPlane) {
		cairo_set_line_width(cr, thickness_scaling);
		cairo_move_to(cr, origin_x, origin_y - 0.5 * height);
		cairo_line_to(cr, origin_x, origin_y);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	}
	// Draw Vectors in upper front quarter
	for (i = 0; i < spinVector->size; i++) {
		x = spinVector->x[i];
		y = spinVector->y[i];
		z = spinVector->z[i];
		cairo_set_line_width(cr,  (3 - slimness) * thickness_scaling);
		if (z >= -0.005 && (x >= -0.005 || y >= -0.005))
			draw_vector_to_context(settings, cr, width, height, origin_x, origin_y, x, y, z,
					       spinTypeIsS, spinVector->spinType[i], spinVector->selected[i]);
	}
	if (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramGrapefruit) {
		draw_grapefruit_paths(controller, TRUE, TRUE, cr, width, height, origin_x, origin_y, spinTypeIsS);
	}
	// Create sphere for grapefruit diagram (front half)
	if (!showXYPlane) {
		cairo_set_line_width(cr, thickness_scaling);
		xyPlane_origin_x = origin_x;
		xyPlane_origin_y = origin_y;
		cairo_arc(cr, xyPlane_origin_x, xyPlane_origin_y, MIN(width, height) / 2.0, 0, 2.0 * G_PI);
		cairo_move_to(cr, origin_x, origin_y - height / 2);
		for (float z = 1; z >= -1; z -= 0.01) {
			x = sqrtf(1 - pow(z, 2));
			y = x * sin45;
			x *= cos45;
			cairo_line_to(cr, origin_x + 0.5 * y * width, origin_y + 0.1 * x * height - 0.5 * z * height);
		}
		cairo_move_to(cr, origin_x, origin_y - height / 2);
		for (float z = 1; z >= -1; z -= 0.01) {
			y = sqrtf(1 - pow(z, 2));
			y = y * cos45;
			x = -y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * y * width, origin_y + 0.1 * x * height - 0.5 * z * height);
		}
		cairo_move_to(cr, origin_x + 0.5 * -1 * width, origin_y + 0.1 * 0 * height);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * temp * width, origin_y + 0.1 * x * height);
		}
		cairo_move_to(cr, origin_x + 0.5 * 1 * width, origin_y + 0.1 * 0 * height);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			cairo_line_to(cr, origin_x + 0.5 * temp * width, origin_y + 0.1 * x * height);
		}
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_stroke(cr);
	}
	// Axis labels
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 20);
	if (showXYPlane) {
		cairo_move_to(cr, origin_x - 0.352 * width - 20, origin_y + 0.352 * height + 15);
		cairo_show_text(cr, "x");
		cairo_move_to(cr, origin_x + 0.352 * width + 10, origin_y + 0.352 * height + 15);
		cairo_show_text(cr, "y");
	} else {
		cairo_move_to(cr, origin_x - 0.35 * width - 20, origin_y + 0.35 * 0.2 * height + 15);
		cairo_show_text(cr, "x");
		cairo_move_to(cr, origin_x + 0.35 * width + 10, origin_y + 0.35 * 0.2 * height + 15);
		cairo_show_text(cr, "y");
		cairo_move_to(cr, origin_x + 10, origin_y - 0.5 * height + 13);
		cairo_show_text(cr, "z");
	}
	cairo_stroke(cr);

	return FALSE;
}

void draw_vector_to_context(InsensitiveSettings *settings, cairo_t *cr,
			    float width, float height, float origin_x, float origin_y,
			    float x, float y, float z, unsigned int spinTypeIsS, float type, gboolean active)
{
	float temp, drawX, drawY;

	if ((type == spinTypeI && !spinTypeIsS) || (type == spinTypeS && spinTypeIsS)) {
		// Rotate the x,y-plane for -45° about the z-axis for correct display
		temp = x * cos45 - y * sin45;
		y = y * cos45 + x * sin45;
		x = temp;
		if (insensitive_settings_get_vectorDiagramType(settings) == VectorDiagramXYplane) {
			// x-component is drawn to y-coordinate of window
			drawY = origin_y + 0.5 * x * width;
			// y-component is drawn to x-coordinate of window
			drawX = origin_x + 0.5 * y * width;
		} else {
			// x-component is drawn to y-coordinate of window
			drawY = origin_y + 0.1 * x * height;
			// y-component is drawn to x-coordinate of window
			drawX = origin_x + 0.5 * y * width;
			// z-component is drawn to y-coordinate of window
			drawY -= 0.5 * z * height;
		}
		// Draw vector
		cairo_move_to(cr, origin_x, origin_y);
		cairo_line_to(cr, drawX, drawY);
		if (type == spinTypeI) {
			if (active)
				cairo_set_source_rgb(cr, 1.0, 0.4, 0.4);
			else
				cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
		} else if (type == spinTypeS) {
			if (active)
				cairo_set_source_rgb(cr, 0.3, 0.8, 1.0);
			else
				cairo_set_source_rgb(cr, 0.0, 0.5, 1.0);
		}
		cairo_stroke(cr);
	}
}


void draw_grapefruit_paths(InsensitiveController *controller, gboolean upper, gboolean front, cairo_t *cr,
			               float width, float height, float origin_x, float origin_y, unsigned int spinTypeIsS)
{
	unsigned int i, j, size;
	float x, y, z;
	VectorCoordinates *firstCoordinate, *coordinate;
	float temp, slimness, drawX, drawY, lastX, lastY;
	gboolean newLine;
	GPtrArray *grapefruitPaths = insensitive_controller_get_grapefruit_path(controller);

	slimness = (width < 300) ? 1.0 : 0.0;
	if (grapefruitPaths->len > 0) {
		firstCoordinate = malloc(sizeof(VectorCoordinates));
		coordinate = malloc(sizeof(VectorCoordinates));
		firstCoordinate = g_ptr_array_index(grapefruitPaths, 0);
		size = firstCoordinate->size;
		if (front)
			cairo_set_line_width(cr, 2.5 - slimness);
		else
			cairo_set_line_width(cr, 2 - slimness);
		for (i = 0; i < size; i++) {
			if ((firstCoordinate->spinType[i] == spinTypeI && !spinTypeIsS) || (firstCoordinate->spinType[i] == spinTypeS && spinTypeIsS)) {
				newLine = TRUE;
				for (j = 0; j < grapefruitPaths->len; j++) {
					coordinate = g_ptr_array_index(grapefruitPaths, j);
					x = coordinate->x[i];
					y = coordinate->y[i];
					z = coordinate->z[i];
					// Rotate the x,y-plane for -45° about the z-axis for correct display
					temp = x * cos45 - y * sin45;
					y = y * cos45 + x * sin45;
					x = temp;
					// x-component is drawn to y-coordinate of window
					drawY = origin_y + 0.1 * x * height;
					// y-component is drawn to x-coordinate of window
					drawX = origin_x + 0.5 * y * width;
					// z-component is drawn to y-coordinate of window
					drawY -= 0.5 * z * height;
					if (j == 0) {
						lastX = drawX;
						lastY = drawY;
					}
					// Draw lower hind quarter
					if (!upper && !front) {
						if (z < 0 && coordinate->x[i] < 0 && coordinate->y[i] < 0) {
							if (firstCoordinate->spinType[i] == spinTypeI) {
								cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);          // 0.85 (too light) or 0.425 (too dark)
							} else if (firstCoordinate->spinType[i] == spinTypeS) {
								cairo_set_source_rgba(cr, 0.0, 0.213, 0.425, 1.0);      //0.0, 0.425, 0.85, 0.45);
							}
							if (newLine) {
								cairo_move_to(cr, lastX, lastY);
								newLine = FALSE;
							}
							cairo_line_to(cr, drawX, drawY);
						} else
							newLine = TRUE;
					}
					// Draw lower front quarter
					if (!upper && front) {
						if (z < 0 && (coordinate->x[i] >= 0 || coordinate->y[i] >= 0)) {
							if (firstCoordinate->spinType[i] == spinTypeI) {
								cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
							} else if (firstCoordinate->spinType[i] == spinTypeS) {
								cairo_set_source_rgba(cr, 0.0, 0.213, 0.425, 1.0);
							}
							if (newLine) {
								cairo_move_to(cr, lastX, lastY);
								newLine = FALSE;
							}
							cairo_line_to(cr, drawX, drawY);
						} else
							newLine = TRUE;
					}
					// Draw upper hind quarter
					if (upper && !front) {
						if (z >= 0 && coordinate->x[i] < 0 && coordinate->y[i] < 0) {
							if (firstCoordinate->spinType[i] == spinTypeI) {
								cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
							} else if (firstCoordinate->spinType[i] == spinTypeS) {
								cairo_set_source_rgba(cr, 0.0, 0.213, 0.425, 0.45);
							}
							if (newLine) {
								cairo_move_to(cr, lastX, lastY);
								newLine = FALSE;
							}
							cairo_line_to(cr, drawX, drawY);
						} else
							newLine = TRUE;
					}
					// Draw upper front quarter
					if (upper && front) {
						if (z >= 0 && (coordinate->x[i] >= 0 || coordinate->y[i] >= 0)) {
							if (firstCoordinate->spinType[i] == spinTypeI) {
								cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
							} else if (firstCoordinate->spinType[i] == spinTypeS) {
								cairo_set_source_rgba(cr, 0.0, 0.213, 0.425, 1.0);
							}
							if (newLine) {
								cairo_move_to(cr, lastX, lastY);
								newLine = FALSE;
							}
							cairo_line_to(cr, drawX, drawY);
						} else
							newLine = TRUE;
					}
					lastX = drawX;
					lastY = drawY;
				}
				cairo_stroke(cr);
			}
		}
	}
}


G_MODULE_EXPORT gboolean draw_spinEditor_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	//GtkStyleContext *context;
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	InsensitiveController *controller = window->controller;
	InsensitiveSpinSystem *spinsystem = controller->spinSystem;
	InsensitiveSettings *settings = controller->settings;
	float width, height, alignment;//, origin_x, origin_y;
	float alphaOfStroke, angle;
	float center_spin_x[4], center_spin_y[4], adjust_x, adjust_y;
    float shift, constant;
    gchar *label = malloc(20 * sizeof(gchar));
    gchar *unit = malloc(4 * sizeof(gchar));
    cairo_text_extents_t extents;
	unsigned int n, m;
	unsigned int spins = insensitive_spinsystem_get_spins(spinsystem);
	float *couplingMatrix = insensitive_spinsystem_get_raw_couplingmatrix(spinsystem);

	//context = gtk_widget_get_style_context(widget);
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	//origin_x = 0.5 * gtk_widget_get_allocated_width(widget);
	//origin_y = 0.5 * gtk_widget_get_allocated_height(widget);

	center_spin_x[0] = width / 2 - icon_half_width;
	center_spin_x[1] = 128;
	center_spin_x[2] = width - 128 - 2 * icon_half_width;
	center_spin_x[3] = center_spin_x[0];
	center_spin_y[0] = 24;
	center_spin_y[1] = height / 2 - icon_half_width;
	center_spin_y[2] = center_spin_y[1];
	center_spin_y[3] = height - 24 - 2 * icon_half_width;

	// Draw line for drag and drop spin coupling
	if (window->spinEditor_drawLine) {
        cairo_set_line_width(cr, 5);
	    cairo_set_source_rgba(cr, 0.65, 0.65, 0.65, 1.0);
	    cairo_move_to(cr, window->spinEditor_linkSource_x, window->spinEditor_linkSource_y);
	    cairo_line_to(cr, window->spinEditor_linkTarget_x, window->spinEditor_linkTarget_y);
	    cairo_stroke(cr);
        cairo_arc(cr, window->spinEditor_linkTarget_x, window->spinEditor_linkTarget_y, 5.0, 0, 2.0 * G_PI);
        cairo_fill(cr);
	}
	// Draw lines between all coupling spin pairs
	for (n = 0; n < spins /*- 1*/; n++)
		for (m = n + 1; m < spins; m++) {
			// Make alpha channel correspond to coupling constant
			switch (window->displayedConstant) {
			case DipolarConstant:
                constant = insensitive_spinsystem_get_dipolarcouplingconstant_between_spins(spinsystem, n, m);
                strcpy(unit, "kHz");
                alphaOfStroke = 1 - couplingMatrix[m * spins + n];
				break;
			case DistanceConstant:
                constant = insensitive_spinsystem_get_distance_between_spins(spinsystem, n, m);
                strcpy(unit, "nm");
				alphaOfStroke = 1 - couplingMatrix[m * spins + n];
				break;
			case ScalarConstant:
                constant = insensitive_spinsystem_get_jcouplingconstant_between_spins(spinsystem, n, m);
                strcpy(unit, "Hz");
				if (couplingMatrix[n * spins + m] >= 1)
					alphaOfStroke = 1;
				else
					alphaOfStroke = fabsf(couplingMatrix[n * spins + m]);
			}
			// Draw line between coupling spins
            if(constant != 0.0) {
                if(n == 0 && m == 1) {
                    adjust_x = 10;
                    adjust_y = 32;
                } else if(n == 0 && m == 2) {
                    adjust_x = -2;
                    adjust_y = 22;
                } else if(n == 0 && m == 3) {
                    adjust_x = 30;
                    adjust_y = 48;
                } else if(n == 1 && m == 2) {
                    adjust_x = -64;
                    adjust_y = 22;
                } else if(n == 1 && m == 3) {
                    adjust_x = 10;
                    adjust_y = 6;
                } else if(n == 2 && m == 3) {
                    adjust_x = 22;
                    adjust_y = 48;
                }
			    cairo_set_line_width(cr, 2);
			    cairo_move_to(cr, center_spin_x[m] + 0.75 * icon_half_width, center_spin_y[m] + 0.85 * icon_half_width);
			    cairo_line_to(cr, center_spin_x[n] + 0.75 * icon_half_width, center_spin_y[n] + 0.85 * icon_half_width);
			    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, alphaOfStroke);
			    cairo_stroke(cr);
                cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 1.0);
                cairo_select_font_face(cr, "Lucida Grande", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			    if (!cairo_get_font_face(cr))
				    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
                cairo_set_font_size(cr, 10);
                sprintf(label, "%.2f %s", constant, unit);
                cairo_text_extents (cr, label, &extents);
                alignment = sin(extents.width / 2 + extents.x_bearing);
                cairo_move_to(cr, (center_spin_x[m] + center_spin_x[n]) / 2 - alignment + adjust_x, (center_spin_y[m] + center_spin_y[n]) / 2 + adjust_y);
                cairo_save(cr);
                angle = atanf((center_spin_y[m] - center_spin_y[n]) / (center_spin_x[m] - center_spin_x[n]));
                cairo_rotate(cr, angle);
        	    cairo_show_text(cr, label);
                cairo_restore(cr);
            }
		}
	// Paint the spin icons
    for (n = 0; n < spins; n++) {
        if (insensitive_spinsystem_get_spintype_for_spin(spinsystem, n) == spinTypeI) {
            if (insensitive_controller_get_selected_spin(controller) == n)
                cairo_set_source_surface(cr, window->ispin_selected_image, center_spin_x[n], center_spin_y[n]);
            else
	            cairo_set_source_surface(cr, window->ispin_image, center_spin_x[n], center_spin_y[n]);
        } else {
            if (insensitive_controller_get_selected_spin(controller) == n)
                cairo_set_source_surface(cr, window->sspin_selected_image, center_spin_x[n], center_spin_y[n]);
            else
	            cairo_set_source_surface(cr, window->sspin_image, center_spin_x[n], center_spin_y[n]);
        }
    	cairo_paint(cr);
        cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 1.0);
        cairo_select_font_face(cr, "Lucida Grande", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	    if (!cairo_get_font_face(cr))
			  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        sprintf(label, "Spin %d", n + 1);
        cairo_text_extents (cr, label, &extents);
        alignment = (n == 1 || n == 3) ? (extents.width + extents.x_bearing) : -(icon_half_width + 14);
        cairo_move_to(cr, center_spin_x[n] - alignment, center_spin_y[n] + icon_half_width - 16);
    	cairo_show_text(cr, label);
        sprintf(label, "%c-spin", insensitive_spinsystem_get_spintype_for_spin(spinsystem, n) ? 'S' : 'I');
        cairo_text_extents (cr, label, &extents);
        alignment = (n == 1 || n == 3) ? (extents.width + extents.x_bearing) : -(icon_half_width + 14);
        cairo_move_to(cr, center_spin_x[n] - alignment, center_spin_y[n] + icon_half_width - 2);
    	cairo_show_text(cr, label);
        shift = insensitive_spinsystem_get_larmorfrequency_for_spin(spinsystem, n) * insensitive_controller_get_unitConversion(window->controller);
        sprintf(label, "Ω = %.2f %s", shift, insensitive_settings_get_larmorFrequencyInDegreesPerSeconds(settings) ? "°/s" : "Hz");
        cairo_text_extents (cr, label, &extents);
        alignment = (n == 1 || n == 3) ? (extents.width + extents.x_bearing) : -(icon_half_width + 14);
        cairo_move_to(cr, center_spin_x[n] - alignment, center_spin_y[n] + icon_half_width + 12);
    	cairo_show_text(cr, label);
        cairo_stroke(cr);
    }
    free(label);
    free(unit);
	return FALSE;
}


G_MODULE_EXPORT void on_spinEditor_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    float width, height;
    float center_spin_x[4], center_spin_y[4];
    unsigned int i;

    width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	center_spin_x[0] = width / 2 - icon_half_width;
	center_spin_x[1] = 128;
	center_spin_x[2] = width - 128 - 2 * icon_half_width;
	center_spin_x[3] = center_spin_x[0];
	center_spin_y[0] = 24;
	center_spin_y[1] = height / 2 - icon_half_width;
	center_spin_y[2] = center_spin_y[1];
	center_spin_y[3] = height - 24 - 2 * icon_half_width;

    for (i = 0; i < maxNumberOfSpins; i++) {
        if ((event->x >= center_spin_x[i]) && (event->x <= center_spin_x[i] + icon_half_width * 1.5)
            && (event->y >= center_spin_y[i]) && (event->y <= center_spin_y[i] + icon_half_width * 1.5)) {
                if (event->type == GDK_2BUTTON_PRESS) {
                    set_spin_type(window, i);
                    spin_number_was_changed(window);
                } else if (event->button == GDK_RIGHTBUTTON) {
                    // Context menu at mouse location
                } else {
                    insensitive_controller_set_selected_spin(window->controller, i);
                    window->spinEditor_drawLine = TRUE;
                    window->spinEditor_linkSource_x = center_spin_x[i] + 0.75 * icon_half_width;
                    window->spinEditor_linkSource_y = center_spin_y[i] + 0.85 * icon_half_width;
                    window->spinEditor_linkTarget_x = event->x;
                    window->spinEditor_linkTarget_y = event->y;
                }
                break;
        }
    }
}


G_MODULE_EXPORT void on_spinEditor_drawingarea_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    float width, height;
    float center_spin_x[4], center_spin_y[4];
    float value;
    unsigned int i, selected;

    width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	center_spin_x[0] = width / 2 - icon_half_width;
	center_spin_x[1] = 128;
	center_spin_x[2] = width - 128 - 2 * icon_half_width;
	center_spin_x[3] = center_spin_x[0];
	center_spin_y[0] = 32;
	center_spin_y[1] = height / 2 - icon_half_width;
	center_spin_y[2] = center_spin_y[1];
	center_spin_y[3] = height - 32 - 2 * icon_half_width;

    for (i = 0; i < maxNumberOfSpins; i++) {
        if ((event->x >= center_spin_x[i]) && (event->x <= center_spin_x[i] + icon_half_width * 1.5)
            && (event->y >= center_spin_y[i]) && (event->y <= center_spin_y[i] + icon_half_width * 1.5)) {
                selected = insensitive_controller_get_selected_spin(window->controller);
                if(i != selected) {
					switch(window->displayedConstant) {
                    case ScalarConstant:
                        value = atof(gtk_entry_get_text(window->scalarConstant_entry));
                        insensitive_controller_set_jCouplingConstant_between_spins(window->controller, selected, i, value);
                        break;
                    case DipolarConstant:
                        value = atof(gtk_entry_get_text(window->dipolarConstant_entry));
                        insensitive_controller_set_dipolarCouplingConstant_between_spins(window->controller, selected, i, value);
                        break;
                    case DistanceConstant:
                        value = atof(gtk_entry_get_text(window->distanceConstant_entry));
                        insensitive_controller_set_distance_between_spins(window->controller, selected, i, value);
                    }
                } else {
                    spin_was_selected(window, i);
                }
                break;
        }
    }
    window->spinEditor_drawLine = FALSE;
    gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
}


G_MODULE_EXPORT void on_spinEditor_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    if (window->spinEditor_drawLine) {
        window->spinEditor_linkTarget_x = event->x;
        window->spinEditor_linkTarget_y = event->y;
        gtk_widget_queue_draw((GtkWidget *)window->spinEditor_drawingarea);
    }
}


void set_energy_values(InsensitiveWindow *window, float *array, unsigned int levels,
                       GPtrArray *names, int *transitions, float *probabilities)
{
    unsigned int i;
    gchar *nameCopy;

    window->numberOfLevels = levels;
    if (window->energyLevel != NULL) {
        free(window->energyLevel);
        window->energyLevel = NULL;
    }
    window->energyLevel = malloc(levels * sizeof(float));
    if (window->energyLevelOrig != NULL) {
        free(window->energyLevelOrig);
        window->energyLevelOrig = NULL;
    }
    window->energyLevelOrig = malloc(levels * sizeof(float));
    if (window->transition != NULL) {
        free(window->transition);
        window->transition = NULL;
    }
    window->transition = malloc(levels * lb(levels) * sizeof(int));
    if (window->transitionProbability != NULL) {
        free(window->transitionProbability);
        window->transitionProbability = NULL;
    }
    window->transitionProbability = malloc(levels * lb(levels) * sizeof(float));

    for (i = 0; i < levels; i++) {
        window->energyLevel[i] = array[i];
        window->energyLevelOrig[i] = array[i];
    }
    for (i = 0; i < levels * lb(levels); i++) {
        window->transition[i] = transitions[i];
        window->transitionProbability[i] = probabilities[i];
    }
    if (window->levelNames != NULL)
        g_ptr_array_free(window->levelNames, TRUE);
    window->levelNames = g_ptr_array_new();
    if (window->levelNamesOrig != NULL)
        g_ptr_array_free(window->levelNamesOrig, TRUE);
    window->levelNamesOrig = g_ptr_array_new();
    for (i = 0; i < names->len; i++) {
        nameCopy = malloc(energy_level_str_len * sizeof(gchar));
        strcpy(nameCopy, g_ptr_array_index(names, i));
        g_ptr_array_add(window->levelNames, nameCopy);
        nameCopy = malloc(energy_level_str_len * sizeof(gchar));
        strcpy(nameCopy, g_ptr_array_index(names, i));
        g_ptr_array_add(window->levelNamesOrig, nameCopy);
    }
    gtk_widget_queue_draw((GtkWidget *)window->energyLevel_drawingarea);
}


G_MODULE_EXPORT void draw_energyLevel_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	unsigned int i, j, t, spins;
	float width, height, origin_x, origin_y, alignment;
    float min, max, minStep, *energyLevelCopy, energyPosition, spacing;
    unsigned int *degeneracy;
    gchar *label1, *label2, *temp;
    float label1_y, label2_y;
    float energyStartPoint, energyEndPoint, oneLevelWidth, overlap;
    int n, xPosition, signumDeltaE, transitionIndex;
    int distance;
    float *valuePtr;
    cairo_text_extents_t extents;

	if(window->energyLevel != NULL && window->numberOfLevels > 1) {
        width = gtk_widget_get_allocated_width(widget);
	    height = gtk_widget_get_allocated_height(widget);
	    origin_x = 0.0;
	    origin_y = height;

        // Determine number of spins from allocated memory
        spins = lb(window->numberOfLevels);
        // Determine smallest and largest value
        // FLT_MAX indicates an unused entry that will be ignored when drawing the level diagram
        min = window->energyLevel[0];
        max = window->energyLevel[0];
        for (i = 1; i < window->numberOfLevels; i++) {
            if(window->energyLevel[i] < min && window->energyLevel[i] != FLT_MAX)
                min = window->energyLevel[i];
            if(window->energyLevel[i] > max && window->energyLevel[i] != FLT_MAX)
                max = window->energyLevel[i];
        }
        // Set maximal height of level display
        spacing = 20;
        height = height - 2 * spacing;
        width = width - 50;
        minStep = 2.5 * (max - min) / height;
        // Identify transitions
        GPtrArray *transitionArray2D = g_ptr_array_new();
        energyLevelCopy = malloc(window->numberOfLevels * sizeof(float));
        for (i = 0; i < window->numberOfLevels; i++) {
            energyLevelCopy[i] = window->energyLevel[i];
            if(window->energyLevel[i] != FLT_MAX) {
                // Save the transitions of this level into the DeltaE-array
                GPtrArray *transitionsForLevel = g_ptr_array_new();
                for (t = 0; t < spins; t++) {
                    if ((window->transition[i * spins + t] != -1)) {
                        valuePtr = malloc(sizeof(float));
                        *valuePtr = window->energyLevel[window->transition[i * spins + t]];
                        g_ptr_array_add(transitionsForLevel, valuePtr);
                        valuePtr = malloc(sizeof(float));
                        *valuePtr = window->transitionProbability[i * spins + t];
                        g_ptr_array_add(transitionsForLevel, valuePtr);
                    } else {
                        valuePtr = malloc(sizeof(float));
                        *valuePtr = FLT_MAX;
                        g_ptr_array_add(transitionsForLevel, valuePtr);
                        valuePtr = malloc(sizeof(float));
                        *valuePtr = 0.0;
                        g_ptr_array_add(transitionsForLevel, valuePtr);
                    }
                }
                g_ptr_array_add(transitionArray2D, transitionsForLevel);
            }
        }
        // Identify degenerate energy levels
        degeneracy = malloc(window->numberOfLevels * sizeof(unsigned int));
        for (i = 0; i < window->numberOfLevels; i++)
            degeneracy[i] = 1;
        for (i = 0; i < window->numberOfLevels; i++) {
            for (j = i + 1; j < window->numberOfLevels; j++) {
                // Test only if the energy level has been detected/saved in the array
                if (window->energyLevel[j] != FLT_MAX) {
                    // Test for degeneracy or near-degeneracy (resolution-dependent!)
                    if (fabs(window->energyLevel[j] - window->energyLevel[i]) < minStep) {
                        temp = g_ptr_array_index(window->levelNames, i);
                        if (*temp == '|') {
                            temp = malloc(energy_level_str_len * sizeof(gchar));
                            strcpy(temp, g_ptr_array_index(window->levelNames, j));
                            strcat(temp, ", ");
                            strcat(temp, g_ptr_array_index(window->levelNames, i));
                            strcpy(g_ptr_array_index(window->levelNames, i), temp);
                            strcpy(g_ptr_array_index(window->levelNames, j), "");
                            free(temp);
                        } else {
                            strcat(g_ptr_array_index(window->levelNames, i), ", ");
                            strcat(g_ptr_array_index(window->levelNames, i), g_ptr_array_index(window->levelNames, j));
                            strcpy(g_ptr_array_index(window->levelNames, j), "");
                        }
                        window->energyLevel[i] = (window->energyLevel[i] + window->energyLevel[j]) / 2;
                        window->energyLevel[j] = FLT_MAX;
                        degeneracy[i]++;
                        degeneracy[j] = 0;
                    }
                }
            }
        }
        // Perform the drawing
        transitionIndex = 0;
        distance = (5 - spins) * 7;
        cairo_set_line_width(cr, 2);
        if (min == max) {
            cairo_move_to(cr, origin_x + spacing, origin_y - (0.5 * height) - spacing);
            cairo_line_to(cr, origin_x + width - spacing, origin_y - (0.5 * height) - spacing);
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
            cairo_stroke(cr);
        } else {
            n = 1;
            //labelArray = [[NSMutableArray alloc] init];
            cairo_select_font_face(cr, "default", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL); // Lucida Grande
            cairo_set_font_size(cr, 12);
            cairo_text_extents (cr, "β₁β₂β₃β₄,", &extents);
            for (i = 0; i < window->numberOfLevels; i++) {
                if(degeneracy[i] != 0 && window->energyLevel[i] != FLT_MAX) {
                    oneLevelWidth = (width - spacing * (degeneracy[i] + 1)) / degeneracy[i];
                    energyPosition = origin_y - ((window->energyLevel[i] - min) / (max - min) * height) - spacing;
                    for (j = 0; j < degeneracy[i]; j++) {
                        // Draw levels
                        cairo_move_to(cr, origin_x + (j + 1) * spacing + j * oneLevelWidth, energyPosition);
                        cairo_line_to(cr, origin_x + (j + 1) * spacing + (j + 1) * oneLevelWidth, energyPosition);
                        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
                        cairo_stroke(cr);
                        // Draw transitions
                        GPtrArray *transitionsForLevel;
                        for (t = 0; t < spins; t++) {
                            transitionsForLevel = g_ptr_array_index(transitionArray2D, transitionIndex);
                            valuePtr = g_ptr_array_index(transitionsForLevel, 2 * t);
                            energyEndPoint = *valuePtr;
                            if (energyEndPoint != FLT_MAX) {
                                energyStartPoint = origin_y - ((energyLevelCopy[transitionIndex] - min) / (max - min) * height) - spacing;
                                energyEndPoint = origin_y - ((energyEndPoint - min) / (max - min) * height) - spacing;
                                signumDeltaE = (int)copysign(1, energyEndPoint - energyStartPoint);
                                xPosition = origin_x + spacing + distance * ((n <= 32) ? n : n - 32);
                                // Transition line
                                cairo_move_to(cr, xPosition, energyStartPoint);
                                cairo_line_to(cr, xPosition, energyEndPoint);
                                // Bottom arrow
                                cairo_move_to(cr, xPosition, energyStartPoint);
                                cairo_line_to(cr, xPosition - 3, energyStartPoint + 6 * signumDeltaE);
                                cairo_move_to(cr, xPosition, energyStartPoint);
                                cairo_line_to(cr, xPosition + 3, energyStartPoint + 6 * signumDeltaE);
                                // Top arrow
                                cairo_move_to(cr, xPosition, energyEndPoint);
                                cairo_line_to(cr, xPosition - 3, energyEndPoint - 6 * signumDeltaE);
                                cairo_move_to(cr, xPosition, energyEndPoint);
                                cairo_line_to(cr, xPosition + 3, energyEndPoint - 6 * signumDeltaE);
                                valuePtr = g_ptr_array_index(transitionsForLevel, 2 * t + 1);
                                cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, *valuePtr);
                                cairo_stroke(cr);
                                n++;
                            }
                        }
                        g_ptr_array_free(transitionsForLevel, TRUE);
                        transitionIndex++;
                    }
                    // Create labels
                    /*NSAttributedString *labelName = [[NSAttributedString alloc] initWithString:[levelNames objectAtIndex:i] attributes: attributes];
                    float height = 16 * degeneracy[i];
                    VerticallyCenteredTextField *label = [[VerticallyCenteredTextField alloc] initWithFrame:NSMakeRect(width - 16, energyPosition + 2 - height / 2, 60, height)];
                    [label setAttributedStringValue:labelName];
                    [labelArray addObject:label];
                    [label release];*/
                }
            }
            // Combine overlapping labels
            for (i = 0; i < window->levelNames->len - 1; i++) {
                for (j = i + 1; j < window->levelNames->len; j++) {
                    // Determine the label that is further up the energy axis
                    label1 = g_ptr_array_index(window->levelNames, i);
                    label2 = g_ptr_array_index(window->levelNames, j);
                    if(strlen(label1) > 0 && strlen(label2) > 0) {
                        label1_y = origin_y - ((window->energyLevel[i] - min) / (max - min) * height) - spacing;
                        label2_y = origin_y - ((window->energyLevel[j] - min) / (max - min) * height) - spacing;
                        if(label2_y < label1_y) {
                            temp = label1;
                            label1 = label2;
                            label2 = temp;
                        }
                        overlap = fabsf(label1_y - label2_y);
                        if (overlap < extents.height) {
                            // Combine text of overlapping labels in the upper one
                            if (*label1 == '|') {
                                temp = malloc(energy_level_str_len * sizeof(gchar));
                                strcpy(temp, label1);
                                strcat(temp, ", ");
                                strcat(temp, label2);
                                strcpy(label2, temp);
                                free(temp);
                            } else {
                                strcat(label2, ", ");
                                strcat(label2, label1);
                            }
                            *label1 = '\0';
                            // Rearrange combined label
                            window->energyLevel[j] = (window->energyLevel[i] + window->energyLevel[j]) / 2;
                        }
                    }
                }
            }
            //float rowsPresent, rowsNeeded;
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
            for (i = 0; i < window->levelNames->len; i++) {
                label1 = g_ptr_array_index(window->levelNames, i);
                label1_y = origin_y - ((window->energyLevel[i] - min) / (max - min) * height) - spacing;
                if (strcmp(label1, "")) {
                    cairo_move_to(cr, width - 16, label1_y - 2 + extents.height / 2);
    	            cairo_show_text(cr, label1);
                    cairo_stroke(cr);
                    /*rowsPresent = [[labelArray objectAtIndex:i] frame].size.height / 16;
                    rowsNeeded = [[[labelArray objectAtIndex:i] attributedStringValue] size].height / 16;
                    rowsNeeded += ceil([[[labelArray objectAtIndex:i] attributedStringValue] size].width / 59) - 1;
                    if (rowsNeeded < rowsPresent) {
                        newFrame = NSMakeRect([[labelArray objectAtIndex:i] frame].origin.x,
                                              [[labelArray objectAtIndex:i] frame].origin.y + ((rowsPresent - rowsNeeded) * 8),
                                              [[labelArray objectAtIndex:i] frame].size.width,
                                              rowsNeeded * 16);
                        [[labelArray objectAtIndex:i] setFrame:newFrame];
                    }
                    [[[labelArray objectAtIndex:i] attributedStringValue] drawInRect:[[labelArray objectAtIndex:i] frame]];*/
                }
            }
        }
        free(degeneracy);
        free(energyLevelCopy);
        g_ptr_array_free(transitionArray2D, TRUE);

        // Restore original energy level values
        for(i = 0; i < window->numberOfLevels; i++)
            window->energyLevel[i] = window->energyLevelOrig[i];
        for(i = 0; i < window->levelNames->len; i++)
            strcpy(g_ptr_array_index(window->levelNames, i), g_ptr_array_index(window->levelNamesOrig, i));
    }
}


G_MODULE_EXPORT void draw_pulseSequence_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int width, height;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_surface(cr, window->pulseSequence_surface, 0, 0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}


G_MODULE_EXPORT void draw_pulseSequenceStep_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int width, height;

    width = gtk_widget_get_allocated_width(GTK_WIDGET(window->pulseSequence_drawingarea));
    height = gtk_widget_get_allocated_height(GTK_WIDGET(window->pulseSequence_drawingarea));

    if (window->pulseSequence_surface != NULL) {
        cairo_scale(cr, 0.707, 0.707);
        cairo_set_source_surface(cr, window->pulseSequence_surface, 0, 0);
	    cairo_rectangle(cr, 0, 0, width, height);
	    cairo_fill(cr);
    }
}


void create_pulseSequence_view(InsensitiveWindow *window, int width, int height)
{
	cairo_t *cr = cairo_create(window->pulseSequence_surface);
    InsensitiveController *controller = window->controller;
	InsensitivePulseSequence *pulseSequence = controller->pulseSequence;
	SequenceElement *lastElement;
	float factor, x;
	unsigned int i, numberOfElements;//, selected;
	int colorPulseSandwich = 0, pulseIndex = 0, delayIndex = 0, gradientIndex = 0;
	float currentPosition = 60, stepWidth, heightOfRectangle;
	float stepMarkerYPosition, standardStepMarkerYPosition, lastStepYPosition, stepMarkerXPosition = -1;
	float pulseHeight;//, pulseWidth;
	gboolean lastStepHasIDecoupling, lastStepHasSDecoupling;
	gboolean sequenceInvolvesISpinAction = FALSE, sequenceInvolvesSSpinAction = FALSE;
	gboolean sequenceInvolvesISpins = FALSE, sequenceInvolvesSSpins = FALSE, sequenceInvolvesGradients = FALSE;
	SequenceElement *currentElement;

	factor = height / 283; //267;
	numberOfElements = insensitive_pulsesequence_get_number_of_elements(pulseSequence);
	lastElement = insensitive_pulsesequence_get_last_element(pulseSequence);

	// Scan for pulses and gradients
	for (i = 0; i < numberOfElements; i++) {
		currentElement = insensitive_pulsesequence_get_element_at_index(pulseSequence, i);
		if (currentElement->activeISpins || currentElement->iDecoupling) {
			sequenceInvolvesISpins = TRUE;
			if (currentElement->activeISpins)
				sequenceInvolvesISpinAction = TRUE;
		}
		if (currentElement->activeSSpins || currentElement->sDecoupling) {
			sequenceInvolvesSSpins = TRUE;
			if (currentElement->activeSSpins)
				sequenceInvolvesSSpinAction = TRUE;
		}
		if (currentElement->type == SequenceTypeGradient) {
			sequenceInvolvesGradients = TRUE;
		}
	}
	if (!sequenceInvolvesISpins && !sequenceInvolvesSSpins && !sequenceInvolvesGradients)
		sequenceInvolvesISpins = TRUE;

	window->I_line = height - 200;
	window->S_line = height - 130;
	window->G_line = height - 60;
	heightOfRectangle = 260;
	if (!sequenceInvolvesISpins) {
		window->S_line -= 70;
		window->G_line -= 70;
		heightOfRectangle -= 70;
	}
	if (!sequenceInvolvesSSpins) {
		window->G_line -= 70;
		heightOfRectangle -= 70;
	}
	if (!sequenceInvolvesGradients) {
		window->G_line -= 70;
		heightOfRectangle -= 70;
	}
	i = 1 << sequenceInvolvesISpins;
	i <<= sequenceInvolvesSSpins;
	i <<= sequenceInvolvesGradients;
	i = lb(i);
	if (i == 1) {
		window->I_line += 70;
		window->S_line += 70;
		window->G_line += 70;
	} else if (i == 2) {
		window->I_line += 35;
		window->S_line += 35;
		window->G_line += 35;
	}
	window->I_line *= factor;
	window->S_line *= factor;
	window->G_line *= factor;

	if (numberOfElements > 0) {
		if (lastElement->type == SequenceTypeFID) {
			standardStepMarkerYPosition = (lastElement->activeSSpins) ? window->S_line : window->I_line;
		} else {
			standardStepMarkerYPosition = (!sequenceInvolvesISpins && sequenceInvolvesSSpins) ? window->S_line : window->I_line;
		}
	} else {
		standardStepMarkerYPosition = window->I_line;
	}
	stepMarkerYPosition = standardStepMarkerYPosition;

	// Rescale the labels (I, S, Gz)
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	cairo_select_font_face(cr, "Verdana", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    if (!cairo_get_font_face(cr))
	    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 24 * factor);
	if (sequenceInvolvesISpins) {
		cairo_move_to(cr, 22 * factor, window->I_line - 10 * factor);
		cairo_show_text(cr, "I");
	}
	if (sequenceInvolvesSSpins) {
		cairo_move_to(cr, 22 * factor, window->S_line - 10 * factor);
		cairo_show_text(cr, "S");
	}
	if (sequenceInvolvesGradients) {
		cairo_move_to(cr, 22 * factor, window->G_line - 10 * factor);
		cairo_show_text(cr, "G");
		cairo_set_font_size(cr, 14 * factor);
		cairo_move_to(cr, 41 * factor, window->G_line - 8 * factor);
		cairo_show_text(cr, "z");
	}

	cairo_set_line_width(cr, 2 * factor);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);

	// Draw beginning of time lines for I and S spins and Gradients
	if (sequenceInvolvesISpins) {
		cairo_move_to(cr, 20 * factor, window->I_line);
		cairo_line_to(cr, currentPosition * factor, window->I_line);
	}
	if (sequenceInvolvesSSpins) {
		cairo_move_to(cr, 20 * factor, window->S_line);
		cairo_line_to(cr, currentPosition * factor, window->S_line);
	}
	if (sequenceInvolvesGradients) {
		cairo_move_to(cr, 20 * factor, window->G_line);
		cairo_line_to(cr, currentPosition * factor, window->G_line);
	}
	cairo_stroke(cr);

	lastStepHasIDecoupling = FALSE;
	lastStepHasSDecoupling = FALSE;

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

	for (i = 0; i < numberOfElements; i++) {
		currentElement = insensitive_pulsesequence_get_element_at_index(pulseSequence, i);
		// Position step marker if only if this is the current step of the sequence
		if (i == window->nextStepInPulseSequence) {
			stepMarkerXPosition = currentPosition;
			switch (currentElement->type) {
			case SequenceTypeGradient:
				stepMarkerYPosition = window->G_line;
				break;
			case SequenceTypeFID:
				stepMarkerYPosition = standardStepMarkerYPosition;
				break;
			case SequenceTypePulse:
				if (currentElement->activeSSpins && !currentElement->activeISpins)
					stepMarkerYPosition = window->S_line;
				else if (currentElement->activeISpins && !currentElement->activeSSpins)
					stepMarkerYPosition = window->I_line;
				else
					stepMarkerYPosition = standardStepMarkerYPosition;
				break;
			default:
				stepMarkerYPosition = lastStepYPosition;
				break;
			}
		}
		// then save the position anyway in case the next step is a delay
		switch (currentElement->type) {
		case SequenceTypeGradient:
			lastStepYPosition = window->G_line;
			break;
		case SequenceTypeFID:
			lastStepYPosition = standardStepMarkerYPosition;
			break;
		case SequenceTypePulse:
			if (currentElement->activeSSpins && !currentElement->activeISpins)
				lastStepYPosition = window->S_line;
			else if (currentElement->activeISpins && !currentElement->activeSSpins)
				lastStepYPosition = window->I_line;
			else
				lastStepYPosition = standardStepMarkerYPosition;
			break;
		default:
			break;
		}
		switch (currentElement->type) {
		case SequenceTypePulse:
			pulseIndex++;
			stepWidth = currentElement->time / 360 * 50;
			if (currentElement == window->editedElement)
				cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 1.0);
			else
				cairo_set_source_rgba(cr, 1.0 - ((int)currentElement->secondParameter % 180) / 360, 0.0, 0.0, 1.0 - (float)((int)currentElement->secondParameter % 180) / 180);
			// Hard pulse on I spins
			if (currentElement->activeISpins || currentElement->selectiveIPulse) {
				if (currentElement->pulseStrength > 1.0)
					pulseHeight = 1.0;
				else if (currentElement->pulseStrength <= 0.001)
					pulseHeight = 0.25;
				else
					pulseHeight = (log10f(currentElement->pulseStrength) + 4) / 4;
				//pulseWidth = 1.0;// currentElement->pulseDuration * 10;
				if (currentElement->pulseEnvelope == Rectangle) {
					cairo_rectangle(cr, currentPosition * factor, window->I_line, stepWidth * factor, -pulseHeight * 50 * factor);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
				} else if (currentElement->pulseEnvelope == Gaussian) {
					cairo_move_to(cr, currentPosition * factor, window->I_line);
					for (x = 0; x < stepWidth; x += 0.25)
						cairo_line_to(cr, (currentPosition + x) * factor, window->I_line - 50 * sin(x * M_PI / stepWidth) * factor * pulseHeight);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
					cairo_close_path(cr);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
				} else if (currentElement->pulseEnvelope == DANTE) {
					cairo_move_to(cr, currentPosition * factor, window->I_line);
					for (x = 0; x < stepWidth; x += stepWidth / 4) {
						cairo_line_to(cr, (currentPosition + x) * factor, window->I_line - pulseHeight * 50 * factor);
						cairo_line_to(cr, (currentPosition + x + stepWidth / 16) * factor, window->I_line - pulseHeight * 50 * factor);
						cairo_line_to(cr, (currentPosition + x + stepWidth / 16) * factor, window->I_line);
						cairo_line_to(cr, (currentPosition + x + stepWidth / 4) * factor, window->I_line);
					}
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
					cairo_close_path(cr);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
				} else {
					cairo_move_to(cr, currentPosition * factor, window->I_line);
					for (x = 0; x < stepWidth; x += 0.25)
						cairo_line_to(cr, (currentPosition + x) * factor, window->I_line - 50 * sinc((x - (stepWidth / 2)) * M_PI / (stepWidth / 6)) * factor * pulseHeight);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
					cairo_close_path(cr);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
				}
				lastStepHasIDecoupling = FALSE;
				add_label_for_element(cr, currentElement->type,
						      (currentPosition + stepWidth / 2) * factor,
						      window->I_line - 57 * factor,
						      pulseIndex);
			// I spin decoupling
			} else if (currentElement->iDecoupling) {
				if (currentElement->spinlock) {
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
				} else
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
				cairo_rectangle(cr, currentPosition * factor, window->I_line, stepWidth * factor, -20 * factor);
				cairo_fill_preserve(cr);
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_stroke(cr);
				if (lastStepHasIDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, (currentPosition - 2) * factor, window->I_line - factor, 4 * factor, -18 * factor);
					cairo_fill(cr);
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				}
				lastStepHasIDecoupling = TRUE;
			// No pulse on I spins
			} else {
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_move_to(cr, currentPosition * factor, window->I_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
				cairo_stroke(cr);
				lastStepHasIDecoupling = FALSE;
			}
			// S spins
			if (sequenceInvolvesSSpins) {
				if (currentElement == window->editedElement)
					cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 1.0);
				else
					cairo_set_source_rgba(cr, 1.0 - ((int)currentElement->secondParameter % 180) / 360, 0.0, 0.0, 1.0 - (float)((int)currentElement->secondParameter % 180) / 180);
				// Hard pulse on S spins
				if (currentElement->activeSSpins || currentElement->selectiveSPulse) {
					if (currentElement->pulseStrength > 1.0)
						pulseHeight = 1.0;
					else if (currentElement->pulseStrength <= 0.001)
						pulseHeight = 0.25;
					else
						pulseHeight = (log10f(currentElement->pulseStrength) + 4) / 4;
					if (currentElement->pulseEnvelope == Rectangle) {
						cairo_rectangle(cr, currentPosition * factor, window->S_line, stepWidth * factor, -pulseHeight * 50 * factor);
						cairo_fill_preserve(cr);
						cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						cairo_stroke(cr);
					} else if (currentElement->pulseEnvelope == Gaussian) {
						cairo_move_to(cr, currentPosition * factor, window->S_line);
						for (float x = 0; x < stepWidth; x += 0.25)
							cairo_line_to(cr, (currentPosition + x) * factor, window->S_line - 50 * sin(x * M_PI / stepWidth) * factor * pulseHeight);
						cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
						cairo_close_path(cr);
						cairo_fill_preserve(cr);
						cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						cairo_stroke(cr);
					} else if (currentElement->pulseEnvelope == DANTE) {
						cairo_move_to(cr, currentPosition * factor, window->S_line);
						for (x = 0; x < stepWidth; x += stepWidth / 4) {
							cairo_line_to(cr, (currentPosition + x) * factor, window->S_line - pulseHeight * 50 * factor);
							cairo_line_to(cr, (currentPosition + x + stepWidth / 16) * factor, window->S_line - pulseHeight * 50 * factor);
							cairo_line_to(cr, (currentPosition + x + stepWidth / 16) * factor, window->S_line);
							cairo_line_to(cr, (currentPosition + x + stepWidth / 4) * factor, window->S_line);
						}
						cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
						cairo_close_path(cr);
						cairo_fill_preserve(cr);
						cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						cairo_stroke(cr);
					} else {
						cairo_move_to(cr, currentPosition * factor, window->S_line);
						for (x = 0; x < stepWidth; x += 0.25)
							cairo_line_to(cr, (currentPosition + x) * factor, window->S_line - 50 * sinc((x - (stepWidth / 2)) * M_PI / (stepWidth / 6)) * factor * pulseHeight);
						cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
						cairo_close_path(cr);
						cairo_fill_preserve(cr);
						cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						cairo_stroke(cr);
					}
					lastStepHasSDecoupling = FALSE;
					add_label_for_element(cr, currentElement->type,
							      (currentPosition + stepWidth / 2) * factor,
							      window->S_line - 57 * factor,
							      pulseIndex);
				// S spin decoupling
				} else if (currentElement->sDecoupling) {
					if (currentElement->spinlock) {
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					} else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, currentPosition * factor, window->S_line, stepWidth * factor, -20 * factor);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
					if (lastStepHasSDecoupling) {
						if (currentElement->spinlock)
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
						else
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						cairo_rectangle(cr, (currentPosition - 2) * factor, window->S_line - factor, 4 * factor, -18 * factor);
						cairo_fill(cr);
                        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					}
					lastStepHasSDecoupling = TRUE;
			    // No pulse on S spins
				} else {
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_move_to(cr, currentPosition * factor, window->S_line);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
					cairo_stroke(cr);
					lastStepHasSDecoupling = FALSE;
				}
			}
			if (sequenceInvolvesGradients) {
				cairo_move_to(cr, currentPosition * factor, window->G_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->G_line);
				cairo_stroke(cr);
			}
			// Draw rectangle over selected spin echo sandwich
			if (colorPulseSandwich > 0) {
                cairo_rectangle(cr, currentPosition * factor, window->G_line + 50 * factor, stepWidth * factor, -heightOfRectangle * factor);
				cairo_set_source_rgba(cr, 0.5, 0.5, 1.0, 0.15);
				cairo_fill(cr);
				colorPulseSandwich--;
			}
			currentPosition += stepWidth;
			break;
		case SequenceTypeEvolution:
			stepWidth = currentElement->time * 100;
			// Check for decoupling of I spins
			if (currentElement->iDecoupling) {
				if (currentElement->spinlock)
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
				else
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
				cairo_rectangle(cr, currentPosition * factor, window->I_line, stepWidth * factor, -20 * factor);
				cairo_fill_preserve(cr);
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_stroke(cr);
				if (lastStepHasIDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, (currentPosition - 2) * factor, window->I_line - factor, 4 * factor, -18 * factor);
					cairo_fill(cr);
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				}
				lastStepHasIDecoupling = TRUE;
			} else {
                cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_move_to(cr, currentPosition * factor, window->I_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
				cairo_stroke(cr);
				lastStepHasIDecoupling = FALSE;
			}
			// Check for decoupling of S spins
			if (sequenceInvolvesSSpins) {
				if (currentElement->sDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, currentPosition * factor, window->S_line, stepWidth * factor, -20 * factor);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
					if (lastStepHasSDecoupling) {
						if (currentElement->spinlock)
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
						else
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						cairo_rectangle(cr, (currentPosition - 2) * factor, window->S_line - factor, 4 * factor, -18 * factor);
						cairo_fill(cr);
                        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					}
					lastStepHasSDecoupling = TRUE;
				} else {
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_move_to(cr, currentPosition * factor, window->S_line);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
					cairo_stroke(cr);
					lastStepHasSDecoupling = FALSE;
				}
			}
			if (sequenceInvolvesGradients) {
				cairo_move_to(cr, currentPosition * factor, window->G_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->G_line);
				cairo_stroke(cr);
			}
			// Draw rectangle over selected evolution time
			if ((abs(window->variableEvolutionTime) == (int)i + 1) || (colorPulseSandwich > 0)) {
				// ------------------------------------------------------------------------------ //
				delayIndex++; // Comment out if any t1 time should not increment the delay index. //
				// ------------------------------------------------------------------------------ //
				if (-window->variableEvolutionTime == (int)i + 1) {
					if (insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 1)->type == SequenceTypePulse)
						colorPulseSandwich = 3;
					else if (insensitive_pulsesequence_get_element_at_index(pulseSequence, i + 1)->type == SequenceTypeGradient)
						colorPulseSandwich = 5;
				}
				cairo_rectangle(cr, currentPosition * factor, window->G_line + 50 * factor, stepWidth * factor, -heightOfRectangle * factor);
				if (currentElement == window->editedElement)
					cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 0.25);
				else
					cairo_set_source_rgba(cr, 0.5, 0.5, 1.0, 0.15);
				cairo_fill(cr);
				if (colorPulseSandwich > 0)
					colorPulseSandwich--;
				if (sequenceInvolvesISpinAction)
					add_label_for_element(cr, SequenceTypeFID,
							      (currentPosition + stepWidth / 2) * factor,
							      window->I_line - 27 * factor,
							      (window->variableEvolutionTime < 1) ? -1 : 1);
				if (sequenceInvolvesSSpinAction)
					add_label_for_element(cr, SequenceTypeFID,
							      (currentPosition + stepWidth / 2) * factor,
							      window->S_line - 27 * factor,
							      (window->variableEvolutionTime < 1) ? -1 : 1);
			} else {
				delayIndex++;
				if (sequenceInvolvesISpinAction)
					add_label_for_element(cr, currentElement->type,
							      (currentPosition + stepWidth / 2) * factor,
							      window->I_line - 27 * factor,
							      delayIndex);
				if (sequenceInvolvesSSpinAction)
					add_label_for_element(cr, currentElement->type,
							      (currentPosition + stepWidth / 2) * factor,
							      window->S_line - 27 * factor,
							      delayIndex);
				// Draw rectangle over delay being edited
				if (currentElement == window->editedElement) {
					cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 0.25);
					cairo_rectangle(cr, currentPosition * factor, window->G_line + 50 * factor, stepWidth * factor, -heightOfRectangle * factor);
					cairo_fill(cr);
				}
			}
			currentPosition += stepWidth;
			break;
		case SequenceTypeGradient:
			stepWidth = currentElement->time * 30;
			if (currentElement->iDecoupling) {
				if (currentElement->spinlock)
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
				else
					cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
				cairo_rectangle(cr, currentPosition * factor, window->I_line, stepWidth * factor, -20 * factor);
				cairo_fill_preserve(cr);
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_stroke(cr);
				if (lastStepHasIDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, (currentPosition - 2) * factor, window->I_line - factor, 4 * factor, -18 * factor);
					cairo_fill(cr);
				}
				lastStepHasIDecoupling = TRUE;
			} else {
                cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
				cairo_move_to(cr, currentPosition * factor, window->I_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
				cairo_stroke(cr);
				lastStepHasIDecoupling = FALSE;
			}
			if (sequenceInvolvesSSpins) {
				if (currentElement->sDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, currentPosition * factor, window->S_line, stepWidth * factor, -20 * factor);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
					if (lastStepHasSDecoupling) {
						if (currentElement->spinlock)
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
						else
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						cairo_rectangle(cr, (currentPosition - 2) * factor, window->S_line - factor, 4 * factor, -18 * factor);
						cairo_fill(cr);
					}
					lastStepHasSDecoupling = TRUE;
				} else {
                    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_move_to(cr, currentPosition * factor, window->S_line);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
					cairo_stroke(cr);
					lastStepHasSDecoupling = FALSE;
				}
			}
			if (currentElement == window->editedElement)
				cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 1.0);
			else
				cairo_set_source_rgba(cr, 0.08, 0.82, 0.35, fabs(currentElement->secondParameter) / 32000);
			cairo_move_to(cr, currentPosition * factor, window->G_line);
			for (x = 0; x < stepWidth; x += 0.25)
				cairo_line_to(cr, (currentPosition + x) * factor, window->G_line - currentElement->secondParameter / 640 * sin(x * M_PI / stepWidth) * factor);
			cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->G_line);
			cairo_close_path(cr);
			cairo_fill_preserve(cr);
			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
			cairo_stroke(cr);
			add_label_for_element(cr, currentElement->type,
					      (currentPosition + stepWidth / 2) * factor,
					      window->G_line - 7 * factor - ((currentElement->secondParameter > 0) ? currentElement->secondParameter : 0) / 640 * factor,
					      ++gradientIndex);
			// Draw rectangle over selected spin echo sandwich
			if (colorPulseSandwich > 0) {
				cairo_rectangle(cr, currentPosition * factor, window->G_line + 50 * factor, stepWidth * factor, -heightOfRectangle * factor);
				cairo_set_source_rgba(cr, 0.5, 0.5, 1.0, 0.15);
				cairo_fill(cr);
				colorPulseSandwich--;
			}
			currentPosition += stepWidth;
			break;
		case SequenceTypeFID:
			stepWidth = 100;
			cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
			cairo_move_to(cr, currentPosition * factor, window->I_line);
			if (currentElement->activeISpins) {
				for (x = 0; x < stepWidth; x++)
					cairo_line_to(cr, (currentPosition + x) * factor, window->I_line - 30 * sin(x * 0.6) * exp(-x / 20) * factor);
				cairo_stroke(cr);
				add_label_for_element(cr, currentElement->type,
						      (currentPosition + stepWidth / 2) * factor,
						      window->I_line - 27 * factor,
						      (window->variableEvolutionTime == 0) ? 1 : 2);
			} else {
				if (currentElement->iDecoupling) {
					if (currentElement->spinlock)
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					else
						cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
					cairo_rectangle(cr, currentPosition * factor, window->I_line, stepWidth * factor, -20 * factor);
					cairo_fill_preserve(cr);
					cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					cairo_stroke(cr);
					if (lastStepHasIDecoupling) {
                        if (currentElement->spinlock)
						    cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					    else
						    cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						cairo_rectangle(cr, (currentPosition - 2) * factor, window->I_line - factor, 4 * factor, -18 * factor);
						cairo_fill(cr);
                        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
					}
					lastStepHasIDecoupling = TRUE;
				} else if (sequenceInvolvesISpins) {
					cairo_move_to(cr, currentPosition * factor, window->I_line);
					cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->I_line);
					cairo_stroke(cr);
					lastStepHasIDecoupling = FALSE;
				}
			}
			if (sequenceInvolvesSSpins) {
				cairo_move_to(cr, currentPosition * factor, window->S_line);
				if (currentElement->activeSSpins) {
					for (x = 0; x < stepWidth; x += 0.25)
						cairo_line_to(cr, (currentPosition + x) * factor, window->S_line - 30 * sin(x * 0.6) * exp(-x / 20) * factor);
					cairo_stroke(cr);
					add_label_for_element(cr, currentElement->type,
							      (currentPosition + stepWidth / 2) * factor,
							      window->S_line - 27 * factor,
							      (window->variableEvolutionTime == 0) ? 1 : 2);
				} else {
					if (currentElement->sDecoupling) {
						if (currentElement->spinlock)
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
						else
							cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						cairo_rectangle(cr, currentPosition * factor, window->S_line, stepWidth * factor, -20 * factor);
						cairo_fill_preserve(cr);
						cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						cairo_stroke(cr);
						if (lastStepHasSDecoupling) {
                            if (currentElement->spinlock)
						        cairo_set_source_rgba(cr, 1.0, 0.5, 0.8, 1.0);
					        else
						        cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
						    cairo_rectangle(cr, (currentPosition - 2) * factor, window->S_line - factor, 4 * factor, -18 * factor);
						    cairo_fill(cr);
                            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
						}
						lastStepHasSDecoupling = TRUE;
					} else {
						cairo_move_to(cr, currentPosition * factor, window->S_line);
						cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->S_line);
						cairo_stroke(cr);
						lastStepHasSDecoupling = FALSE;
					}
				}
			}
			if (sequenceInvolvesGradients) {
				cairo_move_to(cr, currentPosition * factor, window->G_line);
				cairo_line_to(cr, (currentPosition + stepWidth) * factor, window->G_line);
				cairo_stroke(cr);
			}
			// Draw rectangle over delay being edited
			if (currentElement == window->editedElement) {
				cairo_set_source_rgba(cr, 0.99, 0.8, 0.4, 0.25);
                cairo_rectangle(cr, currentPosition * factor, window->G_line + 50 * factor, stepWidth * factor, -heightOfRectangle * factor);
				cairo_fill(cr);
			}
			currentPosition += stepWidth;
			break;
		default:
			g_print("I don't understand this command!\n");
		}
	}
	if (stepMarkerXPosition > 0 && window->nextStepInPulseSequence > 0) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
		cairo_move_to(cr, (stepMarkerXPosition - 6) * factor, stepMarkerYPosition);
		cairo_line_to(cr, stepMarkerXPosition * factor, stepMarkerYPosition + 6 * factor);
		cairo_line_to(cr, (stepMarkerXPosition + 6) * factor, stepMarkerYPosition);
		cairo_line_to(cr, stepMarkerXPosition * factor, stepMarkerYPosition - 6 * factor);
		cairo_close_path(cr);
		cairo_fill_preserve(cr);
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		cairo_stroke(cr);
	}
}


void add_label_for_element(cairo_t *cr, enum SequenceType type, float x, float y, int index)
{
	gchar *string = malloc(14 * sizeof(gchar));
	gchar *string_with_indices, *x_char;
	cairo_text_extents_t extents;

	if (type == SequenceTypePulse)
		sprintf(string, "φ%d", index);
	else if (type == SequenceTypeEvolution)
		sprintf(string, "τ%d", index);
	else if (type == SequenceTypeGradient)
		sprintf(string, "G%d", index);
	else if (type == SequenceTypeFID) {
		if (index < 0)
			sprintf(string, "t%d/x", -index);
		else
			sprintf(string, "t%d", index);
	}
	string_with_indices = replace_numbers_by_indices(string);
	free(string);
	x_char = strchr(string_with_indices, 'x');
	if (x_char != NULL)
		*x_char = '2';

	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	cairo_select_font_face(cr, "LucidaGrande", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    if (!cairo_get_font_face(cr))
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 12);
	cairo_text_extents(cr, string_with_indices, &extents);
	cairo_move_to(cr, x - (extents.width + extents.x_bearing) / 2, y);
	cairo_show_text(cr, string_with_indices);
	free(string_with_indices);
}


G_MODULE_EXPORT int get_sequenceElementIndex_from_mouse_position(InsensitiveWindow *window, float mousePosition)
{
    int i;
    float position, stepWidth;
    InsensitivePulseSequence *pulseSequence = window->controller->pulseSequence;
    SequenceElement *element;

    position = 60;
    for (i = 0; i < insensitive_pulsesequence_get_number_of_elements(pulseSequence); i++) {
        element = insensitive_pulsesequence_get_element_at_index(pulseSequence, i);
        switch(element->type) {
            case SequenceTypePulse:
                stepWidth = element->time / 360 * 50;
                break;
            case SequenceTypeShift:
            case SequenceTypeCoupling:
            case SequenceTypeRelaxation:
            case SequenceTypeEvolution:
                stepWidth = element->time * 100;
                break;
            case SequenceTypeGradient:
                stepWidth = element->time * 30;
                break;
            case SequenceTypeFID:
                stepWidth = 100;
        }
        if(mousePosition > position && mousePosition <= position + stepWidth)
            return i;
        else
            position += stepWidth;
    }
    return -1;
}


G_MODULE_EXPORT void on_pulseSequence_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int index;
    SequenceElement *selectedElement;
    float y = event->y;// - [sequenceScrollView frame].origin.y;

    if(TRUE || window->userInteractionEnabled) {
        index = get_sequenceElementIndex_from_mouse_position(window, event->x /*+ + [sequenceScrollView documentVisibleRect].origin.x*/);
        if(index >= 0) {
            selectedElement = insensitive_pulsesequence_get_element_at_index(window->controller->pulseSequence, index);
            if ((y < window->I_line && y > window->I_line - 50 && selectedElement->type == SequenceTypePulse && (selectedElement->activeISpins || selectedElement->selectiveIPulse))
                || (y < window->S_line && y > window->S_line - 50 && selectedElement->type == SequenceTypePulse && (selectedElement->activeSSpins || selectedElement->selectiveSPulse))
                || (y < window->G_line + 50 && y > window->G_line - 50 && selectedElement->type == SequenceTypeGradient)
                || (y < window->I_line + 50 && y > window->I_line - 50 && selectedElement->type == SequenceTypeFID && selectedElement->activeISpins)
                || (y < window->S_line + 50 && y > window->S_line - 50 && selectedElement->type == SequenceTypeFID && selectedElement->activeSSpins)
                || selectedElement->type == SequenceTypeEvolution)
                edit_sequence_element(window, index);
            else
                cancel_editing_sequence_element(window);
        } else
            cancel_editing_sequence_element(window);
    }
}


G_MODULE_EXPORT void draw_coherencePathway_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	InsensitiveWindow *window = (InsensitiveWindow *)user_data;
	unsigned int i, step;
	float *alpha;
	float complex *tuple;
    gchar *label;
	GPtrArray *path;
    cairo_text_extents_t extents;

	if (window->alphaArray != NULL && window->pathArray != NULL && window->labelArray != NULL) {
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_select_font_face(cr, "LucidaGrande", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	    cairo_set_font_size(cr, 12);
        cairo_move_to(cr, 18.0 * window->pathway_scaling, 12.0 * window->pathway_scaling);
        if (window->numberOfISpinOrders > 1 && window->numberOfISpinPathways > 0)
		    cairo_show_text(cr, "Coherence Pathway for I Spins:");
        else if (window->numberOfSSpinOrders > 1 && window->numberOfSSpinPathways > 0)
            cairo_show_text(cr, "Coherence Pathway for S Spins:");
        if(window->numberOfISpinOrders > 1 && window->numberOfISpinPathways > 0
           && window->numberOfSSpinOrders > 1 && window->numberOfSSpinPathways > 0) {
            cairo_move_to(cr, 18.0 * window->pathway_scaling, (44 + window->numberOfISpinOrders * 15) * window->pathway_scaling);
            cairo_show_text(cr, "Coherence Pathway for S Spins:");
        }
        if (window->labelArray->len > 0)
            for (i = 0; i < window->labelArray->len / 2; i++) {
				tuple = g_ptr_array_index(window->labelArray, 2 * i);
                label = g_ptr_array_index(window->labelArray, 2 * i + 1);
                cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
                cairo_select_font_face(cr, "LucidaGrande", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	            cairo_set_font_size(cr, 12);
	            cairo_text_extents(cr, label, &extents);
	            cairo_move_to(cr, crealf(*tuple) - (extents.width + extents.x_bearing), cimagf(*tuple));
	            cairo_show_text(cr, label);
            }
        cairo_set_line_width(cr, 2.0 * window->pathway_scaling);
		if (window->alphaArray->len == window->pathArray->len)
			for (i = 0; i < window->pathArray->len; i++) {
                path = g_ptr_array_index(window->pathArray, i);
				alpha = g_ptr_array_index(window->alphaArray, i);
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, *alpha);
				tuple = g_ptr_array_index(path, 0);
				cairo_move_to(cr, crealf(*tuple), cimagf(*tuple));
				for (step = 1; step < path->len; step++) {
					tuple = g_ptr_array_index(path, step);
					cairo_line_to(cr, crealf(*tuple), cimagf(*tuple));
				}
				cairo_stroke(cr);
			}
	}
}


G_MODULE_EXPORT void draw_graph_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    int width, height;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    if (window->spectrum_surface != NULL)
        cairo_surface_destroy(window->spectrum_surface);
    window->spectrum_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    create_graph_view(window, width, height);

    cairo_set_source_surface(cr, window->spectrum_surface, 0, 0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}


void create_graph_view(InsensitiveWindow *window, int surface_width, int surface_height)
{
	cairo_t *cr = cairo_create(window->spectrum_surface);
    unsigned int i, x, y;
    int *peak, pos;
    float stepSizeX, stepSizeY, deflection;
    float current_x, current_y;
    float width, height, origin_x, origin_y;//, center_y;
    const int gap_top = GAP_TOP;
    const int gap_bottom = GAP_BOTTOM;
    const int gap_left = GAP_LEFT;
    const int gap_right = window->twoDimensionalSpectrum ? 58 : 27;
    float tickmark_value;
    gchar *ticklabel, *unit, *frequencyLabel;
    float freq1, freq2;
    float lastValueOutOfRange;

    width = surface_width;
    height = surface_height;
    origin_x = 0.0;
    origin_y = 0.0;

    if (window->drawScale) {
        origin_x += gap_left;
        origin_y += gap_top;
        width -= gap_left + gap_right;
        height -= gap_top + gap_bottom;
    }
    current_x = width / 10;
    current_y = height / 10;
    if (!window->twoDimensionalSpectrum || window->plotMode != Stacked) {
        if (insensitive_settings_get_showGrid(window->controller->settings)) {
            cairo_set_line_width(cr, 0.5);
            cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
            for (i = 0; i <= 10; i++) {
                cairo_move_to(cr, i * current_x + origin_x, origin_y);
                cairo_line_to(cr, i * current_x + origin_x, height + origin_y);
                cairo_move_to(cr, origin_x, i * current_y + origin_y);
                cairo_line_to(cr, width + origin_x, i * current_y + origin_y);

            }
            cairo_stroke(cr);
        }
    }
    if (window->drawScale && (window->plotMode != Stacked || !window->twoDimensionalSpectrum)) {
        cairo_set_line_width(cr, 1.0);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 12);
        ticklabel = malloc(10 * sizeof(gchar));
        unit = malloc(7 * sizeof(gchar));
        // Draw x axis
        if (window->showsFrequencyDomain)
            strcpy(unit, "Hz\0");
        else
            strcpy(unit, "s\0");
        cairo_move_to(cr, origin_x, origin_y + height + 3.0);
        cairo_line_to(cr, origin_x + width, origin_y + height + 3.0);
        for (i = 0; i <= 10; i++) {
            if (window->showsFrequencyDomain)
                tickmark_value = ((float)i / 10.0 - 0.5) / insensitive_settings_get_dwellTime(window->controller->settings);
            else
                tickmark_value = (float)i * 10.0 * insensitive_settings_get_dwellTime(window->controller->settings);
            cairo_move_to(cr, i * current_x + origin_x, origin_y + height + 3.0);
            cairo_line_to(cr, i * current_x + origin_x, origin_y + height + 8.0);
            if (i == 10) {
                sprintf(ticklabel, "%2.2f %s", tickmark_value, unit);
                cairo_move_to(cr, i * current_x + origin_x - 24.0, origin_y + height + 20.0);
            } else {
                sprintf(ticklabel, "%2.2f", tickmark_value);
                cairo_move_to(cr, i * current_x + origin_x - ((tickmark_value < 0) ? 11.0 : 4.0), origin_y + height + 20.0);
            }
            cairo_show_text(cr, ticklabel);
        }
        // Draw y axis
        if (window->twoDimensionalSpectrum) {
            if (window->spectrumIsDOSY2D)
                strcpy(unit, "cm²/s\0");
            else if (window->shows2DFrequencyDomain)
                strcpy(unit, "Hz\0");
            else
                strcpy(unit, "s\0");
            cairo_move_to(cr, origin_x + width + 3.0, origin_y);
            cairo_line_to(cr, origin_x + width + 3.0, origin_y + height);
            for (i = 0; i <= 10; i++) {
                if (window->showsFrequencyDomain)
                    tickmark_value = ((float)i / 10.0 - 0.5) / insensitive_settings_get_dwellTime(window->controller->settings);
                else
                    tickmark_value = (float)i * 10.0 * insensitive_settings_get_dwellTime(window->controller->settings);
                cairo_move_to(cr, origin_x + width + 3.0, i * current_y + origin_y);
                cairo_line_to(cr, origin_x + width + 8.0, i * current_y + origin_y);
                if (window->spectrumIsDOSY2D) {
                    if (i == 0)
                        sprintf(ticklabel, "%s", unit);
                    else
                        strcpy(ticklabel, "");
                } else {
                    if (i == 0)
                        sprintf(ticklabel, "%2.1f %s", tickmark_value, unit);
                    else
                        sprintf(ticklabel, "%2.1f", tickmark_value);
                }
                cairo_move_to(cr, origin_x + width + 8.0 + 2.0, i * current_y + origin_y + 4.0);
                cairo_show_text(cr, ticklabel);
            }
        }
        cairo_stroke(cr);
        free(ticklabel);
        free(unit);
    }
    // 2D spectra (stacked plot)
    if(window->twoDimensionalSpectrum && window->plotMode == Stacked) {
        origin_y += height - 0.1 * height;
        height *= 0.8;
        stepSizeX = width / (window->indirectDataPoints + window->lastDataPointDisplayed);
        stepSizeY = height / window->indirectDataPoints;
        cairo_set_line_width(cr, 1);
        for(y = 0; y < window->indirectDataPoints; y++) {
            current_x = origin_x + stepSizeX * (window->indirectDataPoints - y);
            current_y = origin_y - stepSizeY * (window->indirectDataPoints - y);
            // Imaginary part
            if(window->drawImaginaryPart) {
                for(x = 0; x < window->lastDataPointDisplayed; x++) {
                    if(isnan(window->displayedData.imagp[y * window->lastDataPointDisplayed + x]))
                        window->displayedData.imagp[y * window->lastDataPointDisplayed + x] = 0.0;
                    if(x == 0)
                        cairo_move_to(cr,
                                      current_x + x * stepSizeX,
                                      current_y + 2 - window->magnification * window->displayedData.imagp[y * window->lastDataPointDisplayed + x]);
                    else
                        cairo_line_to(cr,
                                      current_x + x * stepSizeX,
                                      current_y + 2 - window->magnification * window->displayedData.imagp[y * window->lastDataPointDisplayed + x]);
                }
                cairo_set_source_rgba(cr, 0.0, 0.75, 0.0, 1.0);
                cairo_stroke(cr);
            }
            // Real part
            if(window->drawRealPart) {
                for(x = 0; x < window->lastDataPointDisplayed; x++) {
                    if(isnan(window->displayedData.realp[y * window->lastDataPointDisplayed + x]))
                        window->displayedData.realp[y * window->lastDataPointDisplayed + x] = 0.0;
                    if(x == 0)
                        cairo_move_to(cr,
                                      current_x + x * stepSizeX,
                                      current_y - window->magnification * window->displayedData.realp[y * window->lastDataPointDisplayed + x]);
                    else
                        cairo_line_to(cr,
                                      current_x + x * stepSizeX,
                                      current_y - window->magnification * window->displayedData.realp[y * window->lastDataPointDisplayed + x]);
                }
                cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
                cairo_stroke(cr);
            }
        }
    // 2D spectra (contour plot)
    } else if(window->twoDimensionalSpectrum && window->plotMode == Contours) {
        GPtrArray *contours;
        ContourLine *cl;
        // Draw contour plot
        cairo_set_line_width(cr, 0.5);
        contours = g_ptr_array_new();
        compute_contours(window, contours, FALSE);
        for (i = 0; i < contours->len; i++) {
            cl = g_ptr_array_index(contours, i);
            cairo_move_to(cr, cl->x1, cl->y1);
            cairo_line_to(cr, cl->x2, cl->y2);
        }
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_stroke(cr);
        g_ptr_array_free(contours, TRUE);
        contours = g_ptr_array_new();
        compute_contours(window, contours, TRUE);
        for (i = 0; i < contours->len; i++) {
            cl = g_ptr_array_index(contours, i);
            cairo_move_to(cr, cl->x1, cl->y1);
            cairo_line_to(cr, cl->x2, cl->y2);
        }
        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
        cairo_stroke(cr);
        g_ptr_array_free(contours, TRUE);
    // 2D Spectra (rastered plot)
    } else if(window->twoDimensionalSpectrum && window->plotMode == Raster && window->lastDataPointDisplayed > 0) {
        cairo_surface_t *matrix;
        unsigned char *current_row;
        int stride;
        uint32_t a, r, g, b;
        float value;
        float maxValue = 0;

        // Determine maximum peak height
        for (i = 0; i < window->maxDataPoints; i++) {
            if (window->drawRealPart && fabsf(window->data.realp[i]) > maxValue)
                maxValue = fabsf(window->data.realp[i]);
            if (!window->drawRealPart && fabsf(window->data.imagp[i]) > maxValue)
                maxValue = fabsf(window->data.imagp[i]);
        }
        matrix = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                             window->lastDataPointDisplayed,
                                                             window->indirectDataPoints);
        if (cairo_surface_status(matrix) == CAIRO_STATUS_SUCCESS) {
            cairo_surface_flush(matrix);
            current_row = cairo_image_surface_get_data(matrix);
            stride = cairo_image_surface_get_stride(matrix);
            g = 0;
            for (y = 0; y < window->indirectDataPoints; y++) {
                uint32_t *row = (void *)current_row;
                for (x = 0; x < window->lastDataPointDisplayed; x++) {
                    if(window->drawRealPart)
                        value = window->magnification * window->displayedData.realp[y * window->lastDataPointDisplayed + x] / maxValue;
                    else
                        value = window->magnification * window->displayedData.imagp[y * window->lastDataPointDisplayed + x] / maxValue;
                    a = (int)(255 * fabsf(value));
                    if (a > 255)
                        a = 255;
                    if(value > 0.0) {
                        r = 0;
                        b = (int)(255 * value);
                        if (b > 255)
                            b = 255;
                    } else {
                        r = (int)(255 * fabsf(value));
                        b = 0;
                        if (r > 255)
                            r = 255;
                    }
                    row[x] = (a << 24) | (r << 16) | (g << 8) | b;
                }
                current_row += stride;
            }
            cairo_surface_mark_dirty(matrix);
            cairo_save(cr);
            cairo_scale(cr, width / window->lastDataPointDisplayed, height / window->indirectDataPoints);
            cairo_set_source_surface(cr, matrix,
                                     origin_x / width * window->lastDataPointDisplayed,
                                     origin_y / height * window->indirectDataPoints);
            cairo_paint(cr);
            cairo_restore(cr);
        }
    // 1D-Spectra
    } else {
        // Draw coordinate system
        if(window->drawOrdinate) {
            width -= 2.0;
            height -= 2.0;
        }
        stepSizeX = width / (window->maxDataPoints - 1);
        stepSizeY = height / window->maxOrdinateValue * window->magnification;
        stepSizeY *= (window->shiftedBaseline && window->showsFrequencyDomain) ? 2 : 1;
        if(window->abscissaCentered) {
            origin_x += window->drawOrdinate ? 2.0 : 0.0;
            if (window->shiftedBaseline && window->showsFrequencyDomain)
                origin_y += 19 * height / 20;
            else
                origin_y += height / 2;
            stepSizeY /= 2;
            //center_y = origin_y;
        } else {
            origin_x += window->drawOrdinate ? 2.0 : 0.0;
            origin_y += height / 4;
            //center_y = origin_y + height / 4;
        }
        cairo_set_line_width(cr, 3);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        if(window->drawOrdinate) {
            if(window->ordinateCentered) {
                // Pivot point
                if(window->drawPivotPoint && !window->noCursor) {
                    cairo_set_line_width(cr, 2);
                    cairo_move_to(cr, origin_x + (window->maxDataPoints * (0.5 + window->cursorX)) * stepSizeX, 0);
                    cairo_line_to(cr, origin_x + (window->maxDataPoints * (0.5 + window->cursorX)) * stepSizeX, height);
                    cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
                    cairo_stroke(cr);
                    cairo_set_line_width(cr, 3);
                }
            }
        }
        // Draw graph
        if (window->maxDataPoints > 0) {
            // Draw the imaginary graph in green
            if (window->drawImaginaryPart) {
                cairo_set_line_width(cr, window->lineWidth);
                for (i = 0; i < window->lastDataPointDisplayed; i++) {
                    deflection = window->displayedData.imagp[i];
                    if (window->showsFrequencyDomain)
                        deflection -= window->baselineIm;
                    deflection = origin_y - stepSizeY * deflection;
                    if (deflection < gap_top)
                        deflection = gap_top;
                    else if (deflection > gap_top + height)
                        deflection = gap_top + height;
                    if (i == 0)
                        cairo_move_to(cr, origin_x, deflection);
                    else if(lastValueOutOfRange && deflection != gap_top && deflection != gap_top + height) {
                        cairo_move_to(cr, origin_x + (i - 0.5) * stepSizeX, lastValueOutOfRange);
                        cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                    } else if (!lastValueOutOfRange && deflection != gap_top && deflection != gap_top + height)
                        cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                    if (deflection == gap_top || deflection == gap_top + height) {
                        if (!lastValueOutOfRange)
                            cairo_line_to(cr, origin_x + (i - 0.5) * stepSizeX, deflection);
                        else if (lastValueOutOfRange != deflection) {
                            cairo_move_to(cr, origin_x + i * stepSizeX, lastValueOutOfRange);
                            cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                        }
                        lastValueOutOfRange = deflection;
                    } else
                        lastValueOutOfRange = 0.0;
                }
                cairo_set_source_rgba(cr, 0.0, 0.75, 0.0, 1.0);
                cairo_stroke(cr);
            }
            // Draw the real graph in red
            if (window->drawRealPart) {
                lastValueOutOfRange = 0.0;
                cairo_set_line_width(cr, window->lineWidth);
                for (i = 0; i < window->lastDataPointDisplayed; i++) {
                    deflection = window->displayedData.realp[i];
                    if (window->showsFrequencyDomain)
                        deflection -= window->baselineRe;
                    deflection = origin_y - stepSizeY * deflection;
                    if (deflection < gap_top)
                        deflection = gap_top;
                    else if (deflection > gap_top + height)
                        deflection = gap_top + height;
                    if (i == 0)
                        cairo_move_to(cr, origin_x, deflection);
                    else if(lastValueOutOfRange && deflection != gap_top && deflection != gap_top + height) {
                        cairo_move_to(cr, origin_x + (i - 0.5) * stepSizeX, lastValueOutOfRange);
                        cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                    } else if (!lastValueOutOfRange && deflection != gap_top && deflection != gap_top + height)
                        cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                    if (deflection == gap_top || deflection == gap_top + height) {
                        if (!lastValueOutOfRange)
                            cairo_line_to(cr, origin_x + (i - 0.5) * stepSizeX, deflection);
                        else if (lastValueOutOfRange != deflection) {
                            cairo_move_to(cr, origin_x + i * stepSizeX, lastValueOutOfRange);
                            cairo_line_to(cr, origin_x + i * stepSizeX, deflection);
                        }
                        lastValueOutOfRange = deflection;
                    } else
                        lastValueOutOfRange = 0.0;
                }
                cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
                cairo_stroke(cr);
            }
            // Draw integral
            if(window->drawIntegral && window->showsFrequencyDomain) {
                if(!(window->integral[i] != window->integral[i])) {
                    cairo_set_line_width(cr, window->lineWidth);
                    stepSizeY = 1.8 * height / window->maxDataPoints;
                    if (window->shiftedBaseline)
                        stepSizeY *= 2;
                    cairo_move_to(cr, origin_x, origin_y - stepSizeY * window->integral[0]);
                    for(i = 1; i < window->lastDataPointDisplayed; i++)
                        cairo_line_to(cr, origin_x + i * stepSizeX, origin_y - stepSizeY * window->integral[i]);
                    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
                    cairo_stroke(cr);
                }
            }
            // Draw window function
            if(window->drawWindow && !window->showsFrequencyDomain && window->windowFunctionType != WFNone) {
                float value, initial = height / 2;
                switch(window->windowFunctionType) {
                case WFWeightedHann:
                    initial /= 16.8;
                    break;
                case WFTraficante:
                    initial /= 2.75;
                    break;
                case WFTraficanteSN:
                    initial /= 1.34;
                    break;
                default:
                    break;
                }
                cairo_set_line_width(cr, window->lineWidth);
                stepSizeY = 1.8 * height / window->maxDataPoints;
                switch(window->windowFunctionType) {
                case WFLorentzGaussTransformation:
                case WFGaussPseudoEchoTransformation:
                    //value = lorentz_gauss_transformation(0, window->maxDataPoints, 10.0, 10.0 * 2, 0.5);
                    value = exp(-pow(0 - window->gaussianShift * window->maxDataPoints, 2) / (2 * pow(5 * window->gaussianWidth, 2)));
                    break;
                default:
                    value = window_function(window->windowFunctionType, 0, window->maxDataPoints);
                }
                cairo_move_to(cr, origin_x, origin_y - initial * value);
                for(i = 1; i < window->lastDataPointDisplayed; i++) {
                    switch(window->windowFunctionType) {
                    case WFLorentzGaussTransformation:
                    case WFGaussPseudoEchoTransformation:
                        //value = lorentz_gauss_transformation(i, window->maxDataPoints, 10.0, 10.0 * 2, 0.5);
                        value = exp(-pow(i - window->gaussianShift * window->maxDataPoints, 2) / (2 * pow(5 * window->gaussianWidth, 2)));
                        break;
                    default:
                        value = window_function(window->windowFunctionType, i, window->maxDataPoints);
                    }
                    cairo_line_to(cr, origin_x + i * stepSizeX, origin_y - initial * value);
                }
                cairo_set_source_rgba(cr, 1.0, 0.0, 1.0, 1.0);
                cairo_stroke(cr);
            }
        }
        // Draw automatically picked peaks
        if (window->numberOfPeaks > 0) {
            cairo_set_line_width(cr, 1.0);
            y = gap_top;
            for (i = 0; i < window->numberOfPeaks; i++) {
                peak = g_ptr_array_index(window->peaks, i);
                pos = *peak * stepSizeX;
                if(pos < 0) {
                    pos *= -1;
                    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
                } else {
                    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
                }
                pos += gap_left;
                cairo_move_to(cr, pos, 0.0);
                cairo_line_to(cr, pos, y);
                cairo_stroke(cr);
            }
        }
    }
    // Draw cursor
    if ((window->drawCursor || window->drawPivotPoint) && window->domainOf2DSpectrum > 0
        && !(window->plotMode == Stacked && window->twoDimensionalSpectrum)
        /*&& window->cursorX > gap_left && window->cursorX < gap_left + width
        && window->cursorY > gap_top && window->cursorY < height + gap_top*/) {
        frequencyLabel = malloc(40 * sizeof(gchar));
        cairo_set_line_width(cr, 0.5);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        if (window->drawPivotPoint) {
            cairo_move_to(cr, (window->pivotPoint + 0.5) * width + gap_left, gap_top);
            cairo_line_to(cr, (window->pivotPoint + 0.5) * width + gap_left, gap_top + height);
            freq2 = window->pivotPoint / insensitive_settings_get_dwellTime(window->controller->settings);
        } else {
            cairo_move_to(cr, window->cursorX, gap_top);
            cairo_line_to(cr, window->cursorX, gap_top + height);
            freq2 = ((window->cursorX - gap_left) / width - 0.5) / insensitive_settings_get_dwellTime(window->controller->settings);
        }
        if (window->twoDimensionalSpectrum && window->domainOf2DSpectrum >= 2 && !window->drawPivotPoint) {
            cairo_move_to(cr, gap_left, window->cursorY);
            cairo_line_to(cr, gap_left + width, window->cursorY);
            freq1 = ((window->cursorY - gap_top) / height - 0.5) / insensitive_settings_get_dwellTime(window->controller->settings);
            sprintf(frequencyLabel, "F₁ = %.2f Hz / F₂ = %.2f Hz", freq1, freq2);
        } else {
            if (window->drawPivotPoint)
                sprintf(frequencyLabel, "Pivot point = %.2f Hz", freq2);
            else
                sprintf(frequencyLabel, "Frequency = %.2f Hz", freq2);
        }
        cairo_stroke(cr);
        cairo_select_font_face(cr, "default", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14);
        cairo_move_to(cr, origin_x, gap_top - 4.0);
        cairo_show_text(cr, frequencyLabel);
        free(frequencyLabel);
    }
}


float contour_height(InsensitiveWindow *window, int x, int y, int maxX, int maxY, gboolean negativeValues)
{
    unsigned int xx, yy;
    float h;

    xx = (int)(((float)x / (float)maxX) * (float)window->lastDataPointDisplayed);
	yy = window->indirectDataPoints - (int)((((float)y + 1) / (float)maxY) * (float)window->indirectDataPoints);

    if(xx >= window->lastDataPointDisplayed || yy >= window->indirectDataPoints /*|| xx < 0 || yy < 0*/)
        h = 0;
    else {
        if(window->drawRealPart)
            h = window->magnification * window->displayedData.realp[yy * window->lastDataPointDisplayed + xx];
        else
            h = window->magnification * window->displayedData.imagp[yy * window->lastDataPointDisplayed + xx];
    }

    if(negativeValues)
        return contourBase - (h * countOfContours / 10.0);
    else
        return contourBase + (h * countOfContours / 10.0);
}


float contour_x(InsensitiveWindow *window, int x)
{
	/*GtkAllocation* alloc = g_new(GtkAllocation, 1);
    float minX;

    gtk_widget_get_allocation(GTK_WIDGET(window->spectrum_drawingarea), alloc);
    minX = alloc->width;
    g_free(alloc);*/

	return x * contourCellSize + GAP_LEFT;
}


float contour_y(InsensitiveWindow *window, int y)
{
	GtkAllocation* alloc = g_new(GtkAllocation, 1);
    float minY;

    gtk_widget_get_allocation(GTK_WIDGET(window->spectrum_drawingarea), alloc);
    minY = alloc->height;
    g_free(alloc);

	return -y * contourCellSize + minY - GAP_BOTTOM;
}


float contour_for_index(int index)
{
    float findex = (float)index;

	return (contourInterval * findex) - contourBase;
}


void compute_contours(InsensitiveWindow *window, GPtrArray *contours, gboolean negativeValues)
{
#define xsect(p1,p2) (h[p2]*xh[p1]-h[p1]*xh[p2])/(h[p2]-h[p1])
#define ysect(p1,p2) (h[p2]*yh[p1]-h[p1]*yh[p2])/(h[p2]-h[p1])

    int	ilb, jlb, iub, jub, nc;
	int	m1, m2, m3, case_value;
	float dmin, dmax;
	int	i, j, k, m;
	float h[5];
	int	sh[5];
	float xh[5], yh[5];
	int	im[4] = {0,1,1,0}, jm[4]={0,0,1,1};
	int	castab[3][3][3] = {
        { {0,0,8},{0,2,5},{7,6,9} },
        { {0,3,4},{1,3,1},{4,3,0} },
        { {9,6,7},{5,2,0},{8,0,0} }
    };

	float temp1, temp2, temp3, temp4, zmin, zmax, d1, d2, z;
    ContourLine *cl;
    GtkAllocation* alloc = g_new(GtkAllocation, 1);

    gtk_widget_get_allocation(GTK_WIDGET(window->spectrum_drawingarea), alloc);
    // min extent of DEM cells horizontally
	ilb = 0;
    // min extent of DEM cells vertically
	jlb = 0;
    // max extent of DEM cells horizontally
    iub = alloc->width - GAP_LEFT;
    iub -= window->twoDimensionalSpectrum ? 58.0 : 27.0;
    iub /= contourCellSize;
    // max extent of DEM cells vertically
    jub = (alloc->height - GAP_TOP - GAP_BOTTOM) / contourCellSize;
    g_free(alloc);

	nc = countOfContours;			        // number of contours to plot
	zmin = contour_for_index(0);		    // minimum height
	zmax = contour_for_index(nc - 1);	    // maximum height

	for (j = jub - 1; j >= jlb; j--) {
		for (i = ilb; i <= iub - 1; i++) {
			// Get the min and max of heights to be found in this DEM cell
            d1 = contour_height(window, i, j, iub, jub, negativeValues);
			d2 = contour_height(window, i, j + 1, iub, jub, negativeValues);
			temp1 = MIN(d1, d2);
			temp3 = MAX(d1, d2);

			d1 = contour_height(window, i + 1, j, iub, jub, negativeValues);
			d2 = contour_height(window, i + 1, j + 1, iub, jub, negativeValues);
			temp2 = MIN(d1, d2);
			temp4 = MAX(d1, d2);

			dmin = MIN(temp1, temp2);
			dmax = MAX(temp3, temp4);

            // If outside the overall contour range of interest, ignore the cell
			if (dmax < zmin || dmin > zmax)
				continue;

			for (k = 0; k < nc; k++) {
				// Get the contour height value corresponding to this index
                z = contour_for_index(k);

				// If the height doesn't intersect this DEM cell's height range, nothing to do
				if (z < dmin || z > dmax)
					continue;

				for (m = 4; m >= 0; m--) {
					if(m > 0) {
						h[m] = contour_height(window, i + im[m - 1], j + jm[m - 1], iub, jub, negativeValues) - z;
						xh[m] = contour_x(window, i + im[m - 1]);
						yh[m] = contour_y(window, j + jm[m - 1]);
					} else {
						h[0]  = 0.25 * (h[1] + h[2] + h[3] + h[4]);
						xh[0] = 0.50 * (contour_x(window, i) + contour_x(window, i + 1));
						yh[0] = 0.50 * (contour_y(window, j) + contour_y(window, j + 1));
					}

					if (h[m] > 0.0)
						sh[m] = 1;
					else if (h[m] < 0.0)
						sh[m] = -1;
					else
						sh[m] = 0;
				}

				/*
                 Note: at this stage the relative heights of the corners and the
                 centre are in the h array, and the corresponding coordinates are
                 in the xh and yh arrays. The centre of the box is indexed by 0
                 and the 4 corners by 1 to 4 as shown below.
                 Each triangle is then indexed by the parameter m, and the 3
                 vertices of each triangle are indexed by parameters m1,m2,and m3.
                 It is assumed that the centre of the box is always vertex 2
                 though this is important only when all 3 vertices lie exactly on
                 the same contour level, in which case only the side of the box
                 is drawn.
                 vertex 4 +-------------------+ vertex 3
                 | \               / |
                 |   \    m=3    /   |
                 |     \       /     |
                 |       \   /       |
                 |  m=2    X   m=2   |       the centre is vertex 0
                 |       /   \       |
                 |     /       \     |
                 |   /    m=1    \   |
                 | /               \ |
                 vertex 1 +-------------------+ vertex 2
                 */
				/* Scan each triangle in the box */

				for (m = 1; m <= 4; m++) {
					m1 = m;
					m2 = 0;
					if (m != 4)
						m3 = m + 1;
					else
						m3 = 1;

					if ((case_value = castab[sh[m1] + 1][sh[m2] + 1][sh[m3] + 1]) == 0)
						continue;

                    cl = malloc(sizeof(ContourLine));
                    cl->index = k;
					switch (case_value) {
					case 1: // Line between vertices 1 and 2
                        cl->x1 = xh[m1];
                        cl->y1 = yh[m1];
                        cl->x2 = xh[m2];
                        cl->y2 = yh[m2];
                        break;
					case 2: // Line between vertices 2 and 3
                        cl->x1 = xh[m2];
                        cl->y1 = yh[m2];
                        cl->x2 = xh[m3];
                        cl->y2 = yh[m3];
                        break;
					case 3: // Line between vertices 3 and 1
                        cl->x1 = xh[m3];
                        cl->y1 = yh[m3];
                        cl->x2 = xh[m1];
                        cl->y2 = yh[m1];
                        break;
					case 4: // Line between vertex 1 and side 2-3
                        cl->x1 = xh[m1];
                        cl->y1 = yh[m1];
                        cl->x2 = xsect(m2,m3);
                        cl->y2 = ysect(m2,m3);
                        break;
					case 5: // Line between vertex 2 and side 3-1
                        cl->x1 = xh[m2];
                        cl->y1 = yh[m2];
                        cl->x2 = xsect(m3,m1);
                        cl->y2 = ysect(m3,m1);
                        break;
					case 6: // Line between vertex 3 and side 1-2
                        cl->x1 = xh[m3];
                        cl->y1 = yh[m3];
                        cl->x2 = xsect(m3,m2);
                        cl->y2 = ysect(m3,m2);
                        break;
					case 7: // Line between sides 1-2 and 2-3
                        cl->x1 = xsect(m1,m2);
                        cl->y1 = ysect(m1,m2);
                        cl->x2 = xsect(m2,m3);
                        cl->y2 = ysect(m2,m3);
                        break;
					case 8: // Line between sides 2-3 and 3-1
                        cl->x1 = xsect(m2,m3);
                        cl->y1 = ysect(m2,m3);
                        cl->x2 = xsect(m3,m1);
                        cl->y2 = ysect(m3,m1);
                        break;
					case 9: // Line between sides 3-1 and 1-2
                        cl->x1 = xsect(m3,m1);
                        cl->y1 = ysect(m3,m1);
                        cl->x2 = xsect(m1,m2);
                        cl->y2 = ysect(m1,m2);
                        break;
                    default:
                        cl->x1 = 0.0;
                        cl->y1 = 0.0;
                        cl->x2 = 0.0;
                        cl->y2 = 0.0;
                        break;
				    }

					// Finally output the line
                    g_ptr_array_add(contours, cl);
				}
			}
		}
	}
}



G_MODULE_EXPORT gboolean on_spectrum_drawingarea_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;
    double delta_x, delta_y;

    if(window->allowResizing && (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN || event->direction == GDK_SCROLL_SMOOTH))
        if (gdk_event_get_scroll_deltas((GdkEvent *)event, &delta_x, &delta_y)) {
            window->magnification += 0.1 * event->delta_y;
            if (window->magnification < 0.01)
                window->magnification = 0.01;
            gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
        }
    return FALSE;
}


G_MODULE_EXPORT void on_spectrum_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->drawCursor = TRUE;
    window->cursorX = event->x;
    window->cursorY = event->y;
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


G_MODULE_EXPORT void on_spectrum_drawingarea_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->drawCursor = FALSE;
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}


G_MODULE_EXPORT void on_spectrum_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    InsensitiveWindow *window = (InsensitiveWindow *)user_data;

    window->cursorX = event->x;
    window->cursorY = event->y;
    gtk_widget_queue_draw((GtkWidget *)window->spectrum_drawingarea);
}
