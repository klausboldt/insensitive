/* insensitive-pulseshaper.c
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
#include "insensitive-pulseshaper.h"
#include "insensitive-library.h"
#include "insensitive-controller.h"
#include "insensitive-settings.h"


struct _InsensitivePulseShaper
{
	GtkWindow           parent_instance;

	GtkDrawingArea      *timeDomain_drawingarea, *frequencyDomain_drawingarea;
    GtkComboBoxText     *pulseShape_combobox;
    GtkEntry            *time_entry, *frequency_entry;
    GtkRadioButton      *Mxy_phase_radiobutton, *Mx_My_radiobutton, *Mz_radiobutton, *Mxy_FT_radiobutton;
    GtkScale            *time_slider, *frequency_slider;
    GtkAdjustment       *time_adjustment, *frequency_adjustment;
    GtkLabel            *scaleLabel200, *scaleLabel400, *scaleLabel600, *scaleLabel800, *scaleLabel1000;

    InsensitiveController *controller;

    float lineWidth;
    DSPSplitComplex timeDomainData, frequencyDomainData;
};


G_DEFINE_TYPE(InsensitivePulseShaper, insensitive_pulse_shaper, GTK_TYPE_WINDOW)


static void insensitive_pulse_shaper_class_init(InsensitivePulseShaperClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-pulseshaper.ui");

	gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, timeDomain_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, frequencyDomain_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, pulseShape_combobox);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, time_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, frequency_entry);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, Mxy_phase_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, Mx_My_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, Mz_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, Mxy_FT_radiobutton);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, time_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, frequency_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, time_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, frequency_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, scaleLabel200);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, scaleLabel400);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, scaleLabel600);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, scaleLabel800);
    gtk_widget_class_bind_template_child(widget_class, InsensitivePulseShaper, scaleLabel1000);
}


static void insensitive_pulse_shaper_dispose(GObject *gobject)
{
	InsensitivePulseShaper *self = (InsensitivePulseShaper *)gobject;

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

	G_OBJECT_CLASS(insensitive_pulse_shaper_parent_class)->dispose(gobject);
}


static void insensitive_pulse_shaper_finalize(GObject *gobject)
{
	InsensitivePulseShaper *self = (InsensitivePulseShaper *)gobject;

    g_free(self->timeDomainData.realp);
    g_free(self->timeDomainData.imagp);
    g_free(self->frequencyDomainData.realp);
    g_free(self->frequencyDomainData.imagp);

	G_OBJECT_CLASS(insensitive_pulse_shaper_parent_class)->finalize(gobject);
}


static void insensitive_pulse_shaper_init(InsensitivePulseShaper *self)
{
    int i;

    gtk_widget_init_template(GTK_WIDGET(self));

    self->lineWidth = 2.0;
    self->timeDomainData.realp = malloc(pulsePowerSpectrumResolution * sizeof(float));
    self->timeDomainData.imagp = malloc(pulsePowerSpectrumResolution * sizeof(float));
    self->frequencyDomainData.realp = malloc(pulsePowerSpectrumCenter * sizeof(float));
    self->frequencyDomainData.imagp = malloc(pulsePowerSpectrumCenter * sizeof(float));
    self->controller = NULL;

    for (i = 1; i <= 512; i += 51)
        gtk_scale_add_mark(self->time_slider, i, GTK_POS_BOTTOM, NULL);
    gtk_scale_add_mark(self->frequency_slider, -127, GTK_POS_BOTTOM, NULL);
    gtk_scale_add_mark(self->frequency_slider, -64, GTK_POS_BOTTOM, NULL);
    gtk_scale_add_mark(self->frequency_slider, 0, GTK_POS_BOTTOM, NULL);
    gtk_scale_add_mark(self->frequency_slider, 64, GTK_POS_BOTTOM, NULL);
    gtk_scale_add_mark(self->frequency_slider, 127, GTK_POS_BOTTOM, NULL);

#ifndef __APPLE__
    GList *list;
    list = gtk_container_get_children(GTK_CONTAINER(self->Mxy_phase_radiobutton));
    gtk_label_set_markup(GTK_LABEL(list->data), "M<sub>xy</sub>+ φ  ");
    list = gtk_container_get_children(GTK_CONTAINER(self->Mx_My_radiobutton));
    gtk_label_set_markup(GTK_LABEL(list->data), "M<sub>x</sub>+ M<sub>y</sub>  ");
    list = gtk_container_get_children(GTK_CONTAINER(self->Mz_radiobutton));
    gtk_label_set_markup(GTK_LABEL(list->data), "M<sub>z</sub>  ");
    list = gtk_container_get_children(GTK_CONTAINER(self->Mxy_FT_radiobutton));
    gtk_label_set_markup(GTK_LABEL(list->data), "M<sub>xy</sub>+ FT");
#endif /* __APPLE__ */
}


void insensitive_pulse_shaper_set_controller(InsensitivePulseShaper *self, gpointer controller)
{
    self->controller = (InsensitiveController *)controller;

    insensitive_pulse_shaper_set_pulsePowerDisplayMode(self, insensitive_settings_get_excitationProfile(self->controller->settings));
    insensitive_pulse_shaper_set_ignoreOffResonanceEffectsForPulses(self, insensitive_settings_get_ignoreOffResonanceEffectsForPulses(self->controller->settings));
}


void insensitive_pulse_shaper_refreshGraphs(InsensitivePulseShaper *self)
{
	DSPSplitComplex pulseShape, pulsePowerSpectrum;
	int i;

	if (self != NULL && self->controller != NULL) {
		pulseShape = insensitive_controller_get_pulseShape(self->controller);
		pulsePowerSpectrum = insensitive_controller_get_pulsePowerSpectrum(self->controller);
		for (i = 0; i < pulsePowerSpectrumResolution; i++) {
			self->timeDomainData.realp[i] = pulseShape.realp[i];
			self->timeDomainData.imagp[i] = pulseShape.imagp[i];
			if (i < pulsePowerSpectrumCenter) {
				self->frequencyDomainData.realp[i] = pulsePowerSpectrum.realp[i];
				self->frequencyDomainData.imagp[i] = pulsePowerSpectrum.imagp[i];
			}
		}
		gtk_widget_queue_draw((GtkWidget *)self->timeDomain_drawingarea);
		gtk_widget_queue_draw((GtkWidget *)self->frequencyDomain_drawingarea);
	}
}


void insensitive_pulse_shaper_set_pulseLengthScale_for_flipAngle(InsensitivePulseShaper *self, float flipAngle)
{
	gchar *str;
	float factor = (flipAngle == 0) ? 0.01 : flipAngle / 360;

	if (self != NULL) {
		str = malloc(10 * sizeof(gchar));
		sprintf(str, "%.0f", factor * 200);
		gtk_label_set_text(self->scaleLabel200, str);
		sprintf(str, "%.0f", factor * 400);
		gtk_label_set_text(self->scaleLabel400, str);
		sprintf(str, "%.0f", factor * 600);
		gtk_label_set_text(self->scaleLabel600, str);
		sprintf(str, "%.0f", factor * 800);
		gtk_label_set_text(self->scaleLabel800, str);
		sprintf(str, "%.0f", factor * 1000);
		gtk_label_set_text(self->scaleLabel1000, str);
		free(str);
	}
}


void insensitive_pulse_shaper_set_pulseLength(InsensitivePulseShaper *self, float value)
{
	gchar *str;

	if (self != NULL && self->controller != NULL) {
		str = malloc(10 * sizeof(gchar));
		gtk_adjustment_set_value(self->time_adjustment,
					 pulseDurationToSliderScale(value, insensitive_settings_get_flipAngle(self->controller->settings)));
		sprintf(str, "%.3f", value);
		gtk_entry_set_text(self->time_entry, str);
		free(str);
	}
}


G_MODULE_EXPORT void on_time_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;
    float value = atof(gtk_entry_get_text(entry));

    if (window != NULL && window->controller != NULL)
        insensitive_controller_set_pulseDuration(window->controller, value);
}


G_MODULE_EXPORT void on_time_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;
    float value = gtk_adjustment_get_value(adjustment);

    if (window != NULL && window->controller != NULL)
        insensitive_controller_set_pulseLength(window->controller, value);
}


G_MODULE_EXPORT void on_reset_timeDomain_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;

    if (window != NULL && window->controller != NULL)
        insensitive_controller_make_hard_pulse(window->controller);
}


void insensitive_pulse_shaper_set_pulseFrequency(InsensitivePulseShaper *self, float value)
{
	gchar *str;

	if (self != NULL) {
		str = malloc(5 * sizeof(gchar));
		gtk_adjustment_set_value(self->frequency_adjustment, value);
		sprintf(str, "%.2f", value);
		gtk_entry_set_text(self->frequency_entry, str);
		free(str);
	}
}


G_MODULE_EXPORT void on_frequency_entry_activate(GtkEntry *entry, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;
    float value = atof(gtk_entry_get_text(entry));

    if (window != NULL && window->controller != NULL)
        insensitive_controller_set_pulseFrequency(window->controller, value);
}


G_MODULE_EXPORT void on_frequency_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;
    float value = gtk_adjustment_get_value(adjustment);

    if (window != NULL && window->controller != NULL)
        insensitive_controller_set_pulseFrequency(window->controller, value);
}


G_MODULE_EXPORT void on_reset_frequencyDomain_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;

    if (window != NULL && window->controller != NULL)
        insensitive_controller_set_pulseFrequency(window->controller, standardPulseFrequency);
}


void insensitive_pulse_shaper_set_pulseEnvelope(InsensitivePulseShaper *self, enum PulseEnvelope value)
{
    if (self != NULL)
        gtk_combo_box_set_active((GtkComboBox *)self->pulseShape_combobox, (gint)value);
}


G_MODULE_EXPORT void on_pulseShape_combobox_changed(GtkComboBox *combobox, gpointer user_data)
{
    InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;

    if (window != NULL && window->controller != NULL)
	    insensitive_controller_set_pulseEnvelope(window->controller,
	    					                     (enum PulseEnvelope)gtk_combo_box_get_active((GtkComboBox *)combobox));
}


void insensitive_pulse_shaper_set_pulsePowerDisplayMode(InsensitivePulseShaper *self, enum ExcitationProfile value)
{
	if (self != NULL) {
		switch (value) {
		case Mxy_Phase:
			gtk_toggle_button_set_active((GtkToggleButton *)self->Mxy_phase_radiobutton, TRUE);
			on_pulsePowerDisplayMode_changed(self->Mxy_phase_radiobutton, self);
			break;
		case Mx_My:
			gtk_toggle_button_set_active((GtkToggleButton *)self->Mx_My_radiobutton, TRUE);
			on_pulsePowerDisplayMode_changed(self->Mx_My_radiobutton, self);
			break;
		case Mz:
			gtk_toggle_button_set_active((GtkToggleButton *)self->Mz_radiobutton, TRUE);
			on_pulsePowerDisplayMode_changed(self->Mz_radiobutton, self);
			break;
        case Mxy_FT:
			gtk_toggle_button_set_active((GtkToggleButton *)self->Mxy_FT_radiobutton, TRUE);
			on_pulsePowerDisplayMode_changed(self->Mxy_FT_radiobutton, self);
			break;
		}
	}
}


G_MODULE_EXPORT void on_pulsePowerDisplayMode_changed(GtkRadioButton *button, gpointer user_data)
{
	InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;

	if (window != NULL && window->controller != NULL) {
		if (button == window->Mxy_phase_radiobutton)
			insensitive_settings_set_excitationProfile(window->controller->settings, Mxy_Phase);
		else if (button == window->Mx_My_radiobutton)
			insensitive_settings_set_excitationProfile(window->controller->settings, Mx_My);
		else if (button == window->Mz_radiobutton)
			insensitive_settings_set_excitationProfile(window->controller->settings, Mz);
        else if (button == window->Mxy_FT_radiobutton)
			insensitive_settings_set_excitationProfile(window->controller->settings, Mxy_FT);
		insensitive_controller_create_pulse_powerspectrum(window->controller);
	}
}


void insensitive_pulse_shaper_set_ignoreOffResonanceEffectsForPulses(InsensitivePulseShaper *self, gboolean value)
{
	if (self != NULL && self->controller != NULL) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->Mxy_phase_radiobutton), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(self->Mxy_phase_radiobutton), !value);
		gtk_widget_set_sensitive(GTK_WIDGET(self->Mx_My_radiobutton), !value);
		gtk_widget_set_sensitive(GTK_WIDGET(self->Mz_radiobutton), !value);
        gtk_widget_set_sensitive(GTK_WIDGET(self->Mxy_FT_radiobutton), !value);
	}
}


G_MODULE_EXPORT void draw_pulse_shaper_graph_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	InsensitivePulseShaper *window = (InsensitivePulseShaper *)user_data;
	GtkDrawingArea *drawingarea = (GtkDrawingArea *)widget;
    gboolean plotFrequencyDomain = (drawingarea == window->frequencyDomain_drawingarea);
    unsigned int i, maxDataPoints, index;
	float stepSizeX, stepSizeY, stepSizeX_FT;
	float width, height, origin_x, origin_y;
    DSPSplitComplex displayedData;
    double dashes[2];
    enum ExcitationProfile excitationProfile;

    maxDataPoints = plotFrequencyDomain ? pulsePowerSpectrumCenter : pulsePowerSpectrumResolution;
    displayedData = plotFrequencyDomain ? window->frequencyDomainData : window->timeDomainData;
    if (insensitive_settings_get_ignoreOffResonanceEffectsForPulses(window->controller->settings)) {
        excitationProfile = Mxy_Phase;
	} else {
        excitationProfile = insensitive_settings_get_excitationProfile(window->controller->settings);
	}
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	origin_x = 0.0;
	origin_y = 0.0;

	cairo_rectangle(cr, origin_x, origin_y, width, height);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
	cairo_stroke(cr);

	stepSizeX = width / (maxDataPoints - 1);
    stepSizeY = height;
    origin_x = 0.0;
    if (plotFrequencyDomain) {
        switch (excitationProfile) {
        case Mxy_Phase:
            stepSizeY /= 1.7;
            origin_y = 3 * height / 4;
            break;
        case Mx_My:
            stepSizeY /= 1.2;
            origin_y = height / 2;
		    stepSizeY /= 2;
            break;
        case Mz:
            stepSizeY /= 2.4;
            origin_y = height / 2;
            break;
        case Mxy_FT:
            stepSizeY /= 1.5;
            origin_y = 5 * height / 6;
        }
    } else {
        stepSizeY /= 1.5;
		origin_y = height / 2;
		stepSizeY /= 2;
	}
	cairo_set_line_width(cr, 3);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

	// Draw graph
	if (maxDataPoints > 0) {
		// Draw the imaginary graph in green
		if (plotFrequencyDomain && insensitive_settings_get_excitationProfile(window->controller->settings) != Mz
            && !insensitive_settings_get_ignoreOffResonanceEffectsForPulses(window->controller->settings)) {
			cairo_set_line_width(cr, window->lineWidth);
            if (insensitive_settings_get_excitationProfile(window->controller->settings) == Mxy_FT) {
                // Only show the central 1/10th of the FFT, but to improve the resolution the
                // time domain data is expanded 4-fold, which means only show the central 4/10th
                for (i = 0; i < 0.4 * maxDataPoints + 2; i++) {
                    index = i + 0.5 * maxDataPoints * (1 - 0.4);
                    stepSizeX_FT = width / (0.8 * maxDataPoints + 2.0);
                    stepSizeX_FT *= 2;
			        if (i == 0) {
			    	    cairo_move_to(cr, origin_x, origin_y - stepSizeY * displayedData.imagp[index]);
			        } else {
			    	    cairo_line_to(cr, origin_x + i * stepSizeX_FT, origin_y - stepSizeY * displayedData.imagp[index]);
			        }
                }
                cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
                dashes[0] = 6.0;
                dashes[1] = 3.0;
                cairo_set_dash(cr, dashes, 2, 0.0);
            } else {
			    for (i = 0; i < maxDataPoints; i++) {
				    if (i == 0) {
					    cairo_move_to(cr, origin_x, origin_y - stepSizeY * displayedData.imagp[i]);
				    } else {
					    cairo_line_to(cr, origin_x + i * stepSizeX, origin_y - stepSizeY * displayedData.imagp[i]);
				    }
			    }
			    cairo_set_source_rgba(cr, 0.0, 0.75, 0.0, 1.0);
            }
			cairo_stroke(cr);
		}
		// Draw the real graph in red
		cairo_set_line_width(cr, window->lineWidth);
        cairo_set_dash(cr, NULL, 0, 0.0);
        if (plotFrequencyDomain && insensitive_settings_get_ignoreOffResonanceEffectsForPulses(window->controller->settings)) {
            // Only show the central 1/10th of the FFT, but to improve the resolution the
            // time domain data is expanded 4-fold, which means only show the central 4/10th
            for (i = 0; i <= 0.4 * maxDataPoints + 2; i++) {
                index = i + 0.5 * maxDataPoints * (1 - 0.4);
                stepSizeX = width / (0.8 * maxDataPoints + 2.0);
                stepSizeX *= 2;
			    if (i == 0) {
			    	cairo_move_to(cr, origin_x, origin_y - stepSizeY * displayedData.realp[index]);
			    } else {
			    	cairo_line_to(cr, origin_x + i * stepSizeX, origin_y - stepSizeY * displayedData.realp[index]);
			    }
            }
        } else {
		    for (i = 0; i < maxDataPoints; i++) {
			    if (i == 0) {
			    	cairo_move_to(cr, origin_x, origin_y - stepSizeY * displayedData.realp[i]);
			    } else {
			    	cairo_line_to(cr, origin_x + i * stepSizeX, origin_y - stepSizeY * displayedData.realp[i]);
			    }
            }
		}
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		cairo_stroke(cr);
	}
}
