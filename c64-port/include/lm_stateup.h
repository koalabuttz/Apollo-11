/* SPDX-License-Identifier: MIT */
#ifndef LM_STATEUP_H
#define LM_STATEUP_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 FRESH_START_AND_RESTART, bank 13: STATEUP (the interpretive
 * state-vector extrapolation called from INTEGRATION_INITIALIZATION for
 * P00) and THISVINT, as interpretive routines of the control-transfer ABI
 * (docs/NATIVE_TRANSFER_ABI.md). Each routine is one order-pair word, keyed
 * by LOC under BANKSET at the interpreter fetch point NEWOPS (06070): every
 * order is the compiled lm_interp_steps / lm_interp_orders path for its
 * words, so a run goes from pair to pair through the trampoline until LOC
 * leaves the file (CALL INTEGRV, STCALL TDEC1 INTSTALL, CALL SETIFLGS,
 * CLRGO NODOFLAG ENDINT, THISVINT's RVQ) or DANZIG finds NEWJOB positive
 * (CHANG2). io is a p66_bias_io: the switch orders' INHINT/RELINT act on
 * its interrupt-enable state. */
#define LM_STATEUP_ROUTINES 11u
#define LM_STATEUP_NEWOPS 06070u
#define LM_STATEUP_LOC 0164u
#define LM_STATEUP_BANKSET 0165u
extern const agc_routine_table lm_stateup_interpretive;
/* Order-pair addresses (bank 13), for tests. */
extern const uint16_t lm_stateup_entries[LM_STATEUP_ROUTINES];
#endif
