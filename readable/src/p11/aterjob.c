/* SPDX-License-Identifier: MIT */
/* P11.agc of the CM, part 1 (Comanche055 pp. 542-547, bank 34): ATERJOB,
 * the job that "PROVIDES ATTITUDE ERROR DISPLAYS TO THE FDAI DURING SONE
 * BOOST", about every half second, and steers the Saturn with them when
 * the crew has the S/C control of Saturn switch on; and the small
 * routines its interpretive code calls (DELSTOR, RLTST, NOPOLY, RESCALES).
 *
 * Until RPSTART after liftoff the desired attitude is the one at liftoff
 * (OGC, IGC and MGC as MATRXJOB stored them: NOPOLY, "ATTITUDE HOLD");
 * then, until POLYSTOP, the pitch polynomial (POLYNUM's coefficients,
 * POWRSERS) and the roll "LAUNCHAZ-AZIMUTH-.5+SATRLRT*T". The
 * interpretive code turns these into "THE ATTITUDE ERRORS ... AK, AK1,
 * AK2 AS INPUT TO NEEDLER, THE FDAI ATTITUDE ERROR DISPLAY ROUTINE"
 * (parts 2 and 3).
 *
 * SATSW is the takeover's state: +1 display only, +0 the needles
 * initialized for steering (the errors at that moment become the bias,
 * BIASAK), -1 steering (the errors less the bias, rescaled for the
 * Saturn). */
#include "p11.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

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
