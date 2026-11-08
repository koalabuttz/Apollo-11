/* SPDX-License-Identifier: MIT */
/* AUTOMATIC_MANEUVERS.agc (Comanche055 pp. 1029-1034, bank 21): "JET
 * SWITCHING LOGIC AND CALCULATION OF REQUIRED ROTATION COMMANDS". For each
 * axis in turn (JLOOP, SPNDX 2, 1, 0), the rate error EDOT (the body rate
 * ADOT less the commanded rate WBODY, except in attitude hold) and the
 * attitude error AERR are placed in the phase plane of the listing's FIG.
 * 1, with the dead zone ADB widened by the flat region AF: the rate error
 * made positive, the signs of the others turned with it (POSVEL, NEGVEL),
 * and J6., J7, J8, NJ22 and NJ23 find the region (R18 to R24). Each
 * region's code puts the change in rate it calls for in KMPAC for JTIME
 * (jet_time.c): J18 the rate error taken off, J20 and J24 a rate on the
 * hysteresis slope (SLOPE2), J21 and J22 the rate error to WL either
 * way (JN, JP); J23, in the dead zone, fires only when the axis' forced
 * firing bit is set. The faithful C is c64-port/src/cm_auto_man.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void j6(void);
static void j7(void);
static void j8(void);
static void signck2(void);
static void nj22(void);
static void nj23(void);
static void signck4(void);

/* ---- JETS, JLOOP -- p. 1030 ---- */

/* JETS: T5TEMP "ADB+AF", the dead zone and the "FLAT REGION = .044 DEG",
 * then the loop from the yaw axis, SPNDX 2. (TS T5TEMP overflows only for
 * an ADB at the top of the range, and skips the CAF TWO: A is then +1 or
 * -1.) */
void jets(void)
{
    ca(ADDRESS_OF(ADB));
    ad(FOUR);
    if (!ts(ADDRESS_OF(T5TEMP))) set_A(TWO);
    jloop();
}

/* JLOOP: the axis in SPNDX, twice it in DPNDX, and the axis' body rate
 * ADOT into EDOT; "HOLDFLAG = +0 MEANS THAT DAP IS IN ATTITUDE HOLD AND
 * RATE DAMPING IS OVER": otherwise the commanded rate WBODY taken off. (A
 * TS that overflows skips the next word: past the DOUBLE, A is +1 or -1;
 * past the EXTEND, the DCA runs unextended, as CA of its address field,
 * ADOT +1, indexed.) */
void jloop(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) double_A();
    if (ts(ADDRESS_OF(DPNDX)))
        ca(maneuver_index_read(ADDRESS_OF(ADOT) + 1u, a_word()));
    else
        maneuver_dca_indexed(ADDRESS_OF(ADOT), a_word());
    dxch(ADDRESS_OF(EDOT));
    ca(ADDRESS_OF(HOLDFLAG));
    if (A_is_zero()) { /* EXTEND; BZF INHOLD */
        continue_at(INHOLD);
        return;
    }
    maneuver_dcs_indexed(ADDRESS_OF(WBODY), dpndx());
    das(ADDRESS_OF(EDOT)); /* "= ADOT-WBODY" */
    continue_at(INHOLD);
}

/* INHOLD: the axis' attitude error, "AERR = BIAS + AK", and the sign of
 * the rate error EDOT, its upper word or, that +0 or -0, its lower one:
 * POSVEL for + (or both zero), NEGVEL for -. */
void inhold(void)
{
    ca(maneuver_index_read(ADDRESS_OF(ERRORX), spndx()));
    ts(ADDRESS_OF(AERR));
    switch (ccs_at(ADDRESS_OF(EDOT))) {
    case POSITIVE: posvel(); return;
    case NEGATIVE: negvel(); return;
    default: signck1(); return;
    }
}

/* SIGNCK1: the lower word's sign (+0 counts as +). */
void signck1(void)
{
    enum sign sign = ccs_at(ADDRESS_OF(EDOT) + 1u);

    if (sign == POSITIVE || sign == IS_PLUS_ZERO)
        posvel();
    else
        negvel();
}

/* POSVEL: the rate error positive: EDOTVEL, ADBVEL and AERRVEL the rate
 * error, "+(ADB+AF)" and the attitude error as they are; J6. is called
 * by TC (Q the word after it, NEGVEL, and nothing returns there). */
void posvel(void)
{
    dca(ADDRESS_OF(EDOT));
    dxch(ADDRESS_OF(EDOTVEL));
    ca(ADDRESS_OF(T5TEMP));
    ts(ADDRESS_OF(ADBVEL));
    ca(ADDRESS_OF(AERR));
    ts(ADDRESS_OF(AERRVEL));
    tc_at(POSVEL + 7u);
    j6();
}

/* NEGVEL: the rate error negative: all three turned, "-(ADB+AF)". */
void negvel(void)
{
    dcs(ADDRESS_OF(EDOT));
    dxch(ADDRESS_OF(EDOTVEL));
    cs(ADDRESS_OF(T5TEMP));
    ts(ADDRESS_OF(ADBVEL));
    cs(ADDRESS_OF(AERR));
    ts(ADDRESS_OF(AERRVEL));
    j6();
}

/* ---- J6., J7, J8, the regions -- p. 1031 ---- */

/* J6.: the attitude error (AERRVEL, in A) at most ADB less WLH/SLOP, "=
 * WL+H/SLOPE = .83333 DEG": J8. Past ADB+AF: J7. Otherwise, with the
 * error plus the rate error over the switch line's SLOPE at least ADB,
 * J18; less, J23. */
static void j6(void)
{
    su_at(ADDRESS_OF(ADB));
    ad(WLH_SLOP);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF J8 */
        j8();
        return;
    }
    cs(ADDRESS_OF(T5TEMP));
    ad_at(ADDRESS_OF(AERRVEL));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        j7();
        return;
    }
    dcs(ADDRESS_OF(EDOTVEL));
    dv_at(ADDRESS_OF(SLOPE));
    su_at(ADDRESS_OF(AERRVEL));
    ad_at(ADDRESS_OF(ADB));
    if (A_is_zero_or_minus()) /* EXTEND; BZMF J18 */
        continue_at(J18);
    else
        continue_at(J23);
}

/* J7: the attitude error past ADB+AF by no more than WL-H/SLP, "= WL-H/SLOPE
 * = .5 DEG": J20; further: J21. */
static void j7(void)
{
    set_A(negate(WL_H_SLP)); /* CS */
    su_at(ADDRESS_OF(T5TEMP));
    ad_at(ADDRESS_OF(AERRVEL));
    if (A_is_zero_or_minus()) /* EXTEND; BZMF J20 */
        continue_at(J20);
    else
        continue_at(J21);
}

/* J8: the rate error less WLH, "= WL+H = 0.5 DEG/SEC", in WTEMP: +
 * (its upper word, or that +0 or -0, its lower one +), J22; -, NJ22. */
static void j8(void)
{
    dcs(ADDRESS_OF(WLH));
    dxch(ADDRESS_OF(WTEMP));
    dca(ADDRESS_OF(EDOTVEL));
    das(ADDRESS_OF(WTEMP));
    switch (ccs_at(ADDRESS_OF(WTEMP))) {
    case POSITIVE: continue_at(J22); return;
    case NEGATIVE: nj22(); return;
    default: signck2(); return;
    }
}

/* SIGNCK2: the lower word's sign (+0 counts as +). */
static void signck2(void)
{
    enum sign sign = ccs_at(ADDRESS_OF(WTEMP) + 1u);

    if (sign == POSITIVE || sign == IS_PLUS_ZERO)
        continue_at(J22);
    else
        nj22();
}

/* NJ22: the rate error, on at NJ22 +2 (EXTEND; DV SLOPE), a key of the
 * faithful port's next part. */
static void nj22(void)
{
    dca(ADDRESS_OF(EDOTVEL));
    continue_at(NJ22 + 2u);
}

/* ---- NJ22, NJ23, the regions R22 to R24 -- pp. 1031-1032 ---- */

/* NJ22 +2: the rate error over SLOPE, plus ADB+AF, plus the attitude error:
 * not negative, J23. Negative: the rate error less WLMH ("WL - H") in
 * WTEMP, and with that + (its upper word, or that +0 or -0, its lower one
 * +), J23 again; -, NJ23. */
void nj22_slope(void)
{
    dv_at(ADDRESS_OF(SLOPE));
    ad_at(ADDRESS_OF(T5TEMP));
    ad_at(ADDRESS_OF(AERRVEL));
    if (ccs_register(A_ADDRESS) != NEGATIVE) { /* CCS A */
        continue_at(J23);
        return;
    }
    dcs(ADDRESS_OF(WLMH));
    dxch(ADDRESS_OF(WTEMP));
    dca(ADDRESS_OF(EDOTVEL));
    das(ADDRESS_OF(WTEMP));
    switch (ccs_at(ADDRESS_OF(WTEMP))) {
    case POSITIVE: continue_at(J23); return;
    case NEGATIVE: nj23(); return;
    default: signck3(); return;
    }
}

/* SIGNCK3 -1: TCF NJ23. */
void tcf_nj23(void)
{
    nj23();
}

/* SIGNCK3: the lower word's sign (+0 counts as +). */
void signck3(void)
{
    enum sign sign = ccs_at(ADDRESS_OF(WTEMP) + 1u);

    if (sign == POSITIVE || sign == IS_PLUS_ZERO)
        continue_at(J23);
    else
        nj23();
}

/* NJ23: the attitude error plus ADB+AF plus WL-H/SLP: + (or +0), J24;
 * - (or -0), J22. */
static void nj23(void)
{
    ca(ADDRESS_OF(AERRVEL));
    ad_at(ADDRESS_OF(T5TEMP));
    ad(WL_H_SLP);
    switch (ccs_register(A_ADDRESS)) { /* CCS A */
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(J24); return;
    default: j22(); return;
    }
}

/* ---- The rotation commands, in KMPAC -- pp. 1032-1033 ---- */

/* J18: KMPAC = -EDOT, the change that takes the rate error to zero. */
void j18(void)
{
    dcs(ADDRESS_OF(EDOT));
    dxch(ADDRESS_OF(KMPAC));
    continue_at(JTIME);
}

/* J20: the rate on the hysteresis slope for the attitude error, ADBVEL -
 * AERR times SLOPE2 ("= 0.8 DEG/SEC/DEG"), less the rate error. */
void j20(void)
{
    cs(ADDRESS_OF(AERR));
    ad_at(ADDRESS_OF(ADBVEL));
    mp(SLOPE2);
    dxch(ADDRESS_OF(KMPAC));
    dcs(ADDRESS_OF(EDOT));
    das(ADDRESS_OF(KMPAC));
    continue_at(JTIME);
}

/* J21: by the rate error's sign (its upper word, or that +0 or -0, its
 * lower one; +0 counts as +), JP for +, JN for -. */
void j21(void)
{
    switch (ccs_at(ADDRESS_OF(EDOT))) {
    case POSITIVE: jp(); return;
    case NEGATIVE: jn(); return;
    default: signck4(); return;
    }
}

/* SIGNCK4: the lower word's sign. */
static void signck4(void)
{
    enum sign sign = ccs_at(ADDRESS_OF(EDOT) + 1u);

    if (sign == POSITIVE || sign == IS_PLUS_ZERO)
        jp();
    else
        jn();
}

/* JN: KMPAC = WL - EDOT, the change that leaves the rate error at WL (0.4
 * degrees a second). */
void jn(void)
{
    dcs(ADDRESS_OF(EDOT));
    dxch(ADDRESS_OF(KMPAC));
    dca(ADDRESS_OF(WL));
    das(ADDRESS_OF(KMPAC));
    continue_at(JTIME);
}

/* JP: KMPAC = -WL - EDOT, the change that leaves it at -WL. */
void jp(void)
{
    dcs(ADDRESS_OF(EDOT));
    dxch(ADDRESS_OF(KMPAC));
    dcs(ADDRESS_OF(WL));
    das(ADDRESS_OF(KMPAC));
    continue_at(JTIME);
}

/* J22: by the rate error's sign, JN for + and JP for -. CCS goes on at the
 * word 1, 2, 3 or 4 after it, by the sign: those words (TCF JN, TCF
 * SIGNCK5, TCF JP, and SIGNCK5 itself, for -0) start the faithful port's
 * next part, where its run stops. */
void j22(void)
{
    switch (ccs_at(ADDRESS_OF(EDOT))) {
    case POSITIVE: continue_at(J22 + 1u); return;
    case IS_PLUS_ZERO: continue_at(J22 + 2u); return;
    case NEGATIVE: continue_at(J22 + 3u); return;
    default: continue_at(SIGNCK5); return;
    }
}

/* TCF JN and TCF JP, J22's and SIGNCK5's words. */
void tcf_jn(void)
{
    continue_at(JN);
}

void tcf_jp(void)
{
    continue_at(JP);
}

/* J22 +2: TCF SIGNCK5. */
void tcf_signck5(void)
{
    signck5();
}

/* SIGNCK5: the lower word's sign: JN for + (or +0), JP for - (or -0). */
void signck5(void)
{
    enum sign sign = ccs_at(ADDRESS_OF(EDOT) + 1u);

    if (sign == POSITIVE || sign == IS_PLUS_ZERO)
        continue_at(JN);
    else
        continue_at(JP);
}

/* ---- J23, J24 -- pp. 1033-1034 ---- */

/* J23: "RESET RATE DAMPING FLAG", the axis' bit of RCSFLAGS (BIT13 less
 * SPNDX: "BIT13 FOR ROLL", BIT12 pitch, BIT11 yaw). "IS THERE TO BE A
 * FORCED FIRING ON THIS AXIS" (the axis' bits of OCT01400, OCT00060 or
 * OCT00300 in RCSFLAGS): "NO, GO TO DOJET +2 AND DO NOTHING"; "YES, GO TO
 * J18 AND FORCE A FIRING". */
void j23(void)
{
    cs(maneuver_index_read(ADDRESS_OF(BIT13), spndx()));
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    ca(maneuver_index_read(ADDRESS_OF(OCT01400), spndx()));
    mask_at(ADDRESS_OF(RCSFLAGS));
    if (A_is_zero()) { /* EXTEND; BZF DOJET +2 */
        dojet_next();
        return;
    }
    continue_at(J18);
}

/* J24: the rate on the hysteresis slope the other way, -(AERR + ADBVEL)
 * times SLOPE2, less the rate error. */
void j24(void)
{
    cs(ADDRESS_OF(AERR));
    su_at(ADDRESS_OF(ADBVEL));
    mp(SLOPE2);
    j24_kmpac();
}

/* J24 +5: DXCH KMPAC, and the rate error taken off. */
void j24_kmpac(void)
{
    dxch(ADDRESS_OF(KMPAC));
    dcs(ADDRESS_OF(EDOT));
    das(ADDRESS_OF(KMPAC));
    jtime();
}

#endif
