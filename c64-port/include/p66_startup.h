/* SPDX-License-Identifier: MIT */
#ifndef P66_STARTUP_H
#define P66_STARTUP_H
#include "phase.h"
#include "waitlist.h"
#define P66_RODCOMP_LOCATION UINT16_C(03275)
#define P66_RODTASK_LOCATION UINT16_C(03263)
#define P66_RODCOMP_BBCON UINT16_C(064067)
typedef struct {
 agc_register a,l;
 agc_word q,bbank,superbank,interrupt_enabled;
 agc_word exectem1,exectem2;
} p66_task_io;
typedef struct { agc_register a; agc_word rodcount; } p66_operator_state;
/* DESCBITS through RESUME entry. A is the original channel-16 input;
 * bit7 decrements, otherwise increments, retaining ADS register overflow.
 * The platform owns interrupt exclusion and interrupted-context restoration. */
void p66_descbits(p66_operator_state *);
/* Actual RODTASK entry, original PRIO22/FINDVAC 2CADR, through TASKOVER
 * entry or BAILOUT1 entry. Does not schedule another RODTASK. Source BBCON
 * includes SUPERBNK=060 even when the task caller's channel7 differs.
 * EXECTEM1/2 alias addresses061/062; callers reconcile shared scratch.
 * Valid Executive allocation states required (VACUSE=0 or its free base;
 * idle NEWJOB=-0 requires free core0). BBANK/SUPERBNK must select original
 * physical fixed bank32; no mutation on rejected input. */
enum exec_status p66_rodtask(executive *,p66_task_io *);
/* Actual P66VERTA entry: terminate group3, TWIDDLE +100cs RODTASK, return
 * immediately to RODCOMP entry. TIME1/2/3 stay stable during this service.
 * phase scratch aliases WAITEXIT/BANK/ADR/TEMP at061..064, exactly as source.
 * The returned PHASE scratch must be reconciled with any Executive aliases
 * before job admission; objects are distinct and caller owns exclusion.
 * Returns WL_FULL at original BAILOUT1 without rollback of insertion. */
enum wl_status p66_verta(phase_state *,waitlist *);
#endif
