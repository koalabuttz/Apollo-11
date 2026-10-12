/* SPDX-License-Identifier: MIT */
#ifndef CM_STATEUP_H
#define CM_STATEUP_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Comanche055 FRESH_START_AND_RESTART interpretive code: bank 13 STATEUP
 * (the interpretive state-vector extrapolation called from
 * INTEGRATION_INITIALIZATION for P00), STATEND and THISVINT, and bank 04's
 * R00 (CALL INTSTALL; DUMMYAD EXIT), as interpretive routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Each routine is one
 * order-pair word, keyed by LOC under BANKSET at the interpreter fetch point
 * NEWOPS (06037): every order is the compiled Comanche055 interpreter path
 * (include/cm_interp.h) for its words, so a run goes from pair to pair
 * through the trampoline until LOC leaves the file (CALL INTEGRV, STCALL
 * TDEC1 INTSTALL, CALL SETIFLGS, CLRGO NODOFLAG ENDINT, THISVINT's RVQ, R00's
 * CALL INTSTALL), EXIT leaves the interpreter for the basic word after it
 * (R00's CS OCT1400, a cm-v37 entry), or DANZIG finds NEWJOB positive
 * (CHANG2). io is a p66_bias_io: the switch orders' INHINT/RELINT act on its
 * interrupt-enable state. */
#define CM_STATEUP_ROUTINES 12u
#define CM_STATEUP_NEWOPS 06037u
#define CM_STATEUP_LOC 0164u
#define CM_STATEUP_BANKSET 0165u
extern const agc_routine_table cm_stateup_interpretive;
/* Order-pair addresses (bank 13; 02131 and 02133 are bank 04), for tests. */
extern const uint16_t cm_stateup_entries[CM_STATEUP_ROUTINES];
#endif
