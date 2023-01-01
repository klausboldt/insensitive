/* insensitive-pulsesequence.h
 *
 * Copyright 2009-2023 Klaus Boldt
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

#ifndef __INSENSITIVE_PULSESEQUENCE_H__
#define __INSENSITIVE_PULSESEQUENCE_H__

#pragma once

#include <glib-object.h>

#include "insensitive.h"
#include "insensitive-settings.h"


enum SequenceType {
    SequenceTypePulse = 0,
    SequenceTypeEvolution = 1,
    SequenceTypeGradient = 2,
    SequenceTypeFID = 3,
    SequenceTypeShift = 4,
    SequenceTypeCoupling = 5,
    SequenceTypeRelaxation = 6
};

typedef struct {
    enum SequenceType type;
    float time, secondParameter;
    unsigned int pulseArray;
    /*gboolean iDecoupling, sDecoupling;
    gboolean activeISpins, activeSSpins;
    gboolean selectiveIPulse, selectiveSPulse;
    gboolean spinlock;*/
    char iDecoupling, sDecoupling;
    char activeISpins, activeSSpins;
    char selectiveIPulse, selectiveSPulse;
    char spinlock;
    float pulseDuration, pulseStrength, pulseFrequency;
    enum PulseEnvelope pulseEnvelope;
} SequenceElement;


struct _InsensitivePulseSequence
{
  GObject parent_instance;

   int position;
   GPtrArray *sequence;
};


G_BEGIN_DECLS

#define INSENSITIVE_TYPE_PULSESEQUENCE (insensitive_pulsesequence_get_type())

G_DECLARE_FINAL_TYPE(InsensitivePulseSequence, insensitive_pulsesequence, INSENSITIVE, PULSESEQUENCE, GObject)

InsensitivePulseSequence *insensitive_pulsesequence_new(void);
InsensitivePulseSequence *insensitive_pulsesequence_copy(InsensitivePulseSequence *self);

void insensitive_pulsesequence_add_element(InsensitivePulseSequence *self, enum SequenceType type, InsensitiveSettings *settings);
void insensitive_pulsesequence_add_sequence_element(InsensitivePulseSequence *self, SequenceElement *newElement);
void insensitive_pulsesequence_erase_sequence(InsensitivePulseSequence *self);
int insensitive_pulsesequence_get_number_of_elements(InsensitivePulseSequence *self);
int insensitive_pulsesequence_get_position(InsensitivePulseSequence *self);
void insensitive_pulsesequence_set_position(InsensitivePulseSequence *self, int value);
GPtrArray *insensitive_pulsesequence_get_sequenceArray(InsensitivePulseSequence *self);
void insensitive_pulsesequence_set_sequenceArray(InsensitivePulseSequence *self, GPtrArray *array);
SequenceElement *insensitive_pulsesequence_get_element_at_index(InsensitivePulseSequence *self, unsigned int index);
SequenceElement *insensitive_pulsesequence_get_last_element(InsensitivePulseSequence *self);
float insensitive_pulsesequence_perform_actions_on_spinsystem(InsensitivePulseSequence *self,
                                                              gpointer spinsystem_,
                                                              unsigned int start,
                                                              unsigned int stride,
                                                              InsensitiveSettings *settings,
                                                              gpointer controller_);
float insensitive_pulsesequence_gradient_perform_actions_on_spinsystem(InsensitivePulseSequence *self,
								                                       gpointer spinsystem_,
                                                                       InsensitiveSettings *settings);

G_END_DECLS

#endif /* __INSENSITIVE_PULSESEQUENCE_H__ */
