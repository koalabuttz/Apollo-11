/* SPDX-License-Identifier: MIT */
/* WAITLIST (WAITLIST.agc): the AGC's timer queue.
 *
 * A task is a short piece of work that runs in interrupt, at a set time.
 * WAITLIST keeps up to nine of them, in time order:
 *   - TIME3 counts up every centisecond and interrupts (T3RUPT) when it
 *     overflows: it holds 1.0 - (time to the first task);
 *   - LST1 +0 to +7 hold the gaps between tasks 1-9, as -(gap) + 1;
 *   - LST2 +0 to +17 hold the tasks' 2CADRs.
 * An empty place holds ENDTASK's task, SVCT3, which comes round every 81.93
 * seconds.
 *
 * Each entry is called with the machine as the original's TC leaves it and
 * returns with Z at the address the original continues at. */
#ifndef READABLE_WAITLIST_H
#define READABLE_WAITLIST_H

#include "../agc/agc.h"

/* Adding tasks: delay in A (centiseconds), task after the TC. */
void waitlist(const word task[2]);
void twiddle(word task_address);
/* From a task: wait, then carry on (FIXDELAY's delay is after the TC). */
void fixdelay(word delay);
void vardelay(void);
/* DLY2 -1: DELAYJOB's way in (TS WAITADR, then DLY2). */
void dly2_minus1(void);

/* T3RUPT (from its lead-in at 04014) and the end of a task. */
void t3rupt_lead_in(void);
void taskover(void);

/* The end of every interrupt: RESUME and the later ways in. */
void resume(void);
void noqrsm(void);
void resume_plus3(void);
void noqbrsm(void);
void noqbrsm_plus1(void);

/* SVCT3: ENDTASK's task, and its two continuations. */
void svct3(void);
void svct3_after_novac(void);
void svct3x_after_fixdelay(void);

/* Delays longer than WAITLIST's 162.5 seconds. */
void longcall(const word task[2]);
void lngcall2(void);
void longcycl(void);
void longrtrn(void);
void getcadr(void);

#endif
