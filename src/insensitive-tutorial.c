/* insensitive-tutorial.c
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
#include "insensitive-tutorial.h"


struct _InsensitiveTutorial {
	GtkWindow           parent_instance;

	WebKitWebView       *tutorial_webview;
};


G_DEFINE_TYPE(InsensitiveTutorial, insensitive_tutorial, GTK_TYPE_WINDOW)


static void insensitive_tutorial_class_init(InsensitiveTutorialClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    g_type_ensure(WEBKIT_TYPE_WEB_VIEW);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-tutorial.ui");

	gtk_widget_class_bind_template_child(widget_class, InsensitiveTutorial, tutorial_webview);
}


static void insensitive_tutorial_init(InsensitiveTutorial *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    load_default_page(NULL, self->tutorial_webview);
}


G_MODULE_EXPORT void on_back_button_clicked(GtkToolItem *button, gpointer user_data)
{
    webkit_web_view_go_back((WebKitWebView *)user_data);
}


G_MODULE_EXPORT void on_forward_button_clicked(GtkToolItem *button, gpointer user_data)
{
    webkit_web_view_go_forward((WebKitWebView *)user_data);
}


G_MODULE_EXPORT void load_default_page(GtkToolItem *button, gpointer user_data)
{
    WebKitWebView *webview = (WebKitWebView *)user_data;
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", "default.html", NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			webkit_web_view_load_uri(webview, g_filename_to_uri(filename, NULL, NULL));
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}


G_MODULE_EXPORT void load_manual_page(GtkToolItem *button, gpointer user_data)
{
    WebKitWebView *webview = (WebKitWebView *)user_data;
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", "manual.html", NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			webkit_web_view_load_uri(webview, g_filename_to_uri(filename, NULL, NULL));
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}


G_MODULE_EXPORT void load_cmd_page(GtkToolItem *button, gpointer user_data)
{
    WebKitWebView *webview = (WebKitWebView *)user_data;
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", "commandline.html", NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			webkit_web_view_load_uri(webview, g_filename_to_uri(filename, NULL, NULL));
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}


G_MODULE_EXPORT void load_index_page(GtkToolItem *button, gpointer user_data)
{
    WebKitWebView *webview = (WebKitWebView *)user_data;
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", "index.html", NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			webkit_web_view_load_uri(webview, g_filename_to_uri(filename, NULL, NULL));
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}


G_MODULE_EXPORT void load_arbitrary_page(gchar *html_filename, gpointer user_data)
{
    InsensitiveTutorial *window = (InsensitiveTutorial *)user_data;
    gchar *filename;
    const gchar * const *dirs = g_get_system_data_dirs();

    while (*dirs != NULL) {
        filename = g_build_filename(*dirs++, "insensitive", "doc", html_filename, NULL);
        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
			webkit_web_view_load_uri(window->tutorial_webview, g_filename_to_uri(filename, NULL, NULL));
            g_free(filename);
            break;
        }
		g_free(filename);
	}
}
