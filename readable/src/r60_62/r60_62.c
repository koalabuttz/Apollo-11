/* SPDX-License-Identifier: MIT */
/* R60_62.agc (Luminary099 pp. 472-485, Comanche055 pp. 390-398). R60 is
 * "CALLED AS A GENERAL SUBROUTINE TO MANEUVER THE LM TO A SPECIFIED
 * ATTITUDE" (the CM's the same for the CSM): with the 3-axis flag clear,
 * VECPOINT works out the final gimbal angles (CPHI, CTHETA, CPSI) that
 * point the spacecraft axis SCAXIS along POINTVSM; the crew is asked to
 * "PLEASE PERFORM AUTO MANEUVER" (verb 50 noun 18, by GOPERF2R); proceed
 * does the maneuver through KALCMANU (GOMANUR) once the switches are on
 * auto, enter ends R60, back to the caller (TEMPR60), and terminate ends
 * the program (GOTOPOOH, or GOTOV56 in R61). R62, verb 49, flashes the
 * gimbal angles (verb 06 noun 22) for the crew to load, then sets the
 * 3-axis flag and calls R60 for them. The LM's BALLANGS turns the gimbal
 * angles into the FDAI ball's (noun 18); CHKLINUS and RELINUS see R60
 * through a priority display (LINUS) over a restart.
 *
 * Its interpretive code (VECPOINT, the calls of it, and the LM's
 * BALLANGS) is in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs (src/binding/r60_62_routines.c),
 * and its basic code is below. The faithful C is c64-port/src/lm_r60_r62.c
 * and cm_r60_r62.c.
 *
 * Each call out of the file (BANKCALL, MAKECADR, UPFLAG, DOWNFLAG,
 * PHASCHNG, PRIOCHNG, LINUS) leaves Z at the routine and Q at the word
 * after the TC, where the routine comes back to; BANKJUMP, ENDOFJOB,
 * ENDEXT, GOTOPOOH and GOTOV56 don't come back. */
#include "r60_62.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). */
static void tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
}

/* TC to an erasable word that holds a return address (MPAC +2, TBASE1 or
 * TBASE2): the word runs as TC <address>. Not modelled: a word that is
 * not such an address, as for TC Q. */
static void tc_saved(unsigned address)
{
    word to = fetch(address);

    if ((to & 070000u) || to < 010u) not_modelled();
    tc_through(address);
}

/* ---- R62 (both missions) ---- */

static void gomove(void);

/* R62DISP -- LM p. 485, CM p. 398. "ROUTINE FOR INITIATING AUTOMATIC
 * MANEUVER VIA KEYBOARD (V49)": GOFLASH, through BANKCALL, flashes verb 06
 * noun 22 ("ICDU ANGLES", CPHI, CTHETA, CPSI) and comes back for the
 * crew's answer: terminate, proceed or enter. (The LM's label is
 * R62FLASH; R62DISP EQUALS it.) */
void r62disp(void)
{
    set_A(V06N22); /* "FLASH V06N22 AND */
    tc_at(R62DISP + 1u);
    continue_at(BANKCALL); /* CADR GOFLASH: ICDU ANGLES" */
}

/* R62DISP +3 -- "TERMINATE". */
void r62disp_terminate(void)
{
    continue_at(ENDEXT);
}

/* R62DISP +4 -- "PROCEED". */
void r62disp_proceed(void)
{
    gomove();
}

/* R62DISP +5 -- "ENTER": "ASTRONAUT MAY LOAD NEW ICDUS AT THIS POINT",
 * and the display again. */
void r62disp_enter(void)
{
    r62disp();
}

/* GOMOVE -- "SET FOR 3-AXIS MANEUVER": the angles are the crew's, so R60
 * does not work them out. */
static void gomove(void)
{
    tc_at(GOMOVE);
    continue_at(UPFLAG); /* ADRES 3AXISFLG */
}

/* GOMOVE +2 -- R60 (R60LEM or R60CSM), through BANKCALL. */
void gomove_after_upflag(void)
{
    tc_at(GOMOVE + 2u);
    continue_at(BANKCALL); /* CADR R60LEM, R60CSM */
}

/* GOMOVE +4 -- "END R62". */
void gomove_after_bankcall(void)
{
    continue_at(ENDEXT);
}

#if LUMINARY099

/* ---- R60, the LM's ---- */

static void three_axis_test(void (*yes)(void), word intpret_at);
static void redomanc(void);
static void endmanu1(void);
static bool chklinus(void);
static void automanv(void);
static void rer60(void);
static void r61test(void);

/* R60LEM -- LM p. 473. MAKECADR gives the caller's return as a CADR, for
 * TEMPR60 ("CALLERS RETURN ADDRESS"). */
void r60lem(void)
{
    tc_at(R60LEM);
    continue_at(MAKECADR);
}

/* R60LEM +1, REDOMANN -- LM p. 473. "1. IF THE 3-AXIS FLAG IS NOT SET THE
 * FINAL CDU ANGLES ARE CALCULATED (VECPOINT)". (A word with overflow in A
 * would skip the CAF: MAKECADR's CADR has none.) */
void r60lem_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(TEMPR60))) set_A(THREE_AXIS_BIT); /* REDOMANN */
    three_axis_test(toball, REDOMANN + 4u);
}

/* MASK FLAGWRD5; CCS A: "IS 3-AXIS FLAG SET": yes (the bit, positive), on
 * to yes; no (+0), the TC INTPRET at intpret_at into the CALL of VECPOINT.
 * (A masked flag word is never negative, but for a TS's overflow skip:
 * not modelled.) */
static void three_axis_test(void (*yes)(void), word intpret_at)
{
    mask_at(ADDRESS_OF(FLAGWRD5));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: yes(); return; /* "YES" */
    case IS_PLUS_ZERO:
        tc_at(intpret_at);
        intpret();
        return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* REDOMANN +5 -- LM p. 473. VECPOINT gives the angles in MPAC to MPAC +2:
 * "STORE FINAL ANGLES - CPHI,CTHETA,CPSI". */
static const word redomann_words[] = {
    ORDER(CALL),
    ADDRESS(VECPOINT), /* "TO COMPUTE FINAL ANGLES" */
    STORE(CPHI),       /* "STORE FINAL ANGLES - CPHI,CTHETA,CPSI" */
    ORDER(EXIT),
};

/* TOBALL -- LM p. 473. "2. THE FDAI BALL ANGLES (NOUN 18) ARE CALCULATED
 * (BALLANGS)". */
void toball(void)
{
    tc_at(TOBALL);
    continue_at(BANKCALL); /* CADR BALLANGS: "TO CONVERT ANGLES TO FDAI" */
}

/* TOBALLA -- LM p. 473. "3. REQUEST FLASHING DISPLAY V50 N18 PLEASE
 * PERFORM AUTO MANEUVER." GOPERF2R, through BANKCALL, makes an immediate
 * return to the word after its terminate, proceed and enter returns, and
 * comes back to one of those for the crew's answer. */
void toballa(void)
{
    set_A(V06N18);
    tc_at(TOBALLA + 1u);
    continue_at(BANKCALL); /* CADR GOPERF2R: "DISPLAY PLEASE PERFORM AUTO MANEUVER" */
}

/* TOBALLA +3 -- terminate. */
void toballa_terminate(void)
{
    tc_at(TOBALLA + 3u);
    r61test();
}

/* TOBALLA +4 -- "PROCEED". */
void toballa_proceed(void)
{
    tc_at(TOBALLA + 4u);
    redomanc();
}

/* TOBALLA +5 -- "ENTER I.E. FINISHED WITH R60". */
void toballa_enter(void)
{
    tc_at(TOBALLA + 5u);
    endmanu1();
}

/* TOBALLA +6 -- GOPERF2R's immediate return: "TO CHECK FOR PRIORITY
 * DISPLAYS", then the job ends, to wait for the crew. */
void toballa_immediate_return(void)
{
    tc_at(TOBALLA + 6u);
    if (chklinus()) toballa_after_chklinus();
}

/* TOBALLA +7 */
void toballa_after_chklinus(void)
{
    tc_at(TOBALLA + 7u);
    continue_at(ENDOFJOB);
}

/* REDOMANC -- LM p. 474. Steps 6 and 7: the final angles again, unless
 * the 3-axis flag is set, then the ball angles (TOBALLC). */
static void redomanc(void)
{
    set_A(THREE_AXIS_BIT);
    three_axis_test(toballc, REDOMANC + 4u);
}

/* REDOMANC +5 -- LM p. 474. */
static const word redomanc_words[] = {
    ORDER(CALL),
    ADDRESS(VECPOINT), /* "TO COMPUTE FINAL ANGLES" */
    STORE(CPHI),       /* "STORE ANGLES" */
    ORDER(EXIT),
};

/* TOBALLC -- LM p. 474. */
void toballc(void)
{
    tc_at(TOBALLC);
    continue_at(BANKCALL); /* CADR BALLANGS: "TO CONVERT ANGLES TO FDAI" */
}

/* TOBALLC +2 -- "CHECK AUTO MODE": steps 8 and 9. */
void toballc_after_ballangs(void)
{
    tc_at(TOBALLC + 2u);
    g_n_auto();
    toballc_after_g_n_auto();
}

/* TOBALLC +3 -- G+N,AUTO's answer: "NOT AUTO, GO REREQUEST AUTO
 * MANEUVER" (positive), or AUTOMANV (+0). (The answer is a masked bit,
 * never negative; a negative A here skips AUTOMANV's CAF, as the AGC
 * does.) */
void toballc_after_g_n_auto(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: toballa(); return;
    case IS_PLUS_ZERO: automanv(); return;
    case NEGATIVE:
        tc_at(AUTOMANV + 1u);
        continue_at(BANKCALL); /* CADR GODSPR */
        return;
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* AUTOMANV -- LM p. 474. "10. NONFLASHING DISPLAY V06N18 (FDAI
 * ANGLES)", by GODSPR. */
static void automanv(void)
{
    set_A(V06N18); /* "STATIC DISPLAY DURING AUTO MANEUVER" */
    tc_at(AUTOMANV + 1u);
    continue_at(BANKCALL); /* CADR GODSPR */
}

/* AUTOMANV +3 -- "TO CHECK FOR PRIORITY DISPLAYS", then the maneuver. */
void automanv_after_godspr(void)
{
    tc_at(AUTOMANV + 3u);
    if (chklinus()) startmnv();
}

/* STARTMNV -- LM p. 474. "12. DO A MANEUVER CALCULATION AND ICDU DRIVE
 * ROUTINE TO ACHIEVE FINAL GIMBAL ANGLES (GOMANUR)". */
void startmnv(void)
{
    tc_at(STARTMNV);
    continue_at(BANKCALL); /* CADR GOMANUR: "PERFORM MANEUVER VIA KALCMANU" */
}

/* ENDMANUV -- LM p. 474. "13. AT END OF MANEUVER GO TO STEP 3." */
void endmanuv(void)
{
    toballa(); /* TCF TOBALLA: "FINISHED MANEUVER." */
}

/* ENDMANU1 -- LM p. 474. "RESET 3-AXIS FLAG" and back to the caller,
 * at the CADR in TEMPR60. */
static void endmanu1(void)
{
    tc_at(ENDMANU1);
    continue_at(DOWNFLAG); /* ADRES 3AXISFLG */
}

/* ENDMANU1 +2 */
void endmanu1_after_downflag(void)
{
    ca(ADDRESS_OF(TEMPR60)); /* CAE */
    tc_at(ENDMANU1 + 3u);
    continue_at(BANKJUMP);
}

/* CHKLINUS -- LM p. 474. "IS PRIORITY DISPLAY FLAG SET?" If not, back to
 * the caller ("NO - EXIT": true). If so ("4. IF PRIORITY DISPLAY FLAG IS
 * SET DO A PHASECHANGE"), the return is saved in MPAC +2, TBASE2 gets the
 * place for a restart, 3 before the Q of the last display that BUF2
 * "HOLDS", and the phase is 2.13 (RELINUS); then LINUS makes the display a
 * priority one. (False: the routine has gone to PHASCHNG.) */
static bool chklinus(void)
{
    cs(ADDRESS_OF(FLAGWRD4));
    mask(PDSPFBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_q(); return true; /* "NO - EXIT" */
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (a masked bit) */
    }
    ca(Q_ADDRESS);
    if (ts(MPAC_AT(2))) not_modelled(); /* "SAVE RETURN" (an address: no overflow) */
    set_A(negate(THREE));               /* "OBTAIN LOCATION FOR RESTART" */
    ad_at(BUF2_AT(0));                  /* "HOLDS Q OF LAST DISPLAY" */
    if (ts(ADDRESS_OF(TBASE2))) not_modelled();
    tc_at(CHKLINUS + 011u);
    continue_at(PHASCHNG); /* OCT 00132 */
    return false;
}

/* CHKLINUS +013 */
void chklinus_after_phaschng(void)
{
    set_A(BIT7);
    tc_at(CHKLINUS + 014u);
    continue_at(LINUS); /* "GO SET BITS FOR PRIORITY DISPLAY" */
}

/* CHKLINUS +015 -- back to CHKLINUS's caller, at the return saved in
 * MPAC +2. */
void chklinus_after_linus(void)
{
    tc_at(CHKLINUS + 015u);
    tc_saved(MPAC_AT(2));
}

/* RELINUS -- LM p. 475. Where the restart comes (phase 2.13) after a
 * priority display: "RESTORE ORIGINAL PRIORITY". */
void relinus(void)
{
    set_A(PRIO26);
    tc_at(RELINUS + 1u);
    continue_at(PRIOCHNG);
}

/* RELINUS +2 -- "DON'T CONTINUE R60 UNLESS TRACKFLAG ON": with it on,
 * RER60. Otherwise the job ends with the restart phase set back for its
 * program: "IS IT P20?" (RNDVZBIT): "SET 2.7 SPOT FOR P20"; if not, "MUST
 * BE P25, SET 2.11 SPOT". */
void relinus_after_priochng(void)
{
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(FLAGWRD1));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: rer60(); return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (a masked bit) */
    }
    set_A(RNDVZBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:             /* "YES" */
        tc_at(RELINUS + 011u); /* TC +4 */
        tc_at(RELINUS + 015u);
        continue_at(PHASCHNG); /* OCT 40072: "SET 2.7 SPOT FOR P20" */
        return;
    case IS_PLUS_ZERO: relinus_not_p20(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (a masked bit) */
    }
}

/* RELINUS +012 -- "NO, MUST BE P25, SET 2.11 SPOT". */
void relinus_not_p20(void)
{
    tc_at(RELINUS + 012u);
    continue_at(PHASCHNG); /* OCT 40112 */
}

/* RELINUS +014 */
void relinus_after_p25_phaschng(void)
{
    tc_at(RELINUS + 014u);
    continue_at(ENDOFJOB);
}

/* RELINUS +017 */
void relinus_after_p20_phaschng(void)
{
    tc_at(RELINUS + 017u);
    continue_at(ENDOFJOB);
}

/* RER60 -- LM p. 475. "SET PRIO DISPLAY FLAG AFTER RESTART", and on at
 * the place CHKLINUS left in TBASE2. */
static void rer60(void)
{
    tc_at(RER60);
    continue_at(UPFLAG); /* ADRES PDSPFLAG */
}

/* RER60 +2 */
void rer60_after_upflag(void)
{
    tc_at(RER60 + 2u);
    tc_saved(ADDRESS_OF(TBASE2));
}

/* R61TEST -- LM p. 475. Terminate: "IF WE ARE IN P00 IT MUST BE V49 OR
 * V89", "THUS WE GO TO ENDEXT VIA USER" (ENDMANU1); otherwise, "ARE WE IN
 * R61 (P20 OR P25)": the priority display flag says so, and GOTOV56 ends
 * it; if not, GOTOPOOH ends the program. */
static void r61test(void)
{
    ca(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF ENDMANU1 */
        endmanu1();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD4));
    mask(PDSPFBIT);
    if (A_is_zero()) { /* EXTEND; BZF GOTOPOOH: "NO" */
        continue_at(GOTOPOOH);
        return;
    }
    tc_at(R61TEST + 7u);
    continue_at(GOTOV56); /* "YES" */
}

/* G+N,AUTO -- LM pp. 475-476. Bit 10 of channel 30, "NOT IN G+N C(A) =
 * +", returns at once (CCS leaves it positive); in G+N, on to ISITAUTO. (A
 * masked bit is never negative.) */
void g_n_auto(void)
{
    read_channel(030u);
    mask(BIT10);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_q(); return;
    case IS_PLUS_ZERO: isitauto(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* ISITAUTO -- LM p. 476. "CHECK FOR AUTO MODE": bit 14 of channel 31,
 * "(+) = NOT IN AUTO, (+0) = AOK". */
void isitauto(void)
{
    read_channel(031u);
    mask(BIT14);
    tc_q();
}

/* ---- BALLANGS ---- */

/* BALLANGS -- LM pp. 477-478. "COMPUTES LM FDAI BALL DISPLAY ANGLES":
 * MAKECADR gives the return as a CADR, for BALLEXIT. */
void ballangs(void)
{
    tc_at(BALLANGS);
    continue_at(MAKECADR);
}

/* BALLANGS +1 -- the gimbal angles CPHI, CTHETA and CPSI ("THE OUTER,
 * INNER, & MIDDLE GIMBAL ANGLES") into CDUSPOT's X, Y and Z words (+4, +0,
 * +2), for CD*TR*G, then into the interpreter. (A word with overflow in A
 * would skip the first CA: MAKECADR's CADR has none. The words CA loads
 * have none either.) */
void ballangs_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(BALLEXIT))) ca(ADDRESS_OF(CPHI));
    (void)ts(ADDRESS_OF(CDUSPOT) + 4u);
    ca(ADDRESS_OF(CTHETA));
    (void)ts(ADDRESS_OF(CDUSPOT));
    ca(ADDRESS_OF(CPSI));
    (void)ts(ADDRESS_OF(CDUSPOT) + 2u);
    tc_at(BALLANGS + 010u);
    intpret();
}

/* BALLANGS +011 -- LM p. 478. CD*TR*G puts the sines and cosines of the
 * three angles in SINCDU and COSCDU. Then the ball's angles from the
 * gimbals': "YAW = ARCSIN(-SXCZ)", "ROLL = ARCTAN(SZ/CZCX)", and pitch
 * the angle whose sine is "CXSY + SXSZCY" and cosine "CXCY - SXSZSY"
 * (ARCTAN of SINTH and COSTH, "RETURNS WITH D(MPAC) = PITCH"). VDEF
 * makes the vector "ROLL, PITCH, YAW", and V1STO2S the ball's angles in
 * two's complement, FDAIX, FDAIY and FDAIZ. */
static const word ballangs_words[] = {
    ORDERS(SETPD, CALL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(CD_STARTR_STARG),   /* CD*TR*G */
    ORDERS(DLOAD, DMP),         /* */
    ADDRESS_PLUS_ONE(SINCDUX),  /* "SIN (OGA)" */
    ADDRESS_PLUS_ONE(COSCDUZ),  /* "COS (MGA)" */
    ORDERS(SL1, DCOMP),         /* "SCALE" */
    ORDERS(ARCSIN, PDDL),       /* "YAW = ARCSIN(-SXCZ) INTO 0 PD" */
    ADDRESS_PLUS_ONE(SINCDUZ),  /* */
    STODL(SINTH),               /* "(SINTH = 18D IN PD)" */
    ADDRESS_PLUS_ONE(COSCDUZ),  /* */
    ORDERS(DMP, SL1),           /* "RESCALE" */
    ADDRESS_PLUS_ONE(COSCDUX),  /* */
    STCALL(COSTH),              /* "(COSTH= 16D IN PD)" */
    ADDRESS(ARCTAN),            /* */
    ORDERS(PDDL, DMP),          /* "ROLL = ARCTAN(SZ/CZCX) INTO 2 PD" */
    ADDRESS_PLUS_ONE(SINCDUZ),  /* */
    ADDRESS_PLUS_ONE(SINCDUX),  /* */
    ORDERS(SL2, PUSH),          /* "SXSZ INTO 4 PD" */
    ORDERS(DMP, PDDL),          /* "SXSZCY INTO 4 PD" */
    ADDRESS_PLUS_ONE(COSCDUY),  /* */
    ORDERS(DMP, PDDL),          /* "SXSZSY INTO 6 PD" */
    ADDRESS_PLUS_ONE(SINCDUY),  /* */
    ADDRESS_PLUS_ONE(COSCDUX),  /* */
    ORDERS(DMP, SL1),           /* "CXCY" */
    ADDRESS_PLUS_ONE(COSCDUY),  /* */
    ORDERS(DSU, STADR),         /* "PULL UP FROM 6 PD" */
    AFTER_STADR(STODL(COSTH)),  /* "COSTH = CXCY - SXSZSY" */
    ADDRESS_PLUS_ONE(SINCDUY),  /* */
    ORDERS(DMP, SL1),           /* */
    ADDRESS_PLUS_ONE(COSCDUX),  /* "CXSY" */
    ORDERS(DAD, STADR),         /* "PULL UP FROM 4 PD" */
    AFTER_STADR(STCALL(SINTH)), /* "SINTH = CXSY + SXSZCY" */
    ADDRESS(ARCTAN),            /* "RETURNS WITH D(MPAC) = PITCH" */
    ORDERS(PDDL, VDEF),         /* "PITCH INTO 2 PD, ROLL INTO MPAC FROM 2PD" */
    ORDER(RTB),                 /* "VDEF MAKES V(MPAC) = ROLL, PITCH, YAW" */
    ADDRESS(V1STO2S),           /* */
    STORE(FDAIX),               /* "MODE IS TP" */
    ORDER(EXIT),
};

/* ENDBALL -- LM p. 478. Back at the CADR in BALLEXIT. */
void endball(void)
{
    ca(ADDRESS_OF(BALLEXIT));
    tc_at(ENDBALL + 1u);
    continue_at(BANKJUMP);
}

#else

/* ---- R60, the CM's ---- */

static void three_axis_test(void (*yes)(void), word intpret_at);
static void redomanc(void);
static void endmanu1(void);
static bool chklinus(void);
static void goredo20(void);
static void r61test(void);

/* R60CSM -- CM p. 390. MAKECADR gives the caller's return as a CADR, for
 * TEMPR60. */
void r60csm(void)
{
    tc_at(R60CSM);
    continue_at(MAKECADR);
}

/* R60CSM +1, REDOMANN -- CM p. 390. Unless the 3-axis flag (BIT6 of
 * FLAGWRD5) is set, VECPOINT works out the final angles. (A word with
 * overflow in A would skip the CAF: MAKECADR's CADR has none.) */
void r60csm_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(TEMPR60))) set_A(BIT6); /* REDOMANN */
    three_axis_test(toball, REDOMANN + 4u);
}

/* MASK FLAGWRD5; CCS A: "IS 3-AXIS FLAG SET": yes (the bit, positive), on
 * to yes; no (+0), the TC INTPRET at intpret_at into the CALL of VECPOINT.
 * (A masked flag word is never negative, but for a TS's overflow skip:
 * not modelled.) */
static void three_axis_test(void (*yes)(void), word intpret_at)
{
    mask_at(ADDRESS_OF(FLAGWRD5));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: yes(); return; /* "YES" */
    case IS_PLUS_ZERO:
        tc_at(intpret_at);
        intpret();
        return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* REDOMANN +5 -- CM p. 390. */
static const word redomann_words[] = {
    ORDER(CALL),
    ADDRESS(VECPOINT), /* "TO COMPUTE FINAL ANGLES" */
    STORE(CPHI),       /* "STORE FINAL ANGLES - CPHI,CTHETA,CPSI" */
    ORDER(EXIT),
};

/* TOBALL -- CM p. 390. GOPERF2R, through BANKCALL, puts up verb 50 noun
 * 18 ("DISPLAY PLEASE PERFORM AUTO MANEUVER"), makes an immediate return
 * to the word after its terminate, proceed and enter returns, and comes
 * back to one of those for the crew's answer. (The CM has no BALLANGS
 * here: noun 18 shows the gimbal angles.) */
void toball(void)
{
    set_A(V06N18);
    tc_at(TOBALL + 1u);
    continue_at(BANKCALL); /* CADR GOPERF2R */
}

/* TOBALL +3 -- terminate. */
void toball_terminate(void)
{
    tc_at(TOBALL + 3u);
    r61test();
}

/* TOBALL +4 -- "PROCEED". */
void toball_proceed(void)
{
    tc_at(TOBALL + 4u);
    redomanc();
}

/* TOBALL +5 -- "ENTER I.E. FINISHED WITH R60". */
void toball_enter(void)
{
    endmanu1(); /* TCF ENDMANU1 */
}

/* TOBALL +6 -- GOPERF2R's immediate return: "TO CHECK FOR PRIORITY
 * DISPLAYS", then the job ends, to wait for the crew. */
void toball_immediate_return(void)
{
    tc_at(TOBALL + 6u);
    if (chklinus()) toball_after_chklinus();
}

/* TOBALL +7 */
void toball_after_chklinus(void)
{
    tc_at(TOBALL + 7u);
    continue_at(ENDOFJOB);
}

/* REDOMANC -- CM p. 390. The final angles again, unless the 3-axis flag
 * is set. */
static void redomanc(void)
{
    set_A(BIT6);
    three_axis_test(toballc, REDOMANC + 4u);
}

/* REDOMANC +5 -- CM p. 390. */
static const word redomanc_words[] = {
    ORDER(CALL),
    ADDRESS(VECPOINT), /* "TO COMPUTE FINAL ANGLES" */
    STORE(CPHI),       /* "STORE ANGLES" */
    ORDER(EXIT),
};

/* TOBALLC -- CM pp. 390-391. "IS MODE AUTO AND CTL GNC": channel 31's
 * bits 13 and 14 (PRIO30's), exclusive-or'ed with the bits PRIO30 sets,
 * and bit 15 of it, must all be 0. If so ("AUTO, NON-FLASH N18"), "SET UP
 * NON-FLASHING V06 N18" by GODSPR; if not ("NOT AUTO"), the request again
 * (TOBALL). */
void toballc(void)
{
    set_A(PRIO30);
    rxor_channel(031u);
    mask(BITS13_14_15);
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        toball();       /* TCF TOBALL: "NOT AUTO" */
        return;
    }
    set_A(V06N18);
    tc_at(TOBALLC + 010u);
    continue_at(BANKCALL); /* CADR GODSPR */
}

/* TOBALLC +012 -- the priority display check, then the maneuver. */
void toballc_after_godspr(void)
{
    tc_at(TOBALLC + 012u);
    if (chklinus()) startmnv();
}

/* STARTMNV -- CM p. 391. The maneuver through KALCMANU (GOMANUR). */
void startmnv(void)
{
    tc_at(STARTMNV);
    continue_at(BANKCALL); /* CADR GOMANUR */
}

/* ENDMANUV -- CM p. 391. "FINISHED MANEUVER": the request again. */
void endmanuv(void)
{
    toball(); /* TCF TOBALL */
}

/* ENDMANU1 -- CM p. 391. "RESET 3-AXIS FLAG" ("BIT 6 FLAG 5") and back to
 * the caller, at the CADR in TEMPR60. */
static void endmanu1(void)
{
    tc_at(ENDMANU1);
    continue_at(DOWNFLAG); /* ADRES 3AXISFLG */
}

/* ENDMANU1 +2 */
void endmanu1_after_downflag(void)
{
    ca(ADDRESS_OF(TEMPR60)); /* CAE */
    tc_at(ENDMANU1 + 3u);
    continue_at(BANKJUMP);
}

/* CHKLINUS -- CM p. 391. "IS PRIORITY DISPLAY FLAG SET" (BIT12 of
 * FLAGWRD4)? If not, back to the caller ("NO - EXIT": true). If so, the
 * return is saved in MPAC +2, TBASE1 gets the place for a restart, 3
 * before the Q of the last display that BUF2 "HOLDS", the phase is 1.7
 * ("1.7SPOT FOR RELINUS"), and LINUS makes the display a priority one.
 * (False: the routine has gone to PHASCHNG.) */
static bool chklinus(void)
{
    cs(ADDRESS_OF(FLAGWRD4));
    mask(BIT12);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_q(); return true; /* "NO - EXIT" */
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (a masked bit) */
    }
    ca(Q_ADDRESS);
    if (ts(MPAC_AT(2))) not_modelled(); /* "SAVE RETURN" (an address: no overflow) */
    set_A(negate(THREE));               /* "OBTAIN LOCATION FOR RESTART." */
    ad_at(BUF2_AT(0));                  /* "HOLDS Q OF LAST DISPLAY" */
    if (ts(ADDRESS_OF(TBASE1))) not_modelled();
    tc_at(CHKLINUS + 011u);
    continue_at(PHASCHNG); /* OCT 71 */
    return false;
}

/* CHKLINUS +013 */
void chklinus_after_phaschng(void)
{
    set_A(BIT7);
    tc_at(CHKLINUS + 014u);
    continue_at(LINUS); /* "GO SET BITS FOR PRIORITY DISPLAY" */
}

/* CHKLINUS +015 -- back to CHKLINUS's caller, at the return saved in
 * MPAC +2. */
void chklinus_after_linus(void)
{
    tc_at(CHKLINUS + 015u);
    tc_saved(MPAC_AT(2));
}

/* RELINUS -- CM pp. 391-392. Where the restart comes (phase 1.7) after a
 * priority display. "IS TRACK FLAG ON" (BIT5 of FLAGWRD1)? If not,
 * GOREDO20. If so, the "R60 PRIODSP FLAG" and TARG1FLG ("FOR R52") are
 * set, OPTIND is reset (it "WAS SET TO -1 BY V379"), the job
 * gets its "ORIGINAL PRIORITY" back, and R60 goes on at the place
 * CHKLINUS left in TBASE1. */
void relinus(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(FLAGWRD1));
    if (A_is_zero()) { /* EXTEND; BZF GOREDO20: "NO" */
        goredo20();
        return;
    }
    tc_at(RELINUS + 4u);
    continue_at(UPFLAG); /* ADRES PDSPFLAG */
}

/* RELINUS +6 */
void relinus_after_upflag(void)
{
    tc_at(RELINUS + 6u);
    continue_at(UPFLAG); /* ADRES TARG1FLG */
}

/* RELINUS +010 -- (A is +0, so the TS never skips.) */
void relinus_after_targ1flg(void)
{
    set_A(ZERO);                                /* "RESET TO ZERO, SINCE */
    if (!ts(ADDRESS_OF(OPTIND))) set_A(PRIO14); /* OPTIND WAS SET TO -1 BY V379" */
    tc_at(RELINUS + 013u);
    continue_at(PRIOCHNG); /* "RESTORE ORIGINAL PRIORITY" */
}

/* RELINUS +014 */
void relinus_after_priochng(void)
{
    tc_at(RELINUS + 014u);
    tc_saved(ADDRESS_OF(TBASE1));
}

/* GOREDO20 -- CM p. 392. The phase back to P20's: "1.11 FOR PIKUP20". */
static void goredo20(void)
{
    tc_at(GOREDO20);
    continue_at(PHASCHNG); /* OCT 111 */
}

/* GOREDO20 +2 */
void goredo20_after_phaschng(void)
{
    tc_at(GOREDO20 + 2u);
    continue_at(ENDOFJOB);
}

/* R61TEST -- CM p. 392. Terminate: "ARE WE IN P00. IF YES THIS MUST BE
 * VERB49 OR VERB89 SO DO ENDEXT." ("RESET 3-AXIS & RUTURN. USER DOES
 * ENDEXT", ENDMANU1); otherwise "ARE WE IN R61 (P20)": the priority
 * display flag says so, and GOTOV56 ends it; if not, GOTOPOOH ends the
 * program. */
static void r61test(void)
{
    ca(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF ENDMANU1 */
        endmanu1();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD4));
    mask(BIT12);
    if (A_is_zero()) { /* EXTEND; BZF GOTOPOOH: "NO" */
        continue_at(GOTOPOOH);
        return;
    }
    tc_at(R61TEST + 7u);
    continue_at(GOTOV56); /* "YES" */
}

#endif

/* ROT180 -- LM p. 483, CM p. 396. "IF VF, VI ANTIPARALLEL, 180 DEG
 * ROTATION": the turn is 180 degrees (HIDPHALF) about the unit of (MIS +6 x X) x SCAXIS, an
 * axis at right angles to SCAXIS in the plane of the spacecraft X axis and
 * the stable member Y axis seen from the spacecraft (MIS +6 is that row);
 * if that cross product is too small, about X itself (PICKX). The CM's
 * listing names its X unit vector HIUNITX, the same word as HIDPHALF. */
#define ROT180_WORDS(unitx)                                                                        \
    /* ROT180 */ ORDERS(VLOAD, VXV), ADDRESS_PLUS_ONE_AT(MIS, 6), ADDRESS_PLUS_ONE(unitx),         \
        ORDERS(UNIT, VXV), ADDRESS_PLUS_ONE(SCAXIS), ORDERS(UNIT, BOV), ADDRESS(PICKX),            \
        STODL(COF), ADDRESS_PLUS_ONE_D(36), ORDERS(DSU, BMN), ADDRESS_PLUS_ONE(DPB_14),            \
        ADDRESS(PICKX), ORDER(VLOAD), ADDRESS_PLUS_ONE(COF), /* XROT */ STODL(COF),                \
        ADDRESS_PLUS_ONE(HIDPHALF), ORDER(GOTO), ADDRESS(COMPMATX),                                \
        /* PICKX */ ORDERS(VLOAD, GOTO), ADDRESS_PLUS_ONE(unitx), ADDRESS(XROT)
#define ROT180_WORDS_CM ROT180_WORDS(HIUNITX)

/* ---- VECPOINT (both missions) ---- */

/* VECPOINT -- LM pp. 480-483, CM pp. 393-396. "THIS INTERPRETIVE
 * SUBROUTINE MAY BE USED TO POINT A SPACECRAFT AXIS IN A DESIRED
 * DIRECTION": SCAXIS, the axis in spacecraft coordinates, and POINTVSM,
 * the direction in stable member coordinates, both half-unit vectors. It
 * "COMPUTES A SET OF THREE GIMBAL ANGLES (2S COMPLEMENT) CORRESPONDING TO
 * THE CROSS-PRODUCT ROTATION BETWEEN SCAXIS AND POINTVSM" and returns them
 * in MPAC to MPAC +2 (outer, inner, middle).
 *
 * From the present CDU angles (READCDUK; the LM's VECPNT1 takes the
 * desired ones, CDUD, in MPAC instead), MIS, the spacecraft axes in stable
 * member axes, and the direction VF in them. The rotation from VI
 * (SCAXIS) to VF is about COF, VF x VI, by the angle whose cosine is VF .
 * VI; DELCOMP's matrix for it, times MIS, is MFS, the final spacecraft
 * axes. If the final middle gimbal angle is under 59 degrees, or the axis
 * is within 49.4 degrees of the thrust axis (X), those are the angles
 * (FINDGIMB). Otherwise the attitude is turned about SCAXIS out of gimbal
 * lock, the short way: 50 degrees when pointing the AOT (the axis within
 * 29.5 degrees of X... "SEE IF WE ARE POINTING THE AOT"), else 35. When VF
 * and VI are parallel (the cross product under 2^-14), the present angles
 * are the answer; antiparallel, a 180-degree turn (ROT180) about an axis
 * in the plane of X and the stable member's Y, or X itself. DCMTOCDU takes
 * the angles from the matrix, V1STO2S makes them two's complement, and
 * GOTO VECQTEMP returns. */
static const word vecpoint_words[] = {
#if LUMINARY099
    /* VECPNT1 */ ORDERS(STQ, BOV), /* "THIS ENTRY USES DESIRED CDUS */
    ADDRESS(VECQTEMP),              /* NOT PRESENT-ENTER WITH CDUD'S IN MPAC" */
    ADDRESS(VECPNT2),
    /* VECPNT2 */ ORDERS(AXC_2, GOTO),
    ADDRESS(MIS),
    ADDRESS(STORANG),
#endif
    /* VECPOINT */ ORDERS(STQ, BOV), /* "SAVE RETURN ADDRESS" */
    ADDRESS(VECQTEMP),
    ADDRESS(VECLEAR), /* "AND CLEAR OVFIND" */
    /* VECLEAR */ ORDERS(AXC_2, RTB),
    ADDRESS(MIS),      /* "READ THE PRESENT CDU ANGLES AND */
    ADDRESS(READCDUK), /* STORE THEM IN PD25, 26, 27" */
    /* STORANG */ STCALL_D(25),
    ADDRESS(CDUTODCM), /* "S/C AXES TO STABLE MEMBER AXES (MIS)" */
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(POINTVSM), /* "RESOLVE THE POINTING DIRECTION VF INTO */
    ADDRESS_PLUS_ONE(MIS),      /* INITIAL S/C AXES ( VF = POINTVSM)" */
    ORDER(UNIT),
    STORE_D(28),              /* "PD 28 29 30 31 32 33" */
    ORDERS(VXV, UNIT),        /* "TAKE THE CROSS PRODUCT VF X VI */
    ADDRESS_PLUS_ONE(SCAXIS), /* WHERE VI = SCAXIS" */
    ORDERS(BOV, VCOMP),
    ADDRESS(PICKAXIS),
    STODL(COF),               /* "CHECK MAGNITUDE */
    ADDRESS_PLUS_ONE_D(36),   /* OF CROSS PRODUCT */
    ORDERS(DSU, BMN),         /* VECTOR, IF LESS */
    ADDRESS_PLUS_ONE(DPB_14), /* THAN B-14 ASSUME */
    ADDRESS(PICKAXIS),        /* UNIT OPERATION */
    ORDERS(VLOAD, DOT),       /* INVALID." */
    ADDRESS_PLUS_ONE(SCAXIS),
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(SL1, ARCCOS),
    /* COMPMATX */ ORDER(CALL), /* "NOW COMPUTE THE TRANSFORMATION FROM */
    ADDRESS(DELCOMP),           /* FINAL S/C AXES TO INITIAL S/C AXES MFI" */
    ORDERS(AXC_1, AXC_2),
    ADDRESS(MIS), /* "COMPUTE THE TRANSFORMATION FROM FINAL */
#if LUMINARY099
    ADDRESS(KEL), /* S/C AXES TO STABLE MEMBER AXES" */
#else
    ADDRESS(DEL), /* S/C AXES TO STABLE MEMBER AXES" */
#endif
    ORDER(CALL),   /* "MFS = MIS MFI */
    ADDRESS(MXM3), /* (IN PD LIST)" */
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE_D(6), /* "MFS6 = SIN(CPSI)" */
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(SINGIMLC), /* "= SIN(59 DEGS)" */
    ADDRESS(FINDGIMB),          /* "/CPSI/ LESS THAN 59 DEGS" */
    ORDERS(DLOAD, ABS),         /* "CHECK TO SEE IF WE ARE POINTING */
    ADDRESS_PLUS_ONE(SCAXIS),   /* THE THRUST AXIS" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(SINVEC1), /* "SIN 49.4 DEGS" */
    ADDRESS(FINDGIMB),         /* "IF SO, WE ARE TRYING TO POINT IT INTO */
    ORDER(VLOAD),              /* GIMBAL LOCK, ABORT COULD GO HERE" */
    ORDER(STADR),              /* "STORE MFS (IN PD LIST) IN MIS" */
    AFTER_STADR(STOVL_AT(MIS, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(MIS, 6)),
    ORDER(STADR),
    AFTER_STADR(STOVL(MIS)),
    ADDRESS_PLUS_ONE_AT(MIS, 6),    /* "INNER GIMBAL AXIS IN FINAL S/C AXES" */
    ORDERS(BPL, VCOMP),             /* "LOCATE THE IG AXIS DIRECTION CLOSEST TO */
    ADDRESS(IGSAMEX),               /* FINAL X S/C AXIS" */
    /* IGSAMEX */ ORDERS(VXV, BMN), /* "FIND THE SHORTEST WAY OF ROTATING THE */
    ADDRESS_PLUS_ONE(SCAXIS),       /* S/C OUT OF GIMBAL LOCK BY A ROTATION */
    ADDRESS(U_EQ_SCAXIS),           /* ABOUT +- SCAXIS" */
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(SCAXIS),
    STCALL(COF), /* "ROTATE ABOUT -SCAXIS" */
    ADDRESS(CHEKAXIS),
    /* U=SCAXIS */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(SCAXIS),
    STORE(COF), /* "ROTATE ABOUT + SCAXIS" */
    /* CHEKAXIS */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(SCAXIS), /* "SEE IF WE ARE POINTING THE AOT" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(SINVEC2), /* "SIN 29.5 DEGS" */
    ADDRESS(PICKANG1),         /* "IF SO, ROTATE 50 DEGS ABOUT +- SCAXIS" */
    ORDERS(DLOAD, GOTO),       /* "IF NOT, MUST BE POINTING THE TRANSPONDER */
    ADDRESS_PLUS_ONE(VECANG2), /* OR SOME VECTOR IN THE Y, OR Z PLANE */
    ADDRESS(COMPMFSN),         /* IN THIS CASE ROTATE 35 DEGS TO GET OUT" */
    /* PICKANG1 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(VECANG1), /* "= 50 DEGS" */
    /* COMPMFSN */ ORDER(CALL),
    ADDRESS(DELCOMP),     /* "COMPUTE THE ROTATION ABOUT SCAXIS TO */
    ORDERS(AXC_1, AXC_2), /* BRING MFS OUT OF GIMBAL LOCK" */
    ADDRESS(MIS),
#if LUMINARY099
    ADDRESS(KEL),
#else
    ADDRESS(DEL),
#endif
    ORDER(CALL),   /* "COMPUTE THE NEW TRANSFORMATION FROM */
    ADDRESS(MXM3), /* DESIRED S/C AXES TO STABLE MEMBER AXES" */
    /* FINDGIMB */ ORDERS(AXC_1, CALL),
    ADDRESS_D(0),      /* "EXTRACT THE COMMANDED CDU ANGLES FROM */
    ADDRESS(DCMTOCDU), /* THIS MATRIX" */
    ORDERS(RTB, SETPD),
    ADDRESS(V1STO2S), /* "CONVERT TO 2:S COMPLEMENT" */
    ADDRESS_PLUS_ONE_D(0),
    ORDER(GOTO),
    ADDRESS(VECQTEMP),                 /* "RETURN TO CALLER" */
    /* PICKAXIS */ ORDERS(VLOAD, DOT), /* "IF VF X VI = 0, FIND VF . VI" */
    ADDRESS_PLUS_ONE_D(28),
    ADDRESS_PLUS_ONE(SCAXIS),
    ORDERS(BMN, TLOAD),
    ADDRESS(ROT180),
    ADDRESS_PLUS_ONE_D(25),
    ORDER(GOTO),       /* "IF VF = VI, CDU DESIRED = PRESENT CDU */
    ADDRESS(VECQTEMP), /* PRESENT CDU ANGLES" */
#if !LUMINARY099
    ROT180_WORDS_CM,
#endif
};

#if LUMINARY099
#define R60_BANK       026u
#define VECPOINT_FIRST 02032u /* VECPNT1 */

/* The LM's ROT180, in bank 26 with R60. */
static const word rot180_words[] = {ROT180_WORDS(HIDPHALF)};

const struct interpretive_code BALLANGS_CODE = {026u, BALLANGS + 011u, COUNT(ballangs_words),
                                                ballangs_words};
const struct interpretive_code ROT180_CODE = {026u, (word)(02000u | (I_ROT180 & 01777u)),
                                              COUNT(rot180_words), rot180_words};
#else
#define R60_BANK       027u
#define VECPOINT_FIRST 02126u /* VECPOINT */
#endif

const struct interpretive_code REDOMANN_CODE = {R60_BANK, REDOMANN + 5u, COUNT(redomann_words),
                                                redomann_words};
const struct interpretive_code REDOMANC_CODE = {R60_BANK, REDOMANC + 5u, COUNT(redomanc_words),
                                                redomanc_words};
const struct interpretive_code VECPOINT_CODE = {027u, VECPOINT_FIRST, COUNT(vecpoint_words),
                                                vecpoint_words};
