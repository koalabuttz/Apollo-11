/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 16 (Luminary099 pp. 593-598): GOTOV56, and R29's start
 * (R29, R29REM0J, PREPOS29, R29.LOS and SETUP29D).
 *
 * GOTOV56: "P20 TERMINATES BY GOTOV56 INSTEAD OF GOTOPOOH", TRMTRACK
 * through SUPDXCHZ.
 *
 * R29, the "RENDEZVOUS RADAR DESIGNATE DURING POWERED FLIGHT", "DESIGNATES
 * THE RENDEZVOUS RADAR TOWARD THE COMPUTES LOS TO THE CSM, WITH THE CHIEF
 * OBJECTIVE OF OBTAINING RANGE AND RANGE RATE DATA AT 2-SECOND INTERVALS
 * FOR TRANSMISSION TO THE GROUND". SERVICER runs it every 2 seconds, and
 * it goes back to SERVICER at NOR29NOW.
 *
 * The first time (RADMODES bit 10 clear), R29 turns the designate on and
 * gets the radar ready: in mode 1 a job (R29REM0J) remodes it; in mode 2 a
 * task (PREPOS29) puts the trunnion to -180 degrees. After that, R29.LOS
 * brings the line of sight and its velocity up to the present time, and
 * when the designate loop is ready for them (LOSCMFLG clear) SETUP29D
 * gives them to it, starting the loop's task (BEGDES29) if it has not
 * started.
 *
 * The listing names R29's job R29REM0J (a zero); the assembled program
 * spells it R29REMOJ, and so does this code.
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* GOTOV56 -- p. 593: "P20 TERMINATES BY GOTOV56 INSTEAD OF GOTOPOOH":
 * TRMTRACK's 2CADR (VB56CADR) through SUPDXCHZ. */
void gotov56(void)
{
    set_L(VB56CADR[1]); /* EXTEND; DCA VB56CADR */
    set_A(VB56CADR[0]);
    continue_at(SUPDXCHZ); /* TCF SUPDXCHZ */
}

/* ---- R29 ---- */

/* R29.LOS -- p. 597: the time since the state vector's (T-PIPTIME) in MPAC,
 * "SCALED B-28"; "SET MODE TO DOUBLE PRECISION" (A, DAS's overflow, is 0:
 * the TS does not skip); "T-PIPTIME NOW SCALED B-17" (MP BIT12), and the
 * interpreter. */
static void r29_los(void)
{
    dcs(ADDRESS_OF(PIPTIME)); /* EXTEND; DCS PIPTIME */
    dxch(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    das(ADDRESS_OF(MPAC));  /* "(MPAC) = T-PIPTIME, SCALED B-28." */
    ts(ADDRESS_OF(MODE));   /* "SET MODE TO DOUBLE PRECISION." */
    ca(ADDRESS_OF(MPAC) + 1u);
    mp(BIT12); /* EXTEND; MP BIT12 */
    dxch(ADDRESS_OF(MPAC));
    tc_at(R29_LOS + 013u);
    intpret();
}

/* R29 -- p. 596: "BRANCH IF DESIGNATION IS ALREADY ON." (R29.LOS). Else
 * "SHOW THAT DESIGNATION IS NOW ON.", "REMOVE RR TRACK ENABLE DISCRETE.",
 * "CLEAR LOSCMFLG TO SHOW DES. LOOP IS OFF.", "SHOW THAT DES. LOOP IS NOT
 * REQUESTED." (STATE's OLDESBIT), and "ENABLE RR ERROR COUNTERS."
 * (SETRRECR). (A MASK's result: the TSs do not skip.) */
void r29(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(DESIGBIT);
    if (A_is_zero()) { /* EXTEND; BZF R29.LOS */
        r29_los();
        return;
    }
    inhint();
    ads(ADDRESS_OF(RADMODES)); /* "SHOW THAT DESIGNATION IS NOW ON." */
    set_A(negate(BIT14));
    wand_channel(CHAN12); /* "REMOVE RR TRACK ENABLE DISCRETE." */
    set_A(negate(LOSCMBIT));
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2)); /* "CLEAR LOSCMFLG TO SHOW DES. LOOP IS OFF." */
    set_A(negate(OLDESBIT));
    mask_at(ADDRESS_OF(STATE));
    ts(ADDRESS_OF(STATE));       /* "SHOW THAT DES. LOOP IS NOT REQUESTED." */
    tc_to(R29 + 017u, BANKCALL); /* CADR SETRRECR */
}

/* SETPRPOS: "TASK TO SET TRUNNION ANGLE TO 180 DEG." (PREPOS29). */
static void setprpos(void)
{
    set_A(ONE);
    tc_to(SETPRPOS + 1u, WAITLIST); /* 2CADR PREPOS29 */
}

/* R29 +021: "TEST RR MODE BIT.": "MODE 2." SETPRPOS; "MODE 1: MUST
 * REMODE.", R29REM0J, which "NEEDS OWN JOB TO RADSTALL IN.". */
void r29_after_setrrecr(void)
{
    set_A(ANTENBIT);
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) { /* CCS A */
        setprpos();       /* "MODE 2." */
        return;
    }
    set_A(PRIO21);            /* "MODE 1: MUST REMODE." */
    tc_to(R29 + 026u, NOVAC); /* 2CADR R29REM0J */
}

/* R29 +031: "CLEAR DESIGNATE FLAG IN RADMODES BEFORE CALLING REMODE" (the
 * TS does not skip), "SHOW THAT REMODING IS ON.", and "CONTINUE SERVICER
 * FUNCTIONS." */
void r29_after_novac(void)
{
    set_A(negate(DESIGBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    set_A(REMODBIT);
    ads(ADDRESS_OF(RADMODES)); /* "SHOW THAT REMODING IS ON." */
    continue_at(NOR29NOW);     /* TCF NOR29NOW */
}

/* SETPRPOS +4: "SHOW THAT REPOSITIONING IS ON.", and NOR29NOW. */
void setprpos_after_waitlist(void)
{
    set_A(REPOSBIT);
    ads(ADDRESS_OF(RADMODES));
    continue_at(NOR29NOW); /* TCF NOR29NOW */
}

/* R29REM0J -- p. 597, the job: "FORCE RENDEZVOUS RADAR INTO MODE 2.":
 * "REMODE MUST RUN AS A TASK.", and the job waits in RADSTALL "FOR END OF
 * REMODING"; it ends either way ("BAD EXIT CAN'T HAPPEN."). */
void r29remoj(void)
{
    set_A(ONE);
    tc_to(R29REMOJ + 1u, WAITLIST); /* 2CADR REMODE */
}

void r29remoj_after_waitlist(void)
{
    tc_to(R29REMOJ + 4u, BANKCALL); /* CADR RADSTALL */
}

void r29remoj_bad_exit(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB: "BAD EXIT CAN'T HAPPEN." */
}

void r29remoj_good_exit(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* PREPOS29 -- p. 597, the task: "PREPOSITION THE RR TRUNNION ANGLE TO -180
 * DEG." (NEGMAX) by RRTONLY, which returns once the trunnion is there. */
void prepos29(void)
{
    set_A(NEGMAX); /* "-180 DEG." */
    tc_at(PREPOS29 + 1u);
    rrtonly(); /* "DRIVE TRUNNION CDU." */
}

/* PREPOS29 +2: "SHOW THAT REPOSITIONING IS OFF." (the TS does not skip),
 * and the task ends. */
void prepos29_after_rrtonly(void)
{
    set_A(negate(REPOSBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    continue_at(TASKOVER); /* TCF TASKOVER */
}

/* R29.LOS +014 -- p. 598: "PUSH DOWN T-PIPTIME"; "LOSVEL = V(CSM) - V";
 * "SWAP LOSVEL FOR T-PIPTIME, MULTIPLY THEM AND ADD THE RESULT TO R(CSM) -
 * R TO GET AN UP-TO-DATE LOS VECTOR IN SM AXES." "IF DESIGNATE LOOP IS
 * OFF, CHANGE LOSCM- FLG TO ON AND GO TO SET UP NEW DATA." (SETUP29D);
 * EXIT, "IF DES. LOOP IS ON, LET IT USE OLD DATA." */
static const word r29_los_words[] = {
    ORDERS(PDVL, VSU),                             /* "PUSH DOWN T-PIPTIME" */
    ADDRESS_PLUS_ONE(V_CSM),  ADDRESS_PLUS_ONE(V), /* "LOSVEL = V(CSM) - V" */
    ORDERS(PDDL, VXSC),                            /* "SWAP LOSVEL FOR T-PIPTIME, MULTIPLY THEM" */
    ORDERS(VAD, VSU),         ADDRESS_PLUS_ONE(R_CSM),
    ADDRESS_PLUS_ONE(R),      ORDERS(BOFSET, EXIT), /* "(BOFSET DOES ITS THING INHINTED.)" */
    SWITCH(BOFSET, LOSCMFLG), ADDRESS(SETUP29D),
};

/* R29.LOS +026: "IF DES. LOOP IS ON, LET IT USE OLD DATA." */
void r29_los_after_exit(void)
{
    continue_at(NOR29NOW); /* TCF NOR29NOW */
}

/* SETUP29D -- p. 598: the "LINE-OF-SIGHT VECTOR, STABLE MEMBER AXES" to
 * LOSSM; "1/2 SECOND'S WORTH OF LOS VELOCITY." to LOSVDT/4. "LET R29DLOOP
 * USE NEW DATA." (LOSCMFLG clear); EXIT. */
static const word setup29d_words[] = {
    STOVL(LOSSM), /* "LINE-OF-SIGHT VECTOR, STABLE MEMBER AXES" */
    ADDRESS_PLUS_ONE_D(0),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(_5SECB17),
    STORE(LOSVDT_4), /* "1/2 SECOND'S WORTH OF LOS VELOCITY." */
    ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, LOSCMFLG), /* "LET R29DLOOP USE NEW DATA." */
};

/* SETUP29D +7: "BRANCH IF R29 DES. LOOP IS REQUESTED." (NOR29NOW).
 * "OTHERWISE REQUEST IT NOW.", and "SEE IF TASK SHOULD BE OFFSET ONE
 * SECOND." by PIPCTR: positive, "-96D +100D = 4." centiseconds; +0, "0
 * +100D = 100D."; negative, the CCS's |PIPCTR| - 1. "START BEGDES29 TASK
 * ASAP." Not modelled: PIPCTR -0, which runs the 2CADR as an instruction. */
void setup29d_after_exit(void)
{
    cs(ADDRESS_OF(STATE));
    mask(OLDESBIT);
    if (A_is_zero()) { /* EXTEND; BZF NOR29NOW */
        continue_at(NOR29NOW);
        return;
    }
    inhint();
    ads(ADDRESS_OF(STATE)); /* "OTHERWISE REQUEST IT NOW." */
    switch (ccs_at(ADDRESS_OF(PIPCTR))) {
    case POSITIVE:
        set_A(negate(SUPER110)); /* "-96D +100D = 4." */
        ad(ONE_SEC);
        break;
    case IS_PLUS_ZERO: ad(ONE_SEC); break; /* "0 +100D = 100D." */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    tc_to(SETUP29D + 020u, WAITLIST); /* 2CADR BEGDES29 */
}

/* SETUP29D +023: "RELINT AND CONTINUE SERVICER FUNCTIONS." */
void setup29d_after_waitlist(void)
{
    continue_at(NOR29NOW); /* TCF NOR29NOW */
}

/* ---- The interpretive code ---- */

const struct interpretive_code R29_LOS_CODE = {033u, R29_LOS + 014u, COUNT(r29_los_words),
                                               r29_los_words};
const struct interpretive_code SETUP29D_CODE = {033u, SETUP29D, COUNT(setup29d_words),
                                                setup29d_words};

#endif
