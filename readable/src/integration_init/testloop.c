/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, RVCON and TESTLOOP to DT/2MAX (LM pp.
 * 1217-1219, CM pp. 1321-1324), in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * RVCON is the conic: the state at TDEC in one step, by Kepler's equation
 * (KEPPREP). TESTLOOP is the precision integration's loop: each pass
 * picks the next time step, DT/2 (half of it, B-19), and goes on at
 * TIMESTEP (ORBITAL_INTEGRATION), which comes back here; when what is
 * left is under DT/2MIN, A-PCHK. The step grows with the distance from
 * the central body, as the orbit's period does: 0.3 of the square root of
 * RC cubed over MU, "TRUNCATED TO A MULTIPLE" of 128 centiseconds, and at
 * most DT/2MAX. */
#include "integration_init.h"
#include "../agc/names.h"

static const word testloop_words[] = {
    /* RVCON -- LM p. 1217 (CM p. 1321): the interval TDEC - TET in TAU.,
     * RECTIFY and KEPPREP; the time of the new state, TET + TC (from
     * KEPPREP), in TET, and RECTOUT. */
    /* RVCON */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TDEC),
    ADDRESS_PLUS_ONE(TET),
    STCALL(TAU_),
    ADDRESS(RECTIFY),
    ORDER(CALL),
    ADDRESS(KEPPREP),
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(TC),
    ADDRESS_PLUS_ONE(TET),
    STCALL(TET),
    ADDRESS(RECTOUT),
    /* TESTLOOP -- LM p. 1218 (CM p. 1323): with QUITFLAG set, STATEFLG
     * cleared and INTEXIT ("STOP INTEGRATION"). Otherwise X2 the central
     * body (PBODY), and RC, the length of RCV, pushed at 10D ("RC TO
     * 10D"). MIDFLAG set when RC is at least RME,2 (RME, or the moon's
     * RMM). */
    /* TESTLOOP */ ORDERS(BOF, CLRGO),
    SWITCH(BOF, QUITFLAG),
    ADDRESS_AT(TESTLOOP, 5), /* (+3) */
    SWITCH(CLRGO, STATEFLG),
    ADDRESS(INTEXIT), /* "STOP INTEGRATION" */
    /* +3 */ ORDERS(SETPD, LXA_2),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS(PBODY),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(RCV),
    ORDERS(PUSH, CLEAR), /* "RC TO 10D" */
    SWITCH(CLEAR, MIDFLAG),
    ORDERS(DSU_STAR, BMN),
    ADDRESS_PLUS_ONE_AT_X2(RME, 0),
    ADDRESS(NORFINAL), /* (+3) */
    ORDER(SET),
    SWITCH(SET, MIDFLAG),
    /* NORFINAL: RC cubed (ABVAL left its square at 34D) over MU, the
     * square root times .3D; shifted to DT/2's scale and its low bits
     * shifted out ("DT IS TRUNCATED TO A MULTIPLE"), pushed at 12D. An
     * overflow, or more than DT/2MAX, is MAXDT. */
    /* NORFINAL */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS_PLUS_ONE_D(34),
    ORDERS(SR1R, DDV_STAR),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    ORDERS(SQRT, DMP),
    ADDRESS_PLUS_ONE(_3D),
    ORDERS(SR3, SR4), /* "DT IS TRUNCATED TO A MULTIPLE" */
    ORDERS(DLOAD, SL),
    ADDRESS_PLUS_ONE(MPAC),
    SL_COUNT(15), /* "OF 128 CSECS." */
    ORDERS(PUSH, BOV),
    ADDRESS(MAXDT),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE(DT_2MAX),
    ADDRESS(MAXDT),
    /* DT/2COMP: TDEC - TET, its words brought to one sign (SGNAGREE),
     * at B-19: DT/2. An overflow is GETMAXDT; a step under the largest
     * (12D) is POOHCHK. */
    /* DT/2COMP */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TDEC),
    ADDRESS_PLUS_ONE(TET),
    ORDERS(RTB, SL),
    ADDRESS(SGNAGREE),
    SL_COUNT(8),
    STORE(DT_2), /* "B-19" */
    ORDERS(BOV, ABS),
    ADDRESS(GETMAXDT),
    ORDERS(DSU, BMN), /* "IS TIME TO INTEG. TO GR THAN MAXTIME" */
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS(POOHCHK),
    /* USEMAXDT -- LM p. 1218 (CM p. 1323): the largest step, with DT/2's
     * sign, as DT/2. */
    /* USEMAXDT */ ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS_PLUS_ONE(DT_2),
    STCALL(DT_2),
    ADDRESS(POOHCHK),
    /* MAXDT -- LM p. 1219 (CM p. 1324): "EXCHANGE DT/2MAX WITH COMPUTED
     * MAX.", and DT/2COMP. */
    /* MAXDT */ ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(DT_2MAX),
    ORDER(GOTO),
    ADDRESS(DT_2COMP),
    /* GETMAXDT: TDEC - TET past DT/2's scale; its sign (SIGNMPAC) for
     * DT/2, and USEMAXDT. */
    /* GETMAXDT */ ORDER(RTB),
    ADDRESS(SIGNMPAC),
    STCALL(DT_2),
    ADDRESS(USEMAXDT),
    /* POOHCHK: a step under DT/2MIN, and the integration is done
     * (A-PCHK). Outside P00 (MODREG not zero), TIMESTEP. In P00 a
     * CSMPREC or LEMPREC ("WAS THIS CALL VIA CSM(LEM)PREC") takes the
     * step too; otherwise a step under the largest ends the integration
     * (A-PCHK), and so does one started anew (NEWIFLG, cleared) with
     * TDEC before TET ("NO BACKWARD INTEGRATION") or with TDEC - TET under
     * four steps. */
    /* POOHCHK */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(DT_2),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(DT_2MIN),
    ADDRESS(A_PCHK),
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(MODREG),
    ADDRESS_AT(POOHCHK, 012), /* (+3) */
    ORDER(GOTO),
    ADDRESS(TIMESTEP),
    ORDER(BON), /* "WAS THIS CALL VIA CSM(LEM)PREC" */
    SWITCH(BON, PRECIFLG),
    ADDRESS(TIMESTEP), /* "YES" */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(DT_2),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(BMN, BOFCLR),
    ADDRESS(A_PCHK),
    SWITCH(BOFCLR, NEWIFLG),
    ADDRESS(TIMESTEP),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TDEC),
    ADDRESS_PLUS_ONE(TET),
    ORDER(BMN), /* "NO BACKWARD INTEGRATION" */
    ADDRESS(INTEXIT),
    ORDERS(PDDL, SR4),
    ADDRESS_PLUS_ONE(DT_2), /* "IS 4(DT) LS" (TDEC - TET) */
    ORDERS(SR2R, BDSU),
    ORDERS(BMN, GOTO),
    ADDRESS(INTEXIT),
    ADDRESS(TIMESTEP),
    /* DT/2MIN and DT/2MAX: the bounds on DT/2. */
    /* DT/2MIN */ NUMBER(000000), /* 2DEC 3 B-20 */
    NUMBER(001400),
    /* DT/2MAX */ NUMBER(014152), /* 2DEC 4000 E2 B-20 */
    NUMBER(000000),
};

const struct interpretive_code TESTLOOP_CODE = {CODE_BANK(I_RVCON), CODE_Z(I_RVCON),
                                                COUNT(testloop_words), testloop_words};
