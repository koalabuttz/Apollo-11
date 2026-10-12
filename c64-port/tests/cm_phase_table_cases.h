/* SPDX-License-Identifier: MIT */
#ifndef CM_PHASE_TABLE_CASES_H
#define CM_PHASE_TABLE_CASES_H
#include "cm_phase_table.h"
#include "cm_native_image.h"
/* Each case enters one PHASE_TABLE_MAINTENANCE entry as its caller does and
 * runs through the trampoline (with cm_interbank_routines for BANKJUMP)
 * until a point this file does not own: the caller's return, or CHECKMM's
 * Q+1/Q+2 in INTERPRETER. NOVAC is a test stub that records its entry
 * registers and the 2CADR at Q, then returns to Q+2, as the original does;
 * tools/cm_phase_table_reference.c does the same in the original program.
 * Case n uses caller site n % (sites + 2): every TC NEWMODEX, CHECKMM,
 * PHASCHNG and 2PHSCHNG site, then NEWMODEA and MMDSPLAY. Images, banks,
 * SUPERBNK, the interrupt-enable state and MODREG (equal to the mode a
 * CHECKMM case tests, half the time) are pseudo-random. */
#define CM_PT_CASES 4096u
typedef struct { agc_word a,l,q,cadr,bbcon; uint8_t calls; } cm_pt_log;
void cm_pt_input(apollo_overlay_state *,cm_native_io *,uint16_t n);
enum exec_status cm_pt_run(apollo_overlay_state *,cm_native_io *,uint16_t n,cm_pt_log *,uint16_t *steps);
/* Digest in the oracle's order (tools/cm_phase_table_freeze.py). */
uint32_t cm_pt_digest(apollo_overlay_state *,uint16_t n,enum exec_status *);
uint16_t cm_pt_site_count(void);
#endif
