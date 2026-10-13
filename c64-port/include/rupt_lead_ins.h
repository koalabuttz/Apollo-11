/* SPDX-License-Identifier: MIT */
#ifndef RUPT_LEAD_INS_H
#define RUPT_LEAD_INS_H
#include "agc_transfer.h"
/* INTERRUPT_LEAD_INS.agc of both missions as native routines of the
 * control-transfer ABI (docs/RUPT-LEAD-INS.md). Every vector but GOPROG's
 * needs program->hardware with the interrupt in progress; GOPROG (04000,
 * the restart) runs INHINT; CAF GOBB; XCH BBANK; TCF GOPROG.
 *
 * Banked vectors: DXCH ARUPT; CAF xxRPTBB; XCH BBANK; TCF handler. Z ends
 * at the handler with BB from the BBCON (SUPERBNK unchanged) and A the old
 * BB:
 *   LM  T4RUPT 04020, KEYRUPT1 04024, KEYRUPT2 04030 (MARKRUPT), UPRUPT
 *       04034, DOWNRUPT 04040 (DODOWNTM), RADAR 04044 (RADAREAD), RUPT10
 *       04050 (PITFALL)
 *   CM  T4RUPT 04020, KEYRUPT1 04024, KEYRUPT2 04030 (MARKRUPT), UPRUPT
 *       04034, DOWNRUPT 04040 (DODOWNTM), RADAR 04044 (VHFREAD), HANDRUPT
 *       04050 (RESUME +3)
 * 2CADR vectors: DXCH ARUPT; EXTEND; DCA 2CADR; DTCB, from the 2CADR in
 * erasable: LM T5RUPT 04010 (T5ADR), CM T6RUPT 04004 (T6LOC). CM T5RUPT
 * 04010 (CS TIME5; AD .5SEC; TCF T5RUPT) and T5RUPT 04065 (EXTEND; BZMF
 * NOQBRSM; EXTEND; DCA T5LOC; DTCB) end at NOQBRSM when TIME5 is within half
 * a second of overflow, else at T5LOC. The LM T6RUPT vector is in
 * lm_t6rupt_routines and the T3RUPT vectors in lm/cm_exec_task routines.
 *
 * cm_lead_in_routines also has Comanche WAITLIST.agc's interrupt exits:
 * RESUME 05333, NOQRSM 05335, NOQRSM+1 05336 (HANDRUPT's entry) and NOQBRSM
 * 05337; they end the interrupt at ZRUPT-1 (as lm_t6rupt_routines does for
 * Luminary). io is p66_bias_io (LM) or cm_native_io (CM); only their
 * superbank and interrupt_enabled are used. A 2CADR outside fixed memory
 * (02000-07777) has no producer and is refused unchanged. */
extern const agc_routine_table lm_lead_in_routines;
extern const agc_routine_table cm_lead_in_routines;
#endif
