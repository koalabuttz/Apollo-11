/* SPDX-License-Identifier: MIT */
#ifndef LM_DAP_INTERFACE_CASES_H
#define LM_DAP_INTERFACE_CASES_H
#include "lm_dap_interface.h"
#include "lm_exec_routines.h"
/* lm-dap-interface cases: each enters one DAP_INTERFACE_SUBROUTINES label
 * (case n uses entry n % 7: RESTORDB, SETMAXDB, SETMINDB, PFLITEDB,
 * ZATTEROR, STOPRATE, ALLCOAST) from a random image with FB bank 20, a
 * random EB and a caller return in Q, and runs through the trampoline
 * (these routines and the native NOVAC) to the caller's return or to
 * BAILOUT1. tools/lm_dap_interface_reference.c runs the same entry in the
 * original program. Returns are SWRETURN (BANKCALL/IBNKCALL callers), a
 * fixed-fixed address, or a bank-20 address outside the file; Executive
 * occupancy is varied so NOVAC also ends at the 1202 alarm. */
#define LM_DAP_ENTRIES 7u
#define LM_DAP_CASES 7168u
#define LM_DAP_RETURNED 0u
#define LM_DAP_ALARM 1u
typedef struct {agc_word stop_z,stop_fb,superbank;} lm_dap_case;
extern const agc_word lm_dap_entry_z[LM_DAP_ENTRIES];
void lm_dap_input(apollo_overlay_state *,p66_bias_io *,lm_dap_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (2: anything else). */
unsigned lm_dap_run(apollo_overlay_state *,uint16_t n,uint16_t *steps);
uint32_t lm_dap_digest(apollo_overlay_state *,uint16_t n);
#endif
