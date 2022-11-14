/* insensitive-singlespins.c
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
#include "insensitive-singlespins.h"
#include "insensitive-library.h"

#define X_RF_CONE_ROTATION -M_PI_4 / 4


struct _InsensitiveSingleSpins {
	GtkWindow           parent_instance;

	GtkDrawingArea      *single_spins_drawingarea;
    GtkToggleButton     *B0_checkbox, *B1_checkbox, *rotatingFrame_checkbox, *ensembleVector_checkbox;
    GtkScale            *T1_slider, *T2_slider, *numberOfSpins_slider, *temperature_slider;
    GtkAdjustment       *T1_adjustment, *T2_adjustment, *numberOfSpins_adjustment, *temperature_adjustment;
    float phase;

    // Objects for the spins
    enum DefinedCoordinate fieldAxis;
    GPtrArray *spinSet;
    unsigned int numberOfSpins;
    guint spinEvolutionTimerNr;
    gboolean realQuantumProbability;
    gboolean timer_is_running;
};


G_DEFINE_TYPE(InsensitiveSingleSpins, insensitive_single_spins, GTK_TYPE_WINDOW)


static void insensitive_single_spins_class_init(InsensitiveSingleSpinsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/klausboldt/insensitive/insensitive-singlespins.ui");

	gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, single_spins_drawingarea);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, B0_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, B1_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, rotatingFrame_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, ensembleVector_checkbox);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, T1_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, T2_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, numberOfSpins_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, temperature_slider);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, T1_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, T2_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, numberOfSpins_adjustment);
    gtk_widget_class_bind_template_child(widget_class, InsensitiveSingleSpins, temperature_adjustment);
}


static void insensitive_single_spins_dispose(GObject *gobject)
{
	InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)gobject;

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

    self->timer_is_running = FALSE;
	g_source_remove(self->spinEvolutionTimerNr);

	G_OBJECT_CLASS(insensitive_single_spins_parent_class)->dispose(gobject);
}


static void insensitive_single_spins_finalize(GObject *gobject)
{
	InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)gobject;

    g_ptr_array_remove_range(self->spinSet, 0, self->spinSet->len);

	G_OBJECT_CLASS(insensitive_single_spins_parent_class)->finalize(gobject);
}


static void insensitive_single_spins_init(InsensitiveSingleSpins *self)
{
    unsigned int i;
    time_t t;

    gtk_widget_init_template(GTK_WIDGET(self));

    self->phase = 0.0;
    self->numberOfSpins = totalNumberOfSingleSpins;

    srand((unsigned)time(&t) + (unsigned long)&self->numberOfSpins - (unsigned long)&self->phase);
    self->spinSet = g_ptr_array_sized_new(self->numberOfSpins);
    for (i = 0; i < self->numberOfSpins; i++) {
        SingleSpinVector *vector = malloc(sizeof(SingleSpinVector));
        g_ptr_array_add(self->spinSet, vector);
    }
    self->fieldAxis = Zfield;
    self->realQuantumProbability = FALSE;
    create_zeeman_state(NULL, self);

    self->timer_is_running = TRUE;
    self->spinEvolutionTimerNr = g_timeout_add(30, spinEvolutionTimerEvent, self);
}


void on_InsensitiveSingleSpins_destroy(InsensitiveSingleSpins *window, gpointer user_data)
{
    window->timer_is_running = FALSE;
	g_source_remove(window->spinEvolutionTimerNr);
}


void on_B0_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;

    if (gtk_toggle_button_get_active(checkbox)) {
        self->fieldAxis = Zfield;
        gtk_toggle_button_set_active(self->B1_checkbox, FALSE);
    } else {
        if(!gtk_toggle_button_get_active(self->B1_checkbox))
            self->fieldAxis = NoField;
    }
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);
}


void on_B1_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;

    if (gtk_toggle_button_get_active(checkbox)) {
        if (gtk_toggle_button_get_active(self->rotatingFrame_checkbox)) {
            self->fieldAxis = Xfield;
            self->phase = X_RF_CONE_ROTATION;
        } else
            self->fieldAxis = XYRotatingField;
        gtk_toggle_button_set_active(self->B0_checkbox, FALSE);
    } else {
        if(!gtk_toggle_button_get_active(self->B0_checkbox))
            self->fieldAxis = NoField;
    }
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);
}


void on_rotatingFrame_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;

    if (!gtk_toggle_button_get_active(checkbox)) {
        if(gtk_toggle_button_get_active(self->B1_checkbox))
            self->fieldAxis = XYRotatingField;
        else if (gtk_toggle_button_get_active(self->B0_checkbox))
            self->fieldAxis = Zfield;
        else
            self->fieldAxis = NoField;
    } else {
        if (gtk_toggle_button_get_active(self->B1_checkbox)) {
            self->fieldAxis = Xfield;
            self->phase = X_RF_CONE_ROTATION;
        } else if (gtk_toggle_button_get_active(self->B0_checkbox)) {
            self->fieldAxis = Zfield;
        } else {
            self->fieldAxis = NoField;
        }
    }
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);
}


gboolean spinEvolutionTimerEvent(gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;
    unsigned int i, spin;
    float random, kT;
    float phaseShift = 0.06283;
    double re1, im1, re2, im2, cos_phi_alpha, sin_phi_alpha, cos_phi_beta, sin_phi_beta;
    float flipAngle = M_PI / 720;
    float cos_fa = cos(flipAngle);
    float sin_fa = sin(flipAngle);
    SingleSpinVector *v;
    gboolean B0 = gtk_toggle_button_get_active(self->B0_checkbox);
    gboolean B1 = gtk_toggle_button_get_active(self->B1_checkbox);

    // Rotating Frame
    if (!gtk_toggle_button_get_active(self->rotatingFrame_checkbox)) {
        if (B0 || B1) {
            self->phase -= phaseShift;
            if (self->phase < 0.0)
                self->phase += 2 * M_PI;
        }
    } else {
        if (!B0 && !B1) {
            self->phase += phaseShift;
            if (self->phase >= 2 * M_PI)
                self->phase -= 2 * M_PI;
        }
    }
    // Spin Evolution
    for (spin = 0; spin < self->numberOfSpins; spin++) {
        v = g_ptr_array_index(self->spinSet, spin);
        if (B1) {
			/* B₁ field is applied */
    		cos_phi_alpha = cos(v->phase_alpha);
    		sin_phi_alpha = sin(v->phase_alpha);
    		cos_phi_beta = cos(v->phase_beta);
    		sin_phi_beta = sin(v->phase_beta);
            re1 = v->A_alpha * cos_fa * cos_phi_alpha + v->A_beta * sin_fa * sin_phi_beta;
            im1 = v->A_alpha * cos_fa * sin_phi_alpha - v->A_beta * sin_fa * cos_phi_beta;
            re2 = v->A_alpha * sin_fa * sin_phi_alpha + v->A_beta * cos_fa * cos_phi_beta;
            im2 = -v->A_alpha * sin_fa * cos_phi_alpha + v->A_beta * cos_fa * sin_phi_beta;
            v->A_alpha = sqrt(pow(re1, 2) + pow(im1, 2));
            v->phase_alpha = atan2(im1, re1);
            if (v->phase_alpha >= 2 * M_PI)
                v->phase_alpha -= 2 * M_PI;
            if (v->phase_alpha < 0.0)
                v->phase_alpha += 2 * M_PI;
            v->A_beta = sqrt(pow(re2, 2) + pow(im2, 2));
            v->phase_beta = atan2(im2, re2);
            if (v->phase_beta >= 2 * M_PI)
                v->phase_beta -= 2 * M_PI;
            if (v->phase_beta < 0.0)
                v->phase_beta += 2 * M_PI;
        } else {
			/* B₀ field or no field is applied */
            double temp;
			for (i = 0; i < gtk_adjustment_get_value(self->T1_adjustment); i++) {
				/* Randomise populations (0 <= θ <= π), consider temperature for equilibrium state */
                random = (float)rand() / (float)RAND_MAX;
                if (!B0 && !B1)
                    kT = 0.0;
                else
                    kT = 0.4 * gtk_adjustment_get_value(self->temperature_adjustment);
                if (random > 0.5 + kT) {
                    temp = pow(v->A_alpha, 2);
                    temp += 0.01;
                    v->A_alpha = (temp >= 1.0) ? 1.0 : sqrt(temp);
                    v->A_beta = (temp >= 1.0) ? 0.0 : sqrt(1 - temp);
                } else if (random < 0.5 + kT) {
                    temp = pow(v->A_alpha, 2);
                    temp -= 0.01;
                    v->A_alpha = (temp <= 0.0) ? 0.0 : sqrt(temp);
                    v->A_beta = (temp <= 0.0) ? 1.0 : sqrt(1 - temp);
                }
            }
            for (i = 0; i < gtk_adjustment_get_value(self->T2_adjustment); i++) {
				/* Randomise phase relationship (-π/2 <= Δϕ <= π/2) */
                random = (0.5 - (float)rand() / (float)RAND_MAX) * phaseShift * 0.5;
                v->phase_alpha += random;
				v->phase_beta -= random;
				if (fabs(v->phase_alpha - v->phase_beta) > M_PI) {
					v->phase_alpha -= M_PI_2;
					v->phase_beta -= M_PI_2;
				}
            }
        }
    }
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);

    return self->timer_is_running;
}


void on_temperature_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
	InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;
}


void on_ensembleVector_checkbox_toggled(GtkToggleButton *checkbox, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;
}


void on_numberOfSpins_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;

    self->numberOfSpins = (unsigned int)gtk_adjustment_get_value(adjustment);
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);
}


void create_zeeman_state(GtkButton *button, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;
    unsigned int i;
    double phi, random, temp;

    for (i = 0; i < self->numberOfSpins; i++) {
        SingleSpinVector *v = g_ptr_array_index(self->spinSet, i);
        v->A_alpha = 1.0;
        v->A_beta = 0.0;
		phi = 0;
		v->phase_alpha = phi;
        v->phase_beta = phi;
		/* Randomise populations (0 <= θ <= π), but retain α state majority */
        random = (float)rand() / (float)RAND_MAX;
		random = (2 * M_PI * random - M_PI) * (0.45 + 0.75 * gtk_adjustment_get_value(self->temperature_adjustment));
		/*
		 * How to calculate the coefficients for the temperature parameter (kT):
		 *
		 * self->temperature_adjustment has values from -0.50 ... 0.0
		 * smallest (coldest) sensible value for kT is 0.025, highest according to taste (e.g. 0.025 ... 0.45)
		 * subtract highest value (0.45) on both sides to yield 0.0 for the highest value: -0.375 ... 0.0
		 * then divide by -0.50 (lower boundary of the adjustment): 0.75 ... 0.0
		 * Use left hand value for multiplicator and chosen highest value as summand: 0.45 + 0.75 * gtk_...
		 *
		 * For a sharper focus (max kT fills half the sphere with vectors) use: 0.25 + 0.45 * gtk_...
		 *
		 */
		temp = v->A_alpha;
		v->A_alpha = v->A_alpha * cos(random) - v->A_beta * sin(random);
		v->A_beta = v->A_beta * cos(random) + temp * sin(random);
		/* Randomise phase relationship (-π/2 <= Δϕ <= π/2) */
		random = ((float)rand() / (float)RAND_MAX * M_PI) - M_PI_2;
		v->phase_alpha += random;
		v->phase_beta -= random;
		if (fabs(v->phase_alpha - v->phase_beta) > M_PI) {
			v->phase_alpha -= M_PI_2;
			v->phase_beta -= M_PI_2;
		}
    }
    if(gtk_toggle_button_get_active(self->B1_checkbox) && gtk_toggle_button_get_active(self->rotatingFrame_checkbox))
        self->phase = X_RF_CONE_ROTATION;
    else
        self->phase = 0.0;
    gtk_widget_queue_draw((GtkWidget *)self->single_spins_drawingarea);
}


void create_superposition_state(GtkButton *button, gpointer user_data)
{
    InsensitiveSingleSpins *self = (InsensitiveSingleSpins *)user_data;
    unsigned int i;
    double re1, im1, re2, im2, cos_phi_alpha, sin_phi_alpha, cos_phi_beta, sin_phi_beta;

    create_zeeman_state(NULL, self);
    for (i = 0; i < self->numberOfSpins; i++) {
        SingleSpinVector *v = g_ptr_array_index(self->spinSet, i);
        cos_phi_alpha = cos(v->phase_alpha);
        sin_phi_alpha = sin(v->phase_alpha);
        cos_phi_beta = cos(v->phase_beta);
        sin_phi_beta = sin(v->phase_beta);
        re1 = v->A_alpha * M_SQRT1_2 * cos_phi_alpha + v->A_beta * M_SQRT1_2 * sin_phi_beta;
        im1 = v->A_alpha * M_SQRT1_2 * sin_phi_alpha - v->A_beta * M_SQRT1_2 * cos_phi_beta;
        re2 = v->A_alpha * M_SQRT1_2 * sin_phi_alpha + v->A_beta * M_SQRT1_2 * cos_phi_beta;
        im2 = -v->A_alpha * M_SQRT1_2 * cos_phi_alpha + v->A_beta * M_SQRT1_2 * sin_phi_beta;
        v->A_alpha = sqrt(pow(re1, 2) + pow(im1, 2));
        v->phase_alpha = atan2(im1, re1);
        /*if(v->phase_alpha < -M_PI)
            v->phase_alpha += 2 * M_PI;
        if(v->phase_alpha > M_PI)
            v->phase_alpha -= 2 * M_PI;*/
        v->A_beta = sqrt(pow(re2, 2) + pow(im2, 2));
        v->phase_beta = atan2(im2, re2);
        /*if(v->phase_beta < -M_PI)
            v->phase_beta += 2 * M_PI;
        if(v->phase_beta > M_PI)
            v->phase_beta -= 2 * M_PI;*/
    }
}


/*
- (void)toggleSingleSpinEvolutionTimer:(BOOL)newState
{
    if(newState == YES) {
        spinEvolutionTimer = [[NSTimer scheduledTimerWithTimeInterval:0.03
                                                               target:self
                                                             selector:@selector(spinEvolutionTimerEvent:)
                                                             userInfo:nil
                                                              repeats:YES] retain];
    } else {
        [spinEvolutionTimer invalidate];
        [spinEvolutionTimer release];
        spinEvolutionTimer = nil;
    }
}


- (void)windowDidChangeOcclusionState:(NSNotification *)notification
{
    if(self.window.occlusionState & NSWindowOcclusionStateVisible)
        [self toggleSingleSpinEvolutionTimer:YES];
    else
        [self toggleSingleSpinEvolutionTimer:NO];
}
*/


void draw_single_spins_view(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	InsensitiveSingleSpins *window = (InsensitiveSingleSpins *)user_data;
	unsigned int i;
	float centre_x, centre_y, halfWidth, circleWidth;
	const float coneAngle = 0.8410742;
	double random;
	double x1, x2, y1, y2;
	double precessionAngle, x, temp, max_width, max_shift, perspective;
	double state, angle;
	double a_alpha, b_alpha, a_beta, b_beta, x_amplitude, y_amplitude, z_amplitude;
	double sum_x, sum_y, sum_z, sum_angle;
	DSPComplex *operator_Ix, *operator_Iy, *operator_Iz, *matrix, z;
	SingleSpinVector *v;
	int counter = 0;
	float bottom, top, left, right, proj_x, proj_y, arrowangle;
	float width, height, origin_x, origin_y, y, circle_width;
    cairo_text_extents_t extents;
    gchar *label;
	gboolean B0 = gtk_toggle_button_get_active(window->B0_checkbox);
	gboolean B1 = gtk_toggle_button_get_active(window->B1_checkbox);
	gboolean rotatingFrame = gtk_toggle_button_get_active(window->rotatingFrame_checkbox);
	gboolean ensembleVector = gtk_toggle_button_get_active(window->ensembleVector_checkbox);

	cairo_set_line_width(cr, 3.0);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_select_font_face(cr, "Helvetica Neue", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 13);
    label = malloc(5 * sizeof(gchar));
	// z axis
	if (B0 || (B1 && !rotatingFrame)) {
		origin_x = 7.0;
		origin_y = 7.0;
		width = 31.0;
		height = gtk_widget_get_allocated_height(widget) - 60.0 - origin_y;
		centre_x = origin_x + width / 2;
		centre_y = origin_y + height / 2;
		top = origin_y + 25.0;
		bottom = origin_y + height - 25.0;
		left = origin_x + 25.0;
		right = origin_x + width - 25.0;
		cairo_move_to(cr, centre_x, bottom);
		cairo_line_to(cr, centre_x, top);
		cairo_line_to(cr, centre_x - 5, top + 7);
		cairo_move_to(cr, centre_x, top);
		cairo_line_to(cr, centre_x + 5, top + 7);
        if (rotatingFrame) {
            strcpy(label, "ΔB");
            cairo_text_extents (cr, label, &extents);
            cairo_move_to(cr, centre_x - extents.width / 2, 20.0);
		    cairo_show_text(cr, label);
        } else {
            strcpy(label, "B₀");
            cairo_text_extents (cr, label, &extents);
            cairo_move_to(cr, centre_x - extents.width / 2, 20.0);
		    cairo_show_text(cr, label);
        }
		cairo_stroke(cr);
	}
	// x axis and x,y plane
    if (B1) {
        origin_x = 41.0;
	    height = 71.0;
        origin_y = gtk_widget_get_allocated_height(widget) - height;
	    width = gtk_widget_get_allocated_width(widget) - 2 * origin_x; //gtk_widget_get_allocated_width(widget) - origin_x - 7.0;
        centre_x = origin_x + width / 2;
	    centre_y = origin_y + height / 2;
        top = origin_y + 25.0;
	    bottom = origin_y + height - 25.0;
	    left = origin_x + 25.0;
	    right = origin_x + width - 25.0;
	    if (rotatingFrame) {
		    cairo_move_to(cr, centre_x, centre_y);
		    cairo_line_to(cr, left, centre_y);
		    cairo_line_to(cr, left + 9, centre_y + 3);
		    cairo_move_to(cr, left, centre_y);
		    cairo_line_to(cr, left + 9, centre_y - 1);
	    }
	    if (!rotatingFrame) {
		    arrowangle = 0.045;
		    proj_x = (centre_x - left) * cos(window->phase);
		    proj_y = height / 2 * sin(window->phase);
		    cairo_move_to(cr, centre_x, centre_y);
		    cairo_line_to(cr, centre_x - proj_x, centre_y - proj_y);
		    cairo_line_to(cr,
		    	          centre_x - (centre_x - left - 8.6) * cos(window->phase - arrowangle),
		    	          centre_y - (height / 2 - 4) * sin(window->phase - arrowangle));
		    cairo_move_to(cr, centre_x - proj_x, centre_y - proj_y);
		    cairo_line_to(cr,
			              centre_x - (centre_x - left - 8.6) * cos(window->phase + arrowangle),
			              centre_y - (height / 2 - 4) * sin(window->phase + arrowangle));
	    }
        cairo_move_to(cr, origin_x, centre_y);
		cairo_show_text(cr, "B₁");
        cairo_stroke(cr);
    }
    free(label);
	// spin vectors
	origin_x = 41.0;
	origin_y = 7.0;
	width = gtk_widget_get_allocated_width(widget) - 2 * origin_x;
	height = gtk_widget_get_allocated_height(widget) - origin_y - 71.0;
	centre_x = origin_x + width / 2;
	centre_y = origin_y + height / 2;
	halfWidth = (width < height) ? centre_x : centre_y;
	circleWidth = 8.0;
	if (ensembleVector) {
		sum_x = 0.0;
		sum_y = 0.0;
		sum_z = 0.0;
		operator_Ix = Ix(0, 1);
		operator_Iy = Iy(0, 1);
		operator_Iz = Iz(0, 1);
		matrix = malloc(4 * sizeof(DSPComplex));
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			a_alpha = cos(v->phase_alpha + window->phase / 2) * v->A_alpha;
			b_alpha = sin(v->phase_alpha + window->phase / 2) * v->A_alpha;
			a_beta = cos(v->phase_beta - window->phase / 2) * v->A_beta;
			b_beta = sin(v->phase_beta - window->phase / 2) * v->A_beta;
			matrix[0] = complex_rect(a_alpha * a_alpha + b_alpha * b_alpha, 0.0);
			matrix[1] = complex_rect(a_alpha * a_beta + b_alpha * b_beta, b_alpha * a_beta - a_alpha * b_beta);
			matrix[2] = complex_rect(a_alpha * a_beta + b_alpha * b_beta, a_alpha * b_beta - b_alpha * a_beta);
			matrix[3] = complex_rect(a_beta * a_beta + b_beta * b_beta, 0.0);
			z = expectation_value(matrix, operator_Ix, 2);
			sum_x += z.real;
			z = expectation_value(matrix, operator_Iy, 2);
			sum_y += z.real;
			z = expectation_value(matrix, operator_Iz, 2);
			sum_z += z.real;
		}
		free(operator_Ix);
		free(operator_Iy);
		free(operator_Iz);
		free(matrix);
		sum_angle = acos(sum_z / sqrt(pow(sum_x, 2) + pow(sum_y, 2) + pow(sum_z, 2)));
		counter = 0;
	}

	// Draw sum of all vectors in back
	if (ensembleVector && sum_y < 0.0 && sum_angle >= coneAngle && sum_angle <= M_PI - coneAngle) {
		temp = 0.9 * halfWidth / (float)window->numberOfSpins;
		cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
		cairo_set_line_width(cr, 3.0);
		cairo_move_to(cr, centre_x, centre_y);
		cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
		cairo_stroke(cr);
	}
	cairo_set_line_width(cr, 1);
	cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
	switch (window->fieldAxis) {
	case NoField:
		if (ensembleVector && (sum_angle < coneAngle || sum_angle > M_PI - coneAngle || sum_y > 0.0)) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		cairo_set_line_width(cr, 1.0);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		halfWidth *= M_SQRT2;
        cairo_arc(cr, centre_x, centre_y, halfWidth * cos(coneAngle), 0.0, 2 * M_PI);
		cairo_stroke(cr);
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		operator_Ix = Ix(0, 1);
		operator_Iz = Iz(0, 1);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			DSPComplex *matrix, z;
			matrix = malloc(4 * sizeof(DSPComplex));
			a_alpha = cos(v->phase_alpha + window->phase / 2) * v->A_alpha;
			b_alpha = sin(v->phase_alpha + window->phase / 2) * v->A_alpha;
			a_beta = cos(v->phase_beta - window->phase / 2) * v->A_beta;
			b_beta = sin(v->phase_beta - window->phase / 2) * v->A_beta;
			matrix[0] = complex_rect(a_alpha * a_alpha + b_alpha * b_alpha, 0.0);
			matrix[1] = complex_rect(a_alpha * a_beta + b_alpha * b_beta, b_alpha * a_beta - a_alpha * b_beta);
			matrix[2] = complex_rect(a_alpha * a_beta + b_alpha * b_beta, a_alpha * b_beta - b_alpha * a_beta);
			matrix[3] = complex_rect(a_beta * a_beta + b_beta * b_beta, 0.0);
			z = expectation_value(matrix, operator_Ix, 2);
			x1 = z.real * halfWidth * cos(coneAngle) * 2.0;
			z = expectation_value(matrix, operator_Iz, 2);
			y1 = z.real * halfWidth * cos(coneAngle) * 2.0;
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + x1, centre_y - y1);
		}
		cairo_stroke(cr);
		free(operator_Ix);
		free(operator_Iz);
		break;
	case Zfield:
		// Hind half
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
		x2 = -halfWidth * cos(coneAngle);
		y2 = -halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = -halfWidth * sin(coneAngle) + x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = halfWidth * cos(coneAngle);
		y2 = -halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = -halfWidth * sin(coneAngle) + x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = -halfWidth * cos(coneAngle);
		y2 = halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = halfWidth * sin(coneAngle) + x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = halfWidth * cos(coneAngle);
		y2 = halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = halfWidth * sin(coneAngle) + x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		cairo_stroke(cr);
		// Draw cone
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_move_to(cr, centre_x - halfWidth * cos(coneAngle), centre_y + halfWidth * sin(coneAngle));
		cairo_line_to(cr, centre_x + halfWidth * cos(coneAngle), centre_y - halfWidth * sin(coneAngle));
		cairo_move_to(cr, centre_x - halfWidth * cos(coneAngle), centre_y - halfWidth * sin(coneAngle));
		cairo_line_to(cr, centre_x + halfWidth * cos(coneAngle), centre_y + halfWidth * sin(coneAngle));
		cairo_stroke(cr);
		// Draw hind vectors
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			if (window->realQuantumProbability) {
				random = (float)rand() / (float)RAND_MAX;
				if (random < pow(v->A_alpha, 2)) {
					state = 1.0;    // draw upward
				} else if (random < pow(v->A_alpha, 2)) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			} else {
				if (v->A_alpha > M_SQRT1_2) {
					state = 1.0;    // draw upward
				} else if (v->A_alpha < M_SQRT1_2) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			}
			angle = (v->phase_alpha - v->phase_beta) - window->phase + M_PI_2;
			while (angle < 0.0) angle += 2 * M_PI;
			while (angle > 2 * M_PI) angle -= 2 * M_PI;
			if (angle < M_PI_2 || angle > 3 * M_PI_2) {
				cairo_move_to(cr, centre_x, centre_y);
				cairo_line_to(cr, centre_x - halfWidth * cos(coneAngle) * sin(angle),
					      centre_y - (halfWidth * sin(coneAngle) + state * cos(state * angle) * circleWidth) * state);
			}
		}
		cairo_stroke(cr);
		// Draw sum of all vectors in front
		if (ensembleVector && (sum_angle < coneAngle || sum_angle > M_PI - coneAngle)) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		// Draw front vectors
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		cairo_set_line_width(cr, 1.0);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			if (window->realQuantumProbability) {
				random = (float)rand() / (float)RAND_MAX;
				if (random < pow(v->A_alpha, 2)) {
					state = 1.0;    // draw upward
				} else if (random < pow(v->A_alpha, 2)) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			} else {
				if (v->A_alpha > M_SQRT1_2) {
					state = 1.0;    // draw upward
				} else if (v->A_alpha < M_SQRT1_2) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			}
			angle = (v->phase_alpha - v->phase_beta) - window->phase + M_PI_2;
			while (angle < 0.0) angle += 2 * M_PI;
			while (angle > 2 * M_PI) angle -= 2 * M_PI;
			if (angle >= M_PI_2 && angle <= 3 * M_PI_2) {
				cairo_move_to(cr, centre_x, centre_y);
				cairo_line_to(cr, centre_x - halfWidth * cos(coneAngle) * sin(angle),
					      centre_y - (halfWidth * sin(coneAngle) + state * cos(-state * angle) * circleWidth) * state);
			}
		}
		cairo_stroke(cr);
		// Front half
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		x2 = -halfWidth * cos(coneAngle);
		y2 = -halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = -halfWidth * sin(coneAngle) - x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = halfWidth * cos(coneAngle);
		y2 = -halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = -halfWidth * sin(coneAngle) - x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = -halfWidth * cos(coneAngle);
		y2 = halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = halfWidth * sin(coneAngle) - x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x2 = halfWidth * cos(coneAngle);
		y2 = halfWidth * sin(coneAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x2 = temp * halfWidth * cos(coneAngle);
			y2 = halfWidth * sin(coneAngle) - x * circleWidth;
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		cairo_stroke(cr);
		// Draw sum of all vectors in front
		if (ensembleVector && sum_y > 0.0 && sum_angle >= coneAngle && sum_angle <= M_PI - coneAngle) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		break;
	case Xfield:
	case Yfield:
		// Draw rotated cone around z
		perspective = window->phase;
		max_width = 2 * halfWidth * cos(coneAngle);
		max_shift = halfWidth * sin(coneAngle);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		cairo_move_to(cr, centre_x, centre_y);
		cairo_line_to(cr, centre_x - halfWidth * sin(coneAngle) * cos(perspective), centre_y - halfWidth * cos(coneAngle));
		cairo_move_to(cr, centre_x, centre_y);
		cairo_line_to(cr, centre_x - halfWidth * sin(coneAngle) * cos(perspective), centre_y + halfWidth * cos(coneAngle));
		cairo_stroke(cr);
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
        // Draw hind circle
        cairo_save(cr);
        cairo_translate(cr, centre_x + halfWidth * sin(coneAngle) * cos(perspective), centre_y);
        cairo_scale(cr, max_width * sin(perspective) / 2.0, max_width / 2.0);
        cairo_arc(cr, 0.0, 0.0, 1.0, 3 * M_PI_2, M_PI_2);
        cairo_restore(cr);
        cairo_stroke(cr);
		cairo_save(cr);
        cairo_translate(cr, centre_x - halfWidth * sin(coneAngle) * cos(perspective), centre_y);
        cairo_scale(cr, max_width * sin(perspective) / 2.0, max_width / 2.0);
        cairo_arc(cr, 0.0, 0.0, 1.0, 3 * M_PI_2, M_PI_2);
        cairo_restore(cr);
        cairo_stroke(cr);
        // Draw hind cone
		cairo_move_to(cr, centre_x + halfWidth * sin(coneAngle) * cos(perspective), centre_y + halfWidth * cos(coneAngle));
		cairo_line_to(cr, centre_x, centre_y);
		cairo_move_to(cr, centre_x + halfWidth * sin(coneAngle) * cos(perspective), centre_y - halfWidth * cos(coneAngle));
		cairo_line_to(cr, centre_x, centre_y);
		cairo_stroke(cr);
		// Draw hind vectors
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			x_amplitude = cos(v->phase_alpha - v->phase_beta);
			y_amplitude = sin(v->phase_alpha - v->phase_beta);
			if (y_amplitude < 0) {
				if (window->realQuantumProbability) {
					random = 1 - 2 * (float)rand() / (float)RAND_MAX;
					if (random < x_amplitude) {
						state = 1.0;    // draw upward
					} else if (random < x_amplitude) {
						state = -1.0;   // draw downward
					} else {
						random = (float)rand() / (float)RAND_MAX;
						state = (random > 0.5) ? 1.0 : -1.0;
					}
				} else {
					if (x_amplitude > 0.0) {
						state = 1.0;    // draw left
					} else if (x_amplitude < 0.0) {
						state = -1.0;   // draw right
					} else {
						random = (float)rand() / (float)RAND_MAX;
						state = (random > 0.5) ? 1.0 : -1.0;
					}
				}
				angle = 2 * acos(v->A_alpha);
				z_amplitude = cos(angle);
				x2 = halfWidth * sin(coneAngle) * cos(perspective) * state;
				x2 += 0.5 * max_width * sin(perspective) * sin(angle);
				if ((sin(v->phase_alpha - v->phase_beta) > 0.0 && v->A_alpha > M_SQRT1_2) ||
					(sin(v->phase_alpha - v->phase_beta) > 0.0 && v->A_alpha <= M_SQRT1_2))
					x2 *= -1.0;
				y2 = halfWidth * cos(coneAngle) * z_amplitude;
				cairo_move_to(cr, centre_x, centre_y);
				cairo_line_to(cr, centre_x + x2, centre_y - y2);
			}
		}
		cairo_stroke(cr);
		// Draw front vectors
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			x_amplitude = cos(v->phase_alpha - v->phase_beta);
			y_amplitude = sin(v->phase_alpha - v->phase_beta);
			if (y_amplitude > 0) {
				if (window->realQuantumProbability) {
					random = 1 - 2 * (float)rand() / (float)RAND_MAX;
					if (random < x_amplitude) {
						state = 1.0;    // draw upward
					} else if (random < x_amplitude) {
						state = -1.0;   // draw downward
					} else {
						random = (float)rand() / (float)RAND_MAX;
						state = (random > 0.5) ? 1.0 : -1.0;
					}
				} else {
					if (x_amplitude > 0.0) {
						state = 1.0;    // draw left
					} else if (x_amplitude < 0.0) {
						state = -1.0;   // draw right
					} else {
						random = (float)rand() / (float)RAND_MAX;
						state = (random > 0.5) ? 1.0 : -1.0;
					}
				}
				angle = 2 * acos(v->A_alpha);
				z_amplitude = cos(angle);
				x2 = halfWidth * sin(coneAngle) * cos(perspective) * state;
				x2 += 0.5 * max_width * sin(perspective) * sin(angle);
				if ((sin(v->phase_alpha - v->phase_beta) > 0.0 && v->A_alpha > M_SQRT1_2) ||
					(sin(v->phase_alpha - v->phase_beta) > 0.0 && v->A_alpha <= M_SQRT1_2))
					x2 *= -1.0;
				y2 = halfWidth * cos(coneAngle) * z_amplitude;
				cairo_move_to(cr, centre_x, centre_y);
				cairo_line_to(cr, centre_x + x2, centre_y - y2);
			}
		}
		cairo_stroke(cr);
		// Draw top of rotated cone around z
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
        // Draw hind circle
        cairo_save(cr);
        cairo_translate(cr, centre_x - halfWidth * sin(coneAngle) * cos(perspective), centre_y);
        cairo_scale(cr, max_width * sin(perspective) / 2.0, max_width / 2.0);
        cairo_arc(cr, 0.0, 0.0, 1.0, M_PI_2, 3 * M_PI_2);
        cairo_restore(cr);
        cairo_stroke(cr);
		cairo_save(cr);
        cairo_translate(cr, centre_x + halfWidth * sin(coneAngle) * cos(perspective), centre_y);
        cairo_scale(cr, max_width * sin(perspective) / 2.0, max_width / 2.0);
        cairo_arc(cr, 0.0, 0.0, 1.0, M_PI_2, 3 * M_PI_2);
        cairo_restore(cr);
        cairo_stroke(cr);
		// Draw sum of all vectors in back
		if (ensembleVector && (sum_angle < coneAngle || sum_angle > M_PI - coneAngle || sum_y > 0.0)) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		break;
	case XYRotatingField:
		precessionAngle = cos(window->phase) / 5;
		circleWidth = sin(window->phase) * 8.0;
		// Hind half
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 0.25);
		x1 = -halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = -halfWidth * sin(coneAngle) - x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = -halfWidth * sin(coneAngle) - x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = -halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = halfWidth * sin(coneAngle) - x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = halfWidth * sin(coneAngle) - x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		cairo_stroke(cr);
		// Cone
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		x1 = -halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		x1 = halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		x1 = -halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		x1 = halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		cairo_stroke(cr);
		// Draw hind vectors
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			if (window->realQuantumProbability) {
				random = (float)rand() / (float)RAND_MAX;
				if (random < pow(v->A_alpha, 2)) {
					state = 1.0;    // draw upward
				} else if (random < pow(v->A_alpha, 2)) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			} else {
				if (v->A_alpha > M_SQRT1_2) {
					state = 1.0;    // draw upward
				} else if (v->A_alpha < M_SQRT1_2) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			}
			angle = (v->phase_alpha - v->phase_beta) - window->phase + M_PI_2;
			while (angle < 0.0) angle += 2 * M_PI;
			while (angle > 2 * M_PI) angle -= 2 * M_PI;
			if (angle < M_PI_2 || angle > 3 * M_PI_2) {
				cairo_move_to(cr, centre_x, centre_y);
				x1 = -halfWidth * cos(coneAngle) * sin(angle);
				y1 = (halfWidth * sin(coneAngle) - state * cos(state * angle) * circleWidth) * state;
				x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
				y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
				cairo_line_to(cr, centre_x + x2, centre_y - y2);
			}
		}
		cairo_stroke(cr);
		// Draw sum of all vectors in front
		if (ensembleVector && (sum_angle < coneAngle || sum_angle > M_PI - coneAngle)) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		// Draw front vectors
		cairo_set_line_width(cr, 1.0);
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		for (i = 0; i < window->numberOfSpins; i++) {
			v = g_ptr_array_index(window->spinSet, i);
			if (window->realQuantumProbability) {
				random = (float)rand() / (float)RAND_MAX;
				if (random < pow(v->A_alpha, 2)) {
					state = 1.0;    // draw upward
				} else if (random < pow(v->A_alpha, 2)) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			} else {
				if (v->A_alpha > M_SQRT1_2) {
					state = 1.0;    // draw upward
				} else if (v->A_alpha < M_SQRT1_2) {
					state = -1.0;   // draw downward
				} else {
					random = (float)rand() / (float)RAND_MAX;
					state = (random > 0.5) ? 1.0 : -1.0;
				}
			}
			angle = (v->phase_alpha - v->phase_beta) - window->phase + M_PI_2;
			if (angle < 0.0) angle += 2 * M_PI;
			while (angle < 0.0) angle += 2 * M_PI;
			while (angle > 2 * M_PI) angle -= 2 * M_PI;
			if (angle >= M_PI_2 && angle <= 3 * M_PI_2) {
				cairo_move_to(cr, centre_x, centre_y);
				x1 = -halfWidth * cos(coneAngle) * sin(angle);
				y1 = (halfWidth * sin(coneAngle) - state * cos(state * angle) * circleWidth) * state;
				x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
				y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
				cairo_line_to(cr, centre_x + x2, centre_y - y2);
			}
		}
		cairo_stroke(cr);
		// Front half
		cairo_set_source_rgba(cr, 0.1, 0.75, 0.1, 1.0);
		x1 = -halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = -halfWidth * sin(coneAngle) + x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = halfWidth * cos(coneAngle);
		y1 = -halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = -halfWidth * sin(coneAngle) + x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = -halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y <= cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = y * cos45 + x * sin45;
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = halfWidth * sin(coneAngle) + x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		x1 = halfWidth * cos(coneAngle);
		y1 = halfWidth * sin(coneAngle);
		x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
		y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
		cairo_move_to(cr, x2 + centre_x, centre_y - y2);
		for (float y = sin45; y < cos45; y += 0.01) {
			x = sqrtf(1 - pow(y, 2));
			temp = -(y * cos45 + x * sin45);
			x = x * cos45 - y * sin45;
			x1 = temp * halfWidth * cos(coneAngle);
			y1 = halfWidth * sin(coneAngle) + x * circleWidth;
			x2 = x1 * cos(precessionAngle) - y1 * sin(precessionAngle);
			y2 = x1 * sin(precessionAngle) + y1 * cos(precessionAngle);
			cairo_line_to(cr, x2 + centre_x, centre_y - y2);
		}
		cairo_stroke(cr);
		// Draw sum of all vectors in front
		if (ensembleVector && sum_y > 0.0 && sum_angle >= coneAngle && sum_angle <= M_PI - coneAngle) {
			temp = 0.9 * halfWidth / (float)window->numberOfSpins;
			cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
			cairo_set_line_width(cr, 3.0);
			cairo_move_to(cr, centre_x, centre_y);
			cairo_line_to(cr, centre_x + (1.8 * temp * sum_x), centre_y - 1.8 * temp * sum_z);
			cairo_stroke(cr);
		}
		break;
	}
}

