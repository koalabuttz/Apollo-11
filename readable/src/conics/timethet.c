/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, TIMETHET (LM p. 1186, CM p. 1289): TIME-THETA,
 * in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs.
 *
 * "GIVEN AN INITIAL STATE VECTOR AND A DESIRED TRUE-ANOMALY-DIFFERENCE
 * THROUGH WHICH THE STATE IS TO BE UPDATED ALONG A CONIC TRAJECTORY,
 * CALCULATES THE CORRESPONDING TIME-OF-FLIGHT" (LM p. 1166, CM p. 1269),
 * and the new state as well if asked. In: RVEC and VVEC, the state; SNTH
 * and CSTH, the sine and cosine of the angle; RVSW, set if only the time
 * is wanted; X1, the central body (-2D for the earth, -10D for the moon).
 * Out: T, the time; with RVSW clear, the new velocity in MPAC and the new
 * position at 0D of the push-down list. With no solution (COGAFLAG or
 * INFINFLG set) it goes to ABTCONIC, and does not return.
 *
 * PARAM gives P and COGA from the state, GETX the X for the angle, and
 * DELTIME the time for the X; NEWSTATE, the state there. */
#include "conics.h"
#include "../agc/names.h"

static const word timethet_words[] = {
    /* TIMETHET -- LM p. 1186 (CM p. 1289): the return to RTNTT, SETPD 0
     * and BOV +1 (which clears the overflow indicator); RVEC pushed, VVEC
     * in MPAC ("SETUP FOR PARAM CALL"), and PARAM. An overflow there is
     * COGAOVFL; then GETX. */
    /* TIMETHET */ ORDERS(STQ, SETPD), /* "PL AT 0" */
    ADDRESS(RTNTT),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(BOV),
    ADDRESS_AT(TIMETHET, 5), /* (+1) */
    ORDERS(VLOAD, PDVL),     /* "PL AT 6" */
    ADDRESS_PLUS_ONE(RVEC),
    ADDRESS_PLUS_ONE(VVEC),
    ORDER(CALL),
    ADDRESS(PARAM),
    ORDERS(BOV, CALL), /* "PL AT 0" */
    ADDRESS(COGAOVFL),
    ADDRESS(GETX),
    /* COMMNOUT: XI in MPAC for DELTIME (X squared is at 0D). With
     * INFINFLG set, GETX found no solution: ABTCONIC. Otherwise COGAFLAG
     * cleared and DELTIME, the time. With RVSW set that is all, and the
     * GOTO through RTNTT returns; otherwise NEWSTATE first. */
    /* COMMNOUT */ ORDERS(DLOAD, BON),
    ADDRESS_PLUS_ONE(XI),
    SWITCH(BON, INFINFLG),
    ADDRESS(ABTCONIC),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, COGAFLAG),
    ADDRESS(DELTIME),
    ORDERS(BON, CALL),
    SWITCH(BON, RVSW),
    ADDRESS(RTNTT),
    ADDRESS(NEWSTATE),
    ORDER(GOTO),
    ADDRESS(RTNTT),
    /* COGAOVFL: COGA overflowed, the angle between position and velocity
     * too near 0 or 180 degrees: COGAFLAG set, and to ABTCONIC. */
    /* COGAOVFL */ ORDER(SETGO),
    SWITCH(SETGO, COGAFLAG),
    ADDRESS(ABTCONIC),
};

const struct interpretive_code TIMETHET_CODE = {I_TIMETHET >> 10, K_TIMETHET, COUNT(timethet_words),
                                                timethet_words};
