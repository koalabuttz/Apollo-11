/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 805-807 (Luminary099 only):
 * "COMPUTE STATE IN GUIDANCE COORDINATES" (CALCRGVG, RGVGCALC), and
 * BRSPOT3, on to the phase's guidance. The faithful C is
 * c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* CALCRGVG: "IN IGNALG, COMPUTE V FROM INTEGRATION OUTPUT AND TRIM
 * CORRECTION TERM COMPUTED LAST PASS AND LEFT IN UNFC/2": V is VATT1 in
 * stable member coordinates (REFSMMAT) plus UNFC/2. The EXIT runs on
 * into RGVGCALC. */
void calcrgvg(void)
{
    tc_at(CALCRGVG);
    intpret();
}

static const word calcrgvg_words[] = {
    /* CALCRGVG +1 */ ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(VATT1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VSR1, VAD),
    ADDRESS_PLUS_ONE(UNFC_2),
    STORE(V),
    ORDER(EXIT),
};

const struct interpretive_code CALCRGVG_CODE = {031u, CALCRGVG + 1u, COUNT(calcrgvg_words),
                                                calcrgvg_words};

/* RGVGCALC: "ENTER HERE TO RECOMPUTE RG AND VG". As the listing has it,
 *
 *     ANGTERM = V + R * WM
 *     RGU = CG (R - LAND)
 *     VGU = CG (V - WM * R)
 *     VHORIZ = 8 ABVAL (0, VG , VG )
 *                            2    1
 *     LOOKANGL = ARCSIN(UNIT(R - LAND).XMBPIP)
 *
 * the velocity relative to the surface, the state in guidance
 * coordinates (CG), the horizontal velocity "FOR DISPLAY DURING P65", and
 * the range (RANGEDSP). The interpretive code leaves the sine of the
 * depression angle in MPAC, for LOOKANGL ("NOW IN MPAC IS
 * SINE(LOOKANGL)/4"); the program takes it with XNBPIP. */
void rgvgcalc(void)
{
    tc_at(RGVGCALC);
    intpret();
}

static const word rgvgcalc_words[] = {
    /* RGVGCALC +1 */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(R),
    ADDRESS_PLUS_ONE(WM),
    ORDERS(VAD, VSR2), /* "RESCALE TO UNITS OF 2(9) M/CS" */
    ADDRESS_PLUS_ONE(V),
    STORE(ANGTERM),
    ORDER(MXV),
    ADDRESS_PLUS_ONE(CG), /* "NO SHIFT SINCE ANGTERM IS DOUBLE SIZED" */
    STORE(VGU),
    ORDERS(PDDL, VDEF),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDERS(ABVAL, SL3),
    STOVL(VHORIZ), /* "VHORIZ FOR DISPLAY DURING P65." */
    ADDRESS_PLUS_ONE(R),
    ORDERS(VSU, PUSH), /* "PUSH DOWN R - LAND" */
    ADDRESS_PLUS_ONE(LAND),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(CG),
    STORE(RGU),
    ORDER(ABVAL),
    STOVL(RANGEDSP),
    ORDERS(RTB, DOT), /* "NOW IN MPAC IS SINE(LOOKANGL)/4" */
    ADDRESS(NORMUNIT),
    ADDRESS_PLUS_ONE(XNBPIP),
    ORDER(EXIT),
};

const struct interpretive_code RGVGCALC_CODE = {031u, RGVGCALC + 1u, COUNT(rgvgcalc_words),
                                                rgvgcalc_words};

/* After the EXIT: "RESET PUSH DOWN POINTER", and "COMPUTE LOOKANGLE
 * ITSELF": the arcsine (SPARCSIN, through BANKCALL) of twice MPAC's high
 * word. (A is FIXLOC: TS PUSHLOC never skips.) */
void rgvgcalc_after_exit(void)
{
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(PUSHLOC)); /* "RESET PUSH DOWN POINTER" */
    ca(ADDRESS_OF(MPAC));
    double_A();
    tc_to(RGVGCALC + 036u, BANKCALL); /* CADR SPARCSIN -1 */
}

/* SPARCSIN's return: LOOKANGL, rounded (1/2DEG) and in degrees (times
 * 180DEGS), "FOR DISPLAY DURING P64"; then BRSPOT3. (MP leaves no
 * overflow: TS LOOKANGL never skips.) */
void rgvgcalc_after_sparcsin(void)
{
    ad(HALF_DEG);
    mp(ONE_EIGHTY_DEGS);
    ts(ADDRESS_OF(LOOKANGL));
    brspot3();
}

/* BRSPOT3: the phase's WHATGUID word. */
void brspot3(void)
{
    tcf_by_phase(WHATGUID, ADDRESS_OF(WCHPHASE));
}

#endif
