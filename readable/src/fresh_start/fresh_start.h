/* SPDX-License-Identifier: MIT */
/* Fresh start and restart (FRESH_START_AND_RESTART.agc, its bank 05 part
 * and GOTOPOOH):
 *   - SLAP1, the fresh start (V36), sets the whole program to its starting
 *     state: no jobs, no tasks, no restart groups, the displays blank, P00;
 *   - GOPROG, the hardware restart, comes after a GOJAM (a power transient,
 *     a parity fail, a TC trap...): it keeps the program's state, clears
 *     its jobs and tasks, checks the phase table and has RESTARTS start
 *     again every restart group that was running;
 *   - ENEMA (and GOPROG2 in the LM), the software restart, does the same
 *     without a GOJAM;
 *   - MR.KLEAN, P00KLEAN and V37KLEAN end restart groups.
 * Each routine runs until it leaves the file (through Z): to MMDSPLAY,
 * SWCALL, ALARM, WAITLIST and the others, or with POSTJUMP to DUMMYJOB +2
 * (the Executive's idle loop) or GOP00FIX (P00). The places those return
 * to are routines too. */
#ifndef READABLE_FRESH_START_H
#define READABLE_FRESH_START_H

#include "../agc/agc.h"

void slap1(void);
void mr_klean(void);
void p00klean(void);
void v37klean(void);
void goprog(void);
void enema(void);
#if LUMINARY099
void goprog2(void);
#else
void goprog_after_vac5stor(void);
void elrskip_after_setcoars(void);
void nocoarse(void);
void goprog3(void);
void goprog3_after_stoprate(void);
void t5idloc(void);
void gotopooh_after_phaschng(void);
#endif
void goprog3_after_mmdsplay(void);
void pinact(void);
void ptbad_after_alarm(void);
void gotopooh(void);

#endif
