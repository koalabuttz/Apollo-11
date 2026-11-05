/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc, pp. 845-851 (Luminary099 only, bank 30): ASCENT,
 * the ascent guidance equations, and ASCTERM, the end of each cycle. The
 * faithful C is c64-port/src/lm_ascent.c.
 *
 * The frame: UNIT/R/ up (the unit position), ZAXIS1 down-range (the unit
 * normal to the plane of QAXIS, the CSM's orbit normal, and UNIT/R/, as
 * P12's NEWLOAD sets it), and LAXIS = ZAXIS1 x UNIT/R/, across the plane.
 * The targets: RDOTD, YDOTD and ZDOTD, the velocity's three components at
 * injection, and RCO and YCO, its radius and out-of-plane distance. */
#include "ascent.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- ASCENT -- pp. 845-850 ---- */

/* The state in the frame: /R/MAG, the radius; ZDOT and YDOT, the velocity
 * down-range and across (and Y, the distance across, from YCOMP); GEFF,
 * the effective gravity, the gravity's radial part less the centrifugal
 * term H^2/R^3 (H the angular momentum):
 *
 *     GEFF = G.UR + |UR x V|^2 / /R/MAG
 *
 * The velocity to be gained, compensated for gravity over TGO:
 *
 *     VGVECT = DRDOT UR + DYDOT L + DZDOT Z - TGO GEFF UR
 *
 * (DRDOT = RDOTD - RDOT, DYDOT = YDOTD - YDOT, DZDOT = ZDOTD - ZDOT), and
 * VGBODY, VGVECT in the LM's axes (XNBPIP) for noun 85. With FLRCS set
 * (the RCS trims the end of the burn), TGO = |VG| / AT/RCS and the cycle
 * ends (ASCTERM2). MAINENG, on the engine: the time to go,
 *
 *     TGO = TBUP (VG / VE) (1 - KT1 VG / VE) - TTO
 *
 * (TTO for the thrust's tailoff), and TTOGO -TGO for the displays. With
 * IDLEFLAG clear and less than 4 seconds to go, ENGOFF. T2TEST: with less
 * than 2 seconds to go (T2A), the steering's constants stay as they are
 * (CMPONENT). Else L, -LOG(1 - TGO / TBUP) by LOGSUB, and D12 from it;
 * with FLPC set, or once less than 10 seconds are left (T3, which sets
 * FLPC), no rates: PRATE and YRATE zero (NORATES). RATES: the pitch and
 * yaw rates B and D that bring the radius and the distance across to
 * their targets (DR = RCO - R - RDOT TGO, DY = YCO - Y - YDOT TGO):
 *
 *     PRATE = B = (D21 DRDOT - DR) / (E TGO)
 *     YRATE = D = (D21 DYDOT - DY) / (E TGO)
 *
 * (D21 = TGO - D12, E = TGO / 2 - D21). "B>0 NOT PERMITTED" (B becomes 0);
 * CHKBMAG holds B / TBUP no lower than PRLIMIT. CONST: the constants
 *
 *     PCONS = A = -DRDOT / L - D12 B
 *     YCONS = C = -DYDOT / L - D12 D
 *
 * CMPONENT: the thrust's components, 100CS on (the T - T0 of the
 * listing's comments):
 *
 *     ATR = (A + B 100CS) / TBUP - GEFF   (up)
 *     ATY = (C + D 100CS) / TBUP          (across)
 *     ATP = SQRT(AT^2 - AH^2)             (down-range, AH = ATY L + ATR UR)
 *
 * with ATP the sign of DZDOT; where AH is more than AT, AH scaled down to
 * AT and ATP zero (NO-ATP). UNFC/2, the thrust's direction, is AH + ATP
 * ZAXIS1. Then on P12's first pass (FLPI) back to P12RET; in the vertical
 * rise (FLVR) to CHECKALT. MAINLINE: UNWC/2, the attitude's second
 * direction, -UNIT/R/; until TXO, ASCTERM; with ROTFLAG set, ANG1CHEK;
 * then CLRXFLAG: NOR29FLG and XOVINFLG clear, and ASCTERM. */
static const word ascent_words[] = {
    /* ASCENT */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(R),
    STOVL(_R_MAG),
    ADDRESS_PLUS_ONE(ZAXIS1),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(V), /* "Z.V = ZDOT*2(-8)." */
    STOVL(ZDOT),         /* "ZDOT*2(-7)" */
    ADDRESS_PLUS_ONE(ZAXIS1),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UNIT_R_), /* "Z X UR = LAXIS*2(-2)" */
    STORE(LAXIS),              /* "LAXIS*2(-1)" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(V), /* "L.V = YDOT*2(-8)." */
    STCALL(YDOT),        /* "YDOT * 2(-7)" */
    ADDRESS(YCOMP),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(GDT1_2), /* "LOAD GDT1/2*2(-7) M/CS." */
    ORDERS(V_SC, DOT),
    ADDRESS_PLUS_ONE(2SEC_18),
    ADDRESS_PLUS_ONE(UNIT_R_), /* "G.UR*2(9) = GR*2(9)." */
    ORDERS(PDVL, VXV),         /* "STORE IN PDL(0)" */
    ADDRESS_PLUS_ONE(UNIT_R_), /* "LOAD UNIT/R/ *2(-1)" */
    ADDRESS_PLUS_ONE(V),       /* "UR*2(-1) X V*2(-7) = H/R*2(-8)." */
    ORDERS(VSQ, DDV),          /* "H(2)/R(2)*2(-16)." */
    ADDRESS_PLUS_ONE(_R_MAG),  /* "H(2)/R(3)*2(9)." */
    ORDERS(SL1, DAD),
    ORDER(STADR),
    AFTER_STADR(STODL(GEFF)),
    ADDRESS_PLUS_ONE(ZDOTD),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(ZDOT),
    STORE(DZDOT), /* "DZDOT = (ZDOTD - ZDOT) * 2(7) M/CS." */
    ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(ZAXIS1),
    ADDRESS_PLUS_ONE(YDOTD),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(YDOT),
    STORE(DYDOT), /* "DYDOT = (YDOTD - YDOT) *2(7) M/CS." */
    ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(LAXIS),
    ADDRESS_PLUS_ONE(RDOTD),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(RDOT),
    STORE(DRDOT), /* "DRDOT = (RDOTD - RDOT) * 2(7) M/CS." */
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VAD, VSL1),
    ORDER(STADR),
    AFTER_STADR(STORE(VGVECT)),
    ORDERS(DLOAD, DMP),    /* "LOAD TGO" */
    ADDRESS_PLUS_ONE(TGO), /* "TGO GEFF" */
    ADDRESS_PLUS_ONE(GEFF),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UNIT_R_), /* "TGO GEFF UR" */
    ORDER(BVSU),
    ADDRESS_PLUS_ONE(VGVECT), /* "COMPENSATED FOR GEFF" */
    STORE(VGVECT),            /* "STORE FOR DOWNLINK" */
    ORDERS(MXV, VSL1),        /* "GET VGBODY FOR N85 DISPLAY" */
    ADDRESS_PLUS_ONE(XNBPIP),
    STOVL(VGBODY),
    ADDRESS_PLUS_ONE(VGVECT),
    ORDERS(ABVAL, BOFF), /* "MAGNITUDE OF VGVECT" */
    SWITCH(BOFF, FLRCS), /* "IF FLRCS=0,D0 NORMAL GUIDANCE" */
    ADDRESS(MAINENG),
    ORDER(DDV), /* "USE TGO=VG/AT WITH RCS" */
    ADDRESS_PLUS_ONE(AT_RCS),
    STCALL(TGO), /* "THIS WILL BE USED ON NEXT CYCLE" */
    ADDRESS(ASCTERM2),
    /* MAINENG */ ORDERS(DDV, PUSH), /* "VG/VE IN PDL(0)" */
    ADDRESS_PLUS_ONE(VE),
    ORDERS(DMP, BDSU), /* "1 - KT VG/VE" */
    ADDRESS_PLUS_ONE(KT1),
    ADDRESS_PLUS_ONE(NEARONE),
    ORDERS(DMP, DMP),       /* "TBUP VG(1-KT VG/VE)/VE" */
    ADDRESS_PLUS_ONE(TBUP), /* "= TGO" */
    ORDER(DSU),             /* "COMPENSATE FOR TAILOFF" */
    ADDRESS_PLUS_ONE(TTO),
    STORE(TGO),
    ORDERS(SR, DCOMP),
    SR_COUNT(11),
    STODL(TTOGO), /* "TGO *2(-28) CS" */
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(BON, DSU),
    SWITCH(BON, IDLEFLAG),
    ADDRESS(T2TEST),
    ADDRESS_PLUS_ONE(4SEC_17), /* "( TGO - 4 )*2(-17) CS." */
    ORDER(BMN),
    ADDRESS(ENGOFF),
    /* T2TEST */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DSU, BMN), /* "IF TGO - T2 NEG., GO TO CMPONENT" */
    ADDRESS_PLUS_ONE(T2A),
    ADDRESS(CMPONENT),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TBUP),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DDV, CALL), /* "1- TGO/TBUP" */
    ADDRESS_PLUS_ONE(TBUP),
    ADDRESS(LOGSUB),
    ORDERS(SL, PUSH), /* "-L IN PDL(0)" */
    SL_COUNT(5),
    ORDERS(BDDV, BDSU), /* "-TGO/L*2(-17)" */
    ADDRESS_PLUS_ONE(TGO),
    ADDRESS_PLUS_ONE(TBUP), /* "TBUP + TGO/L = D12*2(-17)" */
    ORDERS(PUSH, BON),      /* "STORE IN PDL(2)" */
    SWITCH(BON, FLPC),      /* "IF FLPC = 1, GO TO CONST" */
    ADDRESS(NORATES),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TGO),
    ADDRESS_PLUS_ONE(T3),
    ORDERS(BPL, SET), /* "FLPC=1" */
    ADDRESS(RATES),
    SWITCH(SET, FLPC),
    /* NORATES */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STORE(PRATE), /* "B = 0" */
    STORE(YRATE), /* "D = 0" */
    ORDER(GOTO),
    ADDRESS(CONST), /* "GO TO CONST" */
    /* RATES */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TGO),
    ADDRESS_PLUS_ONE_D(2), /* "TGO - D12 = D21*2(-17)" */
    ORDERS(PUSH, SL1),     /* "IN PDL(4)" */
    ORDERS(BDSU, SL3),     /* "(1/2TGO - D21)*2(-13) = E * 2(-13)" */
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(PDDL, DMP), /* "IN PDL(6)" */
    ADDRESS_PLUS_ONE(TGO),
    ADDRESS_PLUS_ONE(RDOT),   /* "RDOT TGO * 2(-24)" */
    ORDERS(DAD, DSU),         /* "R + RDOT TGO" */
    ADDRESS_PLUS_ONE(_R_MAG), /* "R + RDOT TGO - RCO" */
    ADDRESS_PLUS_ONE(RCO),    /* "MPAC = -DR *2(-24)." */
    ORDERS(PDDL, DMP),        /* "-DR IN PDL(8)" */
    ADDRESS_PLUS_ONE(DRDOT),
    ADDRESS_PLUS_ONE_D(4), /* "D21 DRDOT*2(-24)" */
    ORDERS(DAD, SL2),      /* "(D21 DRDOT-DR)*2(-22)" */
    ORDERS(DDV, DDV),
    ADDRESS_PLUS_ONE_D(6), /* "(D21 DRDOT-DR)/E*2(-9)" */
    ADDRESS_PLUS_ONE(TGO),
    STORE(PRATE),       /* "B * 2(8)" */
    ORDERS(BMN, DLOAD), /* "B>0 NOT PERMITTED" */
    ADDRESS(CHKBMAG),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STCALL(PRATE),
    ADDRESS(PROK),
    /* CHKBMAG */ ORDERS(SR4, DDV), /* "B*2(4)" */
    ADDRESS_PLUS_ONE(TBUP),         /* "(B / TAU) * 2(21)" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(PRLIMIT),
    ADDRESS(PROK),
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(PRLIMIT),
    ADDRESS_PLUS_ONE(TBUP), /* "B MAX. * 2(4)" */
    ORDER(SL4),             /* "BMAX*2(8)" */
    STORE(PRATE),
    /* PROK */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DMP, DAD), /* "YDOT TGO" */
    ADDRESS_PLUS_ONE(YDOT),
    ADDRESS_PLUS_ONE(Y), /* "Y + YDOT TGO" */
    ORDERS(DSU, PDDL),   /* "Y + YDOT TGO - YCO" */
    ADDRESS_PLUS_ONE(YCO),
    ADDRESS_PLUS_ONE(DYDOT),
    ORDERS(DMP, DAD), /* "D21 DYDOT - DY" */
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(SL2, DDV),   /* "(D21 DYDOT - DY)/E*2(-9)" */
    ORDERS(DDV, SETPD), /* "(D21 DYDOT - DY)/E TGO*2(8)" */
    ADDRESS_PLUS_ONE(TGO),
    ADDRESS_PLUS_ONE_D(4),
    STORE(YRATE),
    /* CONST */ ORDERS(DLOAD, DMP), /* "LOAD B*2(8)" */
    ADDRESS_PLUS_ONE(PRATE),        /* "B D12*2(-9)" */
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(PDDL, DDV),       /* "D12 B IN PDL(4)" */
    ADDRESS_PLUS_ONE(DRDOT), /* "LOAD DRDOT*2(-7)" */
    ADDRESS_PLUS_ONE_D(0),   /* "-DRDOT/L*2(-7)" */
    ORDERS(SR2, DSU),        /* "(-DRDOT/L-D12 B)=A*2(-9)" */
    ORDER(STADR),
    AFTER_STADR(STODL(PCONS)),
    ADDRESS_PLUS_ONE(YRATE), /* "D*2(8)" */
    ORDERS(DMP, PDDL),       /* "D12 D,EXCH WITH -L IN PDL(0)" */
    ORDERS(BDDV, SR2),       /* "-DYDOT/L*2(-9)" */
    ADDRESS_PLUS_ONE(DYDOT),
    ORDER(DSU), /* "(-DYDOT/L-D12 D)=C*2(-9)" */
    ADDRESS_PLUS_ONE_D(0),
    STORE(YCONS),
    /* CMPONENT */ ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(100CS),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(PRATE), /* "B(T-T0)*2(-9)" */
    ORDERS(DAD, DDV),        /* "(A+B(T-T0))*2(-9)" */
    ADDRESS_PLUS_ONE(PCONS), /* "(A+B(T-T0))/TBUP*2(8)" */
    ADDRESS_PLUS_ONE(TBUP),
    ORDERS(SL1, DSU),
    ADDRESS_PLUS_ONE(GEFF), /* "ATR*2(9)" */
    STODL(ATR),
    ADDRESS_PLUS_ONE(100CS),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(YRATE),
    ADDRESS_PLUS_ONE(YCONS), /* "(C+D(T-T0))*2(-9)" */
    ORDERS(DDV, SL1),
    ADDRESS_PLUS_ONE(TBUP),
    STORE(ATY),         /* "ATY*2(9)" */
    ORDERS(VXSC, PDDL), /* "ATY UY*2(8)" */
    ADDRESS_PLUS_ONE(LAXIS),
    ADDRESS_PLUS_ONE(ATR),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VSL1, PUSH),     /* "AH*2(9) IN PDL(0)" */
    ORDERS(ABVAL, PDDL),    /* "AH(2) IN PDL(34)" */
    ADDRESS_PLUS_ONE(AT),   /* "AHMAG IN PDL(6)" */
    ORDERS(DSQ, DSU),       /* "(AT(2)-AH(2))*2(18)" */
    ADDRESS_PLUS_ONE_D(34), /* "=ATP2*2(18)" */
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(AT),
    ORDERS(DSQ, DSU),       /* "(AT(2)KR(2)-AH(2))*2(18)" */
    ADDRESS_PLUS_ONE_D(34), /* "=ATP3*2(18)" */
    ORDERS(BMN, DLOAD),     /* "IF ATP3 NEG,GO TO NO-ATP" */
    ADDRESS(NO_ATP),        /* "LOAD ATP2, IF ATP3 POS" */
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(SQRT, GOTO), /* "ATP*2(9)" */
    ADDRESS(AIMER),
    /* NO-ATP */ ORDERS(DLOAD, BDDV), /* "KR AT/AH = KH" */
    ADDRESS_PLUS_ONE_D(6),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE_D(0),
    STODL_D(0), /* "STORE NEW AH IN PDL(0)" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* AIMER */ ORDER(SIGN),
    ADDRESS_PLUS_ONE(DZDOT),
    STORE(ATP),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(ZAXIS1), /* "ATP ZAXIS *2(8)." */
    ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE_D(0),
    STORE(UNFC_2), /* "WILL BE OVERWRITTEN IF IN VERT. RISE." */
    ORDERS(SETPD, BON),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(BON, FLPI),
    ADDRESS(P12RET),
    ORDER(BON),
    SWITCH(BON, FLVR),
    ADDRESS(CHECKALT),
    /* MAINLINE */ ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(UNIT_R_),
    STODL(UNWC_2),
    ADDRESS_PLUS_ONE(TXO),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(PIPTIME),
    ADDRESS(ASCTERM),
    ORDER(BON),
    SWITCH(BON, ROTFLAG),
    ADDRESS(ANG1CHEK),
    /* CLRXFLAG */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, NOR29FLG), /* "START r29 IN ASCENT PHASE." */
    SWITCH(CLEAR, XOVINFLG), /* "ALLOW X-AXIS OVERRIDE" */
    /* ASCTERM */ ORDER(EXIT),
};

const struct interpretive_code ASCENT_CODE = {030u, ASCENT, COUNT(ascent_words), ascent_words};

/* ---- ASCTERM -- p. 850 ---- */

/* After ASCTERM's EXIT: in the RCS trim (FLRCS), the job ends (ASCTERM3);
 * else the attitude commands (ASCTERM_CALL_CODE). (After MASK, A is +0 or
 * the flag's bit.) */
void ascterm_after_exit(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(FLRCSBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        ascterm3();
        return;
    }
    tc_at(ASCTERM + 5u);
    intpret();
}

/* CALL FINDCDUW -2: the CDU angles the DAP steers to, for the thrust's
 * direction UNFC/2 and UNWC/2 (FINDCDUW--GUIDAP_INTERFACE.agc). Then
 * ASCTERM1: EXIT. */
static const word ascterm_call_words[] = {
    /* ASCTERM +6 */ ORDER(CALL),
    ADDRESS_AT(FINDCDUW, -2),
    /* ASCTERM1 */ ORDER(EXIT),
};

const struct interpretive_code ASCTERM_CALL_CODE = {030u, ASCTERM + 6u, COUNT(ascterm_call_words),
                                                    ascterm_call_words};

/* ASCTERM1 +1: "INSURE THAT THE NOUN 63 DISPLAY IS BYPASSED IF WE ARE IN
 * THE RCS TRIMMING MODE OF OPERATION" (FLRCS), and "BYPASS DISPLAYS IF
 * ENGINE FAILURE IS INDICATED." (FLUNDISP); else verb 06 noun 63 (GODSPR,
 * through BANKCALL). (After each MASK, A is +0 or the flag's bit.) */
void ascterm1_after_exit(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(FLRCSBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        ascterm3();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD8));
    mask(FLUNDBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        ascterm3();
        return;
    }
    set_A(V06N63_STAR);
    tc_to(ASCTERM1 + 012u, BANKCALL); /* CADR GODSPR */
}

/* GODSPR's return: ASCTERM3. */
void ascterm1_after_godspr(void)
{
    ascterm3();
}

/* ASCTERM2: EXIT (the RCS trim's TGO is set). */
static const word ascterm2_words[] = {
    /* ASCTERM2 */ ORDER(EXIT),
};

const struct interpretive_code ASCTERM2_CODE = {030u, ASCTERM2, COUNT(ascterm2_words),
                                                ascterm2_words};

/* ASCTERM3: the cycle's job ends (ENDOFJOB). */
void ascterm3(void)
{
    continue_at(ENDOFJOB);
}

/* ASCTERM4 (from ATMAG, the engine not yet thrusting): EXIT; ... */
static const word ascterm4_words[] = {
    /* ASCTERM4 */ ORDER(EXIT),
};

const struct interpretive_code ASCTERM4_CODE = {030u, ASCTERM4, COUNT(ascterm4_words),
                                                ascterm4_words};

/* ... "NO GUIDANCE THIS CYCLE -- HENCE ZERO THE DAP COMMANDED ERRORS."
 * (ZATTEROR, through IBNKCALL, interrupts held), ... */
void ascterm4_after_exit(void)
{
    inhint();
    tc_to(ASCTERM4 + 2u, IBNKCALL); /* CADR ZATTEROR */
}

/* ... and ASCTERM1 +1, the display. */
void ascterm4_after_zatteror(void)
{
    ascterm1_after_exit();
}

/* ---- CHECKALT -- pp. 850-851 ---- */

/* CHECKALT, in the vertical rise: "IF H LT 25K CHECK Z AXIS ORIENTATION"
 * (the height, /R/MAG less /LAND/, under 25KFT: CHECKYAW). EXITVR: the
 * vertical rise over (FLVR clear); with ROTFLAG set, MAINLINE; else TXO,
 * ten seconds on (10SECS), and MAINLINE. EXITVR1: ROTFLAG clear, and
 * EXITVR. */
static const word checkalt_words[] = {
    /* CHECKALT */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(_R_MAG),
    ADDRESS_PLUS_ONE(_LAND_),
    ORDERS(DSU, BMN), /* "IF H LT 25K CHECK Z AXIS ORIENTATION" */
    ADDRESS_PLUS_ONE(25KFT),
    ADDRESS(CHECKYAW),
    /* EXITVR */ ORDERS(CLEAR, BON),
    SWITCH(CLEAR, FLVR),
    SWITCH(BON, ROTFLAG),
    ADDRESS(MAINLINE),
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(PIPTIME),
    ADDRESS_PLUS_ONE(10SECS),
    STCALL(TXO),
    ADDRESS(MAINLINE),
    /* EXITVR1 */ ORDER(CLRGO),
    SWITCH(CLRGO, ROTFLAG),
    ADDRESS(EXITVR),
};

const struct interpretive_code CHECKALT_CODE = {030u, CHECKALT, COUNT(checkalt_words),
                                                checkalt_words};

#endif
