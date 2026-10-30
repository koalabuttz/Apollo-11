/* SPDX-License-Identifier: MIT */
/* Phase table maintenance (PHASE_TABLE_MAINTENANCE.agc): the major mode on
 * the DSKY, and the phase changes that tell a restart what to start again.
 *
 * The AGC can restart at any moment (a power transient, a hardware alarm,
 * a program alarm that asks for one). A restart clears the Executive and
 * WAITLIST and starts again only what the program has registered: six
 * restart groups, each with a phase that names the jobs, tasks or LONGCALLs
 * to start. A program changes a group's phase as it goes (TC PHASCHNG; OCT
 * ...), so that after a restart it carries on from its last safe point.
 * The listing's long comment on p. 1295 (LM) describes the phase words.
 *
 * Each entry is called with the machine as the original's TC leaves it and
 * returns with Z at the address the original continues at. Words after
 * the TC are read with read_word(). */
#ifndef READABLE_PHASE_TABLE_H
#define READABLE_PHASE_TABLE_H

#include "../agc/agc.h"

/* The major mode. */
void newmodex(void);
void newmodea(void);
void mmdsplay(void);
void checkmm(void);
void setupdsp(void);
void setupdsp_after_novac(void);

/* Phase changes. */
void two_phschng(void);
void phaschng(void);
#if LUMINARY099
void phschnga(void);
#endif
void phschng2(void);
void con2(void);

#endif
