/* SPDX-License-Identifier: MIT */
/* TIME_OF_FREE_FALL.agc, both missions' (Luminary099 pp. 1268-1283,
 * Comanche055 pp. 1373-1388), in bank 27: the same code in each. The
 * interpretive words are in the listing's notation
 * (src/interpreter/notation.h); T(X) and ENDT(X), basic code, are C.
 *
 * "THE TFF SUBROUTINES MAY BE USED IN EITHER EARTH OR MOON CENTERED
 * COORDINATES." The user leaves the state vector in RONE and VONE and
 * "1/SQRT(MU)" in TFF/RTMU, at the earth's scale or the moon's; the
 * routines keep what they work out in the push-down list's words 10D to
 * 36D (TFFDELQ to TFFTEM), and the norm counts in X1 and X2:
 *
 * TFFCONIC (or TFFCONMU, with TFF/RTMU already loaded) works out the
 * conic: the present radius (RMAG1, and NRMAG normalized, its count in
 * X1), the "SEMI LATUS RECTUM" (TFFNP), "ALFA = 2/RN - VN.VN / MU" (the
 * reciprocal of the semi-major axis, TFFALFA), its root (TFFRTALF, the
 * count in X2) and "1/ALFA" (TFF1/ALF; 0 for an alfa too small).
 *
 * TFFRP/RA, from those, the perigee radius ("RP = P/(1+E)") into RPER, and
 * the apogee radius ("RA = (1+E) / ALFA") into RAPO, or POSMAX where it
 * has none.
 *
 * CALCTFF, the "FREE-FALL TIME OF FLIGHT FROM PRESENT POSITION RN AND
 * VELOCITY VN TO A RADIUS LENGTH SPECIFIED BY RTERM", or CALCTPER the time
 * to perigee, into MPAC in centiseconds; POSMAX where the conic never
 * reaches RTERM. The time is the series T(X) (POLY), with an ellipse's
 * period added where the time comes out negative, or, for an ellipse
 * "HAVING ABS(DEL ECC ANOM) GEQ 90 DEG", another form (TFFELL).
 *
 * The binding is src/binding/time_free_fall_routines.c. */
#include "time_free_fall.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static const word tff_words[] = {
    /* TFFCONIC -- LM p. 1271 (CM p. 1376): 1/SQRT(MU) from MPAC into
     * TFF/RTMU. TFFCONMU: the unit of RONE pushed, and its magnitude
     * into RMAG1 and, normalized, into NRMAG ("-NR" into X1). */
    /* TFFCONIC */ STORE(TFF_RTMU),
    /* TFFCONMU */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RONE),
    ORDER(PDDL), /* "UR/2 TO PDL+0, +5" */
    ADDRESS_PLUS_ONE_D(36),
    STORE(RMAG1),
    ORDER(NORM),
    ADDRESS_PLUS_ONE(X1),
    /* The velocity over the root of MU into VONE'; P, the semi-latus
     * rectum, the square of R x V over MU, into TFFNP. */
    STOVL(NRMAG),
    ADDRESS_PLUS_ONE(VONE),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(TFF_RTMU),
    STORE(VONE_PRIME), /* "VN/SQRT(MU)" */
    ORDERS(VXSC, VXV),
    ADDRESS_PLUS_ONE(NRMAG),
    ORDERS(VSL1, VSQ),
    STODL(TFFNP), /* "LC P" */
    ADDRESS_PLUS_ONE(TFF1_4),
    /* Alfa: 2/RMAG less the square of VONE' (into TFFVSQ, "-(V SQ/MU)"),
     * shifted by the count; its root's magnitude, normalized ("X2 =
     * -NA"), into TFFRTALF. */
    ORDERS(DDV, PDVL), /* "(2/RMAG)" */
    ADDRESS_PLUS_ONE(NRMAG),
    ADDRESS_PLUS_ONE(VONE_PRIME),
    ORDERS(VSQ, DCOMP), /* "KEEP MPAC+2 HONEST FOR SQRT." */
    STORE(TFFVSQ),
    ORDERS(SR_STAR, DAD),
    /* LM p. 1272 (CM p. 1377) */
    SR_X1(-6), /* "GET -VSQ/MU" */
    ORDER(STADR),
    AFTER_STADR(STORE(TFFALFA)),
    ORDERS(SL_STAR, PUSH), /* "TEMP SAVE ALFA" */
    SL_X1(-6),
    ORDERS(ABS, SQRT),
    ORDER(NORM),
    ADDRESS_PLUS_ONE(X2), /* "X2 = -NA" */
    STORE(TFFRTALF),
    /* 1/alfa, by the square of the root with alfa's sign: "SET 1/ALFA =0,
     * TO SHOW SMALL ALFA". */
    ORDERS(DSQ, SIGN), /* "NOT SO ACCURATE, BUT OK" */
    ORDERS(BZE, BDDV),
    ADDRESS_AT(TFFCONIC, 045), /* +2 */
    ADDRESS_PLUS_ONE(TFF1_4),
    /* +2 */ STORE(TFF1_ALF),
    /* DUMPCNIC */ ORDER(RVQ),

    /* TFFRP/RA -- LM p. 1274 (CM p. 1379): E, the root of 1 - P ALFA;
     * RPER, P/(1+E), shifted back by the count. */
    /* TFFRP/RA */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(TFFALFA),
    ADDRESS_PLUS_ONE(TFFNP),
    ORDERS(SR_STAR, DCOMP),
    SR_X1(-8), /* "ALFA P (-4)" */
    ORDERS(DAD, ABS),
    ADDRESS_PLUS_ONE(DP2_M4),
    ORDERS(SQRT, DAD),
    ADDRESS_PLUS_ONE(TFF1_4),
    ORDERS(PUSH, BDDV), /* "(1+E)  (-2)  TO PDL+0" */
    ADDRESS_PLUS_ONE(TFFNP),
    ORDERS(SR_STAR, SR_STAR), /* "(DOES SR THEN SL TO AVOID OVFL)" */
    SR_X1(0),
    SR_X1(-7), /* "(EFFECTIVE SL)" */
    STODL(RPER),
    /* RAPO, (1+E) times 1/ALFA, shifted by the count: POSMAX ("AS A TAG")
     * where alfa is 0, where it overflows or where it is negative. */
    ORDERS(DMP, BOVB),
    ADDRESS_PLUS_ONE(TFF1_ALF),
    ADDRESS(TCDANZIG), /* "CLEAR OVFIND, IF ON." */
    ORDERS(BZE, SL_STAR),
    ADDRESS(MAXRA), /* "SET POSMAX IF ALFA=0" */
    SL_X2(-5),
    ORDERS(SL_STAR, BOV),
    SL_X2(0),
    ADDRESS(MAXRA),       /* "SET POSMAX IF OVFL." */
    ORDER(BPL),           /* "CONTINUE WITH VALID RAPO." */
    ADDRESS_AT(MAXRA, 2), /* +3 */
    /* MAXRA */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(NEARONE),
    /* +3 */ STORE(RAPO),
    /* DUMPRPRA */ ORDER(RVQ),

    /* CALCTPER and CALCTFF -- LM p. 1277 (CM p. 1382): TFFSW set for the
     * time to perigee ("ENTER WITH RPER IN MPAC"), clear for the time to
     * RTERM ("ENTER WITH RTERM IN MPAC"). */
    /* CALCTPER */ ORDER(SETGO),
    SWITCH(SETGO, TFFSW),
    ADDRESS_AT(CALCTFF, 2), /* +3 */
    /* CALCTFF */ ORDER(CLEAR),
    SWITCH(CLEAR, TFFSW),
    /* +3 */ STORE(RTERM),
    ORDER(SL_STAR),
    SL_X1(0),
    STORE(NRTERM),
    /* Q2, the root of RTERM (2 - RTERM ALFA) - P (0 for the time to
     * perigee), negative ("INBOUND SIDE"); none to RTERM: MAXTFF1. */
    ORDERS(DMP, BDSU),
    ADDRESS_PLUS_ONE(TFFALFA),
    ADDRESS_PLUS_ONE(TFF1_4),
    ORDERS(PUSH, DMP), /* "(2-ALFA RTERM)  (-3)  TO PDL+0" */
    ADDRESS_PLUS_ONE(NRTERM),
    ORDERS(PDDL, SR_STAR), /* "RTERM(2-ALFA RTERM) TO PDL+2" */
    ADDRESS_PLUS_ONE(TFFNP),
    SR_X1(-6),
    ORDERS(DCOMP, DAD), /* "DUE TO SHIFTS, KEEP PRECISION FOR SQRT" */
    ORDER(SR_STAR),
    SR_X1(0),
    ORDERS(BOFF, DLOAD), /* "CHECK TFF /TPER SWITCH" */
    SWITCH(BOFF, TFFSW),
    ADDRESS_AT(CALCTFF, 025),   /* +2: "IF TFF, CONTINUE" */
    ADDRESS_PLUS_ONE(TFFZEROS), /* "IF TPER, SET Q2 = 0" */
    /* +2 */ ORDERS(BMN, SQRT),
    ADDRESS(MAXTFF1),    /* "NO FREE FALL CONIC TO RTERM FROM HERE" */
    ORDERS(DCOMP, BOVB), /* "RT IS ON INBOUND SIDE.  ASSURE OVFIND=0" */
    ADDRESS(TCDANZIG),   /* "ANY PORT IN A STORM." */
    /* Q1, RN . VN over the root of MU, into TFFQ1; outbound (Q1
     * positive), NUM = Q2 - Q1 into TFFX, and DEN = 2 - ALFA (RTERM +
     * RN); inbound, INBOUND's. */
    STOVL(TFFTEM), /* "Q2" */
    ADDRESS_PLUS_ONE(VONE_PRIME),
    ORDERS(DOT, SL3),
    ADDRESS_PLUS_ONE(RONE),
    STORE(TFFQ1), /* "Q1, SAVE FOR GONEPAST TEST." */
    ORDERS(BMN, BDSU),
    ADDRESS(INBOUND), /* "USE ALTERNATE Z" */
    ADDRESS_PLUS_ONE(TFFTEM),
    STODL(TFFX), /* "NUM=Q2-Q1" */
    ADDRESS_PLUS_ONE(TFFALFA),
    ORDERS(DMP, BDSU),
    ADDRESS_PLUS_ONE(NRMAG),
    /* SAVEDEN -- LM p. 1278 (CM p. 1383): DEN pushed; with its magnitude
     * at least B(-22), on to TFFXTEST; else DEN 0, and for the time to
     * perigee of an ellipse TFFEL1 ("ASSUME 90"), or a time 0. */
    /* SAVEDEN */ ORDERS(PUSH, ABS), /* "DEN TO PDL+0" */
    ORDERS(DAD, BOV),                /* "INDETERMINANCY TEST" */
    ADDRESS_PLUS_ONE(LIM_M22),
    ADDRESS(TFFXTEST),   /* "GO IF DEN >/= B(-22)" */
    ORDERS(DLOAD, PDDL), /* "SET DEN=0 OTHERWISE" */
    ADDRESS_PLUS_ONE(TFFZEROS),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(TFFALFA),
    ORDERS(BMN, DLOAD),
    ADDRESS(TFFEL1),
    /* DUMPTFF1 */ ORDER(RVQ),  /* "RETURN TFF =0" */
                                /* INBOUND: NUM = RTERM - RN, and DEN = Q2 + Q1. */
    /* INBOUND */ ORDER(DLOAD), /* "RESET PDL+0" */
    ORDERS(DLOAD, DSU),         /* "ALTERNATE Z CALC" */
    ADDRESS_PLUS_ONE(RTERM),
    ADDRESS_PLUS_ONE(RMAG1),
    STODL(TFFX), /* "NUM=RTERM-RN" */
    ADDRESS_PLUS_ONE(TFFTEM),
    ORDERS(DAD, GOTO),
    ADDRESS_PLUS_ONE(TFFQ1),
    ADDRESS(SAVEDEN), /* "DEN = Q2+Q1" */
                      /* TFFXTEST: NUM times the root of alfa over the magnitude of DEN; its
     * overflow is an ellipse past 90 degrees, TFFELL. Else Z = NUM/DEN
     * into TFFTEM. */
    /* TFFXTEST */ ORDERS(DAD, PDDL),
    ADDRESS_PLUS_ONE(DP_M22), /* "RESTORE ABS(DEN) TO MPAC" */
    ADDRESS_PLUS_ONE(TFFX),
    ORDERS(DMP, SR_STAR),
    ADDRESS_PLUS_ONE(TFFRTALF),
    SR_X2(-3), /* "X2=-NA" */
    ORDER(DDV),
    ORDERS(DLOAD, BOV), /* "(THE DLOAD IS SHARED WITH TFFELL)" */
    ADDRESS_PLUS_ONE(TFFX),
    ADDRESS(TFFELL), /* "USE EQN FOR DELE GEQ 90, LEQ -90" */
    ORDERS(DDV, STADR),
    AFTER_STADR(STORE(TFFTEM)), /* "Z SAVE FOR SIGN OF SDELF." */
    /* P Z Z, with the sign of Z, into TFFTEM for the entry display
     * ("ARG IS USED IN TFF/TRIG"); X = ALFA Z Z into TFFX. */
    ORDERS(PUSH, DSQ), /* "Z TO PDL+0" */
    ORDERS(PUSH, DMP),
    ADDRESS_PLUS_ONE(TFFNP),
    ORDERS(SL, SIGN),
    SL_COUNT(5),
    ADDRESS_PLUS_ONE(TFFTEM), /* "AFFIX SIGN FOR SDELF (ENTRY DISPLAY)" */
    STODL(TFFTEM),
    ORDERS(PUSH, DMP), /* "RESTORE PUSH LOC" */
    ADDRESS_PLUS_ONE(TFFALFA),
    ORDER(SL_STAR),
    SL_X1(0),
    STORE(TFFX),
    /* T(X), the series, by RTB (the right-hand order waits in EDOP):
     * TFF = (RTERM + RN - 2 Z Z T(X)) Z, over the root of MU at ENDTFF. */
    ORDERS(RTB, DMP),
    ADDRESS(T_X), /* "POLY" */
    ORDERS(SR2, BDSU),
    ADDRESS_PLUS_ONE(RTERM),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(RMAG1),
    ORDERS(SR3, BPL),
    ADDRESS(ENDTFF), /* "(NO PUSH UP)" */
    /* Negative: with Q1 positive (gone past), NEGTFF; for a hyperbola
     * too; for an ellipse, the period added, "2 PI /(ALFA SQRT(ALFA))". */
    ORDERS(PUSH, SIGN),      /* "TFF SQRT(MU) TO PDL+0" */
    ADDRESS_PLUS_ONE(TFFQ1), /* "Q1 FOR GONEPAST TEST" */
    ORDERS(BPL, DLOAD),      /* "GONE PAST ?" */
    ADDRESS(NEGTFF),         /* "YES. TFF < 0." */
    ADDRESS_PLUS_ONE(TFF1_ALF),
    ORDERS(DCOMP, BPL), /* "ALFA > 0 ?" */
    ADDRESS(NEGTFF),    /* "NO. TFF IS NEGATIVE." */
    ORDER(DCOMP),       /* "YES.  CORRECT FOR ORB PERIOD." */
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(PI_16), /* "2 PI (-5)" */
    ADDRESS_PLUS_ONE(TFFRTALF),
    ORDERS(SL_STAR, SL_STAR),
    SL_X2(-4), /* "X2=-NA" */
    SL_X2(-4),
    ORDERS(SL_STAR, DAD),
    SL_X2(0),
    /* ENDTFF -- LM p. 1279 (CM p. 1384): times 1/SQRT(MU), the time in
     * centiseconds; POSMAX on overflow. */
    /* ENDTFF */ ORDERS(DMP, BOV),
    ADDRESS_PLUS_ONE(TFF_RTMU),
    ADDRESS(MAXTFF),           /* "SET POSMAX IN OVFL." */
    /* DUMPTFF2 */ ORDER(RVQ), /* "RETURN TFF (-28) CS IN MPAC." */
    /* NEGTFF -- LM p. 1280 (CM p. 1385): the time from the list, negative. */
    /* NEGTFF */ ORDER(DLOAD),
    ORDER(GOTO),
    ADDRESS(ENDTFF),
    /* MAXTFF1 */ ORDER(DLOAD), /* "RESET PDL" */
    /* MAXTFF */ ORDERS(DLOAD, RVQ),
    ADDRESS_PLUS_ONE(NEARONE),
    /* TFFELL: "TIME OF FLIGHT ELLIPSE WHEN DEL (ECCENTRIC ANOM) GEQ 90 AND
     * LEQ -90." D/N (DEN over NUM) pushed; TFFEL1 enters with 0 there. */
    /* TFFELL */ ORDER(SL2),
    ORDERS(BDDV, PUSH),              /* "TEMP SAVE D/N IN PDL+0" */
    /* TFFEL1 */ ORDERS(DLOAD, DSU), /* "(ENTER WITH D/N=0 IN PDL+0)" */
    ADDRESS_PLUS_ONE(TFFTEM),
    ADDRESS_PLUS_ONE(TFFQ1),
    STODL(TFFDELQ), /* "Q2-Q1" */
    ORDER(STADR),
    AFTER_STADR(STORE(TFFTEM)),
    /* X = 1/ALFA Z Z into TFFX ("IN CASE X= 1.0, CONTINUE" with its
     * overflow), and the series by RTB T(X). */
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE(TFF1_ALF),
    SL_X2(0),
    ORDERS(PUSH, DMP),
    ADDRESS_PLUS_ONE(TFFTEM),
    ORDERS(SL_STAR, BOVB),
    SL_X2(0),          /* "X2= -NA" */
    ADDRESS(SIGNMPAC), /* "IN CASE X= 1.0, CONTINUE" */
    STORE(TFFX),       /* "X=1/ALFA ZSQ" */
    ORDERS(RTB, DMP),
    ADDRESS(T_X), /* "POLY" */
    ADDRESS_PLUS_ONE(TFFX),
    ORDERS(SR3, DSU),
    ADDRESS_PLUS_ONE(DP2_M3),
    ORDERS(DMP, PUSH),
    /* The sign of SDELF, Q1 + R/Z, into TFFTEM, and P/ALFA with it for
     * the entry display. */
    ORDERS(DLOAD, DMP), /* "GET SIGN FOR SDELF" */
    ADDRESS_PLUS_ONE(TFFTEM),
    ADDRESS_PLUS_ONE(RMAG1),
    ORDERS(SL2, DAD),
    ADDRESS_PLUS_ONE(TFFQ1),
    STODL(TFFTEM), /* "(Q1+R 1/Z) =SGN OF SDELF" */
    ADDRESS_PLUS_ONE(TFFNP),
    ORDERS(DMP, SL_STAR), /* "CALC FOR ARG FOR TFF/TRIG." */
    ADDRESS_PLUS_ONE(TFF1_ALF),
    SL_X2(1),
    ORDERS(SIGN, SL_STAR),
    ADDRESS_PLUS_ONE(TFFTEM), /* "AFFIX SIGN FOR SDELF" */
    SL_X2(0),
    STODL(TFFTEM), /* "P/ALFA" */
    ADDRESS_PLUS_ONE(TFF1_ALF),
    /* TFF = (PI/SQRT(ALFA) - Q2 + Q1 + 2(X T(X) - 1)/ALFA Z)/ALFA, to
     * ENDTFF. */
    ORDERS(SQRT, DMP),
    ADDRESS_PLUS_ONE(PI_16), /* "PI (-4)" */
    ORDER(DAD),
    ORDERS(SL_STAR, DSU),
    SL_X2(-1),
    ADDRESS_PLUS_ONE(TFFDELQ),
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE(TFF1_ALF),
    SL_X2(-3),
    ORDERS(SL_STAR, GOTO),
    SL_X2(-4),
    ADDRESS(ENDTFF), /* "TFF SQRT(MU) IN MPAC" */
};

const struct interpretive_code TFF_CODE = {CODE_BANK(I_TFFCONIC), CODE_Z(I_TFFCONIC),
                                           COUNT(tff_words), tff_words};

/* T(X) -- LM p. 1282 (CM p. 1387): TC POLY, the series in the words
 * after it, DEC 4 ("N-1") and five 2DECs from 1/3 on; POLY returns past
 * them, to ENDT(X). */
void t_x(void)
{
    tc_at(T_X);
    continue_at(POLY);
}

/* ENDT(X), and TCDANZIG the same word: TC DANZIG, which finds the RTB's
 * right-hand order in EDOP. */
void endt_x(void)
{
    tc_at(ENDT_X);
    continue_at(DANZIG);
}

/* The series' coefficients, after T(X)'s TC POLY. */
static const word series_words[] = {
    NUMBER(000004),                 /* DEC 4 */
    NUMBER(012525),                 /* 2DEC 3.333333333 E-1 */
    NUMBER(012525), NUMBER(071463), /* 2DEC* -1.999819135 E-1* */
    NUMBER(057703), NUMBER(004423), /* 2DEC* 1.418148467 E-1* */
    NUMBER(017645), NUMBER(074604), /* 2DEC* -1.01310997 E-1* */
    NUMBER(043667), NUMBER(001626), /* 2DEC* 5.609004986 E-2* */
    NUMBER(037256), NUMBER(077404), /* 2DEC* -1.536156925 E-2* */
    NUMBER(052071),
};

const struct interpretive_code TFF_SERIES = {CODE_BANK(I_T_X), (word)(CODE_Z(I_T_X) + 1u),
                                             COUNT(series_words), series_words};

/* The constants -- LM p. 1283 (CM p. 1388): the CM's 1/RTMU first
 * ("MODIFIED EARTH MU"); PI/16, 1.0 - B(-22), B(-22), B-3 and B-4; R300K,
 * 300 km, which no code of the file reads; and NEARONE, POSMAX's
 * stand-in. TFFZEROS and TFF1/4 are HI6ZEROS and HIDP1/4. */
static const word constant_words[] = {
#if COMANCHE055
    /* 1/RTMU */ NUMBER(024775), /* 2DEC* .5005750271 E-5 B17* */
    NUMBER(030424),
#endif
    /* PI/16 */ NUMBER(006220), /* 2DEC 3.141592653 B-4 */
    NUMBER(037553),
    /* LIM(-22) */ NUMBER(037777), /* "1.0 -B(-22)" */
    NUMBER(037700),
    /* DP(-22) */ NUMBER(000000), /* "B(-22)" */
    NUMBER(000100),
    /* DP2(-3) */ NUMBER(004000), /* 2DEC 1 B-3 */
    NUMBER(000000),
    /* DP2(-4) */ NUMBER(002000), /* "1/16" */
    NUMBER(000000),
    /* R300K */ NUMBER(000305), /* 2DEC 6464778 B-29 */
    NUMBER(011205),
    /* NEARONE */ NUMBER(037777), /* 2DEC .999999999 */
    NUMBER(037777),
};

#if COMANCHE055
const struct interpretive_code TFF_CONSTANTS = {CODE_BANK(I_1_RTMU), CODE_Z(I_1_RTMU),
                                                COUNT(constant_words), constant_words};
#else
const struct interpretive_code TFF_CONSTANTS = {CODE_BANK(I_PI_16), CODE_Z(I_PI_16),
                                                COUNT(constant_words), constant_words};
#endif
