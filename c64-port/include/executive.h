/* SPDX-License-Identifier: MIT */
#ifndef EXECUTIVE_H
#define EXECUTIVE_H
#include "agc_word.h"
#define EXEC_CORE_COUNT 8
#define EXEC_ADMISSION_COUNT 8 /* NO.CORES=7 is a CCS down-counter, inclusive of zero. */
#define EXEC_CORE_WORDS 12
#define EXEC_VAC_COUNT 5
#define EXEC_VAC_WORDS 44
#define EXEC_LOW9 UINT16_C(0777)
#define EXEC_FAKEPRET UINT16_C(0110) /* MPAC(0154) - 36 decimal */
#define EXEC_VAC1USE UINT16_C(0400)
enum exec_core_field { EXEC_MPAC=0, EXEC_MODE=7, EXEC_LOC=8, EXEC_BANK=9,
                       EXEC_PUSH=10, EXEC_PRIORITY=11 };
enum exec_status { EXEC_OK=0, EXEC_NO_VAC=01201, EXEC_NO_CORE=01202,
                   EXEC_OUT_OF_SCOPE=1, EXEC_BAILOUT_PENDING=2 };
typedef struct {
    agc_word core[EXEC_CORE_COUNT][EXEC_CORE_WORDS];
    agc_word vac[EXEC_VAC_COUNT][EXEC_VAC_WORDS];
    agc_word newjob, newprio, newloc[2], locctr, fixloc, ovfind;
    agc_word alarm; /* Native bailout boundary marker; no restart implementation. */
} executive;
/* Initializes only the Executive state; not a flight-computer fresh start. */
void exec_init(executive *state);
/* Priority is the caller's packed upper-five-bit field (01000..037000).
 * Location and bank are an opaque historical 2CADR, not native function pointers.
 * Runs through CORFOUND/SETLOC to ENDFIND, or stops at BAILOUT1 entry.
 * Interrupt exclusion is the platform caller's responsibility. */
enum exec_status exec_admit(executive *state, agc_word priority,
                           agc_word location, agc_word bank, int needs_vac);
/* ENDJOB1 and EJSCAN, stopping before CHANJOB/DUMMYJOB dispatch.
 * Returns a slot index 1..7, or -1 for idle. Does not execute or swap jobs. */
int exec_endjob_select(executive *state);
#endif
