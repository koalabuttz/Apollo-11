/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc (Luminary099 pp. 843-856): the LM's ascent guidance,
 * for P12 and the aborts P70 and P71, in the order of the listing:
 *
 *   ATMAG (atmag.c, bank 34): each guidance cycle's start. The measured
 *     velocity gain gives the thrust acceleration AT and the burn-up time
 *     TBUP, then ASCENT.
 *   ASCENT (guidance.c, bank 30): the guidance equations. The velocity to
 *     be gained, the time to go TGO, the pitch and yaw rates and their
 *     constants, the thrust direction UNFC/2 and the attitude to hold
 *     (UNWC/2). Then ASCTERM: noun 63 on the display, and the job ends.
 *   ANG1CHEK, CHECKYAW, KEEPVR (vertical_rise.c, banks 27, 7 and 14): the
 *     vertical rise, and the yaw check below 25,000 feet.
 *   ENGOFF, CUTOFF, TERMASC (engoff.c, banks 14 and 30): the engine shut
 *     off at the time to go, the cutoff's displays (nouns 63 and 85), and
 *     back to P00.
 *   YCOMP and LOGSUB (subroutines.c, banks 27 and 30): the out-of-plane
 *     distance Y, and -LOG(X).
 *
 * The faithful C is c64-port/src/lm_ascent.c.
 *
 * Each function runs from a word the program can resume at (an entry, a
 * subroutine's return, the word after an EXIT) until its code leaves the
 * file: at a TC or TCF out of it (Z the routine, Q the return), or at a TC
 * INTPRET, run on to NEWOPS (intpret()). The faithful port stops there
 * too: the binding, src/binding/ascent_routines.c, calls each from its
 * key. */
#ifndef READABLE_ASCENT_H
#define READABLE_ASCENT_H

#include "../agc/agc.h"

#if LUMINARY099
#include "../interpreter/notation.h"

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ATMAG -- atmag.c */
void atmag(void);                /* ATMAG */
void atmag_after_phaschng(void); /* PHASCHNG's return: TC INTPRET */
void atmag_after_exit(void);     /* the word after the EXIT */
extern const struct interpretive_code ATMAG_CODE, ATMAG_AFTER_EXIT_CODE;

/* ASCENT and ASCTERM -- guidance.c */
void ascterm_after_exit(void);      /* the word after ASCTERM's EXIT */
void ascterm1_after_exit(void);     /* the word after ASCTERM1's EXIT */
void ascterm1_after_godspr(void);   /* BANKCALL's return */
void ascterm3(void);                /* ASCTERM3 */
void ascterm4_after_exit(void);     /* the word after ASCTERM4's EXIT */
void ascterm4_after_zatteror(void); /* IBNKCALL's return */
extern const struct interpretive_code ASCENT_CODE, ASCTERM_CALL_CODE, ASCTERM2_CODE, ASCTERM4_CODE,
    CHECKALT_CODE;

/* ANG1CHEK, CHECKYAW, KEEPVR -- vertical_rise.c */
extern const struct interpretive_code ANG1CHEK_CODE, CHECKYAW_CODE, KEEPVR_CODE;

/* ENGOFF, CUTOFF, CUTOFF2, TERMASC -- engoff.c */
void engoff_after_exit(void);      /* the word after the EXIT: TC TPAGREE */
void engoff_after_tpagree(void);   /* TPAGREE's return */
void engoff_after_twiddle(void);   /* TWIDDLE's return: TC PHASCHNG */
void engoff_restart_words(void);   /* the restart words after OCT 47014 */
void engoff_intpret(void);         /* PHASCHNG's return: TC INTPRET */
void engoff1(void);                /* ENGOFF1 */
void engoff1_after_enginof2(void); /* IBNKCALL's return */
void engoff1_after_findvac(void);  /* FINDVAC's return: TC PHASCHNG */
void engoff1_after_phaschng(void); /* PHASCHNG's return */
void cutoff(void);                 /* CUTOFF */
void cutoff_after_upflag(void);    /* UPFLAG's return (and CUTOFF -5) */
void cutoff_terminate(void);       /* GOFLASH's first return */
void cutoff_proceed(void);         /* GOFLASH's second */
void cutoff_enter(void);           /* GOFLASH's third */
void cutoff1_after_zatteror(void); /* IBNKCALL's return */
void cutoff1_after_setmindb(void); /* IBNKCALL's return */
void cutoff2(void);                /* CUTOFF2 */
void cutoff2_after_phaschng(void); /* PHASCHNG's return */
void cutoff2_terminate(void);      /* GOFLASH's first return */
void cutoff2_proceed(void);        /* GOFLASH's second */
void cutoff2_enter(void);          /* GOFLASH's third */
void termasc(void);                /* TERMASC */
void termasc_after_phaschng(void); /* PHASCHNG's return */
void termasc_after_restordb(void); /* IBNKCALL's return */
void termasc_after_downflag(void); /* DOWNFLAG's return */
extern const struct interpretive_code ENGOFF_CODE, ENGOFF_SET_CODE;

/* YCOMP and LOGSUB -- subroutines.c */
void logsub_after_exit(void);    /* the word after the EXIT: TC POLY */
void logsub_after_poly(void);    /* POLY's return */
void logsub_after_shortmp(void); /* SHORTMP's return */
extern const struct interpretive_code YCOMP_CODE, LOGSUB_CODE, LOGSUB_RVQ_CODE;
#endif

#endif
