/* SPDX-License-Identifier: MIT */
/* The LM's T6 interrupt (T6-RUPT_PROGRAMS.agc): the RCS jets on time.
 *
 * The LM's digital autopilot decides on each pass which reaction control
 * jets to fire and for how long. To turn them off (or on) between passes
 * it uses TIME6, a clock that counts down in steps of 1/1600 second and
 * interrupts (T6RUPT) when it runs out. The autopilot queues up to two
 * such moments: T6NEXT and T6FURTHA, each a TIME6 count and a writer
 * (NXT6ADR: 0 for the P-axis jets in channel 6, 4 and 13 for the U and V
 * jets in channel 5). On T6RUPT, DOT6RUPT runs T6JOBCHK: the jets of the
 * moment come on (or off), the queue moves up one, and the clock starts
 * again for the next moment, if any.
 *
 * Comanche055 has no such file: the CM's T6RUPT goes where T6LOC says
 * (src/lead_ins/lead_ins.c). */
#ifndef READABLE_T6RUPT_H
#define READABLE_T6RUPT_H

#include "../agc/agc.h"

#if LUMINARY099
void dot6rupt(void);
void t6jobchk(void);
void writep_minus1(void);
void writep(void);
void writeu_minus1(void);
void writeu(void);
void writev_minus1(void);
void writev(void);
#endif

#endif
