/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part C (Comanche055 p. 1076, bank 15):
 * "DEAD ZONE LOGIC USED BY ENTRY DIGITAL AUTOPILOTS."
 *
 * Each test takes an error or a rate (over 180 degrees) in A and leaves a
 * tag for the jet code tables: +0 inside the dead zone, +1 or -1 outside
 * it, by its sign ("GENERATE TAG, SET C(A)= -+1 OUTSIDE DZ", "SET C(A) =
 * +0 INSIDE"); back by TC Q. The limits are 1 less the dead zone, so that
 * a value past the zone overflows:
 *   3DDZ, YAWLIM: 3.6 degrees ("YAWLIM=1.0-3.6/180=16384-329=16055");
 *   2D/SDZ, YDOTLIM: "YDOT DZ = 2 DEG/SEC";
 *   BIASEDZ, for the extra atmospheric DAP: the error, biased by CM/BIAS
 *     (.6 degrees) against the rate, through 3DDZ; then 4D/SDZ, a rate of
 *     4 degrees a second or more (4D/SLIM) overriding the error's tag; then
 *     the rate's own through 2D/SDZ. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- The jet code tables -- p. 1080 ---- */

/* The fixed words the file reads under INDEX: its tables, YJETCODE and
 * P/RJCODE, each with the word either side, and ANGOVCOR's LIMITS with
 * the word before it and two after. The faithful port keeps copies of
 * these words only, and reads no others under INDEX. */
static bool table_word(unsigned address)
{
    return (address + 1u >= YJETCODE && address <= YJETCODE + 1u) ||
           (address + 1u >= P_RJCODE && address <= P_RJCODE + 1u) ||
           (address + 1u >= LIMITS && address <= LIMITS + 2u);
}

/* INDEX A; CA (or CAF) base: the word A words on from base (A -1, +0 or
 * +1 for a tag). An index that lands in fixed memory outside the tables
 * is not modelled; one in erasable memory reads it, but not Z. */
void index_a_ca(word base)
{
    int at = (int)base + value_of(a_word());

    if (at < 0 || at > 07777 || at == (int)Z_ADDRESS) not_modelled();
    if (at < 02000) {
        ca((unsigned)at);
        return;
    }
    if (!table_word((unsigned)at)) not_modelled();
    set_A(fetch_fixed((word)at));
}

/* INDEX A; CAF table: the jet code for A's tag. */
void jet_code(word table)
{
    index_a_ca(table);
}

/* ---- The tags' ends ---- */

/* DZNOCOM: the tag to JETEM +1, and +0 in A for the caller (an overflow,
 * outside the zone, skips the CA ZERO: A +1 or -1); back by TC Q. */
static void dznocom(void)
{
    if (!ts(ADDRESS_OF(JETEM) + 1u)) set_A(ZERO); /* CA ZERO */
    tc_q();
}

/* DZCOM: the tag's sign turned, then DZNOCOM. */
static void dzcom(void)
{
    com();
    dznocom();
}

/* ---- 3DDZ -- p. 1076 ---- */

/* 3DDZ: CCS A, then YAWLIM added to the diminished error (its comment
 * reads "YAWLIM=1.0-3/180=16384-273=16111"; the constant is 16055);
 * positive or +0 on at DZCOM, negative or -0 at DZNOCOM. */
void three_d_dz(void)
{
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        ad(YAWLIM);
        dzcom();
        break;
    case IS_PLUS_ZERO: dzcom(); break;
    case NEGATIVE:
        ad(YAWLIM);
        dznocom();
        break;
    case IS_MINUS_ZERO: dznocom(); break;
    }
}

/* ---- BIASEDZ -- p. 1076 ---- */

/* BIASEDZ: "BIASED DZ FOR EXT ATM DAP.": "SAVE RATE/180.  ERROR/180 IS IN
 * L." (JETEM2); "START ERROR DZ.": the error biased against the rate (CS
 * CM/BIAS for a positive rate, CA CM/BIAS for a negative one, +0 for
 * either zero), "BIAS THE ERROR."; "SAVE CALLERS RETURN ADDRESS." (LXCH
 * Q) and "GO GENERATE THE ERROR BIT." (3DDZ, back at BIASEDZ +010). (An
 * overflow at TS JETEM2 skips the CCS: CS CM/BIAS.) */
void biasedz(void)
{
    enum sign rate = POSITIVE;

    if (!ts(ADDRESS_OF(JETEM2))) rate = ccs_register(A_ADDRESS);
    if (rate == POSITIVE) set_A(negate(CM_BIAS)); /* CS */
    if (rate == NEGATIVE) set_A(CM_BIAS);         /* CA */
    ad_at(L_ADDRESS);
    lxch(Q_ADDRESS);
    tc_at(BIASEDZ + 7u);
    three_d_dz();
}

/* BIASEDZ +010: "BIT TO L, RESTORE CALLERS Q." (DXCH L). 4D/SDZ: "CAME
 * HERE IN EXT ATM. C(L) = ERROR BIT": the rate's magnitude, less 1
 * (CCS), with 4D/SLIM added; "IF RATE GEQ 4D/S, SET L=0 AND TAKE" "JET
 * BITS ACCORDING TO SGN OF RATE." (TS A skips on the overflow: ZL), else
 * "RATE OK. CONTINUE"; the rate back in A (XCH JETEM2) and on to 2D/SDZ
 * "TO GET SIGN.". */
void biasedz_after_3ddz(void)
{
    dxch(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(JETEM2))) {
    case POSITIVE:
    case NEGATIVE: ad(FOUR_D_SLIM); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    if (ts(A_ADDRESS)) zl(); /* "RATE GEQ 4 D/S.  OVER RIDE ERROR BIT" */
    xch(ADDRESS_OF(JETEM2));
    two_d_sdz();
}

/* ---- 2D/SDZ -- p. 1076 ---- */

/* 2D/SDZ: "COME HERE TO TEST IF A WITHIN 2DEG/S DZ": CCS A, then YDOTLIM,
 * "1.0 - YDOT DZ", added; positive or +0 on at DZNOCOM, negative or -0
 * at DZCOM. */
void two_d_sdz(void)
{
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        ad(YDOTLIM);
        dznocom();
        break;
    case IS_PLUS_ZERO: dznocom(); break;
    case NEGATIVE:
        ad(YDOTLIM);
        dzcom();
        break;
    case IS_MINUS_ZERO: dzcom(); break;
    }
}

#endif
