/* SPDX-License-Identifier: MIT */
/* The Executive (EXECUTIVE.agc): the AGC's job scheduler.
 *
 * A job is a piece of work that runs until it ends, sleeps or yields to a
 * job of higher priority. Each job in the system owns a core set: twelve
 * erasable words holding its multi-purpose accumulator (MPAC), its place
 * (LOC, BANKSET), its interpretive push-down pointer (PUSHLOC) and its
 * PRIORITY. The running job's core set is always core set 0; a job switch
 * exchanges core sets. Interpretive jobs also own a VAC area, 43 words of
 * work space.
 *
 * Each entry is called with the machine as the original's TC leaves it (A,
 * L and Q are its arguments) and returns with Z at the address the original
 * continues at. Inline operands (the 2CADR after TC NOVAC) come in as
 * arguments, read by the binding from the program's operand table. */
#ifndef READABLE_EXECUTIVE_H
#define READABLE_EXECUTIVE_H

#include "../agc/agc.h"

/* Starting jobs. */
void novac(const word job[2]);
void findvac(const word job[2]);
void spvacin(void);
void spvac(void);

/* Changing jobs. */
void chang1(void);
void chang2(void);
void jobsleep(void);
void jobwake(void);
void priochng(void);
void endofjob(void);

/* The idle loop. */
void dummyjob(void);
void dummyjob_plus2(void);
void advan(void);
void nuchang2(void);
void supdxchz(void);
void supdxchz_plus1(void);

/* Called at every exchange of core sets (CHANJOB +4: the job in core set
 * newjob/12 takes core set 0) and when the job in core set 0 ends (ENDOFJOB).
 * They change nothing in the AGC; the binding defines them (the C64
 * resident pages overlay modules with them). */
void executive_exchanging_core_sets(word newjob);
void executive_ending_job(void);

#endif
