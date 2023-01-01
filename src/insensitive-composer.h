/* insensitive-composer.h
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

#define INSENSITIVE_TYPE_COMPOSER (insensitive_composer_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveComposer, insensitive_composer, INSENSITIVE, COMPOSER, GtkWindow)

#define INSENSITIVE_TYPE_COMPOSER_ROW (insensitive_composer_row_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveComposerRow, insensitive_composer_row, INSENSITIVE, COMPOSER_ROW, GtkListBoxRow)

G_END_DECLS

void insensitive_composer_set_controller(InsensitiveComposer *self, gpointer newController);
void insensitive_composer_reset(InsensitiveComposer *self);
void insensitive_composer_initialize(InsensitiveComposer *self);
void insensitive_composer_add_operator(GtkToolButton *toolbutton, gpointer user_data);
void insensitive_composer_delete_operator(InsensitiveComposer *self, InsensitiveComposerRow *row);
void insensitive_composer_update_productOperator_string(InsensitiveComposer *self);
void on_updateMatrix_button_clicked(GtkToolButton *toolbutton, gpointer user_data);
