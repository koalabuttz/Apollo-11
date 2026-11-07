/* SPDX-License-Identifier: MIT */
/* P37_P70.agc, the CM's (Comanche055 pp. 890-933): P37, "RETURN TO
 * EARTH". "A RETURN TO EARTH TRAJECTORY IS COMPUTED PROVIDED THE CSM IS
 * OUTSIDE THE LUNAR SPHERE OF INFLUENCE AT THE" "TIME OF IGNITION.":
 * first a conic trajectory, shown to the crew, then if they want it a
 * precision one; on their final word the program "COMPUTES AND STORES THE
 * TARGET PARAMETERS FOR RETURN TO EARTH FOR USE BY SPS PROGRAM (P40) OR
 * RCS PROGRAM (P41)." The faithful C is c64-port/src/cm_p37_p70.c.
 *
 * The files, in the listing's order (written part by part, README.md):
 *   part 1, p37.c: P37's main line, the conic trajectory (pp. 890-895,
 *     bank 36); p37_displays.c: RTEVN, the precision display, the
 *     targeting and the end (P37G to P37W), and the display subroutines
 *     (pp. 895-899, bank 36);
 *   part 2, rtedisp.c: the alarm display, the time to a radius and the
 *     display calculation with the landing site (pp. 900-904, bank 32);
 *   part 3, prec100.c: the state at ignition and the precision
 *     trajectory (pp. 905-913, banks 32 and 16);
 *   part 4, v2t100.c: the integration calls and the velocity after the
 *     burn (pp. 914-923, banks 31 and 32).
 * The binding (src/binding/cm_p37_p70_routines.c) runs every key of the
 * faithful port's: a key a part has written on these routines, any other
 * on the faithful port's.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the faithful port's keys, by the listing's labels. Each call out of the
 * file (PHASCHNG, BANKCALL, POLY, BLANKET) leaves Z at the routine and Q
 * at the word after the TC, where the routine comes back to; ENDOFJOB and
 * GOTOPOOH don't come back. A display through P370GOF (GOFLASH) comes
 * back to the word after its TCR on recycle and to the next on proceed;
 * one through P370GOFR (GOFLASHR) to the word after its TCR on its
 * immediate return, three words on from it on recycle and four on
 * proceed. */
#ifndef READABLE_P37_P70_H
#define READABLE_P37_P70_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file (or to a keyed label): Q gets the
 * return, and the routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* Part 1, P37's main line -- p37.c */
void p37(void);                /* P37 */
void p37_after_phaschng(void); /* P37 +2 */
void p37_tig(void);            /* P37 +013 (after EXIT) */
void p37_gamma(void);          /* P37 +016 (P370GOF's proceed) */
void rte299(void);             /* RTE299 (P37GFRB1's proceed) */
void rte317_after_exit(void);  /* RTE317 +2 */
void rte317_after_poly(void);  /* RTE317 +014 */
void rte369_after_exit(void);  /* RTE369 +3 */
void rte369_after_poly(void);  /* RTE369 +015 */
extern const struct interpretive_code P37_CODE, RTE299_CODE, RTE317_POLY_CODE, RTE369_POLY_CODE;

/* Part 1, the displays -- p37_displays.c */
void rtevn_after_exit(void);      /* RTEVN +4 */
void rtevn_blank_r3(void);        /* RTEVN +6 (GOFLASHR's immediate return) */
void rtevn_v6n60(void);           /* RTEVN +015 (P370GOF's proceed) */
void rtevn_v6n39(void);           /* RTEVN +012 (GOFLASHR's proceed) */
void rtevn_v6n81(void);           /* RTEVN +020 (P37GFRB1's proceed) */
void rtevn_return(void);          /* RTEVN +023 (P370GOF's proceed) */
void p37n_after_exit(void);       /* P37N */
void p37n_option(void);           /* P37N +4 (P370GOF's recycle) */
void p37n_proceed(void);          /* P37N +7 */
void p37t_after_exit(void);       /* P37T +3 */
void p37t_after_poly(void);       /* P37T +013 */
void p37t_tig(void);              /* P37T +026 (after EXIT) */
void p37t_proceed(void);          /* P37T +031 */
void blank37(void);               /* 37BLANK (GOFLASHR's immediate return) */
void blank37_after_blanket(void); /* 37BLANK +2 */
void blank37_recycle(void);       /* 37BLANK +3 */
void blank37_proceed(void);       /* 37BLANK +4 */
void p370gofr_terminate(void);    /* P370GOFR +4 (GOFLASHR's returns) */
void p370gofr_proceed(void);      /* P370GOFR +5 */
void p370gofr_recycle(void);      /* P370GOFR +6 */
void p370gofr_immediate(void);    /* P370GOFR +7 */
void p370gof_terminate(void);     /* P370GOF +4 (GOFLASH's returns) */
void p370gof_proceed(void);       /* P370GOF +5 */
void p370gof_recycle(void);       /* P370GOF +6 */
extern const struct interpretive_code RTEVN_CODE, RTEVN_RETURN_CODE, P37N_CODE, P37T_POLY_CODE,
    P37T_TIG_CODE;

/* The display subroutines, as the file calls them (TCR from at). */
void p37gfrb1(word at); /* P37GFRB1 */
void p370gofr(word at); /* P370GOFR */
void p370gof(word at);  /* P370GOF */

/* Part 2, P370ALRM, TMRAD100 and RTEDISP -- rtedisp.c */
void p370alrm_after_exit(void);     /* P370ALRM +2 */
void p370alrm_after_varalarm(void); /* P370ALRM +4 (and proceed, +010) */
void p370alrm_terminate(void);      /* P370ALRM +7 (GOFLASH's returns) */
void p370alrm_recycle(void);        /* P370ALRM +011 */
extern const struct interpretive_code P370ALRM_CODE, P370ALRM_GOTO_CODE, TMRAD100_CODE, RTD18_CODE;

/* Part 3, INVC100 and PREC100 -- prec100.c (order pairs only) */
extern const struct interpretive_code INVC100_CODE, PREC100_CODE, PREC130_CODE, PREC175_CODE,
    PREC220_CODE;

/* Part 4, RTENCK1 to RTENCK3 and V2T100 -- v2t100.c */
void v2terror_after_exit(void); /* V2TERROR +1 */
extern const struct interpretive_code RTENCK_CODE, V2T100_CODE, V2T101_CODE, V2T102_CODE;

#endif

#endif
