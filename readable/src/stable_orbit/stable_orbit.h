/* SPDX-License-Identifier: MIT */
/* STABLE_ORBIT_-_P38-P39.agc, both missions' (Luminary099 pp. 723-730,
 * Comanche055 pp. 525-532): the stable orbit rendezvous programs, P38 and
 * P78, and the stable orbit midcourse programs, P39 and P79. The faithful
 * C is c64-port/src/lm_stable_orbit.c and cm_stable_orbit.c.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter; the entries below are the basic code's. Each ends where its
 * code leaves it: at a TC out of the file (Z the routine, Q the return), at
 * a TC INTPRET (Z INTPRET) or, for the CM, at the TC RTRN that runs the word
 * VNDSPLY keeps there (Z RTRN). The binding, src/binding/
 * stable_orbit_routines.c, runs on from a TC INTPRET into the interpreter,
 * as the faithful port does. */
#ifndef READABLE_STABLE_ORBIT_H
#define READABLE_STABLE_ORBIT_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* The displays' CAF lines, which have no labels: CENTANG's (the LM's P78
 * +8, after its BANKCALL P20FLGON and its CAF DECTWO; TS NN, the CM's P78
 * +4), the option's after it, and DELTA R's (after OPTN1's EXIT: OPTN1 +10
 * in the LM, OPTN1 +11 in the CM, which stores DSPTEM1 there). */
#if LUMINARY099
#define CENTANG_DISPLAY (P78 + 010u)
#define DELTAR_DISPLAY  (OPTN1 + 012u)
#else
#define CENTANG_DISPLAY (P78 + 4u)
#define DELTAR_DISPLAY  (OPTN1 + 013u)
#endif
#define OPTION_DISPLAY (CENTANG_DISPLAY + 015u)

extern const struct interpretive_code P38_CODE, JUNCTN1_CODE, MAINRTN1_CODE, DSPLY81_CODE, P39_CODE;

void p38(void);                   /* P38 */
void p38_active(void);            /* P38 +2 (LM), +1 (CM): TC +3, TC +2 */
void p78(void);                   /* P78 */
void p78_active(void);            /* P78 +2 (LM), +1 (CM) */
void p38_tig(void);               /* P78 +4 (LM), +2 (CM): V06N33 */
void p38_centang(void);           /* V06N55 */
void p38_centang_terminate(void); /* its returns */
void p38_centang_proceed(void);
void p38_centang_recycle(void);
void p38_centang_blank(void);    /* immediate return */
void p38_centang_end(void);      /* after BLANKET */
void p38_option_terminate(void); /* V04N06's returns */
void p38_option_proceed(void);
void p38_option_recycle(void);
void p38_option_blank(void);
void p38_option_end(void);
void p38_deltar(void); /* V06N57, after OPTN1's EXIT */
void p38_deltar_terminate(void);
void p38_deltar_proceed(void);
void p38_deltar_recycle(void);
void p38_deltar_blank(void);
void p38_deltar_end(void);
void p38_intercept_done(void); /* V06N34's return: TC INTPRET */
void p39p79_exit(void);        /* P39P79 +1: TC DSPLY81 */
void dsply58_exit(void);       /* DSPLY58 +1: V06N58 */
void dsply81(void);            /* DSPLY81: V06N81 */
void dsply81_done(void);       /* DSPLY81 +2: TC INTPRET */
void p39(void);                /* P39 */
void p39_active(void);         /* P39 +2 (LM), +1 (CM) */
void p79(void);                /* P79 */
void p79_active(void);         /* P79 +2 (LM), +1 (CM) */
void p39_p79a_done(void);      /* P39/P79A +3 (LM), +2 (CM): TC INTPRET */
void vndsply_terminate(void);  /* VNDSPLY +6 */
void vndsply_proceed(void);    /* VNDSPLY +7: TC RTRN */
void vndsply_recycle(void);    /* VNDSPLY +8 */
#if !LUMINARY099
void rtrn(void); /* the CM's RTRN, E4,1767: the TC VNDSPLY saved there */
#endif

#endif
