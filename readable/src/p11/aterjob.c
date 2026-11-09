/* SPDX-License-Identifier: MIT */
/* P11.agc of the CM (Comanche055 pp. 542-547, banks 34 and 37): ATERJOB,
 * the job that "PROVIDES ATTITUDE ERROR DISPLAYS TO THE FDAI DURING SONE
 * BOOST", about every half second, and steers the Saturn with them when
 * the crew has the S/C control of Saturn switch on; the small routines
 * its interpretive code calls (DELSTOR, RLTST, NOPOLY, RESCALES); and
 * S11.1, VHHDOT's display of V1, HDOT and H.
 *
 * Until RPSTART after liftoff the desired attitude is the one at liftoff
 * (OGC, IGC and MGC as MATRXJOB stored them: NOPOLY, "ATTITUDE HOLD");
 * then, until POLYSTOP, the pitch polynomial (POLYNUM's coefficients,
 * POWRSERS) and the roll "LAUNCHAZ-AZIMUTH-.5+SATRLRT*T". The
 * interpretive code turns these into "THE ATTITUDE ERRORS ... AK, AK1,
 * AK2 AS INPUT TO NEEDLER, THE FDAI ATTITUDE ERROR DISPLAY ROUTINE"
 * (ATTDISP1 to NOPOLYM, in bank 37, and ATTDISP2).
 *
 * SATSW is the takeover's state: +1 display only, +0 the needles
 * initialized for steering (the errors at that moment become the bias,
 * BIASAK), -1 steering (the errors less the bias, rescaled for the
 * Saturn). */
#include "p11.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void attdisp(void);
static void atreset(void);
static void satout(void);

/* ---- ATERJOB -- pp. 542-543 ---- */

/* NOPOLY: "COMES HERE IF ATTITUDE HOLD": the interpretive code at
 * NOPOLYM. */
void nopoly(void)
{
    tc_at(NOPOLY);
    intpret();
}

/* NOPOLY +1: the push-down list emptied, and on in bank 37 at NOPOLYM. */
static const word nopoly_words[] = {
    ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(NOPOLYM),
};

const struct interpretive_code NOPOLY_CODE = {034u, NOPOLY + 1u, COUNT(nopoly_words), nopoly_words};

/* ATTDISP: the time since RPSTART; until then, or past POLYSTOP, or once
 * TIME1 has overflowed into TIME2 (the display "IS DISABLED UPON OVFLO OF
 * TIME1"), "ATTITUDE HOLD". Else MPAC, "TIME1-RPSTART", saved in SPOLYARG
 * "FOR USE IN ROLL CALCULATION", and the pitch polynomial evaluated
 * (POWRSERS, "SCALED TO 32 REVOLUTIONS"). (An overflow in the TS MPAC
 * skips the TS SPOLYARG.) */
static void attdisp(void)
{
    cs(ADDRESS_OF(RPSTART));
    ad_at(ADDRESS_OF(TIME1));
    if (A_is_zero_or_minus()) { /* "IF MINUS THEN ATTITUDE HOLD" */
        nopoly();
        return;
    }
    if (!ts(ADDRESS_OF(MPAC))) ts(ADDRESS_OF(SPOLYARG));
    ad_at(ADDRESS_OF(POLYSTOP)); /* "NEG PITCHOVER TIME IN CSECS" */
    if (!A_is_zero_or_minus()) {
        tc_at(ATTDISP + 011u);
        nopoly(); /* "GO TO ATTITUDE HOLD" */
        return;
    }
    ca(ADDRESS_OF(TIME2));
    if (!A_is_zero_or_minus()) {
        tc_at(ATTDISP + 015u);
        nopoly();
        return;
    }
    ca(ADDRESS_OF(POLYNUM));
    ts(L_ADDRESS);
    set_A(COEFPOLY); /* ADRES POLYLOC */
    tc_to(ATTDISP + 021u, POWRSERS);
}

/* ATTDISP +022: "RETURN WITH PITCH(32REV) STORED MPAC, MPAC +1"; MODE
 * +0, double precision; then the interpretive code. */
void attdisp_after_powrsers(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MODE));
    tc_at(ATTDISP + 024u);
    intpret();
}

/* ATTDISP +025: the pitch in revolutions, "32(PITCH(32REV))=PITCH(REV)",
 * pushed, and on in bank 37 (ATTDISP1, "AROUND SETLOC"). */
static const word attdisp_words[] = {
    ORDERS(SETPD, SL), ADDRESS_PLUS_ONE_D(0),
    SL_COUNT(5),       ORDER(PUSH), /* "LET P(RAD)=2*PI*PITCH(REV)" */
    ORDER(GOTO),       ADDRESS(ATTDISP1),
};

const struct interpretive_code ATTDISP_CODE = {034u, ATTDISP + 025u, COUNT(attdisp_words),
                                               attdisp_words};

/* ---- ATTDISP1 to NOPOLYM -- pp. 543-544, bank 37 ---- */

/* ATTDISP1: "CONSTRUCT SM TO S/C MATRIX, TSMV" from the pitch P (pushed)
 * and the roll, "LAUNCHAZ-AZIMUTH-.5+SATRLRT*T" ("TIME1 - RSPSTART ,CSECS
 * B-14.", SPOLYARG, times SATRLRT), "ASSUMING X(SM) ALONG LAUNCH
 * AZIMUTH"; RTB RLTST, "DETERMINE IF ROLLOUT IS COMPLETED" (and if it
 * is, no roll), ends the pass, and ATTDISPR goes on. TSMV, halved: -.5*COS(P),
 * .5*SIN(P) and 0 at 14D, 10D and 12D; at 16D to 26D the products with
 * the roll R. From them "THE HALF UNIT VECTORS XDC,YDC,ZDC = INPUT TO
 * CALCGTA", which computes the "DESIRED SM ORIENTATION" OGC, IGC and MGC.
 *
 * NOPOLYM, where "ATTITUDE HOLD" (NOPOLY) enters too: the desired angles
 * pushed twice, so that from word 2 of the list they read IGC, MGC, OGC,
 * the "Y,Z,X ORDER OF CDUSPOT" ("CHANGE IGC TO MGC FOR COMPATIBILITY");
 * CDUTRIG the CDUs, their sines and cosines. Those three, made two's
 * complement single precision (V1STO2S), into BOOSTEMP; the list's first
 * six words +0; the CDUs (CDUSPOT) the same way, and RTB DELSTOR, "STORE
 * DELTACDU INTO PDL 0,2,4" (DTHETA, DPSI, DPHI), ends the pass. Then MPAC
 * gets DPHI + DTHETA*SIN(CDUZ), with the "CHANGE SCALE OF AK TO 2REVS",
 * and ATTDISP2 in bank 34 goes on. */
static const word attdisp1_words[] = {
    /* ATTDISP1 */ ORDERS(COS, DCOMP),
    STODL_D(14), /* "-.5*COS(P)" */
    ORDER(SIN),
    STODL_D(10), /* ".5*SIN(P)" */
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_D(12), /* "0" */
    ORDERS(SLOAD, DMP),
    ADDRESS_PLUS_ONE(SPOLYARG),
    ADDRESS_PLUS_ONE(SATRLRT),
    ORDERS(SL, DSU),
    SL_COUNT(14),
    ADDRESS_PLUS_ONE(DPHALF),
    ORDERS(DAD, DSU),
    ADDRESS_PLUS_ONE(LAUNCHAZ), /* "LAUNCHAZ = ANGLE FROM NORTH TO X(SM)." */
    ADDRESS_PLUS_ONE(AZIMUTH),  /* "AZIMUTH = -ANGLE FROM NORTH TO Z(S/C)" */
    ORDER(RTB),
    ADDRESS(RLTST),
    /* ATTDISPR */ ORDERS(PUSH, COS), /* "CONTINUE COMPUTING TSMV" */
    ORDER(PUSH),                      /* "LET R(RAD) = 2*PI*ROLL(REV)" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(14),
    STODL_D(22), /* "-.5*COS(R)*COS(P)" */
    ORDER(DCOMP),
    STORE_D(18), /* "-.5*COS(R)" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(10),
    STODL_D(26), /* "-.5*COS(R)*SIN(P)" */
    ORDERS(SIN, PUSH),
    STORE_D(24), /* ".5*SIN(R)" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(14),
    STODL_D(16), /* "-.5*SIN(R)*COS(P)" */
    ORDER(DCOMP),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(10),
    STOVL_D(20), /* "-.5*SIN(R)*SIN(P)" */
    ADDRESS_PLUS_ONE_D(10),
    ORDER(UNIT),
    STOVL(XDC), /* "XDC = .5*UNIT(SIN(P),0,-COS(P))" */
    ADDRESS_PLUS_ONE_D(16),
    ORDER(UNIT),
    STOVL(YDC),
    ADDRESS_PLUS_ONE_D(22),
    ORDER(UNIT),
    STCALL(ZDC),
    ADDRESS(CALCGTA),
    /* NOPOLYM */ ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(OGC),
    ORDERS(PUSH, CALL),
    ADDRESS(CDUTRIG), /* "WITH Y,Z,X ORDER OF CDUSPOT" */
    ORDERS(VLOAD, RTB),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS(V1STO2S),
    STOVL(BOOSTEMP),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STOVL_D(0),
    ADDRESS_PLUS_ONE(CDUSPOT),
    ORDERS(RTB, RTB),
    ADDRESS(V1STO2S),
    ADDRESS(DELSTOR),
    STODL_D(10),
    ADDRESS_PLUS_ONE(SINCDUZ),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SR2), /* "CHANGE SCALE OF AK TO 2REVS" */
    ADDRESS_PLUS_ONE_D(4),
    ORDER(GOTO),
    ADDRESS(ATTDISP2),
};

const struct interpretive_code ATTDISP1_CODE = {037u, ATTDISP1, COUNT(attdisp1_words),
                                                attdisp1_words};

/* STRSAT, with the S/C control of Saturn switch on: "CHECK IF NEEDLER HAS
 * BEEN INITIALIZED" (SATSW +0 or negative): "YES - CONTINUE"; else
 * ATRESET. */
static void strsat(void)
{
    ca(ADDRESS_OF(SATSW));
    if (A_is_zero_or_minus()) {
        attdisp();
        return;
    }
    atreset();
}

/* ATERJOB: with FLAGWRD6's bits 14 and 15 set, "SATURN STICK ON - KILL
 * JOB". Else "CHECK IF S/C CONTROL OF SATURN PANEL SWITCH IS ON" (channel
 * 30's bit 10 +0): "IT IS - GO STEER" (STRSAT). "IT IS NOT - WAS IT ON
 * LAST CYCLE": SATSW positive, "NO - CONTINUE"; +0 or negative, "YES -
 * REINITIALIZE NEEDLER" (ATRESET); -0, ATTDISP too (with no TC). */
void aterjob(void)
{
    ca(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (!A_is_zero()) {
        tc_to(ATERJOB + 4u, ENDOFJOB);
        return;
    }
    set_A(BIT10);
    rand_channel(030u);
    if (A_is_zero()) {
        strsat();
        return;
    }
    switch (ccs_at(ADDRESS_OF(SATSW))) {
    case POSITIVE:
        tc_at(ATERJOB + 013u);
        attdisp();
        return;
    case IS_PLUS_ZERO:
        tc_at(ATERJOB + 014u);
        atreset();
        return;
    case NEGATIVE:
        tc_at(ATERJOB + 015u);
        atreset();
        return;
    case IS_MINUS_ZERO: attdisp(); return;
    }
}

/* ---- After the attitude errors -- pp. 545-546 ---- */

/* SATOUT: "DISPLAY ATTITUDE ERRORS ON FDAI VIA NEEDLER". */
static void satout(void)
{
    tc_to(SATOUT, BANKCALL); /* CADR NEEDLER */
}

/* ATERSET: "DELAY .25 SEC" (DELAYJOB; "EXECUTION + DELAY =.56SEC
 * APPROX"). */
void aterset(void)
{
    set_A(OCT31);
    tc_to(ATERSET + 1u, BANKCALL); /* CADR DELAYJOB */
}

/* ATERSET +3: "END OF ATT ERROR DISPLAY CYCLE": the next. */
void aterset_after_delayjob(void)
{
    tc_at(ATERSET + 3u);
    aterjob();
}

/* RESCALES: "RESCALE AK S FOR NEW HARDWARE SCALING FOR STEERING SATURN":
 * A times SATSCALE, doubled twice; TC Q, back to the TS after the
 * caller's TC. */
static void rescales(void)
{
    mp_at(ADDRESS_OF(SATSCALE));
    ddoubl();
    ddoubl();
    tc_at(Q_ADDRESS);
}

/* STEERSAT +6, after "AKS = AKS - STORED BIAS": each rescaled. An
 * overflow in a TS skips the next word: the CA of the next error, or
 * after AK2, SATOUT's TC BANKCALL, so that its CADR NEEDLER runs as an
 * instruction (CS 2420, a word of ATTDISP2's) and ATERSET follows. */
void steersat_after_exit(void)
{
    ca(ADDRESS_OF(AK));
    tc_at(STEERSAT + 7u);
    rescales();
    if (!ts(ADDRESS_OF(AK))) ca(ADDRESS_OF(AK1));
    tc_at(STEERSAT + 012u);
    rescales();
    if (!ts(ADDRESS_OF(AK1))) ca(ADDRESS_OF(AK2));
    tc_at(STEERSAT + 015u);
    rescales();
    if (!ts(ADDRESS_OF(AK2))) {
        satout();
        return;
    }
    cs(ADDRESS_FIELD(fetch_fixed(SATOUT + 1u)));
    aterset();
}

/* STEERSAT: "STEER L/V" (the interpretive code: the bias taken off). */
static void steersat(void)
{
    tc_at(STEERSAT);
    intpret();
}

/* STEERSAT +1: "AKS = AKS - STORED BIAS" (BIASAK holds the bias
 * complemented: AKLOAD's), the three errors at once as a triple. */
static const word steersat_words[] = {
    ORDERS(TLOAD, TAD), ADDRESS_PLUS_ONE(BIASAK), ADDRESS_PLUS_ONE(AK), STORE(AK), ORDER(EXIT),
};

const struct interpretive_code STEERSAT_CODE = {034u, STEERSAT + 1u, COUNT(steersat_words),
                                                steersat_words};

/* AKLOAD: "STORE AKS INTO BIAS COMPLEMENTED"; SATSW -1, "SET SW TO
 * STEER"; "GO STEER". */
static void akload(void)
{
    cs(ADDRESS_OF(AK));
    ts(ADDRESS_OF(BIASAK));
    cs(ADDRESS_OF(AK1));
    ts(ADDRESS_OF(BIASAK) + 1u);
    cs(ADDRESS_OF(AK2));
    ts(ADDRESS_OF(BIASAK) + 2u);
    set_A(negate(BIT1)); /* CS BIT1 */
    ts(ADDRESS_OF(SATSW));
    tc_at(AKLOAD + 010u);
    steersat();
}

/* ATTDISP2, from bank 37's code (NOPOLYM), with the gimbal angle errors
 * DTHETA, DPSI and DPHI at 0, 2 and 4 of the push-down list and in MPAC
 * the first body axis error: the errors in body axes, by the gimbal
 * angles' sines and cosines (COSCDUZ, COSCDUX, SINCDUX), each halved: 16D
 * ".5(DPHI + DTHETA*SIN(CDUZ))", 17D and 18D from DTHETA and DPSI by the
 * outer angle. "STORE ATTITUDE ERRORS IN AK,AK1,AK2". */
static const word attdisp2_words[] = {
    STODL_D(16),           ADDRESS_PLUS_ONE(COSCDUZ), ORDERS(DMP, PUSH), ADDRESS_PLUS_ONE_D(0),
    ORDERS(DMP, SL1),      ADDRESS_PLUS_ONE(COSCDUX), ORDERS(PDDL, DMP), ADDRESS_PLUS_ONE(SINCDUX),
    ADDRESS_PLUS_ONE_D(2), ORDERS(DAD, SL1),          ORDER(STADR),      AFTER_STADR(STODL_D(17)),
    ORDERS(DMP, SL1),      ADDRESS_PLUS_ONE(SINCDUX), ORDERS(PDDL, DMP), ADDRESS_PLUS_ONE(COSCDUX),
    ADDRESS_PLUS_ONE_D(2), ORDERS(DSU, SL1),          ORDER(STADR),      AFTER_STADR(STORE_D(18)),
    ORDER(TLOAD),          ADDRESS_PLUS_ONE_D(16),    STORE(AK),         ORDER(EXIT),
};

const struct interpretive_code ATTDISP2_CODE = {034u, ATTDISP2, COUNT(attdisp2_words),
                                                attdisp2_words};

/* ATTDISP2 +030, after the attitude errors' EXIT: "CHK TAKEOVER STATUS":
 * SATSW positive, "DISPLAY ONLY" (SATOUT); +0, "STORE BIAS" (AKLOAD);
 * negative, "STEER L/V" (STEERSAT). SATSW -0 would run STEERSAT's
 * interpretive words as instructions: not modelled. */
void attdisp2_after_exit(void)
{
    ca(ADDRESS_OF(SATSW));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        tc_at(ATTDISP2 + 032u);
        satout();
        return;
    case IS_PLUS_ZERO:
        tc_at(ATTDISP2 + 033u);
        akload();
        return;
    case NEGATIVE: steersat(); return;
    case IS_MINUS_ZERO: not_modelled(); return;
    }
}

/* TAKEON: "ENABLE SIVB TAKEOVER" (channel 12's bit 9); SATSW +0,
 * "INDICATE NEEDLER WAS INITIALIZED". */
static void takeon(void)
{
    set_A(BIT9);
    wor_channel(012u);
    set_A(ZERO);
    ts(ADDRESS_OF(SATSW));
    tc_at(TAKEON + 5u);
    satout();
}

/* ATRESET: RCSFLAGS' bit 3 set, the "INITIALIZATION SW FOR NEEDLER", "AND
 * GO DO IT". */
static void atreset(void)
{
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    tc_to(ATRESET + 3u, BANKCALL); /* CADR NEEDLER */
}

/* ATRESET +5: "60 MS -WAIT TILL IMUERRCNTR ZEROED" (DELAYJOB, REVCNT). */
void atreset_after_needler(void)
{
    set_A(REVCNT);
    tc_to(ATRESET + 6u, BANKCALL); /* CADR DELAYJOB */
}

/* ATRESET +010: "CHECK SW STATUS": SATSW positive, "STEER INIT." (TAKEON);
 * else "RETURN TO DISPLAY": SATSW +1, "SW = DISPLAY ON", and "DISABLE SIVB
 * TAKEOVER" (channel 12's bit 9). (SATSW -0 skips to the TS with A +0:
 * SATSW +0.) */
void atreset_after_delayjob(void)
{
    switch (ccs_at(ADDRESS_OF(SATSW))) {
    case POSITIVE:
        tc_at(ATRESET + 011u);
        takeon();
        return;
    case IS_PLUS_ZERO:
        tc_at(ATRESET + 012u); /* TC +1 */
        set_A(BIT1);
        break;
    case NEGATIVE: set_A(BIT1); break;
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(SATSW));
    set_A(negate(BIT9)); /* CS BIT9 */
    wand_channel(012u);
    tc_at(ATRESET + 020u);
    satout();
}

/* ---- S11.1 -- p. 546 ---- */

/* S11.1, VHHDOT's CALL: VMAGI, "VI SCALED 2(7) IN METERS/CSEC"; HDOT, VN
 * along the unit RN, doubled; ALTI, RN's length (which UNIT leaves at
 * 36D) less RPAD, the pad's radius, or with AMOONFLG set less the landing
 * site's (RLS's length, shifted right two places to RN's scale). */
static const word s11_1_words[] = {
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(VN),
    STOVL(VMAGI),
    ADDRESS_PLUS_ONE(RN),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(VN),
    ORDER(SL1),
    STODL(HDOT),
    ADDRESS_PLUS_ONE(RPAD),
    ORDERS(BOF, VLOAD),
    SWITCH(BOF, AMOONFLG),
    ADDRESS(EARTHALT),
    ADDRESS_PLUS_ONE(RLS),
    ORDERS(ABVAL, SR2),
    /* EARTHALT */ ORDER(BDSU),
    ADDRESS_PLUS_ONE_D(36),
    STORE(ALTI),
    ORDER(RVQ),
};

const struct interpretive_code S11_1_CODE = {034u, S11_1, COUNT(s11_1_words), s11_1_words};

/* ---- DELSTOR, RLTST, NOPOLY, RESCALES -- pp. 546-547 ---- */

/* The push-down list's word n (INDEX FIXLOC; TS n): the faithful port
 * stores into any erasable word but FB, Z and BB (FIXLOC a signed count);
 * at any other, not modelled. */
static unsigned work_area(unsigned n)
{
    word fixloc = FIXLOC;
    long at = (long)n + ((fixloc & SIGN_BIT) ? -(long)(MINUS_ZERO & ~fixloc) : (long)fixloc);

    if (at < 0 || at > 01777 || at == FBANK_ADDRESS || at == Z_ADDRESS || at == BBANK_ADDRESS)
        not_modelled();
    return (unsigned)at;
}

/* DELSTOR, an RTB routine: "STORE DELTACDU INTO PDL 0,2,4": the
 * difference of BOOSTEMP's CDU angles and MPAC's (single precision, MSU);
 * then the pair goes on (DANZIG). */
void delstor(void)
{
    unsigned k;

    for (k = 0; k < 3u; ++k) {
        ca(ADDRESS_OF(BOOSTEMP) + k);
        msu_at(ADDRESS_OF(MPAC) + k);
        ts(work_area(2u * k));
    }
    continue_at(DANZIG);
}

/* RLTST, an RTB routine: "DETERMINE IF ROLLOUT IS COMPLETED": the roll
 * angle (MPAC) and the roll rate (SATRLRT +1) of "UNLIKE SIGNS STILL
 * ROLLING"; else "ROLLOUT COMPLETED", MPAC +0: "ZERO OUT ROLL
 * CONTRIBUTION" (the two +0 words of MBDYTCTL +2). */
void rltst(void)
{
    ca(ADDRESS_OF(MPAC));
    mp_at(ADDRESS_OF(SATRLRT) + 1u);
    if (A_is_zero_or_minus()) {
        continue_at(DANZIG);
        return;
    }
    dca(MBDYTCTL + 2u);
    dxch(ADDRESS_OF(MPAC));
    tc_to(RLTST + 010u, DANZIG);
}

#endif
