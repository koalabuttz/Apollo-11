/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 859-864 (Comanche055, bank 25): "START RANGE
 * PREDICTION" (RANGER), the end of the hunt test (the step in LEWD,
 * SIDETRAK, NEGAMA, DHOOKYQ7), its restart (PRE-HUNT, FOREHUNT), and
 * GOTOUPSY, into the up control. The faithful C is
 * c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- RANGER -- pp. 859-861 ---- */

/* RANGER, with GAMMAL from HUNTEST: the range the trajectory HUNTEST found
 * would fly, in five pieces. The Kepler arc above the atmosphere, "ASKEP/2
 * = ARCSIN(VBARS COSG SING/E)" ("COSG = 1-GAMMAL SQ/2, TRUNCATED SERIES",
 * E its eccentricity); the "FINAL PHASE RANGE", "ASP1 = Q2 + Q3 VL"; the
 * "UP PHASE RANGE", "ASPUP= -C12 LOG(V1 Q7/VBARS A0)/GAMMAL1" (V1 squared);
 * the "RANGE TO PULL OUT", "ASPDWN = KC3 RDOT V / A0"; and the "GAMMA
 * CORRECTION", "ASP3 = Q5(Q6-GAMMAL)". "SAVE HI-WORD OF ASP'S FOR TM.",
 * then the sum less THETAH: "DIFF = (ASP-THETAH) / 2 PI".
 *
 * "IF ABS(THETAH-ASP) -25NM NEG, GO TO UPSY" (GOTOUPSY). The first time
 * (HIND clear), on to the next pass with a step in LEWD (GETLEWD); else
 * DCONSTD when overshooting ("EVENTUALLY SETS MODE = HUNTEST."), or the
 * step by the secant, "DLEWD = DLEWD (DIFF/(DIFFOLD-DIFF))". LWDSTORE:
 * LEWD + DLEWD, the new lift; "IF LEWD+DLEWD NEG, DLEWD=-LEWD/2". Then
 * SIDETRAK: out of the interpreter, ... */
static const word ranger_words[] = {
    /* RANGER */ ORDERS(DSQ, SR2), /* "COSG = 1-GAMMAL SQ/2, TRUNCATED SERIES" */
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(HALVE),
    STODL(COSG_2),
    ADDRESS_PLUS_ONE(VBARS), /* "E=SQRT(1+VBARS........" */
    ORDERS(DSU, DMP),
    ADDRESS_PLUS_ONE(HALVE),
    ADDRESS_PLUS_ONE(VBARS),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(COSG_2),
    ADDRESS_PLUS_ONE(COSG_2),
    ORDERS(SL2, DAD),
    ADDRESS_PLUS_ONE(C1_16), /* "C1/16 = 1/16" */
    ORDERS(SQRT, PDDL),      /* "E/4 INTO PDL" */
    ADDRESS_PLUS_ONE(VBARS),
    ORDERS(DMP, DMP), /* "ASKEP/2 = ARCSIN(VBARS COSG SING/E)" */
    ADDRESS_PLUS_ONE(COSG_2),
    ADDRESS_PLUS_ONE(GAMMAL),
    ORDERS(DDV, ASIN),
    ORDERS(SL1, PUSH), /* "ASKEP TO PDL 0." */
    STODL(ASKEP),      /* "BALLISTIC RANGE ASKEP/2PI" */
    ADDRESS_PLUS_ONE(VL),
    ORDERS(DMP, DAD), /* "ASP1 = Q2 + Q3 VL" */
    ADDRESS_PLUS_ONE(Q3),
    ADDRESS_PLUS_ONE(Q2),
    STORE(ASP1),       /* "FINAL PHASE RANGE ASP1/2 PI" */
    ORDERS(PDDL, DSQ), /* "ASP1 TO PDL 2." */
    ADDRESS_PLUS_ONE(V1),
    ORDERS(DMP, DDV), /* "ASPUP= -C12 LOG(V1 Q7/VBARS A0)/GAMMAL1" */
    ADDRESS_PLUS_ONE(Q7),
    ADDRESS_PLUS_ONE(VBARS),
    ORDERS(DDV, CALL),
    ADDRESS_PLUS_ONE(A0),
    ADDRESS(LOG), /* "RETURN WITH -LOG IN MPAC" */
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(C12),
    ADDRESS_PLUS_ONE(GAMMAL1),
    STORE(ASPUP),           /* "UP PHASE RANGE ASPUP / 2 PI" */
    ORDERS(PDDL, DMP),      /* "ASPUP TO PDL 4." */
    ADDRESS_PLUS_ONE(KC3),  /* "KC3 = -4 VS VS / 2 PI 805 RE" */
    ADDRESS_PLUS_ONE(RDOT), /* "ASPDWN = KC3 RDOT V / A0" */
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(V),
    ADDRESS_PLUS_ONE(A0),
    ORDERS(DDV, PUSH), /* "ASPDWN TO PDL 6." */
    ADDRESS_PLUS_ONE(LAD),
    STODL(ASPDWN), /* "RANGE TO PULL OUT ASPDWN /2 PI" */
    ADDRESS_PLUS_ONE(Q6),
    ORDERS(DSU, DMP), /* "ASP3 = Q5(Q6-GAMMAL)" */
    ADDRESS_PLUS_ONE(GAMMAL),
    ADDRESS_PLUS_ONE(Q5),
    STOVL(ASP3),             /* "GAMMA CORRECTION ASP3/2PI" */
    ADDRESS_PLUS_ONE(ASKEP), /* "GET HI-WD AND" */
    STODL(ASPS_TM),          /* "SAVE HI-WORD OF ASP'S FOR TM." */
    ADDRESS_PLUS_ONE(ASP3),
    ORDERS(DAD, DAD),  /* "ASPDWN FROM PDL 6." "ASPUP FROM PDL 4." */
    ORDERS(DAD, DAD),  /* "ASP1 FROM PDL 2." "ASKEP FROM PDL 0." */
    ORDERS(DSU, BOVB), /* "CLEAR OVFIND." */
    ADDRESS_PLUS_ONE(THETAH),
    ADDRESS(TCDANZIG),
    STORE(DIFF),      /* "DIFF = (ASP-THETAH) / 2 PI" */
    ORDERS(ABS, DSU), /* "IF ABS(THETAH-ASP) -25NM NEG, GO TO UPSY" */
    ADDRESS_PLUS_ONE(25NM),
    ORDERS(BMN, BON),
    ADDRESS(GOTOUPSY),
    SWITCH(BON, HIND),
    ADDRESS(GETLEWD),
    ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE(DIFF),
    ADDRESS(DCONSTD),                  /* "EVENTUALLY SETS MODE = HUNTEST." */
    /* GETDLEWD */ ORDERS(DLOAD, DMP), /* "DLEWD = DLEWD (DIFF/(DIFFOLD-DIFF))" */
    ADDRESS_PLUS_ONE(DLEWD),
    ADDRESS_PLUS_ONE(DIFF),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE(DIFFOLD),
    ADDRESS_PLUS_ONE(DIFF),
    ORDER(BDDV),
    /* LWDSTORE */ ORDER(STADR),
    AFTER_STADR(STORE(DLEWD)),
    ORDERS(DAD, BMN), /* "IF LEWD+DLEWD NEG, DLEWD=-LEWD/2" */
    ADDRESS_PLUS_ONE(LEWD),
    ADDRESS(LEWDPTR),
    ORDER(BOV),
    ADDRESS(LEWDOVFL),
    STORE(LEWD),
    /* SIDETRAK */ ORDER(EXIT),
};

const struct interpretive_code RANGER_CODE = {025u, RANGER, COUNT(ranger_words), ranger_words};

/* ---- SIDETRAK -- p. 861 ---- */

/* ... to wait out the hunt's next pass at a lower priority: "DROP GRP 5
 * RESTART PRIO TO 1 LESS THAN" "GRP 4." (PRIO16 in PHSPRDT5), and
 * "RESTART GRP 4 AT PRE-HUNT." (PHASCHNG, OCT 00474); ... */
void sidetrak_after_exit(void)
{
    set_A(EBENTRY);
    ts(EBANK_ADDRESS);
    set_A(PRIO16);
    ts(ADDRESS_OF(PHSPRDT5));
    tc_to(SIDETRAK + 5u, PHASCHNG);
}

/* ... "CONTINUE GRP 5 AT LOWER PRIO THAN EITHER" "GRP 4 OR SERVICER."
 * (PRIOCHNG); ... */
void sidetrak_after_phaschng(void)
{
    set_A(PRIO16);
    tc_to(SIDETRAK + 010u, PRIOCHNG);
}

/* ... and "SIDETRACK NEXT PASS UNTIL THIS ONE DONE.": GOTOADDR ENDEXIT
 * (ADENDEXT), so the two-second pass leaves at once until this one is
 * through. Back in the interpreter, ... */
void sidetrak_after_priochng(void)
{
    set_A(ADENDEXT);
    ts(ADDRESS_OF(GOTOADDR));
    tc_at(SIDETRAK + 013u);
    intpret();
}

/* ... HIND set and DIFFOLD the range error, and Q7 Q7F again: back to
 * HUNTEST, the next pass of the hunt. LEWDOVFL, when LEWD + DLEWD
 * overflows: LEWD NEARONE and DCONSTD, "ALSO WILL SET MODE = HUNTEST".
 * LEWDPTR, when it is negative: -LEWD/2, stored through LWDSTORE.
 *
 * NEGAMA, "PART OF HUNTEST", with GAMMAL negative: a lower VL, "DEL VL =
 * (GAMMAL VL/3)/(LEWD/3-DVL" "(2/3 + AHOOKDV)(CH1 GS/DHOOK VL))"; Q7 again
 * from it, "Q7=((1-VL/FACT1)SQ - ALP)/FACT2" (DHOOKYQ7), VBARS, and "SET
 * GAMMAL = 0" (HUNTEST3). DHOOKYQ7, the "SUBROUTINE TO CALC DHOOK OR Q7)"
 * from the velocity in MPAC: ((1 - MPAC/FACT1) squared - ALP) / FACT2. */
static const word sidetrak_words[] = {
    /* SIDETRAK +014 */ ORDERS(DLOAD, SET),
    ADDRESS_PLUS_ONE(DIFF),
    SWITCH(SET, HIND),
    STODL(DIFFOLD), /* "DIFFOLD / 2 PI" */
    ADDRESS_PLUS_ONE(Q7F),
    STCALL(Q7),       /* "Q7 / 805 FPSS" */
    ADDRESS(HUNTEST), /* "(GO TO)" */
    /* LEWDOVFL */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(NEARONE),
    STCALL(LEWD),
    ADDRESS(DCONSTD), /* "ALSO WILL SET MODE = HUNTEST" */
    /* LEWDPTR */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(LEWD),
    ORDERS(DCOMP, GOTO),
    ADDRESS(LWDSTORE),
    /* NEGAMA */ ORDERS(DMP, DMP), /* "ENTER WITH GAMMAL IN MPAC" */
    ADDRESS_PLUS_ONE(VL),
    ADDRESS_PLUS_ONE(1_3RD),
    ORDERS(PDDL, DMP), /* "PUSH GAMMAL VL/3" */
    ADDRESS_PLUS_ONE(LEWD),
    ADDRESS_PLUS_ONE(1_3RD),
    ORDERS(PDDL, DAD), /* "PUSH LEWD/3" */
    ADDRESS_PLUS_ONE(AHOOKDV),
    ADDRESS_PLUS_ONE(1_24TH),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(DVL),
    ADDRESS_PLUS_ONE(CH1),
    ORDERS(DDV, DDV),
    ADDRESS_PLUS_ONE(DHOOK),
    ADDRESS_PLUS_ONE(VL),
    ORDERS(BDSU, BDDV), /* "LEWD/3" "GAMMAL VL /3" */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(VL),
    STCALL(VL),        /* "VL/2 VS" */
    ADDRESS(DHOOKYQ7), /* "GO CALC Q7" */
    STODL(Q7),         /* "Q7 / 25G" */
    ADDRESS_PLUS_ONE(VL),
    ORDER(DSQ),
    STODL(VBARS), /* "VBARS / 4 VS VS" */
    ADDRESS_PLUS_ONE(3ZEROS),
    ORDER(GOTO), /* "SET GAMMAL = 0" */
    ADDRESS(HUNTEST3),
    /* DHOOKYQ7 */ ORDERS(SR1, DDV), /* "SUBROUTINE TO CALC DHOOK OR Q7)" */
    ADDRESS_PLUS_ONE(FACT1),
    ORDERS(BDSU, SL1),
    ADDRESS_PLUS_ONE(HALVE),
    ORDERS(DSQ, DSU),
    ADDRESS_PLUS_ONE(ALP),
    ORDERS(DDV, RVQ),
    ADDRESS_PLUS_ONE(FACT2),
};

const struct interpretive_code SIDETRAK_CODE = {025u, SIDETRAK + 014u, COUNT(sidetrak_words),
                                                sidetrak_words};

/* ---- PRE-HUNT, FOREHUNT -- p. 863 ---- */

/* PRE-HUNT: "COME TO PRE-HUNT WHEN RESTART OCCURS AFTER" "HUNTEST IS
 * SIDE-TRACKED AT SIDETRAK." Into the interpreter, ... */
void pre_hunt(void)
{
    tc_at(PRE_HUNT);
    intpret();
}

/* ... HIND clear and FOREHUNT, "RE-INITIALIZE HUNTEST AFTER RE-START.",
 * then HUNTEST. FOREHUNT, to "INITIALIZE HUNTEST.": DIFFOLD zero, DLEWD
 * DLEWD0 and LEWD LEWD1, the first guess at the lift. */
static const word pre_hunt_words[] = {
    /* PRE-HUNT +1 */ ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, HIND), /* "HIND 99D BIT 6 FLAG 6" */
    ADDRESS(FOREHUNT),   /* "RE-INITIALIZE HUNTEST AFTER RE-START." */
    ORDER(GOTO),
    ADDRESS(HUNTEST),
    /* FOREHUNT */ ORDER(DLOAD), /* "INITIALIZE HUNTEST." */
    ADDRESS_PLUS_ONE(3ZEROS),
    STODL(DIFFOLD),
    ADDRESS_PLUS_ONE(DLEWD0),
    STODL(DLEWD),
    ADDRESS_PLUS_ONE(LEWD1),
    STORE(LEWD),
    ORDER(RVQ),
};

const struct interpretive_code PRE_HUNT_CODE = {025u, PRE_HUNT + 1u, COUNT(pre_hunt_words),
                                                pre_hunt_words};

/* ---- GOTOUPSY -- p. 864 ---- */

/* GOTOUPSY, the "END OF HUNTEST": P65 (MM 65) for the display, "SET MODE
 * = UPCONTRL.", and on into UPCONTRL ("RETURN FROM P65 DIRECTLY TO
 * UPCONTRL"). */
static const word gotoupsy_words[] = {
    /* GOTOUPSY */ ORDER(RTB), /* "END OF HUNTEST" */
    ADDRESS(P65),              /* "HUNTEST USE OF GRP4 IS DISABLED BY P65" */
};

const struct interpretive_code GOTOUPSY_CODE = {025u, GOTOUPSY, COUNT(gotoupsy_words),
                                                gotoupsy_words};

#endif
