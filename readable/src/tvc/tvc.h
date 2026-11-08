/* SPDX-License-Identifier: MIT */
/* The CM's thrust vector control (TVC), the autopilot of an SPS burn.
 *
 * TVCINITIALIZE.agc (Comanche055 pp. 937-944, bank 17), "TVC DAP
 * INITIALIZATION AND STARTUP CALL", c64-port/src/cm_tvc_init.c:
 *
 *   TVCDAPON, MRCLEAN, TVCINIT1 to TVCINIT4 (tvcinitialize.c): the start
 *     of the TVC DAP, in T5 interrupts ("T5LOC=2CADR(TVCDAPON,EBANK=BZERO),
 *     T5=.6SECT5", "CALLED BY "DOTVCON" IN P40"). It zeroes the DAP's
 *     erasables, then "PERFORMS TVCDAP INITIALIZATION (GAINS, TIMING
 *     PARAMETERS, FILTER VARIABLES, ETC.)", starts the needles (NEEDLER),
 *     calls TVCEXEC by WAITLIST and the DAP itself (DAPINIT) by T5.
 *   PRESWTCH, SWICHOVR (tvcinitialize.c): the switch to the low bandwidth
 *     gains and filter of the docked CSM/LM, from verb 46 or a restart.
 *   LOADCOEF, S40.15 (tvcinitialize.c): the filter coefficients loaded
 *     into N10 on, and the gains 1/CONACC and VARK.
 *
 * TVCEXECUTIVE.agc (pp. 945-950, bank 16), c64-port/src/cm_tvc_exec.c:
 *
 *   TVCEXEC to TVCEXFIN (tvcexecutive.c): the half-second task TVCINIT4
 *     starts, "CALL TVCEXECUTIVE (ROLLDAP CALL, ETC)": the roll DAP's
 *     errors and task (ROLLPREP), the needles (NEEDLEUP), the variable
 *     gains and mass (VARGAINS, GAINCHNG, EXECCOPY), and the one-shot and
 *     repetitive trim corrections (1SHOTCHK to CNTRCOPY).
 *
 * TVCMASSPROP.agc (pp. 951-955, bank 5), c64-port/src/cm_tvc_massprop.c:
 *
 *   MASSPROP, FIXCW (tvcmassprop.c): the CSM's (or the docked CSM/LM's)
 *     moments of inertia IXX, IAVG and IAVG/TLX as straight lines in the
 *     CSM's mass, and WEIGHT/G.
 *
 * TVCRESTARTS.agc (pp. 956-960, bank 16), c64-port/src/cm_tvc_restarts.c:
 *
 *   REDOTVC to EXRSTRT (tvcrestarts.c): the "TVC RESTART PACKAGE", which
 *     restarts TVCEXECUTIVE at its phase (EXRSTRT), finishes a DAP
 *     copycycle or repeats the initialization or the switch-over, and
 *     brings up the outcounters again (ENABL1, ENABL2, CMDSOUT).
 *
 * All basic code. Each function runs from a word the program can come
 * back to (an entry, the word after a call out of the file) until control
 * leaves the file's code (RESUME, NOQRSM, IBNKCALL, ISWCALL, WAITLIST,
 * POSTJUMP, TASKOVER, TC Q, TC RTRNLOC, EXRSTRT's TCF), or reaches a word
 * where the faithful port's run stops: a key of its own, run by the
 * function named for it. */
#ifndef READABLE_TVC_H
#define READABLE_TVC_H

#include "../agc/agc.h"

#if COMANCHE055

/* ---- Shared by the files' code ---- */

/* A word's value as a signed number (-0 is 0). */
static inline int value_of(word value)
{
    return (value & SIGN_BIT) ? -(int)negate(value) : (int)value;
}

/* TC Q, the end of LOADCOEF, S40.15 and MASSPROP: the word in Q runs as
 * the instruction TC <Q>, so Z gets the address and Q becomes 3. Q 3
 * itself (Q after a TC Q) names the word at 3, EBANK: the word in Q, 3,
 * runs as RELINT, and then EBANK's word, an address in fixed memory, as
 * TC <EBANK>, which leaves Q 4. Not modelled (the faithful port refuses
 * them too): any other Q below 010 (taken overflow-corrected, as the AGC
 * reads a register's word), or above 07777. The bindings go on at a word
 * of the file that is a key, as the faithful port does. */
static inline void tc_q(void)
{
    word at = overflow_corrected(Q);

    if (at == Q_ADDRESS + 1u && EBANK >= 02000u) {
        relint();
        tc_at(Q_ADDRESS + 1u);
        Z = EBANK;
        return;
    }
    if (at < 010u || at > 07777u) not_modelled();
    tc_at(Q_ADDRESS);
    Z = at;
}

/* TVCINITIALIZE.agc -- tvcinitialize.c */
void tvcdapon(void);                /* TVCDAPON */
void mrclean(void);                 /* MRCLEAN */
void tvcinit1(void);                /* TVCINIT1 */
void tvcinit1_after_massprop(void); /* TVCINIT1 +5 */
void tvcinit1_after_loadcoef(void); /* TVCINIT1 +025 */
void tvcinit2(void);                /* TVCINIT2 */
void tvcinit2_repfrac(void);        /* TVCINIT2 +021 */
void tvcinit4(void);                /* TVCINIT4 */
void tvcinit4_after_waitlist(void); /* TVCINIT4 +017 */
void tvcinit4_t5(void);             /* TVCINIT4 +021 */
void preswtch(void);                /* PRESWTCH */
void preswtch_after_swichovr(void); /* PRESWTCH +1 */
void swichovr_restart(void);        /* SWICHOVR +5 */
void swichovr_after_s40_15(void);   /* SWICHOVR +026 */
void swichovr_after_loadcoef(void); /* SWICHOVR +043 */
void rtrnloc_return(void);          /* TC RTRNLOC, run as TC <return> */
void loadcoef(void);                /* LOADCOEF */
void loadcoef_dca6(void);           /* LOADCOEF +014 */
void loadcoef_n10_6(void);          /* LOADCOEF +017 */
void loadcoef_n10_8(void);          /* LOADCOEF +023 */
void s40_15(void);                  /* S40.15 */
void s40_15_switchover(void);       /* S40.15 +7 */

/* TVCEXECUTIVE.agc -- tvcexecutive.c */
void tvcexec(void);               /* TVCEXEC */
void rollprep(void);              /* ROLLPREP */
void needleup(void);              /* NEEDLEUP */
void vargains(void);              /* VARGAINS */
void gainchng(void);              /* GAINCHNG */
void gainchng_after_fixcw(void);  /* GAINCHNG +2 */
void gainchng_after_s40_15(void); /* GAINCHNG +4 */
void execcopy_copycycle(void);    /* EXECCOPY +1 */
void oneshotchk(void);            /* 1SHOTCHK */
void tempset(void);               /* TEMPSET */
void corsetup(void);              /* CORSETUP */
void corcopy_copycycle(void);     /* CORCOPY +1 */
void cntrcopy(void);              /* CNTRCOPY */

/* TVCMASSPROP.agc -- tvcmassprop.c */
void massprop(void); /* MASSPROP */
void fixcw(void);    /* FIXCW */

/* TVCRESTARTS.agc -- tvcrestarts.c */
void redotvc(void);                 /* REDOTVC */
void tvcdaphs(void);                /* TVCDAPHS */
void enabl2(void);                  /* ENABL2 */
void cmdsout(void);                 /* CMDSOUT */
void phschk2_mrclean(void);         /* PHSCHK2 +5, where EXRSTRT can go on */
void chkstrk(void);                 /* CHKSTRK */
void fincopy_after_copycycle(void); /* FINCOPY +3 */
void exrstrt(void);                 /* EXRSTRT */

#endif

#endif
