/* SPDX-License-Identifier: MIT */
/* AUTOMATIC_MANEUVERS.agc (Comanche055 pp. 1029-1031, bank 21): "JET
 * SWITCHING LOGIC AND CALCULATION OF REQUIRED ROTATION COMMANDS", the
 * start. For each axis in turn (JLOOP, SPNDX 2, 1, 0), the rate error EDOT
 * (the body rate ADOT less the commanded rate WBODY, except in attitude
 * hold) and the attitude error AERR are placed in the phase plane of the
 * listing's FIG. 1, with the dead zone ADB widened by the flat region AF:
 * the rate error made positive, the signs of the others turned with it
 * (POSVEL, NEGVEL), and J6., J7 and J8 find the region (R18 to R24) whose
 * code (J18 to J23) works out the rotation command. The faithful C is
 * c64-port/src/cm_auto_man.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void j6(void);
static void j7(void);
static void j8(void);
static void signck2(void);
static void nj22(void);

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

/* NJ22: the rate error, on at NJ22 +2 (EXTEND; DV SLOPE). */
static void nj22(void)
{
    dca(ADDRESS_OF(EDOTVEL));
    continue_at(NJ22 + 2u);
}

#endif
