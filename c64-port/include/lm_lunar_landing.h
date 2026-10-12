/* SPDX-License-Identifier: MIT */
#ifndef LM_LUNAR_LANDING_H
#define LM_LUNAR_LANDING_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 THE_LUNAR_LANDING.agc, all of it (P63 in bank 32
 * 02776-03262, P68's LANDJUNK in bank 31 02144-02215), as native routines
 * of the control-transfer ABI. See docs/LM-LUNAR-LANDING.md.
 *
 * Every basic instruction is one ai_* effect on the image in source order;
 * every interpretive order pair is the compiled lm_interp_steps /
 * lm_interp_orders path for its words. A routine starts at its key (Z, or
 * LOC at NEWOPS 06070 for an interpretive key) and runs the file's code
 * and the interpreter until control leaves: a basic address of another
 * file (PHASCHNG, BANKCALL, IBNKCALL, POSTJUMP, ENDOFJOB, GOTOPOOH), the
 * target of an RTB (R51P63), NEWOPS with LOC outside the file (a CALL or
 * STCALL), POODOO at a UNIT or ABVAL alarm, or CHANG2 at a DANZIG with
 * NEWJOB positive.
 *
 *   P63LM 32,2776      PHASCHNG; 32,3000 BANKCALL R02BOTH
 *   32,3002            R02BOTH's return: the P63 set-up, channel 12, the
 *                      flags and the ignition algorithm's set-up to
 *                      STCALL TPIP RP-TO-R
 *   32,3040 3044 3051  RP-TO-R's, GUIDINIT's and LEMPREC's returns: on to
 *                      IGNALOOP's STCALL R MUNGRAV
 *   32,3100            MUNGRAV's return: STCALL GDT/2 ?GUIDSUB
 *   DDUMCALC 32,3102   ?GUIDSUB's return (POSTJUMP): the TDEC1 iteration
 *                      to CALL INTSTALL, or DDUMGOOD to IGNALGRT's PHASCHNG
 *   32,3146 3161       INTSTALL's and INTEGRVS's returns (GOTO IGNALOOP)
 *   32,3204 3207       PHASCHNG's return (BANKCALL STCLOK2) and STCLOK2's
 *                      (TCF ENDOFJOB)
 *   ASTNRET 32,3210    the job FINDVAC starts: SSP QMAJ, RTB R51P63
 *   P63SPOT2 32,3215   R51P63's GOTO QMAJ: the KALCMANU set-up, IBNKCALL
 *                      PFLITEDB; 32,3230 and 3233 its and R60LEM's returns
 *   P63SPOT3 32,3235   PHASCHNG's return: the LR antenna test, GOPERF1 or
 *                      SETPOS1; 32,3245-3247 GOPERF1's returns; 32,3251
 *                      SETPOS1's return (POSTJUMP BURNBABY)
 *   LANDJUNK 31,2144   P68: PHASCHNG; 31,2146 2151 2153 its, ZATTEROR's and
 *                      SETMAXDB's returns, to CALL LAT-LONG
 *   31,2167 2176       LAT-LONG's and R-TO-RP's returns: RLS, the V06N43
 *                      display; 31,2203-2205 GOFLASH's returns
 *   31,2213            REFMF's return: EXIT, TCF GOTOPOOH
 *
 * io is lm_lld_io: channel 12 (WAND) and channel 33 (RAND); INHINT, RELINT
 * and TC INTPRET set control.interrupt_enabled.
 *
 * Refused (EXEC_OUT_OF_SCOPE): a basic key under an FB other than the
 * key's bank, an interpretive key whose BANKSET or FB is not the key's
 * bank, a work area that is not one of the five VAC areas with PUSHLOC
 * inside it, EDOP not +0, NEWJOB negative at a DANZIG, DDUMCALC with
 * overflow in A (TS NIGNLOOP would skip TC INTPRET), a fixed operand
 * outside the certified data, a missing io record or undo image. A
 * refusal can come after the run has started; the image and io are then
 * restored from io->undo, which the caller supplies. */
typedef struct {
 p66_bias_io control;
 agc_word chan12,chan33;
 apollo_overlay_state *undo;   /* scratch for the entry image */
} lm_lld_io;
extern const agc_routine_table lm_lunar_landing_routines;
extern const agc_routine_table lm_lunar_landing_interpretive;
#define LLD_BASIC_ROUTINES 21u
#define LLD_INTERPRETIVE_ROUTINES 10u
#define LLD_ROUTINES (LLD_BASIC_ROUTINES+LLD_INTERPRETIVE_ROUTINES)
/* Physical keys, basic then interpretive, in table order. */
extern const uint16_t lld_routine_keys[LLD_ROUTINES];
#define LM_LLD_FB31 062000u
#define LM_LLD_FB32 064000u
#endif
