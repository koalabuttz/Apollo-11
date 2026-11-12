/* SPDX-License-Identifier: MIT */
/* Q_R-AXIS_RCS_AUTOPILOT.agc, the LM's: Luminary099 pages 1442-1459, bank
 * 17, in EBANK 6 (the listing's EBANK= CDUXD); MAKERUPT in fixed-fixed. The
 * faithful C is c64-port/src/lm_q_r_axis.c. */
#include "q_r_axis.h"

#if LUMINARY099

/* RETURN: TC Q. */
static void tc_q(void)
{
    tc_through(Q_ADDRESS);
}

/* CCS of a word that is never negative here (a MASK of a positive
 * constant or of a channel bit, POLYTEMP, NUMBERT counting down): whether
 * it was positive (A then one less). Its negative and -0 exits are never
 * taken. */
static bool ccs_positive(unsigned address)
{
    switch (ccs_at(address)) {
    case POSITIVE: return true;
    case IS_PLUS_ZERO: return false;
    default: not_modelled(); return false;
    }
}

/* AXISCTR: "0,1 FOR U,V", what INDEX AXISCTR adds. */
static word axis(void)
{
    word count = fetch(ADDRESS_OF(AXISCTR));
    if (count > 1u) not_modelled();
    return count;
}

/* INDEX AXISCTR; INDEX SKIPU; TCF +1: SKIPU (or SKIPV) is 0, or 4 to skip
 * the axis: "FOR LESS THAN 150MS FIRING". Zero: CA FOUR; INDEX AXISCTR;
 * TS SKIPU; TCF LOOPER, and the axis is done for this pass. Returns
 * whether it was. */
static bool skipped(void)
{
    unsigned skip = indexed(ADDRESS_OF(SKIPU), axis());

    switch (fetch(skip)) {
    case 0u:
        set_A(FOUR);
        ts(skip);
        continue_at(LOOPER);
        return true;
    case 4u: return false; /* TCF +5, the word four past the TCF +1 */
    default: not_modelled(); return false;
    }
}

static void q_rorgts(void);
static void x_translation(void);
static bool selctsub(word at);
static void enteruv(void);
static void aftertj(void);
static void norotat(void);
static void feedback(void);

/* ---- CALLQERR -- LM p. 1442 ---- */

/* CALLQERR -- LM p. 1442: "CALCULATE Q,R ERRORS UNLESS THESE AXES" "ARE IN
 * MANUAL RATE COMMAND." (bit 13 of channel 31 is 1 in auto; in manual,
 * OURRCBIT of DAPBOOLS 0 is rate command: "IF SO BYPASS CALCULATION OF
 * ERRORS."). */
void callqerr(void)
{
    bool errors = true;

    set_A(BIT13);
    rand_channel(CHAN31);
    if (!ccs_positive(A_ADDRESS)) { /* else TCF +5: "IN AUTO COMPUTE Q,R ERRORS" */
        cs(ADDRESS_OF(DAPBOOLS));
        mask(OURRCBIT);
        errors = !A_is_zero();
    }
    if (errors) {
        tc_at(CALLQERR + 011u);
        qerrcalc();
    }
    q_rorgts();
}

/* Q,RORGTS -- LM p. 1442: "CHOOSE CONTROL SYSTEM FOR THIS DAP PASS:" by
 * COTROLER: positive the trim gimbal ("GTS (ALTERNATES WITH RCS WHEN
 * DOCKED)"), +0 TRYGTS ("GTS IF ALLOWED, OTHERWISE RCS"), negative RCS (-0:
 * RCS +1, A +0). */
static void q_rorgts(void)
{
    switch (ccs_at(ADDRESS_OF(COTROLER))) {
    case POSITIVE:
        set_L(GTSCADR[1]); /* GOTOGTS: EXTEND; DCA GTSCADR; DTCB */
        set_A(GTSCADR[0]);
        dtcb(GOTOGTS + 2u);
        return;
    case IS_PLUS_ZERO: continue_at(TRYGTS); return;
    case NEGATIVE: continue_at(RCS); return;
    case IS_MINUS_ZERO: continue_at(RCS + 1u); return;
    }
}

/* RCS -- LM p. 1442: COTROLER +0, then the rate differences EDOTQ, EDOTR
 * turned to the U,V axes, OMEGAU, OMEGAV. */
void rcs(void)
{
    set_A(ZERO);
    rcs_plus1();
}

/* RCS +1 -- LM p. 1442, then SENSEGET: "X - TRANSLATION". The hand
 * controller's bits 7 and 8 of channel 31 (0: +X or -X asked) "OVERRIDE THE
 * INTERNAL BITS" (ULLAGER of DAPBOOLS, ullage asked by the program). With
 * none, "STORE NULL TRANSLATION POLICIES" and SENSETYP, the jets to favor:
 * +0, or two ("FAVOR +X JETS DURING AN APS BURN."). */
void rcs_plus1(void)
{
    ts(ADDRESS_OF(COTROLER));
    dxch(ADDRESS_OF(EDOTQ));
    tc_at(RCS + 3u);
    rot_touv();
    dxch(ADDRESS_OF(OMEGAU));

    set_A(BIT7); /* SENSEGET */
    rand_channel(CHAN31);
    if (A_is_zero()) {
        set_A(ONE); /* +X0RULGE */
        x_translation();
        return;
    }
    set_A(BIT8);
    rand_channel(CHAN31);
    if (A_is_zero()) {
        x_translation(); /* -XTRANS, A +0 */
        return;
    }
    set_A(ULLAGER);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (ccs_positive(A_ADDRESS)) {
        set_A(ONE); /* TCF +X0RULGE */
        x_translation();
        return;
    }
    ts(ADDRESS_OF(NEXTU));
    ts(ADDRESS_OF(NEXTV));
    cs(ADDRESS_OF(DAPBOOLS)); /* "BURNING OR DRIFTING?" */
    mask(DRIFTBIT);
    if (!A_is_zero()) {
        ca(ADDRESS_OF(FLGWRD10)); /* "DPS (INCLUDING DOCKED) OR APS?" */
        mask(APSFLBIT);
        if (ccs_positive(A_ADDRESS)) set_A(TWO);
    }
    ts(ADDRESS_OF(SENSETYP)); /* TSENSE */
    continue_at(QRCONTRL);
}

/* MIMRET's and TSNUMBRT's CCS A of AORBTRAN: "THREE FOR B" "TWO FOR A
 * SYSTEM 2 JET X TRANS", in NUMBERT. */
static void two_jets(void)
{
    set_A(ccs_positive(A_ADDRESS) ? THREE : TWO); /* CA ONE; AD TWO, or AD TWO to +0 */
    ts(ADDRESS_OF(NUMBERT));
}

/* +X0RULGE (A one) or -XTRANS (A +0): ROTINDEX 5 or 4 (the +X or -X
 * jets of TYPEPOLY, which ALLJETS runs on into), SENSETYP 2 or 1 ("FAVOR
 * APPROPRIATE JETS DURING TRANS."), NUMBERT the policy: four jets
 * (ACC4OR2X of DAPBOOLS), or the A or B pair (AORBTRAN). SELCTSUB's
 * jets, or the policy a failure leaves, go to NEXTU and NEXTV: "CODES OF
 * TRANSLATION FOR AFTER ROTATION". "AN ALARM RESULTS WHEN NO POLICY IS
 * AVAILABLE BECAUSE OF FAILURES." (OCT 02002, read through Q). */
static void x_translation(void)
{
    ad(FOUR);
    ts(ADDRESS_OF(ROTINDEX));
    ad(NEG3);
    ts(ADDRESS_OF(SENSETYP));
    ca(ADDRESS_OF(DAPBOOLS));
    mask(ACC4OR2X);
    if (ccs_positive(A_ADDRESS)) {
        set_A(FOUR); /* TRANS4 */
        ts(ADDRESS_OF(NUMBERT));
    } else {
        ca(ADDRESS_OF(DAPBOOLS));
        mask(AORBTRAN);
        two_jets();
    }
    if (!selctsub(TSNUMBRT + 1u)) return;
    if (!ccs_positive(ADDRESS_OF(POLYTEMP))) {
        tc_to(TSNUMBRT + 4u, ALARM); /* TC ALARM; OCT 02002 */
        return;
    }
    senseget_after_alarm();
}

/* 17,2172, where ALARM returns: the policy's jets in 00314OCT (6, 13, 5,
 * 14: the U jets) to NEXTU, the others to NEXTV. Then QRCONTRL. */
void senseget_after_alarm(void)
{
    set_A(OCT_00314);
    mask_at(ADDRESS_OF(POLYTEMP));
    ts(ADDRESS_OF(NEXTU)); /* TSNEXTS */
    set_A(negate(OCT_00314));
    mask_at(ADDRESS_OF(POLYTEMP));
    ts(ADDRESS_OF(NEXTV));
    continue_at(QRCONTRL);
}

/* ---- The control mode -- LM pp. 1444-1446 ---- */

/* QRCONTRL -- LM p. 1444: "Q,R-AXES RCS CONTROL MODE SELECTION": bit 13 of
 * channel 31 ("BITS INVERTED") 1 is auto, to ATTSTEER; else PULSES of
 * DAPBOOLS "MINIMUM IMPULSE MODE", or CHEKSTIK: "RATE COMMAND/ATTITUDE HOLD
 * MODE". The minimum impulse mode zeroes the attitude errors first (IBNKCALL
 * ZATTEROR, in P-AXIS). */
void qrcontrl(void)
{
    set_A(BIT13);
    rand_channel(CHAN31);
    if (ccs_positive(A_ADDRESS)) {
        continue_at(ATTSTEER);
        return;
    }
    set_A(PULSES); /* CHKBIT10: "PULSES = 1 FOR MIN IMP USE OF RHC" */
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (A_is_zero()) {
        continue_at(CHEKSTIK);
        return;
    }
    inhint();
    tc_to(QRCONTRL + 012u, IBNKCALL); /* CADR ZATTEROR */
}

/* The minimum impulse: +-14MS on the U and V axes, both for a Q pulse,
 * one negated for an R. Back to MINRTN by RETJADR when LOOPER has done
 * the U axis. */
static void min_impulse(word u, word v)
{
    set_A(u);
    ts(ADDRESS_OF(TJU));
    set_A(v);
    ts(ADDRESS_OF(TJV)); /* MINQR */
    set_A(MINADR);
    ts(ADDRESS_OF(RETJADR));
    set_A(ONE);
    ts(ADDRESS_OF(OLDQRMIN));
    continue_at(MINRTN);
}

/* 17,2214, where ZATTEROR returns: "MINIMUM IMPULSE MODE". QERROR and
 * RERROR +0 ("FOR DISPLAYS"), channel 31 to TEMP31; one pulse for each
 * deflection of the hand controller out of detent, the first of +Q, -Q,
 * +R, -R (bits 1, 2, 5 and 6 at 0), while OLDQRMIN is +0. After a pulse
 * (OLDQRMIN one), CHECKIN keeps in OLDQRMIN the bits deflected, and no
 * pulse fires again until the hand controller is back in detent. */
void qrcontrl_after_zatteror(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(QERROR));
    ts(ADDRESS_OF(RERROR));
    relint();
    read_channel(CHAN31);
    ts(ADDRESS_OF(TEMP31)); /* "IS EQUAL TO DAPTEMP1" */
    switch (ccs_at(ADDRESS_OF(OLDQRMIN))) {
    case POSITIVE: /* TCF CHECKIN */
        cs(ADDRESS_OF(TEMP31));
        mask(OCT63);
        ts(ADDRESS_OF(OLDQRMIN));
        continue_at(XTRANS);
        return;
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(TEMP31)); /* FIREQR */
        mask(BIT1);
        break;
    case NEGATIVE: mask(BIT1); break;
    case IS_MINUS_ZERO: break; /* EXTEND; BZF +QMIN, A +0 */
    }
    if (A_is_zero()) {
        min_impulse(K14MS, negate(K14MS)); /* +QMIN */
        return;
    }
    ca(ADDRESS_OF(TEMP31));
    mask(BIT2);
    if (A_is_zero()) {
        min_impulse(negate(K14MS), K14MS); /* -QMIN */
        return;
    }
    ca(ADDRESS_OF(TEMP31));
    mask(BIT5);
    if (A_is_zero()) {
        min_impulse(K14MS, K14MS); /* +RMIN */
        return;
    }
    ca(ADDRESS_OF(TEMP31));
    mask(BIT6);
    if (A_is_zero()) {
        min_impulse(negate(K14MS), negate(K14MS)); /* -RMIN */
        return;
    }
    continue_at(XTRANS);
}

/* MINRTN -- LM p. 1445, with the axis in A: "IF DOCKED, USE 60MS MINIMUM
 * IMPULSE". Then MIMRET: NUMBERT the A or B pair, and AFTERTJ. */
void minrtn(void)
{
    ts(ADDRESS_OF(AXISCTR));
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (!A_is_zero()) {
        unsigned tju = indexed(ADDRESS_OF(TJU), axis());
        switch (ccs_at(tju)) {
        case POSITIVE: set_A(K60MS); break;
        case NEGATIVE: set_A(negate(K60MS)); break;
        case IS_PLUS_ZERO:
        case IS_MINUS_ZERO: break; /* TS TJU of +0 */
        }
        ts(tju);
    }
    ca(ADDRESS_OF(DAPBOOLS)); /* MIMRET */
    mask(AORBTRAN);
    two_jets();
    aftertj();
}

/* ---- The rate command mode -- LM pp. 1446-1449 ---- */

/* One axis' commanded rate, "WC = A*(B+|D|)*D": the hand controller's
 * deflection (SAVEHAND, or SAVEHAND +1 for R) times LINRAT plus its
 * magnitude, times STIKSENS ("COMMAND Q RATE, SCALED 45 DEG/SEC"), to
 * last (QLAST or RLAST); the change from last time to A. at_ccs: whether
 * the CCS of the deflection runs (an overflow of the TS before it skips
 * it). */
static void commanded_rate(unsigned hand, unsigned last, bool at_ccs)
{
    if (at_ccs) (void)ccs_at(hand); /* TCF +3, +2, +1: the magnitude -1 */
    double_A();
    double_A();
    ad(LINRAT);
    mp_at(hand);
    ca(L_ADDRESS);
    mp_at(ADDRESS_OF(STIKSENS));
    xch(last);
    com();
    ad_at(last);
}

/* Whether a commanded rate's change (DAPTEMP3 or DAPTEMP4) passes the
 * breakout level: its magnitude -1 plus -RATEDB positive. The CCS's
 * exits are TCs, at at, +1 and +2 (Q set, to no purpose). */
static bool breaks_out(unsigned change, word at)
{
    switch (ccs_at(change)) {
    case POSITIVE: tc_at(at); break;
    case IS_PLUS_ZERO: tc_at(at + 1u); break;
    case NEGATIVE: tc_at(at + 2u); break;
    case IS_MINUS_ZERO: break;
    }
    ad_at(ADDRESS_OF(MRATEDB)); /* AD -RATEDB */
    return !A_is_zero_or_minus();
}

/* CHEKSTIK -- LM p. 1446: "RATE COMMAND MODE:" "NOT IN GTS WHEN IN ATT
 * HOLD", and COTROLER -1: "COME BACK TO RCS NEXT TIME". In detent (bit 15 of
 * CH31TEMP 1): if the last pass was not manual rate command, STILLRCS,
 * attitude hold; else "BIT 9 IS 0." and DAMPING, no deflection. Out of
 * detent (RHCACTIV) or damping: the commanded Q and R rates, the rate
 * differences (OMEGAQ, OMEGAR less the commands) turned to U,V. A change of
 * command past the breakout level is direct rate control (ENTERUV -2, TCQR
 * 40CYCL: 4 seconds); else direct rate continues if it ran last time (QRBIT
 * of RCSFLAGS), or "PSEUDO-AUTO CONTROL." by STILLRCS. */
void chekstik(void)
{
    bool at_ccs;

    ts(ADDRESS_OF(INGTS));
    set_A(negate(ONE)); /* "1/ACCS WILL DO THE NULLING DRIVES" */
    ts(ADDRESS_OF(COTROLER));
    set_A(BIT15);
    mask_at(ADDRESS_OF(CH31TEMP));
    if (!A_is_zero()) { /* else "BRANCH IF OUT OF DETENT." */
        set_A(OURRCBIT);
        mask_at(ADDRESS_OF(DAPBOOLS));
        if (A_is_zero()) {
            continue_at(STILLRCS);
            return;
        }
        set_A(negate(BIT9));
        mask_at(ADDRESS_OF(RCSFLAGS));
        ts(ADDRESS_OF(RCSFLAGS)); /* "BIT 9 IS 0." */
        set_A(ZERO);              /* DAMPING */
        ts(ADDRESS_OF(SAVEHAND));
        ts(ADDRESS_OF(SAVEHAND) + 1u);
    }
    commanded_rate(ADDRESS_OF(SAVEHAND), ADDRESS_OF(QLAST), true); /* RHCACTIV */
    at_ccs = !ts(ADDRESS_OF(DAPTEMP3));
    commanded_rate(ADDRESS_OF(SAVEHAND) + 1u, ADDRESS_OF(RLAST), at_ccs);
    if (!ts(ADDRESS_OF(DAPTEMP4))) cs(ADDRESS_OF(QLAST));
    ad_at(ADDRESS_OF(OMEGAQ));
    if (!ts(ADDRESS_OF(QRATEDIF))) cs(ADDRESS_OF(RLAST));
    ad_at(ADDRESS_OF(OMEGAR));
    if (!ts(ADDRESS_OF(RRATEDIF))) dxch(ADDRESS_OF(QRATEDIF)); /* ENTERQR */
    tc_at(ENTERQR + 1u); /* "TRANSFORM RATES FROM Q,R TO U,V AXES" */
    rot_touv();
    dxch(ADDRESS_OF(URATEDIF));
    if (breaks_out(ADDRESS_OF(DAPTEMP3), ENTERQR + 4u) ||
        breaks_out(ADDRESS_OF(DAPTEMP4), ENTERQR + 014u)) {
        set_A(K40CYCL); /* "BREAKOUT LEVEL EXCEEDED.  DIRECT RATE." */
        ts(ADDRESS_OF(TCQR));
        enteruv();
        return;
    }
    ca(ADDRESS_OF(RCSFLAGS));
    mask(QRBIT);
    if (A_is_zero()) {
        continue_at(STILLRCS);
        return;
    }
    enteruv(); /* "CONTINUE DIRECT RATE CONTROL." */
}

/* ENTERUV -- LM p. 1448: "DIRECT RATE CONTROL": the attitude errors zeroed
 * (IBNKCALL ZATTEROR). */
static void enteruv(void)
{
    inhint();
    tc_to(ENTERUV + 1u, IBNKCALL); /* FCADR ZATTEROR */
}

/* Whether a rate difference (URATEDIF or VRATEDIF) is within the target
 * deadband: its magnitude -1 plus TARGETDB (-RATEDB) not positive. The
 * CCS's exits in VDB are TCs, at at, +1 and +2; elsewhere TCFs (at 0). */
static bool within(unsigned rate, word at)
{
    switch (ccs_at(rate)) {
    case POSITIVE:
        if (at) tc_at(at);
        break;
    case IS_PLUS_ZERO:
        if (at) tc_at(at + 1u);
        break;
    case NEGATIVE:
        if (at) tc_at(at + 2u);
        break;
    case IS_MINUS_ZERO: break;
    }
    ad_at(ADDRESS_OF(TARGETDB));
    return A_is_zero_or_minus();
}

/* 17,2454, where ZATTEROR returns: the manual attitude errors zeroed.
 * "IF TARGET DB IS EXCEEDED, CONTINUE" "DIRECT RATE CONTROL."; a rate
 * difference within it is zeroed, and with both within, "SWITCH TO
 * PSEUDO-AUTO CONTROL ON NEXT" "PASS." (TOPSEUDO); so too once TCQR runs
 * out: "BRANCH IF TIME EXCEEDS 4 SEC." Then the jet times by BACKHAND,
 * RETJADR back to it. */
void enteruv_after_zatteror(void)
{
    bool pseudo = false;

    relint();
    set_A(ZERO);
    ts(ADDRESS_OF(DYERROR));
    ts(ADDRESS_OF(DYERROR) + 1u);
    ts(ADDRESS_OF(DZERROR));
    ts(ADDRESS_OF(DZERROR) + 1u);
    if (!within(ADDRESS_OF(URATEDIF), 0u)) {
        if (within(ADDRESS_OF(VRATEDIF), 0u)) {
            set_A(ZERO);
            ts(ADDRESS_OF(VRATEDIF));
        }
    } else if (within(ADDRESS_OF(VRATEDIF), VDB + 1u)) { /* VDB */
        pseudo = true;
    } else {
        set_A(ZERO);
        ts(ADDRESS_OF(URATEDIF));
    }
    if (!pseudo) {
        ca(ADDRESS_OF(TCQR)); /* QRTIME: "DIRECT RATE TIME CHECK." */
        pseudo = A_is_zero_or_minus();
    }
    if (pseudo) {
        set_A(negate(QRBIT)); /* TOPSEUDO */
        mask_at(ADDRESS_OF(RCSFLAGS));
        ts(ADDRESS_OF(RCSFLAGS)); /* "BIT 11 IS 0." */
    } else {
        cs(ADDRESS_OF(RCSFLAGS));
        mask(QRBIT);
        ads(ADDRESS_OF(RCSFLAGS)); /* "BIT 11 IS 1." */
        tc_at(QRTIME + 6u);        /* TC +4 */
    }
    set_A(HANDADR);
    ts(ADDRESS_OF(RETJADR));
    set_A(ONE);
    continue_at(BACKHAND);
}

/* BACKHAND -- LM p. 1449, with the axis in A: NUMBERT four (two jets), the
 * axis skipped or not; then the jet time for the rate difference: URATEDIF
 * times 1/ANET2 of the axis and its sign (the inverse acceleration two jets
 * give, with the offset acceleration: AXISDIFF of TJET_LAW finds the V axis'
 * words), doubled with Q, pinned on an overflow ("RIGHT SIGN AND BIGGER THAN
 * 150MS"). SETTIME: "SCALED AT 10.67 WHICH IS CLOSE TO 10.24", in TJU or
 * TJV. Then AFTERTJ. */
void backhand(void)
{
    word u;

    ts(ADDRESS_OF(AXISCTR));
    set_A(FOUR);
    ts(ADDRESS_OF(NUMBERT));
    if (skipped()) return;
    u = axis();
    switch (ccs_at(indexed(ADDRESS_OF(URATEDIF), u))) {
    case POSITIVE: set_A(ZERO); break;
    case NEGATIVE: set_A(ONE); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* A +0 */
    }
    ad_at(indexed(AXISDIFF, u));
    cs(indexed(ADDRESS_OF(1_ANET2) + 1u, a_word())); /* INDEX A: 0, 1, 16 or 17 */
    mp_at(indexed(ADDRESS_OF(URATEDIF), u));         /* "JET TIME IN A, SCALED 32 SEC" */
    ts(Q_ADDRESS);
    ddoubl(); /* DAS A */
    ad_at(Q_ADDRESS);
    if (ts(A_ADDRESS)) ca(Q_ADDRESS); /* "OVERFLOW SKIP" */
    ts(indexed(ADDRESS_OF(TJU), u));  /* SETTIME */
    aftertj();
}

/* ---- The trim gimbal or the jets -- LM pp. 1449-1450 ---- */

/* TRYGTS -- LM p. 1449: "GTS WILL BE TRIED IF" "USEQRJTS = 0," "ALLOWGTS
 * POS," "JETS
 * ARE OFF (Q,R-AXES)" (channel 5 +0); else RCS. GOTOGTS: by DTCB to GTS,
 * in bank 21. While INGTS: "YES.  SET UP A DAMPED NULLING DRIVE.",
 * TIMEGMBL; "NO.  NULLING WAS SET UP BEFORE.  DO RCS." */
void trygts(void)
{
    set_A(USEQRJTS);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (ccs_positive(A_ADDRESS)) { /* "IS JET USE MANDATORY." */
        continue_at(RCS);
        return;
    }
    switch (ccs_at(ADDRESS_OF(ALLOWGTS))) { /* "DOES AOSTASK OK CONTROL FOR GTS?" */
    case IS_PLUS_ZERO: continue_at(RCS); return;
    case IS_MINUS_ZERO: not_modelled(); return; /* READ CHAN5 without its EXTEND */
    default: break;
    }
    read_channel(CHAN5);
    switch (ccs_register(A_ADDRESS)) { /* eight jet bits: never negative */
    case POSITIVE: break;              /* TCF CHKINGTS */
    case IS_PLUS_ZERO:
        set_L(GTSCADR[1]); /* GOTOGTS */
        set_A(GTSCADR[0]);
        dtcb(GOTOGTS + 2u);
        return;
    default: not_modelled(); return;
    }
    switch (ccs_at(ADDRESS_OF(INGTS))) { /* CHKINGTS */
    case IS_PLUS_ZERO: continue_at(RCS); return;
    case IS_MINUS_ZERO: break; /* to TC IBNKCALL, with no INHINT */
    default: inhint(); break;
    }
    tc_to(CHKINGTS + 4u, IBNKCALL); /* CADR TIMEGMBL */
}

/* 17,2620, where TIMEGMBL returns: INGTS +0, then RCS. */
void chkingts_after_timegmbl(void)
{
    relint();
    set_A(ZERO);
    ts(ADDRESS_OF(INGTS));
    continue_at(RCS);
}

/* ---- QERRCALC -- LM p. 1451 ---- */

/* QERRCALC -- LM p. 1451: "SUBROUTINE TO COMPUTE Q,R-AXES ATTITUDE ERRORS
 * FOR USE IN THE RCS AND GTS CONTROL LAWS AND THE DISPLAYS." The Y and Z CDU
 * errors ("CDU ANGLE -- ANGLE DESIRED (Y-AXIS)") through M21, M22 and M31,
 * M32, plus DELQEROR and DELREROR, "KALCMANU INERFACE ERROR", to QERROR and
 * RERROR: "SAVE Q-ERROR FOR EIGHT-BALL DISPLAY." */
void qerrcalc(void)
{
    ca(ADDRESS_OF(CDUY)); /* CAE */
    msu_at(ADDRESS_OF(CDUYD));
    ts(ADDRESS_OF(DAPTEMP1)); /* "SAVE FOR RERRCALC" */
    mp_at(ADDRESS_OF(M21));
    ts(ADDRESS_OF(E));
    ca(ADDRESS_OF(CDUZ));
    msu_at(ADDRESS_OF(CDUZD));
    ts(ADDRESS_OF(DAPTEMP2));
    mp_at(ADDRESS_OF(M22));
    ad_at(ADDRESS_OF(DELQEROR));
    ad_at(ADDRESS_OF(E));
    xch(ADDRESS_OF(QERROR));

    ca(ADDRESS_OF(DAPTEMP1)); /* RERRCALC */
    mp_at(ADDRESS_OF(M31));
    ts(ADDRESS_OF(E));
    ca(ADDRESS_OF(DAPTEMP2));
    mp_at(ADDRESS_OF(M32));
    ad_at(ADDRESS_OF(DELREROR));
    ad_at(ADDRESS_OF(E));
    xch(ADDRESS_OF(RERROR));
    tc_q();
}

/* ---- ATTSTEER and TJLAW -- LM p. 1452 ---- */

/* STILLRCS -- LM p. 1452, which ATTSTEER equals: ATTSTEER "IS THE ENTRY
 * POINT FOR
 * Q,R-AXES (U,V-AXES) ATTITUDE CONTROL USING THE REACTION CONTROL SYSTEM".
 * The Q,R errors turned to
 * UERROR and VERROR; then TJLAW for the V axis, RETJADR back to TJLAW +3
 * for the U. */
void stillrcs(void)
{
    ca(ADDRESS_OF(RERROR));
    lxch(A_ADDRESS);
    ca(ADDRESS_OF(QERROR));
    tc_at(STILLRCS + 3u);
    rot_touv();
    dxch(ADDRESS_OF(UERROR));
    set_A(TJLAWADR); /* TJLAW */
    ts(ADDRESS_OF(RETJADR));
    set_A(ONE);
    tjlaw_plus3();
}

/* TJLAW +3 -- LM p. 1452, with the axis in A: "PREPARES CALL TO TJETLAW (OR
 * SPSRCS(DOCKED))", the axis' error in E and rate in EDOT. Docked, "IF
 * GIMBAL USABLE DO GTS CONTROL" "ON THE NEXT PASS." ("GIMBAL USABLE.
 * STORE POSITIVE VALUE.") and SPSRCS: "DETERMINE RCS CONTROL". */
void tjlaw_plus3(void)
{
    word u;

    ts(ADDRESS_OF(AXISCTR));
    if (skipped()) return;
    u = axis();
    ca(indexed(ADDRESS_OF(UERROR), u));
    ts(ADDRESS_OF(E));
    ca(indexed(ADDRESS_OF(OMEGAU), u));
    ts(ADDRESS_OF(EDOT));
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (!ccs_positive(A_ADDRESS)) {
        tc_to(TJLAW + 025u, TJETLAW);
        return;
    }
    cs(ADDRESS_OF(DAPBOOLS)); /* +3 */
    mask(USEQRJTS);
    if (ccs_positive(A_ADDRESS)) ts(ADDRESS_OF(COTROLER));
    inhint();
    tc_to(TJLAW + 034u, IBNKCALL); /* CADR SPSRCS */
}

/* 17,2714, where TJETLAW returns: TCF AFTERTJ. */
void tjlaw_after_tjetlaw(void)
{
    aftertj();
}

/* 17,2724, where SPSRCS returns: "ALWAYS CALL FOR 2-JET CONTROL ABOUT
 * U,V." "FALL THROUGH TO JET SELECTION, ETC." */
void tjlaw_after_spsrcs(void)
{
    relint();
    set_A(FOUR);
    ts(ADDRESS_OF(NUMBERT));
    aftertj();
}

/* ---- Q,R-JET-SELECTION-LOGIC -- LM pp. 1452-1455 ---- */

/* AFTERTJ -- LM p. 1453: "IF SNUFFBIT SET DURING A DPS BURN GO TO" "XTRANS;
 * THAT IS, INHIBIT CONTROL.", and so in drifting flight (DRIFTBIT 0); else
 * DOROTAT. */
static void aftertj(void)
{
    ca(ADDRESS_OF(FLAGWRD5));
    mask(SNUFFBIT);
    if (!A_is_zero()) {
        cs(ADDRESS_OF(FLGWRD10));
        mask(APSFLBIT);
        if (!A_is_zero()) {
            ca(ADDRESS_OF(DAPBOOLS));
            mask(DRIFTBIT);
            if (A_is_zero()) {
                continue_at(XTRANS);
                return;
            }
        }
    }

    /* DOROTAT: the axis' jet time, none to NOROTAT; its magnitude in
     * ABSTJ, ROTINDEX "0 1 2 3 = -U -V +U +V". Under 150 ms, DOSKIP; else
     * SELCTSUB's jets to WRITEP (INDEXES: the U or V jets' place in
     * channel 5), and FEEDBACK. */
    set_A(TWO);
    ts(L_ADDRESS);
    switch (ccs_at(indexed(ADDRESS_OF(TJU), axis()))) {
    case POSITIVE: break; /* TCF +5 */
    case NEGATIVE: zl(); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: norotat(); return;
    }
    ad(ONE);
    ts(ADDRESS_OF(ABSTJ));
    ca(ADDRESS_OF(AXISCTR));
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(ROTINDEX));
    ca(ADDRESS_OF(ABSTJ));
    ad(M150MS);
    if (A_is_zero_or_minus()) {
        continue_at(DOSKIP);
        return;
    }
    if (!selctsub(DOROTAT + 022u)) return;
    ca(indexed(INDEXES, axis()));
    ts(L_ADDRESS);
    ca(ADDRESS_OF(POLYTEMP));
    inhint();
    tc_to(DOROTAT + 031u, indexed(WRITEP, (word)L)); /* INDEX L; TC WRITEP */
}

/* 17,2775, where WRITEP returns: RELINT; TCF FEEDBACK. */
void dorotat_after_writep(void)
{
    relint();
    feedback();
}

/* NOROTAT -- LM p. 1454: the axis' jets off (WRITEP -1, indexed by
 * INDEXES). */
static void norotat(void)
{
    ca(indexed(INDEXES, axis()));
    inhint();
    tc_to(NOROTAT + 4u, indexed(WRITEP - 1u, a_word())); /* INDEX A; TC WRITEP -1 */
}

/* 17,3004, where WRITEP -1 returns: RELINT, then LOOPER. */
void norotat_after_writep(void)
{
    relint();
    continue_at(LOOPER);
}

/* LOOPER -- LM p. 1454: AXISCTR 1 (the V axis done) back by TC RETJADR to
 * the U axis, AXISCTR +0; +0, the pass is done: CLOSEOUT. (Its negative and
 * -0 exits enter DOSKIP and DOSKIP +1.) RETJADR holds GENADR MINRTN,
 * BACKHAND or TJLAW +3, which runs as TC to it. */
void looper(void)
{
    word to;

    switch (ccs_at(ADDRESS_OF(AXISCTR))) {
    case POSITIVE: break;
    case IS_PLUS_ZERO: closeout(); return;
    case NEGATIVE: continue_at(DOSKIP); return;
    case IS_MINUS_ZERO: continue_at(DOSKIP + 1u); return;
    }
    tc_at(LOOPER + 1u);
    to = fetch(ADDRESS_OF(RETJADR));
    if (to != MINRTN && to != BACKHAND && to != TJLAW + 3u) not_modelled();
    tc_through(ADDRESS_OF(RETJADR));
}

/* DOSKIP -- LM p. 1454: a time under 150 ms. At 14 ms (+TJMINT6) or more,
 * NOTMIN; under it, ABSTJ and the axis' time made 14 ms ("TIMES LESS THAN 14
 * MSEC ARE TAKEN TO CALL FOR A SINGLE-JET" "MINIMUM IMPULSE, WITH THE JET
 * CHOSEN SEMI-RANDOMLY.": the low bit of LOSCALAR picks NUMBERT 0 or 1, -X
 * or +X), but with a translation's SENSETYP the jet that helps it: "ENSURE
 * MIN-IMPULSE NOT AGAINST TRANS". */
void doskip(void)
{
    cs(ADDRESS_OF(ABSTJ));
    doskip_plus1();
}

/* NOTMIN -- LM p. 1454: SELCTSUB's jets to WRITEP, then JTLST: off again
 * after ABSTJ. */
static void notmin(void)
{
    if (!selctsub(NOTMIN)) return;
    ca(indexed(INDEXES, axis()));
    inhint();
    ts(ADDRESS_OF(T6FURTHA) + 1u);
    ca(ADDRESS_OF(POLYTEMP));
    tc_to(NOTMIN + 7u, indexed(WRITEP, fetch(ADDRESS_OF(T6FURTHA) + 1u)));
}

void doskip_plus1(void)
{
    unsigned tju;

    ad(TJMINT6);
    if (A_is_zero_or_minus()) {
        notmin();
        return;
    }
    ads(ADDRESS_OF(ABSTJ));
    tju = indexed(ADDRESS_OF(TJU), axis());
    switch (ccs_at(tju)) {
    case POSITIVE: set_A(TJMINT6); break;
    case NEGATIVE: set_A(negate(TJMINT6)); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* TS TJU of +0 */
    }
    ts(tju);
    switch (ccs_at(ADDRESS_OF(SENSETYP))) {
    case POSITIVE: break; /* TCF NOTMIN -1: NUMBERT SENSETYP -1 */
    case IS_PLUS_ZERO:
        read_channel(LOSCALAR);
        mask(ONE);
        break;
    case NEGATIVE:
        inhint(); /* READ LOSCALAR without its EXTEND: TC 4, INHINT */
        mask(ONE);
        break;
    case IS_MINUS_ZERO: mask(ONE); break;
    }
    ts(ADDRESS_OF(NUMBERT));
    notmin();
}

/* 17,3042, where WRITEP returns: JTLST ("IN QR BANK BY NOW"), the axis'
 * SKIPU +0, and FEEDBACK. */
void notmin_after_writep(void)
{
    ca(ADDRESS_OF(ABSTJ));
    ts(ADDRESS_OF(T6FURTHA));
    tc_at(NOTMIN + 012u);
    jtlst();
    relint();
    set_A(ZERO);
    ts(indexed(ADDRESS_OF(SKIPU), axis()));
    feedback();
}

/* FEEDBACK -- LM p. 1455: NO.UJETS or NO.VJETS for the "RATE DERIVATION
 * FEEDBACK": ONE for NUMBERT three or less, else TWO. Then LOOPER. */
static void feedback(void)
{
    set_A(negate(THREE));
    ad_at(ADDRESS_OF(NUMBERT));
    set_A(A_is_zero_or_minus() ? ONE : TWO);
    ts(indexed(ADDRESS_OF(NO_UJETS), axis()));
    continue_at(LOOPER);
}

/* XTRANS -- LM p. 1455: no rotation: TJU and TJV +0; SKIPU and SKIPV made 4,
 * and for each that was not +0 the axis' jets written by WRITEU -1 or WRITEV
 * -1. Then CLOSEOUT. */
void xtrans(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(TJU));
    ts(ADDRESS_OF(TJV));
    set_A(FOUR);
    inhint();
    xch(ADDRESS_OF(SKIPU));
    if (!A_is_zero()) {
        tc_to(XTRANS + 010u, WRITEU - 1u);
        return;
    }
    xtrans_after_writeu();
}

/* 17,3074, where WRITEU -1 returns. */
void xtrans_after_writeu(void)
{
    set_A(FOUR);
    xch(ADDRESS_OF(SKIPV));
    relint();
    if (A_is_zero()) {
        closeout();
        return;
    }
    inhint();
    tc_to(XTRANS + 017u, WRITEV - 1u);
}

/* 17,3103, where WRITEV -1 returns: RELINT; TCF CLOSEOUT. */
void xtrans_after_writev(void)
{
    relint();
    closeout();
}

/* ---- JTLST and ROT-TOUV -- LM pp. 1456-1457 ---- */

/* JTLST -- LM p. 1456: "THE JET LIST:" "THIS IS A WAITLIST FOR T6RUPTS."
 * "THE LIST IS EXACTLY 3 LONG." The new time, T6FURTHA, and the axis to
 * write then, T6FURTHA +1: before the next T6RUPT (TIME6), it goes first and
 * the rest move down, T6 on (TURNON: bit 15 of channel 13); else before the
 * second (MIDORLST), second; else last (LASTCHG), its time less the others'.
 * (An overflow there skips the TC Q into ROT-TOUV, which returns through the
 * same Q.) */
void jtlst(void)
{
    cs(ADDRESS_OF(T6FURTHA));
    ad_at(ADDRESS_OF(TIME6));
    if (!A_is_zero_or_minus()) { /* "TIME6 -- TI IS IN A" */
        lxch(ADDRESS_OF(NXT6ADR));
        dxch(ADDRESS_OF(T6NEXT));
        dxch(ADDRESS_OF(T6FURTHA));
        ts(ADDRESS_OF(TIME6));
        lxch(ADDRESS_OF(NXT6ADR));
        set_A(BIT15); /* TURNON */
        wor_channel(CHAN13);
        tc_q();
        return;
    }
    ad_at(ADDRESS_OF(T6NEXT));   /* MIDORLST */
    if (!A_is_zero_or_minus()) { /* "TIME6 + T6NEXT - T IS IN A" */
        lxch(ADDRESS_OF(T6NEXT) + 1u);
        dxch(ADDRESS_OF(T6FURTHA));
        su_at(ADDRESS_OF(TIME6));
        dxch(ADDRESS_OF(T6NEXT));
        tc_q();
        return;
    }
    cs(A_ADDRESS); /* LASTCHG */
    ad(NEG0);
    if (ts(ADDRESS_OF(T6FURTHA))) {
        rot_touv();
        return;
    }
    tc_q();
}

/* ROT-TOUV -- LM p. 1457: "ROT-TOUV IS ENTERED WITH THE Q-COMPONENT OF THE
 * QUANTITY TO BE TRANSFORMED IN A AND THE R-COMPONENT IN L." "IN THE U-V
 * SYSTEM NO CROSS-COUPLING IS" "PRODUCED FROM RCS JET FIRINGS." The U
 * component, COEFFQ.(Q) + COEFFR.(R), to A; the V component, -COEFFQ.(Q) +
 * COEFFR.(R), to L; each, overflowed, "LIMITED TO POSMAX OR NEGMAX" (an
 * overflowed sum's A, +1 or -1, INDEXes LIMITS -1 or +1, complemented). */
void rot_touv(void)
{
    lxch(ADDRESS_OF(ROTEMP2)); /* "(R) IS PUT INTO ROTEMP2" */
    mp_at(ADDRESS_OF(COEFFQ));
    xch(ADDRESS_OF(ROTEMP2)); /* "(R) GOES TO A AND COEFFQ.(Q) TO ROTEMP2" */
    mp_at(ADDRESS_OF(COEFFR));
    ts(L_ADDRESS); /* "COEFFR.(R) IS PUT INTO L" */
    ad_at(ADDRESS_OF(ROTEMP2));
    if (ts(ADDRESS_OF(ROTEMP1))) {
        cs(indexed(LIMITS, a_word()));
        ts(ADDRESS_OF(ROTEMP1));
    }
    cs(ADDRESS_OF(ROTEMP2));
    ad_at(L_ADDRESS); /* "-COEFFQ.(Q) + COEFFR.(R) IS NOW IN A" */
    if (ts(ZERO_ADDRESS)) cs(indexed(LIMITS, a_word()));
    lxch(ADDRESS_OF(ROTEMP1));
    tc_q();
}

/* ---- SELCTSUB -- LM pp. 1457-1458 ---- */

/* The jets of ALLJETS for ROTINDEX (the rotation) and TYPEPOLY for
 * NUMBERT (the policy: -X, +X, A, B or all) in POLYTEMP; A those of them
 * failed (CH5MASK). */
static void policy(void)
{
    ca(indexed(ALLJETS, fetch(ADDRESS_OF(ROTINDEX))));
    mask_at(indexed(TYPEPOLY, fetch(ADDRESS_OF(NUMBERT))));
    ts(ADDRESS_OF(POLYTEMP));
    mask_at(ADDRESS_OF(CH5MASK));
}

/* SELCTSUB -- LM p. 1457, by TC at at: the policy NUMBERT asks, if none of
 * its jets for the rotation has failed; else (FAILOOP) the first such from
 * three down: "JETS OF THE DESIRED POLICY ARE SELECTED." "THE ALARM IS SENT"
 * "IF NONE CAN BE FOUND." (the axis' time +0; TC ALARM, OCT 02004). Returns
 * false at the alarm. */
static bool selctsub(word at)
{
    tc_at(at);
    policy();
    if (ccs_positive(A_ADDRESS)) {
        set_A(THREE);
        for (;;) {
            ts(ADDRESS_OF(NUMBERT)); /* FAILOOP */
            policy();
            if (A_is_zero()) break; /* BZF FAILOOP -2: TC Q */
            if (!ccs_positive(ADDRESS_OF(NUMBERT))) {
                ts(indexed(ADDRESS_OF(TJU), axis()));
                tc_to(SELCTSUB + 027u, ALARM);
                return false;
            }
        }
    }
    tc_q();
    return true;
}

/* 17,3224, where ALARM returns: TCF NOROTAT. */
void selctsub_after_alarm(void)
{
    norotat();
}

/* ---- CLOSEOUT -- LM p. 1458 ---- */

/* CLOSEOUT -- LM p. 1458: "THE FOLLOWING SETS THE INTERRUPT FLIP-FLOP AS
 * SOON AS POSSIBLE, WHICH PERMITS A RETURN TO THE INTERRUPTED JOB."
 * MAKERUPT: EXTEND; EDRUPT MAKERUPT, an interrupt by program: ZRUPT the word
 * after it, BRUPT the instruction itself, interrupt in progress, and on at
 * address 0: A, ADRRUPT, runs as TC ENDJASK. ENDJASK: the job's A, L, B, Q
 * (from DAPARUPT, DAPBQRPT) back in the interrupt registers, and DAPZRUPT
 * NEGMAX: "NEGATIVE DAPZRUPT SIGNALS JASK IS OVER."; ZRUPT the job's Z. Then
 * NOQRSM resumes it. */
void closeout(void)
{
    set_A(ADRRUPT);
    tc_at(CLOSEOUT + 1u); /* TC MAKERUPT */
    store(ADDRESS_OF(ZRUPT), MAKERUPT + 2u);
    store(ADDRESS_OF(BRUPT), fetch_fixed(MAKERUPT + 1u));
    *agc.in_interrupt = 1u;
    tc_at(0u); /* ENDJASK */
    dxch(ADDRESS_OF(DAPARUPT));
    dxch(ADDRESS_OF(ARUPT));
    dxch(ADDRESS_OF(DAPBQRPT));
    xch(ADDRESS_OF(BRUPT));
    lxch(Q_ADDRESS);
    set_A(NEGMAX);
    dxch(ADDRESS_OF(DAPZRUPT));
    dxch(ADDRESS_OF(ZRUPT));
    continue_at(NOQRSM);
}

#endif
