/* insensitive-tutorial.h
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
#include <webkit2/webkit2.h>

#include "insensitive.h"

G_BEGIN_DECLS

#define INSENSITIVE_TYPE_TUTORIAL (insensitive_tutorial_get_type())

G_DECLARE_FINAL_TYPE(InsensitiveTutorial, insensitive_tutorial, INSENSITIVE, TUTORIAL, GtkWindow)

G_END_DECLS

void on_back_button_clicked(GtkToolItem *button, gpointer user_data);
void on_forward_button_clicked(GtkToolItem *button, gpointer user_data);
void load_default_page(GtkToolItem *button, gpointer user_data);
void load_manual_page(GtkToolItem *button, gpointer user_data);
void load_cmd_page(GtkToolItem *button, gpointer user_data);
void load_index_page(GtkToolItem *button, gpointer user_data);
void load_arbitrary_page(gchar *html_file, gpointer user_data);
