/* SPDX-License-Identifier: MIT */
/* P70-P71.agc, pp. 832-836 (Luminary099 only, bank 32): GOABORT, where
 * the abort starts after ENEMA; P70INIT, the descent engine's numbers and
 * the targets for P70; P71RET, the ascent engine's for P71; and the
 * subroutines TGOCOMP and THROTUP. The faithful C is
 * c64-port/src/lm_p70_p71_file.c. */
#include "p70_p71.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- GOABORT -- pp. 832-833 ---- */

/* GOABORT: the interpretive code (GOABORT_CODE), ... */
void goabort(void)
{
    tc_at(GOABORT);
    intpret();
}

/* ... which sets the desired thrust direction from the CDUs (INITCDUW). */
static const word goabort_words[] = {
    /* GOABORT +1 */ ORDER(CALL),
    ADDRESS(INITCDUW),
    ORDER(EXIT),
};

const struct interpretive_code GOABORT_CODE = {032u, GOABORT + 1u, COUNT(goabort_words),
                                               goabort_words};

/* After the EXIT: DVCNTR four, WHICH the abort's table of the ignition
 * steps (WHICHADR, ABRTABLE), and the flags: FLRCS clear (the jets are not
 * the engine), ... (Each TS stores a constant: none skips.) */
void goabort_after_exit(void)
{
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    set_A(WHICHADR);
    ts(ADDRESS_OF(WHICH));
    tc_to(GOABORT + 010u, DOWNFLAG); /* ADRES FLRCS */
}

/* ... FLUNDISP clear, ... */
void goabort_after_flrcs(void)
{
    tc_to(GOABORT + 012u, DOWNFLAG); /* ADRES FLUNDISP */
}

/* ... IDLEFLAG clear, ... */
void goabort_after_flundisp(void)
{
    tc_to(GOABORT + 014u, DOWNFLAG); /* ADRES IDLEFLAG */
}

/* ... and "INSURE 4-JET TRANSLATION CAPABILITY" (ACC4-2FL set). */
void goabort_after_idleflag(void)
{
    tc_to(GOABORT + 016u, UPFLAG); /* ADRES ACC4-2FL */
}

/* Then CHECKMM, with the program 70 (70DEC): in P70 its return is
 * P70INIT, ... */
void goabort_after_acc4_2fl(void)
{
    tc_to(GOABORT + 020u, CHECKMM);
}

/* ... and otherwise the word after, TCF P71RET. */
void goabort_after_checkmm(void)
{
    p71ret();
}

/* ---- P70INIT -- pp. 833-835 ---- */

/* P70INIT: the interpretive code, ... */
void p70init(void)
{
    tc_at(P70INIT);
    intpret();
}

/* ... the time to go from now to the ignition (TGOCOMP), the descent
 * engine's burn-up time TBUP (MASS over MDOTDPS), 1/DV1 to 1/DV3, AT, TTO,
 * "INITIALIZE DPS EXHAUST VELOCITY" (VE, from DPSVEX), FLAP set and
 * COMMINIT; then, with X1 for the DPS coefficients (0), BOTHPOLY. P71
 * comes in at INJTARG, with X1 for the APS coefficients (8). BOTHPOLY:
 *
 *     ZDOTD = A + TGO(B + TGO(C + TGO D))     (ABTCOF,1: A, B, C, D)
 *
 * but no less than VMIN; RDOTD from ABTRDOT; Y (YCOMP), and YCO the part
 * of /Y/ over YLIM, signed as Y ("IF <0, XR<.5DEG, LEAVE YCO AT 0"); and
 * "XRANGE IN CASE ASTRONAUT WANTS TO LOOK" (YCO - Y). UPTHROT: FLVR set,
 * and the basic code. */
static const word p70init_words[] = {
    /* P70INIT +1 */ ORDER(CALL),
    ADDRESS(TGOCOMP),
    ORDERS(DLOAD, SL),
    ADDRESS_PLUS_ONE(MDOTDPS),
    SL_COUNT(4),
    ORDER(BDDV),
    ADDRESS_PLUS_ONE(MASS),
    STODL(TBUP),
    ADDRESS_PLUS_ONE(MASS),
    ORDERS(DDV, SR1),
    ADDRESS_PLUS_ONE(K_1_DV),
    STORE(1_DV1),
    STORE(1_DV2),
    STORE(1_DV3),
    ORDER(BDDV),
    ADDRESS_PLUS_ONE(K_AT),
    STODL(AT),
    ADDRESS_PLUS_ONE(DTDECAY),
    ORDERS(DCOMP, SL),
    SL_COUNT(11),
    STORE(TTO),
    ORDERS(SLOAD, DCOMP),
    ADDRESS_PLUS_ONE(DPSVEX),
    ORDER(SR2),
    STORE(VE), /* "INITIALIZE DPS EXHAUST VELOCITY" */
    ORDERS(SET, CALL),
    SWITCH(SET, FLAP),
    ADDRESS(COMMINIT),
    ORDERS(AXC_1, GOTO), /* "RETURN HERE IN P70, SE X1 FOR DPS COEFF." */
    NUMBER(0),
    ADDRESS(BOTHPOLY),
    /* INJTARG */ ORDER(AXC_1), /* "RETURN HERE IN P71, SET X1 FOR APS COEFF" */
    NUMBER(8),
    /* BOTHPOLY */ ORDERS(DLOAD_STAR, DMP), /* "TGO D" */
    ADDRESS_PLUS_ONE_AT_X1(ABTCOF, 0),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DAD_STAR, DMP),
    ADDRESS_PLUS_ONE_AT_X1(ABTCOF, 2), /* "TGO(C+TGO D)" */
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DAD_STAR, DMP),
    ADDRESS_PLUS_ONE_AT_X1(ABTCOF, 4), /* "TGO(B+TGO(C+TGO D))" */
    ADDRESS_PLUS_ONE(TGO),
    ORDER(DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(ABTCOF, 6), /* "A+TGO(B+TGO(C+TGO D))" */
    STORE(ZDOTD),                      /* "STORE TENTATIVELY IN ZDOTD" */
    ORDERS(DSU, BPL),                  /* "CHECK AGAINST MINIMUM" */
    ADDRESS_PLUS_ONE(VMIN),
    ADDRESS(UPRATE), /* "IF BIG ENOUGH, LEAVE ZDOTD AS IS ." */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(VMIN),
    STORE(ZDOTD), /* "IF TOO SMALL, REPLACE WITH MINIMUM." */
    /* UPRATE */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ABTRDOT),
    STCALL(RDOTD),  /* "INITIALZE RDOTD." */
    ADDRESS(YCOMP), /* "COMPUTE Y" */
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(YLIM), /* "/Y/-DYMAX" */
    ORDERS(BMN, SIGN),
    ADDRESS(YOK),
    ADDRESS_PLUS_ONE(Y),
    STORE(YCO),
    /* YOK */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(YCO),
    ADDRESS_PLUS_ONE(Y), /* "COMPUTE XRANGE IN CASE ASTRONAUT WANTS" */
    ORDER(SR),
    SR_COUNT(5),
    STORE(XRANGE), /* "TO LOOK." */
    /* UPTHROT */ ORDERS(SET, EXIT),
    SWITCH(SET, FLVR),
};

const struct interpretive_code P70INIT_CODE = {032u, P70INIT + 1u, COUNT(p70init_words),
                                               p70init_words};

/* After UPTHROT's EXIT: "SET ROTFLAG", ... */
void upthrot_after_exit(void)
{
    tc_to(UPTHROT + 2u, UPFLAG); /* ADRES ROTFLAG */
}

/* ... THROTUP, whose return is the word after, ... */
void upthrot_after_rotflag(void)
{
    tc_at(UPTHROT + 4u);
    throtup();
}

/* ... and restart group 4 at 4.2 (PHASCHNG, OCT 04024): ... */
void upthrot_after_throtup(void)
{
    tc_to(UPTHROT + 5u, PHASCHNG);
}

/* UPTHROT1 -3: "VERIFY THAT THE PANEL SWITCHES ARE PROPERLY SET" (P40AUTO,
 * through BANKCALL), ... */
void upthrot1_minus3(void)
{
    tc_to(UPTHROT1 - 3u, BANKCALL); /* CADR P40AUTO */
}

/* ... THROTUP again, ... */
void upthrot1_after_p40auto(void)
{
    tc_at(UPTHROT1 - 1u);
    throtup();
}

/* ... and UPTHROT1: "SET SERVICER TO CALL ASCENT GUIDANCE" (AVGEXIT gets
 * ATMAGAD, 2CADR ATMAG), and GRP4OFF: "TERMINATE USE OF GROUP 4"
 * (PHASCHNG, OCT 00004), ... */
void upthrot1(void)
{
    set_A(ATMAGAD[0]);
    set_L(ATMAGAD[1]);
    dxch(ADDRESS_OF(AVGEXIT));
    tc_to(GRP4OFF, PHASCHNG);
}

/* ... and ENDOFJOB. */
void grp4off_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* ---- P71RET -- p. 835 ---- */

/* P71RET: LETABORT clear, ... */
void p71ret(void)
{
    tc_to(P71RET, DOWNFLAG); /* ADRES LETABORT */
}

/* ... "SET DVMON THRESHOLD TO THE ASCENT VALUE" (THRESH2 to DVTHRUSH),
 * and the interpretive code. (A is a constant: TS does not skip.) */
void p71ret_after_letabort(void)
{
    set_A(THRESH2);
    ts(ADDRESS_OF(DVTHRUSH));
    tc_at(P71RET + 4u);
    intpret();
}

/* With FLAP clear (P70 has not run), "TGO=T-TIG" (TGOCOMP), and P12INIT,
 * which comes back to INJTARG (QPRET: "WILL EXIT P12INIT TO INJTARG").
 * With FLAP set, OLDTIME: "IF FLAP=1,GTO=2 TGO" in TGO1, and P12INIT,
 * back to the EXIT. */
static const word p71ret_words[] = {
    /* P71RET +5 */ ORDERS(BON, CALL),
    SWITCH(BON, FLAP),
    ADDRESS(OLDTIME),
    ADDRESS(TGOCOMP), /* "IF FLAP=0, TGO=T-TIG" */
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(QPRET),
    ADDRESS(INJTARG),                 /* CADR INJTARG */
    ADDRESS(P12INIT),                 /* "WILL EXIT P12INIT TO INJTARG" */
    /* OLDTIME */ ORDERS(DLOAD, SL1), /* "IF FLAP=1,GTO=2 TGO" */
    ADDRESS_PLUS_ONE(TGO),
    STCALL(TGO1),
    ADDRESS(P12INIT),
    ORDER(EXIT),
};

const struct interpretive_code P71RET_CODE = {032u, P71RET + 5u, COUNT(p71ret_words), p71ret_words};

/* After the EXIT: restart group 4 at 4.2 (PHASCHNG, OCT 04024), ... */
void p71ret_after_exit(void)
{
    tc_to(OLDTIME + 5u, PHASCHNG);
}

/* ... TGO from TGO1, and UPTHROT1 -3. */
void p71ret_after_phaschng(void)
{
    dca(ADDRESS_OF(TGO1));
    dxch(ADDRESS_OF(TGO));
    upthrot1_minus3();
}

/* ---- TGOCOMP and THROTUP -- p. 836 ---- */

/* TGOCOMP: TGO, the time now (LOADTIME) less TIG, and back (RVQ). */
static const word tgocomp_words[] = {
    /* TGOCOMP */ ORDERS(RTB, DSU),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TIG),
    ORDER(SL),
    SL_COUNT(11),
    STORE(TGO),
    ORDER(RVQ),
};

const struct interpretive_code TGOCOMP_CODE = {032u, TGOCOMP, COUNT(tgocomp_words), tgocomp_words};

/* THROTUP: THRUST BIT13, the engine's throttle all the way up, sent
 * (channel 14 bit 4), and back (TC Q). (A is a constant: TS THRUST does
 * not skip.) */
void throtup(void)
{
    set_A(BIT13);
    ts(ADDRESS_OF(THRUST));
    set_A(BIT4);
    wor_channel(CHAN14);
    tc_q();
}

#endif
