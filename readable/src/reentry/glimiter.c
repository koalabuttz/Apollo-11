/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 871-873 (Comanche055, bank 25): the end of the
 * final phase (HAVDNRNG), the g limiter (GLIMITER) every phase's L/D goes
 * through, and the lateral logic and the roll command (LIMITL/D to
 * ENDEXIT). The faithful C is c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- HAVDNRNG, GLIMITER -- pp. 871-872 ---- */

/* HAVDNRNG, with the range error: DNRNGERR "= (PREDANG - THETA) /360",
 * "FALL SHORT IF NEG, OVERSHOOT IF POS"; the L/D for it, "L/D = LOD +
 * (THETA- PREDANG)/ Y", with "FX= DRANGE/D L/D = Y", GOMAXL/D when it
 * overflows; on to GLIMITER.
 *
 * GONEGLAD, past the target: "SET L/D = -LAD" ("ANY NEGATIVE NUMBER WILL
 * DO", the word at GONEGLAD itself). GOMAXL/D: "L/D = LAD SIGN(MPAC)", "AND
 * FALL INTO GLIMITER SECTION".
 *
 * GLIMITER: "IF GMAX/2-D POS, GO TO LIMITL/D"; "IF GMAX  -D NEG, GO TO
 * GOPOSLAD", full lift up. Between: "2HS(GMAX-D) (LEQ/GMAX+LAD) INTO PD",
 * and "XLIM = SQRT(PD+(2HSGMAX/V)SQ)", the sink rate the drag can take:
 * "IF RDOT+XLIM POS, GO TO LIMITL/D", else lift up (GOPOSLAD).
 *
 * LIMITL/D: L/D1, the command, from L/D; "NO LATERAL CONTROL IF PAST
 * TARGET". "Y= KLAT VSQUARE + LATBIAS", the crossrange the CM may stray.
 * L350: "IF ABS(L/D)-L/DCMINR NEG, GO TO L353"; "IF K2ROLL LATANG NEG, GO
 * TO L357" (the least L/D the lateral control may have), and "Y = Y/2".
 * L353: "IF LATANG SIGN(K2ROLL)-Y POS, SWITCH" the side the roll is on,
 * "K2ROLL = -K2ROLL", unless NOSWITCH. L355: "ROLLC = ACOS( (L/D1) / LAD)"
 * on K2ROLL's side, NOSWITCH cleared; ENDEXIT, out of the interpreter. */
static const word havdnrng_words[] = {
    /* HAVDNRNG */ STORE(DNRNGERR), /* "= (PREDANG - THETA) /360" */
    ORDER(DCOMP),                   /* "FALL SHORT IF NEG, OVERSHOOT IF POS" */
    ORDERS(BOVB, DDV),
    ADDRESS(TCDANZIG),    /* "CLEAR OVFIND IF ON." */
    ADDRESS_PLUS_ONE(FX), /* "FX= DRANGE/D L/D = Y" */
    ORDERS(SL, BOV),
    SL_COUNT(5),
    ADDRESS(GOMAXL_D),
    ORDERS(DAD, BOV),
    ADDRESS_PLUS_ONE(LOD),
    ADDRESS(GOMAXL_D),
    STCALL(L_D),
    ADDRESS(GLIMITER),               /* "(GO TO)" */
    /* GONEGLAD */ ORDER(DLOAD),     /* "SET L/D = -LAD" */
    ADDRESS_PLUS_ONE(GONEGLAD),      /* "(ANY NEGATIVE NUMBER WILL DO)" */
    /* GOMAXL/D */ ORDERS(RTB, DMP), /* "L/D = LAD SIGN(MPAC)" */
    ADDRESS(SIGNMPAC),
    ADDRESS_PLUS_ONE(LAD),
    STORE(L_D),                        /* "AND FALL INTO GLIMITER SECTION" */
    /* GLIMITER */ ORDERS(DLOAD, DSU), /* "IF GMAX/2-D POS, GO TO LIMITL/D" */
    ADDRESS_PLUS_ONE(GMAX_2),
    ADDRESS_PLUS_ONE(D),
    ORDERS(BPL, DAD), /* "IF GMAX  -D NEG, GO TO GOPOSLAD" */
    ADDRESS(LIMITL_D),
    ADDRESS_PLUS_ONE(GMAX_2),
    ORDERS(BMN, DMP),
    ADDRESS(GOPOSLAD),
    ADDRESS_PLUS_ONE(2HS),
    ORDERS(PDDL, DMP), /* "2HS(GMAX-D) INTO PD" */
    ADDRESS_PLUS_ONE(LEQ),
    ADDRESS_PLUS_ONE(1_GMAX),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(LAD),
    ORDERS(PDDL, DDV), /* "2HS(GMAX-D) (LEQ/GMAX+LAD) INTO PD" */
    ADDRESS_PLUS_ONE(2HSGMXSQ),
    ADDRESS_PLUS_ONE(VSQUARE),
    ORDERS(DAD, SQRT), /* "XLIM = SQRT(PD+(2HSGMAX/V)SQ)" */
    ORDERS(DAD, BPL),  /* "IF RDOT+XLIM POS, GO TO LIMITL/D" */
    ADDRESS_PLUS_ONE(RDOT),
    ADDRESS(LIMITL_D),
    /* GOPOSLAD */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(LAD),
    /* STOREL/D */ STORE(L_D),
    /* LIMITL/D */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(L_D),
    STODL(L_D1),
    ADDRESS_PLUS_ONE(VSQUARE),
    ORDER(BON), /* "NO LATERAL CONTROL IF PAST TARGET" */
    SWITCH(BON, GONEPAST),
    ADDRESS(L355),
    ORDERS(DMP, DAD), /* "Y= KLAT VSQUARE + LATBIAS" */
    ADDRESS_PLUS_ONE(KLAT),
    ADDRESS_PLUS_ONE(LATBIAS),    /* "Y INTO PD" */
    /* L350 */ ORDERS(PDDL, ABS), /* "IF ABS(L/D)-L/DCMINR NEG, GO TO L353" */
    ADDRESS_PLUS_ONE(L_D),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(L_DCMINR),
    ADDRESS(L353),
    ORDERS(DLOAD, SIGN), /* "IF K2ROLL LATANG NEG, GO TO L357" */
    ADDRESS_PLUS_ONE(LATANG),
    ADDRESS_PLUS_ONE(K2ROLL),
    ORDERS(BMN, DLOAD),
    ADDRESS(L357),
    ORDERS(SR1, PUSH),              /* "Y = Y/2" */
    /* L353 */ ORDERS(DLOAD, SIGN), /* "IF LATANG SIGN(K2ROLL)-Y POS, SWITCH" */
    ADDRESS_PLUS_ONE(LATANG),
    ADDRESS_PLUS_ONE(K2ROLL),
    ORDER(DSU),
    ORDERS(BMN, DLOAD),
    ADDRESS(L355),
    ADDRESS_PLUS_ONE(K2ROLL),
    ORDERS(BONCLR, DCOMP), /* "IF NOSWITCH =1, K2ROLL= K2ROLL" */
    SWITCH(BONCLR, NOSWITCH),
    ADDRESS(L355),
    STORE(K2ROLL),                 /* "K2ROLL = -K2ROLL" */
    /* L355 */ ORDERS(DLOAD, DDV), /* "ROLLC = ACOS( (L/D1) / LAD)" */
    ADDRESS_PLUS_ONE(L_D1),
    ADDRESS_PLUS_ONE(LAD), /* "MPAC SET TO +-1 IF OVERFLOW***" */
    ORDERS(SR1, ACOS),
    ORDERS(SIGN, CLEAR),
    ADDRESS_PLUS_ONE(K2ROLL),
    SWITCH(CLEAR, NOSWITCH),
    STORE(ROLLC),
    /* ENDEXIT */ ORDER(EXIT),
};

const struct interpretive_code HAVDNRNG_CODE = {025u, HAVDNRNG, COUNT(havdnrng_words),
                                                havdnrng_words};

#endif
