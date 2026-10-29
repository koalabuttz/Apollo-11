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
                   EXEC_OUT_OF_SCOPE=1, EXEC_BAILOUT_PENDING=2,
                   EXEC_IDLE=3, EXEC_UNMAPPED=4, EXEC_INVALID_ACTION=5,
                   EXEC_QUEUE_FULL=6, EXEC_NOT_FOUND=7 };
typedef struct { agc_word location, bank; } exec_address;
typedef struct {
    agc_word core[EXEC_CORE_COUNT][EXEC_CORE_WORDS];
    agc_word vac[EXEC_VAC_COUNT][EXEC_VAC_WORDS];
    agc_word newjob, newprio, newloc[2], locctr, fixloc, ovfind;
    agc_word alarm; /* Native boundary marker; explicit recovery stages are in restart.h. */
} executive;
/* Initializes only the Executive state; not a flight-computer fresh start. */
void exec_init(executive *state);
/* Priority is the caller's packed upper-five-bit field (01000..037000).
 * Location and bank are an opaque historical 2CADR, not native function pointers.
 * Runs through CORFOUND/SETLOC to ENDFIND, or stops at BAILOUT1 entry.
 * Interrupt exclusion is the platform caller's responsibility. */
enum exec_status exec_admit(executive *state, agc_word priority,
                           agc_word location, agc_word bank, int needs_vac);
/* SPVAC entry: priority is already staged in NEWPRIO; A/L supply address.
 * Stops at ENDFIND or BAILOUT1 as exec_admit does. q is the caller's Q,
 * adjusted by -2 even on a resource bailout. q must be distinct from state;
 * caller owns IRQ exclusion. */
enum exec_status exec_spvac(executive *state, exec_address address, agc_word *q);
typedef struct { agc_word intbit15, edop; } exec_interpreter;
typedef struct { exec_address address; uint8_t interpretive; } exec_transfer;
/* CHANG2 from DANZIG's EDOP=+0 safe point. Core-zero LOC is the completed
 * interpretive word's address, BANKSET its BBCON. Dispatches a basic context
 * through DTCB, or resumes an interpretive context through INTRSM, stopping
 * before NEWOPS fetch. control and next must be distinct owned objects.
 * Caller owns IRQ exclusion. No opcode decoder or suspended native C stack. */
enum exec_status exec_change_interpretive(executive *state, agc_word superbank,
                                         exec_interpreter *control,
                                         exec_transfer *next);
/* ENDJOB1 and EJSCAN, stopping before CHANJOB/DUMMYJOB dispatch.
 * Returns a slot index 1..7, or -1 for idle. Does not execute or swap jobs. */
int exec_endjob_select(executive *state);
/* Basic CHANG1/CHANJOB and DTCB. Bank is the complete historical BBCON
 * (BBANK plus SUPERBNK bits), not a native memory bank. NEWJOB must name
 * an active basic context at slot 1..7. No mutation on unsupported input. */
enum exec_status exec_change_basic(executive *state, exec_address resume,
                                  exec_address *next);
/* ENDJOB1 through basic dispatch, or DUMMYJOB's NEWJOB=-0 store.
 * superbank is the current channel-7 value (bits 0160 only).
 * Unsupported interpreter selection is rejected before releasing resources. */
enum exec_status exec_complete_basic(executive *state, agc_word superbank,
                                    exec_address *next);
/* Bounded idle entry: NUDIRECT or NUCHANG2/CHANJOB+4, with activity A=2
 * and scratch L=0. Does not translate idle self-check or channel-11 lamp I/O. */
enum exec_status exec_start_basic(executive *state, exec_address *next);
/* JOBSLEEP: wake_cadr is a one-word fixed CADR, not a basic location.
 * bank is the current complete BBCON. Retains the core/VAC while dormant;
 * dispatches a waiting basic job or returns idle without releasing resources. */
enum exec_status exec_sleep_basic(executive *state, agc_word wake_cadr,
                                 agc_word bank, exec_address *next);
/* JOBWAKE: first matching dormant CADR across all eight cores. Returns
 * EXEC_NOT_FOUND (nonfatal), with LOCCTR=-1, or OK with its displacement.
 * Caller L scratch is adapted to +0 at SETLOC/SPECTEST. No dispatch. */
enum exec_status exec_wake_basic(executive *state, agc_word wake_cadr);
/* PRIOCHNG: change upper priority, retain low nine bits, then scan/dispatch.
 * resume is the current caller's basic return pair. No-change keeps its physical
 * SUPERBNK in next metadata; original LOC/BANKSET epilogue debris is preserved. */
enum exec_status exec_priority_basic(executive *state, agc_word priority,
                                    exec_address resume, exec_address *next);
#endif
