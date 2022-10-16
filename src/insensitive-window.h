/* insensitive-window.h
 *
 * Copyright 2021 Klaus Boldt
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

#pragma once

#include <gtk/gtk.h>

#include "insensitive.h"
#include "insensitive-controller.h"


struct _InsensitiveWindow {
	GtkApplicationWindow parent_instance;

	/* Template widgets */
	GtkHeaderBar        *header_bar;
	GtkEntry            *command_line;
	GtkToolButton       *spinsystem_toolbutton, *spinstate_toolbutton, *pulsesequence_toolbutton, *spectrum_toolbutton;
	GtkNotebook         *mainwindow_notebook;

    GtkSpinButton       *spinNumber_spinbutton;
	GtkAdjustment       *spinNumber_adjustment;
    GtkEntry            *chemicalShift_entry;
    GtkLabel            *chemicalShift_unit_label;
    GtkComboBoxText     *chemicalShift_units_combobox;
    GtkComboBoxText     *displayedConstant_combobox;
    GtkEntry            *scalarConstant_entry, *dipolarConstant_entry, *distanceConstant_entry;
    GtkButton           *addSpin_button, *removeSpin_button, *reset_constants_button, *rotate_button;
    GtkComboBoxText     *gyroI_combobox, *gyroS_combobox;
    GtkToggleButton     *strong_coupling_checkbox_spinsystem;
    GtkDrawingArea      *spinEditor_drawingarea;
    GtkDrawingArea      *energyLevel_drawingarea;

	GtkTextBuffer       *productoperator_textbuffer;
	GtkButton           *pulse_button;
	GtkButton           *chemicalShift_button;
	GtkButton           *coupling_button;
	GtkButton           *relaxation_button;
	GtkButton           *freeEvolution_button;
	GtkButton           *gradient_button;
	GtkButton           *acquire_button;
	GtkButton           *equilibrium_button;
	GtkButton           *undo_button;
	GtkSpinner          *acquisition_spinner;
	GtkDrawingArea      *iSpinVector_drawingarea, *sSpinVector_drawingarea, *matrix_drawingarea;

	GtkComboBoxText     *pulseEnvelope_combobox;
	GtkEntry            *flipAngle_entry;
	GtkAdjustment       *flipAngle_adjustment;
    GtkScale            *flipAngle_scale, *phase_scale, *pulseFrequency_scale;
	GtkEntry            *pulseDuration_entry;
	GtkEntry            *pulseStrength_entry;
	GtkButton           *hardpulse_button;
	GtkButton           *softpulse_button;
	GtkButton           *softerpulse_button;
	GtkButton           *selectivepulse_button;
	GtkEntry            *pulseFrequency_entry;
	GtkAdjustment       *pulseFrequency_adjustment;
	GtkEntry            *phase_entry;
	GtkAdjustment       *phase_adjustment;
	GtkToggleButton     *spin1_checkbox, *spin2_checkbox, *spin3_checkbox, *spin4_checkbox;
	GtkToggleButton     *ispins_checkbox, *sspins_checkbox, *allspins_checkbox;
	GtkToggleButton     **spin_checkbox_array;
	GtkToggleButton     *strong_coupling_checkbox_spinstate;
	GtkToggleButton     *dipolar_relaxation_checkbox;
	GtkToggleButton     *animation_checkbox;
	GtkToggleButton     *include_relaxation_checkbox;
	GtkEntry            *T1_entry, *T2_entry;
	GtkEntry            *correlationTime_entry;
	GtkEntry            *delay_entry;
	GtkComboBoxText     *delay_combobox;
	GtkToggleButton     *dephasingJitter_checkbox;
	GtkToggleButton     *iDecoupling_checkbox, *sDecoupling_checkbox;
	GtkToggleButton     *spinlock_checkbox;
	GtkEntry            *gradient_strength_entry, *gradient_duration_entry;
	GtkComboBoxText     *gradient_strength_combobox;
	GtkToggleButton     *diffusion_checkbox;
	GtkSpinButton       *datapoints_spinbutton;
	GtkEntry            *dwelltime_entry;
	GtkAdjustment       *datapoints_adjustment;
	GtkEntry            *noiseLevel_entry;
	GtkToggleButton     *acquisitionAfterNextPulse_checkbox;
	GtkRadioButton      *detectISignal_radiobutton, *detectSSignal_radiobutton;
	GtkToggleButton     *zeroFilling_checkbox;
	GtkComboBoxText     *vectorDisplayType_combobox;
	GtkComboBoxText     *matrixDisplayType_combobox;
	GtkComboBoxText     *operatorBasis_combobox;
	GtkComboBoxText     *vectorDiagramType_combobox;
	GtkToggleButton     *color1stOrderCoherences_checkbox;

    InsensitivePulseShaper *pulse_shaper_window;
    InsensitiveComposer *matrix_composer_window;

    cairo_surface_t     *pulseSequence_surface;
    GtkDrawingArea      *pulseSequence_drawingarea, *pulseSequenceStep_drawingarea;
    GtkNotebook         *bottomDisplay_notebook;
    GtkComboBoxText     *detectionMethod_combobox, *bottomDisplay_combobox;
    GtkComboBoxText     *phaseCycles_combobox, *evolutionTimes_combobox;
    GtkEntry            *phaseCycles_entry;
    GtkButton           *record_button, *play_button, *step_button;
    GtkTreeView         *phaseCycling_treeview;
    GtkTreeModel        *phaseCycling_treemodel;
    GtkListStore        *phaseCycling_liststore;
    GtkDrawingArea      *coherencePathway_drawingarea;
    GtkTextBuffer       *pulseProgram_textbuffer;
    GtkNotebook         *pp_edit_notebook;
    GtkComboBoxText     *pp_edit_pulse_combobox, *pp_edit_pulse_shape_combobox;
    GtkEntry            *pp_edit_pulse_entry;
    GtkToggleButton     *pp_edit_pulse_idecoupling_checkbox, *pp_edit_pulse_sdecoupling_checkbox;
    GtkButton           *pp_edit_pulse_ok_button;
    GtkComboBoxText     *pp_edit_delay_combobox;
    GtkEntry            *pp_edit_delay_entry;
    GtkToggleButton     *pp_edit_delay_idecoupling_checkbox, *pp_edit_delay_sdecoupling_checkbox, *pp_edit_delay_spinlock_checkbox;
    GtkButton           *pp_edit_delay_ok_button;
    GtkComboBoxText     *pp_edit_gradient_combobox;
    GtkEntry            *pp_edit_gradient_entry;
    GtkButton           *pp_edit_gradient_ok_button;
    GtkComboBoxText     *pp_edit_fid_combobox;
    GtkToggleButton     *pp_edit_fid_idetect_radiobutton, *pp_edit_fid_sdetect_radiobutton;
    GtkToggleButton     *pp_edit_fid_idecoupling_checkbox, *pp_edit_fid_sdecoupling_checkbox, *pp_edit_fid_spinlock_checkbox;
    GtkButton           *pp_edit_fid_ok_button;
    GtkWindow           *step_window;
    GtkButton           *stepWindow_button;

    cairo_surface_t     *spectrum_surface;
    GtkDrawingArea      *spectrum_drawingarea;
    GtkStack            *spectrum_tools_stack;
    GtkWidget           *fourier_stack_child;
    GtkProgressBar      *spectrum_progressbar;
    GtkButton           *fid_button, *fft1D_button, *fft2D_button, *magnitude_button;
    GtkLabel            *dataPoints_label;
    GtkLabel            *noiseLevelSpectrum_label, *plotStyle_label, *dataSet_label, *lineWidth_label;
    GtkEntry            *noiseLevelSpectrum_entry, *signalToNoiseThreshold_entry;
    GtkComboBox         *apodization_combobox;
    GtkLabel            *gaussianWidth_label, *gaussianWidthUnit_label, *gaussianShift_label;
    GtkEntry            *gaussianWidth_entry;
    GtkScale            *gaussianShift_slider;
    GtkAdjustment       *gaussianShift_adjustment;
    GtkScale            *zeroOrder_slider, *firstOrder_slider, *pivotPoint_slider;
    GtkAdjustment       *zeroOrder_adjustment, *firstOrder_adjustment, *pivotPoint_adjustment;
    GtkToggleButton     *showReal_checkbox, *showImaginary_checkbox;
    GtkToggleButton     *integral_checkbox, *grid_checkbox, *window_checkbox, *shiftBaseline_checkbox;
    GtkComboBox         *plotStyle_combobox, *lineWidth_combobox;
    GtkEntry            *lineWidth_entry;
    GtkMenuButton       *tilt_menubutton, *symmetrize_menubutton;
    GtkToggleButton     *absDispReal_radiobutton, *absDispImag_radiobutton;
    GtkTextView         *spectrumParameters_textview;
    GtkTextBuffer       *spectrumParameters_textbuffer;
    GtkButton           *dosyToolbox_button, *toggleParameters_button;
    GtkWindow           *dosyToolBox_window;
    GtkButton           *dosyFirstTrace_button, *dosyPickPeaks_button, *dosyFitLorentzian_button;
    GtkButton           *dosyFitExponential_button, *dosyPlotSpectrum_button;

	InsensitiveController *controller;
    GPtrArray *commandHistory;
    unsigned int commandHistoryPosition;

    // Spin Editor and Energy Level Diagram
    enum SpinEditorConstant displayedConstant;
    cairo_surface_t *ispin_image, *ispin_selected_image, *sspin_image, *sspin_selected_image;
    gboolean spinEditor_drawLine;
    float spinEditor_linkSource_x, spinEditor_linkSource_y, spinEditor_linkTarget_x, spinEditor_linkTarget_y;
    unsigned int numberOfLevels;
    int *transition;
    float *energyLevel, *energyLevelOrig, *transitionProbability;
    GPtrArray *levelNames, *levelNamesOrig;

    // Pulse Sequence and Coherence Transfer Pathway
    gboolean needsToRecalculateCoherencePathways;
    float I_line, S_line, G_line;
    int variableEvolutionTime;
    unsigned int nextStepInPulseSequence;
    gboolean userInteractionEnabled, acquisitionIsBeingPerformed;
    SequenceElement *editedElement, *secondHalfOfSandwichElement;
    int numberOfISpinOrders, **pathwaysForISpins;
    int numberOfSSpinOrders, **pathwaysForSSpins;
    long numberOfISpinPathways, numberOfSSpinPathways;
    int numberOfPulses, errorCode;
    float pathway_scaling;
    float *coefficientsForISpins, maxCoefficientForISpins;
    float *coefficientsForSSpins, maxCoefficientForSSpins;
    GPtrArray *labelArray, *pathArray, *alphaArray;
    gboolean showOnlyNCoherences;
    GtkTextTag *keywordTag, *commentTag, *numberTag, *stringTag, *variableTag, *preprocessorTag, *filenameTag;

    // Spectrum
    int domainOf2DSpectrum, statesDataSet, coherencePathwayType;
    gboolean twoDimensionalSpectrum, drawPivotPoint, drawCursor, parametersVisible;
    enum OpenFileState openFileState_for_spinSystem, openFileState_for_pulseSequence, openFileState_for_spectrum;
    float zeroOrderPhaseShift, firstOrderPhaseShift, pivotPoint;
    float parameterScrollbarShift;
    unsigned int maxDataPoints, lastDataPointDisplayed, indirectDataPoints, numberOfPeaks;
    float cursorX, cursorY, scaling, lineWidth, baselineRe, baselineIm;
    float maxOrdinateValue, magnification, *integral, integralMaximum, *apodizationT2, *apodizationT1;
    gboolean abscissaCentered, ordinateCentered, autoMaximize, allowResizing, hideYCursor;
    gboolean showsFrequencyDomain, shows2DFrequencyDomain;
    gboolean noCursor, shiftedBaseline;
    enum PlotMode plotMode;
    gboolean drawRealPart, drawImaginaryPart, drawIntegral, drawAbscissa, drawOrdinate, drawScale, drawWindow;
    DSPSplitComplex data, phase, noise, displayedData;
    enum WindowFunctionType windowFunctionType;
    float gaussianWidth, gaussianShift;
    double spectrumProgressBarMaximum;
    guint removePivotPointTimerNr;
    gboolean spectrumIsDOSY2D;
    GPtrArray *peaks;
};


G_BEGIN_DECLS

#define INSENSITIVE_TYPE_WINDOW (insensitive_window_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveWindow, insensitive_window, INSENSITIVE, WINDOW, GtkApplicationWindow)

G_END_DECLS

void on_open_file_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void open_file(GtkWidget *chooser, InsensitiveWindow *window);
void show_open_file_error(InsensitiveWindow *window, gchar *filename);
void on_preferences_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void quit_insensitive(InsensitiveWindow *window);

/* Output Actions */
void show_mainWindow_notebook_page(InsensitiveWindow *self, unsigned int page);
void init_settings(InsensitiveWindow *self, InsensitiveSettings *settings);
void set_selectable_delay_times(InsensitiveWindow *self, gchar **names, unsigned int size);
void set_user_controls_enabled(InsensitiveWindow *self, gboolean value);
void enable_animation_checkbox(InsensitiveWindow *self, gboolean value);
void set_acquisition_is_running(InsensitiveWindow *self, gboolean value);
void enable_acquisition_button(InsensitiveWindow *self, gboolean value);
void start_progress_indicator(InsensitiveWindow *self);
void stop_progress_indicator(InsensitiveWindow *self);
void spin_state_was_changed(InsensitiveWindow *window);
void spin_number_was_changed(InsensitiveWindow *window);

/* Input Actions */
void on_equilibrium_button_clicked(GtkButton *button, gpointer user_data);
void on_pulse_button_clicked(GtkButton *button, gpointer user_data);
void on_chemicalShift_button_clicked(GtkButton *button, gpointer user_data);
void on_coupling_button_clicked(GtkButton *button, gpointer user_data);
void on_relaxation_button_clicked(GtkButton *button, gpointer user_data);
void on_freeEvolution_button_clicked(GtkButton *button, gpointer user_data);
void on_gradient_button_clicked(GtkButton *button, gpointer user_data);
void on_acquire_button_clicked(GtkButton *button, gpointer user_data);
void on_undo_button_clicked(GtkButton *button, gpointer user_data);
void on_notebook_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_matrix_composer_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_pulse_shaper_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_single_spins_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_tutorial_toolbutton_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_about_menu_item_activate(GtkMenuItem *item, gpointer *user_data);

/* Settings Getters and Setters */
void set_pulseEnvelope(InsensitiveWindow *self, enum PulseEnvelope value);
void on_pulseEnvelope_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void set_flipAngle(InsensitiveWindow *window, float value);
void on_flipAngle_adjustment_changed(GtkAdjustment *slider, gpointer user_data);
void on_flipAngle_entry_activate(GtkEntry *entry, gpointer user_data);
void set_pulseDuration(InsensitiveWindow *window, float value);
void on_pulseDuration_entry_activate(GtkEntry *entry, gpointer user_data);
void set_pulseStrength(InsensitiveWindow *window, float value);
void on_pulseStrength_entry_activate(GtkEntry *entry, gpointer user_data);
void on_hardpulse_button_clicked(GtkButton *button, gpointer user_data);
void on_softpulse_button_clicked(GtkButton *button, gpointer user_data);
void on_softerpulse_button_clicked(GtkButton *button, gpointer user_data);
void on_selectivepulse_button_clicked(GtkButton *button, gpointer user_data);
void set_pulseFrequency(InsensitiveWindow *window, float value);
void on_pulseFrequency_adjustment_changed(GtkAdjustment *slider, gpointer user_data);
void on_pulseFrequency_entry_activate(GtkEntry *entry, gpointer user_data);
void set_phase(InsensitiveWindow *window, float value);
void on_phase_adjustment_changed(GtkAdjustment *slider, gpointer user_data);
void on_phase_entry_activate(GtkEntry *entry, gpointer user_data);
void set_spin_checkboxes(InsensitiveWindow *window, int pulseArray);
void on_spin_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_iSpins_checkbox(InsensitiveWindow *window, gboolean value);
void on_iSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_sSpins_checkbox(InsensitiveWindow *window, gboolean value);
void on_sSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_allSpins_checkbox(InsensitiveWindow *window, gboolean value);
void on_allSpins_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_strongCoupling_checkbox(InsensitiveWindow *window, gboolean value);
void on_strongCoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_dipolarRelaxation_checkbox(InsensitiveWindow *window, gboolean value);
void on_dipolar_relaxation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_animation_checkbox(InsensitiveWindow *window, gboolean value);
void on_animation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_include_relaxation_checkbox(InsensitiveWindow *window, gboolean value);
void on_include_relaxation_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_T1(InsensitiveWindow *window, float value);
void on_T1_entry_activate(GtkEntry *entry, gpointer user_data);
void set_T2(InsensitiveWindow *window, float value);
void on_T2_entry_activate(GtkEntry *entry, gpointer user_data);
void set_correlationTime(InsensitiveWindow *window, float value);
void on_correlationTime_entry_activate(GtkEntry *entry, gpointer user_data);
void set_delay(InsensitiveWindow *window, float value);
void on_delay_entry_activate(GtkEntry *entry, gpointer user_data);
void on_delay_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void on_correlationTime_entry_activate(GtkEntry *entry, gpointer user_data);
void set_dephasingJitter_checkbox(InsensitiveWindow *window, gboolean value);
void on_dephasingJitter_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_iDecoupling_checkbox(InsensitiveWindow *window, gboolean value);
void on_iDecoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_sDecoupling_checkbox(InsensitiveWindow *window, gboolean value);
void on_sDecoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_spinlock(InsensitiveWindow *window, gboolean value);
void on_spinlock_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_gradient_strength(InsensitiveWindow *window, float value);
void on_gradient_strength_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void on_gradient_strength_entry_activate(GtkEntry *entry, gpointer user_data);
void set_gradient_duration(InsensitiveWindow *window, float value);
void on_gradient_duration_entry_activate(GtkEntry *entry, gpointer user_data);
void set_diffusion(InsensitiveWindow *window, gboolean value);
void on_diffusion_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_dataPoints(InsensitiveWindow *window, int value);
void on_datapoints_spinbutton_change_value(GtkSpinButton *spinbutton, gpointer user_data);
void on_datapoints_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
void set_dwellTime(InsensitiveWindow *window, float value);
void on_dwelltime_entry_activate(GtkEntry *entry, gpointer user_data);
void set_noiseLevel(InsensitiveWindow *window, float value);
void on_noise_level_entry_activate(GtkEntry *entry, gpointer user_data);
void set_acquisitionAfterNextPulse(InsensitiveWindow *window, gboolean value);
void disable_acquireAfterNextPulse(InsensitiveWindow *window);
void on_acquisitionAfterNextPulse_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_pulseBeforeAcquisition(InsensitiveWindow *window, gboolean value);
void disable_pulseBeforeAcquisition(InsensitiveWindow *window);
void on_pulseBeforeAcquisition_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_detectISignal(InsensitiveWindow *window, gboolean value);
void on_detectISignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data);
void set_detectSSignal(InsensitiveWindow *window, gboolean value);
void on_detectSSignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data);
void detectSignal_radiobutton_group_changed(GtkRadioButton *radiobutton, gpointer user_data);
void set_zeroFilling(InsensitiveWindow *window, gboolean value);
void on_zeroFilling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_vectorDisplayType(InsensitiveWindow *self, enum VectorDisplayType value);
void on_vectorDisplayType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void set_operatorBasis(InsensitiveWindow *self, enum OperatorBasis value);
void on_operatorBasis_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void set_color1stOrderCoherences(InsensitiveWindow *window, gboolean value);
void on_color1stOrderCoherences_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_matrixDisplayType(InsensitiveWindow  *window, enum MatrixDisplayType value);
void on_matrixDisplayType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void set_vectorDiagramType(InsensitiveWindow  *window, enum VectorDiagramType value);
void on_vectorDiagramType_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);

/* Spin Editor */
gboolean perform_open_spinSystem(InsensitiveWindow *window, xmlNodePtr node);
void perform_save_spinSystem(GtkMenuItem *menuitem, gpointer user_data);
void set_openedFileState_for_spinSystem(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename);
void spin_was_selected(InsensitiveWindow *window, unsigned int spin);
void set_spin_type(InsensitiveWindow *window, unsigned int spin);
void on_chemicalShift_entry_activate(GtkEntry *entry, gpointer user_data);
void on_removeSpin_button_clicked(GtkButton *button, gpointer user_data);
void on_scalarConstant_entry_activate(GtkEntry *entry, gpointer user_data);
void on_dipolarConstant_entry_activate(GtkEntry *entry, gpointer user_data);
void on_distanceConstant_entry_activate(GtkEntry *entry, gpointer user_data);
void set_larmorFrequency(InsensitiveWindow *window, float value);
void set_scalarConstant(InsensitiveWindow *window, float value);
void set_dipolarConstant(InsensitiveWindow *window, float value);
void set_distanceConstant(InsensitiveWindow *window, float value);
void on_addSpin_button_clicked(GtkButton *button, gpointer user_data);
void on_removeSpin_button_clicked(GtkButton *button, gpointer user_data);
void on_spinNumber_stepper_value_changed(GtkAdjustment *slider, gpointer user_data);
void on_spinNumber_stepper_activate(GtkEntry *entry, gpointer user_data);
void set_spin_number(InsensitiveWindow *window, unsigned int number);
void on_reset_constants_button_clicked(GtkButton *button, gpointer user_data);
void on_chemicalShift_units_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void set_chemicalShift_units_to_degreesPerSecond(InsensitiveWindow *window, gboolean value);
void on_displayedConstant_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_rotate_button_clicked(GtkButton *button, gpointer user_data);
void on_gyro_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void set_gyromagneticRatio_comboboxes(InsensitiveWindow *window, unsigned int codeForI, unsigned int codeForS);

/* Pulse Sequence */
void set_recording_button_clicked(InsensitiveWindow *window, gboolean value);
void set_variable_evolution_time(InsensitiveWindow *window, int value);
void allow_spectrum_acquisition(InsensitiveWindow *window, gboolean value);
void enable_pulseSequence_play_button(InsensitiveWindow *window, gboolean value);
void set_acquisition_in_background(InsensitiveWindow *window, gboolean value);
void set_phaseCycling_combobox(InsensitiveWindow *window, int value);
void set_detectionMethod(InsensitiveWindow *window, enum PurePhaseDetectionMethod value);
void set_current_step_in_pulseSequence(InsensitiveWindow *window, unsigned int value);
void update_pulseSequence(InsensitiveWindow *window);
void resize_pulseSequence_view(InsensitiveWindow *window);
void redraw_pulseSequence(InsensitiveWindow *window);
void on_bottomDisplay_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void close_coherencePathway(InsensitiveWindow *window);
void display_pulseProgram_code(InsensitiveWindow *window);
void export_pulse_program(GtkMenuItem *menuitem, gpointer user_data);
void update_evolutionTimes_combobox(InsensitiveWindow *window);
void insert_column_into_phaseCyclingTable(InsensitiveWindow *window, unsigned int old_n_columns, unsigned int n);
void remove_last_column_from_phaseCyclingTable(InsensitiveWindow *window);
void reset_phaseCyclingTable(InsensitiveWindow *window);
void update_phaseCyclingTable(InsensitiveWindow *window, unsigned int number_of_columns);
void on_phaseCycling_treeview_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer user_data);
void on_record_button_clicked(GtkButton *button, gpointer user_data);
void on_play_button_clicked(GtkButton *button, gpointer user_data);
void on_step_button_clicked(GtkButton *button, gpointer user_data);
void on_evolutionTimes_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_detectionMethod_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_phaseCycles_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void on_phaseCycles_entry_activate(GtkEntry *entry, gpointer user_data);
void on_acquire2DSpectrum_button_clicked(GtkButton *button, gpointer user_data);
void on_erase_button_clicked(GtkButton *button, gpointer user_data);
gboolean perform_open_pulseProgram(InsensitiveWindow *window, xmlNodePtr node);
void perform_save_pulseProgram(GtkMenuItem *menuitem, gpointer user_data);
void set_openedFileState_for_pulseSequence(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename);
void edit_sequence_element(InsensitiveWindow *window, int index);
void on_pp_edit_pulse_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_pp_edit_delay_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_pp_edit_gradient_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_pp_edit_fid_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_editing_pulsesequence_finished(gpointer sender, gpointer user_data);
void cancel_editing_sequence_element(InsensitiveWindow *window);
void erase_coherencePathway(InsensitiveWindow *window);
void set_iSpin_coherencePathway_coefficients(InsensitiveWindow *window, DSPComplex *coefficients);
void set_sSpin_coherencePathway_coefficients(InsensitiveWindow *window, DSPComplex *coefficients);
void set_needsToRecalculateCoherencePathways(InsensitiveWindow *window, gboolean value);
void draw_coherencePathway(InsensitiveWindow *window);
gpointer calculate_coherence_paths(gpointer user_data);

/* Spectrum */
gboolean shows_2D_spectrum(InsensitiveWindow *window);
gboolean get_showsFrequencyDomain(InsensitiveWindow *window);
void set_2D_mode(InsensitiveWindow *window, gboolean value);
void go_to_fft_panel(InsensitiveWindow *window);
void set_indirect_dataPoints(InsensitiveWindow *window, int value);
void enable_fft_along_t1(InsensitiveWindow *window, gboolean value);
void enable_fft_along_t2(InsensitiveWindow *window, gboolean value);
void enable_symmerization(InsensitiveWindow *window, gboolean value);
void reset_spectrum_display(InsensitiveWindow *window);
void reset_window_function(InsensitiveWindow *window);
void set_1D_dataPoints(InsensitiveWindow *window, unsigned int points, unsigned int max);
void set_complex_spectrum(InsensitiveWindow *window, DSPSplitComplex data, unsigned int points, unsigned int max);
void set_dataPoints_label(InsensitiveWindow *window, unsigned int points, unsigned int max);
void on_lineWidth_entry_activate(GtkEntry *entry, gpointer user_data);
void on_lineWidth_combobox_changed(GtkComboBoxText *combobox, gpointer user_data);
void phase_slider_changed(InsensitiveWindow *window);
float get_zero_order_phase(InsensitiveWindow *window);
void on_zeroOrder_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
float get_first_order_phase(InsensitiveWindow *window);
void on_firstOrder_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
float get_pivot_point(InsensitiveWindow *window);
void on_pivotPoint_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
gboolean removePivotPointTimerEvent(gpointer user_data);
void on_resetPhase_button_clicked(GtkButton *button, gpointer user_data);
gboolean get_showRealSpectrum(InsensitiveWindow *window);
void set_showRealSpectrum(InsensitiveWindow *window, gboolean value);
void on_showReal_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
gboolean get_showImaginarySpectrum(InsensitiveWindow *window);
void set_showImaginarySpectrum(InsensitiveWindow *window, gboolean value);
void on_showImaginary_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
gboolean get_showIntegral(InsensitiveWindow *window);
void set_showIntegral(InsensitiveWindow *window, gboolean value);
void on_integral_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
gboolean get_show_windowFunction(InsensitiveWindow *window);
void set_show_windowFunction(InsensitiveWindow *window, gboolean value);
void on_window_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void set_display_frequency_domain(InsensitiveWindow *self, gboolean value);
enum PlotMode get_plotMode(InsensitiveWindow *window);
void on_grid_checkbox_toggled(GtkToggleButton *button, gpointer user_data);
void on_plotStyle_combobox_changed(GtkComboBox *combobox, gpointer user_data);
int get_current_spectrum_domain(InsensitiveWindow *window);
void on_fid_button_clicked(GtkButton *button, gpointer user_data);
void on_fft1D_button_clicked(GtkButton *button, gpointer user_data);
void on_fft2D_button_clicked(GtkButton *button, gpointer user_data);
void on_magnitude_button_clicked(GtkButton *button, gpointer user_data);
int get_statesDataSet(InsensitiveWindow *window);

void on_apodization_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void on_sym_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_syma_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_symj_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_tilt_jres_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_tilt_secsy_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_dosyToolbox_button_clicked(GtkButton *button, gpointer user_data);
void on_dosy_show_1D_trace_only_button_clicked(GtkButton *button, gpointer user_data);
void on_dosy_fit_button_clicked(GtkButton *button, gpointer user_data);
void on_dosy_spectrum_button_clicked(GtkButton *button, gpointer user_data);
void on_auto_peak_picking_button_clicked(GtkButton *button, gpointer user_data);
void on_dosy_fit_lorentzian_peaks_button_clicked(GtkButton *button, gpointer user_data);
void reset_dosy_panel(InsensitiveWindow *window);
void on_signalToNoiseThreshold_entry_activate(GtkEntry *entry, gpointer user_data);

void set_apodization(InsensitiveWindow *window, float *pointerT2, float *pointerT1);
void show_spectrum_progressbar(InsensitiveWindow *window, gboolean value);
void set_spectrum_progressbar_maximum(InsensitiveWindow *window, double value);
void add_to_spectrum_progressbar(InsensitiveWindow *window, double value);
void play_sound(InsensitiveWindow *window);

void set_openedFileState_for_spectrum(InsensitiveWindow *window, enum OpenFileState state, const gchar *filename);
gboolean perform_open_spectrum(InsensitiveWindow *window, xmlNodePtr node);
void perform_save_spectrum(GtkMenuItem *menuitem, gpointer user_data);
void export_spectrum(GtkMenuItem *menuitem, InsensitiveWindow *window);
void update_spectrum_parameter_panel(InsensitiveWindow *window);

void show_spectrumParameters_textview(InsensitiveWindow *window, gboolean value);
void on_toggleParameters_button_clicked(GtkButton *button, gpointer user_data);
void set_maxDataPoints(InsensitiveWindow *window, unsigned int value);
void set_phase_correction(InsensitiveWindow *window, float phase0, float phase1, float pivot);
void set_noise_spectrum(InsensitiveWindow *window, DSPSplitComplex splitComplex);
float magnification(InsensitiveWindow *window);
void set_magnification(InsensitiveWindow *window, float value);
void reset_magnification(InsensitiveWindow *window);
void recalculate_graph(InsensitiveWindow *window);
DSPSplitComplex displayed_graph(InsensitiveWindow *window);

/* Command Line */
void execute_command(GtkEntry *entry, gpointer user_data);
void show_command_error(GtkWidget *widget, gpointer window);
void alert_for_invalid_fourier_transform(InsensitiveWindow *window, unsigned int dimension);

/* Drawing */
gboolean draw_matrix_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_vector_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void draw_vector_to_context(InsensitiveSettings *settings, cairo_t *cr,
                            float width, float height, float origin_x, float origin_y,
						    float x, float y, float z, unsigned int i_or_s, float type, gboolean active);
void draw_grapefruit_paths(InsensitiveController *controller, gboolean upper, gboolean front, cairo_t *cr,
                           float width, float height, float origin_x, float origin_y, unsigned int i_or_s);
gboolean draw_spinEditor_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_spinEditor_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void set_energy_values(InsensitiveWindow *window, float *array, unsigned int levels,
                       GPtrArray *names, int *transitions, float *probabilities);
void draw_energyLevel_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void draw_pulseSequence_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void create_pulseSequence_view(InsensitiveWindow *window, int width, int height);
void add_label_for_element(cairo_t *cr, enum SequenceType type, float x, float y, int index);
int get_sequenceElementIndex_from_mouse_position(InsensitiveWindow *window, float mousePosition);
void on_pulseSequence_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void draw_coherencePathway_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void draw_graph_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
float contour_height(InsensitiveWindow *window, int x, int y, int maxX, int maxY, gboolean negativeValues);
float contour_for_index(int index);
void compute_contours(InsensitiveWindow *window, GPtrArray *contours, gboolean negativeValues);
void on_spectrum_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_spectrum_drawingarea_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_spectrum_drawingarea_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
