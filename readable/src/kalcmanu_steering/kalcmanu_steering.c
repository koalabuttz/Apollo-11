/* SPDX-License-Identifier: MIT */
/* KALCMANU_STEERING.agc (Luminary099 pp. 365-369): "GENERATION OF STEERING
 * COMMANDS FOR DIGITAL AUTOPILOT FREE FALL MANEUVERS. NEW COMMANDS WILL BE
 * GENERATED EVERY ONE SECOND DURING THE MANEUVER." Its interpretive code
 * in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs (src/binding/kalcmanu_steering_routines.c), and
 * its basic code. The faithful C is c64-port/src/lm_kalcmanu_steering.c.
 *
 * KALCMANU (ATTITUDE_MANEUVER_ROUTINE) has worked out the maneuver: the
 * axis and the angle, and from them (WCALC, GIMBAL_LOCK_AVOIDANCE) DEL, the
 * matrix that turns the spacecraft through one second's worth of it, the
 * rates BRATE and the time TM. Then once a second, NEWDELHI: MIS (the
 * spacecraft's axes in stable member axes) turned through DEL, the CDU
 * angles of the new attitude, NCDU, and the difference from the last
 * ones, a tenth of it a tenth of a second for the DAP to add to CDUXD,
 * CDUYD, CDUZD. MANUSTAT starts it (the rates and the time), CONTMANU
 * calls for the next second under WAITLIST (UPDTCALL starts NEWDELHI as a
 * job), and within a second of the end MANUSTOP stops it on the final
 * angles and gives the computer back to the routine that asked for the
 * maneuver (SPVAC, at ATTCADR's 2CADR).
 *
 * Each call out of the file (BANKCALL, WAITLIST, FINDVAC, SPVAC) leaves Z
 * at the routine and Q at the word after the TC, where the routine comes
 * back to; ENDOFJOB, TASKOVER and NOGO -2 don't come back. */
#include "kalcmanu_steering.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void incrdcdu(void);
static void tmanuchk(void);
static void manustal(void);
static void contmanu(void);

/* ---- The new angles, each second ---- */

/* NEWDELHI -- LM p. 365. "CHECK FOR AUTO STABILIZATION ONLY": ISITAUTO,
 * through BANKCALL, reads the mode control switch (bit 14 of channel 31). */
void newdelhi(void)
{
    tc_at(NEWDELHI);
    continue_at(BANKCALL); /* CADR ISITAUTO */
}

/* NEWDELHI +2 -- LM p. 365. ISITAUTO's answer: "(+) = NOT IN AUTO" gives
 * the maneuver up (NOGO -2); "(+0) = AOK" goes on to NEWANGL. (Its answer is
 * a bit of a channel: never negative.) */
void newdelhi_after_isitauto(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: continue_at(NOGO - 2u); return; /* TCF NOGO -2 */
    case IS_PLUS_ZERO: newangl(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (into NEWANGL's interpretive words) */
    }
}

/* NEWANGL -- LM p. 365. Into the interpreter, for NEWANGL_CODE. */
void newangl(void)
{
    tc_at(NEWANGL);
    intpret();
}

/* NEWANGL +1 -- LM p. 365.
 * "COMPUTE THE NEW MATRIX FROM S/C TO STABLE MEMBER AXES": MXM3 multiplies
 * MIS (X1) by KEL (X2), MIS turned through one second of the maneuver; it
 * leaves the product on the push-down list, and three STADRs store it back
 * into MIS a row at a time, the last row first. DCMTOCDU "PICK[S] UP THE
 * NEW CDU ANGLES FROM MATRIX", and RTB V1STO2S makes them CDU angles, "NEW
 * CDU ANGLES", NCDU. BONCLR CALCMAN2: on the first second (CALCMAN2 set,
 * "TO START MANEUVER") the flag is cleared and MANUSTAT starts it; on the
 * others EXIT goes straight on to the increments. */
static const word newangl_words[] = {
    ORDERS(AXC_1, AXC_2), ADDRESS(MIS), /* "COMPUTE THE NEW MATRIX FROM S/C TO" */
    ADDRESS(KEL),                       /* "STABLE MEMBER AXES" */
    ORDER(CALL),          ADDRESS(MXM3),
    ORDERS(VLOAD, STADR), AFTER_STADR(STOVL_AT(MIS, 12)), /* "CALCULATE NEW DESIRED CDU ANGLES" */
    ORDER(STADR),         AFTER_STADR(STOVL_AT(MIS, 6)),
    ORDER(STADR),         AFTER_STADR(STORE(MIS)),
    ORDERS(AXC_1, CALL),  ADDRESS(MIS),
    ADDRESS(DCMTOCDU), /* "PICK UP THE NEW CDU ANGLES FROM MATRIX" */
    ORDER(RTB),           ADDRESS(V1STO2S),
    STORE(NCDU), /* "NEW CDU ANGLES" */
    ORDERS(BONCLR, EXIT), SWITCH(BONCLR, CALCMAN2),
    ADDRESS(MANUSTAT), /* "TO START MANEUVER" */
};

/* NEWANGL +025 -- LM p. 365. After the EXIT ("+0 OTHERWISE"): the
 * increments for the three axes, from the last one (2) down. */
void newangl_after_exit(void)
{
    set_A(TWO); /* CAF TWO */
    incrdcdu();
}

/* INCRDCDU -- LM pp. 365-366.
 * For each axis (SPNDX 2, 1, 0): the new angle less the last one ("INITIAL
 * CDU ANGLES OR PREVIOUS DESIRED CDU ANGLES", BCDU), as MSU gives it, in
 * ones' complement; a tenth of it (DT/TAU) "CONVERT[ED] TO 2S COMPLEMENT"
 * (the CCS table: a negative one, -(|x| - 1), has the bits of -|x| in two's
 * complement) is DELDCDU, the "ANGLE INCREMENTS TO BE ADDED TO CDUXD,
 * CDUYD, CDUZD EVERY TENTH SECOND BY LEM DAP". The new angle becomes the
 * last one, and the last one goes to CDUXD (XCH BCDU; TS CDUXD): the DAP
 * starts the second from it. Then the time ("RELINT" after MANUSTAT's
 * INHINT). (SPNDX counts 2, 1, 0, so each INDEX is an axis and every TS
 * stores a word: none skips.) */
static void incrdcdu(void)
{
    word axis;

    do {
        ts(ADDRESS_OF(SPNDX));
        axis = fetch(ADDRESS_OF(SPNDX));
        ca(indexed(ADDRESS_OF(BCDU), axis));     /* "INITIAL CDU ANGLES */
        msu_at(indexed(ADDRESS_OF(NCDU), axis)); /* OR PREVIOUS DESIRED CDU ANGLES" */
        mp(DT_TAU);
        switch (ccs_at(A_ADDRESS)) {   /* "CONVERT TO 2S COMPLEMENT" */
        case POSITIVE: ad(ONE); break; /* AD ONE; TCF +2 */
        case IS_PLUS_ZERO: break;      /* TCF +2 */
        case NEGATIVE: com(); break;
        case IS_MINUS_ZERO: break;
        }
        ts(indexed(ADDRESS_OF(DELDCDU), axis)); /* "ANGLE INCREMENTS TO BE ADDED TO */
        ca(indexed(ADDRESS_OF(NCDU), axis));    /* CDUXD, CDUYD, CDUZD EVERY TENTH */
        xch(indexed(ADDRESS_OF(BCDU), axis));   /* SECOND BY LEM DAP" */
        ts(indexed(ADDRESS_OF(CDUXD), axis));
    } while (ccs_at(ADDRESS_OF(SPNDX)) == POSITIVE); /* "LOOP FOR THREE AXES" */
    relint();
    tmanuchk();
}

/* ---- The time ---- */

/* TC Q: the routine returns to the address in Q, and Q becomes 3.
 * Returns the address. */
static word tc_q(void)
{
    word at = (word)(Q & 07777u);

    Q = 3u;
    Z = at;
    return at;
}

/* TIMECHK -- LM p. 366.
 * TM (MANUSTAT made it the time the maneuver ends, less a second) less the
 * time now (TIME2, TIME1), in TTEMP: the time left. TIMECHK returns to the
 * word after its TC with more than a second left (either word positive);
 * otherwise one word further on (2NDRETRN), or two with a part of a
 * second left (MANUOFF: less than ONESEK below TM): A, the centiseconds
 * till the end, plus one. */
static word timechk(void)
{
    dcs(ADDRESS_OF(TIME2)); /* EXTEND; DCS TIME2 */
    dxch(ADDRESS_OF(TTEMP));
    dca(ADDRESS_OF(TM)); /* EXTEND; DCA TM */
    das(ADDRESS_OF(TTEMP));
    switch (ccs_at(ADDRESS_OF(TTEMP))) {
    case POSITIVE: return tc_q();                  /* TC Q */
    case NEGATIVE: incr(Q_ADDRESS); return tc_q(); /* TCF 2NDRETRN */
    case IS_PLUS_ZERO:                             /* TCF +2 */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(ADDRESS_OF(TTEMP) + 1u)) {
    case POSITIVE: return tc_q(); /* TC Q */
    case NEGATIVE: com(); break;
    case IS_PLUS_ZERO: /* TCF MANUOFF */
    case IS_MINUS_ZERO: break;
    }
    /* MANUOFF */
    ad(ONESEK_LOW);
    if (!A_is_zero_or_minus()) incr(Q_ADDRESS); /* EXTEND; BZMF 2NDRETRN */
    incr(Q_ADDRESS);                            /* 2NDRETRN */
    return tc_q();
}

/* TMANUCHK -- LM p. 366. "COMPARE PRESENT TIME WITH TIME TO TERMINATE
 * MANEUVER": with more than a second left, on (CONTMANU); with none, MANUSTOP
 * a centisecond from now (CAF ONE); with a part of one, then (MANUSTAL).
 * (TIMECHK returns to one of the three: TMANUCHK's TC leaves Q at the
 * first.) */
static void tmanuchk(void)
{
    tc_at(TMANUCHK);
    switch (timechk()) {
    case TMANUCHK + 1u: contmanu(); return; /* TCF CONTMANU */
    case TMANUCHK + 2u: set_A(ONE); break;  /* CAF ONE */
    default: break;                         /* MANUSTAL */
    }
    manustal();
}

/* TMANUCHK +1 -- LM p. 366. TIMECHK's first return: TCF CONTMANU. */
void tmanuchk_after_timechk(void)
{
    contmanu();
}

/* MANUSTAL -- LM p. 366. "END MAJOR PART OF MANEUVER WITHIN 1 SEC UNDER
 * WAITLIST CALL TO MANUSTOP", A centiseconds from now. */
static void manustal(void)
{
    inhint();
    tc_at(MANUSTAL + 1u);
    continue_at(WAITLIST); /* 2CADR MANUSTOP */
}

/* MANUSTAL +4 -- LM p. 366. After WAITLIST: the job ends. */
void manustal_after_waitlist(void)
{
    relint();
    continue_at(ENDOFJOB);
}

/* ---- Starting the maneuver ---- */

/* MANUSTAT -- LM p. 366. The "INITIALIZATION ROUTINE FOR AUTOMATIC
 * MANEUVERS", where BONCLR goes on the first second: EXIT. */
static const word manustat_words[] = {
    ORDER(EXIT),
};

/* MANUSTAT +1 -- LM pp. 366-367.
 * TM, the maneuver's time, plus the time now: "TM+TO MANEUVER COMPLETION
 * TIME", less ONESEK: "(TM+TO)-1". Then, for each axis (KSPNDX 2, 1, 0,
 * KDPNDX twice that, for the double words of BRATE), "STORE MANEUVER
 * RATE IN OMEGAPD, OMEGAQD, OMEGARD" and "COMPUTE ATTITUDE ERROR OFFSET =
 * (WX)ABS(WX)/2AJX WHERE AJX= 2-JET ACCELERATION": -|WX| times BIASCALE
 * ("= -1/16") times WX, over 1JACC, the axis's acceleration, is DELPEROR
 * (DELQEROR, DELREROR), which the DAP steers by. The next second starts a
 * second from now (NEXTIME), and the increments for this one go on
 * (INCRDCDU -1). (KSPNDX counts 2, 1, 0: every TS stores a word.) */
void manustat_after_exit(void)
{
    word axis;

    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA TIME2 */
    das(ADDRESS_OF(TM));    /* "TM+TO MANEUVER COMPLETION TIME" */
    dcs(ONESEK);            /* EXTEND; DCS ONESEK */
    das(ADDRESS_OF(TM));    /* "(TM+TO)-1" */
    inhint();
    set_A(TWO); /* CAF TWO */
    do {
        /* RATEBIAS */
        ts(ADDRESS_OF(KSPNDX));
        axis = fetch(ADDRESS_OF(KSPNDX));
        double_A();
        ts(ADDRESS_OF(KDPNDX));
        ca(indexed(ADDRESS_OF(BRATE), a_word())); /* INDEX A */
        ts(indexed(ADDRESS_OF(OMEGAPD), axis));   /* "STORE MANEUVER RATE IN */
        if (!A_is_zero_or_minus()) com();         /* OMEGAPD, OMEGAQD, OMEGARD" */
        mp(BIASCALE);                             /* "= -1/16" */
        mp_at(indexed(ADDRESS_OF(BRATE), fetch(ADDRESS_OF(KDPNDX))));
        dv_at(indexed(ADDRESS_OF(1JACC), axis)); /* "=AJX $ 90 DEG/SEC-SEC" */
        ts(indexed(ADDRESS_OF(DELPEROR), axis)); /* "$ 180 DEG" */
    } while (ccs_at(ADDRESS_OF(KSPNDX)) == POSITIVE);
    ca(ADDRESS_OF(TIME1));
    ad(ONESEK_LOW);
    xch(ADDRESS_OF(NEXTIME));
    newangl_after_exit(); /* TCF INCRDCDU -1 */
}

/* ---- The next second ---- */

/* CONTMANU -- LM p. 367. "RESET FOR NEXT DCDU UPDATE": NEXTIME less
 * TIME1, the centiseconds till the next second, as WAITLIST takes them
 * (CCS less one, and AD ONE puts it back); a negative one is TIME1 come
 * round past 037777 (AD NEGMAX; COM makes it 040000 less). */
static void contmanu(void)
{
    cs(ADDRESS_OF(TIME1));
    ad_at(ADDRESS_OF(NEXTIME));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ad(ONE); break; /* AD ONE; TCF MANUCALL */
    case IS_PLUS_ZERO: break;      /* TCF MANUCALL */
    case NEGATIVE:
        ad(NEGMAX);
        com();
        break;
    case IS_MINUS_ZERO: com(); break;
    }
    /* MANUCALL: "CALL FOR NEXT UPDATE VIA WAITLIST" */
    inhint();
    tc_at(MANUCALL + 1u);
    continue_at(WAITLIST); /* 2CADR UPDTCALL */
}

/* MANUCALL +4 -- LM p. 368. After WAITLIST: "INCREMENT TIME FOR NEXT
 * UPDATE", and the job ends. */
void manucall_after_waitlist(void)
{
    set_A(ONESEK_LOW); /* CAF ONESEK +1 */
    ads(ADDRESS_OF(NEXTIME));
    continue_at(ENDOFJOB);
}

/* UPDTCALL -- LM p. 368. The WAITLIST task: "SATELLITE PROGRAM TO CALL FOR
 * UPDATE OF STEERING COMMANDS", NEWDELHI as a job with a VAC area, at
 * priority 26. */
void updtcall(void)
{
    set_A(PRIO26);
    tc_at(UPDTCALL + 1u);
    continue_at(FINDVAC); /* 2CADR NEWDELHI */
}

/* UPDTCALL +4 -- LM p. 368. After FINDVAC: the task ends. */
void updtcall_after_findvac(void)
{
    tc_at(UPDTCALL + 4u);
    continue_at(TASKOVER);
}

/* ---- Ending the maneuver ---- */

/* MANUSTOP -- LM p. 369. "ROUTINE FOR TERMINATING AUTOMATIC MANEUVERS"
 * (a WAITLIST task): "ZERO MANEUVER RATES" and the increments and offsets
 * of each axis, and "SET DESIRED GIMBAL ANGLES TO DESIRED FINAL GIMBAL
 * ANGLES": CPSI, CTHETA, CPHI. (Each TS stores a word: none skips.) */
void manustop(void)
{
    set_A(ZERO); /* CAF ZERO: "ZERO MANEUVER RATES" */
    ts(ADDRESS_OF(DELDCDU2));
    ts(ADDRESS_OF(OMEGARD));
    ts(ADDRESS_OF(DELREROR));
    ts(ADDRESS_OF(DELDCDU1));
    ts(ADDRESS_OF(OMEGAQD));
    ts(ADDRESS_OF(DELQEROR));
    ca(ADDRESS_OF(CPSI));  /* "SET DESIRED GIMBAL ANGLES TO */
    ts(ADDRESS_OF(CDUZD)); /* DESIRED FINAL GIMBAL ANGLES" */
    ca(ADDRESS_OF(CTHETA));
    ts(ADDRESS_OF(CDUYD));
    /* ENDROLL: "NO FINAL YAW" */
    ca(ADDRESS_OF(CPHI));
    ts(ADDRESS_OF(CDUXD));
    set_A(ZERO);
    ts(ADDRESS_OF(OMEGAPD)); /* "I.E. MANEUVER DID NOT GO THRU */
    ts(ADDRESS_OF(DELDCDU)); /* GIMBAL LOCK ORIGINALLY" */
    ts(ADDRESS_OF(DELPEROR));
    goodmanu();
}

/* GOODMANU -- LM p. 369. "RESTORE USERS PRIO" (ATTPRIO, for SPVAC) and
 * "ZERO ATTCADR", taking its 2CADR, the user's return, into A,L; SPVAC
 * starts it again: "RETURN TO USER". */
void goodmanu(void)
{
    ca(ADDRESS_OF(ATTPRIO)); /* "RESTORE USERS PRIO" */
    ts(ADDRESS_OF(NEWPRIO));
    set_A(ZERO); /* CA ZERO: "ZERO ATTCADR" */
    dxch(ADDRESS_OF(ATTCADR));
    tc_at(GOODMANU + 4u);
    continue_at(SPVAC); /* "RETURN TO USER" */
}

/* GOODMANU +5 -- LM p. 369. After SPVAC: the task ends. */
void goodmanu_after_spvac(void)
{
    tc_at(GOODMANU + 5u);
    continue_at(TASKOVER);
}

const struct interpretive_code NEWANGL_CODE = {022u, NEWANGL + 1u, COUNT(newangl_words),
                                               newangl_words};
const struct interpretive_code MANUSTAT_CODE = {022u, MANUSTAT, COUNT(manustat_words),
                                                manustat_words};

#endif
