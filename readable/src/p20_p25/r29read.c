/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 18 (Luminary099 pp. 605-607): R29's radar reading,
 * the task R29READ and the job R29RDJOB (bank 24).
 *
 * "RADAR READING:  TASK R29READ AND JOB R29RDJOB.  ALL JOBS ARE NOVAC
 * TYPE." R29LOKON starts the task when the designate ends. Every 2
 * seconds the task starts the job, for as long as reading is "STILL
 * ALLOWED (NO TRACKER FAIL ETC.)" (READRFLG).
 *
 * The job reads the range rate, saving the time and the radar's and
 * IMU's CDU angles at the reading; then it reads the range ("BAD READ OR
 * SCALE CHANGE ... WHICH?": after a scale change it reads again), and lays
 * the lot out for the downlink in one go, inhibited, with TRKMKCNT 1 to
 * "SHOW THAT DOWNLINK DATA IS CONSISTENT."
 *
 * Any trouble ends the reading at ENDRRD29: R29 over (NOR29FLG), the
 * radar out of auto mode, or a bad read. There the job zeroes TRKMKCNT
 * ("DISCREDIT DOWNTEL."), clears READRFLG and removes the track enable, so
 * that R29 designates again. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- R29READ, the task ---- */

/* R29READ -- p. 605, "CALLED BY WAITLIST": "START JOB TO READ AND DOWNLINK
 * FOR R29." */
void r29read(void)
{
    set_A(PRIO26);
    tc_to(R29READ + 1u, NOVAC); /* 2CADR R29RDJOB */
}

/* R29READ +4: wait 2 seconds. */
void r29read_after_novac(void)
{
    set_A(TWO_SECS);
    tc_to(R29READ + 5u, VARDELAY);
}

/* R29READ +6: "2 SECONDS LATER, SEE IF READING IS STILL ALLOWED (NO
 * TRACKER FAIL ETC.)": "IT'S OK:  CALL IT AGAIN."; "IT AIN'T:  WAIT FOR
 * REDESIGNATE." */
void r29read_after_vardelay(void)
{
    ca(ADDRESS_OF(FLAGWRD3));
    mask(READRBIT);
    if (bit_set_in_A())       /* CCS A */
        continue_at(R29READ); /* TCF R29READ: "IT'S OK:  CALL IT AGAIN." */
    else
        continue_at(TASKOVER); /* "IT AIN'T:  WAIT FOR REDESIGNATE." */
}

/* ---- R29RDJOB, the job ---- */

/* ENDRRD29 -- p. 606: "TROUBLE MADE US COME HERE TO LEAVE THE RR-READING
 * MODE.  DISCREDIT DOWNTEL." (A +0: the TS does not skip), and clear
 * READRFLG. */
static void endrrd29(void)
{
    set_A(0u);
    ts(ADDRESS_OF(TRKMKCNT));
    tc_to(ENDRRD29 + 2u, DOWNFLAG); /* ADRES READRFLG */
}

/* R29RDJOB -- p. 605, "CALLED VIA NOVAC": test "NOR29FLG" ("R29 IS NOW
 * OVER, STOP AT ONCE.") and the "RR-NOT-IN-AUTO-MODE BIT" ("ASTRO TOOK RR
 * OUT OF AUTO MODE."); else "INITIATE READING OF RANGE RATE." */
void r29rdjob(void)
{
    ca(ADDRESS_OF(FLAGWRD3));
    mask(NR29FBIT);
    if (bit_set_in_A()) { /* CCS A: test "NOR29FLG". */
        endrrd29();       /* "R29 IS NOW OVER, STOP AT ONCE." */
        return;
    }
    ca(ADDRESS_OF(RADMODES));
    mask(AUTOMBIT);
    if (bit_set_in_A()) { /* CCS A */
        endrrd29();       /* "ASTRO TOOK RR OUT OF AUTO MODE." */
        return;
    }
    tc_to(R29RDJOB + 010u, BANKCALL); /* CADR RRRDOT */
}

/* R29RDJOB +012: "GOT TO SLEEP UNTIL IT'S READY." */
void r29rdjob_after_rrrdot(void)
{
    tc_to(R29RDJOB + 012u, BANKCALL); /* CADR RADSTALL */
}

/* R29RDJOB +014: RADSTALL's bad return, "BAD READ; REDESIGNATE." */
void r29rdjob_bad_read(void)
{
    endrrd29();
}

/* R29RANGE -- p. 606: "INITIATE READING OF RR RANGE." */
static void r29range(void)
{
    tc_to(R29RANGE, BANKCALL); /* CADR RRRANGE */
}

/* R29RDJOB +015: the range rate is read. The "TIME OF RR READING, FOR
 * DOWNLINK." (TIMEHOLD) to MPAC; then, inhibited to "BE SURE OF 5
 * CONSISTENT CDU ANGLES.", the "RRCDU ANGLES AT RR READ" (CDUT, CDUS) to
 * MPAC +2 and the "IMUCDU ANGLES AT RR READ" (CDUY, CDUZ, CDUX) to MPAC +4
 * and +6: "MPAC'S 7 WORDS ARE BUFFERED FOR COPYCYCLE." Then R29RANGE. */
void r29rdjob_rate_read(void)
{
    dxch(ADDRESS_OF(TIMEHOLD));
    dxch(ADDRESS_OF(MPAC)); /* "TIME OF RR READING, FOR DOWNLINK." */
    inhint();               /* "BE SURE OF 5 CONSISTENT CDU ANGLES." */
    dca(ADDRESS_OF(CDUT));  /* EXTEND; DCA CDUT */
    dxch(ADDRESS_OF(MPAC) + 2u);
    dca(ADDRESS_OF(CDUY)); /* EXTEND; DCA CDUY */
    dxch(ADDRESS_OF(MPAC) + 4u);
    ca(ADDRESS_OF(CDUX));
    if (ts(ADDRESS_OF(MPAC) + 6u)) not_modelled(); /* (A as CA leaves it does not overflow) */
    r29range();
}

/* R29RANGE +2: "GO TO SLEEP UNTIL IT'S READY." */
void r29range_after_rrrange(void)
{
    tc_to(R29RANGE + 2u, BANKCALL); /* CADR RADSTALL */
}

/* R29RRR? -- p. 606, RADSTALL's bad return: "BAD READ OR SCALE CHANGE
 * ... WHICH?" FLAGWRD5 bit 10 (RNGSCFLG) clear: "NO, A BAD READ;
 * REDESIGNATE."; set: "YES; CLEAR FLAG AND READ AGAIN." */
void r29range_bad_read(void)
{
    cs(ADDRESS_OF(FLAGWRD5));
    mask(BIT10);
    if (bit_set_in_A()) { /* CCS A: "WAS IT A SCALE CHANGE (REAL OR PHONY)?" */
        endrrd29();       /* "NO, A BAD READ; REDESIGNATE." */
        return;
    }
    tc_to(R29RRR_Q + 4u, DOWNFLAG); /* ADRES RNGSCFLG */
}

/* R29RRR? +6: "YES; CLEAR FLAG AND READ AGAIN." */
void r29rrr_q_after_downflag(void)
{
    r29range(); /* TCF R29RANGE */
}

/* R29RANGE +5: the range is read. "COPYCYCLE TO LAY OUT NEW R29 DOWNLINK."
 * (inhibited): DNRRANGE's two words to RM, MPAC's time to MKTIME, the radar's angles to TANGNB and the IMU's to AIG, AMG and AOG
 * (each DXCH takes the word pair the one before left in A and L). Then
 * TRKMKCNT 1, to "SHOW THAT DOWNLINK DATA IS CONSISTENT.", and the job
 * ends. */
void r29range_read(void)
{
    inhint();
    dxch(ADDRESS_OF(DNRRANGE)); /* "COPYCYCLE TO LAY OUT NEW R29 DOWNLINK." */
    dxch(ADDRESS_OF(RM));
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(MKTIME));
    dxch(ADDRESS_OF(MPAC) + 2u);
    dxch(ADDRESS_OF(TANGNB));
    dxch(ADDRESS_OF(MPAC) + 4u);
    dxch(ADDRESS_OF(AIG));
    ca(ADDRESS_OF(MPAC) + 6u);
    if (ts(ADDRESS_OF(AOG))) not_modelled(); /* (A as CA leaves it does not overflow) */
    set_A(ONE);
    ts(ADDRESS_OF(TRKMKCNT)); /* "SHOW THAT DOWNLINK DATA IS CONSISTENT." */
    continue_at(ENDOFJOB);    /* TCF ENDOFJOB */
}

/* ENDRRD29 +4 -- p. 607: "REMOVE TRACK-ENABLE DISCRETE.", and the job
 * ends. */
void endrrd29_after_downflag(void)
{
    set_A(negate(BIT14));
    wand_channel(CHAN12); /* "REMOVE TRACK-ENABLE DISCRETE." */
    continue_at(ENDOFJOB);
}

#endif
