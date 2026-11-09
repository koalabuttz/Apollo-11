/* SPDX-License-Identifier: MIT */
/* P11.agc, the CM's (Comanche055 pp. 533-550): the "EARTH ORBIT
 * INSERTION MONITOR PROGRAM" and the "SATURN TAKEOVER FUNCTION". The
 * faithful C is c64-port/src/cm_p11.c.
 *
 * P11 starts at liftoff (from the gyrocompassing, P02, "WHEN LIFTOFF
 * DISCRETE IS RECEIVED", or "BACKUP THRU VERB 75 ENTER"): it zeroes the
 * clock, moves TEPHEM on to the moment it was zeroed, starts SERVICER at
 * PREREAD1, computes the launch REFSMMAT, and sets SERVICER's exit to
 * VHHDOT, which shows V1, HDOT and H (noun 62) every 2 seconds. ATERTASK
 * starts ATERJOB, which drives the FDAI's attitude error needles through
 * the boost: from the attitude at liftoff, then from the pitch and roll
 * polynomials. With the S/C control of Saturn switch on, the errors steer
 * the Saturn instead. SATSTKON (verb 46) hands the Saturn to the rotation
 * hand controller: SATSTICK, every 100 ms, sends its rate commands.
 *
 * Written in three parts (README.md):
 *   part 1: all the basic code; p11.c: P11, REP11, VHHDOT and ATERTASK
 *     (pp. 535-540, bank 34); aterjob.c: ATERJOB to TAKEON, and DELSTOR,
 *     RLTST, NOPOLY and RESCALES (pp. 542-547, bank 34); satstick.c:
 *     SATSTKON (bank 43), REDOSAT and SATSTICK (bank 23) (pp. 548-550).
 *   part 2: the order pairs of bank 34 (in p11.c and aterjob.c).
 *   part 3: the order pairs of bank 37 (ATTDISP1 to NOPOLYM).
 * The binding (src/binding/cm_p11_routines.c) runs every key of the
 * faithful port's on these routines.
 *
 * Each function runs from its word until control leaves the file
 * (PHASCHNG, IBNKCALL, WAITLIST, BANKCALL, FINDVAC, DANZIG, RESUME, ...)
 * or reaches NEWOPS through a TC INTPRET. A call out of the file leaves Z
 * at the routine called and Q at the word after the TC, where it comes
 * back; the faithful port keys a routine at each such return. A TC or TCF
 * to a word of this file is a call of its function. */
#ifndef READABLE_P11_H
#define READABLE_P11_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file: Q gets the return, and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* EXTEND; DCA of a two-word constant: A gets its first word, L its
 * second. */
static inline void dca_constant(const word pair[2])
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* ---- p11.c: P11, REP11, VHHDOT, ATERTASK, GETDOWN -- pp. 535-541 ---- */
void p11(void);                     /* P11 */
void rep11a(void);                  /* REP11A */
void rep11a_after_phaschng(void);   /* REP11A +015 */
void rep11a_after_preread1(void);   /* REP11A +027 */
void rep11a_start_display(void);    /* REP11A +032 (PHASCHNG's return) */
void rep11a_after_waitlist(void);   /* REP11A +036 */
void rep11a_after_newmodex(void);   /* REP11A +040 */
void rep11a_after_upflag(void);     /* REP11A +042 */
void rep11a_after_cleandsp(void);   /* REP11A +046 */
void rep11a_after_2phschng(void);   /* REP11A +051 */
void matrxjob(void);                /* MATRXJOB */
void matrxjob_after_exit(void);     /* MATRXJOB +0101 */
void matrxjob_after_phaschng(void); /* MATRXJOB +0103 */
void matrxjob_after_waitlist(void); /* MATRXJOB +0127 */
void matrxjob_after_2phschng(void); /* MATRXJOB +0132 */
void rep11(void);                   /* REP11 */
void vhhdot(void);                  /* VHHDOT */
void vhhdot_after_exit(void);       /* VHHDOT +4 */
void vhhdot_after_phaschng(void);   /* VHHDOT +6 */
void atertask(void);                /* ATERTASK */
void atertask_after_findvac(void);  /* ATERTASK +4 */
void atertask_after_needler(void);  /* ATERTASK +011 */
/* The interpretive code (part 2): at REP11A +056, MATRXJOB +012, VHHDOT
 * +1 and GETDOWN. */
extern const struct interpretive_code P11_CODE, MATRXJOB_CODE, VHHDOT_CODE, GETDOWN_CODE;

/* ---- aterjob.c: ATERJOB to TAKEON, S11.1, DELSTOR to RESCALES -- pp. 542-547 ---- */
void aterjob(void);                /* ATERJOB */
void attdisp_after_powrsers(void); /* ATTDISP +022 */
void attdisp2_after_exit(void);    /* ATTDISP2 +030 */
void steersat_after_exit(void);    /* STEERSAT +6 */
void aterset(void);                /* ATERSET */
void aterset_after_delayjob(void); /* ATERSET +3 */
void atreset_after_needler(void);  /* ATRESET +5 */
void atreset_after_delayjob(void); /* ATRESET +010 */
void delstor(void);                /* DELSTOR */
void rltst(void);                  /* RLTST */
void nopoly(void);                 /* NOPOLY */
/* The interpretive code (part 2): at ATTDISP +025, ATTDISP2, STEERSAT +1,
 * S11.1 and NOPOLY +1. */
extern const struct interpretive_code ATTDISP_CODE, ATTDISP2_CODE, STEERSAT_CODE, S11_1_CODE,
    NOPOLY_CODE;
/* And bank 37's (part 3): ATTDISP1 to NOPOLYM. */
extern const struct interpretive_code ATTDISP1_CODE;

/* ---- satstick.c: SATSTKON, REDOSAT, SATSTICK -- pp. 548-550 ---- */
void satstkon(void);                /* SATSTKON */
void satstkon_after_zerojet(void);  /* SATSTKON +013 */
void redosat(void);                 /* REDOSAT */
void redosat_after_needler(void);   /* REDOSAT +010 */
void satstick(void);                /* SATSTICK */
void satstick_after_stickchk(void); /* SATSTICK +015 */
void satstick_after_needler(void);  /* SATSTICK +030 */

#endif
#endif
