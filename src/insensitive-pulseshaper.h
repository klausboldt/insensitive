/* insensitive-pulseshaper.h
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

G_BEGIN_DECLS

#define INSENSITIVE_TYPE_PULSESHAPER (insensitive_pulse_shaper_get_type())

G_DECLARE_FINAL_TYPE(InsensitivePulseShaper, insensitive_pulse_shaper, INSENSITIVE, PULSESHAPER, GtkWindow)

G_END_DECLS

void insensitive_pulse_shaper_set_controller(InsensitivePulseShaper *self, gpointer controller);
void insensitive_pulse_shaper_refreshGraphs(InsensitivePulseShaper *self);
void insensitive_pulse_shaper_set_pulseLengthScale_for_flipAngle(InsensitivePulseShaper *self, float flipAngle);
void insensitive_pulse_shaper_set_pulseLength(InsensitivePulseShaper *self, float value);
void on_time_entry_activate(GtkEntry *entry, gpointer user_data);
void on_time_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
void on_reset_timeDomain_button_clicked(GtkButton *button, gpointer user_data);
void insensitive_pulse_shaper_set_pulseFrequency(InsensitivePulseShaper *self, float value);
void on_frequency_entry_activate(GtkEntry *entry, gpointer user_data);
void on_frequency_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);
void on_reset_frequencyDomain_button_clicked(GtkButton *button, gpointer user_data);
void insensitive_pulse_shaper_set_pulseEnvelope(InsensitivePulseShaper *self, enum PulseEnvelope value);
void on_pulseShape_combobox_changed(GtkComboBox *combobox, gpointer user_data);
void insensitive_pulse_shaper_set_pulsePowerDisplayMode(InsensitivePulseShaper *self, enum ExcitationProfile value);
void on_pulsePowerDisplayMode_changed(GtkRadioButton *button, gpointer user_data);
void insensitive_pulse_shaper_set_ignoreOffResonanceEffectsForPulses(InsensitivePulseShaper *self, gboolean value);
void draw_pulse_shaper_graph_view(GtkWidget *widget, cairo_t *cr, gpointer user_data);
