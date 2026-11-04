/* SPDX-License-Identifier: MIT */
/* GROUND_TRACKING_DETERMINATION_PROGRAM.agc (Luminary099 pp. 654-657):
 * P21, "GROUND TRACKING DETERMINATION PROGRAM". Its basic code and its
 * interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/ground_track_routines.c). The faithful C is
 * c64-port/src/lm_ground_track.c.
 *
 * P21, as its program description has it: "TO PROVIDE THE ASTRONAUT
 * DETAILS OF THE LM OR CSM GROUND TRACK WITHOUT THE NEED FOR GROUND
 * COMMUNICATION (REQUESTED BY DSKY)", by "V37E21E". It ends when the
 * astronaut terminates it ("V34E").
 *
 * Each BANKCALL (to GOPERF4 or GOFLASH) leaves Z at BANKCALL and Q at its
 * CADR; the display comes back to the word after the CADR on terminate,
 * the next on proceed, and the next when a value is loaded through the
 * DSKY (or, for the last display, on a recycle, verb 32). */
#include "ground_track.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- PROG21 and P21PROG1 -- LM p. 655 ---- */

/* PROG21 +2: "OPTION 2", the vehicle, in R2 of a please perform
 * (GOPERF4: verb 04 noun 06, then verb 50 noun 06 flashing). */
static void request_vehicle(void)
{
    set_A(BIT2);
    tc_to(PROG21 + 3u, BANKCALL); /* CADR GOPERF4 */
}

/* PROG21: "ASSUMED VEHICLE IS LM , R2 = 00001" (OPTION2). */
void prog21(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(OPTION2));
    request_vehicle();
}

/* PROG21 +5: "TERMINATE". */
void prog21_terminate(void)
{
    tc_to(PROG21 + 5u, GOTOPOOH);
}

/* PROG21 +6: "PROCEED VALUE OF ASSUMED VEHICLE OK" (TC +2, to
 * P21PROG1). */
void prog21_proceed(void)
{
    tc_at(PROG21 + 6u);
    p21prog1();
}

/* PROG21 +7: "R2 LOADED THROUGH DSKY" (TC -5): the option shown again. */
void prog21_loaded(void)
{
    tc_at(PROG21 + 7u);
    request_vehicle();
}

/* P21PROG1: "LOAD DESIRED TIME OF LAT-LONG.", verb 06 noun 34 (into
 * DSPTEM1), flashed by GOFLASH. */
void p21prog1(void)
{
    set_A(V6N34);
    tc_to(P21PROG1 + 1u, BANKCALL); /* CADR GOFLASH */
}

/* P21PROG1 +3: "TERM". */
void p21prog1_terminate(void)
{
    tc_to(P21PROG1 + 3u, GOTOPOOH);
}

/* P21PROG1 +4: "PROCEED VALUES OK" (TC +2): to the interpreter, for
 * P21_CODE. */
void p21prog1_proceed(void)
{
    tc_at(P21PROG1 + 4u);
    tc_at(P21PROG1 + 6u);
    intpret();
}

/* P21PROG1 +5: "TIME LOADED THROUGH DSKY" (TC -5): the time shown
 * again. */
void p21prog1_loaded(void)
{
    tc_at(P21PROG1 + 5u);
    p21prog1();
}

/* ---- The integration and the display data -- LM pp. 655-657 ---- */

/* P21PROG1 +7: "INTEGRATE TO TIME SPECIFIED IN TDEC" (TDEC1, from
 * DSPTEM1), after INTSTALL. P21FLAG is set after a first pass:
 * "ON---RECYCLE USING BASE VECTOR" (P21CONT). On the first pass ("OFF--IST
 * PASS CALL BASE VECTOR"), the vehicle from OPTION2, SR1: "ZERO--THIS
 * VEHICLE(LM)", "ONE--OTHER VEHICLE(CM)" (VINTFLAG); DIM0FLAG and
 * INTYPFLG clear ("PRECISION"), and INTEGRV integrates the vehicle's
 * state vector, "-AND -SAVE BASE VECTOR" (P21VSAVE). */
static const word p21_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(DSPTEM1),
    STCALL(TDEC1), /* "INTEGRATE TO TIME SPECIFIED IN TDEC" */
    ADDRESS(INTSTALL),
    ORDERS(BON, CLEAR),
    SWITCH(BON, P21FLAG),
    ADDRESS(P21CONT), /* "ON---RECYCLE USING BASE VECTOR" */
    SWITCH(CLEAR, VINTFLAG),
    ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(OPTION2),
    ORDERS(BHIZ, SET),
    ADDRESS_AT(PROG21, 034u), /* "+2": "ZERO--THIS VEHICLE(LM)" */
    SWITCH(SET, VINTFLAG),    /* "ONE--OTHER VEHICLE(CM)" */
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(CLEAR, INTYPFLG), /* "PRECISION" */
    ORDER(CALL),
    ADDRESS(INTEGRV), /* "CALCULATE" */
    ORDER(GOTO),
    ADDRESS(P21VSAVE),
    /* P21CONT: "RECYCLE--INTEG FROM BASE VECTOR": its position, velocity
     * and time to RCV, VCV and TET; DIM0FLAG clear, and MOONFLAG set
     * when P21ORIG is the Moon's ("ZERO=EARTH", "---2=MOON"); INTEGRVS
     * integrates from there. */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(P21BASER),
    STOVL(RCV), /* "--POS" */
    ADDRESS_PLUS_ONE(P21BASEV),
    STODL(VCV), /* "--VEL" */
    ADDRESS_PLUS_ONE(P21TIME),
    STORE(TET), /* "--TIME" */
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(CLEAR, MOONFLAG),
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(P21ORIG),
    ADDRESS_AT(P21CONT, 017u), /* "+3": "ZERO=EARTH" */
    ORDER(SET),
    SWITCH(SET, MOONFLAG),
    ORDER(CALL),
    ADDRESS(INTEGRVS),
    /* P21VSAVE: "SAVE CURRENT BASEVECTOR", the integrated state (TAT,
     * RATT1, VATT1) to P21TIME, P21BASER and P21BASEV; P21VEL, the speed
     * (rescaled by X2, SL* 0,2), "FOR N91 DISP"; P21GAM, the flight path
     * angle, "SIN-1 U(R).U(V)"; P21ORIG, the origin (X2: "0=EARTH").
     * P21FLAG set, for a recycle to integrate on from the base vector;
     * but not for the LM (OPTION2, SR1, zero) on the surface
     * (SURFFLAG). */
    ORDER(DLOAD), /* "SAVE CURRENT BASEVECTOR" */
    ADDRESS_PLUS_ONE(TAT),
    STOVL(P21TIME), /* "--TIME" */
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(P21BASER), /* "--POS B-29 OR B-27" */
    ADDRESS_PLUS_ONE(VATT1),
    STORE(P21BASEV), /* "--VEL B-07 OR B-05" */
    ORDERS(ABVAL, SL_STAR),
    VSL_X2(0),
    STOVL(P21VEL), /* "VEL/ FOR N91 DISP" */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(VATT), /* "U(R).V" */
    ORDERS(DDV, ASIN),      /* "U(R).U(V)" */
    ADDRESS_PLUS_ONE(P21VEL),
    STORE(P21GAM), /* "SIN-1 U(R).U(V) , -90 TO &90" */
    ORDERS(SXA_2, SLOAD),
    ADDRESS(P21ORIG), /* "0=EARTH" */
    ADDRESS_PLUS_ONE(OPTION2),
    ORDERS(SR1, BHIZ),
    ADDRESS_AT(P21VSAVE, 027u), /* "+3" */
    ORDER(GOTO),
    ADDRESS_AT(P21VSAVE, 032u), /* "+4" */
    ORDER(BON),
    SWITCH(BON, SURFFLAG),
    ADDRESS(P21DSP),
    ORDER(SET),
    SWITCH(SET, P21FLAG),
    /* P21DSP: "GENERATE DISPLAY DATA": LUNAFLAG from X2 ("0 = EARTH");
     * the position and time (ALPHAV, TAT) to LAT-LONG, with ERADFLAG
     * clear; P21ALT, the altitude over 100, "ALT/100 FOR N91 DISP". */
    ORDERS(CLEAR, SLOAD), /* "GENERATE DISPLAY DATA" */
    SWITCH(CLEAR, LUNAFLAG),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(BZE, SET),
    ADDRESS_AT(P21DSP, 6u), /* "+2": "0 = EARTH" */
    SWITCH(SET, LUNAFLAG),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(TAT),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, ERADFLAG),
    ADDRESS(LAT_LONG),
    ORDER(DMP), /* "MPAC = ALT, METERS B-29" */
    ADDRESS_PLUS_ONE(K_01),
    STORE(P21ALT), /* "ALT/100 FOR N91 DISP" */
    ORDER(EXIT),
};

/* P21DSP +21, after the EXIT: "DISPLAY LAT, LONG, ALT", verb 06 noun 43,
 * flashed by GOFLASH ("LAT, LONG = 1/2 REVS B0", "ALT = KM B14"). */
void p21dsp_after_exit(void)
{
    set_A(V06N43);
    tc_to(P21DSP + 022u, BANKCALL); /* CADR GOFLASH */
}

/* P21DSP +24: "TERM". */
void p21dsp_terminate(void)
{
    tc_to(P21DSP + 024u, GOTOPOOH);
}

/* P21DSP +25: proceed, which ends the program too. */
void p21dsp_proceed(void)
{
    tc_to(P21DSP + 025u, GOTOPOOH);
}

/* P21DSP +26: "V32E RECYCLE": to the interpreter, for
 * P21_RECYCLE_CODE. */
void p21dsp_recycle(void)
{
    tc_at(P21DSP + 026u);
    intpret();
}

/* P21DSP +27: the next time, P21TIME plus "600 SECONDS OR 10 MIN", to
 * DSPTEM1, and back to P21PROG1 (RTB) to show it. */
static const word p21_recycle_words[] = {
    ORDERS(DLOAD, DAD),       ADDRESS_PLUS_ONE(P21TIME),
    ADDRESS_PLUS_ONE(600SEC), /* "600 SECONDS OR 10 MIN" */
    STORE(DSPTEM1),           ORDER(RTB),
    ADDRESS(P21PROG1),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P21_CODE = {024u, P21PROG1 + 7u, COUNT(p21_words), p21_words};
const struct interpretive_code P21_RECYCLE_CODE = {024u, P21DSP + 027u, COUNT(p21_recycle_words),
                                                   p21_recycle_words};

#endif
