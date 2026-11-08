/* SPDX-License-Identifier: MIT */
/* SERVICER207.agc (Comanche055 pp. 819-836, bank 37): the CM's average-G
 * loop. "THE ROUTINES DESCRIBED BELOW ARE USED TO CALCULATE VALUES OF RN,
 * VN, AND GDT/2 DURING ACCELERATED FLIGHT." Every two seconds READACCS, a
 * task, reads the accelerometers (the PIPAs) and sets up the SERVICER job,
 * which compensates the counts, adds them up (DVTOTAL), moves the state
 * vector on by them and by the computed gravity (AVERAGEG) and hands over
 * through AVGEXIT. In the order of the listing:
 *
 *   PREREAD, READACCS, CHEKAVEG, AVEGOUT, QUIKREAD (preread.c, pp.
 *     822-827): the start, and the two-second task. During entry it also
 *     saves the PIPA counts for the downlink every half second (QUIKREAD)
 *     and starts the entry DAP's task (SETJTAG).
 *   SERVICER, AVERAGEG, AVGEND, SERVEXIT, NORMLIZE (servicer.c, pp.
 *     828-831): the job, the last pass and the first.
 *   PIPASR, REREADAC (pipasr.c, pp. 832-834): the PIPA reader and its
 *     restart.
 *   CALCGRAV, CALCRVG (calcgrav.c, pp. 835-836): the average G
 *     integration, all interpretive.
 *
 * The faithful C is c64-port/src/cm_servicer.c.
 *
 * Each function runs from a word the program can resume at (an entry, a
 * return from a routine out of the file, the word after an EXIT, or one
 * a TC in the file returns to) until its code leaves it: at a TC or TCF
 * out of the file (Z the routine, Q the return), at a TC INTPRET (run on
 * to NEWOPS), or at a transfer to an address the code computes (TC Q, TC
 * RUPTREG1, AVERAGEG's DXCH Z). The binding,
 * src/binding/cm_servicer_routines.c, runs on from a computed address
 * while it is one of those words. A TC to one of the file's own
 * routines calls its function.
 *
 * Words of the listing no instruction names, so no C name either: the
 * constants 2.5SEC (DEC 250) and MDOTFAIL (DEC 144.0 B-16), after
 * JTAGTIME (p. 830). */
#ifndef READABLE_SERVICER_CM_H
#define READABLE_SERVICER_CM_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* EXTEND; DCA of a 2CADR constant: A gets its first word, L its second. */
static inline void dca_2cadr(const word pair[2])
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* TC to a return the code keeps in an erasable word (TC Q, TC RUPTREG1),
 * the address to in it: Q gets the word after, and to runs next. Not
 * modelled: an address outside 010-07777, where the faithful port stops. */
static inline void tc_to_return(unsigned address, word to)
{
    if (to < 010u || to > 07777u) not_modelled();
    tc_at((word)address);
    continue_at(to);
}

/* PREREAD and READACCS -- preread.c */
void preread(void);                 /* PREREAD */
void redo5_31(void);                /* REDO5.31 (NOVAC's return) */
void redo5_31_after_preread1(void); /* REDO5.31 +1 */
void redo5_31_after_findvac(void);  /* REDO5.31 +5 */
void redo5_31_after_waitlist(void); /* REDO5.31 +011 */
void redo5_31_after_newphase(void); /* REDO5.31 +014 */
void preread1(void);                /* PREREAD1 */
void preread1_after_pipasr(void);   /* PREREAD1 +3 */
void readaccs(void);                /* READACCS */
void pipsdone(void);                /* PIPSDONE (PIPASR's return) */
void redo5_5(void);                 /* REDO5.5 */
void redo5_5_entry_dap(void);       /* REDO5.5 +010 (CHEKAVEG's return) */
void redo5_5_after_quikread(void);  /* REDO5.5 +034 (WAITLIST's return) */
void nosavpip_after_waitlist(void); /* NOSAVPIP +6 */
void nosavpip_after_newphase(void); /* NOSAVPIP +011 */
void chekaveg(void);                /* CHEKAVEG */
void recall_readaccs(void);         /* CHEKAVEG +4 (AVEGOUT's TC's return) */
void makeserv(void);                /* MAKESERV (WAITLIST's return) */
void makeserv_after_findvac(void);  /* MAKESERV +4 */
void makeserv_after_newphase(void); /* MAKESERV +7 */
void quikread(void);                /* QUIKREAD (FIXDELAY's return) */

/* SERVICER to NORMLIZE -- servicer.c */
void servicer(void);                /* SERVICER */
void servicer_inhint(void);         /* SERVICER +1 */
void pipcheck_no_count(void);       /* PIPCHECK +5 (TC +2's return) */
void saturation_test(void);         /* PIPCHECK +6 (TC PIPLOOP's return) */
void pipcheck_after_alarm(void);    /* PIPCHECK +013 */
void piploop(void);                 /* PIPLOOP (TC PIPLOOP's return) */
void piploop_after_phaschng(void);  /* PIPLOOP +7 */
void dvtotup(void);                 /* DVTOTUP (1/PIPA's return) */
void dvtotup_after_exit(void);      /* DVTOTUP +5 */
void averageg_after_phaschng(void); /* AVERAGEG +2 */
void averageg_after_exit(void);     /* AVERAGEG +6 */
void averageg_copy(void);           /* AVERAGEG +010 */
void averageg_after_gentran(void);  /* AVERAGEG +014 */
void averageg_exit(void);           /* AVERAGEG +017 */
void avgend(void);                  /* AVGEND */
void avgend_after_upflag(void);     /* AVGEND +4 */
void avgend_after_2phschng(void);   /* AVGEND +010 */
void avgend_after_exit(void);       /* AVGEND +014 */
void avgend_after_pipfree(void);    /* AVGEND +021 */
void avgend_after_downflag(void);   /* AVGEND +027 */
void avgend_after_downflag_2(void); /* AVGEND +031 */
void avgend_after_2phschng_2(void); /* AVGEND +040 */
void servexit(void);                /* SERVEXIT */
void servexit_after_phaschng(void); /* SERVEXIT +2 */
void normlize(void);                /* NORMLIZE */
void normlize_after_gentran(void);  /* NORMLIZE +4 */
void normlize_after_exit(void);     /* NORMLIZE +015 */
extern const struct interpretive_code DVTOTUP_CODE, AVERAGEG_CODE, AVGEND_CODE, NORMLIZE_CODE,
    CALCGRAV_CODE, CALCRVG_CODE;

/* PIPASR and REREADAC -- pipasr.c */
void pipasr(void);   /* PIPASR (a subroutine: back by TC Q) */
void rereadac(void); /* REREADAC */

#endif

#endif
