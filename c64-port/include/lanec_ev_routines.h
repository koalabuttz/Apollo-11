/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_ROUTINES_H
#define LANEC_EV_ROUTINES_H
/* EXTENDED_VERBS (Luminary099 and Comanche055) as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Every entry and every
 * point where a callee returns into the file that a Lane C suite verifies is
 * registered at its physical address (src/lanec_ev_routines.inc,
 * tools/lanec/routines.py ev) and runs that module's lanec_ev_*_dispatch;
 * LANEC_OUT_OF_SCOPE is EXEC_OUT_OF_SCOPE. The Luminary interpretive words
 * (lanec_ev_lmi) are interpretive routines keyed by LOC under BANKSET.
 *
 * The modules read and write channels 12, 14, 30, 31 and 33 besides the
 * superbank and interrupt enable, so io is the mission's downtelemetry
 * device record, which carries them: an lm_dntm_io for Luminary (its first
 * member the p66_bias_io the INTER-BANK and Executive routines take; output
 * channel 11 comes from program->hardware as for the display interface, and
 * the routines refuse when that is NULL) and a cm_dntm_io for Comanche (its
 * cm_native_io first). */
#include "agc_transfer.h"
extern const agc_routine_table lanec_ev_lm_routines, lanec_ev_lm_interpretive;
extern const agc_routine_table lanec_ev_cm_routines;
/* INTPRET (fixed-fixed 06037) for this file's TC INTPRET callers only: Q must
 * name one of lanec_ev_lm_interpretive's points. A program registers it in
 * place of a general INTPRET routine, which is the interpreter's (none yet). */
extern const agc_routine_table lanec_ev_lm_intpret;
/* The basic points, one table per module, for a program that registers only
 * some of them. */
extern const agc_routine_table lanec_ev_lm_front, lanec_ev_lm_short, lanec_ev_lm_align, lanec_ev_lm_radar,
    lanec_ev_lm_dap, lanec_ev_lm_time, lanec_ev_lm_track;
extern const agc_routine_table lanec_ev_cm_front, lanec_ev_cm_short, lanec_ev_cm_align, lanec_ev_cm_dap,
    lanec_ev_cm_time, lanec_ev_cm_track;
#endif
