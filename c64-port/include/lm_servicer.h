/* SPDX-License-Identifier: MIT */
#ifndef LM_SERVICER_H
#define LM_SERVICER_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 SERVICER.agc, all of it (bank 37 03410-03654, bank 33
 * 02206-02446 and 02447-03776, bank 22 03661-03710, bank 27 03321-03357,
 * bank 32 03770-03772, bank 23 02461-02510, bank 21 02130-02153 and bank 34
 * 03716-03774), as native routines of the control-transfer ABI. See
 * docs/LM-SERVICER.md.
 *
 * Every basic instruction is one ai_* effect on the image in source order;
 * the interpretive passages (GETABVAL, AVERAGEG, AVETOMID's call, NORMLIZE,
 * the R29 and altitude block, CALCGRAV, CALCRVG, RVBOTH, MUNRVG, MUNGRAV,
 * the landing radar updates and SETPOS) run the compiled lm_interp_steps /
 * lm_interp_orders paths for their words. A routine starts at its key (Z,
 * or LOC at NEWOPS 06070 for the interpretive keys) and runs the file's code
 * until control leaves for another file: a basic address (PHASCHNG,
 * BANKCALL, NOVAC, WAITLIST, TASKOVER, ENDOFJOB, SHORTMP, BRANCH, ...), the
 * return of a subroutine entered from outside (TC Q, TC A, TC LRADRET), or
 * NEWOPS with LOC outside the file (a CALL), or SGNAGREE through RTB.
 *
 * The keys are every label of the file and every return point of a call
 * (lmsv_routine_keys): to another file, each return of a multi-return
 * callee included (BRANCH, RADSTALL, PRIOLARM, BANKCALL of ALTCHK), and
 * within the file (TC QUIKFAZ5, GNURVST, COPYCYC, SETPOS and the rest; the
 * interpretive CALLs and STCALLs of CALCGRAV, CALCRVG, MUNGRAV and
 * AVETOMID), so that each C64 segment group's own keys reach all of its
 * code.
 *
 * io is lm_sv_io: channel 11 (WOR and WAND DSALMOUT), channels 12, 32 and
 * 33 (RAND); INHINT, RELINT and TC INTPRET set control.interrupt_enabled.
 *
 * Refused (EXEC_OUT_OF_SCOPE), with the image and io restored from
 * io->undo when the run has started:
 * - an interpretive key whose BANKSET or FB is not its bank, a work area
 *   that is not one of the five VAC areas with PUSHLOC inside it, EDOP not
 *   +0, NEWJOB negative at a DANZIG, a fixed operand outside the constants
 *   the file's orders name, an erasable operand whose words run past the
 *   end of erasable into fixed memory (an indexed address such as DMP*
 *   VZSCAL,2 with an index no producer stores);
 * - an INDEX whose value is not one the file's producers store: ITEMP1 in
 *   COPYCYC (an even count up to 22), A at ALTCHK (0, 2, 4 or 6), FIXLOC at
 *   VELUPDAT (a VAC area), VSELECT* (0, 1 or 2), BUF in GNURVST (0 or 6), Q
 *   in SETPOS (0 or 2);
 * - a TC Q, TC A or TC LRADRET whose word is not a TC into fixed memory;
 * - LOTHRUST with PHASE4 negative, whose CCS would run the PHASCHNG
 *   operand word as an instruction;
 * - a computed transfer, a TS skip on overflow or a CCS outcome landing on
 *   a word that runs only after the EXTEND or INDEX before it (STILBADH or
 *   STILBADV -0, for one);
 * - a missing io record or undo image. */
typedef struct {
 p66_bias_io control;
 agc_word chan11,chan12,chan32,chan33;
 apollo_overlay_state *undo;   /* scratch for the entry image */
} lm_sv_io;
extern const agc_routine_table lm_servicer_routines;
extern const agc_routine_table lm_servicer_interpretive;
/* The flag ADRES operands of the file's TC UPFLAG and DOWNFLAG, for the
 * flag routines' operand lookup. */
#define LMSV_FLAG_SITES 13u
extern const agc_site_operand lm_servicer_flag_sites[LMSV_FLAG_SITES];
#define LMSV_BASIC_ROUTINES 164u
#define LMSV_INTERPRETIVE_ROUTINES 24u
#define LMSV_ROUTINES (LMSV_BASIC_ROUTINES+LMSV_INTERPRETIVE_ROUTINES)
/* Every key (physical), basic then interpretive. */
extern const uint16_t lmsv_routine_keys[LMSV_ROUTINES];
#endif
