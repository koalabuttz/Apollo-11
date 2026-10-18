/* SPDX-License-Identifier: MIT */
#ifndef LM_Q_R_AXIS_H
#define LM_Q_R_AXIS_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 Q_R-AXIS_RCS_AUTOPILOT.agc, all of it (bank 17, 02072-03251,
 * and MAKERUPT at 07754-07755), as native routines of the control-transfer
 * ABI; io is p66_bias_io (the interrupt-enable bit); channels 4, 5, 13 and
 * 31 and the interrupt-in-progress bit are the program's agc_hardware
 * (include/lm_flight_io.h).
 *
 *   CALLQERR  17,2072  from P-AXIS: Q,R errors (QERRCALC), the control
 *                      system choice (GTS, TRYGTS, RCS), X-translation, the
 *                      minimum impulse, rate command and attitude hold
 *                      modes, TJLAW and the Q,R jet selection for the U and
 *                      V axes, to the first call that leaves the file or to
 *                      CLOSEOUT's TCF NOQRSM: TC ALARM, IBNKCALL (ZATTEROR,
 *                      TIMEGMBL, SPSRCS), TC TJETLAW, TC WRITEP (+4, +13),
 *                      TC WRITEP -1 (+4, +13), TC WRITEU -1, TC WRITEV -1,
 *                      DTCB to GTS, or TC RETJADR to a word other than the
 *                      file's three return addresses
 *   17,2172 17,2214 17,2454 17,2620 17,2714 17,2724 17,2775 17,3004
 *   17,3042 17,3074 17,3103 17,3224
 *                      the returns from those calls, continuing the same
 *   CLOSEOUT  17,3236  from GTS: TC MAKERUPT, whose EDRUPT runs A (ADRRUPT)
 *                      as TC ENDJASK; the job's registers back; TCF NOQRSM
 *   QERRCALC  17,2626  Q,R attitude errors; TC Q
 *   JTLST     17,3114  the T6 jet list; TC Q
 *   ROT-TOUV  17,3146  Q,R to U,V; TC Q
 *
 * Refused (EXEC_OUT_OF_SCOPE, image and io unchanged): an entry under an FB
 * other than bank 17 or an EB other than EBANK6 (EBANK= CDUXD); a missing
 * io record; a QERRCALC, JTLST or ROT-TOUV return in Q that is not a TC
 * into fixed memory; a CALLQERR or return entry with SKIPU or SKIPV other
 * than 0 or 4 (INDEX AXISCTR; INDEX SKIPU; TCF +1 would jump anywhere; every
 * producer stores 0 or 4); CALLQERR with ALLOWGTS -0 or channel 5 negative
 * (CCS would enter READ CHAN5 or DCA GTSCADR without its EXTEND; ALLOWGTS
 * is stored as +0 or 1, channel 5 holds eight jet bits); a return
 * entry with AXISCTR other than 0 or 1, NUMBERT outside 0-4 or SENSETYP
 * above 5 (they index the jet tables; the file and TJETLAW store these). */
extern const agc_routine_table lm_q_r_axis_routines;
/* The entries an attitude-hold or jet-firing DAP pass reaches (the pinned DAP set). */
extern const agc_routine_table lm_q_r_axis_hot_routines;
extern const agc_routine_table lm_q_r_axis_tg_hot_routines;
/* The inline operands read through Q by the file's TC ALARM calls (OCT
 * 02002 in X-translation, OCT 02004 in SELCTSUB), as compiled site
 * operands. */
#define LM_QR_SITES 2u
extern const agc_site_operand lm_q_r_axis_sites[LM_QR_SITES];
/* This file's words that no code reaches (1/10S, BIT8,9, SCLNORM, and
 * ZEROTJ's two words), for a program's passive fixed_data reads: the word
 * at a physical address, or false. */
bool lm_q_r_axis_data(uint16_t physical,agc_word *word);
#define LM_QR_FB17 036000u
#define LM_QR_EB 03000u
#define LM_QR_CALLQERR 02072u
#define LM_QR_ALARMRET 02172u
#define LM_QR_MINRET 02214u
#define LM_QR_ENTERUVRET 02454u
#define LM_QR_TIMEGMBLRET 02620u
#define LM_QR_TJETRET 02714u
#define LM_QR_SPSRET 02724u
#define LM_QR_WRITEPRET 02775u
#define LM_QR_NOROTATRET 03004u
#define LM_QR_NOTMINRET 03042u
#define LM_QR_WRITEURET 03074u
#define LM_QR_WRITEVRET 03103u
#define LM_QR_SELALARMRET 03224u
#define LM_QR_CLOSEOUT 03236u
#define LM_QR_QERRCALC 02626u
#define LM_QR_JTLST 03114u
#define LM_QR_ROTTOUV 03146u
/* The parts' keys (src/lm_q_r_axis.c): where one part hands Z to the next. */
#define LM_QR_RCS 02107u
#define LM_QR_RCS1 02110u  /* RCS +1 */
#define LM_QR_QRCONTRL 02200u
#define LM_QR_MINRTN 02273u
#define LM_QR_CHEKSTIK 02323u
#define LM_QR_BACKHAND 02532u
#define LM_QR_TRYGTS 02574u
#define LM_QR_STILLRCS 02661u  /* STILLRCS (ATTSTEER) */
#define LM_QR_TJLAW3 02671u  /* TJLAW +3 */
#define LM_QR_LOOPER 03005u
#define LM_QR_DOSKIP 03010u
#define LM_QR_DOSKIP1 03011u  /* DOSKIP +1 */
#define LM_QR_XTRANS 03063u
#define LM_QR_FIRST 02072u
#define LM_QR_LAST 03251u
#define LM_QR_MAKERUPT 07754u
#define LM_QR_RETJADR 01477u
/* Where the file's calls leave it. */
#define LM_QR_ALARM 05567u
#define LM_QR_IBNKCALL 04674u
#define LM_QR_TJETLAW 03252u   /* bank 17 */
#define LM_QR_WRITEP 05754u
#define LM_QR_WRITEP1 05753u  /* WRITEP -1 */
#define LM_QR_WRITEU1 05757u  /* WRITEU -1 */
#define LM_QR_WRITEV1 05770u  /* WRITEV -1 */
#define LM_QR_GTS 03077u      /* bank 21 */
#define LM_QR_NOQRSM 05272u
#endif
