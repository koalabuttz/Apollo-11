/* SPDX-License-Identifier: MIT */
/* TPI_SEARCH.agc of the CM, part 1 (Comanche055 pp. 551-556, bank 36):
 * S17.1, S17.2, BIS, CONCAUL, ADVCSM and JUNCT3.
 *
 * "BOTH ROUTINES ARE CALLED IN INTERPRETIVE CODE AND RETURN VIA QPRET"
 * (S17.1 through NORMEX, S17.2 through QTEMP, where each keeps its
 * return); "S17.2 RETURNS VIA QPRET FOR NORMAL EXIT AND TO ALARUMS FOR
 * ERROR EXIT". The constants they read (HPE to DP-.002, "**** TEMPORARY
 * ****") sit in bank 35, ahead of P17.
 *
 * All of it is interpretive code, written out as the listing has it
 * (src/interpreter/notation.h), a table for each label a pass can start
 * at. */
#include "tpi_search.h"
#include "../agc/names.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S17.1 -- pp. 552-553 ---- */

/* S17.1: both vehicles to the TPI time TTPI by conic extrapolation, "THE
 * STATE VECTORS OF BOTH VEHICLES" (LEMCONIC and LEMSTORE for the passive
 * one, to RPASS3 and VPASS3; CSMCONIC and CSMSTORE for the active one, to
 * RACT3 and VACT3). Then the altitude difference DELHITE, /RP/ - /RA/,
 * with KFLAG set if it is negative (BMN +2 skips the CLEAR): "OFF = +",
 * "ON = -". E2, the unit normal to the passive orbit (RP cross VP), is
 * kept for S17.2 ("ALMOST IT SAVE FOR 17.2"). The active's position
 * projected into the passive's plane ("RPA") gives THETZERO, the "CENTRAL
 * ANGLE" between the two, signed by which side of the
 * passive the active is: "+= ACTIVE AHEAD -= ACTIVE BEHIND". X1 and X2,
 * the scale indexes the conic routines left, go to XRS ("SAVE INDICES FOR
 * FURTHER USE"), and S17.1 returns through NORMEX. */
static const word s17_1_words[] = {
    /* S17.1 */ ORDERS(STQ, DLOAD),
    ADDRESS(NORMEX),
    ADDRESS_PLUS_ONE(TTPI),
    STCALL(TDEC1), /* "ADVANCE PASSIVE VEHICLE TO TPI" */
    ADDRESS(LEMCONIC),
    ORDER(CALL),
    ADDRESS(LEMSTORE),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPI),
    STCALL(TDEC1), /* "ADVANCE ACTIVE VEHICLE TO TPI" */
    ADDRESS(CSMCONIC),
    ORDER(CALL),
    ADDRESS(CSMSTORE),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(ABVAL, PDVL), /* /RA/ to 0D */
    ADDRESS_PLUS_ONE(RPASS3),
    ORDERS(UNIT, PDDL), /* the unit RP to 0D (PDDL takes /RA/ back first) */
    ORDERS(BDSU, SET),
    ADDRESS_PLUS_ONE_D(36), /* "/RP/ -/RA/" (UNIT left /RP/ in 36D) */
    SWITCH(SET, KFLAG),     /* "OFF = +" */
    ORDERS(BMN, CLEAR),
    ADDRESS_AT(S17_1, 030), /* +2 */
    SWITCH(CLEAR, KFLAG),   /* "ON = -" */
    STOVL(DELHITE),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VPASS3),
    STOVL(E2), /* "ALMOST IT SAVE FOR 17.2" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PUSH, VPROJ),
    ADDRESS_PLUS_ONE(E2),
    ORDERS(VSL2, BVSU), /* "RPA" */
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SL1, ACOS),
    ORDER(PDVL),
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE(RACT3),
    ADDRESS_PLUS_ONE(E2),
    ORDERS(PDDL, SIGN),
    ORDER(STADR),
    AFTER_STADR(STODL(THETZERO)), /* "CENTRAL ANGLE" */
    ADDRESS_PLUS_ONE(X1),
    STCALL(XRS),     /* "SAVE INDICES FOR FURTHER USE" */
    ADDRESS(NORMEX), /* "+= ACTIVE AHEAD  -= ACTIVE BEHIND" */
};

const struct interpretive_code S17_1_CODE = {036u, S17_1, COUNT(s17_1_words), s17_1_words};

/* ---- S17.2 -- pp. 553-554 ---- */

/* S17.2, "COMPUTE SEARCH SECTOR LIMITS": the sine of the active's angle
 * out of the passive's plane (its unit position dotted with E2), the
 * square root of its magnitude doubled, and .002 more ("ADD .002 RADIANS
 * TO IT"); with KFLAG clear complemented (BON +1 skips the DCOMP); times
 * PIINVERS, 1/PI ("REVOLUTIONARY HERES TWO IT"). From it and THETZERO the
 * sector's two angles, as the listing gives them:
 * "PHI(0)=180-(-(THETAZERO +K5IT)), PHI(I)=180-(-THETAZERO+K2IT))", since
 * "SIN(180-ALPHA)=SIN(ALPHA) ETC". IT gets "PHI(I) , -(THETZERO + K2IT)",
 * and the list "PHI(0) , -(THETZERO + K5IT)". */
static const word s17_2_words[] = {
    /* S17.2 */ ORDERS(STQ, VLOAD), /* "COMPUTE SEARCH SECTOR LIMITS" */
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(E2),
    ORDERS(ABS, SQRT),
    ORDERS(SL1, DAD),
    ADDRESS_PLUS_ONE(DPM_002), /* "ADD .002 RADIANS TO IT" */
    ORDERS(BON, DCOMP),        /* "GIVES CORRECT SINE, COSINE MUST BE" */
    SWITCH(BON, KFLAG),        /* "COMP. ADD .5 FOR ANGLE" */
    ADDRESS_AT(S17_2, 013),    /* +1 */
    ORDERS(DMP, SETPD),
    ADDRESS_PLUS_ONE(PIINVERS), /* "REVOLUTIONARY HERES TWO IT" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(THETZERO),
    STORE(IT), /* "PHI(I) , -(THETZERO + K2IT)" */
    ORDERS(PDDL, PUSH),
    ORDERS(SR1, DAD),
    ORDERS(DAD, PUSH), /* "PHI(0) , -(THETZERO + K5IT)" */

    /* The search's start: the sine and cosine of PHI(0) to SNTH and CSTH
     * (the cosine complemented unless it is negative: BMN +2 skips the
     * DCOMP), RVSW set (TIMETHET is to compute no final state), the
     * passive's TPI state scaled by X2 (XRS) to RVEC and VVEC, and
     * TIMETHET, the time the passive takes through the angle, to TF and
     * TFO ("SAVE START TIME AND GET END TIME"). */
    ORDERS(SIN, SET),
    SWITCH(SET, RVSW),
    STODL(SNTH),
    ORDERS(COS, BMN),
    ADDRESS_AT(S17_2, 032), /* +2 */
    ORDER(DCOMP),
    STODL(CSTH),
    ADDRESS_PLUS_ONE(XRS),
    STOVL(X1),
    ADDRESS_PLUS_ONE(RPASS3),
    ORDER(VSR_STAR),
    SR_X2(0),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(VPASS3),
    ORDER(VSR_STAR),
    SR_X2(0),
    STCALL(VVEC),
    ADDRESS(TIMETHET),
    ORDER(DLOAD), /* "SAVE START TIME AND GET END TIME" */
    ADDRESS_PLUS_ONE(T),
    STORE(TF),
    STODL(TFO),

    /* Its end: the same from PHI(I) (IT), with X1 from XRS again, to
     * TFI. */
    ADDRESS_PLUS_ONE(IT),
    ORDERS(PUSH, SIN),
    STODL(SNTH),
    ORDERS(COS, BMN),
    ADDRESS_AT(S17_2, 060), /* +2 */
    ORDER(DCOMP),
    STORE(CSTH),
    ORDERS(LXA_1, CALL),
    ADDRESS(XRS),
    ADDRESS(TIMETHET),

    /* "INITIALIZE LOOP": ITSWICH clear (each Lambert answer tested), TFI
     * the end time ("SAVE TIME FOR LOOP TEST"), DELVEE the largest number,
     * HP MANYFEET (-1.0 B-2), and THETL, the limit on the transfer angle,
     * SEC1THET ("70 DEGREES") with KFLAG set, else SEC2THET ("330
     * DEGREES"); then the first step, at CONCAUL. */
    ORDERS(DLOAD, CLEAR),
    ADDRESS_PLUS_ONE(T),
    SWITCH(CLEAR, ITSWICH),
    STODL(TFI), /* "SAVE TIME FOR LOOP TEST" */
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STODL(DELVEE),
    ADDRESS_PLUS_ONE(MANYFEET),
    STODL(HP),
    ADDRESS_PLUS_ONE(SEC1THET), /* "70 DEGREES" */
    ORDERS(BON, DLOAD),
    SWITCH(BON, KFLAG),
    ADDRESS_AT(S17_2, 0101),    /* +2 */
    ADDRESS_PLUS_ONE(SEC2THET), /* "330 DEGREES" */
    STCALL(THETL),
    ADDRESS(CONCAUL),
};

const struct interpretive_code S17_2_CODE = {036u, S17_2, COUNT(s17_2_words), s17_2_words};

/* ---- BIS, CONCAUL and ADVCSM -- pp. 554-555 ---- */

/* BIS, where the loop comes back to with the next TF: the last transfer
 * angle, from its cosine and sine in CSTH and SNTH (halved for ARCTRIG,
 * "GET 4 QUADRANT THETA"), made positive ("PUT THETA BETWEEN 0,1"). TF
 * less TFI, signed by THETL less the angle ("FAST TIMES"): negative, and
 * the search has run past its end (RNGETEST, "TIME MUST HAVE A STOP"). */
static const word bis_words[] = {
    /* BIS */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(CSTH),
    STODL(COSTH),
    ADDRESS_PLUS_ONE(SNTH),
    ORDER(SR1),
    STCALL(SINTH), /* "GET 4 QUADRANT THETA" */
    ADDRESS(ARCTRIG),
    ORDERS(BPL, DAD),
    ADDRESS_AT(BIS, 012),       /* +2 */
    ADDRESS_PLUS_ONE(DPPOSMAX), /* "PUT THETA BETWEEN 0,1" */
    ORDERS(BDSU, PDDL),
    ADDRESS_PLUS_ONE(THETL),
    ADDRESS_PLUS_ONE(TF),
    ORDERS(DSU, SIGN), /* "FAST TIMES" */
    ADDRESS_PLUS_ONE(TFI),
    ORDER(BMN),
    ADDRESS(RNGETEST), /* "TIME MUST HAVE A STOP" */
};

const struct interpretive_code BIS_CODE = {036u, BIS, COUNT(bis_words), bis_words};

/* CONCAUL, "ADVANCE PASSIVE FOR TARGET VECTOR": the passive to TTPI + TF,
 * the transfer's end, by conic extrapolation (LEMCONIC; with AVFLAG set,
 * "LEM IS ACTIVE", the CSM's, ADVCSM); then JUNCT3. */
static const word concaul_words[] = {
    /* CONCAUL */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPI),
    ORDERS(DAD, BON),
    ADDRESS_PLUS_ONE(TF),
    SWITCH(BON, AVFLAG),
    ADDRESS(ADVCSM),
    STCALL(TDEC1),
    ADDRESS(LEMCONIC),
    ORDER(GOTO),
    ADDRESS(JUNCT3),
    /* ADVCSM */ STCALL(TDEC1),
    ADDRESS(CSMCONIC),
};

const struct interpretive_code CONCAUL_CODE = {036u, CONCAUL, COUNT(concaul_words), concaul_words};

/* ---- JUNCT3 -- pp. 555-556 ---- */

/* JUNCT3: the passive at the transfer's end to VPASS4 and RPASS4 and the
 * target RTARG, TF to DELLT4, and "SAVE BACK VALUES OF HP AND DELVEE"
 * (HPO, DELVEO). "PREPARE FOR LAMBERT": INTIME the TPI time, RTX1 the
 * scale indexes (XRS), the list 0D on HI6ZEROS and EPSFOUR, RINIT and
 * VINIT the active at TPI; then INITVEL, the velocities of the transfer.
 * "COMPUTE H ET CETERA": RELDELV the impulse at TPF, "/V2-VP(TPI+TF)/",
 * MAGVTPI the one at TPI ("V1-VA", "/V1-VA/", DELVEET3), and DELVEE
 * their sum; the transfer's pericenter (PERIAPO, from the active's TPI
 * position and the velocity it needs) scaled by X2 to HP. */
static const word junct3_words[] = {
    /* JUNCT3 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VATT),
    STOVL(VPASS4),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RPASS4),
    STODL(RTARG),
    ADDRESS_PLUS_ONE(TF),
    STODL(DELLT4),
    ADDRESS_PLUS_ONE(HP),
    STODL(HPO),
    ADDRESS_PLUS_ONE(DELVEE),
    STODL(DELVEO),
    ADDRESS_PLUS_ONE(TTPI),
    STODL(INTIME),
    ADDRESS_PLUS_ONE(XRS),
    STODL(RTX1),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(SETPD, PDDL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(EPSFOUR),
    ORDER(PDVL),
    ADDRESS_PLUS_ONE(RACT3),
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VACT3),
    STCALL(VINIT),
    ADDRESS(INITVEL),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VTPRIME),
    ADDRESS_PLUS_ONE(VPASS4),
    ORDERS(ABVAL, PUSH),
    STOVL(RELDELV),             /* "/V2-VP(TPI+TF)/" */
    ADDRESS_PLUS_ONE(DELVEET3), /* "V1-VA" */
    ORDER(ABVAL),               /* "/V1-VA/" */
    STORE(MAGVTPI),
    ORDERS(DAD, STADR),
    AFTER_STADR(STODL(DELVEE)),
    ADDRESS_PLUS_ONE(XRS),
    STOVL(X1),
    ADDRESS_PLUS_ONE(VIPRIME),
    ORDER(VSR_STAR),
    SR_X2(0),
    STOVL(VVEC),
    ADDRESS_PLUS_ONE(RACT3),
    ORDER(VSR_STAR),
    SR_X2(0),
    STCALL(RVEC),
    ADDRESS(PERIAPO),
    ORDERS(LXA_2, DLOAD),
    ADDRESS_AT(XRS, 1),
    ORDER(SL_STAR),
    SL_X2(0),
    STORE(HP),

    /* "ITSWICH DENOTES INTERPOLATION--SOLUTION ACCEPTANCE IS FORCED"
     * (ENDEN). HPERMIN, the least safe pericenter, less HP negative: this
     * one is safe (HALFSAFE). Else "WAS PERICENTER ALT SAFE": HPERMIN less
     * HPO negative, the last one was, and the safe altitude lies between
     * the two (INTERP, "SOLUTION AT HAND"); else HP less HPO,
     * "(HPLIM-HPO)-(HPLIM-HP)=HP-HPO", negative: "IT'S GETTING WORSE -
     * SOUND THE ALARM" (ALARUMS). Else CDSEC, the step, loaded for JUNCT1 (part 2),
     * the next word. */
    ORDERS(BON, DLOAD),
    SWITCH(BON, ITSWICH),
    ADDRESS(ENDEN),
    ADDRESS_PLUS_ONE(HPERMIN),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(HP),
    ADDRESS(HALFSAFE),
    ORDERS(PDDL, DSU), /* "WAS PERICENTER ALT SAFE" */
    ADDRESS_PLUS_ONE(HPERMIN),
    ADDRESS_PLUS_ONE(HPO),
    ORDERS(BMN, DSU), /* "(HPLIM-HPO)-(HPLIM-HP)=HP-HPO" */
    ADDRESS(INTERP),  /* "SOLUTION AT HAND" */
    ORDERS(BMN, DLOAD),
    ADDRESS(ALARUMS), /* "IT'S GETTING WORSE - SOUND THE ALARM" */
    ADDRESS_PLUS_ONE(CDSEC),
};

const struct interpretive_code JUNCT3_CODE = {036u, JUNCT3, COUNT(junct3_words), junct3_words};

#endif
