/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, the CM's (Comanche055 pp. 1063-1092):
 * the entry DAP, which flies the command module through entry once the
 * service module is gone. The faithful C is c64-port/src/cm_entry_dap.c.
 *
 * Every 0.1 s READGYMB reads the gimbal angles and BODYRATE turns their
 * changes into the body rates P, Q and R; the attitude DAP (EXDAPIN) and
 * its jet calls (SETJTAG, CM/RCS) hold the commanded roll and damp pitch
 * and yaw, every 2 s the roll system (TIMETST, JETCALL) times the roll
 * jets, and CM/FDAI drives the needles. CM/DAPON (P61-P67) turns the
 * service module DAP off and starts it all; CM/DAPIC starts the 0.1 s
 * cycle.
 *
 * All basic code. The faithful port's code is in parts (13 of them), and
 * a run stops when control reaches a key of another part. Each function
 * here runs from a key; a TC or jump to another key leaves Z there, and
 * the binding (src/binding/cm_entry_dap_routines.c) goes on at a key of
 * the same part of the faithful port's, as its run does.
 *
 * Written in five parts (README.md):
 *   A: readgymb.c, READGYMB to ANGOVCOR and FLUSHJET (pp. 1063-1065, bank
 *     15 and fixed-fixed); dapon.c, CM/DAPON and CM/DAPIC (pp. 1066-1069,
 *     bank 20).
 *   B to E: BODYRATE to CM/END (pp. 1065 and 1070-1092, bank 15).
 * The binding runs the faithful port's routines for the keys no part has
 * written yet. */
#ifndef READABLE_ENTRY_DAP_H
#define READABLE_ENTRY_DAP_H

#include "../agc/agc.h"

#if COMANCHE055
#include "../tvc/tvc.h" /* tc_to, tc_q */

/* ---- Part A: readgymb.c (bank 15, FLUSHJET in fixed-fixed) ---- */
void flushjet(void);                 /* FLUSHJET */
void readgymb(void);                 /* READGYMB */
void readgymb_after_flushjet(void);  /* READGYMB +012 */
void readgym1(void);                 /* READGYM1 */
void dobrate_q_ok(void);             /* DOBRATE? +4 */
void dobrate_q_first_pass(void);     /* DOBRATE? +5 */
void dobrate_q_noop(void);           /* DOBRATE? +6 */
void dobrate_q_standby(void);        /* DOBRATE? +7 */
void dobrate_q_group6(void);         /* DOBRATE? +010 */
void dobrate_q_after_phaschng(void); /* DOBRATE? +012 */
void dobrate1_after_waitlist(void);  /* DOBRATE1 +5 */
void dobrate1_taskover(void);        /* DOBRATE1 +7 */
void angovcor(void);                 /* ANGOVCOR */

/* ---- Part A: dapon.c (bank 20) ---- */
void cm_dapon(void);                /* CM/DAPON */
void cm_dapon_after_dapbit1(void);  /* CM/DAPON +4 */
void cm_dapon_after_dapbit2(void);  /* CM/DAPON +6 */
void cm_dapon_t5loc(void);          /* CM/DAPON +010 */
void cm_dapon_after_flushjet(void); /* CM/DAPON +015 */
void cm_dapon_plus4_tc(void);       /* CM/DAPON +020 */
void notyet(void);                  /* NOTYET */
void notyet_bankcall(void);         /* NOTYET +1 */
void notyet_plus4(void);            /* NOTYET +3, the listing's "+4" */
void notyet_alfacom(void);          /* NOTYET +020 */
void cm_dapic(void);                /* CM/DAPIC */
void cm_dap2c_jetem(void);          /* CM/DAP2C +5 */
void cm_dap2c_2cadr(void);          /* CM/DAP2C +020 */
void cm_dap2c_after_waitlist(void); /* CM/DAP2C +021 */
void cm_dap2c_beta(void);           /* CM/DAP2C +025 */
void cm_dap2c_beta_ts(void);        /* CM/DAP2C +026 */
void cm_dap2c_2phschng(void);       /* CM/DAP2C +031 */
void cm_dap2c_after_2phschng(void); /* CM/DAP2C +035 */

#endif

#endif
