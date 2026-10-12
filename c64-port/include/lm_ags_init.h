/* SPDX-License-Identifier: MIT */
#ifndef LM_AGS_INIT_H
#define LM_AGS_INIT_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* Luminary099 AGS_INITIALIZATION.agc (R47, bank 32 02005-02216), all of
 * it, as native routines of the control-transfer ABI. See
 * docs/LM-AGS-INIT.md.
 *
 * Every basic instruction is one ai_* effect on the image in source order;
 * every interpretive order pair is the compiled lm_interp_steps /
 * lm_interp_orders path for its words. A routine starts at its key (Z, or
 * LOC at NEWOPS 06070 for an interpretive key) and runs the file's code
 * and the interpreter until control leaves: a basic address of another
 * file (ALARM, ENDEXT, BANKCALL, DOWNFLAG), the target of an RTB
 * (LOADTIME, VECSGNAG), NEWOPS with LOC outside the file (STCALL to
 * LEMPREC or CSMPREC, SCALEVEC's RVQ to a caller elsewhere), or CHANG2 at
 * a DANZIG with NEWJOB positive.
 *
 *   AGSINIT 32,2005    V47: REFSMFLG, ALARM 220 or the AGSK display
 *   32,2013            ALARM's return: TC ENDEXT
 *   32,2025-2027       GOMARKF's terminate, proceed and enter returns (the
 *                      proceed return runs AGSVCALC's flag settings to the
 *                      EXDSPRET display; enter takes the keyed AGSK)
 *   32,2046            EXDSPRET's return: TC INTPRET, RTB LOADTIME
 *   32,2051 2053 2055 2061 2063
 *                      the interpretive returns from LOADTIME, LEMPREC,
 *                      SCALEVEC, CSMPREC and SCALEVEC: the state vectors
 *                      to AGSBUFF, the AGS clock, EXIT, the AGS downlist
 *                      and DELAYJOB
 *   32,2144 2156       VECSGNAG's returns inside SCALEVEC (to its RVQ)
 *   32,2077            DELAYJOB's return: the old downlist, IMUSE, CKSTALL
 *   32,2114            CKSTALL's DELAYJOB return
 *   32,2117 2121 2122  IMUZERO's and IMUSTALL's returns
 *   32,2124            DOWNFLAG's return: the V50N16 display
 *   32,2127-2131       GOMARK3's returns: ENDEXT
 *
 * io is lm_ags_io; INHINT, RELINT and TC INTPRET set
 * control.interrupt_enabled.
 *
 * Refused (EXEC_OUT_OF_SCOPE): a basic key under an FB other than bank
 * 32, an interpretive key whose BANKSET or FB is not bank 32, a work area
 * that is not one of the five VAC areas with PUSHLOC inside it, EDOP not
 * +0, NEWJOB negative at a DANZIG, SCALEVEC's RVQ with a QPRET outside
 * switched fixed memory, a missing io record or undo image. A refusal can
 * come after the run has started; the image and io are then restored from
 * io->undo, which the caller supplies. */
typedef struct {
 p66_bias_io control;
 apollo_overlay_state *undo;   /* scratch for the entry image */
} lm_ags_io;
extern const agc_routine_table lm_ags_init_routines;
extern const agc_routine_table lm_ags_init_interpretive;
#define AGS_BASIC_ROUTINES 15u
#define AGS_INTERPRETIVE_ROUTINES 7u
#define AGS_ROUTINES (AGS_BASIC_ROUTINES+AGS_INTERPRETIVE_ROUTINES)
/* Physical keys, basic then interpretive, in table order. */
extern const uint16_t ags_routine_keys[AGS_ROUTINES];
/* The inline operands this file's TC ALARM and TC DOWNFLAG pass at Q (OCT
 * 220, ADRES NODOFLAG), as compiled site operands. */
#define LM_AGS_SITES 2u
extern const agc_site_operand lm_ags_init_sites[LM_AGS_SITES];
#define LM_AGS_FB32 064000u
#endif
