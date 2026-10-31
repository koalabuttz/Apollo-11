/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 4 (the CM's only): the optics monitor, zeroing and drive
 * (T4RUPT_PROGRAM.agc, OPTMON to the end of the file).
 *
 * T4JUMP comes to OPTMON, and through OPTTEST (part 1) to OPTDRIVE, each
 * every 0.48 seconds. OPTMON watches the optics CDU fail bit (channel 30
 * bit 7) for the OCDU FAIL lamp, and the optics mode switch (channel 33
 * bits 4 and 5: zero optics, manual, or computer, CSC). A change of
 * switch starts or cancels optics zeroing (ZOPTICS: 32 samples, then the
 * zero CDU discrete for 200 ms, ended by the task ENDZOPT), or cancels
 * coarse optics. OPTDRIVE, in bank 10, keeps the shaft's stop zone and,
 * while coarse optics work, works out each CDU's drive command and sends
 * it (channel 14). Every pass ends at RESUME.
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). */
#ifndef READABLE_OPTICS_H
#define READABLE_OPTICS_H

#include "../agc/agc.h"

#if COMANCHE055
void optmon(void);
void zopalarm_after_alarm(void);    /* ZOPALARM +2 */
void ztocsc_after_alarm(void);      /* ZTOCSC +4 */
void setzoend_after_waitlist(void); /* SETZOEND +7 */
void endzopt(void);                 /* the task SETZOEND starts */
void endzopt_after_vardelay(void);  /* ENDZOPT +6 */
void optdrive(void);
void contdrve_after_alarm(void); /* CONTDRVE +016 */
#endif

#endif
