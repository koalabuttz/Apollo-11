/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPATCH_H
#define CM_DISPATCH_H
#include "cm_executive.h"
/* Verified against Comanche055, seven cores; caller owns IRQ exclusion. */
enum exec_status cm_exec_change_interpretive(cm_executive *,agc_word,exec_interpreter *,exec_transfer *);
enum exec_status cm_exec_change_basic(cm_executive *state, exec_address resume,
                                  exec_address *next);
/* ENDJOB1 through basic dispatch, or DUMMYJOB's NEWJOB=-0 store.
 * superbank is the current channel-7 value (bits 0160 only).
 * Unsupported interpreter selection is rejected before releasing resources. */
enum exec_status cm_exec_complete_basic(cm_executive *state, agc_word superbank,
                                    exec_address *next);
/* Bounded idle entry: NUDIRECT or NUCHANG2/CHANJOB+4, with activity A=2
 * and scratch L=0. Does not translate idle self-check or channel-11 lamp I/O. */
enum exec_status cm_exec_start_basic(cm_executive *state, exec_address *next);
/* JOBSLEEP: wake_cadr is a one-word fixed CADR, not a basic location.
 * bank is the current complete BBCON. Retains the core/VAC while dormant;
 * dispatches a waiting basic job or returns idle without releasing resources. */
enum exec_status cm_exec_sleep_basic(cm_executive *state, agc_word wake_cadr,
                                 agc_word bank, exec_address *next);
/* JOBWAKE: first matching dormant CADR across all seven cores. Returns
 * EXEC_NOT_FOUND (nonfatal), with LOCCTR=-1, or OK with its displacement.
 * Caller L scratch is adapted to +0 at SETLOC/SPECTEST. No dispatch. */
enum exec_status cm_exec_wake_basic(cm_executive *state, agc_word wake_cadr);
/* PRIOCHNG: change upper priority, retain low nine bits, then scan/dispatch.
 * resume is the current caller's basic return pair. No-change keeps its physical
 * SUPERBNK in next metadata; original LOC/BANKSET epilogue debris is preserved. */
enum exec_status cm_exec_priority_basic(cm_executive *state, agc_word priority,
                                    exec_address resume, exec_address *next);

#endif
