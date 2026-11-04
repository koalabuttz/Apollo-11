/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 3 (Comanche055 pp. 699-702, bank 24): UPDATEVG and
 * STEERING, SERVICER's exit during the burn. The faithful C is
 * c64-port/src/cm_p40_p47.c.
 *
 * Each pass of SERVICER (every two seconds) ends in STEERING, which
 * calls UPDATEVG: the velocity counted since the burn began (DELVSUM)
 * grows by this pass's (DELVREF), and S40.8 steers by the cross product
 * of the velocity to be gained and its change. Every so many passes
 * (NBRCYCLS) a Lambert burn starts S40.9 for a new velocity to be gained
 * (SETUP.9: VGPREV extrapolated by BDT, the change of the required
 * velocity), from the state vector at this pass. A burn short enough to
 * be timed (IMPULSW set) sets up ENGINOFF once the engine is on: TGO the
 * time from now to TIG, at least a centisecond.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC; a TCF out of it leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* The words of UPDATEVG and STEERING. */
#define UPDATEVG_EXIT (UPDATEVG + 013u)
#define STEERING_EXIT (STEERING + 3u)

/* ---- UPDATEVG -- pp. 699-701 ---- */

/* UPDATEVG_CODE: the return kept in QTEMP1 (STQ). With XDELVFLG set (an
 * external delta V burn), on at CALL40.8. Otherwise, NBRCYCLS negative
 * (the first pass), on at SETUP.9; else DELVSUMP, the velocity counted
 * (DELVSUM plus DELVREF); to basic code. */
static const word updatevg_words[] = {
    ORDERS(STQ, BON),          ADDRESS(QTEMP1),    SWITCH(BON, XDELVFLG),
    ADDRESS(CALL40_8),         ORDERS(SLOAD, BMN), ADDRESS_PLUS_ONE(NBRCYCLS),
    ADDRESS(SETUP_9),          ORDERS(VLOAD, VAD), ADDRESS_PLUS_ONE(DELVSUM),
    ADDRESS_PLUS_ONE(DELVREF), STORE(DELVSUMP),    ORDER(EXIT),
};

/* After the EXIT: NBRCYCLP, the passes counted, NBRCYCLS plus one (an
 * overflow would skip into the OCT after the PHASCHNG, data: not
 * modelled); restart group 1 (PHASCHNG, OCT 10035, "TYPE B RESTART
 * RESTART BELOW AND 5.3 REREADACCS"). */
void updatevg_after_exit(void)
{
    set_A(ONE);
    ad_at(ADDRESS_OF(NBRCYCLS));
    if (ts(ADDRESS_OF(NBRCYCLP))) not_modelled();
    tc_to(UPDATEVG_EXIT + 4u, PHASCHNG);
}

/* UPDATEVG_EXIT +6: the copy cycle, NBRCYCLP to NBRCYCLS; to the
 * interpreter, for CALL40_8_CODE. */
void updatevg_after_phaschng(void)
{
    ca(ADDRESS_OF(NBRCYCLP));
    ts(ADDRESS_OF(NBRCYCLS));
    tc_at(UPDATEVG_EXIT + 010u);
    intpret();
}

/* CALL40_8_CODE: DELVSUMP to DELVSUM. CALL40.8: S40.8, the cross-product
 * steering, and back through QTEMP1. SETUP.9: on the first pass
 * (FIRSTFLG set), straight to SURELY.9. Otherwise VGPREV, the velocity to
 * be gained, carried on to this pass: BDT times the passes counted
 * (NORM'd, the shift kept in X1, "NORM HANDLES ZERO PROPERLY"), shifted
 * back, plus VGTEMP, less DELVSUM. SURELY.9: to basic code. */
static const word call40_8_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(DELVSUMP),
    STORE(DELVSUM),
    /* CALL40.8 */ ORDER(CALL),
    ADDRESS(S40_8),
    ORDER(GOTO),
    ADDRESS(QTEMP1),
    /* SETUP.9 */ ORDERS(BON, SLOAD),
    SWITCH(BON, FIRSTFLG),
    ADDRESS(SURELY_9),
    ADDRESS_PLUS_ONE(NBRCYCLP),
    ORDERS(NORM, VXSC), /* "(NORM HANDLES ZERO PROPERLY)" */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(BDT),
    ORDERS(VSR_STAR, VAD),
    SR_X1(-14),
    ADDRESS_PLUS_ONE(VGTEMP),
    ORDER(VSU),
    ADDRESS_PLUS_ONE(DELVSUM),
    STORE(VGPREV),
    /* SURELY.9 */ ORDER(EXIT),
};

/* SURELY.9 +1, after the EXIT: S40.9 as a job (FINDVAC, PRIO10), */
void surely_9_after_exit(void)
{
    set_A(PRIO10);
    tc_to(SURELY_9 + 2u, FINDVAC); /* 2CADR S40.9 */
}

/* restart groups 1 and 5 (2PHSCHNG, OCT 00051, "1.5 = REDO40.9, PRIO 10",
 * and OCT 10035), */
void surely_9_after_findvac(void)
{
    tc_to(SURELY_9 + 5u, TWO_PHSCHNG);
}

/* and to the interpreter, for SURELY_9_CODE. */
void surely_9_after_2phschng(void)
{
    tc_at(SURELY_9 + 010u);
    intpret();
}

/* SURELY_9_CODE: the state vector for S40.9, "ACTIVE VEHICLE RADIUS
 * VECTOR AT T1" (RN) and "VELOCITY VECTOR AT T1" (VN) to RINIT and VINIT,
 * PIPTIME to TNIT, and TPASS4 less it to DELLT4; DELVSUM and NBRCYCLS
 * zero; on at CALL40.8. */
static const word surely_9_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RN), /* "ACTIVE VEHICLE RADIUS VECTOR AT T1" */
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VN), /* "ACTIVE VEHICLE VELOCITY VECTOR AT T1" */
    STODL(VINIT),
    ADDRESS_PLUS_ONE(PIPTIME),
    STORE(TNIT),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(TPASS4),
    STOVL(DELLT4),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STODL(DELVSUM),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STORE(NBRCYCLS),
    ORDER(GOTO),
    ADDRESS(CALL40_8),
};

/* ---- STEERING -- pp. 701-702 ---- */

/* STEERING: SERVICER's exit (AVEGEXIT) during the burn; to the
 * interpreter, for STEERING_CODE. */
void steering(void)
{
    tc_at(STEERING);
    intpret();
}

/* STEERING_CODE: UPDATEVG. */
static const word steering_words[] = {
    ORDER(CALL),
    ADDRESS(UPDATEVG),
    ORDER(EXIT),
};

/* STEERING +4, after the EXIT: "CHECK IMPULSW" (bit 9 of FLAGWRD2, by CCS
 * A): clear, SERVXT; set, "PRE-IGNITE, REQUEST ENG-OFF, OR POST-OFF":
 * "CHECK ENGINE-ON/-OFF" (bit 13 of DSALMOUT): "ENGINE-OFF, SO PRE-IGNITE
 * OR POST-OFF", SERVXT; on, EBANK 7 (TCR E7SETTER). */
void steering_after_exit(void)
{
    set_A(BIT9); /* "CHECK IMPULSW" */
    mask_at(ADDRESS_OF(FLAGWRD2));
    if (A_is_zero()) { /* CCS A */
        servxt();
        return;
    }
    set_A(BIT13);           /* "CHECK ENGINE-ON/-OFF" */
    rand_channel(DSALMOUT); /* EXTEND */
    if (A_is_zero()) {      /* EXTEND; BZF SERVXT */
        servxt();
        return;
    }
    tc_at(STEERING_EXIT + 014u); /* TCR E7SETTER */
    e7setter();
    steering_after_e7setter();
}

/* STEERING_EXIT +015, E7SETTER's return: INHINT; the time to go, TIG less
 * TIME2, in MPAC, its halves made to agree in sign (TCR DPAGREE). */
void steering_after_e7setter(void)
{
    inhint();
    dca(ADDRESS_OF(TIG)); /* EXTEND */
    dxch(ADDRESS_OF(MPAC));
    dcs(ADDRESS_OF(TIME2)); /* EXTEND */
    das(ADDRESS_OF(MPAC));
    tc_to(STEERING_EXIT + 024u, DPAGREE);
}

/* DPAGREE's return: TGO (+0 and the time to go, "LESS THAN 6 (OR 4)
 * SECONDS TO GO", at least one centisecond, "PROTECT AGAINST NEG/ZRO
 * W.L. CALL"); ENGINOFF after it (WAITLIST). */
void steering_after_dpagree(void)
{
    ca(ADDRESS_OF(MPAC) + 1u);
    if (ccs_register(A_ADDRESS) == NEGATIVE) set_A(ZERO); /* "PROTECT AGAINST NEG/ZRO" */
    ad(ONE);
    xch(L_ADDRESS);
    set_A(ZERO);
    dxch(ADDRESS_OF(TGO));
    ca(ADDRESS_OF(TGO) + 1u);
    tc_to(STEERING_EXIT + 037u, WAITLIST); /* 2CADR ENGINOFF */
}

/* Restart groups 3 and 5 (2PHSCHNG, OCT 40153, "3.15 = ENGINOFF (TGO+1)",
 * and OCT 10035, "5.3 = REREADAC"), */
void steering_after_waitlist(void)
{
    tc_to(STEERING_EXIT + 042u, TWO_PHSCHNG);
}

/* "CLEAR IMPULSW, ENGINOFF IS NOW SET UP" (DOWNFLAG), */
void steering_after_2phschng(void)
{
    tc_to(STEERING_EXIT + 045u, DOWNFLAG); /* ADRES IMPULSW */
}

/* and back to SERVICER (TCF SERVXT). */
void steering_after_downflag(void)
{
    servxt();
}

/* ---- The interpretive code ---- */

const struct interpretive_code UPDATEVG_CODE = {024u, UPDATEVG, COUNT(updatevg_words),
                                                updatevg_words};
const struct interpretive_code CALL40_8_CODE = {024u, UPDATEVG_EXIT + 011u, COUNT(call40_8_words),
                                                call40_8_words};
const struct interpretive_code SURELY_9_CODE = {024u, SURELY_9 + 011u, COUNT(surely_9_words),
                                                surely_9_words};
const struct interpretive_code STEERING_CODE = {024u, STEERING + 1u, COUNT(steering_words),
                                                steering_words};

#endif
