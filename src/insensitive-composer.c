/* insensitive-composer.c
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
#include "insensitive-composer.h"
#include "insensitive-library.h"
#include "insensitive-controller.h"
#include "insensitive-spinsystem.h"


/********************************** Insensitive Composer Row **********************************/


struct _InsensitiveComposerRow
{
    GtkListBoxRow parent;

    GtkEntry            *coefficient_entry;
    GtkLabel            *factor_label;
    GtkComboBoxText     *spin1_combobox, *spin2_combobox, *spin3_combobox, *spin4_combobox;
    GtkButton           *delete_button;
    InsensitiveComposer *parentComposer;

    unsigned int        codedProductOperator, numberOfSpins, spinTypeArray;
    float               coefficientForProductOperator;
    int                 activeSpins;
    GPtrArray           *comboBoxArray;
};


struct _InsensitiveComposerRowClass
{
    GtkListBoxRowClass parent_class;
};


G_DEFINE_TYPE (InsensitiveComposerRow, insensitive_composer_row, GTK_TYPE_LIST_BOX_ROW);


static void insensitive_composer_row_finalize(GObject *obj)
{
	InsensitiveComposerRow *self = (InsensitiveComposerRow *)obj;

	G_OBJECT_CLASS(insensitive_composer_row_parent_class)->finalize(obj);
}


static void insensitive_composer_row_class_init(InsensitiveComposerRowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = insensitive_composer_row_finalize;

	gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-composer-row.ui");
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, coefficient_entry);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, factor_label);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, spin1_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, spin2_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, spin3_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, spin4_combobox);
	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposerRow, delete_button);
}


static void insensitive_composer_row_init(InsensitiveComposerRow *row)
{
	gtk_widget_init_template(GTK_WIDGET(row));
}


static InsensitiveComposerRow *insensitive_composer_row_new(InsensitiveComposer *composer, InsensitiveSpinSystem *spinsystem)
{
    InsensitiveComposerRow *row = g_object_new(INSENSITIVE_TYPE_COMPOSER_ROW, NULL);

    row->parentComposer = composer;
    if (spinsystem) {
        if(spinsystem->spins < 4)
            gtk_widget_set_visible((GtkWidget *)row->spin4_combobox, FALSE);
        if(spinsystem->spins < 3)
            gtk_widget_set_visible((GtkWidget *)row->spin3_combobox, FALSE);
        if(spinsystem->spins < 2)
            gtk_widget_set_visible((GtkWidget *)row->spin2_combobox, FALSE);
    }
    row->codedProductOperator = 0;
    row->coefficientForProductOperator = 1.0;
    row->numberOfSpins = spinsystem->spins;
    row->activeSpins = 0;
    row->spinTypeArray = spinsystem->spinTypeArray;
    row->comboBoxArray = g_ptr_array_new();
    g_ptr_array_add(row->comboBoxArray, row->spin1_combobox);
    g_ptr_array_add(row->comboBoxArray, row->spin2_combobox);
    g_ptr_array_add(row->comboBoxArray, row->spin3_combobox);
    g_ptr_array_add(row->comboBoxArray, row->spin4_combobox);

	return row;
}


void on_coefficient_entry_changed(GtkEntry *entry, gpointer user_data)
{
    InsensitiveComposerRow *self = (InsensitiveComposerRow *)user_data;

    self->coefficientForProductOperator = atof(gtk_entry_get_text(entry));
    insensitive_composer_update_productOperator_string(self->parentComposer);
}


void on_spin_combobox_changed(GtkComboBoxText *combobox, gpointer user_data)
{
    InsensitiveComposerRow *self = (InsensitiveComposerRow *)user_data;
    unsigned int i;
    gchar *str = malloc(3 * sizeof(gchar));

    self->activeSpins = -1;
	// Add one active spin for each NSPopUpButton that is not the identity operator E/2
	for (i = 0; i < self->numberOfSpins; i++)
        if (strcmp(gtk_combo_box_text_get_active_text(g_ptr_array_index(self->comboBoxArray, i)), "E/2"))
            self->activeSpins++;
	// Set the normalization factor to 1 if product operator is identity operator E/2
	if(self->activeSpins == -1) self->activeSpins++;

	// Factor is 2^activeSpins
    sprintf(str, "%d", pow2(self->activeSpins));
    gtk_label_set_text(self->factor_label, str);
    free(str);

    /**********************************************************/
	/*                                                        */
	/*   To index the different product operators a base-4    */
	/*   system is used:                                      */
	/*   With the basis vectors coded as 0 = 0, z = 1, x = 2  */
	/*   and y = 3. The product operator 4 I_2z I3_y in a     */
	/*   system of three spins would be encoded as:           */
	/*     index = 310 (base-4) = 28 (base-10)                */
	/*   (the indexing must be read from right to left.)      */
	/*                                                        */
	/**********************************************************/

    self->codedProductOperator = 0;
	for (i = 0; i < self->numberOfSpins; i++) {
        str = gtk_combo_box_text_get_active_text(g_ptr_array_index(self->comboBoxArray, i));
        if (*(str + 2) == 'z')
            self->codedProductOperator += pow(4, i);
        else if (*(str + 2) == 'x')
            self->codedProductOperator += 2 * pow(4, i);
        else if (*(str + 2) == 'y')
            self->codedProductOperator += 3 * pow(4, i);
    }

	// Send message to controller that operator has been changed
    insensitive_composer_update_productOperator_string((InsensitiveComposer *)self->parentComposer);
}


void on_delete_button_clicked(GtkButton *button, gpointer user_data)
{
    InsensitiveComposerRow *self = (InsensitiveComposerRow *)user_data;

    insensitive_composer_delete_operator(self->parentComposer, self);
}


/************************************ Insensitive Composer ************************************/


struct _InsensitiveComposer
{
	GtkWindow           parent_instance;

    InsensitiveController *controller;
    unsigned int        numberOfRows;

    GtkToolButton       *addOperator_button;
    GtkListBox          *composer_listbox;
    GtkTextView         *productOperatorComposer_textview;
    GtkTextBuffer       *productOperatorComposer_textbuffer;
};


G_DEFINE_TYPE(InsensitiveComposer, insensitive_composer, GTK_TYPE_WINDOW)


InsensitiveComposer* insensitive_composer_new()
{
	return (InsensitiveComposer *)g_object_new(INSENSITIVE_TYPE_COMPOSER, NULL);
}


static void insensitive_composer_class_init(InsensitiveComposerClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-composer.ui");

	gtk_widget_class_bind_template_child(widget_class, InsensitiveComposer, addOperator_button);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveComposer, composer_listbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveComposer, productOperatorComposer_textview);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveComposer, productOperatorComposer_textbuffer);
}


static void insensitive_composer_dispose(GObject *gobject)
{
	InsensitiveComposer *self = (InsensitiveComposer *)gobject;

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

	G_OBJECT_CLASS(insensitive_composer_parent_class)->dispose(gobject);
}


static void insensitive_composer_finalize(GObject *gobject)
{
	InsensitiveComposer *self = (InsensitiveComposer *)gobject;

	G_OBJECT_CLASS(insensitive_composer_parent_class)->finalize(gobject);
}


static void insensitive_composer_init(InsensitiveComposer *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
    self->numberOfRows = 0;
}


void on_InsensitiveComposer_destroy(InsensitiveComposer *self, gpointer user_data)
{
}


void insensitive_composer_set_controller(InsensitiveComposer *self, gpointer newController)
{
    self->controller = (InsensitiveController *)newController;
    insensitive_composer_reset(self);
}


void insensitive_composer_reset(InsensitiveComposer *self)
{
    if (self) {
        gtk_container_foreach((GtkContainer *)self->composer_listbox, (GtkCallback)gtk_widget_destroy, NULL);
        self->numberOfRows = 0;
		insensitive_composer_initialize(self);
	}
}


void insensitive_composer_initialize(InsensitiveComposer *self)
{
    insensitive_composer_add_operator(NULL, self);
    gtk_text_buffer_set_text(self->productOperatorComposer_textbuffer, "E/2", 3);
}


void insensitive_composer_add_operator(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveComposer *self = (InsensitiveComposer *)user_data;
    InsensitiveComposerRow *row = insensitive_composer_row_new(self, self->controller->spinSystem);

    gtk_widget_show(GTK_WIDGET(row));
    gtk_container_add(GTK_CONTAINER(self->composer_listbox), GTK_WIDGET(row));
    self->numberOfRows++;

    // Disable add operator button if number of operators exceeds 4^spins
    gtk_widget_set_sensitive((GtkWidget *)self->addOperator_button,
                             (self->numberOfRows <= pow(4, self->controller->spinSystem->spins)));
	// Enable first trash button after next one was added
    row = (InsensitiveComposerRow *)gtk_list_box_get_row_at_index(self->composer_listbox, 0);
    gtk_widget_set_sensitive((GtkWidget *)row->delete_button, self->numberOfRows > 1);
}


void insensitive_composer_delete_operator(InsensitiveComposer *self, InsensitiveComposerRow *row)
{
    InsensitiveComposerRow *first_row;

    gtk_widget_destroy((GtkWidget *)row);
    self->numberOfRows--;
    insensitive_composer_update_productOperator_string(self);

    // Enable add operator button
	gtk_widget_set_sensitive((GtkWidget *)self->addOperator_button, TRUE);
	// Disable first trash button if only one operator is present
	first_row = (InsensitiveComposerRow *)gtk_list_box_get_row_at_index(self->composer_listbox, 0);
    gtk_widget_set_sensitive((GtkWidget *)first_row->delete_button, self->numberOfRows > 1);
}


void insensitive_composer_update_productOperator_string(InsensitiveComposer *self)
{
    InsensitiveComposerRow *row;
	GString *productOperatorString = g_string_new("");
    gchar *str;
    unsigned int i;

    for (i = 0; i < self->numberOfRows; i++) {
        row = (InsensitiveComposerRow *)gtk_list_box_get_row_at_index(self->composer_listbox, i);
        str = product_operator_from_base4(row->codedProductOperator,
                                          row->numberOfSpins,
                                          row->spinTypeArray,
                                          row->coefficientForProductOperator);
        g_string_append(productOperatorString, str);
        g_free(str);
    }

	// Remove unnecessary leading sign
	if(productOperatorString->str[1] == '+')
        gtk_text_buffer_set_text(self->productOperatorComposer_textbuffer,
                                 productOperatorString->str + 3,
                                 productOperatorString->len - 3);
	else
		gtk_text_buffer_set_text(self->productOperatorComposer_textbuffer,
                                 productOperatorString->str + 1,
                                 productOperatorString->len - 1);
    g_string_free(productOperatorString, TRUE);
}


void on_updateMatrix_button_clicked(GtkToolButton *toolbutton, gpointer user_data)
{
    InsensitiveComposer *self = (InsensitiveComposer *)user_data;
    InsensitiveComposerRow *row;
	unsigned int i;
    int *base4Index;
	float *coefficient;
	gboolean matrixIsIdentity = TRUE;

	base4Index = malloc(self->numberOfRows * sizeof(int));
	coefficient = malloc(self->numberOfRows * sizeof(float));

	for (i = 0; i < self->numberOfRows; i++) {
        row = (InsensitiveComposerRow *)gtk_list_box_get_row_at_index(self->composer_listbox, i);
		base4Index[i] = row->codedProductOperator;
		coefficient[i] = row->coefficientForProductOperator;
		if((base4Index[i] != 0) && (coefficient[i] != 0))
			matrixIsIdentity = FALSE;
	}

	if (!matrixIsIdentity)
        insensitive_controller_update_matrix_with(self->controller, self->numberOfRows, base4Index, coefficient);
	else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(self),
					                               GTK_DIALOG_DESTROY_WITH_PARENT,
					                               GTK_MESSAGE_ERROR,
					                               GTK_BUTTONS_OK,
					                               "The matrix consists only of the identity operator.\nCreate at least one operator with finite magnetisation.");
	    gtk_window_set_title(GTK_WINDOW(dialog), "Composer cannot update density matrix");
	    gtk_dialog_run(GTK_DIALOG(dialog));
	    gtk_widget_destroy(dialog);
    }

    free(base4Index);
    free(coefficient);
}
