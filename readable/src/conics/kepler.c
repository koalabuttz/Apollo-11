/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, KEPLER: KEPLERN to KEPCONVG (LM pp. 1174-1179,
 * CM pp. 1277-1282), interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * "GIVEN AN INITIAL STATE VECTOR AND THE DESIRED TRANSFER TIME THROUGH
 * WHICH THE STATE IS TO BE UPDATED ALONG A CONIC TRAJECTORY, COMPUTES THE
 * NEW, UPDATED STATE VECTOR" (p. 1160). In: RRECT and VRECT, the state;
 * X1, the central body ("-2D OR -10D ACCORDING TO WHETHER THE EARTH OR
 * MOON"); TAU., the time ("MAY BE POS OR NEG"); XKEPNEW, a guess of X, the
 * root of Kepler's equation; TC and XPREV, the last solution's time and X.
 * Out: RCV and VCV, the new state; TC and XPREV. The caller does not come
 * here directly: CSMCONIC or LEMCONIC does, "WHICH GUARANTEES NO
 * INTERRUPTS", with KEPRTN its return.
 *
 * The method is the universal variable X: Kepler's equation gives the
 * time T for an X (DELTIME), and the iteration looks for the X whose T is
 * TAU., by secants within bounds that close in on it, at most 20 times
 * (ITERCTR). */
#include "conics.h"
#include "../agc/names.h"

static const word kepler_words[] = {
    /* KEPLERN -- LM p. 1174 (CM p. 1277): the start. SETPD 0 and BOV +1
     * (which clears the overflow indicator, "CLEAR OVFIND AT KEPLER
     * ENTRY"); the central body's MUTABLE entry, MU and its root and their
     * inverses (VLOAD* MUTABLE,1, X1 -2D for the earth, -10D for the
     * moon), to 14D; ITERCTR 20; URRECT, the unit initial position, and
     * R1 its length. Then C1 = R.V/ROOTMU (KEPC1), C2 = R V.V/MU - 1
     * (KEPC2), and ALPHA = (1 - C2)/R1, the inverse of the semi-major axis.
     *
     * XMAX, the largest X: for an ellipse (ALPHA positive) the X of one
     * period, 2PI/SQRT(ALPHA) (1REV); otherwise SQRT(-50SC/ALPHA). An
     * overflow keeps what MPAC has. */
    /* KEPLERN */ ORDERS(SETPD, BOV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_AT(KEPLERN, 3), /* (+1) */
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 0),
    STOVL_D(14),
    ADDRESS_PLUS_ONE(RRECT),
    ORDERS(UNIT, SSP),
    ADDRESS_PLUS_ONE(ITERCTR),
    NUMBER(20), /* 20D */
    STODL(URRECT),
    ADDRESS_PLUS_ONE_D(36),
    STOVL(R1),
    ADDRESS_PLUS_ONE(RRECT),
    ORDERS(DOT, SL1R),
    ADDRESS_PLUS_ONE(VRECT),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(1_ROOTMU), /* "1/ROOTMU (-17 OR -14)" */
    STOVL(KEPC1),               /* "C1=R.V/ROOTMU (+17 OR +16)" */
    ADDRESS_PLUS_ONE(VRECT),
    ORDERS(VSQ, DMPR),
    ADDRESS_PLUS_ONE(1_MU), /* "1/MU (-34 OR -28)" */
    ORDERS(DMP, SL3),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(DSU, ROUND),
    ADDRESS_PLUS_ONE(D1_64),
    STORE(KEPC2), /* "C2=RV.V/MU -1 (+6)" */
    ORDERS(BDSU, SR1R),
    ADDRESS_PLUS_ONE(D1_64),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(R1),
    STORE(ALPHA),       /* "ALPHA=(1-C2)/R1 (-22 OR -20)" */
    ORDERS(BPL, DLOAD), /* "MAXIMUM X DEPENDS ON TYPE OF CONIC" */
    ADDRESS(1REV),
    ADDRESS_PLUS_ONE(M50SC), /* "-50SC (+12)" */
    ORDERS(DDV, BOV),
    ADDRESS_PLUS_ONE(ALPHA),
    ADDRESS(STOREMAX),
    ORDERS(SQRT, GOTO),
    ADDRESS(STOREMAX),
    /* 1REV */ ORDERS(SQRT, BDDV),
    ADDRESS_PLUS_ONE(2PISC), /* "2PISC (+6)" */
    ORDER(BOV),
    ADDRESS(STOREMAX),

    /* STOREMAX: the period, from XMAX (XMAX/ROOTMU/ALPHA, normalized), and
     * TAU. brought inside it (PERIODCH: one period off at a time, "TO
     * PERMIT BACKDATING BY MORE THAN ONE ORBITAL PERIOD", the page 1160
     * history says). An overflow, or a negative period (no ellipse),
     * leaves TAU. as it is. */
    /* STOREMAX */ STORE(XMAX),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE(1_ROOTMU),
    ADDRESS_PLUS_ONE(ALPHA),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(SL_STAR, DDV),
    SL_X1(-6),
    ORDERS(BOV, BMN),
    ADDRESS(MODDONE),
    ADDRESS(MODDONE),                 /* "MPAC=PERIOD" */
    /* PERIODCH */ ORDERS(PDDL, ABS), /* "0D=PERIOD" */
    ADDRESS_PLUS_ONE(TAU_),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(MODDONE),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(TAU_),
    STODL(TAU_),
    ORDER(GOTO),
    ADDRESS(PERIODCH),

    /* MODDONE: X, the guess (XKEPNEW), must have TAU.'s sign, be nonzero
     * and be less than XMAX, or BADX makes one (XMAX/2 with TAU.'s sign).
     * STORBNDS: the bounds on X, from 0 to XMAX on TAU.'s side
     * (STOREMIN for a positive TAU.). */
    /* MODDONE */ ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(XKEPNEW),
    STORE(X),
    ORDERS(SIGN, BZE),
    ADDRESS_PLUS_ONE(TAU_),
    ADDRESS(BADX),
    ORDERS(BMN, ABS),
    ADDRESS(BADX),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(XMAX),
    ADDRESS(BADX),
    /* STORBNDS */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE(TAU_),
    ADDRESS(STOREMIN),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(XMAX),
    STODL(XMIN),
    ADDRESS_PLUS_ONE(KEPZERO),
#if LUMINARY099
    STORE(XMAX),
    ORDER(GOTO),
#else
    STCALL(XMAX),
#endif
    ADDRESS(DXCOMP),
    /* STOREMIN */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(KEPZERO),
    STORE(XMIN),

    /* DXCOMP: EPSILONT, the time error the iteration accepts (TAU. times
     * BEE22, 2 to the -22), and DELX, X less XPREV, the step from the
     * last solution (XDIFF). */
    /* DXCOMP */ ORDERS(DLOAD, DMPR),
    ADDRESS_PLUS_ONE(TAU_),
    ADDRESS_PLUS_ONE(BEE22),
    ORDER(ABS),
    STODL(EPSILONT),
    ADDRESS_PLUS_ONE(XPREV),
    /* XDIFF */ ORDER(BDSU),
    ADDRESS_PLUS_ONE(X),
    STORE(DELX),

    /* KEPLOOP -- p. 1176 (CM p. 1279): the iteration. XI = ALPHA X
     * squared, and DELTIME the time T that X gives (Kepler's equation);
     * DELT = TAU. - T, and done (KEPCONVG) when it is within EPSILONT.
     * Otherwise the next step is DELX DELT/(T - TC), the secant through
     * the last two points (normalized by the counts in X1 and X2), and the
     * bound on the side of X moves in to X: XMAX for a negative step,
     * XMIN for a positive one. A step that would go past the other bound
     * is cut to 9/10 of the way to it (NDXCHNGE, PDXCHNGE). */
    /* KEPLOOP */ ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE(X), /* "X=XKEP" */
    ORDERS(NORM, PUSH),  /* "0D=XSQ (+34 OR +32 -N1)       PL AT 2" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(ALPHA),
    SRR_X1(-6),
    STCALL(XI), /* "XI=ALPHA XSQ (+6)" */
    ADDRESS(DELTIME),
    ORDERS(BOV, BDSU),
    ADDRESS(TIMEOVFL), /* "UNLIKELY" */
    ADDRESS_PLUS_ONE(TAU_),
    STORE(DELT), /* "DELT=DELINDEP" */
    ORDERS(ABS, BDSU),
    ADDRESS_PLUS_ONE(EPSILONT),
    ORDERS(BPL, DLOAD),
    ADDRESS(KEPCONVG),
    ADDRESS_PLUS_ONE(T),
    ORDERS(DSU, NORM),
    ADDRESS_PLUS_ONE(TC),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(DELX),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(XSU_1, DMP),
    ADDRESS(X2),
    ADDRESS_PLUS_ONE(DELT),
    ORDERS(SLR_STAR, DDV),
    SLR_X1(1),
    ORDERS(SR1, PUSH), /* "0D=TRIAL DELX         PL AT 2" */
    ORDERS(BPL, DLOAD),
    ADDRESS(POSDELX),
    ADDRESS_PLUS_ONE(X),
    STORE(XMAX),       /* "MOVE MAX BOUND IN" */
    ORDERS(BDSU, DSU), /* "PL AT 0" */
    ADDRESS_PLUS_ONE(XMIN),
    ORDERS(BOV, BPL),
    ADDRESS(NDXCHNGE),
    ADDRESS(NDXCHNGE),
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(NEWDELX),
    /* NDXCHNGE */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(XMIN),
    ADDRESS_PLUS_ONE(X),
    ORDERS(DMPR, GOTO), /* "TO FORCE MPAC +2 TO ZERO" */
    ADDRESS_PLUS_ONE(DP9_10),
    ADDRESS(NEWDELX),
    /* POSDELX */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(X),
    STORE(XMIN),       /* "MOVE MIN BOUND IN" */
    ORDERS(BDSU, DSU), /* "PL AT 0" */
    ADDRESS_PLUS_ONE(XMAX),
    ORDERS(BOV, BMN),
    ADDRESS(PDXCHNGE),
    ADDRESS(PDXCHNGE),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(0),

    /* NEWDELX: the step taken (done if it is zero), T kept as TC, and
     * BRNCHCTR: CHECKCTR counts the iteration down (in ITERCTR); at zero
     * KEPCONVG takes what there is, otherwise KEPLOOP again. */
    /* NEWDELX */ STORE(DELX),
    ORDERS(BZE, DAD),
    ADDRESS(KEPCONVG),
    ADDRESS_PLUS_ONE(X),
    STODL(X),
    ADDRESS_PLUS_ONE(T),
    STORE(TC),
    /* BRNCHCTR */ ORDERS(RTB, BHIZ),
    ADDRESS(CHECKCTR),
    ADDRESS(KEPCONVG),
    ORDER(GOTO),
    ADDRESS(KEPLOOP), /* "ITERATE" */
    /* PDXCHNGE */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(XMAX),
    ADDRESS_PLUS_ONE(X),
    ORDERS(DMPR, GOTO), /* "TO FORCE MPAC +2 TO ZERO" */
    ADDRESS_PLUS_ONE(DP9_10),
    ADDRESS(NEWDELX),

    /* BADX -- p. 1177 (CM p. 1280): a guess out of bounds becomes XMAX/2
     * with TAU.'s sign. */
    /* BADX */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(XMAX),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(TAU_),
#if LUMINARY099
    STORE(X),
    ORDER(GOTO),
#else
    STCALL(X),
#endif
    ADDRESS(STORBNDS),

    /* TIMEOVFL -- p. 1178 (CM p. 1281): DELTIME overflowed ("X WAS TOO
     * BIG"): X becomes the bound on its side, and CMNTOVFL halves the step
     * and takes X back by it, and T back to TC (returning, through KEPRTN,
     * if the step is already zero). */
    /* TIMEOVFL */ ORDERS(DLOAD, BMN), /* "X WAS TOO BIG" */
    ADDRESS_PLUS_ONE(X),
    ADDRESS(NEGTOVFL),
    STORE(XMAX),
    /* CMNTOVFL */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(DELX),
    STORE(DELX),
    ORDERS(BZE, BDSU),
    ADDRESS(KEPRTN),
    ADDRESS_PLUS_ONE(X),
    STODL(X),
    ADDRESS_PLUS_ONE(TC),
#if LUMINARY099
    STORE(T),
    ORDER(GOTO),
#else
    STCALL(T),
#endif
    ADDRESS(BRNCHCTR),
#if LUMINARY099
    /* NEGTOVFL */ STORE(XMIN),
    ORDER(GOTO),
#else
    /* NEGTOVFL */ STCALL(XMIN),
#endif
    ADDRESS(CMNTOVFL),

    /* KEPCONVG -- pp. 1178-1179 (CM pp. 1281-1282): the new state, from
     * the f and g functions of X. RCV = (R1 - X squared C(XI)) URRECT +
     * (T - X cubed S(XI)/ROOTMU) VRECT, its length RCNORM; VCV = URRECT
     * (XI S(XI) - 1) X ROOTMU/RCNORM + (1 - X squared C(XI)/RCNORM) VRECT.
     * TC and XPREV keep T and X for the next call, and the return is
     * through KEPRTN (the caller's GOTO, "RETURNS WITH XPREV IN MPAC"). */
    /* KEPCONVG */ ORDERS(DLOAD, SR4R),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(DSU, VXSC),
    ADDRESS_PLUS_ONE(XSQC_XI),
    ADDRESS_PLUS_ONE(URRECT),
    ORDERS(VSL1, PDDL), /* "0D=(R1-XSQC(XI))URRECT (+33 OR +31)" */
    ADDRESS_PLUS_ONE(X),
    ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMPR, DMPR),
    ADDRESS_PLUS_ONE(1_ROOTMU),
    ADDRESS_PLUS_ONE(X),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(S_XI),
    SRR_X1(-7),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(T),
    ORDERS(SL1, VXSC),
    ADDRESS_PLUS_ONE(VRECT),
    ORDERS(VSL1, VAD), /* "PL AT 0" */
    ORDER(VSL4),
    STORE(RCV), /* "RCV (+29 OR +27)" */
    ORDERS(ABVAL, NORM),
    ADDRESS_PLUS_ONE(X2),
    STODL(RCNORM),
    ADDRESS_PLUS_ONE(XI),
    ORDERS(DMPR, DSU),
    ADDRESS_PLUS_ONE(S_XI),
    ADDRESS_PLUS_ONE(D1_128),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(ROOTMU),
    ORDERS(DMP, SLR_STAR),
    ADDRESS_PLUS_ONE(X),
    SLR_X2(-3),
    ORDERS(DDV, VXSC),
    ADDRESS_PLUS_ONE(RCNORM),
    ADDRESS_PLUS_ONE(URRECT),
    ORDERS(VSL1, PDDL), /* "0D=URRECT(XI S(XI)-1)X ROOTMU/RCV (+15" */
    ADDRESS_PLUS_ONE(XSQC_XI),
    ORDERS(SLR_STAR, DDV),
    SLR_X2(-4),
    ADDRESS_PLUS_ONE(RCNORM),
    ORDERS(BDSU, VXSC),
    ADDRESS_PLUS_ONE(D1_256),
    ADDRESS_PLUS_ONE(VRECT),
    ORDERS(VAD, VSL8),
    ORDER(STADR),            /* "PL AT 0" */
    AFTER_STADR(STODL(VCV)), /* "VCV (+7 OR +5)" */
    ADDRESS_PLUS_ONE(T),
    STODL(TC),
    ADDRESS_PLUS_ONE(X),
#if LUMINARY099
    STORE(XPREV),
    ORDER(GOTO),
#else
    STCALL(XPREV),
#endif
    ADDRESS(KEPRTN),
};

const struct interpretive_code KEPLER_CODE = {I_KEPLERN >> 10, K_KEPLERN, COUNT(kepler_words),
                                              kepler_words};
