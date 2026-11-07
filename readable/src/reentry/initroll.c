/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 853-858 (Comanche055, bank 25): "START INITIAL
 * ROLL" (INITROLL, MM 63 and 64) and "START HUNT TEST" (HUNTEST), the
 * first two phases of the roll command equations GOTOADDR selects. The
 * faithful C is c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- INITROLL -- pp. 853-854 ---- */

/* INITROLL, until the drag passes .05G: "IF D- .05G NEG, GO TO LIMITL/D",
 * the roll as STARTENT set it. Once past (P64, "MM = 64, NOW"), KAT, the
 * drag to hold, "KA = KA1 LEQ  + KA2" (LEQ cubed), at most KALIM. "IF
 * V-VFINAL1 NEG, GO TO FINAL PHASE.": GOTOADDR KEP2, "AND IDLE UNTIL D >
 * 0.2 G.  (NO P66 HERE)". Else D0EQ: "D0 = KA3 LEQ + KA4", the constant
 * drag of the next phase, and C/D0 its "(-4/D0) B-8"; L/D is LAD, lift up,
 * but -LAD, lift down, when "V-VFINAL +(RDOT/V)  / K44" (RDOT/V cubed)
 * is positive and does not overflow. INROLOUT, the "END OF PRE .05G PATH
 * OF INITROLL.": INRLSW set, "SET INRLSW AT END FOR RESTART PROTECTION",
 * and the command through LIMITL/D.
 *
 * With INRLSW set, INITRL1: "IF RDOT + VRCONT POS, GO TO HUNTEST"; else
 * KATEST, "IF KAT - D POS, GO TO CONSTD", "IF POS, OUT WITH COMMAND VIA
 * LIMITL/D". FOREHUNT, then GOTOADDR HUNTEST: "INITIALIZE HUNTEST ON
 * FIRST PASS", "MUST GO AFTER FOREHUNT FOR RESTARTS." */
static const word initroll_words[] = {
    /* INITROLL */ ORDERS(BON, BOFF), /* "IF D- .05G NEG, GO TO LIMITL/D" */
    SWITCH(BON, INRLSW),
    ADDRESS(INITRL1),
    SWITCH(BOFF, _05GSW),
    ADDRESS(LIMITL_D),
    ORDERS(DLOAD, DSQ), /* "KA = KA1 LEQ  + KA2" */
    ADDRESS_PLUS_ONE(LEQ),
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(LEQ),
    ADDRESS_PLUS_ONE(1_KA1), /* "= 25 /(64  1.8)" */
    ORDERS(DAD, RTB),
    ADDRESS_PLUS_ONE(KA2), /* "= .2" */
    ADDRESS(P64),
    STORE(KAT),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(KALIM),
    ADDRESS_AT(INITROLL, 20), /* "+4" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(KALIM),
    STORE(KAT),
    ORDERS(DLOAD, DSU), /* "IF V-VFINAL1 NEG, GO TO FINAL PHASE." */
    ADDRESS_PLUS_ONE(V),
    ADDRESS_PLUS_ONE(VFINAL1),
    ORDERS(CLEAR, BPL), /* "(CAN'T CLEAR INRLSW AFTER HERE: RESTARTS)" */
    SWITCH(CLEAR, GONEPAST),
    ADDRESS(D0EQ),
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(GOTOADDR),
    ADDRESS(KEP2),                 /* "AND IDLE UNTIL D > 0.2 G.  (NO P66 HERE)" */
    ADDRESS(INROLOUT),             /* "GO TO LIMITL/D AFTER SETTING INRLSW." */
    /* D0EQ */ ORDERS(DLOAD, DMP), /* "D0 = KA3 LEQ + KA4" */
    ADDRESS_PLUS_ONE(LEQ),
    ADDRESS_PLUS_ONE(KA3),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(KA4),
    STORE(D0), /* "D0/805" */
    ORDERS(BDDV, BOV),
    ADDRESS_PLUS_ONE(C001), /* "(-4/25 G) B-8" */
    ADDRESS_AT(D0EQ, 9),    /* "+1": "CLEAR OVFIND, IF ON." */
    STODL(C_D0),            /* "(-4/D0) B-8" */
    ADDRESS_PLUS_ONE(LAD),
    STODL(L_D),
    ADDRESS_PLUS_ONE(RDOT),
    ORDERS(DDV, PUSH),
    ADDRESS_PLUS_ONE(V),
    ORDERS(DSQ, DMP),
    ORDERS(DDV, DSU),
    ADDRESS_PLUS_ONE(1_K44),
    ADDRESS_PLUS_ONE(VFINAL),
    ORDERS(DAD, BOV),
    ADDRESS_PLUS_ONE(V),
    ADDRESS(INROLOUT),
    ORDERS(BMN, DLOAD),
    ADDRESS(INROLOUT),
    ADDRESS_PLUS_ONE(LAD),
    ORDER(DCOMP),
    STORE(L_D),
    /* INROLOUT */ ORDER(BOFSET), /* "END OF PRE .05G PATH OF INITROLL." */
    SWITCH(BOFSET, INRLSW),       /* "SWITCH IS ZERO INITIALLY." */
    ADDRESS(LIMITL_D),
    /* KATEST */ ORDERS(DLOAD, DSU), /* "IF KAT - D POS, GO TO CONSTD" */
    ADDRESS_PLUS_ONE(KAT),
    ADDRESS_PLUS_ONE(D),
    ORDERS(BPL, GOTO),
    ADDRESS(LIMITL_D),
    ADDRESS(CONSTD),
    /* INITRL1 */ ORDERS(DLOAD, DAD), /* "IF RDOT + VRCONT POS, GO TO HUNTEST" */
    ADDRESS_PLUS_ONE(RDOT),
    ADDRESS_PLUS_ONE(VRCONT),
    ORDERS(BMN, CALL),
    ADDRESS(KATEST),
    ADDRESS(FOREHUNT), /* "INITIALIZE HUNTEST." */
    ORDER(SSP),        /* "INITIALIZE HUNTEST ON FIRST PASS" */
    ADDRESS_PLUS_ONE(GOTOADDR),
    ADDRESS(HUNTEST), /* "MUST GO AFTER FOREHUNT FOR RESTARTS." */
};

const struct interpretive_code INITROLL_CODE = {025u, INITROLL, COUNT(initroll_words),
                                                initroll_words};

/* ---- HUNTEST -- pp. 855-858 ---- */

/* HUNTEST, the "HUNT TEST": whether a skip out above the atmosphere (the
 * Kepler phase) reaches the target. A1, the drag now; TEM1B, "IF RDOT
 * NEG,TEM1B=LAD, OTHERWISE = LEWD"; "V1 = V + RDOT/TEM1B", the velocity
 * as the climb ends; A0 the drag there, "A0=(V1/V)SQ(D+RDOT SQ/(TEM1B 2
 * C1 HS)", and A1 = A0 when climbing. V1LEAD: "IF L/D NEG, V1=V1 - 1000"
 * (VQUIT).
 *
 * HUNTEST1, the exit from the atmosphere: "ALP = 2 C1 HS A0/LEWD V1 V1",
 * "FACT1 = V1 / (1 - ALP)", "FACT2 = ALP(ALP - 1) / A0", and VL, the
 * velocity at the exit drag Q7, "VL=FACT1 (1-SQRT(Q7 FACT2 +ALP) )";
 * "GAMMAL1 = LEWD (V1-VL)/VL", the flight path angle there. "IF VL-VLMIN
 * NEG, GO TO PREFINAL". "IF VSAT-VL NEG, GO TO CONSTD" (BECONSTD, "SET
 * MODE=HUNTEST, CONTINUE IN CONSTD"); else DVL = VSAT - VL, or "DVL = DVL
 * - (VSAT-V1) = V1 - VL" when V1 is below VSAT ("VS1 = V1, IN THIS
 * CASE").
 *
 * GETDHOOK: "DHOOK=((1-VS1/FACT1) SQ -ALP)/FACT2" (DHOOKYQ7), AHOOKDV from
 * it and CHOOK, and "GAMMAL= GAMMAL1-CH1 DVL SQ(1+AHOOK DVL)", to NEGAMA
 * when negative. HUNTEST3: GAMMAL, and "GAMMAL1=GAMMAL1 +Q19
 * (GAMMAL-GAMMAL1)"; on to the range prediction (RANGER) with GAMMAL. */
static const word huntest_words[] = {
    /* HUNTEST */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(D),
    STODL(A1), /* "A1/805 = A1/25G" */
    ADDRESS_PLUS_ONE(LAD),
    STODL(TEM1B),
    ADDRESS_PLUS_ONE(RDOT),
    ORDERS(BMN, DLOAD), /* "IF RDOT NEG,TEM1B=LAD, OTHERWISE = LEWD" */
    ADDRESS(A0CALC),
    ADDRESS_PLUS_ONE(LEWD),
    STODL(TEM1B),
    ADDRESS_PLUS_ONE(RDOT),
    /* A0CALC */ ORDERS(DDV, DAD), /* "V1 = V + RDOT/TEM1B" */
    ADDRESS_PLUS_ONE(TEM1B),
    ADDRESS_PLUS_ONE(V),
    STODL(V1), /* "V1/2 VS" */
    ADDRESS_PLUS_ONE(RDOT),
    ORDERS(DSQ, DDV), /* "A0=(V1/V)SQ(D+RDOT SQ/(TEM1B 2 C1 HS)" */
    ADDRESS_PLUS_ONE(TEM1B),
    ORDERS(DDV, DAD),
    ADDRESS_PLUS_ONE(2C1HS),
    ADDRESS_PLUS_ONE(D),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(V1),
    ADDRESS_PLUS_ONE(V1),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(VSQUARE),
    STODL(A0), /* "A0/805 = A0/25G" */
    ADDRESS_PLUS_ONE(RDOT),
    ORDERS(BPL, DLOAD),
    ADDRESS(V1LEAD),
    ADDRESS_PLUS_ONE(A0),
    STORE(A1),                       /* "A1/25G" */
    /* V1LEAD */ ORDERS(DLOAD, BPL), /* "IF L/D NEG, V1=V1 - 1000" */
    ADDRESS_PLUS_ONE(L_D),
    ADDRESS(HUNTEST1),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(V1),
    ADDRESS_PLUS_ONE(VQUIT),
    STORE(V1),
    /* HUNTEST1 */ ORDERS(DLOAD, DMP), /* "ALP = 2 C1 HS A0/LEWD V1 V1" */
    ADDRESS_PLUS_ONE(A0),
    ADDRESS_PLUS_ONE(2C1HS),
    ORDERS(DDV, SETPD),
    ADDRESS_PLUS_ONE(V1),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DDV, DDV),
    ADDRESS_PLUS_ONE(V1),
    ADDRESS_PLUS_ONE(LEWD),
    STORE(ALP),
    ORDERS(BDSU, BDDV), /* "FACT1 = V1 / (1 - ALP)" */
    ADDRESS_PLUS_ONE(BARELY1),
    ADDRESS_PLUS_ONE(V1),
    STODL(FACT1), /* "FACT1 / 2VS" */
    ADDRESS_PLUS_ONE(ALP),
    ORDERS(DSU, DMP), /* "FACT2 = ALP(ALP - 1) / A0" */
    ADDRESS_PLUS_ONE(BARELY1),
    ADDRESS_PLUS_ONE(ALP),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(A0),
    STORE(FACT2), /* "FACT2 (25G)" */
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(Q7),  /* "Q7 / 805 = Q7 / 25G" */
    ADDRESS_PLUS_ONE(ALP), /* "VL=FACT1 (1-SQRT(Q7 FACT2 +ALP) )" */
    ORDERS(SQRT, BDSU),
    ADDRESS_PLUS_ONE(BARELY1),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(FACT1),
    STORE(VL),         /* "VL / 2 VS" */
    ORDERS(BDSU, DMP), /* "GAMMAL1 = LEWD (V1-VL)/VL" */
    ADDRESS_PLUS_ONE(V1),
    ADDRESS_PLUS_ONE(LEWD),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(VL),
    STODL(GAMMAL1), /* "GAMMAL1 USED IN UPCONTROL" */
    ADDRESS_PLUS_ONE(VL),
    ORDERS(DSU, BMN), /* "IF VL-VLMIN NEG, GO TO PREFINAL" */
    ADDRESS_PLUS_ONE(VLMIN),
    ADDRESS(PREFINAL),
    ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE(VL),
    STODL(VBARS),            /* "VBARS / 4 VS VS" */
    ADDRESS_PLUS_ONE(HALVE), /* "IF VSAT-VL NEG, GO TO CONSTD" */
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(VL),
    ADDRESS(BECONSTD), /* "SET MODE=HUNTEST, CONTINUE IN CONSTD" */
    STODL(DVL),        /* "DVL / 2VS" */
    ADDRESS_PLUS_ONE(HALVE),
    STORE(VS1),       /* "VS1 = VSAT" */
    ORDERS(DSU, BMN), /* "IF V1 GREATER THAN VSAT, GO ON" */
    ADDRESS_PLUS_ONE(V1),
    ADDRESS(GETDHOOK),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(DVL),
    STODL(DVL), /* "DVL = DVL - (VSAT-V1) = V1 - VL" */
    ADDRESS_PLUS_ONE(V1),
    STORE(VS1),                         /* "VS1 = V1, IN THIS CASE" */
    /* GETDHOOK */ ORDERS(DLOAD, CALL), /* "DHOOK=((1-VS1/FACT1) SQ -ALP)/FACT2" */
    ADDRESS_PLUS_ONE(VS1),              /* "VS1 / 2 VS" */
    ADDRESS(DHOOKYQ7),                  /* "GO CALC DHOOK" */
    STORE(DHOOK),                       /* "DHOOK / 25G" */
    ORDERS(SR, DDV),
    SR_COUNT(6), /* "CHOOK" */
    ADDRESS_PLUS_ONE(Q7),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(CHOOK), /* "= .25/16 = (-6)" */
    STORE(AHOOKDV),
    ORDERS(DAD, DMP), /* "GAMMAL= GAMMAL1-CH1 DVL SQ(1+AHOOK DVL)" */
    ADDRESS_PLUS_ONE(1_16TH),
    ADDRESS_PLUS_ONE(CH1),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(DVL),
    ADDRESS_PLUS_ONE(DVL),
    ORDERS(DDV, DDV),
    ADDRESS_PLUS_ONE(DHOOK),
    ADDRESS_PLUS_ONE(VBARS),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE(GAMMAL1),
    ADDRESS(NEGAMA),
    /* HUNTEST3 */ STORE(GAMMAL),
    ORDER(DSU), /* "GAMMAL1=GAMMAL1 +Q19 (GAMMAL-GAMMAL1)" */
    ADDRESS_PLUS_ONE(GAMMAL1),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(Q19),
    ADDRESS_PLUS_ONE(GAMMAL1),
    STODL(GAMMAL1),
    ADDRESS_PLUS_ONE(GAMMAL),
};

const struct interpretive_code HUNTEST_CODE = {025u, HUNTEST, COUNT(huntest_words), huntest_words};

#endif
