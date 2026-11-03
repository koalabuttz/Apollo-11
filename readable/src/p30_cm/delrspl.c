/* SPDX-License-Identifier: MIT */
/* P30-P37.agc (Comanche055 pp. 643-647): DELRSPL, the splash-down error V82
 * shows in P11, and AUGEKUGL, the entry range angle (bank 32), in the
 * listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/cm_p30_routines.c).
 *
 * DELRSPL's function, as its description has it: "CALCULATE (FOR DISPLAY
 * ON CALL) AN APPROXIMATE MEASURE OF IN-PLANE SPLASH DOWN ERROR. IF THE
 * FREE-FALL TRANSFER ANGLE TO 300K FT ABOVE PAD RADIUS IS POSITIVE:
 * SPLASH ERROR= -RANGE TO TARGET + FREE-FALL TRANSFER ANGLE + ESTIMATED
 * ENTRY ANGLE." Otherwise only the range to the target. It goes back "TO
 * V 82 PROG. AT SPLRET", with RSP-RREC, the "RANGE IN REVOLUTIONS".
 *
 * AUGEKUGL estimates the entry range angle, "PHI ENTRY", from the speed
 * at 300,000 feet: K1 and K2 from the tables below CK1K2 and YK1K2 (the
 * speed's band picked by comparing it with V(21K), V(3K), V(4K) and
 * V(400)), then PHI from them (PHICALC), at most MAXPHIC, "2000 NM FOR
 * MAXIMUM PHI ENTRY". The tables are the rope's constants, which the
 * interpreter reads where the listing puts them ("TABLE IS INDEXED. KEEP
 * IN ORDER"). */
#include "p30_cm.h"
#include "../agc/names.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word delrspl_words[] = {
    /* ---- DELRSPL -- pp. 643-644 ---- */

    /* DELRSPL: the free-fall transfer angle to 300K feet, from TFF, at 8D.
     * Positive: "GONE PAST 300K FT ALT", to CANTDO; so too if less one bit
     * it overflows: "POSMAX INDICATES NO 300K FT SOLUTION." Else VGAMCALC
     * ("+GAMMA(REV) IN PMAC,V300 MAG(B-7)=PDL 0"), TFF/TRIG, and the entry
     * angle (AUGEKUGL, "T ENTRY PDL 6"): the estimated angle is the
     * transfer angle (ACOS of CDELF/2) plus it (4D). */
    STORE_D(8),
    ORDERS(BPL, DSU),
    ADDRESS(CANTDO), /* "GONE PAST 300K FT ALT" */
    ADDRESS_PLUS_ONE(1BITDP),
    ORDERS(BOV, CALL),
    ADDRESS(CANTDO),   /* "POSMAX INDICATES NO 300K FT SOLUTION." */
    ADDRESS(VGAMCALC), /* "+GAMMA(REV) IN PMAC,V300 MAG(B-7)=PDL 0" */
    ORDERS(PUSH, CALL),
    ADDRESS(TFF_TRIG),
    ORDER(CALL),
    ADDRESS(AUGEKUGL),
    ORDERS(PDDL, ACOS), /* "T ENTRY PDL 6" */
    ADDRESS_PLUS_ONE(CDELF_2),
    ORDER(DAD),
    ADDRESS_PLUS_ONE_D(4),

    /* GETARG: the angle as THETA(1); the target, LAT(SPL) and the rest, at
     * altitude 0 ("ALT=0 = LAT +4"), at the time of impact: PIPTIME less
     * TSTART82 (V82's start, unless V37FLAG) plus 8D, the time to 300K
     * feet. Its position (LALOTORV, with ERADFLAG clear: "R RECOV. IN
     * ALPHAV AND MPAC") and RONE's, as unit vectors: the range to it is
     * the ARCCOS of their dot product. "ERROR = THETA EST - THETA TARG";
     * a "NEGATIVE NUMBER SIGNIFIES THAT WILL FALL SHORT." */
    STOVL(THETA_1),
    ADDRESS_PLUS_ONE(LAT_SPL),
    STODL(LAT),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STODL(ALT), /* "ALT=0 = LAT +4" */
    ADDRESS_PLUS_ONE(PIPTIME),
    ORDERS(BON, DLOAD),
    SWITCH(BON, V37FLAG),
    ADDRESS_AT(GETARG, 012), /* +2: the DSU DAD */
    ADDRESS_PLUS_ONE(TSTART82),
    ORDERS(DSU, DAD),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, ERADFLAG),
    ADDRESS(LALOTORV), /* "R RECOV. IN ALPHAV AND MPAC" */
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(UNIT, DOT),
    ORDERS(SL1, ARCCOS),
    ORDER(BDSU), /* "ERROR = THETA EST - THETA TARG" */
    ADDRESS_PLUS_ONE(THETA_1),

    /* DELRDONE: "DOWNRANGE RECOVERY RANGE ERROR /360", and back to V82
     * (INTWAKE0, SPLRET). */
    STCALL(RSP_RREC), /* "DOWNRANGE RECOVERY RANGE ERROR" */
    ADDRESS(INTWAKE0),
    ORDER(CALL),
    ADDRESS(SPLRET),

    /* CANTDO: "INITIALIZE ERASE TO DOT TARGET AND UR FOR RANGE ANGLE.": a
     * half "TO PDL 0 FOR DEN IN DDV.", "ZERO TO PDL 2 FOR PHI ENTRY", and
     * 8D zero: "GO SET RSP-RREC =0" at GETARG. */
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(HIDPHALF), /* "TO PDL 0 FOR DEN IN DDV." */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDER(PUSH), /* "ZERO TO PDL 2 FOR PHI ENTRY" */
    STCALL_D(8),
    ADDRESS(GETARG), /* "GO SET RSP-RREC =0" */

    /* ---- AUGEKUGL -- pp. 644-645 ---- */

    /* AUGEKUGL: X1, X2 and S1 10, 8 and 6 (X1CON's DECs: the vector at
     * X1CON -2 stored at X1 -2), then the speed at 0D (VGAMCALC's V300)
     * against V(21K), V(3K), V(4K) and V(400) in turn, the index
     * registers stepping (XSU, XCHX, SXA) to the tables' entries for its
     * band, and on at LOOPSET. */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(X1CON, -2),
    STODL_AT(X1, -2),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(V_21K),
    ADDRESS(LOOPSET),
    ORDERS(XSU_1, XCHX_2),
    ADDRESS(S1),
    ADDRESS(X1),
    ORDERS(XCHX_2, DSU),
    ADDRESS(S1),
    ADDRESS_PLUS_ONE(V_3K),
    ORDERS(BMN, XCHX_2),
    ADDRESS(LOOPSET),
    ADDRESS(S1),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(V_4K),
    ADDRESS(LOOPSET),
    ORDERS(XCHX_2, XCHX_2),
    ADDRESS(S1),
    ADDRESS(X1),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(V_400),
    ADDRESS(LOOPSET),
    ORDER(SXA_1),
    ADDRESS(S1),

    /* LOOPSET: X1 one more, then K1K2LOOP; K2CALC: S1 from X1. */
    ORDERS(INCR_1, GOTO),
    NUMBER(1), /* DEC 1 */
    ADDRESS(K1K2LOOP),
    ORDER(SXA_1),
    ADDRESS(S1),

    /* K1K2LOOP: the speed less the band's V(32K) +1,1 entry, times YK1K2
     * +1,1, plus CK1K2 +1,1, pushed, twice (TIX,1 by way of K2CALC): K1
     * and K2. Their quotient, pushed: an overflow, a negative one or one
     * past MAXPHIC gives MAXPHI instead. */
    ORDERS(DLOAD, DSU_STAR),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_AT_X1(V_32K, 1),
    ORDERS(DMP_STAR, DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(YK1K2, 1),
    ADDRESS_PLUS_ONE_AT_X1(CK1K2, 1),
    ORDERS(PDDL, TIX_1),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS(K2CALC),
    ORDERS(DSU, BDDV),
    ORDERS(PUSH, BOV),
    ADDRESS(MAXPHI),
    ORDERS(BMN, DSU),
    ADDRESS(MAXPHI),
    ADDRESS_PLUS_ONE(MAXPHIC),
    ORDER(BPL),
    ADDRESS(MAXPHI),

    /* PHICALC: "PHI ENTRY PDL 4D": below V(26K), TLESS26 ("8660PHI/V")
     * over the speed; else TGR26CON ("PHI/3"); times 4D (TENT), and back
     * (RVQ). */
    ORDERS(DLOAD, DSU), /* "PHI ENTRY PDL 4D" */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(V_26K),
    ORDERS(BPL, DLOAD),
    ADDRESS(TGR26),
    ADDRESS_PLUS_ONE(TLESS26),
    ORDER(DDV),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DMP, RVQ),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(TGR26CON),
    ADDRESS(TENT),

    /* MAXPHI: MAXPHIC, "2000 NM FOR MAXIMUM PHI ENTRY", pushed (PDDL),
     * and PHICALC. */
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(MAXPHIC),
    ORDER(GOTO),
    ADDRESS(PHICALC),
};

const struct interpretive_code DELRSPL_CODE = {032u, DELRSPL, COUNT(delrspl_words), delrspl_words};

#endif
