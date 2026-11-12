/* SPDX-License-Identifier: MIT */
/* DAPIDLER_PROGRAM.agc, the LM's: Luminary099 pages 1410-1420. The idler in
 * bank 16 (CHEKBITS to MOREIDLE, pp. 1410-1413) works in EBANK 6 (the
 * listing's EBANK= AOSQ), and so do the attitude error needles in bank 20
 * (ALTDSPLY, NEEDLER, pp. 1414-1420): "EBANK MUST BE SET TO 6". The two
 * banks' parts reach each other by DTCB. The faithful C is
 * c64-port/src/lm_dapidler.c. */
#include "dapidler.h"

#if LUMINARY099

/* EXTEND; DCA of a 2CADR. */
static void dca_constant(const word *pair)
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* ---- The idler, bank 16 -- LM pp. 1410-1413 ---- */

/* MOREIDLE -- LM p. 1413: "CALCULATE Q,R-AXES ATTITUDE ERRORS." by IBNKCALL
 * to QERRCALC (then CALCPERR, and SHUTDOWN). */
static void moreidle(void)
{
    tc_to(MOREIDLE, IBNKCALL);
}

/* SETTIME5 -- LM p. 1412: T5RUPT again in 100 ms, and RESUME the interrupted
 * program. */
static void settime5(void)
{
    set_A(MS100);
    ts(ADDRESS_OF(TIME5));
    continue_at(RESUME);
}

/* SHUTDOWN -- LM p. 1413: the next T5RUPT to DAPIDLER again; "KILL ANY
 * POSSIBLE JET REQUESTS", "COMMAND JETS OFF." (channels 5 and 6 to +0),
 * "TURN TRIM GIMBAL OFF" (BGIM23's bits of channel 12 cleared); "RETURN IN
 * 100 MSEC." */
static void idle_shutdown(void)
{
    dca_constant(IDLERADR);
    dxch(ADDRESS_OF(T5ADR));
    set_A(ZERO);
    ts(ADDRESS_OF(NEXTP));
    ts(ADDRESS_OF(NEXTU));
    ts(ADDRESS_OF(NEXTV));
    write_channel(CHAN5);
    write_channel(CHAN6);
    set_A(negate(BGIM23));
    wand_channel(CHAN12);
    settime5();
}

/* JUMPDSP -- LM p. 1413: "TRANSFER TO BANK 20" "FOR ATTITUDE ERROR
 * DISPLAYS", ALTDSPLY, by DTCB (A gets the word after it, L the old
 * BBANK). */
static void jumpdsp(void)
{
    dca_constant(DSPCADR);
    dtcb(JUMPDSP + 2u);
}

/* CHEKBITS -- LM p. 1410. Bits 13 and 14 of channel 31 both 1 (read
 * inverted: both 0): the mode select switch is off, "AND SO THE DAP SHOULD
 * BE OFF," and MOREIDLE. Else, with the IMU usable (bit 6 of IMODES33
 * clear), the needles (JUMPDSP); and with it not usable, "SET UP
 * INITIALIZATION" "FLAG FOR ATT ERROR DISPLAY ROUTINE." (bit 3 of RCSFLAGS)
 * and SHUTDOWN. The caller's return stays in Q, for CHEKMORE's RETURN. */
void chekbits(void)
{
    read_channel(CHAN31);
    com();
    mask(BIT13_14);
    if (A == 0u) { /* EXTEND; BZF MOREIDLE */
        moreidle();
        return;
    }

    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: jumpdsp(); return;
    case IS_PLUS_ZERO: cs(ADDRESS_OF(RCSFLAGS)); /* fall through */
    case NEGATIVE: mask(BIT3);                   /* fall through */
    case IS_MINUS_ZERO: break;
    }
    ads(ADDRESS_OF(RCSFLAGS));
    idle_shutdown();
}

/* DAPIDLER +1 -- LM p. 1411, past its LXCH: QRUPT gets Q, and 1/ACCSET is
 * set up once since the last fresh start or restart (bit 13 of RCSFLAGS then
 * set), by a NOVAC job, which returns to CHECKUP; or, already set up,
 * CHECKUP. */
static void dapidler_plus1(void)
{
    qxch(ADDRESS_OF(QRUPT));
    ca(ADDRESS_OF(RCSFLAGS));
    mask(BIT13);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: checkup(); return;
    case IS_PLUS_ZERO: set_A(BIT13); /* fall through */
    case NEGATIVE:
        ads(ADDRESS_OF(RCSFLAGS)); /* "BIT 13 IS 1." */
                                   /* fall through */
    case IS_MINUS_ZERO: break;
    }
    set_A(PRIO27);
    tc_to(CHECKUP - 3u, NOVAC); /* "SET UP JOB TO DO A LITTLE INITIALIZATION" */
}

/* CHEKMORE -- LM p. 1410: bit 10 of channel 30 ("PGNCS CONTROL OF S/C",
 * "BITS IN 30 ARE INVERTED"): set, the LM is not under PGNCS control,
 * MOREIDLE; clear, RETURN to CHEKBITS's caller. (A RAND leaves no negative
 * A: CCS's last two exits run on into DAPIDLER and its +1.) */
void chekmore(void)
{
    set_A(BIT10);
    rand_channel(CHAN30);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: moreidle(); return;
    case IS_PLUS_ZERO: tc_through(Q_ADDRESS); return; /* RETURN */
    case NEGATIVE: dapidler(); return;
    case IS_MINUS_ZERO: dapidler_plus1(); return;
    }
}

/* DAPIDLER -- LM p. 1411, "DAPIDLER ENTRY.": "INTERRUPT LEAD INS
 * (CONTINUED)", L to BANKRUPT. */
void dapidler(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    dapidler_plus1();
}

/* CHECKUP -- LM p. 1411: TC CHEKBITS, "CHECK TO SEE IF LM DAP IS TO GO ON
 * AND", which returns (by CHEKMORE) to CHECKUP +1. */
void checkup(void)
{
    tc_at(CHECKUP);
    chekbits();
}

/* CHECKUP +1 -- LM p. 1411: "IF 1/ACCS HAS NOT BEEN COMPLETED, IDLE."
 * (ACCSOKAY of DAPBOOLS clear), MOREIDLE. Else STARTDAP: "ZERO ATTITUDE
 * ERROR AND DESIRED RATES." by IBNKCALL to ZATTEROR. */
void checkup_after_chekbits(void)
{
    ca(ADDRESS_OF(DAPBOOLS));
    mask(ACCSOKAY);
    if (A == 0u) { /* EXTEND; BZF MOREIDLE */
        moreidle();
        return;
    }
    tc_to(STARTDAP, IBNKCALL);
}

/* STARTDAP +2 -- LM p. 1411, where ZATTEROR returns: "INITIALIZE:" the DAP's
 * erasables to +0 (the jet times, the rates, the trap counts, the offset
 * acceleration estimates and their downlist copies, the Kalman filter terms,
 * the gimbal drive timers and the minimum impulse words), MANFLAG's bits of
 * RCSFLAGS cleared, the old CDU angles OLDXFORP, OLDYFORP and OLDZFORQ from
 * the CDUs, bit 12 of RCSFLAGS set, the T6 jet timing cleared (TIME6, T6NEXT
 * and T6FURTHA at POSMAX), DAPZRUPT -10, the trap counts 2, and the next
 * T5RUPT to PAXIS: the DAP is on. */
void startdap_after_zatteror(void)
{
    static const unsigned zeroed[] = {
        ADDRESS_OF(TJP),       ADDRESS_OF(TJU),      ADDRESS_OF(TJV),       ADDRESS_OF(OMEGAP),
        ADDRESS_OF(OMEGAQ),    ADDRESS_OF(OMEGAR),   ADDRESS_OF(TRAPEDP),   ADDRESS_OF(TRAPEDQ),
        ADDRESS_OF(TRAPEDR),   ADDRESS_OF(AOSQ),     ADDRESS_OF(AOSQ) + 1u, ADDRESS_OF(AOSR),
        ADDRESS_OF(AOSR) + 1u, ADDRESS_OF(ALPHAQ),   ADDRESS_OF(ALPHAR),    ADDRESS_OF(NEGUQ),
        ADDRESS_OF(NEGUR),     ADDRESS_OF(AOSQTERM), ADDRESS_OF(AOSRTERM),  ADDRESS_OF(QACCDOT),
        ADDRESS_OF(RACCDOT),   ADDRESS_OF(ALLOWGTS), ADDRESS_OF(COTROLER),  ADDRESS_OF(INGTS),
        ADDRESS_OF(QGIMTIMR),  ADDRESS_OF(RGIMTIMR), ADDRESS_OF(OLDPMIN),   ADDRESS_OF(OLDQRMIN),
        ADDRESS_OF(PJETCTR),   ADDRESS_OF(UJETCTR),  ADDRESS_OF(VJETCTR),
    };
    unsigned i;

    set_A(ZERO);
    for (i = 0; i < sizeof zeroed / sizeof zeroed[0]; ++i) ts(zeroed[i]);

    set_A(negate(MANFLAG)); /* "RCSFLAGS INITIALIZATION." */
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));

    dca(ADDRESS_OF(CDUX)); /* "OLDXFORP AND OLDYFORP" */
    dxch(ADDRESS_OF(OLDXFORP));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(OLDZFORQ));
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT12);
    ads(ADDRESS_OF(RCSFLAGS)); /* "BIT 12 SET TO 1." */

    set_A(FOUR);
    ts(ADDRESS_OF(SKIPU));
    ts(ADDRESS_OF(SKIPV));
    set_A(POSMAX);
    ts(ADDRESS_OF(TIME6));
    ts(ADDRESS_OF(T6NEXT));
    ts(ADDRESS_OF(T6FURTHA));
    set_A(ZERO);
    ts(ADDRESS_OF(T6NEXT) + 1u);
    ts(ADDRESS_OF(T6FURTHA) + 1u);
    ts(ADDRESS_OF(NXT6ADR));
    ts(ADDRESS_OF(NEXTP));
    ts(ADDRESS_OF(NEXTU));
    ts(ADDRESS_OF(NEXTV));
    set_A(negate(TEN));
    ts(ADDRESS_OF(DAPZRUPT)); /* "JASK NOT IN PROGRESS, INITIALIZE NEG." */
    set_A(TWO);
    ts(ADDRESS_OF(NPTRAPS));
    ts(ADDRESS_OF(NQTRAPS));
    ts(ADDRESS_OF(NRTRAPS));
    dca_constant(PAXADIDL);
    dxch(ADDRESS_OF(T5ADR));
    settime5();
}

/* MOREIDLE +2 -- LM p. 1413, where QERRCALC returns: "CALCULATE P AXIS
 * ATTITUDE ERRORS." by IBNKCALL to CALCPERR, which returns to SHUTDOWN. */
void moreidle_after_qerrcalc(void)
{
    tc_to(MOREIDLE + 2u, IBNKCALL);
}

void shutdown_after_calcperr(void)
{
    idle_shutdown();
}

/* ---- The attitude error needles, bank 20 -- LM pp. 1414-1420 ---- */

/* RETNMORE -- LM p. 1420: "RETURN TO CHEKMORE", by DTCB to bank 16. */
static void retnmore(void)
{
    dca_constant(MORECADR);
    dtcb(RETNMORE + 2u);
}

/* TC OVERSUB2 at at: "RETURNS  A  UNCHANGED  OR LIMITED TO" "POSMAX OR
 * NEGMAX IF A HAS OVERFLOW" (TS 7 skips TC Q on an overflow, A then +1 or
 * -1 to INDEX the CS LIMITS by); back by TC Q, which leaves Q 3. */
static void oversub2(word at)
{
    tc_at(at);
    if (ts(ZERO_ADDRESS)) cs(indexed(ADDRESS_OF(LIMITS), a_word()));
    tc_through(Q_ADDRESS);
}

/* DSPLYTOT -- LM pp. 1415-1416: "CALCULATE GIMBAL ANGLE TOTAL ERRORS,
 * RESOLVE INTO PILOT AXES, STORE TOTAL ERRORS FOR NEEDLER." The desired
 * angles CTHETA and CPSI less the CDUs (by MSU, one's complement at pi
 * radians) through the gimbal-to-pilot matrix M21, M22 (the Q error), M31,
 * M32 (the R error) and M11 (the P error, plus CPHI less CDUX: "M13 = 1,
 * SO BYPASS THE MULTIPLICATION."), each pinned by OVERSUB2. Q is kept in
 * ITEMP1 meanwhile ("SAVE Q FOR CHEKBITS RETURN."). */
static void dsplytot(void)
{
    qxch(ADDRESS_OF(ITEMP1));

    ca(ADDRESS_OF(CTHETA));
    msu_at(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(AK)); /* "SAVE FOR R-ERROR CALCULATION." */
    mp_at(ADDRESS_OF(M21));
    xch(ADDRESS_OF(AK) + 1u);
    ca(ADDRESS_OF(CPSI));
    msu_at(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(AK) + 2u);
    mp_at(ADDRESS_OF(M22));
    ad_at(ADDRESS_OF(AK) + 1u);
    oversub2(DSPLYTOT + 020u); /* "PIN NEEDLES IN CASE OF OVERFLOW" */
    ts(ADDRESS_OF(AK) + 1u);

    ca(ADDRESS_OF(AK)); /* "Y-AXIS DIFFERENCE STORED BY Q-AXIS CALC." */
    mp_at(ADDRESS_OF(M31));
    xch(ADDRESS_OF(AK) + 2u); /* "FIRST TERM OF R ERROR." */
    mp_at(ADDRESS_OF(M32));
    ad_at(ADDRESS_OF(AK) + 2u);
    oversub2(DSPLYTOT + 031u);
    ts(ADDRESS_OF(AK) + 2u);

    ca(ADDRESS_OF(AK));
    mp_at(ADDRESS_OF(M11));
    xch(ADDRESS_OF(AK));
    ca(ADDRESS_OF(CPHI));
    msu_at(ADDRESS_OF(CDUX));
    ad_at(ADDRESS_OF(AK));
    oversub2(DSPLYTOT + 043u);
    ts(ADDRESS_OF(AK));

    qxch(ADDRESS_OF(ITEMP1)); /* "RESTORE Q FOR CHEKBITS RETURN." */
    retnmore();
}

/* NEEDLES -- LM p. 1419: the DAC loop, for DINDX 2, 1 and 0 (AK +2, AK +1
 * and AK): "RESCALE INPUTS TO + OR - 1800 DEGREES." (by -0.1), limited to +
 * or - 384 counts (added to + or - DACLIMIT, 16000: "OVFLO CHK", and then
 * "ON OVERFLOW LIMIT OUTPUT TO +-384"); the change from the DAC's present
 * value EDRIVEX added to the CDU command counter, and the new value kept.
 * Then "SET DAC ACTIVITY BITS" (13, 14 and 15 of channel 14). */
static void needles(void)
{
    set_A(TWO);
    do { /* DACLOOP */
        ts(ADDRESS_OF(DINDX));
        set_A(negate(ONETENTH));
        mp_at(indexed(ADDRESS_OF(AK), fetch(ADDRESS_OF(DINDX))));
        ts(L_ADDRESS);
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: ca(DACLIMIT); break;
        case IS_PLUS_ZERO: break;
        case NEGATIVE: cs(DACLIMIT); break;
        case IS_MINUS_ZERO: break;
        }
        ad_at(L_ADDRESS);
        if (ts(ADDRESS_OF(T5TEMP))) {
            ca(indexed(DACLIMIT, a_word()));
            ts(L_ADDRESS);
        }
        cs(indexed(ADDRESS_OF(EDRIVEX), fetch(ADDRESS_OF(DINDX)))); /* "CURRENT VALUE OF DAC" */
        ad_at(L_ADDRESS);
        ads(indexed(ADDRESS_OF(CDUXCMD), fetch(ADDRESS_OF(DINDX))));
        lxch(indexed(ADDRESS_OF(EDRIVEX), fetch(ADDRESS_OF(DINDX))));
    } while (ccs_at(ADDRESS_OF(DINDX)) == POSITIVE); /* CCS DINDX: 2, 1 or 0 */
    set_A(BITS13_14_15);
    wor_channel(CHAN14);
    retnmore();
}

/* NEEDLER -- LM pp. 1417-1419, "FDAI ATTITUDE ERROR DISPLAY SUBROUTINE".
 * Bits 3 and 2 of RCSFLAGS step the turn-on: bit 3 (the first pass), the
 * IMU error counters disabled "TO ZERO DACS", the inputs, the display
 * registers and the CDU command counters zeroed (NEEDLE11), and bit 2 set;
 * bit 2, "ENABLE IMU ERROR COUNTERS" and both clear. With both clear, the
 * needles are driven (NEEDLES) while the error counters are enabled, else
 * bit 3 again ("IF NOT, RE-INITIALIZE NEEDLER."). Each way, RETNMORE. */
static void needler(void)
{
    ca(ADDRESS_OF(RCSFLAGS));
    mask(SIX);
    if (A == 0u) { /* EXTEND; BZF NEEDLES3 */
        set_A(BIT6);
        rand_channel(CHAN12);
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: needles(); return;
        case IS_PLUS_ZERO: cs(ADDRESS_OF(RCSFLAGS)); /* fall through */
        case NEGATIVE: mask(BIT3);                   /* fall through */
        case IS_MINUS_ZERO: break;
        }
        ads(ADDRESS_OF(RCSFLAGS));
        retnmore();
        return;
    }

    mask(BIT3);
    if (A == 0u) { /* EXTEND; BZF NEEDLER2: "BIT3 = 0, BIT2 = 1" */
        set_A(BIT6);
        wor_channel(CHAN12);
        set_A(negate(SIX)); /* "RESET RCSFLAGS TO DISPLAY ATTITUDE" */
        mask_at(ADDRESS_OF(RCSFLAGS));
        ts(ADDRESS_OF(RCSFLAGS));
        retnmore();
        return;
    }

    set_A(negate(BIT6)); /* "FIRST PASS BIT3 = 1" */
    wand_channel(CHAN12);
    set_A(negate(ZERO)); /* NEEDLE11 */
    ts(ADDRESS_OF(AK));  /* "ZERO THE INPUTS ON FIRST PASS" */
    ts(ADDRESS_OF(AK1));
    ts(ADDRESS_OF(AK2));
    ts(ADDRESS_OF(EDRIVEX)); /* "ZERO THE DISPLAY REGISTERS" */
    ts(ADDRESS_OF(EDRIVEY));
    ts(ADDRESS_OF(EDRIVEZ));
    ts(ADDRESS_OF(CDUXCMD)); /* "ZERO THE OUT COUNTERS" */
    ts(ADDRESS_OF(CDUYCMD));
    ts(ADDRESS_OF(CDUZCMD));
    set_A(negate(SIX)); /* "RESET RCSFLAGS FOR PASS2" */
    mask_at(ADDRESS_OF(RCSFLAGS));
    ad(BIT2);
    ts(ADDRESS_OF(RCSFLAGS));
    retnmore();
}

/* ALTDSPLY -- LM p. 1414. It "REVERSES THE DSPLYALT BIT OF RCSFLAGS EACH
 * TIME": set, NEEDLER drives the needles; clear, the
 * errors for the next pass go to AK, AK +1 and AK +2, the DAP's following
 * errors (complemented: "ERROR COMPLEMENTS ARE INPUT TO NEEDLER.") with
 * NEEDLFLG clear, the total attitude errors (DSPLYTOT) with it set. */
void altdsply(void)
{
    ca(ADDRESS_OF(RCSFLAGS)); /* "INVERT THE DISPLAY ALTERNATION BIT." */
    ts(L_ADDRESS);
    set_A(DSPLYALT);
    rxor_lchan();
    ts(ADDRESS_OF(RCSFLAGS));
    mask(DSPLYALT);
    switch (ccs_register(A_ADDRESS)) { /* "IS ALTERNATION FLAG ZERO?" */
    case POSITIVE: needler(); return;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(FLAGWRD0)); /* fall through */
    case NEGATIVE: mask(NEEDLBIT);               /* fall through */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: dsplytot(); return;
    case IS_PLUS_ZERO: cs(ADDRESS_OF(QERROR)); /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(AK) + 1u);    /* fall through */
    case IS_MINUS_ZERO: break;
    }
    cs(ADDRESS_OF(RERROR));
    ts(ADDRESS_OF(AK) + 2u);
    cs(ADDRESS_OF(PERROR));
    xch(ADDRESS_OF(AK));
    retnmore(); /* "DISPLAY THESE THE NEXT TIME THROUGH" */
}

#endif
