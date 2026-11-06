/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 4 (Comanche055 pp. 756-759): R51, the fine
 * alignment (bank 14), and R55, the gyro torque (bank 15).
 *
 * R51, "TO ALIGN THE STABLE MEMBER TO REFSSMAT": two stars are sighted
 * (BESTI and BESTJ, the pair PICAPAR picked, STARIND 1 the first and 0
 * the second). For each, the crew sees its code (V01N70) and may key in
 * another; R52 points the optics at it (in P54, MODREG 54, R56 marks it
 * instead) and SXTSM turns the marks into its direction in the present
 * stable member axes. PLANET gives its vector at the time (a planet
 * moves), which REFSMMAT turns to the desired axes. With both, R54 tests
 * the pair ("STAR DATA TEST") and, the crew accepting it, AXISGEN works
 * out the desired axes in the present ones and R55 torques the gyros to
 * them. Then V50N25, option 14: the crew aligns again (P52C) or ends
 * (ENDP50S).
 *
 * R55, "GYRO TORQUE": the gyro torque angles (CALCGTA), flashed (V06N93)
 * for the crew to accept; IMUPULSE sends them.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, BLANKET, FALTON,
 * CURTAINS) leaves Z at the routine and Q at the word after the TC,
 * where the routine comes back to; GOTOPOOH doesn't come back. The
 * faithful port keys a routine at each such return, at each TC's return
 * in the file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r51dsp(void);
static void r51_4(void);
static void r51b(void);

/* ---- R51 -- pp. 756-758 (bank 14) ---- */

static const word r51_words[] = {
    /* R51 */ ORDER(EXIT),
};

const struct interpretive_code R51_CODE = {014u, R51, COUNT(r51_words), r51_words};

/* R51, after the EXIT: STARIND and MARKINDX 1, the first star. (A is
 * BIT1: TS does not skip.) */
void r51_after_exit(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(STARIND));
    ts(ADDRESS_OF(MARKINDX));
    tc_at(R51_2);
    intpret();
}

/* R51.3, each star: TARG1FLG and TARG2FLG cleared; then a restart point
 * (PHASCHNG, 05024: "RESTART GR 4 FOR R52-R53"). */
static const word r51_3_words[] = {
    /* R51.3 */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, TARG2FLG),
    SWITCH(CLEAR, TARG1FLG),
    ORDER(EXIT),
};

const struct interpretive_code R51_3_CODE = {014u, R51_3, COUNT(r51_3_words), r51_3_words};

void r51_3_after_exit(void)
{
    tc_to(R51_3 + 4u, PHASCHNG); /* OCT 05024, OCT 13000 */
}

/* The star's code: BESTI (by STARIND) holds the star's place in the
 * catalog, its code times 6; times 1/6TH, the code, to STARCODE. (MP
 * leaves A small and positive: TS does not skip.) Then R51DSP. */
void r51_3_restart(void)
{
    ca(indexed_ca(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    mp(ONE_6TH);                                                   /* EXTEND; MP 1/6TH */
    ts(ADDRESS_OF(STARCODE));
    r51dsp();
}

/* R51DSP: the code flashed (V01N70, GOFLASHR); the job goes on at once
 * (+3): R2 and R3 blanked (BLANKET, SIX), and ENDOFJOB. The crew answers
 * later: terminate, proceed (CHKSCODE checks the code) or enter (flash it
 * again, with a code keyed in). */
static void r51dsp(void)
{
    set_A(V01N70);
    tc_to(R51DSP + 1u, BANKCALL); /* CADR GOFLASHR */
}

void r51dsp_terminate(void)
{
    tc_to(R51DSP + 3u, GOTOPOOH);
}

void r51dsp_proceed(void)
{
    tc_at(R51DSP + 4u);                 /* TC +5 */
    tc_at(R51DSP + 011u);               /* TC CHKSCODE */
    if (!chkscode()) r51dsp_bad_code(); /* (its +2 is R51DSP +014) */
}

void r51dsp_enter(void)
{
    tc_at(R51DSP + 5u); /* TC -5 */
    r51dsp();
}

void r51dsp_displayed(void)
{
    set_A(SIX);
    tc_to(R51DSP + 7u, BLANKET);
}

void r51dsp_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* CHKSCODE's return (+0): a code over 50 or below 0. The operator error
 * light (FALTON), and the code flashed again. */
void r51dsp_bad_code(void)
{
    tc_to(R51DSP + 012u, FALTON);
}

void r51dsp_after_falton(void)
{
    tc_at(R51DSP + 013u); /* TC R51DSP */
    r51dsp();
}

/* CHKSCODE's return (+2): a code 0 to 50 (0 a planet; 46 to 50 the sun,
 * the earth and the moon). */
void r51dsp_code_good(void)
{
    tc_at(R51DSP + 014u);
    intpret();
}

/* The star's vector now (LOADTIME, PLANET); S1 0, X1 STARIND: TIX,1
 * takes the first star (1) to R51ST, STARSAV1, the second (0) to
 * STARSAV2. */
static const word r51_star_words[] = {
    /* R51DSP +015 */ ORDERS(RTB, CALL),
    ADDRESS(LOADTIME),
    ADDRESS(PLANET),
    ORDERS(SSP, LXA_1),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(0),
    ADDRESS(STARIND),
    ORDER(TIX_1),
    ADDRESS(R51ST),
    STCALL(STARSAV2), /* "2ND STAR" */
    ADDRESS_AT(R51ST, 1),
    /* R51ST */ STORE(STARSAV1), /* "1ST STAR" */
    ORDER(EXIT),
};

const struct interpretive_code R51_STAR_CODE = {014u, R51DSP + 015u, COUNT(r51_star_words),
                                                r51_star_words};

/* R51ST, after the EXIT: "IS THIS P54" (MODREG 54, the sum -0)? R56
 * marks the star then (R51B); else R52 points the optics at it ("AOP
 * WILL MAKE CALLS TO SIGHTING"). */
void r51st_after_exit(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(OCT66);
    if (A_is_zero()) { /* EXTEND; BZF R51B */
        r51b();
        return;
    }
    tc_at(R51ST + 6u);
    intpret();
}

/* R51A: the marks to the star's direction in stable member axes (SXTSM,
 * "COMPUTE LOS IN SM FROM MARK DATA"), to STARSAV2; then the marks
 * released (MKRELEAS). */
static const word r51_mark_words[] = {
    /* R51ST +7 */ ORDER(CALL), ADDRESS(R52),
    /* R51A */ ORDER(CALL),     ADDRESS(SXTSM), STORE(STARSAV2), ORDER(EXIT),
};

const struct interpretive_code R51_MARK_CODE = {014u, R51ST + 7u, COUNT(r51_mark_words),
                                                r51_mark_words};

void r51a_after_exit(void)
{
    tc_to(R51A + 4u, BANKCALL); /* CADR MKRELEAS */
}

void r51a_after_mkreleas(void)
{
    tc_at(R51A + 6u);
    intpret();
}

/* The star's vector again, at the time of the marks (TSIGHT): a
 * planet's has moved. */
static const word r51a_planet_words[] = {
    /* R51A +7 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    ORDER(EXIT),
};

const struct interpretive_code R51A_PLANET_CODE = {014u, R51A + 7u, COUNT(r51a_planet_words),
                                                   r51a_planet_words};

/* STARIND 1, the first star: R51.4 keeps it and sights the second. +0,
 * both sighted. (STARIND is never negative here: the words for it are
 * interpretive code.) */
void r51a_sighted(void)
{
    switch (ccs_at(ADDRESS_OF(STARIND))) {
    case POSITIVE:
        tc_at(R51A + 014u); /* TC R51.4 */
        r51_4();
        break;
    case IS_PLUS_ZERO: r51a_both_sighted(); break;
    default: not_modelled();
    }
}

void r51a_both_sighted(void)
{
    tc_at(R51A + 015u);
    intpret();
}

/* For R54: the second star's vector in the desired axes (by REFSMMAT)
 * to STARAD, the first's (PLANVEC) to STARAD +6; the marked directions,
 * the second's (STARSAV2) to 6D and the first's (STARSAV1) to 12D. With
 * FREEFLAG set after it (the crew accepts the pair), AXISGEN and R55
 * torque the gyros, and PFRATFLG is cleared. */
static const word r51_torque_words[] = {
    /* R51A +016 */ ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(STARAD),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(STARSAV2),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(STARSAV1),
    STOVL_D(12),
    ADDRESS_PLUS_ONE(PLANVEC),
    STCALL_AT(STARAD, 6),
    ADDRESS(R54), /* "STAR DATA TEST" */
    ORDERS(BOFF, CALL),
    SWITCH(BOFF, FREEFLAG),
    ADDRESS(R51K),
    ADDRESS(AXISGEN),
    ORDER(CALL),
    ADDRESS(R55), /* "GYRO TORQUE" */
    ORDER(CLEAR),
    SWITCH(CLEAR, PFRATFLG),
    /* R51K */ ORDER(EXIT),
};

const struct interpretive_code R51_TORQUE_CODE = {014u, R51A + 016u, COUNT(r51_torque_words),
                                                  r51_torque_words};

/* R51K: V50N25, option 14 (GOPERF1): terminate, proceed ("V33": align
 * again, P52C) or enter (on to ENDP50S). */
void r51k_after_exit(void)
{
    set_A(OCT14);
    tc_to(R51K + 2u, BANKCALL); /* CADR GOPERF1 */
}

void r51k_terminate(void)
{
    tc_to(R51K + 4u, GOTOPOOH);
}

void r51k_proceed(void)
{
    tc_at(R51K + 5u); /* TC +2 */
    r51k_again();
}

void r51k_enter(void)
{
    tc_at(R51K + 6u); /* TC +3 */
    r51k_done();
}

void r51k_again(void)
{
    tc_to(R51K + 7u, BANKCALL); /* CADR P52C */
}

void r51k_done(void)
{
    tc_at(R51K + 011u);
    intpret();
}

static const word r51k_done_words[] = {
    /* R51K +012 */ ORDER(GOTO),
    ADDRESS(ENDP50S),
};

const struct interpretive_code R51K_DONE_CODE = {014u, R51K + 012u, COUNT(r51k_done_words),
                                                 r51k_done_words};

/* R51.4: the first star's vector in the desired axes (by REFSMMAT) to
 * PLANVEC, its marked direction (STARSAV2) to STARSAV1; STARIND 0, and
 * the second star (R51.3). */
static void r51_4(void)
{
    tc_at(R51_4);
    intpret();
}

static const word r51_4_words[] = {
    /* R51.4 +1 */ ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(PLANVEC),
    ADDRESS_PLUS_ONE(STARSAV2),
    STORE(STARSAV1),
    ORDER(SSP),
    ADDRESS_PLUS_ONE(STARIND),
    NUMBER(0),
    ORDER(GOTO),
    ADDRESS(R51_3),
};

const struct interpretive_code R51_4_CODE = {014u, R51_4 + 1u, COUNT(r51_4_words), r51_4_words};

/* R51B, in P54: R56 marks the star; then R51A. */
static void r51b(void)
{
    tc_at(R51B);
    intpret();
}

static const word r51b_words[] = {
    /* R51B +1 */ ORDER(CALL),
    ADDRESS(R56),
    ORDER(GOTO),
    ADDRESS(R51A),
};

const struct interpretive_code R51B_CODE = {014u, R51B + 1u, COUNT(r51b_words), r51b_words};

/* ---- CHKSCODE -- p. 773 (bank 14) ---- */

/* CHKSCODE ("KEEP IN SAME BANK AS R51 AND R53"): STARCODE 0 to 50 (-0
 * too) goes on at the caller's +2 (INDEX Q; TC 00002: Q is the caller's
 * return), and true; any other code, false: TC Q, back to the caller's
 * +0 (Z, with Q 3), whose words the caller runs. */
bool chkscode(void)
{
    bool good;
    word to;

    switch (ccs_at(ADDRESS_OF(STARCODE))) {
    case POSITIVE:
        ad(NEG47); /* -0 at code 50 */
        good = ccs_register(A_ADDRESS) != POSITIVE;
        break;
    case NEGATIVE: good = false; break;
    default: good = true; /* +0 (CCS A: +0 again) or -0 */
    }
    if (!good) {
        tc_through(Q_ADDRESS); /* TC Q */
        return false;
    }
    to = indexed_instruction(00002u, Q); /* INDEX Q; TC 00002 */
    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_at(CHKSCODE + 7u);
    continue_at(to);
    return true;
}

/* ---- R55 -- p. 759 (bank 15) ---- */

/* R55: the caller's return to QMIN, and the gyro torque angles (CALCGTA,
 * from XDC, YDC and ZDC, "REFSMMAT WRT PRESENT STABLE MEMBER"). */
static const word r55_words[] = {
    /* R55 */ ORDER(STQ),     ADDRESS(QMIN), ORDER(CALL), ADDRESS(CALCGTA),
    /* PULSEM */ ORDER(EXIT),
};

const struct interpretive_code R55_CODE = {015u, R55, COUNT(r55_words), r55_words};

/* R55.1: the angles flashed (V06N93): terminate, proceed (torque, R55.2)
 * or enter (no torque, R55RET). */
void r55_1(void)
{
    set_A(V06N93);
    tc_to(R55_1 + 1u, BANKCALL); /* CADR GOFLASH */
}

void r55_terminate(void)
{
    tc_to(R55_1 + 3u, GOTOPOOH);
}

void r55_proceed(void)
{
    tc_at(R55_1 + 4u); /* TC R55.2 */
    r55_2();
}

void r55_enter(void)
{
    tc_at(R55_1 + 5u); /* TC R55RET */
    r55ret();
}

/* R55.2: a restart point (PHASCHNG, 00314); then IMUPULSE torques the
 * gyros (OGC, R55CDR) and IMUSTALL waits (CURTAINS when it fails); then
 * a restart point again (05024). */
void r55_2(void)
{
    tc_to(R55_2, PHASCHNG); /* OCT 00314 */
}

void r55_torque(void)
{
    set_A(R55CDR);
    tc_to(R55_2 + 3u, BANKCALL); /* CADR IMUPULSE */
}

void r55_after_imupulse(void)
{
    tc_to(R55_2 + 5u, BANKCALL); /* CADR IMUSTALL */
}

void r55_failed(void)
{
    tc_to(R55_2 + 7u, CURTAINS);
}

void r55_torqued(void)
{
    tc_to(R55_2 + 010u, PHASCHNG); /* OCT 05024, OCT 13000 */
}

/* R55RET: back to the caller (QMIN). */
void r55ret(void)
{
    tc_at(R55RET);
    intpret();
}

static const word r55ret_words[] = {
    /* R55RET +1 */ ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code R55RET_CODE = {015u, R55RET + 1u, COUNT(r55ret_words), r55ret_words};

#endif
