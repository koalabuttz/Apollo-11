/* SPDX-License-Identifier: MIT */
/* GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc, the LM's (Luminary099 pp.
 * 651-653): P31, general Lambert aimpoint guidance, and REVN1645, the
 * display P30 and P31 end in. The faithful C is c64-port/src/
 * lm_lambert_aim.c (and the older c64-port/src/lm_p31_routines.c, with
 * the same keys).
 *
 * P31's basic code (bank 35) sets P20's flags (P20FLGON) and asks the
 * astronaut, through VNPOOH, for the time of ignition (verb 06 noun 33).
 * Then its interpretive code integrates the LM to TIG, finds the velocity
 * that reaches the target RTARG in DELLT4 (INITVEL, a Lambert solution)
 * and the delta V to it, and the orbit after the burn; the basic code
 * after its EXIT shows the delta V (verb 06 noun 81) and apogee, perigee
 * and delta V (verb 06 noun 42), and REVN1645 shows verb 16 noun 45 until
 * the burn program is chosen.
 *
 * The interpretive code is in the listing's notation (P31_CODE,
 * REVN1645_CODE), run by the readable interpreter. The entries below are
 * the basic code's: P31, the words P20FLGON and VNPOOH return to, and the
 * word after the EXIT. The binding is src/binding/lambert_aimpoint_routines.c. */
#ifndef READABLE_LAMBERT_AIMPOINT_H
#define READABLE_LAMBERT_AIMPOINT_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code P31_CODE, REVN1645_CODE;

void p31(void);                /* P31 */
void p31_after_p20flgon(void); /* P31 +1: the word P20FLGON returns to */
void p31_after_v06n33(void);   /* P31 +3: the word VNPOOH returns to */
void p31_after_exit(void);     /* P31 +63: after the EXIT */
void p31_after_v06n81(void);   /* P31 +65: the word VNPOOH returns to */
void p31_after_v06n42(void);   /* P31 +67: the word VNPOOH returns to */
#endif

#endif
