/* SPDX-License-Identifier: MIT */
/* P76.agc, both missions' (Luminary099 pp. 709-711, Comanche055 pp.
 * 511-513): P76, the target delta V program, verb 37 noun 76. The
 * faithful C is c64-port/src/lm_p76_file.c and cm_p76_file.c, and the
 * older modules lm_p76_routines.c and cm_p76_routines.c, with their own
 * keys for the basic code.
 *
 * The astronaut loads the delta V the other vehicle burned (verb 06 noun
 * 84) and when (noun 33, TIG). P76 integrates the other vehicle's state
 * vector to TIG, adds the delta V (turned from local vertical to the
 * reference frame), then integrates it on to the time of this vehicle's
 * state vector, so both, and the W-matrix, are for the same time.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter; the entries below are the basic code's. Each ends where its
 * code leaves it: at a TC out of the file (Z the routine, Q the return), at
 * a TC INTPRET (Z INTPRET) or at a TCF to ENDP76 (Z ENDP76). The binding,
 * src/binding/p76_routines.c, runs on from there as each faithful module
 * does. */
#ifndef READABLE_P76_H
#define READABLE_P76_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code UPDATE_CODE, ATOPOTH_CODE, OUT_CODE, P76SUB1_CODE;
#if LUMINARY099
extern const struct interpretive_code P76_CODE;
#endif

void p76(void); /* P76 */
#if LUMINARY099
void p76_intpret(void); /* P76 +2 */
#endif
void flash_delta_v(void); /* P76 +7 (LM), +2 (CM): V06N84 */
void delta_v_terminate(void);
void delta_v_proceed(void);
void delta_v_recycle(void);
void flash_tig(void); /* V06N84 +1's display */
void tig_terminate(void);
void tig_proceed(void);
void tig_recycle(void);
void tig_intpret(void);  /* the TC INTPRET after the displays */
void p76_phaschng(void); /* after MINIRECT's EXIT */
void p76_reintflg(void);
void p76_reintflg_intpret(void);
void p76_intwake(void); /* after ATOPOTH's EXIT */
void endp76(void);      /* ENDP76 */

#endif
