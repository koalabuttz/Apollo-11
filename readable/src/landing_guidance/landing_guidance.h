/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc (Luminary099 pp. 798-828): the
 * landing's guidance, written part by part. So far: the phase tables and
 * GUIDSUB (entries.c, pp. 798-799), the main guidance equation to the
 * exit logic (quadguid.c, pp. 808-811), the exits (exits.c, pp. 811-814),
 * the displays (displays.c, pp. 814-815), P65 and P67 (vertguid.c, pp.
 * 815-816), P66's rate of descent mode (p66.c, pp. 816-819), the
 * redesignator and rate of descent traps (redesignator.c, pp. 820-822),
 * the root finder ROOTPSRS (rootpsrs.c, pp. 822-825), and INTPRETX and
 * FASTCHNG (subroutines.c, pp. 825-826). The faithful C is
 * c64-port/src/lm_llge.c. Steps more than one file takes are in
 * indexing.h.
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

/* The phase tables and entry points -- entries.c */
void tcf_table_word(word table, word at);   /* a word of AFTRGUID, WHATEXIT, WHATDISP */
void tcf_by_phase(word table, unsigned at); /* INDEX at; TCF table */
void guidsub(void);                         /* GUIDSUB */
void lunland(void);                         /* LUNLAND */
void calcrgvg(void);                        /* CALCRGVG */

/* QUADGUID to EXTLOGIC -- quadguid.c */
void quadguid(void);                /* QUADGUID */
void afcclend_after_exit(void);     /* AFCCLEND's EXIT +1: TC FASTCHNG */
void afcclend_after_fastchng(void); /* its return; BRSPOT4 */
void brspot4(void);                 /* BRSPOT4 */
void cgcalc(void);                  /* CGCALC */
void extlogic(void);                /* EXTLOGIC */
void exspot1(void);                 /* EXSPOT1 */
void exspot1_after_fastchng(void);  /* its TC FASTCHNG's return */
extern const struct interpretive_code QUADGUID_CODE, CGCALC_CODE;

/* The exits -- exits.c */
void exgsub(void);                /* EXGSUB */
void exgsub_after_exit(void);     /* its EXIT +1 */
void exgsub_postjump(void);       /* +3: TC POSTJUMP, CADR DDUMCALC */
void exbrak(void);                /* EXBRAK */
void exnorm(void);                /* EXNORM */
void exnorm_after_exit(void);     /* its EXIT +1 */
void unwcloop(void);              /* UNWCLOOP */
void steer(void);                 /* STEER? */
void exvert(void);                /* EXVERT */
void ratestop(void);              /* RATESTOP */
void gdump1(void);                /* GDUMP1 */
void gdump1_after_throttle(void); /* GDUMP1 +1: TC INTPRET */
extern const struct interpretive_code EXGSUB_CODE, EXBRAK_CODE, EXNORM_CODE, GDUMP1_CODE;

/* The displays -- displays.c */
void dispexit(void);             /* DISPEXIT */
void dispexit_plus3(void);       /* DISPEXIT +3 */
void p63disps_phaschng(void);    /* P63DISPS -2 */
void p63disps(void);             /* P63DISPS */
void dispcomn(void);             /* DISPCOMN */
void endlljob(void);             /* ENDLLJOB */
void p64disps(void);             /* P64DISPS */
void p64disps_terminate(void);   /* REFLASHR's first return: TCF GOTOPOOH */
void p64ceed(void);              /* P64CEED */
void p64ceed_after_upflag(void); /* UPFLAG's return: TCF ENDOFJOB */
void red_over(void);             /* RED-OVER */
void redes_ok(void);             /* REDES-OK */
void vertdisp(void);             /* VERTDISP */

/* The vertical phase's guidance -- vertguid.c */
void vertguid(void);               /* VERTGUID */
void p65vert(void);                /* P65VERT */
void p66vert(void);                /* P66VERT */
void p67vert(void);                /* P67VERT */
void p67vert_after_phaschng(void); /* P67VERT +2: TC INTPRET */
extern const struct interpretive_code P65VERT_CODE, P67VERT_CODE;

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
void intpretx(void);            /* INTPRETX */
void alm1406_after_alarm(void); /* 1406ALM's ALARM return: TCF RATESTOP */
void fastchng(void);            /* FASTCHNG */
#endif

#endif
