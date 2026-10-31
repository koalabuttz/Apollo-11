/* SPDX-License-Identifier: MIT */
/* R63.agc, the LM's only (Luminary099 pages 338-341): Verb 89, the
 * rendezvous final attitude. The faithful C is c64-port/src/lm_r63.c.
 *
 * "CALCULATES AND DISPLAYS FINAL FDAI BALL ANGLES TO POINT LM +X OR +Z
 * AXIS AT CSM." The crew keys V89E in P00. V89CALL checks the IMU
 * (R02BOTH) and asks which axis (V04N12, R2: 1 for +Z, 2 for +X). V89RECL
 * brings both state vectors to a minute from now (CSMCONIC, LEMCONIC),
 * takes the line of sight from the LM to the CSM in stable member
 * coordinates, and VECPOINT finds the gimbal angles that point the axis
 * along it; BALLANGS turns them into FDAI ball angles for V06N18. RECYCLE
 * computes again; PROCEED clears 3AXISFLG and maneuvers the LM there
 * (R60LEM).
 *
 * Basic code and interpretive code in turn, in bank 26. The interpretive
 * code is in the listing's notation (V89RECL_CODE, ALINEX_CODE,
 * ALINEZ_CODE), run by the readable interpreter. The entries below are the
 * basic code's: the routine's start, V89RECL (RECYCLE's TC), each word a
 * call out of the file returns to (BANKCALL's, GOFLASH's terminate,
 * proceed and data in, DOWNFLAG's) and the words after each EXIT. The
 * binding is src/binding/r63_routines.c. */
#ifndef READABLE_R63_H
#define READABLE_R63_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
extern const struct interpretive_code V89RECL_CODE, ALINEX_CODE, ALINEZ_CODE;

void v89call(void);
void v89call_after_r02both(void);   /* V89CALL +2 */
void v89call_after_terminate(void); /* V89CALL +011: GOFLASH's returns */
void v89call_after_proceed(void);   /* V89CALL +012 */
void v89call_after_data_in(void);   /* V89CALL +013 */
void v89recl(void);
void v89recl_after_exit(void);       /* V89RECL +025 */
void v89call1_after_exit(void);      /* V89CALL1 +4 */
void v89call1_after_ballangs(void);  /* V89CALL1 +6 */
void v89call1_after_terminate(void); /* V89CALL1 +011: GOFLASH's returns */
void v89call1_after_proceed(void);   /* V89CALL1 +012 */
void v89call1_after_recycle(void);   /* V89CALL1 +013 */
void v89call1_after_v89recl(void);   /* V89CALL1 +014: the word after TC V89RECL */
void v89call1_after_downflag(void);  /* V89CALL1 +016 */
void v89call1_after_r60lem(void);    /* V89CALL1 +020 */
#endif

#endif
