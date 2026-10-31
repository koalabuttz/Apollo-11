/* SPDX-License-Identifier: MIT */
/* Service routines (SERVICE_ROUTINES.agc): small subroutines the whole
 * program shares.
 *   - UPFLAG and DOWNFLAG set or clear one named flag bit, UPENT2 and
 *     DOWNENT2 several bits of one flag word.
 *   - DELAYJOB puts the running job to sleep for a given time; a WAITLIST
 *     task (WAKER) wakes it again.
 *   - GENTRAN copies a block of words.
 *   - B5OFF ends an extended verb.
 *   - In the CM, TRFAILOF and TRFAILON turn the TRACKER light off and on.
 *
 * Each routine starts at its label and runs to the instruction that
 * leaves it: a return to the caller (Z the return address), or a jump to
 * another routine (Z its label: JOBSLEEP, TASKOVER, ENDOFJOB...), which
 * the flight program runs next. GENTRAN copies one word per call:
 * gentran_plus1() is its loop. */
#ifndef READABLE_SERVICE_H
#define READABLE_SERVICE_H

#include "../agc/agc.h"

void upent2(void);
void downent2(void);

void upflag(void);
void downflag(void);
void comflag(void);
void debit(void);

void delayjob(void);
void tcgetcad(void);
void waker(void);
void waker_after_jobwake(void);

void gentran(void);
void gentran_plus1(void);

void b5off(void);

#if COMANCHE055
void trfailof(void);
void trfailon(void);
#endif

#endif
