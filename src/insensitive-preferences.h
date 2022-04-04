/* insensitive-preferences.h
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

#define INSENSITIVE_TYPE_PREFERENCES (insensitive_preferences_get_type())

G_DECLARE_FINAL_TYPE(InsensitivePreferences, insensitive_preferences, INSENSITIVE, PREFERENCES, GtkWindow)

void insensitive_preferences_set_controller(InsensitivePreferences *self, gpointer newController);
void on_playSound_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void on_offResonance_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void on_allowShiftCoupling_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data);
void on_export_radiobutton_toggled(GtkToggleButton *radiobutton, gpointer user_data);
void on_coherence_pathway_adjustment_changed(GtkAdjustment *adjustment, gpointer user_data);
void on_font_button_font_set(GtkFontButton *fontButton, gpointer user_data);
void on_reset_button_clicked(GtkButton *button, gpointer user_data);

G_END_DECLS
