/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, both missions' (Luminary099 pp. 658-698, Comanche055
 * pp. 461-500): the rendezvous programs after CDH. P34 and P74, transfer
 * phase initiation (TPI); P35 and P75, the mid-course corrections; and
 * the subroutines they share. The faithful C is c64-port/src/
 * lm_p34_p35.c and cm_p34_p35.c.
 *
 * Written part by part, both missions in each part. Part 1 is P34 and
 * P74 (p34.c), in bank 35 of both missions, and the display subroutines
 * they call (displays.c): DISPLAYE, P3XORP7X and VNPOOH. Part 2 (p35.c)
 * is P35 and P75, and S33/34.1. Part 3 is the subroutines S34/35.1 to
 * S34/35.4, LOMAT, GOINT and INTINT (s34_35.c), and S34/35.5, VN1645 and
 * the clock routines after it (vn1645.c). Part 4 is INITVEL (initvel.c).
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

/* ---- Part 2: P35 and P75, and S33/34.1 (p35.c) ---- */

extern const struct interpretive_code P35_CODE, S33_34_1_CODE, FIGTIME_CODE, OKMAX_CODE;

void p35(void);                     /* P35 */
void p35_after_avflaga(void);       /* P35 +1 */
void p75(void);                     /* P75 */
void p75_after_avflagp(void);       /* P75 +1 */
void p35_p75a(void);                /* P35/P75A */
void p35_p75a_after_p20flgon(void); /* P35/P75A +2, the TC INTPRET */

/* ---- Part 3: the S34/35 subroutines (s34_35.c), and the displays at
 * the end of each pass, VN1645 and its clock (vn1645.c) ---- */

extern const struct interpretive_code S34_35_1_CODE, S34_35_2_CODE, S34_35_3_CODE, LOMAT_CODE,
    S34_35_5_CODE, S34_35_5_LOADED_CODE, FLAGOFF_CODE, VN1645_CODE, GETMGA_CODE, CLUPDATE_CODE;

void s34_35_5_after_exit(void); /* FLAGON's EXIT: verb 06 noun 81 */
#if LUMINARY099
void s34_35_5_terminate(void); /* GOFLASH's returns */
void s34_35_5_proceed(void);
void s34_35_5_recycle(void);
void s34_35_5_compare(void);      /* FLAGON +013: the delta V loaded? */
void s34_35_5_after_upflag(void); /* FLAGON +034, the TC INTPRET */
#else
void s34_35_5_after_vnpooh(void); /* FLAGON +6, the TC INTPRET */
#endif
void flagoff_after_exit(void);   /* FLAGOFF's EXIT: verb 06 noun 59 */
void flagoff_after_vnpooh(void); /* FLAGOFF +5, the TC INTPRET */
void vn1645_after_exit(void);    /* VN1645's EXIT: P3XORP7X */
void vn1645_p3x(void);           /* P3XORP7X's returns */
void vn1645_p7x(void);
void vn1645_mga(void);            /* GET45 -5, the TC INTPRET */
void vn1645_clock(void);          /* GET45's EXIT: COMPTGO */
void vn1645_after_comptgo(void);  /* GET45 +2 */
void vn1645_after_delayjob(void); /* GET45 +7: verb 16 noun 45 */
void vn1645_terminate(void);      /* GOFLASH's returns */
void vn1645_proceed(void);
void vn1645_recycle(void);
void kilclock(void);                /* KILCLOCK */
void n45proc(void);                 /* N45PROC */
void n45proc_after_phaschng(void);  /* N45PROC +6 */
void clupdate(void);                /* CLUPDATE */
void clupdate_after_phaschng(void); /* CLUPDATE +4, the TC INTPRET */

/* ---- Part 4: INITVEL (initvel.c), in bank 11 ---- */

extern const struct interpretive_code INITVEL_CODE;

/* ---- Part 5: MIDGIM and SELECTMU (midgim.c), PERIAPO to SHIFTR1
 * (periapo.c); the LM's in banks 6, 10 and 23, the CM's in 4 and 22 ---- */

extern const struct interpretive_code MIDGIM_CODE, SELECTMU_CODE, PERIAPO_CODE, PRECSET_CODE,
    VECSHIFT_CODE;

#endif
