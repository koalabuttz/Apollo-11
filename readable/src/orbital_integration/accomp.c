/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, ACCOMP, GETRPSV, GAMCOMP, GOBAQUE and INT-ABRT
 * (LM pp. 1230-1233, CM pp. 1337-1340). The interpretive words are in the
 * listing's notation (src/interpreter/notation.h); the basic code after
 * INT-ABRT's EXIT is C.
 *
 * "AGC ROUTINE TO COMPUTE ACCELERATION COMPONENTS." ACCOMP sums, in FV,
 * the accelerations the conic leaves out. ALPHAV comes in as the
 * deviation from the conic; BETAV is the position (the deviation, scaled
 * by the central body, plus RCV), kept in VECTAB for the W-matrix when
 * DIM0FLAG is set. Out of midcourse (MIDFLAG clear) the central body's
 * difference (GAMCOMP) is all, and on to OBLATE. In midcourse LSPOS gives
 * the moon's position from the earth and the sun's (in 2D): BETAV and
 * RPQV the other body's position from the central one (the moon's, or in
 * the moon's sphere the earth's), RPSV the sun's. With DIM0FLAG set, the
 * position from the other body is kept in VECTAB too (and in the LM's
 * RQVV; not while P23 runs, MODREG minus OCT27 zero). GETRPSV adds the
 * other body's pull and the sun's (RPSV, from the moon in its sphere),
 * and goes on to OBLATE.
 *
 * GAMCOMP adds to FV a difference of two pulls of one body that are close
 * numbers, which a subtraction would lose, by a series (Encke's): with
 * ALPHAV and ALPHAM one vector (unit and length) and BETAV the other.
 * First the central body's, at the vehicle and on the conic (ALPHAV the
 * deviation); then the other body's and the sun's, on the vehicle and on
 * the central body (ALPHAV the position, BETAV the body's). The numbers
 * are normalized (the shifts in X2, S1 and S2) and the sum shifted back;
 * then the return, "RETURN IF NO OVERFLOW".
 *
 * GOBAQUE, on an overflow: the step taken back (TAU. and TET less H SR
 * 9D, KEPPREP), the deviation put into the conic (RECTIFY) and the step
 * started again (TESTLOOP, RPQFLAG set); when there was no deviation
 * (TDELTAV zero), INT-ABRT: alarm 430 and abort (POODOO). */
#include "orbital_integration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static const word accomp_words[] = {
    /* ACCOMP -- LM p. 1230 (CM p. 1337): X1 and X2 the central body; FV
     * zero; BETAV the position, into VECTAB with DIM0FLAG set (X2 the
     * step, DIFEQCNT, for the store). */
    /* ACCOMP */ ORDERS(LXA_1, LXA_2),
    ADDRESS(PBODY),
    ADDRESS(PBODY),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STOVL(FV),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-7),
    ADDRESS_PLUS_ONE(RCV),
    STORE(BETAV),
    ORDERS(BOF, XCHX_2),
    SWITCH(BOF, DIM0FLAG),
    ADDRESS_AT(ACCOMP, 022), /* (+5) */
    ADDRESS(DIFEQCNT),
    STORE_AT_X2(VECTAB, 0),
    ORDER(XCHX_2),
    ADDRESS(DIFEQCNT),
    /* The position's unit into ALPHAV and its length into ALPHAM, and
     * GAMCOMP; then BETAV as ALPHAV (the shift, X1, kept in S2). */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(ALPHAV),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE_D(36),
    STORE(ALPHAM),
    ORDER(CALL),
    ADDRESS(GAMCOMP),
    ORDERS(VLOAD, SXA_1),
    ADDRESS_PLUS_ONE(BETAV),
    ADDRESS(S2),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(BETAM),
    STORE(ALPHAM),
    /* Out of midcourse, OBLATE; otherwise the moon's and the sun's
     * positions at TET (LSPOS), X2 2 and X1 back; in the moon's sphere the
     * earth's position from the moon (VCOMP), X2 0. */
    ORDERS(BOF, DLOAD),
    SWITCH(BOF, MIDFLAG),
    ADDRESS(OBLATE),
    ADDRESS_PLUS_ONE(TET),
    ORDER(CALL),
    ADDRESS(LSPOS),
    ORDERS(AXT_2, LXA_1),
    NUMBER(2),
    ADDRESS(S2),
    ORDER(BOF),
    SWITCH(BOF, MOONFLAG),
    ADDRESS_AT(ACCOMP, 055), /* (+3) */
    ORDERS(VCOMP, AXT_2),
    NUMBER(0),
    STORE(BETAV),
    STOVL(RPQV),
    ADDRESS_PLUS_ONE_D(2),
    /* LM p. 1231 (CM p. 1338): the sun's position; with DIM0FLAG set
     * (and in the LM P23 not running), the position from the other body,
     * ALPHAV times ALPHAM, less BETAV, into VECTAB +6. */
    STORE(RPSV),
#if LUMINARY099
    ORDERS(SLOAD, DSU),
    ADDRESS_PLUS_ONE(MODREG),
    ADDRESS_PLUS_ONE(OCT27),
    ORDERS(BHIZ, BOF),
    ADDRESS_AT(ACCOMP, 070), /* (+3) */
    SWITCH(BOF, DIM0FLAG),
    ADDRESS(GETRPSV),
    ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(ALPHAV),
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(VSR_STAR, VSU),
    SR_X2(1),
    ADDRESS_PLUS_ONE(BETAV),
    ORDER(XCHX_2),
    ADDRESS(DIFEQCNT),
    STORE_AT_X2(VECTAB, 6),
    STORE(RQVV),
    ORDER(XCHX_2),
    ADDRESS(DIFEQCNT),
#else
    ORDERS(BOF, VLOAD),
    SWITCH(BOF, DIM0FLAG),
    ADDRESS(GETRPSV),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(VXSC, VSR_STAR),
    ADDRESS_PLUS_ONE(ALPHAM),
    SR_X2(1),
    ORDERS(VSU, XCHX_2),
    ADDRESS_PLUS_ONE(BETAV),
    ADDRESS(DIFEQCNT),
    STORE_AT_X2(VECTAB, 6),
    ORDER(XCHX_2),
    ADDRESS(DIFEQCNT),
#endif
    /* GETRPSV: X1 4 on; the other body's pull (GAMCOMP), RPQFLAG cleared;
     * in the moon's sphere RPSV the sun's position from the moon (RPQV
     * SR 9D, plus RPSV). Then the sun's pull, X2 4 and X1 4 on, and
     * OBLATE. */
    /* GETRPSV */ ORDERS(VLOAD, INCR_1),
    ADDRESS_PLUS_ONE(RPQV),
    NUMBER(4),
    ORDERS(CLEAR, BOF),
    SWITCH(CLEAR, RPQFLAG),
    SWITCH(BOF, MOONFLAG),
    ADDRESS_AT(GETRPSV, 013), /* (+5) */
    ORDERS(VSR, VAD),
    SR_COUNT(9),
    ADDRESS_PLUS_ONE(RPSV),
    STORE(RPSV),
    ORDER(CALL),
    ADDRESS(GAMCOMP),
    ORDERS(AXT_2, INCR_1),
    NUMBER(4),
    NUMBER(4),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RPSV),
    STCALL(BETAV),
    ADDRESS(GAMCOMP),
    ORDER(GOTO),
    ADDRESS(OBLATE),
    /* GAMCOMP: BETAV squared, normalized (the shift in 31D); ALPHAM
     * normalized (in 32D); BETAV's unit and length (BETAM); their ratio
     * normalized (in 33D); the scale (ASCALE,1) and the shifts summed into
     * X2 and S1. */
    /* GAMCOMP */ ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE(BETAV),
    ORDERS(VSQ, SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(NORM, ROUND),
    ADDRESS_PLUS_ONE_D(31),
    ORDERS(PDDL, NORM),       /* "NORMED B SQUARED TO PD LIST" */
    ADDRESS_PLUS_ONE(ALPHAM), /* "NORMALIZE (LESS ONE) LENGTH OF ALPHA" */
    ADDRESS_PLUS_ONE_D(32),   /* "SAVING NORM SCALE FACTOR IN X1" */
    ORDERS(SR1, PDVL),
    ADDRESS_PLUS_ONE(BETAV), /* "C(PDL+2) = ALMOST NORMED ALPHA" */
    ORDER(UNIT),
    STODL(BETAV),
    ADDRESS_PLUS_ONE_D(36),
    STORE(BETAM),
    ORDERS(NORM, BDDV),
    ADDRESS_PLUS_ONE_D(33),
    ORDERS(SR1R, PUSH),
    ORDER(DLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(ASCALE, 0),
    STORE(S1),
    ORDERS(XCHX_2, XAD_2),
    ADDRESS(S1),
    ADDRESS_D(32),
    ORDERS(XSU_2, DLOAD),
    ADDRESS_D(33),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SR_STAR, XCHX_2),
    SR_X2(-1),
    ADDRESS(S1),
    /* The series, on the push-down list. */
    ORDERS(PUSH, SR1R), /* "RHO/4 TO 4D" */
    ORDERS(PDVL, DOT),
    ADDRESS_PLUS_ONE(ALPHAV),
    ADDRESS_PLUS_ONE(BETAV),
    ORDERS(SL1R, BDSU), /* "(RHO/4) - 2(ALPHAV/2.BETAV/2)" */
    ORDERS(PUSH, DMPR), /* "TO PDL+6" */
    ADDRESS_PLUS_ONE_D(4),
    ORDER(SL1),
    ORDERS(PUSH, DAD),
    ADDRESS_PLUS_ONE(DQUARTER),
    ORDERS(PUSH, SQRT),
    ORDERS(DMPR, PUSH),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(SL1, DAD),
    ADDRESS_PLUS_ONE(DQUARTER),
    ORDERS(PDDL, DAD),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS_PLUS_ONE(HALFDP),
    ORDERS(DMPR, SL1),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(DAD, DDV),
    ADDRESS_PLUS_ONE(THREE_8),
    ADDRESS_PLUS_ONE_D(14),
    ORDERS(DMPR, VXSC),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE(BETAV),
    ORDERS(PDVL, VSR3),
    /* LM p. 1233 (CM p. 1340): the sum times MU over the normalized
     * lengths, shifted back by the shifts (S1, S2, 30D, 31D: the overflow
     * cleared first), into FV. */
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(VAD, PUSH), /* "A12 + C(PD+16D) TO PD+16D" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(NORM, ROUND),
    ADDRESS_PLUS_ONE_D(30),
    ORDERS(BDDV, DMP_STAR),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    ORDERS(DCOMP, VXSC),
    ORDERS(XCHX_2, XAD_2),
    ADDRESS(S1),
    ADDRESS(S2),
    ORDERS(XSU_2, XSU_2),
    ADDRESS_D(30),
    ADDRESS_D(31),
    ORDER(BOV),                /* "CLEAR OVIND" */
    ADDRESS_AT(GAMCOMP, 0114), /* (+1) */
    ORDERS(VSR_STAR, XCHX_2),
    SR_X2(-1),
    ADDRESS(S1),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(FV),
    STORE(FV),
    ORDERS(BOV, RVQ), /* "RETURN IF NO OVERFLOW" */
    ADDRESS(GOBAQUE),
    /* GOBAQUE, the +1 of the BOV: with TDELTAV zero, INT-ABRT; otherwise
     * the step (H SR 9D) back off TAU. and TET (KEPPREP), RECTIFY, and
     * the step again. */
    /* GOBAQUE */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(TDELTAV),
    ORDER(BZE),
    ADDRESS(INT_ABRT),
    ORDERS(DLOAD, SR),
    ADDRESS_PLUS_ONE(H),
    SR_COUNT(9),
    ORDERS(PUSH, BDSU),
    ADDRESS_PLUS_ONE(TC),
    STODL(TAU_),
    ADDRESS_PLUS_ONE(TET),
    ORDERS(DSU, STADR),
    AFTER_STADR(STCALL(TET)),
    ADDRESS(KEPPREP),
    ORDER(CALL),
    ADDRESS(RECTIFY),
    ORDER(SETGO),
    SWITCH(SETGO, RPQFLAG),
    ADDRESS(TESTLOOP),
    /* INT-ABRT: out of the interpreter. */
    /* INT-ABRT */ ORDER(EXIT),
};

const struct interpretive_code ACCOMP_CODE = {CODE_BANK(I_ACCOMP), CODE_Z(I_ACCOMP),
                                              COUNT(accomp_words), accomp_words};

/* After INT-ABRT's EXIT: POODOO, which reads the alarm code (OCT 00430)
 * in the word after the TC. */
void int_abrt_after_exit(void)
{
    tc_at((word)(INT_ABRT + 1u)); /* TC POODOO */
    continue_at(POODOO);
}
