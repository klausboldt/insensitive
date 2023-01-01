/* main.c
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

#include <glib/gi18n.h>

#include "insensitive-config.h"
#include "insensitive-window.h"


static void on_activate(GtkApplication *app)
{
	GtkWindow *window;

	/* It's good practice to check your parameters at the beginning of the
	 * function. It helps catch errors early and in development instead of
	 * by your users.
	 */
	g_assert(GTK_IS_APPLICATION(app));

	/* Get the current window or create one if necessary. */
	window = gtk_application_get_active_window(app);
	if (window == NULL)
		window = g_object_new(INSENSITIVE_TYPE_WINDOW,
		                      "application", app,
		                      "default-width", 1024,
		                      "default-height", 768,
		                      NULL);

	/* Ask the window manager/compositor to present the window. */
	gtk_window_present(window);
}


static void on_open(GApplication *app, GFile **files, gint n_files, const gchar *hint)
{
	GList *windows;
	InsensitiveWindow *window;
	int i;

	g_assert(GTK_IS_APPLICATION(app));

	window = INSENSITIVE_WINDOW(gtk_application_get_window_by_id(app, 1));
	if (window == NULL)
		window = g_object_new(INSENSITIVE_TYPE_WINDOW,
		                      "application", app,
		                      "default-width", 1024,
		                      "default-height", 768,
		                      NULL);

	for (i = 0; i < n_files; i++)
		open_file(window, g_file_get_path(files[i]));

	gtk_window_present (GTK_WINDOW(window));
}


int main(int argc, char *argv[])
{
	g_autoptr(GtkApplication) app = NULL;
	int ret;

	/* Set up gettext translations */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	/*
	 * Create a new GtkApplication. The application manages our main loop,
	 * application windows, integration with the window manager/compositor, and
	 * desktop features such as file opening and single-instance applications.
	 */
	app = gtk_application_new("com.klausboldt.insensitive", G_APPLICATION_HANDLES_OPEN);

	/*
	 * We connect to the activate signal to create a window when the application
	 * has been launched. Additionally, this signal notifies us when the user
	 * tries to launch a "second instance" of the application. When they try
	 * to do that, we'll just present any existing window.
	 *
	 * Because we can't pass a pointer to any function type, we have to cast
	 * our "on_activate" function to a GCallback.
	 */
	g_signal_connect(app, "activate", G_CALLBACK (on_activate), NULL);
	g_signal_connect(app, "open", G_CALLBACK (on_open), NULL);

	/*
	 * Run the application. This function will block until the application
	 * exits. Upon return, we have our exit code to return to the shell. (This
	 * is the code you see when you do `echo $?` after running a command in a
	 * terminal.
	 *
	 * Since GtkApplication inherits from GApplication, we use the parent class
	 * method "run". But we need to cast, which is what the "G_APPLICATION()"
	 * macro does.
	 */
	ret = g_application_run(G_APPLICATION (app), argc, argv);

	return ret;
}
