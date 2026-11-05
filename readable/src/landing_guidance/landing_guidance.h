/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc (Luminary099 pp. 798-828): the
 * landing's guidance, written part by part. So far: P66's rate of descent
 * mode (p66.c, pp. 816-819), the redesignator and rate of descent traps
 * (redesignator.c, pp. 820-822), the root finder ROOTPSRS (rootpsrs.c,
 * pp. 822-825), and INTPRETX and FASTCHNG (subroutines.c, pp. 825-826).
 * The faithful C is c64-port/src/lm_llge.c.
 *
 * Each function runs from a word the program can resume at (an entry, a
 * subroutine's return, the word after an EXIT) until its code leaves the
 * file: at a TC or TCF out of it (Z the routine, Q the return), or at a TC
 * INTPRET, run on to NEWOPS (intpret()). The binding,
 * src/binding/llge_routines.c, runs one per key. */
#ifndef READABLE_LANDING_GUIDANCE_H
#define READABLE_LANDING_GUIDANCE_H

#include "../agc/agc.h"

#if LUMINARY099
#include "../interpreter/notation.h"

/* P66 -- p66.c */
void rodtask(void);               /* RODTASK */
void rodtask_after_findvac(void); /* its TCF TASKOVER */
void p66verta(void);              /* P66VERTA */
void p66verta_after_phaschng(void);
void rodcomp(void);                /* RODCOMP */
void itrpnt2_after_exit(void);     /* ITRPNT2 +1 */
void itrpnt2_after_throttle(void); /* ITRPNT2 +4: TC INTPRET */
void vhorcomp_after_exit(void);    /* VHORCOMP's EXIT +1 */
extern const struct interpretive_code ITRPNT1_CODE, VHORCOMP_CODE;

/* The redesignator and rate of descent traps -- redesignator.c */
void pitfall(void);         /* PITFALL */
void pitfall_not_p64(void); /* CHECKMM's return: TCF RESUME */
void pitfall_in_p64(void);  /* CHECKMM's return +1 */
void pitfall_after_twiddle(void);
void redesmon(void);          /* REDESMON */
void resetrpt_taskover(void); /* RESETRPT +3: TCF TASKOVER */
void descbits(void);          /* DESCBITS */

/* ROOTPSRS -- rootpsrs.c */
void rootpsrs(void);                  /* ROOTPSRS */
void rootpsrs_after_shortmp(void);    /* TC USPRCADR, CADR ABS */
void rootpsrs_after_abs(void);        /* the criterion; DERCLOOP */
void dercloop_after_dmpnsub(void);    /* DERCLOOP's DMPNSUB return */
void rootloop_after_derivative(void); /* ROOTLOOP's first POWRSERS return */
void rootloop_after_residual(void);   /* its second: TC USPRCADR, CADR DDV/BDDV */
void rootloop_after_ddv(void);        /* the root corrected; TC USPRCADR, CADR ABS */
void rootloop_after_abs(void);        /* the test: BADROOT, ROOTLOOP or ROOTSTOR */

/* The subroutines -- subroutines.c */
void intpretx(void); /* INTPRETX */
void fastchng(void); /* FASTCHNG */
#endif

#endif
