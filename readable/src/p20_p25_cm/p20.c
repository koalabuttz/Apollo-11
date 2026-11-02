/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 1 (Comanche055 pp. 562-564): P20, the CSM's
 * "RENDEZVOUS NAVIGATION PROGRAM 20" (bank 37).
 *
 * "TO CONTROL THE CSM ATTITUDE AND OPTICS TO ACQUIRE THE LEM IN THE S+T
 * FIELD AND TO POINT THE CSM TRANSPONDER AT THE LEM.  TO UPDATE EITHER THE
 * LEM OR CSM STATE VECTOR (AS SPECIFIED BY THE ASTRONAUT BY THE DSKY
 * ENTRY) ON THE BASIS OF OPTICAL TRACKING DATA". The crew starts it by
 * "V37E20E", and ends it by selecting P00 or by "KEYING IN V56E".
 *
 * PROG20 checks the IMU (R02BOTH), zeroes the mark counters, sets the
 * flags (the LM is the vehicle to update) and brings both state vectors up
 * to the present time, the W matrix with the LM's when it is valid. Then
 * it starts R22, the tracking data job, and goes on at PIKUP20, where a
 * restart comes back to: while the track flag and REFSMMAT are good, the
 * preferred tracking attitude (R61CSM), then R52's automatic optics
 * positioning (P20R52JB), and the job ends holding the attitude.
 *
 * Each call out of the file (BANKCALL, 2PHSCHNG, PRIOCHNG, UPFLAG,
 * DOWNFLAG, FINDVAC) leaves Z at the routine and Q at the word after the
 * TC, where the routine comes back to; ENDOFJOB doesn't come back. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* PROG20 -- p. 562. "IMU STATUS CHECK". */
void prog20(void)
{
    tc_to(PROG20, BANKCALL); /* CADR R02BOTH */
}

/* PROG20 +2: "ZERO REND TRACKING MARK COUNTER" (TRKMKCNT) and "ZERO REND
 * VHF RNG MRK COUNTER" (VHFCNT); "SET PREF TRACK ATT FLAG". (CAF ZERO
 * leaves no overflow for either TS to skip on.) */
void prog20_after_r02both(void)
{
    set_A(ZERO);
    if (ts(ADDRESS_OF(TRKMKCNT))) not_modelled();
    if (ts(ADDRESS_OF(VHFCNT))) not_modelled();
    tc_to(PROG20 + 5u, UPFLAG); /* ADRES PRFTRKAT */
}

/* PROG20 +7: "LEM TO BE UPDATED.  VEHUPFLG RESET." */
void prog20_after_prftrkat(void)
{
    tc_to(PROG20 + 7u, DOWNFLAG); /* ADRES VEHUPFLG */
}

/* PROG20 +011 -- p. 563. "SET TRACKFLAG". */
void prog20_after_vehupflg(void)
{
    tc_to(PROG20 + 011u, UPFLAG); /* ADRES TRACKFLG */
}

/* PROG20 +013: "SET UPDATFLG". */
void prog20_after_trackflg(void)
{
    tc_to(PROG20 + 013u, UPFLAG); /* ADRES UPDATFLG */
}

/* PROG20 +015: "SET RNDVZFLG". */
void prog20_after_updatflg(void)
{
    tc_to(PROG20 + 015u, UPFLAG); /* ADRES RNDVZFLG */
}

/* PROG20 +017: 2PHSCHNG. OCT 4 makes restart group 4 inactive, and OCT
 * 05022 restarts group 2 at PROG20 +023, the word after the phase words,
 * as a job of priority 26 (OCT 26000). */
void prog20_after_rndvzflg(void)
{
    tc_to(PROG20 + 017u, TWO_PHSCHNG);
}

/* PROG20 +023: the state vectors, in interpretive code. */
void prog20_after_2phschng(void)
{
    tc_at(PROG20 + 023u);
    intpret();
}

/* PROG20 +024: the time now (LOADTIME) is MARKTIME, the time both state
 * vectors are brought to. SETINTG sets the integration flags, and INTEGRV
 * integrates a vehicle's state vector: first the LM's ("SET FOR LM
 * INTEGRATION", VINTFLAG clear), with the W matrix if it is valid
 * (RENDWFLG: "SET TO INTEGRATE THE W MATRIX", DIM0FLAG) and the LM is the
 * vehicle to update (VEHUPFLG clear, as PROG20 left it); then GRP2PC's
 * "GROUP 2 PHASE CHANGE" and the CSM's (VINTFLAG set), without it. P20.1
 * clears VINTFLAG when VEHUPFLG is set, as does the BOFF at P20.2 +6 when
 * it is clear: the vehicle each INTEGRV integrates is the one VINTFLAG
 * names. */
static const word prog20_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STCALL(MARKTIME),
    ADDRESS(SETINTG), /* "SET INTEGRATION FLAGS" */
    ORDERS(BOFF, SET),
    SWITCH(BOF, RENDWFLG),
    ADDRESS(P20_1),
    SWITCH(SET, DIM0FLAG), /* "SET TO INTEGRATE THE W MATRIX" */
    /* P20.1 */ ORDERS(BON, CLEAR),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(P20_2),
    SWITCH(CLEAR, VINTFLAG), /* "SET FOR LM INTEGRATION" */
    /* P20.2 */ ORDER(CALL),
    ADDRESS(INTEGRV),
    ORDER(CALL),
    ADDRESS(GRP2PC), /* "GROUP 2 PHASE CHANGE" */
    ORDER(CALL),
    ADDRESS(SETINTG), /* "SET INTEGRATION FLAGS" */
    ORDERS(BOFF, CLEAR),
    SWITCH(BOF, VEHUPFLG),
    ADDRESS(P20_3),
    SWITCH(CLEAR, VINTFLAG), /* "SET FOR LM INTEGRATION" */
    /* P20.3 */ ORDER(CALL),
    ADDRESS(INTEGRV),
    ORDER(EXIT),
};

/* PROG20 +055 -- after the EXIT: R22, the "REND TRACK DATA PROC ROUT", as
 * a job of priority 26 (FINDVAC: with a work area). */
void prog20_after_exit(void)
{
    set_A(PRIO26);
    tc_to(PROG20 + 056u, FINDVAC); /* 2CADR R22 */
}

/* PROG20 +061: 2PHSCHNG. OCT 00072 is group 2's phase 7, which restarts
 * the R22 job (RESTART_TABLES.agc's 2.7SPOT), and OCT 00111 group 1's
 * phase 11, which restarts this job at PIKUP20 (1.11SPOT). */
void prog20_after_findvac(void)
{
    tc_to(PROG20 + 061u, TWO_PHSCHNG);
}

/* PIKUP20 -- p. 563. Priority 14: "ALLOW HIGHER PRIO THAN LAMBERT". */
void pikup20(void)
{
    set_A(PRIO14);
    tc_to(PIKUP20 + 1u, PRIOCHNG);
}

/* PIKUP20 +2: "IS TRACK FLAG SET" (STATE +1 bit 5, TRACKFLG) and, p. 564,
 * "IS REFSMFLG SET" (STATE +3 bit 13): if either is clear, the job ends.
 * Else "INITIALIZE R61 COUNTER" and R61CSM, the preferred tracking
 * attitude. (CAF ZERO leaves no overflow for the TS to skip on.) */
void pikup20_after_priochng(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB: "NO" */
        continue_at(ENDOFJOB);
        return;
    }
    set_A(BIT13);
    mask_at(ADDRESS_OF(STATE) + 3u);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    set_A(ZERO);
    if (ts(ADDRESS_OF(R61CNTR))) not_modelled();
    tc_to(PIKUP20 + 014u, BANKCALL); /* CADR R61CSM */
}

/* PIKUP20 +016: E bank 5 ("EBANK= QMIN"), and "SET TARGET FLAG TO LEM".
 * (CAF EBANK5 leaves no overflow for the TS to skip on.) */
void pikup20_after_r61csm(void)
{
    set_A(EBANK5);
    if (ts(EBANK_ADDRESS)) not_modelled();
    tc_to(PIKUP20 + 020u, UPFLAG); /* ADRES TARG1FLG */
}

/* P20R52JB -- R52 in interpretive code. */
void p20r52jb(void)
{
    tc_at(P20R52JB);
    intpret();
}

/* P20R52JB +1: "SET UP AUTO OPTICS JOB". */
static const word p20r52jb_words[] = {
    ORDER(CALL),
    ADDRESS(R52), /* "SET UP AUTO OPTICS JOB" */
    ORDER(EXIT),
};

/* P20R52JB +4 -- after the EXIT: MKRLEES. */
void p20r52jb_after_exit(void)
{
    tc_to(P20R52JB + 4u, BANKCALL); /* CADR MKRLEES */
}

/* P20R52JB +6: "HOLD PRESENT ATTITUDE" (HOLDFLAG 1), and the job ends.
 * (CAF ONE leaves no overflow for the TS to skip on.) */
void p20r52jb_after_mkrlees(void)
{
    set_A(ONE);
    if (ts(ADDRESS_OF(HOLDFLAG))) not_modelled();
    tc_to(P20R52JB + 010u, ENDOFJOB);
}

/* ---- The interpretive code ---- */

const struct interpretive_code PROG20_CODE = {037u, PROG20 + 024u, COUNT(prog20_words),
                                              prog20_words};
const struct interpretive_code P20R52JB_CODE = {037u, P20R52JB + 1u, COUNT(p20r52jb_words),
                                                p20r52jb_words};

#endif
