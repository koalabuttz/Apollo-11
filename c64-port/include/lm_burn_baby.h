/* SPDX-License-Identifier: MIT */
#ifndef LM_BURN_BABY_H
#define LM_BURN_BABY_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, all of it
 * (bank 36 02022-03146, bank 35 03747-03775, bank 17 02000-02026, bank 27
 * 02200-02245 and KILLTASK in fixed-fixed 06027-06036), as native routines
 * of the control-transfer ABI. See docs/LM-BURN-BABY.md.
 *
 * Every basic instruction is one ai_* effect on the image in source order;
 * the two interpretive passages (P41SPOT and the ABVEL display) run the
 * compiled lm_interp_steps / lm_interp_orders paths for their words. A
 * routine starts at its key (Z, or LOC at NEWOPS 06070 for the four
 * interpretive keys) and runs the file's code until control leaves for
 * another file: a basic address (PHASCHNG, BANKCALL, TWIDDLE, NOVAC,
 * TASKOVER, ENDOFJOB, ...), the return in Q of a subroutine entered from
 * outside (NOULLAGE, ONULLAGE, KILLTASK), or NEWOPS with LOC outside the
 * file (a CALL or STCALL).
 *
 * The keys are every label of the file and every return point of a call to
 * another file (lmbb_routine_keys). The ignition tables are executed as the
 * original does: INDEX WHICH; TCF n runs the table word at WHICH +n.
 *
 * io is lm_bb_io: channel 11 (DSALMOUT, RAND and WRITE), channel 14 (WOR)
 * and channel 30 (RAND); INHINT, RELINT and TC INTPRET set
 * control.interrupt_enabled.
 *
 * Refused (EXEC_OUT_OF_SCOPE), with the image and io restored from
 * io->undo when the run has started:
 * - a key under an FB other than its bank (KILLTASK aside), an
 *   interpretive key whose BANKSET or FB is not bank 36, a work area that
 *   is not one of the five VAC areas with PUSHLOC inside it, EDOP not +0,
 *   NEWJOB negative at a DANZIG;
 * - an INDEX WHICH with WHICH not one of the six table addresses the
 *   file's producers store (P12TABLE, P40TABLE, P41TABLE -5, P42TABLE,
 *   P63TABLE, ABRTABLE), or a table word that is data, not a TCF;
 * - CLOKJOB with DISPDEX -0, or negative and not selecting one of the
 *   display branches of its INDEX A; TCF DISPNOT -1 (the file stores
 *   -CNTDNDEX, -VB97DEX, -VB99DEX and -BLANKDEX; the -25 entry is code
 *   too);
 * - NULLCLOK's TC P40/RET with P40/RET not one of the three returns its
 *   QXCH stores (36,3014, 3057, 3074);
 * - REDO4.2, P40SJUNK or COMFAIL with a negative PHASE5, PHASE3 (or -0)
 *   or PHASE6, whose CCS would run the CADR word after it;
 * - a TC Q (NOULLAGE, ONULLAGE) or KILLTASK entered with Q not a TC word
 *   into fixed memory, or KILLTASK with its CADR not in the compiled site
 *   tables;
 * - a computed transfer landing on a word that runs only after the EXTEND
 *   or INDEX before it;
 * - overflow in A at a TS of MAKECADR's CADR or KILLTSK2's TS ITEMP3;
 * - a missing io record or undo image. */
typedef struct {
 p66_bias_io control;
 agc_word chan11,chan14,chan30;
 apollo_overlay_state *undo;   /* scratch for the entry image */
} lm_bb_io;
extern const agc_routine_table lm_burn_baby_routines;
extern const agc_routine_table lm_burn_baby_interpretive;
/* KILLTASK's callers' CADRs (every TC KILLTASK in Luminary099). */
#define LMBB_SITES 11u
extern const agc_site_operand lm_burn_baby_sites[LMBB_SITES];
/* The flag ADRES operands of the file's TC UPFLAG, DOWNFLAG and INVFLAG,
 * for the flag routines' operand lookup. */
#define LMBB_FLAG_SITES 14u
extern const agc_site_operand lm_burn_baby_flag_sites[LMBB_FLAG_SITES];
#define LMBB_BASIC_ROUTINES 160u
#define LMBB_INTERPRETIVE_ROUTINES 14u
#define LMBB_ROUTINES (LMBB_BASIC_ROUTINES+LMBB_INTERPRETIVE_ROUTINES)
/* Every key (physical), basic then interpretive. */
extern const uint16_t lmbb_routine_keys[LMBB_ROUTINES];
#endif
