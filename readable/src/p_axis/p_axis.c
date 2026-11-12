/* SPDX-License-Identifier: MIT */
/* P-AXIS_RCS_AUTOPILOT.agc, the LM's: Luminary099 pages 1421-1441, bank 16,
 * in EBANK 6 (the listing's EBANK= PERROR). "THE NOMINAL TIME BETWEEN THE
 * P-AXIS RUPTS IS 100 MS IN ALL NON-IDLING MODES OF THE DAP." The faithful C
 * is c64-port/src/lm_p_axis.c. */
#include "p_axis.h"

#if LUMINARY099

/* EXTEND; DCA of a 2CADR. */
static void dca_constant(const word *pair)
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* RETURN: TC Q. */
static void tc_q(void)
{
    tc_through(Q_ADDRESS);
}

/* ---- The subroutines -- LM p. 1422 ---- */

/* 1STOTWOS -- LM p. 1422, by TC at at: a CDU difference (one's complement,
 * from MSU) made two's complement, as the CDUs count: a positive one back as
 * it was (CCS A diminished it; AD ONE), a negative one one nearer zero (CS A
 * of the diminished magnitude). */
static void one_stotwos(word at)
{
    tc_at(at);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: ad(ONE); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: cs(A_ADDRESS); break;
    case IS_MINUS_ZERO: break;
    }
    tc_q();
}

/* DIVIDER -- LM p. 1422: OMEGAU (the double word) over DAPTREG4. */
static void divider(void)
{
    dxch(ADDRESS_OF(OMEGAU));
    dv_at(ADDRESS_OF(DAPTREG4));
    tc_q();
}

/* SUBDIVDE -- LM p. 1422 (or its +3, past the first term), by TC at at: the
 * "OVERFLOW PROTECTION ROUTINE TO GIVE" "POSMAX OR NEGMAX IF THE DIVIDE
 * WOULD" "OVERFLOW": OMEGAU plus A times DAPTEMP3, kept in DAPTEMP5 too,
 * over DAPTREG4 (1/40: the CDU change in 100 ms, a rate), when its high word
 * is small enough (zero, or less than OCT 630 in magnitude); else POSMAX
 * ("45 DEG/SEC") with its sign. */
static void subdivde(word at, bool plus3)
{
    tc_at(at);
    if (!plus3) {
        mp_at(ADDRESS_OF(DAPTEMP3));
        das(ADDRESS_OF(OMEGAU));
    }
    dca(ADDRESS_OF(OMEGAU));
    dxch(ADDRESS_OF(DAPTEMP5));
    switch (ccs_at(ADDRESS_OF(OMEGAU))) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE:
        ad(MOCT630);
        if (A_is_zero_or_minus()) { /* EXTEND; BZMF DIVIDER */
            divider();
            return;
        }
        break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: divider(); return; /* TCF DIVIDER; -0: EXTEND; BZMF DIVIDER, A +0 */
    }
    switch (ccs_at(ADDRESS_OF(OMEGAU))) {
    case POSITIVE: set_A(POSMAX); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: set_A(negate(POSMAX)); break;
    case IS_MINUS_ZERO: break;
    }
    tc_q();
}

/* OVERSUB -- LM p. 1422, by TC at at: "RETURNS A UNCHANGED OR LIMITED TO"
 * "POSMAX OR NEGMAX IF A HAS OVERFLOW" (TS 7 skips TC Q on an overflow, A
 * then +1 or -1 to INDEX the CS BIT15 -1 by: BIT15 or POSMAX,
 * complemented). */
static void oversub(word at)
{
    tc_at(at);
    if (ts(ZERO_ADDRESS)) cs(indexed(BIT15_ADDRESS - 1u, a_word()));
    tc_q();
}

/* ---- PAXIS -- LM pp. 1421-1422 ---- */

/* PAXIS -- LM p. 1421: "INTERRUPT LEAD IN (CONTINUED)" (T5 again in 100 ms,
 * L and Q to
 * BANKRUPT and QRUPT). "IF DAPZRUPT POSITIVE, DAP (JASK) IS" "STILL IN
 * PROGRESS AND A RESTART IS" "CALLED FOR.  IT IS NEVER ZERO": BAILOUT. Else
 * TC CHEKBITS, whose "RETURN IS TC I+1 IF DAP SHOULD STAY ON." (A -0
 * DAPZRUPT would run on past it.) */
void paxis(void)
{
    set_A(MS100);
    ads(ADDRESS_OF(TIME5));
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    switch (ccs_at(ADDRESS_OF(DAPZRUPT))) {
    case POSITIVE: tc_to(PAXIS + 6u, BAILOUT); return;
    case IS_PLUS_ZERO: tc_to(PAXIS + 7u, CHEKBITS); return; /* OCT 02000 runs as TC CHEKBITS */
    case NEGATIVE: paxis_plus10(); return;
    case IS_MINUS_ZERO: paxis_after_chekbits(); return;
    }
}

void paxis_plus10(void)
{
    tc_to(PAXIS + 010u, CHEKBITS);
}

/* PAXIS +11 -- LM p. 1421, where CHEKBITS returns: the CDUs to DAPTREG4, 5
 * and 6 ("READ AND STORE CDU'S"); the KALCMANU-DAP and rate-hold DAP
 * interface: the desired CDU angles moved on by DELCDUX, Y and Z ("EVERY 100
 * MS"); the manual direct rate times TCP and TCQR counted down. Then
 * PAXFILT: "EXECUTE ACDT+C12, IF NEEDED." (the trim gimbal's, CALLGMBL of
 * RCSFLAGS: "CALLGMBL IS NOT BIT15, SO THIS TEST IS" "VALID."), which
 * returns to PAXFILT +4. */
void paxis_after_chekbits(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(DAPTREG4));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(DAPTREG5));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(DAPTREG6));

    ca(ADDRESS_OF(CDUXD));
    msu_at(ADDRESS_OF(DELCDUX));
    one_stotwos(PAXIS + 022u);
    ts(ADDRESS_OF(CDUXD));
    ca(ADDRESS_OF(CDUYD));
    msu_at(ADDRESS_OF(DELCDUY));
    one_stotwos(PAXIS + 027u);
    ts(ADDRESS_OF(CDUYD));
    ca(ADDRESS_OF(CDUZD));
    msu_at(ADDRESS_OF(DELCDUZ));
    one_stotwos(PAXIS + 034u);
    ts(ADDRESS_OF(CDUZD));
    dim(ADDRESS_OF(TCP)); /* "DIMINISH MANUAL CONTROL DIRECT RATE" */
    dim(ADDRESS_OF(TCQR));

    set_A(CALLGMBL); /* PAXFILT */
    mask_at(ADDRESS_OF(RCSFLAGS));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_to(PAXFILT + 3u, ACDTPC12);
        return;
    }
    paxfilt_after_acdt_c12();
}

/* PAXFILT +4 -- LM p. 1428: "SETTING UP THE SUPERJOB": the interrupted
 * program's A, L, B, Q and Z kept in DAPARUPT .. DAPZRUPT (DAPZRUPT
 * positive: the DAP's job in progress), and the interrupt's B and Z made
 * SUPERJOB's word and its address +1; then NOQBRSM +1: "RELINT (JUST IN
 * CASE) AND RESUME, IN THE" "FORM OF A JASK, AT SUPERJOB." */
void paxfilt_after_acdt_c12(void)
{
    dxch(ADDRESS_OF(ARUPT));
    dxch(ADDRESS_OF(DAPARUPT));
    set_A(SUPERJOB_WORD); /* CA SUPERJOB */
    xch(ADDRESS_OF(BRUPT));
    lxch(ADDRESS_OF(QRUPT));
    dxch(ADDRESS_OF(DAPBQRPT));
    set_A(SUPERADR);
    dxch(ADDRESS_OF(ZRUPT));
    dxch(ADDRESS_OF(DAPZRUPT));
    continue_at(NOQBRSM + 1u);
}

/* SUPERJOB -- LM p. 1428: TCF RATELOOP (in TJET_LAW), which returns to
 * BACKP. */
void superjob(void)
{
    continue_at(RATELOOP);
}

/* ---- The rate derivation -- LM pp. 1423-1427 ---- */

/* One axis' CDU-rate (OMEGAU over 1/40, from SUBDIVDE) less its rate
 * estimate, added to the axis' TRAPED (its angle error from the predicted
 * angle), pinned by OVERSUB; and the manual mode attitude error (DXERROR,
 * DYERROR or DZERROR, double) moved on by the CDU change, less the last
 * commanded rate's (PLAST, QLAST or RLAST) over 1/40. */
static void traped_and_error(unsigned omega, unsigned traped, word at, unsigned error,
                             unsigned last)
{
    su_at(omega);
    ads(traped);
    oversub(at);
    ts(traped);
    dca(ADDRESS_OF(DAPTEMP5));
    das(error);
    cs(last);
    mp(ONE_40);
    das(error);
}

/* BACKP -- LM p. 1423: "BEGINNING OF THE RATE DERIVATION", where RATELOOP
 * returns with the jets' effect: JETRATE (1JACC times the P jets' time,
 * DAPTEMP1), and each TRAPED less the change of rate the jets and the offset
 * acceleration (AOSQTERM, AOSRTERM) gave. The CDUs' changes since last time
 * (MSU: "SCALED AT PI"), through the gimbal-to-body matrix M11 .. M32, over
 * 1/40 (SUBDIVDE: "RETURNS WITH CDU-RATE AT PI/4"); each axis' TRAPED and
 * manual error from it. "TRAPEDS HAVE ALL BEEN COMPUTED". */
void backp(void)
{
    ca(ADDRESS_OF(DAPTEMP1));
    mp_at(ADDRESS_OF(1JACC));
    ts(ADDRESS_OF(JETRATE));

    ca(ADDRESS_OF(DAPTREG4)); /* "CDUX IS STORED HERE" */
    ts(L_ADDRESS);
    msu_at(ADDRESS_OF(OLDXFORP));
    lxch(ADDRESS_OF(OLDXFORP));
    ts(ADDRESS_OF(DAPTEMP1));
    set_A(ONE_40);
    ts(ADDRESS_OF(DAPTREG4));
    cs(ADDRESS_OF(JETRATE));
    mp(BIT14);
    ads(ADDRESS_OF(TRAPEDP));
    ca(ADDRESS_OF(JETRATEQ));
    ad_at(ADDRESS_OF(AOSQTERM));
    mp(MBIT14);
    ads(ADDRESS_OF(TRAPEDQ));
    ca(ADDRESS_OF(JETRATER));
    ad_at(ADDRESS_OF(AOSRTERM));
    mp(MBIT14);
    ads(ADDRESS_OF(TRAPEDR));

    ca(ADDRESS_OF(DAPTREG5)); /* "CDUY IS STORED HERE" */
    ts(L_ADDRESS);
    msu_at(ADDRESS_OF(OLDYFORP));
    lxch(ADDRESS_OF(OLDYFORP));
    ts(ADDRESS_OF(DAPTEMP2));
    mp_at(ADDRESS_OF(M11)); /* "M11 SCALED AT 1" */
    ad_at(ADDRESS_OF(DAPTEMP1));
    dxch(ADDRESS_OF(OMEGAU));
    subdivde(BACKP + 044u, true);
    traped_and_error(ADDRESS_OF(OMEGAP), ADDRESS_OF(TRAPEDP), BACKP + 050u, ADDRESS_OF(DXERROR),
                     ADDRESS_OF(PLAST));

    ca(ADDRESS_OF(DAPTREG6)); /* "CDUZ IS STORED HERE" */
    ts(L_ADDRESS);
    msu_at(ADDRESS_OF(OLDZFORQ));
    ts(ADDRESS_OF(DAPTEMP3));
    lxch(ADDRESS_OF(OLDZFORQ));
    ca(ADDRESS_OF(M21));
    mp_at(ADDRESS_OF(DAPTEMP2));
    dxch(ADDRESS_OF(OMEGAU));
    ca(ADDRESS_OF(M22));
    subdivde(BACKP + 074u, false);
    traped_and_error(ADDRESS_OF(OMEGAQ), ADDRESS_OF(TRAPEDQ), BACKP + 0100u, ADDRESS_OF(DYERROR),
                     ADDRESS_OF(QLAST));

    ca(ADDRESS_OF(M31));
    mp_at(ADDRESS_OF(DAPTEMP2));
    dxch(ADDRESS_OF(OMEGAU));
    ca(ADDRESS_OF(M32));
    subdivde(BACKP + 0116u, false);
    traped_and_error(ADDRESS_OF(OMEGAR), ADDRESS_OF(TRAPEDR), BACKP + 0122u, ADDRESS_OF(DZERROR),
                     ADDRESS_OF(RLAST));
    continue_at(BACKP + 0133u);
}

/* The trap filter for the P axis's TRAPEDP: past the trap size (DAPTREG6:
 * "TRAPSIZE > ABOUT 77001"), the rate estimate takes it, over the times it
 * has been accumulated (NPTRAPS), and the count starts again from
 * DAPTREG4 ("ABOUT 10 OR 0 FOR DOCKED OR UNDOCKED"). Returns whether
 * TRAPEDP was large. */
static bool p_trap(void)
{
    switch (ccs_at(ADDRESS_OF(TRAPEDP))) {
    case POSITIVE: break;            /* TCF +2 */
    case IS_PLUS_ZERO: return false; /* TCF SMALPDIF */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: return false; /* EXTEND; BZMF SMALPDIF, A +0 */
    }
    ad_at(ADDRESS_OF(DAPTREG6));
    if (A_is_zero_or_minus()) return false;
    zl();
    lxch(ADDRESS_OF(TRAPEDP));
    set_A(ZERO);
    dv_at(ADDRESS_OF(NPTRAPS));
    ads(ADDRESS_OF(OMEGAP));
    oversub(LMONLY + 021u);
    ts(ADDRESS_OF(OMEGAP));
    ca(ADDRESS_OF(DAPTREG4));
    ts(ADDRESS_OF(NPTRAPS));
    return true;
}

/* The trap filter for the Q or R axis, as for P, and the offset
 * acceleration estimate AOSQ or AOSR moved by the rate's change times FIVE
 * over the old count plus DAPTREG5 ("KAOS > ABOUT 60D"). The rate's change
 * is kept in temp ("SAVE FOR OFFSET ESTIMATE"); at is the OVERSUB's TC. */
static void qr_trap(unsigned traped, unsigned traps, unsigned temp, unsigned omega, word at,
                    unsigned aos)
{
    switch (ccs_at(traped)) {
    case POSITIVE: break; /* TCF +2 */
    case IS_PLUS_ZERO: return;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: return; /* EXTEND; BZMF, A +0 */
    }
    ad_at(ADDRESS_OF(DAPTREG6));
    if (A_is_zero_or_minus()) return;
    zl();
    lxch(traped);
    set_A(ZERO);
    dv_at(traps);
    ts(temp);
    ads(omega);
    oversub(at);
    ts(omega);
    ca(ADDRESS_OF(DAPTREG4));
    xch(traps);
    ad_at(ADDRESS_OF(DAPTREG5));
    xch(temp);
    mp(FIVE);
    dv_at(temp);
    ads(aos);
}

/* BACKP +133 -- LM p. 1425: "PICK UP PAD LOADED STATE ESTIMATOR GAINS"
 * (DKOMEGAN and DKTRAP docked, LMOMEGAN and LMTRAP not: the counts' restart
 * in DAPTREG4, DAPTREG5, the trap size in DAPTREG6); the P axis' trap filter
 * (SMALPDIF: one more accumulation), then P-RATE: the P rate estimate moved
 * by the jets' JETRATE; and the Q axis' trap filter. */
void backp_plus0133(void)
{
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (A_is_zero()) { /* EXTEND; BZF LMONLY: "UNDOCKED" */
        dca(ADDRESS_OF(LMOMEGAN));
        dxch(ADDRESS_OF(DAPTREG4));
        ca(ADDRESS_OF(LMTRAP));
    } else { /* "DOCKED" */
        dca(ADDRESS_OF(DKOMEGAN));
        dxch(ADDRESS_OF(DAPTREG4));
        ca(ADDRESS_OF(DKTRAP));
    }
    ts(ADDRESS_OF(DAPTREG6));
    p_trap();
    incr(ADDRESS_OF(NPTRAPS)); /* SMALPDIF */

    ca(ADDRESS_OF(JETRATE)); /* P-RATE */
    ads(ADDRESS_OF(OMEGAP));
    oversub(P_RATE + 2u);
    ts(ADDRESS_OF(OMEGAP));
    qr_trap(ADDRESS_OF(TRAPEDQ), ADDRESS_OF(NQTRAPS), ADDRESS_OF(DAPTEMP1), ADDRESS_OF(OMEGAQ),
            P_RATE + 021u, ADDRESS_OF(AOSQ));
    continue_at(Q_RATE);
}

/* Q-RATE -- LM p. 1426: the Q rate estimate moved by the jets' JETRATEQ and
 * the offset acceleration's AOSQTERM; the R axis' trap filter. */
void q_rate(void)
{
    incr(ADDRESS_OF(NQTRAPS));
    ca(ADDRESS_OF(JETRATEQ));
    ad_at(ADDRESS_OF(AOSQTERM));
    ads(ADDRESS_OF(OMEGAQ));
    oversub(Q_RATE + 4u);
    ts(ADDRESS_OF(OMEGAQ));
    qr_trap(ADDRESS_OF(TRAPEDR), ADDRESS_OF(NRTRAPS), ADDRESS_OF(DAPTEMP2), ADDRESS_OF(OMEGAR),
            Q_RATE + 023u, ADDRESS_OF(AOSR));
    continue_at(R_RATE);
}

/* R-RATE -- LM p. 1427: the R rate estimate likewise. "END OF RATE
 * DERIVATION"; then the offset estimator: not drifting (DRIFTBIT of DAPBOOLS
 * clear), "ZERO THE OFFSET ACCELERATION VALUES."; drifting, WORKTIME: AOSQ
 * and AOSR moved on by the trim gimbal's acceleration change (QACCDOT,
 * RACCDOT) times CALLCODE ("OCTAL 00032 IS DECIMAL .1 AT 2(6)."), and their
 * 200MS terms for the next pass. Then PRETIMCK. */
void r_rate(void)
{
    incr(ADDRESS_OF(NRTRAPS));
    ca(ADDRESS_OF(JETRATER));
    ad_at(ADDRESS_OF(AOSRTERM));
    ads(ADDRESS_OF(OMEGAR));
    oversub(R_RATE + 4u);
    ts(ADDRESS_OF(OMEGAR));

    cs(ADDRESS_OF(DAPBOOLS));
    mask(DRIFTBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* TCF WORKTIME */
        ca(ADDRESS_OF(QACCDOT));
        mp(CALLCODE);
        das(ADDRESS_OF(AOSQ));
        ca(ADDRESS_OF(AOSQ));
        ts(ADDRESS_OF(ALPHAQ));
        mp(K200MS); /* ".2 AT 1" */
        ts(ADDRESS_OF(AOSQTERM));
        ca(ADDRESS_OF(RACCDOT));
        mp(CALLCODE);
        das(ADDRESS_OF(AOSR));
        ca(ADDRESS_OF(AOSR));
        ts(ADDRESS_OF(ALPHAR));
        mp(K200MS);
        ts(ADDRESS_OF(AOSRTERM));
    } else {
        ts(ADDRESS_OF(ALPHAQ));
        ts(ADDRESS_OF(ALPHAR));
        ts(ADDRESS_OF(AOSQTERM));
        ts(ADDRESS_OF(AOSRTERM));
        ts(ADDRESS_OF(AOSQ));
        ts(ADDRESS_OF(AOSR));
    }
    continue_at(PRETIMCK);
}

/* ---- The gimbal timers and the translation -- LM pp. 1428-1430 ---- */

static void qraxis(void);

/* TURNOFFQ -- LM p. 1428, TURNOFFR: "HALT DRIVES." (the axis' NEGU and
 * ACCDOT +0, its drive bits of channel 12 cleared, and its timer NEGMAX:
 * inactive). */
static void turnoff(unsigned negu, unsigned accdot, word bits, unsigned timer)
{
    ts(negu);
    ts(accdot);
    set_A(negate(bits));
    wand_channel(CHAN12);
    set_A(NEGMAX);
    ts(timer);
}

/* CHKRTIMR -- LM p. 1428: the R timer as PRETIMCK the Q, then the first DIM
 * of CHKRTIMR +3. A -0 timer enters that DIM's word without its EXTEND: ADS
 * PJETCTR, of A +0. */
static void chkrtimr(void)
{
    switch (ccs_at(ADDRESS_OF(RGIMTIMR))) {
    case POSITIVE: ts(ADDRESS_OF(RGIMTIMR)); break; /* DECRTIMR */
    case IS_PLUS_ZERO:
        turnoff(ADDRESS_OF(NEGUR), ADDRESS_OF(RACCDOT), RGIMBITS, ADDRESS_OF(RGIMTIMR));
        break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: ads(ADDRESS_OF(PJETCTR)); return;
    }
    dim(ADDRESS_OF(PJETCTR));
}

/* PRETIMCK -- LM p. 1428: "COUNT DOWN GIMBAL DRIVE TIMERS AND TURN OFF
 * DRIVES IF REQUIRED." (positive: "COUNTING DOWN", +0: "DRIVE SHOULD BE
 * ENDED", negative: "INACTIVE"), then "DECREMENT DOCKED JET INHIBITION
 * COUNTERS". With bit 12 of RCSFLAGS clear, SKIPPAXS: it is set ("BIT 12 SET
 * TO 1.") and the P axis skipped this pass, to QRAXIS; set, TC CHKVISFZ. A
 * -0 Q timer enters the word past the CCS's last exit, CHKRTIMR's TCF
 * DECRTIMR. */
void pretimck(void)
{
    switch (ccs_at(ADDRESS_OF(QGIMTIMR))) {
    case POSITIVE: /* DECQTIMR */
        ts(ADDRESS_OF(QGIMTIMR));
        chkrtimr();
        break;
    case IS_PLUS_ZERO:
        turnoff(ADDRESS_OF(NEGUQ), ADDRESS_OF(QACCDOT), QGIMBITS, ADDRESS_OF(QGIMTIMR));
        chkrtimr();
        break;
    case NEGATIVE: chkrtimr(); break;
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(RGIMTIMR));
        dim(ADDRESS_OF(PJETCTR));
        break;
    }
    dim(ADDRESS_OF(UJETCTR));
    dim(ADDRESS_OF(VJETCTR));
    set_A(BIT12);
    mask_at(ADDRESS_OF(RCSFLAGS));
    if (A_is_zero()) { /* SKIPPAXS */
        cs(ADDRESS_OF(RCSFLAGS));
        mask(BIT12);
        ads(ADDRESS_OF(RCSFLAGS));
        qraxis(); /* "GO TO QRAXIS OR TO CTS." */
        return;
    }
    tc_to(CHKRTIMR + 015u, CHKVISFZ);
}

/* ---- The jet policies -- LM p. 1440 ---- */

/* ABORTYZ +2 -- LM p. 1430, to its TCF TSNEXTP: "INVERT BIT 1 OF RCSFLAGS."
 * (TS RCSFLAGS skips the CA ZERO on an overflow). */
static void abortyz_flip(void)
{
    set_A(BIT1);
    lxch(ADDRESS_OF(RCSFLAGS));
    rxor_lchan();
    if (!ts(ADDRESS_OF(RCSFLAGS))) set_A(ZERO);
}

static void jetsoff(void);

/* SELECTP -- LM p. 1440 (the jet type, NUMBERT, from A by SELECTYZ), by TC
 * at at: the "JET POLICY CONSTRUCTION SUBROUTINE": TYPEP's word for NUMBERT
 * (the pairs, or all four) masked by JETSALL's for ROTINDEX (the rotation or
 * translation), kept in POLYTEMP; with no jet of it failed (CH6MASK),
 * RETURN. Else the next type down ("THIS SUBROUTINE SELECT A SUBSET OF THE
 * DESIRED JETS WHICH HAS NO FAILURE"), by SELECTYZ; past the last, ALARM
 * 2003 (and SELECTP +17's TCF JETSOFF). A ROTINDEX of 7776 (INDXYZ's for a
 * principal translation: "THESE INDEXES OF MASK JETSALL WILL" "CHANGE THE
 * INSTRUCTION AT SELECTP +4" "TO BE TC JETSALL -1") turns the MASK into a TC
 * to TCF ABORTYZ +2, which inverts RCSFLAGS' bit 1 and goes to TSNEXTP.
 * Returns whether it returned. */
static bool selectp(word at, bool selectyz)
{
    tc_at(at);
    if (!selectyz) set_A(SIX);
    for (;;) {
        if (selectyz) ts(ADDRESS_OF(NUMBERT)); /* SELECTYZ */
        ts(ADDRESS_OF(TEMPNUM));
        ca(indexed(TYPEP, fetch(ADDRESS_OF(NUMBERT))));
        if (fetch(ADDRESS_OF(ROTINDEX)) == 07776u) {
            tc_at(SELECTP + 5u); /* INDEX ROTINDEX; MASK JETSALL, made TC JETSALL -1 */
            abortyz_flip();
            continue_at(TSNEXTP);
            return false;
        }
        mask_at(indexed(JETSALL, fetch(ADDRESS_OF(ROTINDEX))));
        ts(ADDRESS_OF(POLYTEMP));
        mask_at(ADDRESS_OF(CH6MASK));
        if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) {
            tc_q();
            return true;
        }
        switch (ccs_at(ADDRESS_OF(TEMPNUM))) {
        case POSITIVE: break; /* TCF +4: SELECTYZ */
        case IS_PLUS_ZERO:
            tc_to(SELECTP + 015u, ALARM); /* OCT 02003 */
            return false;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* TEMPNUM counts down from SIX */
        }
        selectyz = true;
    }
}

/* SELECTP +17 -- LM p. 1440, where ALARM returns: "TCF ALARMJET". */
void selectp_after_alarm(void)
{
    jetsoff();
}

/* ---- The Y-Z translation -- LM pp. 1429-1430 ---- */

/* CHKVISFZ -- LM p. 1429: "Y-X TRANSLATION" from the translation controller
 * ("BITS 9-12 OF CH31", read inverted): none, TSNEXTP with A +0; else its
 * INDXYZ word to ROTINDEX, and TRYUORV: the jets for it (SELECTYZ from SIX).
 * A policy of four jets (NUMBERT still SIX), or a diagonal translation
 * (ROTINDEX past FIVE) on its pair, TSNEXTP -1 (POLYTEMP to NEXTP). A
 * principal one with a pair failed tries the other quads' pair (ALTERYZ:
 * "INVERT BIT 1 OF RCSFLAGS.", ROTINDEX moved on by FOUR or FIVE), and with
 * none left, ABORTYZ: ALARM 2001. */
void chkvisfz(void)
{
    read_channel(CHAN31);
    cs(A_ADDRESS);
    mask(OCT_07400);
    if (A_is_zero()) { /* EXTEND; BZF TSNEXTP */
        continue_at(TSNEXTP);
        return;
    }
    mp(BIT7);
    ca(indexed(INDXYZ, a_word()));
    ts(ADDRESS_OF(ROTINDEX));
    for (;;) { /* TRYUORV */
        set_A(SIX);
        if (!selectp(TRYUORV + 1u, true)) return;
        set_A(negate(SIX));
        ad_at(ADDRESS_OF(NUMBERT));
        if (A_is_zero()) {
            continue_at(TSNEXTP - 1u);
            return;
        }
        set_A(negate(FIVE));
        ad_at(ADDRESS_OF(ROTINDEX));
        if (!A_is_zero_or_minus()) {
            cs(ADDRESS_OF(NUMBERT));
            ad(FOUR);
            if (A_is_zero_or_minus()) {
                continue_at(TSNEXTP - 1u);
                return;
            }
            tc_to(ABORTYZ, ALARM); /* OCT 02001 */
            return;
        }
        set_A(BIT1); /* ALTERYZ */
        lxch(ADDRESS_OF(RCSFLAGS));
        rxor_lchan();
        if (!ts(ADDRESS_OF(RCSFLAGS))) mask(BIT1);
        ad(FOUR);
        ads(ADDRESS_OF(ROTINDEX));
    }
}

/* ---- The state logic and the minimum impulse mode -- LM pp. 1430-1431 ---- */

/* MANMODE -- LM p. 1430: in the pulse mode (PULSES of DAPBOOLS: "PULSES IS
 * ONE FOR PULSE MODE"), the "MINIMUM IMPULSE MODE": PERROR +0, CDUXD the
 * CDU. With OLDPMIN positive (a pulse given), CHECKP: OLDPMIN from the
 * controller's P bits (3 and 4 of channel 31, inverted) and the jets off
 * until it is back in detent; else FIREP: a +P or -P command fires TEN for
 * TJP ("+XMIN" or "-XMIN": "ANYTHING LESS THAN 14MS. CORRECTED" "IN JET
 * SELECTION ROUTINE"), OLDPMIN ONE, to PJETSLEC -6; none, JETSOFF. Not in
 * the pulse mode, DETENTCK. (A negative OLDPMIN enters FIREP at its RAND's
 * EXTEND; the faithful port takes no -0 OLDPMIN.) */
static void manmode(void)
{
    set_A(PULSES);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (A_is_zero()) { /* "BRANCH FOR RATE COMMAND" */
        continue_at(DETENTCK);
        return;
    }
    set_A(ZERO);
    ts(ADDRESS_OF(PERROR));
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUXD));
    switch (ccs_at(ADDRESS_OF(OLDPMIN))) {
    case POSITIVE: /* TCF CHECKP */
        read_channel(CHAN31);
        cs(A_ADDRESS);
        mask(OCT14);
        ts(ADDRESS_OF(OLDPMIN));
        jetsoff();
        return;
    case IS_PLUS_ZERO: set_A(BIT3); break; /* FIREP */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    rand_channel(CHAN31);
    if (A_is_zero()) { /* +XMIN */
        set_A(TEN);
    } else {
        set_A(BIT4);
        rand_channel(CHAN31);
        if (!A_is_zero()) {
            jetsoff();
            return;
        }
        set_A(negate(TEN)); /* -XMIN */
    }
    ts(ADDRESS_OF(TJP));
    set_A(ONE);
    ts(ADDRESS_OF(OLDPMIN));
    continue_at(PJETSLEC - 6u);
}

/* TSNEXTP -1 -- LM p. 1430: the translation's jets, POLYTEMP, to NEXTP. */
void tsnextp_minus1(void)
{
    ca(ADDRESS_OF(POLYTEMP));
    tsnextp();
}

/* ABORTYZ +2 -- LM p. 1430 (JETSALL -1's TCF): RCSFLAGS' bit 1 inverted, and
 * TSNEXTP. */
void abortyz_plus2(void)
{
    abortyz_flip();
    tsnextp();
}

/* TSNEXTP -- LM p. 1430: NEXTP, "THE CHANNEL 6 CODE OF JETS FOR THE DESIRED
 * TRANSLATION.", from A. Then the "STATE LOGIC": in attitude hold (bit 13
 * of channel 31 clear, inverted: "CHECK STICK IF IN ATT. HOLD."), MANMODE;
 * else, XOVINHIB of DAPBOOLS set, PURGENCY ("ATTITUDE STEER DURING
 * VISIBILITY PHASE"), clear, DETENTCK. */
void tsnextp(void)
{
    ts(ADDRESS_OF(NEXTP));
    set_A(BIT13);
    rand_channel(CHAN31);
    if (A_is_zero()) {
        manmode();
        return;
    }
    ca(ADDRESS_OF(DAPBOOLS));
    mask(XOVINHIB);
    continue_at(ccs_register(A_ADDRESS) == POSITIVE ? PURGENCY : DETENTCK);
}

/* ---- The manual rate command mode -- LM pp. 1431-1435 ---- */

/* ZEROENBL -- LM p. 1434, by TC at at: "ENABLES COUNTERS SO THEY CAN BE READ
 * NEXT TIME": the Q and R counts kept in SAVEHAND, all three counters +0,
 * and bits 8 and 9 of channel 13 set: "COUNTERS ZEROED AND ENABLED". */
static void zeroenbl(word at)
{
    tc_at(at);
    lxch(ADDRESS_OF(R_RHCCTR));
    ca(ADDRESS_OF(Q_RHCCTR));
    dxch(ADDRESS_OF(SAVEHAND));
    set_A(ZERO);
    ts(ADDRESS_OF(P_RHCCTR));
    ts(ADDRESS_OF(Q_RHCCTR));
    ts(ADDRESS_OF(R_RHCCTR));
    set_A(BITS8_9);
    wor_channel(CHAN13);
    tc_q();
}

/* JUSTOUT -1 -- LM p. 1433 (PERROR from A, +0), JUSTOUT: "FIRST DETECTION OF
 * OUT OF DETENT (BY OURRCBIT)": "INITIALIZATION -- FIRST MANUAL PASS."
 * (OURRCBIT set, the manual errors, the last commands and the Q and R
 * counters +0, "BITS 10 AND 11 OF RCSFLAGS ARE 0.", "SET 'JUST-IN' BIT TO
 * 1."), ZEROENBL, JETSOFF. */
static void justout_minus1(void)
{
    static const unsigned zeroed[] = {
        ADDRESS_OF(DXERROR),      ADDRESS_OF(DXERROR) + 1u, ADDRESS_OF(DYERROR),
        ADDRESS_OF(DYERROR) + 1u, ADDRESS_OF(DZERROR),      ADDRESS_OF(DZERROR) + 1u,
        ADDRESS_OF(PLAST),        ADDRESS_OF(QLAST),        ADDRESS_OF(RLAST),
        ADDRESS_OF(Q_RHCCTR),     ADDRESS_OF(R_RHCCTR),
    };
    unsigned i;

    ts(ADDRESS_OF(PERROR));
    set_A(OURRCBIT);
    ads(ADDRESS_OF(DAPBOOLS));
    set_A(ZERO);
    for (i = 0; i < sizeof zeroed / sizeof zeroed[0]; ++i) ts(zeroed[i]);
    set_A(PQRBIT);
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT9);
    ads(ADDRESS_OF(RCSFLAGS));
    zeroenbl(JUSTOUT + 024u);
    jetsoff();
}

/* RATEDAMP -- LM p. 1434: the P counter +0, and RATERROR. */
static void ratedamp(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(P_RHCCTR));
    continue_at(RATERROR);
}

/* DETENTCK -- LM p. 1432: the rotation hand controller. "OUT-OF-DETENT" (bit
 * 15 of channel 31 clear), RHCMOVED: the first pass out (OURRCBIT clear),
 * JUSTOUT -1; else RATERROR. In detent: not in rate command last pass,
 * PURGENCY; "JUST IN DETENT??" (bit 9 of RCSFLAGS clear), RUTH: rate damping
 * not yet done in P (PBIT) or Q, R (QRBIT), RATEDAMP; done, RATEDONE
 * ("MANUAL COMMAND AND DAMPING COMPLETED IN" "ALL AXES."). Just in detent,
 * in auto ("(X-AXIS OVERRIDE)"), "ZERO ORBIT (BIT 11) AND JUST-IN BIT (9).";
 * and RATEDAMP. */
void detentck(void)
{
    read_channel(CHAN31);
    ts(ADDRESS_OF(CH31TEMP));
    mask(BIT15);       /* "CHECK OUT-OF-DETENT BIT." */
    if (A_is_zero()) { /* RHCMOVED: "P CONTROL" */
        set_A(OURRCBIT);
        mask_at(ADDRESS_OF(DAPBOOLS));
        if (A_is_zero()) {
            justout_minus1();
            return;
        }
        continue_at(RATERROR);
        return;
    }
    set_A(OURRCBIT);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (A_is_zero()) {
        continue_at(PURGENCY);
        return;
    }
    set_A(BIT9);
    mask_at(ADDRESS_OF(RCSFLAGS));
    if (A_is_zero()) { /* RUTH */
        ca(ADDRESS_OF(RCSFLAGS));
        mask(PBIT);
        if (A_is_zero()) { /* "BRANCH IF P-RATE DAMPING IS FINISHED." */
            ca(ADDRESS_OF(RCSFLAGS));
            mask(QRBIT);
            if (A_is_zero()) { /* RATEDONE */
                set_A(negate(OURRCBIT));
                inhint();
                mask_at(ADDRESS_OF(DAPBOOLS));
                ts(ADDRESS_OF(DAPBOOLS));
                set_A(BIT13);
                rand_channel(CHAN31);
                if (A_is_zero()) { /* to ZATTEROR, which returns to RATEDONE +16 */
                    tc_to(RATEDONE + 014u, IBNKCALL);
                    return;
                }
                ca(ADDRESS_OF(CDUX));
                ts(ADDRESS_OF(CDUXD));
                tc_at(RATEDONE + 013u); /* TC +3 */
                relint();
                continue_at(PURGENCY);
                return;
            }
        }
        ratedamp();
        return;
    }
    set_A(BIT13);
    rand_channel(CHAN31);
    if (!A_is_zero()) {
        set_A(negate(BITS9_11));
        mask_at(ADDRESS_OF(RCSFLAGS));
        ts(ADDRESS_OF(RCSFLAGS));
    }
    ratedamp();
}

/* RATEDONE +16 -- LM p. 1433, where ZATTEROR returns (the attitude errors
 * zeroed): RELINT, PURGENCY. */
void ratedone_after_zatteror(void)
{
    relint();
    purgency();
}

/* PEGI -- LM p. 1435, by TC at at: "DIRECT RATE CONTROL.": the desired CDU
 * the CDU, the X error +0; RCSFLAGS' PBIT set while the rate error (EDOTP)
 * is past the target deadband and TCP has time left (four seconds), cleared
 * otherwise ("FIRE, AN SWITCH TO PSEUDO-AUTO."). Then the jet time for the
 * rate error: TJP, -EDOTP times 1/ANETP doubled, pinned by OVERSUB, times
 * 25/32 ("A CONTAINS TJET SCALED AT 2EXP(4)(16/25)"); past the two jet
 * limit, SIX (four jets) to PJETSLEC -1, else twice the time for two jets to
 * PJETSLEC -6. */
static void pegi(word at)
{
    tc_at(at);
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUXD));
    set_A(ZERO);
    ts(ADDRESS_OF(DXERROR));
    ts(ADDRESS_OF(DXERROR) + 1u);
    ts(ADDRESS_OF(PERROR));              /* "ZERO P-AXIS ERROR FOR MODE1 FDAI DISPLAY" */
    switch (ccs_at(ADDRESS_OF(EDOTP))) { /* each TC lands at the TS ABSEDOTP */
    case POSITIVE: tc_at(PEGI + 7u); break;
    case IS_PLUS_ZERO: tc_at(PEGI + 010u); break;
    case NEGATIVE: tc_at(PEGI + 011u); break;
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(ABSEDOTP));
    ad_at(ADDRESS_OF(TARGETDB));
    bool last = A_is_zero_or_minus();
    if (!last) {
        ca(ADDRESS_OF(TCP));
        last = A_is_zero_or_minus();
    }
    if (last) { /* LAST */
        set_A(negate(PBIT));
        mask_at(ADDRESS_OF(RCSFLAGS));
        ts(ADDRESS_OF(RCSFLAGS)); /* "BIT 10 IS 0." */
    } else {
        cs(ADDRESS_OF(RCSFLAGS));
        mask(PBIT);
        ads(ADDRESS_OF(RCSFLAGS)); /* "BIT 10 IS 1." */
    }
    cs(ADDRESS_OF(EDOTP));
    mp_at(ADDRESS_OF(1_ANETP));
    ddoubl();
    oversub(LAST + 7u);
    mp(K25_32);
    ts(ADDRESS_OF(TJP)); /* "4.JET TIME" */
    ca(ADDRESS_OF(ABSEDOTP));
    ad_at(ADDRESS_OF(M2JETLIM)); /* "COMPARING DELTA RATE WITH 2 JET LIMIT" */
    if (!A_is_zero_or_minus()) {
        set_A(SIX);
        continue_at(PJETSLEC - 1u);
        return;
    }
    ca(ADDRESS_OF(TJP));
    ads(ADDRESS_OF(TJP));
    continue_at(PJETSLEC - 6u);
}

/* RATERROR -- LM p. 1434: the P command from the hand controller's count
 * (P-RHCCTR), "LINEAR/QUADRATIC CONTROLLER SCALING" (the count times itself
 * times 4 plus LINRATP, times STIKSENS), into PLAST; its change from the
 * last (DAPTEMP1), and ZEROENBL (unless that overflowed). EDOTP the rate
 * error. "IF P COMMAND CHANGE EXCEEDS BREAKOUT" "LEVEL, GO TO DIRECT RATE
 * CONTROL." (PEGI, TCP 40CYC: four seconds); "IF NOT" "CHECK FOR DIRECT RATE
 * CONTROL LAST TIME." (PBIT of RCSFLAGS), still PEGI; else "PSEUDO-AUTO
 * CONTROL.": the X error to E and PERROR, and PURGENCY +4. */
void raterror(void)
{
    bool breakout = false;

    ca(ADDRESS_OF(CDUX)); /* "FINDCDUW REQUIRES THAT CDUXD=CDUX DURING" */
    ts(ADDRESS_OF(CDUXD));
    (void)ccs_at(ADDRESS_OF(P_RHCCTR)); /* each exit to the DOUBLE */
    double_A();
    double_A();
    ad(LINRATP);
    mp_at(ADDRESS_OF(P_RHCCTR));
    ca(L_ADDRESS);
    mp_at(ADDRESS_OF(STIKSENS));
    xch(ADDRESS_OF(PLAST));
    com();
    ad_at(ADDRESS_OF(PLAST));
    if (!ts(ADDRESS_OF(DAPTEMP1))) zeroenbl(RATERROR + 022u);
    cs(ADDRESS_OF(PLAST));
    ad_at(ADDRESS_OF(OMEGAP));
    if (ts(ADDRESS_OF(EDOTP))) {
        breakout = true; /* the skip lands at TCF +3 */
    } else {
        switch (ccs_at(ADDRESS_OF(DAPTEMP1))) {
        case POSITIVE: breakout = true; break;
        case IS_PLUS_ZERO: break; /* TCF +8D */
        case NEGATIVE:
        case IS_MINUS_ZERO: breakout = true; break;
        }
    }
    if (breakout) {
        ad_at(ADDRESS_OF(MRATEDB));
        if (!A_is_zero_or_minus()) {
            set_A(K40CYC);
            ts(ADDRESS_OF(TCP));
            pegi(RATERROR + 037u);
            return;
        }
    }
    ca(ADDRESS_OF(RCSFLAGS));
    mask(PBIT);
    if (!A_is_zero()) {
        pegi(RATERROR + 044u); /* "TO PURE RATE COMMAND" */
        return;
    }
    ca(ADDRESS_OF(DXERROR));
    ts(ADDRESS_OF(E)); /* "X-ATTITUDE ERROR (SP)" */
    ts(ADDRESS_OF(PERROR));
    tc_to(RATERROR + 050u, PURGENCY + 4u);
}

/* ---- The P jet selection -- LM pp. 1436-1437 ---- */

/* JETSOFF -- LM p. 1437: TC WRITEP -1 (P's jets off), which returns to
 * JETSOFF +1. */
static void jetsoff(void)
{
    tc_to(JETSOFF, WRITEP - 1u);
}

/* PJETSLEC -1 -- LM p. 1436: "P-JET-SELECTION-ROUTINE (ROTATION)", NUMBERT
 * from A: none for a TJP of zero (JETSOFF); else ABSTJ its magnitude and
 * ROTINDEX 1 for +P, 0 for -P, the policy by SELECTP, the jets' count
 * (NO.PJETS: FOUR, or TWO but for type SIX), and the policy to TC WRITEP. */
void pjetslec_minus1(void)
{
    ts(ADDRESS_OF(NUMBERT));
    set_A(ONE); /* PJETSLEC */
    ts(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(TJP))) {
    case POSITIVE: break; /* TCF +5 */
    case IS_PLUS_ZERO: jetsoff(); return;
    case NEGATIVE: zl(); break;
    case IS_MINUS_ZERO: jetsoff(); return;
    }
    ad(ONE);
    ts(ADDRESS_OF(ABSTJ));
    lxch(ADDRESS_OF(ROTINDEX));
    if (!selectp(PJETSLEC + 013u, false)) return;
    set_A(negate(SIX));
    ad_at(ADDRESS_OF(NUMBERT));
    if (!A_is_zero()) set_A(negate(TWO));
    ad(FOUR);
    ts(ADDRESS_OF(NO_PJETS));
    ca(ADDRESS_OF(POLYTEMP));
    tc_to(PJETSLEC + 024u, WRITEP);
}

/* PJETSLEC -6 -- LM p. 1436: "GOES TO PJETSLEC FOR TWO JETS": NUMBERT FOUR,
 * or FIVE with AORBSYST of DAPBOOLS set (the other system's pair). */
void pjetslec_minus6(void)
{
    set_A(AORBSYST);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(ONE);
    ad(FOUR);
    pjetslec_minus1();
}

/* PJETSLEC +25 -- LM p. 1437, where WRITEP returns: a firing shorter than
 * 150 ms ends by T6 this pass, so to QRAXIS ("GO TO QRAXIS OR TO GTS.");
 * else ABSTJ kept the time over 136 ms, or made up to the minimum, MINTIMES'
 * word for ROTINDEX (TJP), and the rest timed by T6 (T6FURTHA) and JTLST. */
void pjetslec_after_writep(void)
{
    cs(ADDRESS_OF(ABSTJ));
    ad(PLUS150MST6);
    if (A_is_zero_or_minus()) {
        qraxis();
        return;
    }
    ad(M136MST6);
    if (!A_is_zero_or_minus()) {
        ads(ADDRESS_OF(ABSTJ));
        ca(indexed(MINTIMES, fetch(ADDRESS_OF(ROTINDEX))));
        ts(ADDRESS_OF(TJP));
    }
    ca(ADDRESS_OF(ABSTJ));
    zl();
    inhint();
    dxch(ADDRESS_OF(T6FURTHA));
    tc_to(PJETSLEC + 044u, IBNKCALL); /* CADR JTLST */
}

/* ALTSYST -- LM p. 1437, by TC at at: "ALTERNATE P-AXIS JETS" (AORBSYST of
 * DAPBOOLS inverted); RELINT. */
static void altsyst(word at)
{
    tc_at(at);
    ca(ADDRESS_OF(DAPBOOLS));
    ts(L_ADDRESS);
    set_A(AORBSYST);
    rxor_lchan();
    if (!ts(ADDRESS_OF(DAPBOOLS))) relint();
    tc_q();
}

/* PJETSLEC +46 -- LM p. 1437, where JTLST returns: "BIT 12 SET TO 0.",
 * ALTSYST, QRAXIS. */
void pjetslec_after_jtlst(void)
{
    set_A(negate(BIT12));
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    altsyst(PJETSLEC + 051u);
    qraxis();
}

/* JETSOFF +1 -- LM p. 1437, where WRITEP -1 returns: TJP +0, QRAXIS. */
void jetsoff_after_writep(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(TJP));
    qraxis();
}

/* ---- The P-axis error and urgency -- LM pp. 1437-1439 ---- */

/* CALCPERR -- LM p. 1437: "P-ERROR CALCULATION.": (CDUY less CDUYD) times
 * M11, plus CDUX less CDUXD ("M13 = 1 IDENTICALLY IMPLIES NULL
 * MULTIPLICATION."), plus DELPEROR ("KALCMANU INTERFACE ERROR."), in E and
 * PERROR ("SAVE P-ERROR FOR EIGHT-BALL DISPLAY."); RETURN. */
void calcperr(void)
{
    ca(ADDRESS_OF(CDUY));
    msu_at(ADDRESS_OF(CDUYD));
    mp_at(ADDRESS_OF(M11));
    xch(ADDRESS_OF(E)); /* "SAVE FIRST TERM (OF TWO)" */
    ca(ADDRESS_OF(CDUX));
    msu_at(ADDRESS_OF(CDUXD));
    ad_at(ADDRESS_OF(DELPEROR));
    ads(ADDRESS_OF(E));
    xch(ADDRESS_OF(PERROR));
    tc_q();
}

/* PURGENCY +4 -- LM p. 1438 on: AXISCTR from A (-1), and the "P-AXIS URGENCY
 * FUNCTION CALCULATION.": docked (CSMDOCKD), SPSRCS ("IF CSMDOCKD = 1, GOT
 * TO DOCKED RCS LOGIC"); else HEADTJET: SENSETYP +0, and TJETLAW. Both by
 * IBNKCALL, under INHINT. */
static void axisctr_and_urgency(void)
{
    ts(ADDRESS_OF(AXISCTR));
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (!A_is_zero()) {
        inhint();
        tc_to(PURGENCY + 013u, IBNKCALL); /* CADR SPSRCS */
        return;
    }
    set_A(ZERO); /* HEADTJET */
    ts(ADDRESS_OF(SENSETYP));
    inhint();
    tc_to(HEADTJET + 3u, IBNKCALL); /* CADR TJETLAW */
}

/* PURGENCY -- LM p. 1438: "CALCULATE P-AXIS ERRORS." (CALCPERR), and EDOTP
 * "= OMEGAP - OMEGAPD AT PI/4 RAD/SEC" ("THIS CODING IS COMMON TO BOTH LM
 * DAP AND" "SPS-BACKUP MODE."); its overflow skips the CS ONE. */
void purgency(void)
{
    tc_at(PURGENCY);
    calcperr();
    cs(ADDRESS_OF(OMEGAPD));
    ad_at(ADDRESS_OF(OMEGAP));
    if (ts(ADDRESS_OF(EDOTP))) {
        axisctr_and_urgency();
        return;
    }
    purgency_plus4();
}

/* PURGENCY +4 -- LM p. 1438: the pseudo-auto mode's way in, the error in
 * E. */
void purgency_plus4(void)
{
    set_A(negate(ONE));
    axisctr_and_urgency();
}

/* PURGENCY +15 -- LM p. 1438, where SPSRCS returns: with no P firing (TJP
 * +0), DKALT: ALTSYST ("IF TJP = ZERO, CHANGE AORBSYST.") and JETSOFF; else
 * RELINT and PJETSLEC -6 ("SELECT AORBSYST AND USE TWO JETS."). */
void purgency_after_spsrcs(void)
{
    ca(ADDRESS_OF(TJP));
    if (A_is_zero()) {
        altsyst(DKALT);
        jetsoff();
        return;
    }
    relint();
    pjetslec_minus6();
}

/* HEADTJET +5 -- LM p. 1438, where TJETLAW returns: RELINT. A small firing
 * factor (FIREFCT, past -FOURDEG), or a firing (TJP) under 160 ms, two jets
 * (PJETSLEC -6); a longer one, SIX: four jets (PJETSLEC -1); none, JETSOFF.
 * A -0 TJP enters the BZMF with A +0. */
void headtjet_after_tjetlaw(void)
{
    relint();
    cs(ADDRESS_OF(FIREFCT));
    ad(MFOURDEG);
    if (A_is_zero_or_minus()) {
        pjetslec_minus6();
        return;
    }
    switch (ccs_at(ADDRESS_OF(TJP))) {
    case POSITIVE: ad(M160MST6); break; /* TCF +2 */
    case IS_PLUS_ZERO: jetsoff(); return;
    case NEGATIVE: ad(M160MST6); break;
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero_or_minus()) {
        pjetslec_minus6();
        return;
    }
    set_A(SIX);
    pjetslec_minus1();
}

/* ---- QRAXIS -- LM p. 1441 ---- */

/* QRAXIS -- LM p. 1441: the R and Q rate errors (EDOTR, EDOTQ), each pinned
 * by OVERSUB; then "GOES TO Q,R-AXES RCS AUTOPILOT" by DTCB to CALLQERR
 * (bank 17). */
static void qraxis(void)
{
    cs(ADDRESS_OF(OMEGARD));
    ad_at(ADDRESS_OF(OMEGAR));
    oversub(QRAXIS + 2u);
    ts(ADDRESS_OF(EDOTR));
    cs(ADDRESS_OF(OMEGAQD));
    ad_at(ADDRESS_OF(OMEGAQ));
    oversub(QRAXIS + 6u);
    ts(ADDRESS_OF(EDOTQ));
    dca_constant(QERRCALL);
    dtcb(QRAXIS + 012u);
}

#endif
