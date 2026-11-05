/* SPDX-License-Identifier: MIT */
/* LANDING_ANALOG_DISPLAYS.agc, pp. 898-907 (Luminary099 only): LANDISP and
 * the altitude and altitude rate (ALTROUT, ARCOMP, ALTOUT, OLDDATA,
 * NEWDATA, DATAOUT), DISINDAT and INTLZE, the velocity (SPEEDRUN), the
 * forward and lateral velocities and their meters (LATFWDV, VMONITOR), and
 * DISPRSET. The faithful C is c64-port/src/lm_landing_displays.c. */
#include "landing_displays.h"
#include "../agc/names.h"

#if LUMINARY099

static void altrout(void);
static void altrout_plus1(void);
static void altout(void);
static void altout_plus1(void);
static void olddata_plus1(void);
static void newdata(void);
static void dataout(void);
static void disindat(void);
static void speedrun(void);
static void disprset(void);
static void aborton(void);

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC LADQSAVE at at: the word in LADQSAVE runs as an instruction. DISINDAT
 * leaves ALTROUT +1 or ALTOUT +1 there, a TC back to the pass that called
 * it (Q gets LADQSAVE +1). Any other word runs at LADQSAVE, out of the
 * file, as the faithful port leaves it. */
static void tc_ladqsave(word at)
{
    word return_word = fetch(ADDRESS_OF(LADQSAVE));

    tc_at(at);
    if (return_word == ALTROUT + 1u) {
        tc_through(ADDRESS_OF(LADQSAVE));
        altrout_plus1();
    } else if (return_word == ALTOUT + 1u) {
        tc_through(ADDRESS_OF(LADQSAVE));
        altout_plus1();
    } else {
        continue_at(ADDRESS_OF(LADQSAVE));
    }
}

/* ---- LANDISP -- p. 898 ---- */

/* LANDISP: "UPDATE TBASE2 AND PIPCTR SIMULTANEOUSLY" (-TIME1 and PIPCTR1,
 * the word DISINDAT keeps its return in, as LADQSAVE). Then "IS LANDING
 * ANALOG DISPLAYS FLAG SET?" (FLAGWRD7's SWANDBIT): no, DISPRSET. Yes:
 * IMODES33 "BIT 7 = 0 (DO ALTRATE), =1 (DO ALT.)". (A MASK of a positive
 * bit is never negative, so each CCS A goes on at its first or second
 * word.) */
void landisp(void)
{
    lxch(ADDRESS_OF(PIPCTR1));
    cs(ADDRESS_OF(TIME1));
    dxch(ADDRESS_OF(TBASE2));
    cs(ADDRESS_OF(FLAGWRD7));
    mask(SWANDBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        disprset(); /* "NO." */
        return;
    }
    ca(ADDRESS_OF(IMODES33));
    mask(BIT7);
    if (ccs_register(A_ADDRESS) == POSITIVE)
        altout();
    else
        altrout();
}

/* ---- The altitude rate -- pp. 898-899 ---- */

/* ALTROUT: "CHECK MODE SELECT SWITCH AND DIDFLG." DISINDAT comes back to
 * ALTROUT +1 through LADQSAVE, after SPEEDRUN and VMONITOR. */
static void altrout(void)
{
    tc_at(ALTROUT);
    disindat();
}

/* ALTROUT +1: "ALTERNATE ALTITUDE RATE WITH ALTITUDE" (IMODES33 bit 7
 * set), and channel 14 bit 2 for the meter: "ALTRATE (BIT2 = 1), ALTITUDE
 * (BIT2 = 0)". Then ARCOMP: "COMPUTE ALTRATE=RUNIT.VVECT M/CS *2(-6)",
 * "CONVERT ALTRATE TO BIT UNITS (.5FPS/BIT)", and add the "ALTITUDE RATE
 * COMPENSATION FACTOR", DALTRATE, times DT. To the meter goes -ALTRATE
 * when ALTRATE is negative ("SEND POS. PULSES TO ALTM REG."), otherwise
 * ALTRATE with "SIGN BIT = 1": "*DO NOT SEND +0*". (An overflow at TS
 * ALTRATE skips the CS, and BZMF tests the +1 or -1 it leaves.) */
static void altrout_plus1(void)
{
    cs(ADDRESS_OF(IMODES33));
    mask(BIT7);
    ads(ADDRESS_OF(IMODES33));
    set_A(BIT2);
    wor_channel(CHAN14);
    /* ARCOMP */
    ca(ADDRESS_OF(RUNIT));
    mp_at(ADDRESS_OF(VVECT)); /* "MULTIPLY X-COMPONENTS." */
    xch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(RUNIT) + 1u);
    mp_at(ADDRESS_OF(VVECT) + 1u);
    ads(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(RUNIT) + 2u);
    mp_at(ADDRESS_OF(VVECT) + 2u);
    ads(ADDRESS_OF(RUPTREG1)); /* "ALTITUDE RATE IN M/CS *2(-6)." */
    set_A(ARCONV);
    mp_at(ADDRESS_OF(RUPTREG1));
    ddoubl();
    ddoubl();
    xch(ADDRESS_OF(RUPTREG1)); /* "ALTITUDE RATE IN BIT UNITS*2(-14)." */
    ca(ADDRESS_OF(DALTRATE));
    mp_at(ADDRESS_OF(DT));
    ad_at(ADDRESS_OF(RUPTREG1));
    if (!ts(ADDRESS_OF(ALTRATE))) cs(ADDRESS_OF(ALTRATE));
    if (A_is_zero_or_minus()) { /* "CHECK POLARITY OF ALTITUDE RATE." */
        ca(ADDRESS_OF(ALTRATE));
        ad(BIT15);
    }
    dataout();
}

/* DATAOUT: "ACTIVATE THE LANDING ANALOG DISPLAYS": the word to ALTM, and
 * channel 14 bit 3, "BIT3 DRIVES THE ALT/ALTRATE METER." (An overflow at
 * TS ALTM skips the CAF, and the +1 or -1 it leaves goes to channel 14.)
 * The pass ends: TCF TASKOVER. */
static void dataout(void)
{
    if (!ts(ADDRESS_OF(ALTM))) set_A(BIT3);
    wor_channel(CHAN14);
    continue_at(TASKOVER);
}

/* ---- The altitude -- pp. 899-900 ---- */

/* ALTOUT: "CHECK MODE SELECT SWITCH AND DIDFLG.", back at ALTOUT +1. */
static void altout(void)
{
    tc_at(ALTOUT);
    disindat();
}

/* ALTOUT +1: IMODES33 bit 7 clear ("ALTERNATE ALTITUDE RATE WITH
 * ALTITUDE"), channel 14 bit 2 clear for the altitude. ALTBITS "=-1 IF OLD
 * ALT. DATA TOBE EXTRAPOLATED.": OLDDATA. Otherwise ALTBITS +1 holds a new
 * altitude: ALTBITS gets -1 "FOR SWITCH USE NEXT PASS", the altitude goes
 * to ALTSAVE, and a "NEW ALTITUDE EXTRAPOLATION WITH ALTRATE" over DT
 * (DT/BIT10: "RESCALE DT*2(-14) TO *2(-9) TIME IN CS.", times ARTOA2)
 * joins OLDDATA +1. (TS IMODES33 stores a MASK, and TS ALTBITS the +0 CCS
 * leaves for -0: neither skips.) */
static void altout_plus1(void)
{
    set_A(negate(BIT7));
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
    set_A(negate(BIT2));
    wand_channel(CHAN14);
    switch (ccs_at(ADDRESS_OF(ALTBITS))) {
    case NEGATIVE:    /* OLDDATA */
        set_A(ARTOA); /* "RATE APPLIES FOR .5 SEC. (4X/SEC. CYCLE)" */
        olddata_plus1();
        return;
    case IS_MINUS_ZERO: ts(ADDRESS_OF(ALTBITS)); break; /* "SET ALTBITS FROM -0 TO +0." */
    case POSITIVE:
    case IS_PLUS_ZERO: break;
    }
    set_A(negate(ONE));
    dxch(ADDRESS_OF(ALTBITS)); /* "SET ALTBITS=-1 FOR SWITCH USE NEXT PASS." */
    dxch(ADDRESS_OF(ALTSAVE));
    set_A(BIT10);
    xch(Q_ADDRESS);
    zl();
    ca(ADDRESS_OF(DT));
    dv_at(Q_ADDRESS);
    mp(ARTOA2);      /* ".0021322 *2(+8)" */
    olddata_plus1(); /* "RATE APPLIES FOR DT CS." */
}

/* OLDDATA +1: "EXTRAPOLATE WITH ALTITUDE RATE." into the double word
 * ALTSAVE, then "FORCE SIGN AGREEMENT ASSUMING A NON-NEGATIVE ALTSAVE.":
 * POSMAX + 1 added to the low word and POSMAX to the high word overflow
 * only for a non-negative altitude, and the overflow at TS ALTSAVE skips
 * to NEWDATA. "IF ALTSAVE IS NEGATIVE, ZERO ALTSAVE AND ALTSAVE +1 AT
 * ZERODATA." (An overflow at TS ALTSAVE +1 skips the CAF ZERO, leaving
 * its carry, +1 or -1, for the high word.) */
static void olddata_plus1(void)
{
    mp_at(ADDRESS_OF(ALTRATE));
    ddoubl();
    ad_at(ADDRESS_OF(ALTSAVE) + 1u);
    if (!ts(ADDRESS_OF(ALTSAVE) + 1u)) set_A(ZERO);
    ads(ADDRESS_OF(ALTSAVE));
    set_A(POSMAX);
    ad(ONE);
    ad_at(ADDRESS_OF(ALTSAVE) + 1u);
    if (!ts(ADDRESS_OF(ALTSAVE) + 1u)) set_A(ZERO);
    ad(POSMAX);
    ad_at(ADDRESS_OF(ALTSAVE));
    if (ts(ADDRESS_OF(ALTSAVE))) { /* "POSSIBLY SKIP TO NEWDATA." */
        newdata();
        return;
    }
    /* ZERODATA: "ZERO ALTSAVE AND ALTSAVE +1 - - - NO NEGATIVE ALTITUDES
     * ALLOWED." (TS L of +0 does not skip.) Then ZDATA2: the zeros go to
     * ALTSAVE, and NEWDATA. */
    set_A(ZERO);
    ts(L_ADDRESS);
    dxch(ADDRESS_OF(ALTSAVE));
    newdata();
}

/* NEWDATA: "SET NEGATIVE ALTSAVE +1 TO +0." and "PROVIDE A 15 BIT UNSIGNED
 * OUTPUT.": "THE HI-ORDER PART IS +1 OR +0.", so a high word of +1 sets
 * bit 15 over the low word. Then DATAOUT, "DISPATCH UNSIGNED BITS TO ALTM
 * REG." (OLDDATA and ZERODATA leave ALTSAVE +0 or positive; the CCS's
 * negative exit, TCF DATAOUT, is followed as written, and its -0 exit,
 * which would run on into DISINDAT, is not modelled.) */
static void newdata(void)
{
    switch (ccs_at(ADDRESS_OF(ALTSAVE) + 1u)) {
    case NEGATIVE: set_A(ZERO); /* fall through */
    case IS_MINUS_ZERO: ts(ADDRESS_OF(ALTSAVE) + 1u); break;
    case POSITIVE:
    case IS_PLUS_ZERO: break;
    }
    switch (ccs_at(ADDRESS_OF(ALTSAVE))) {
    case POSITIVE: set_A(BIT15); /* fall through */
    case IS_PLUS_ZERO: ad_at(ADDRESS_OF(ALTSAVE) + 1u); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    dataout();
}

/* ---- DISINDAT and INTLZE -- pp. 900-901 ---- */

/* DISINDAT: "SAVE RETURN TO ALTROUT +1 OR ALTOUT +1" in LADQSAVE.
 * "WISHETH THE ASTRONAUT THE ANALOG DISPLAYS? I.E., IS THE MODE SELECT
 * SWITCH IN PGNCS?" (channel 30 bit 6, a RAND that is never negative): no,
 * DISPRSET. If DIDFLAG is set, SPEEDRUN: "PERFORM DATA DISPLAY SEQUENCE."
 * Otherwise "PERFORM INITIALIZATION FUNCTIONS.": "SET DIDFLAG.", IMODES33
 * bit 7 clear "TO DISPLAY ALTRATE FIRST AND ALT. SECOND", and, only "IN
 * DESCENT TRAJECTORY" (FLAGWRD0's R10FLBIT), channel 12 bit 8 ("SET
 * DISPLAY INERTIAL DATA OUTBIT."), the velocity monitor's flags and meters
 * zero, and TC TWIDDLE: INTLZE in BIT4, 40 ms. (TS IMODES33 stores a MASK
 * and the TS's after CAF ZERO store +0: none skips.) */
static void disindat(void)
{
    qxch(ADDRESS_OF(LADQSAVE));
    set_A(BIT6);
    rand_channel(CHAN30);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        disprset(); /* "NO.  ASTRONAUT REQUESTS NO INERTIAL DATA" */
        return;
    }
    cs(ADDRESS_OF(FLAGWRD1)); /* "YES. CHECK STATUS OF DIDFLAG." */
    mask(DIDFLBIT);
    if (A_is_zero()) {
        speedrun();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD1));
    mask(DIDFLBIT);
    ads(ADDRESS_OF(FLAGWRD1));
    set_A(negate(BIT7));
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
    cs(ADDRESS_OF(FLAGWRD0));
    mask(R10FLBIT);
    if (A_is_zero()) {
        continue_at(TASKOVER); /* "NO" */
        return;
    }
    set_A(BIT8);
    wor_channel(CHAN12);
    set_A(ZERO);
    ts(ADDRESS_OF(TRAKLATV)); /* "LATERAL VELOCITY MONITOR FLAG" */
    ts(ADDRESS_OF(TRAKFWDV)); /* "FORWARD VELOCITY MONITOR FLAG" */
    ts(ADDRESS_OF(LATVMETR)); /* "LATVEL MONITOR METER" */
    ts(ADDRESS_OF(FORVMETR)); /* "FORVEL MONITOR METER" */
    set_A(BIT4);
    tc_to(INTLZE - 3u, TWIDDLE); /* ADRES INTLZE */
}

/* INTLZE -1, where TWIDDLE returns: TCF TASKOVER. */
void intlze_minus_one(void)
{
    continue_at(TASKOVER);
}

/* INTLZE: channel 12 bit 2, "ENABLE RR ERROR COUNTER.", and IMODES33 bit
 * 8: "SET INERTIAL DATA FLAG." */
void intlze(void)
{
    set_A(BIT2);
    wor_channel(CHAN12);
    cs(ADDRESS_OF(IMODES33));
    mask(BIT8);
    ads(ADDRESS_OF(IMODES33));
    continue_at(TASKOVER);
}

/* ---- The velocity -- pp. 901-902 ---- */

/* SPEEDRUN: "UPDATE THE VELOCITY VECTOR". DT is "T - TN", the time since
 * the last PIPA reading (two ADs of HALF "CORRECT FOR POSSIBLE OVERFLOW OF
 * TIME1."), and each axis of VVECT, "VVECT = VN + DELV + GN(T-TN) M/CS
 * *2(-5)": GDT/2 times DT over 1SEC, plus SERVICER's V, plus the PIPA
 * count since then (PIPAX, plus PIPATMPX "IN CASE PIPAX HAS BEEN ZEROED")
 * times KPIP1(5). Then "PAUSE 40 MS TO LET OTHER RUPTS IN.": TC VARDELAY.
 * (TS ITEMP5 of 1SEC does not skip.) */
static void speedrun(void)
{
    unsigned axis;

    cs(ADDRESS_OF(PIPTIME) + 1u);
    ad_at(ADDRESS_OF(TIME1)); /* "COMPUTE T - TN" */
    ad(HALF);
    ad(HALF);
    xch(ADDRESS_OF(DT)); /* "SAVE FOR LATER USE" */
    set_A(ONE_SEC);
    ts(ADDRESS_OF(ITEMP5)); /* "INITIALIZE FOR DIVISION LATER" */
    for (axis = 0; axis < 3u; ++axis) {
        dca(ADDRESS_OF(GDT_2) + 2u * axis);
        ddoubl();
        ddoubl();
        mp_at(ADDRESS_OF(DT));
        dv_at(ADDRESS_OF(ITEMP5));
        xch(ADDRESS_OF(VVECT) + axis);  /* "VVECT = G(T-TN) M/CS *2(-5)" */
        dca(ADDRESS_OF(V) + 2u * axis); /* "M/CS *2(-7)" */
        ddoubl();                       /* "RESCALE TO 2(-5)" */
        ddoubl();
        ads(ADDRESS_OF(VVECT) + axis);
        ca(ADDRESS_OF(PIPAX) + axis); /* "DELV CM/SEC *2(-14)" */
        ad_at(ADDRESS_OF(PIPATMPX) + axis);
        mp(KPIP1_5); /* "DELV M/CS *2(-5)" */
        ads(ADDRESS_OF(VVECT) + axis);
    }
    set_A(BIT3);
    tc_to(SPEEDRUN + 0101u, VARDELAY);
}

/* ---- The forward and lateral velocities -- pp. 902-907 ---- */

static void vmonitor(word index);

/* SPEEDRUN +0102, VARDELAY's return. Only "IN DESCENT TRAJECTORY": out of
 * it, TC LADQSAVE, back to ALTROUT +1 or ALTOUT +1. In it:
 *   - the velocity less SERVICER's correction, DELVS: "= VX - DVX
 *     M/CS*2(-5)" in ITEMP1-3 (an overflow at a TS skips the next CA, and
 *     the +1 or -1 it leaves is added in its place);
 *   - VHY and VHZ, "VELOCITY DIRECTED ALONG THE Y-COORDINATE" and Z: the
 *     dot products with UHYP, the "CROSS-RANGE HALF-UNIT VECTOR", and UHZP,
 *     "DOWN-RANGE", doubled;
 *   - GET22/32: "GET SIN(AOG),COS(AOG) FROM GPMATRIX." (M22 and M32, in
 *     E-bank 6);
 *   - LATFWDV: "COMPUTE LATERAL AND FORWARD VELOCITIES.",
 *     "=VHY(COS)AOG+VHZ(SIN)AOG M/CS *2(-5)" to LATVEL and
 *     "=VHZ(COS)AOG-VHY(SIN)AOG M/CS *2(-5)." to FORVEL, each "IN BIT UNITS
 *     *2(-14)." (times VELCONV, doubled);
 *   - VMONITOR for each, "FIRST MONITOR FORWARD THEN LATERAL VEL.", with
 *     ITEMP6 -MAXVBITS: "-547 BIT UNITS (OCTAL) AT 0.5571 FPS/BIT";
 *   - "DRIVE THE X-POINTER DISPLAY." (channel 14's BITSET, PRIO6: the CDU
 *     drive bits for Y and Z), and TC LADQSAVE: "GO TO ALTROUT +1 OR TO
 *     ALTOUT +1".
 * (A MASK of R10FLBIT is never negative, so the CCS goes on at its first
 * or second word. The TS's of GET22/32 store words: none skips.) */
void speedrun_after_vardelay(void)
{
    cs(ADDRESS_OF(FLAGWRD0)); /* "ARE WE IN DESCENT TRAJECTORY?" */
    mask(R10FLBIT);
    if (ccs_register(A_ADDRESS) != POSITIVE) {
        tc_ladqsave(SPEEDRUN + 0106u); /* "NO." */
        return;
    }
    ca(ADDRESS_OF(DELVS));    /* "HI X OF VELOCITY CORRECTION TERM." */
    ad_at(ADDRESS_OF(VVECT)); /* "HI X OF UPDATED VELOCITY VECTOR." */
    if (!ts(ADDRESS_OF(ITEMP1))) ca(ADDRESS_OF(DELVS) + 2u);
    ad_at(ADDRESS_OF(VVECT) + 1u);
    if (!ts(ADDRESS_OF(ITEMP2))) ca(ADDRESS_OF(DELVS) + 4u);
    ad_at(ADDRESS_OF(VVECT) + 2u);
    if (!ts(ADDRESS_OF(ITEMP3))) ca(ADDRESS_OF(ITEMP1));
    mp_at(ADDRESS_OF(UHYP)); /* "HI X OF CROSS-RANGE HALF-UNIT VECTOR." */
    xch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP2));
    mp_at(ADDRESS_OF(UHYP) + 2u);
    ads(ADDRESS_OF(RUPTREG1)); /* "ACCUMULATE PARTIAL PRODUCTS." */
    ca(ADDRESS_OF(ITEMP3));
    mp_at(ADDRESS_OF(UHYP) + 4u);
    ads(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(RUPTREG1));
    double_A();
    xch(ADDRESS_OF(VHY)); /* "VHY=VMP.UHYP M/CS*2(-5)." */
    ca(ADDRESS_OF(ITEMP1));
    mp_at(ADDRESS_OF(UHZP)); /* "HI X OF DOWN-RANGE HALF-UNIT VECTOR." */
    xch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP2));
    mp_at(ADDRESS_OF(UHZP) + 2u);
    ads(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP3));
    mp_at(ADDRESS_OF(UHZP) + 4u);
    ads(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(RUPTREG1));
    double_A();
    xch(ADDRESS_OF(VHZ)); /* "VHZ = VMP.UHZP M/CS*2(-5)." */
    /* GET22/32 */
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(M22));
    ts(ADDRESS_OF(ITEMP3));
    ca(ADDRESS_OF(M32));
    ts(ADDRESS_OF(ITEMP4));
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    /* LATFWDV */
    ca(ADDRESS_OF(ITEMP4));
    mp_at(ADDRESS_OF(VHY));
    xch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP3));
    mp_at(ADDRESS_OF(VHZ));
    ads(ADDRESS_OF(RUPTREG1));
    set_A(VELCONV); /* "CONVERT LATERAL VELOCITY TO BIT UNITS." */
    mp_at(ADDRESS_OF(RUPTREG1));
    ddoubl();
    xch(ADDRESS_OF(LATVEL));
    ca(ADDRESS_OF(ITEMP4)); /* "COMPUTE FORWARD VELOCITY." */
    mp_at(ADDRESS_OF(VHZ));
    xch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP3));
    mp_at(ADDRESS_OF(VHY));
    cs(A_ADDRESS);
    ads(ADDRESS_OF(RUPTREG1));
    set_A(VELCONV); /* "CONVERT FORWARD VELOCITY TO BIT UNITS." */
    mp_at(ADDRESS_OF(RUPTREG1));
    ddoubl();
    xch(ADDRESS_OF(FORVEL));
    set_A(negate(MAXVBITS)); /* "ACC.=-199.9989 FT./SEC." */
    ts(ADDRESS_OF(ITEMP6));
    /* VMONITOR, "LOOP TWICE.": ITEMP5 1 (FORVEL), then 0 (LATVEL). The
     * CCS ITEMP5 leaves +0 in A for the second pass, and for BITSET. */
    set_A(ONE);
    do {
        ts(ADDRESS_OF(ITEMP5));
        vmonitor(fetch(ADDRESS_OF(ITEMP5)));
    } while (ccs_at(ADDRESS_OF(ITEMP5)) == POSITIVE);
    set_A(BITSET);
    wor_channel(CHAN14);
    tc_ladqsave(ZERODATA - 1u);
}

/* The velocity monitor, for one of the two velocities (INDEX ITEMP5):
 * LATVEL +index, its meter LATVMETR +index (how far the needle has been
 * driven), its flag TRAKLATV +index (+1 or -1 while the needle is held at
 * one end of the scale, +0 otherwise) and its drive counter CDUTCMD +index
 * (CDUTCMD or CDUSCMD). The scale runs from -MAXVBITS to +MAXVBITS,
 * "+199.9989 FT.SEC.". Each label below ends at ZEROLSTY with the drive in
 * A, or at ZEROLSTY +3 with the drive in RUPTREG3 +index and the new flag
 * in A. */
static void chklasty(word index);
static void lastok(word index);
static void lastposy(word index);
static void posvmaxy(word index);
static void lastnegy(word index);
static void negvmaxy(word index);
static void lvlimits(word index);
static void latvpos(word index);
static void latvneg(word index);
static void neglmlv(word index);
static void lvminlm(word index);
static void zerolsty(word index);
static void zerolsty_plus3(word index);

#define VELOCITY(index) indexed(ADDRESS_OF(LATVEL), index)
#define METER(index)    indexed(ADDRESS_OF(LATVMETR), index)
#define FLAG(index)     indexed(ADDRESS_OF(TRAKLATV), index)
#define DRIVE(index)    indexed(ADDRESS_OF(RUPTREG3), index)

/* VMONITOR: "FORWARD AND LATERAL VELOCITY LANDING ANALOG DISPLAYS
 * MONITOR." A velocity at or beyond the end of the scale, CHKLASTY; inside
 * it (or zero), LVLIMITS. */
static void vmonitor(word index)
{
    switch (ccs_at(VELOCITY(index))) {
    case POSITIVE:
        cs(VELOCITY(index));
        ad(MAXVBITS); /* "+199.9989 FT.SEC." */
        break;
    case NEGATIVE:
        ca(VELOCITY(index));
        ad(MAXVBITS);
        break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: lvlimits(index); return;
    }
    if (A_is_zero_or_minus())
        chklasty(index);
    else
        lvlimits(index);
}

/* CHKLASTY: off the scale. If the meter stands on the other side of zero
 * from the velocity, a drive of MAXVBITS toward it: LASTPOSY +5
 * (-MAXVBITS) or LASTNEGY +4 (+MAXVBITS). Otherwise LASTOK. */
static void chklasty(word index)
{
    switch (ccs_at(METER(index))) {
    case POSITIVE:
        ca(VELOCITY(index));
        if (A_is_zero_or_minus()) {
            set_A(negate(MAXVBITS)); /* LASTPOSY +5 */
            zerolsty(index);
            return;
        }
        break;
    case NEGATIVE:
        cs(VELOCITY(index));
        if (A_is_zero_or_minus()) {
            set_A(MAXVBITS); /* LASTNEGY +4 */
            zerolsty(index);
            return;
        }
        break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    lastok(index);
}

/* LASTOK: by the flag, LASTPOSY (held at the top), LASTNEGY (at the
 * bottom), or, not held, to the end the velocity is beyond: NEGVMAXY or
 * POSVMAXY. */
static void lastok(word index)
{
    switch (ccs_at(FLAG(index))) {
    case POSITIVE: lastposy(index); return;
    case NEGATIVE: lastnegy(index); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    ca(VELOCITY(index));
    if (A_is_zero_or_minus())
        negvmaxy(index);
    else
        posvmaxy(index);
}

/* LASTPOSY: held at the top. Still positive: POSVMAXY. Now zero or
 * negative: a drive of -MAXVBITS. */
static void lastposy(word index)
{
    ca(VELOCITY(index));
    if (!A_is_zero_or_minus()) {
        posvmaxy(index);
        return;
    }
    set_A(negate(MAXVBITS));
    zerolsty(index);
}

/* POSVMAXY: drive the meter up to MAXVBITS, and set the flag +1. */
static void posvmaxy(word index)
{
    cs(METER(index));
    ad(MAXVBITS);
    xch(DRIVE(index));
    set_A(ONE);
    zerolsty_plus3(index);
}

/* LASTNEGY: held at the bottom. Still zero or negative: NEGVMAXY. Now
 * positive: a drive of +MAXVBITS. */
static void lastnegy(word index)
{
    ca(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        negvmaxy(index);
        return;
    }
    set_A(MAXVBITS);
    zerolsty(index);
}

/* NEGVMAXY: drive the meter down to -MAXVBITS, and set the flag -1. */
static void negvmaxy(word index)
{
    ca(METER(index));
    ad(MAXVBITS);
    com();
    xch(DRIVE(index));
    set_A(negate(ONE));
    zerolsty_plus3(index);
}

/* LVLIMITS: on the scale. Held at the top, LATVPOS; at the bottom,
 * LATVNEG. Not held, with the meter positive or zero: LVMINLM (the drive
 * is the velocity less the meter) unless that is a drive down of more than
 * MAXVBITS (AD ITEMP6); then a drive of -MAXVBITS. A negative meter:
 * NEGLMLV. */
static void lvlimits(word index)
{
    switch (ccs_at(FLAG(index))) {
    case POSITIVE: latvpos(index); return;
    case NEGATIVE: latvneg(index); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    cs(METER(index));
    if (!A_is_zero_or_minus()) {
        neglmlv(index);
        return;
    }
    cs(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    ad_at(ADDRESS_OF(ITEMP6));
    ad_at(METER(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    ad_at(VELOCITY(index));
    su_at(METER(index));
    zerolsty(index);
}

/* LATVPOS: held at the top. A velocity at or above zero: LVMINLM. Below
 * zero, drive the meter back to zero (LATVNEG +4). */
static void latvpos(word index)
{
    cs(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    cs(METER(index));
    zerolsty(index);
}

/* LATVNEG: held at the bottom. A velocity at or below zero: LVMINLM.
 * Above zero, drive the meter back to zero. */
static void latvneg(word index)
{
    ca(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    cs(METER(index));
    zerolsty(index);
}

/* NEGLMLV: a negative meter, not held, the mirror of LVLIMITS: LVMINLM
 * unless that is a drive up of more than MAXVBITS; then +MAXVBITS. */
static void neglmlv(word index)
{
    ca(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    set_A(MAXVBITS);
    ad_at(METER(index));
    com();
    ad_at(VELOCITY(index));
    if (A_is_zero_or_minus()) {
        lvminlm(index);
        return;
    }
    su_at(VELOCITY(index));
    ad_at(METER(index));
    com();
    zerolsty(index);
}

/* LVMINLM: the drive is the velocity less the meter. */
static void lvminlm(word index)
{
    cs(METER(index));
    ad_at(VELOCITY(index));
    zerolsty(index);
}

/* ZEROLSTY: the drive in A to RUPTREG3 +index, and the flag +0. */
static void zerolsty(word index)
{
    xch(DRIVE(index));
    set_A(ZERO);
    zerolsty_plus3(index);
}

/* ZEROLSTY +3: the flag from A; the drive to the CDU drive counter less
 * one ("AVOIDS +0 DINC HARDWARE MALFUNCTION"), and added to the meter.
 * (The flag is +0, +1 or -1, and a word plus -0 does not overflow: neither
 * TS skips.) */
static void zerolsty_plus3(word index)
{
    ts(FLAG(index));
    ca(DRIVE(index));
    ad(NEG0);
    ts(indexed(ADDRESS_OF(CDUTCMD), index));
    ca(DRIVE(index));
    ads(METER(index));
}

/* ---- DISPRSET -- p. 907 ---- */

/* DISPRSET: "ARE WE IN DESCENT TRAJECTORY?" Yes: channel 12 bit 8 off,
 * "REMOVE DISPLAY INERTIAL DATA", and if IMODES33 bit 8 is set ("CHECK IF
 * INERTIAL DATA JUST DISPLAYED.") bit 2 too: "DISABLE RR ERROR COUNTER".
 * Then ABORTON. (A MASK of BIT8 is never negative: the CCS goes on at its
 * first or second word.) */
static void disprset(void)
{
    cs(ADDRESS_OF(FLAGWRD0));
    mask(R10FLBIT);
    if (A_is_zero()) {
        aborton(); /* "NO." */
        return;
    }
    set_A(BIT8);
    mask_at(ADDRESS_OF(IMODES33));
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(BIT2);
    ad(BIT8);
    com();
    wand_channel(CHAN12);
    aborton();
}

/* ABORTON: "RESET INERTIAL DATA, INTERLEAVE FLAGS." (IMODES33 bits 8 and
 * 7, BITS8/7) and "RESET DIDFLAG.", and TCF TASKOVER. (Both TS's store a
 * MASK: neither skips.) */
static void aborton(void)
{
    set_A(negate(BITS8_7));
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
    set_A(negate(DIDFLBIT));
    mask_at(ADDRESS_OF(FLAGWRD1));
    ts(ADDRESS_OF(FLAGWRD1));
    continue_at(TASKOVER);
}

#endif
