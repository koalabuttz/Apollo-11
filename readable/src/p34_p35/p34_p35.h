/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, both missions' (Luminary099 pp. 658-698, Comanche055
 * pp. 461-500): the rendezvous programs after CDH. P34 and P74, transfer
 * phase initiation (TPI); P35 and P75, the mid-course corrections; and
 * the subroutines they share. The faithful C is c64-port/src/
 * lm_p34_p35.c and cm_p34_p35.c.
 *
 * Written part by part, both missions in each part. Part 1 is P34 and
 * P74 (p34.c), in bank 35 of both missions, and the display subroutines
 * they call (displays.c): DISPLAYE, P3XORP7X and VNPOOH.
 *
 * The interpretive code is in the listing's notation, run by the readable
 * interpreter. The entries below are the basic code's: each program's
 * start, the words a routine outside the file comes back to, and the
 * words after each EXIT. The binding is src/binding/p34_routines.c. */
#ifndef READABLE_P34_P35_H
#define READABLE_P34_P35_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* ---- Part 1: P34 and P74 (p34.c) ---- */

extern const struct interpretive_code P34_TTPI_CODE, P34_SWCHCLR_CODE, P34_P74D_CODE, P34_P74E_CODE,
    P34_N58_CODE;
#if !LUMINARY099
extern const struct interpretive_code P34_NN_CODE;
#endif

void p34(void);                     /* P34 */
void p34_after_avflaga(void);       /* P34 +1 */
void p74(void);                     /* P74 */
void p34_p74a(void);                /* P34/P74A, AVFLAGP's return too */
void p34_p74a_after_p20flgon(void); /* P34/P74A +1 */
void p34_p74a_after_vnpooh(void);   /* P34/P74A +3 */
#if !LUMINARY099
void p34_p74a_after_nn(void); /* the CM's P34/P74A +7, after the EXIT */
#endif
void p34_p74a_after_displaye(void); /* the TC INTPRET after DISPLAYE */
void p34_alarm(void);               /* after the BZE EXIT: alarm 611 */
void p34_after_alarm(void);         /* and verb 05 noun 09 */
void p34_terminate(void);           /* GOFLASH's returns */
void p34_proceed(void);
void p34_recycle(void);
void swchclr_after_exit(void);     /* SWCHCLR's EXIT */
void swchclr_after_displaye(void); /* DISPLAYE's return */
void p34_p74d_after_exit(void);    /* P34/P74D's EXIT */
void p34_p74e(void);               /* P34/P74E, VNPOOH's return */
void p34_p74e_after_exit(void);    /* verb 06 noun 58 */
void p34_p74e_after_v06n58(void);  /* VNPOOH's return */

/* The display subroutines (displays.c): each entry, and GOFLASH's three
 * returns after it. */
void displaye(void);
void displaye_terminate(void);
void displaye_proceed(void);
void displaye_recycle(void);
void p3xorp7x(void);
void vnpooh(void);
void vnpooh_terminate(void);
void vnpooh_proceed(void);
void vnpooh_recycle(void);
#if !LUMINARY099
void vnpooh_restore(void); /* VNPOOH +014, after the CM's bank is kept */
#endif

#endif
