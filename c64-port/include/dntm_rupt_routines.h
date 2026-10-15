/* SPDX-License-Identifier: MIT */
#ifndef DNTM_RUPT_ROUTINES_H
#define DNTM_RUPT_ROUTINES_H
#include "agc_transfer.h"
/* The two ends of a DOWNRUPT for the flight store's down-telemetry modules
 * (docs/FLIGHT-STORE.md), on the DNTM io records (lm_dntm_io, cm_dntm_io):
 * the DOWNRUPT vector 04040 (DXCH ARUPT; CAF DNRPTBB; XCH BBANK; TCF
 * DODOWNTM, as src/rupt_lead_ins.c) and RESUME (LM 05270, CM 05333: EXTEND;
 * QXCH QRUPT; CA BANKRUPT; XCH BBANK; DXCH ARUPT; RELINT; RESUME, as
 * src/lm_t6rupt.c and src/rupt_lead_ins.c), where TMRESUME's TCF RESUME
 * goes. Packed beside the DNTM tables, they let one module run take a whole
 * DOWNRUPT, which runs inhibited, from the vector to the resumed Z. Each
 * needs program->hardware with the interrupt in progress and refuses what
 * the routines they copy refuse (tests/test_dntm_rupt.c compares them). */
extern const agc_routine_table lm_dntm_rupt_routines;
extern const agc_routine_table cm_dntm_rupt_routines;
#endif
