/* SPDX-License-Identifier: MIT */
/* GIMBAL_LOCK_AVOIDANCE.agc (Luminary099 p. 364, Comanche055 pp. 412-413):
 * where KALCMANU goes once it has the maneuver's axis (COF) and angle (AM),
 * LOCSKIRT. Despite the file's name, neither mission steers round gimbal
 * lock here: the LM's LOCSKIRT is NOGIMLOC, which sets CALCMAN3 and goes on
 * to WCALC, and the CM's LOCSKIRT is WCALC itself. WCALC works out the
 * maneuver's rates and time and starts the steering (KALCMANU_STEERING).
 * Interpretive code in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs (src/binding/gimbal_lock_routines.c).
 * The faithful C is c64-port/src/lm_gimbal_lock.c and cm_gimbal_lock.c.
 *
 * Both missions' code is in bank 22. The rates (ARATE), ANGLTIME and the
 * CM's QUADROT and BIASCALE are constants the interpreter reads from the
 * rope. ARATE,1 is ARATE indexed by X1, written as any address (an X1
 * address is positive). */
#include "gimbal_lock.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* NOGIMLOC, WCALC -- LM p. 364. "DETECTING GIMBAL LOCK"
 * NOGIMLOC (LOCSKIRT) sets CALCMAN3 (on: "FINAL ROLL IS NECESSARY").
 *
 * WCALC: "CHOOSE THE DESIRED MANEUVER RATE FROM A LIST OF FOUR": RATEINDX
 * (0, 2, 4 or 6, LXC,1 taking its complement into X1) picks one of ARATE's
 * four, 0.2, 0.5, 2 or 10 degrees a second. "COMPUTE THE INCREMENTAL
 * ROTATION MATRIX DEL CORRESPONDING TO A 1 SEC ROTATION ABOUT COF": the rate
 * shifted down 4 is DELCOMP's angle. Then the "COMPONENT MANEUVER RATES",
 * BRATE, the rate along COF, and the "MANEUVER EXECUTION TIME", TM, AM over
 * the rate (with ANGLTIME's "FUDGE FACTOR"), "SCALED AS T2". SETGO sets
 * CALCMAN2 ("START MANEUVER") and goes on to the steering at NEWANGL +1. */
static const word nogimloc_words[] = {
    /* NOGIMLOC */ ORDER(SET),
    SWITCH(SET, CALCMAN3),
    /* WCALC */ ORDERS(LXC_1, DLOAD_STAR),
    ADDRESS(RATEINDX),       /* "CHOOSE THE DESIRED MANEUVER RATE" */
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1: "FROM A LIST OF FOUR" */
    ORDERS(SR4, CALL),       /* "COMPUTE THE INCREMENTAL ROTATION MATRIX" */
    ADDRESS(DELCOMP),        /* "DEL CORRESPONDING TO A 1 SEC ROTATION" */
    ORDERS(DLOAD_STAR, VXSC),
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1 */
    ADDRESS_PLUS_ONE(COF),
    STODL(BRATE), /* "COMPONENT MANEUVER RATES 45 DEG/SEC" */
    ADDRESS_PLUS_ONE(AM),
    ORDERS(DMP, DDV_STAR),
    ADDRESS_PLUS_ONE(ANGLTIME),
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1 */
    ORDER(SR),
    SR_COUNT(5),
    STORE(TM), /* "MANEUVER EXECUTION TIME SCALED AS T2" */
    ORDER(SETGO),
    SWITCH(SETGO, CALCMAN2), /* "0(OFF) = CONTINUE MANEUVER" */
    ADDRESS_AT(NEWANGL, 1),  /* "1(ON) = START MANEUVER" */
};

const struct interpretive_code NOGIMLOC_CODE = {022u, 02744u, COUNT(nogimloc_words),
                                                nogimloc_words};

#else

/* WCALC -- CM pp. 412-413.
 * LOCSKIRT. As the LM's (above), the rate RATEINDX picks from ARATE's four
 * (0.05, 0.2, 0.5 and 2 degrees a second), DELCOMP's matrix for a second of
 * it, the time TM; but the rates BRATE are turned by QUADROT, "ROTATION
 * MATRIX FROM S/C AXES TO CONTROL AXES (X ROT = -7.25 DEG)", and
 * scaled by BIASCALE into BIASTEMP, an "ATTITUDE ERROR BIAS TO PREVENT
 * OVERSHOOT IN SYSTEM". */
static const word wcalc_words[] = {
    /* WCALC */ ORDERS(LXC_1, DLOAD_STAR),
    ADDRESS(RATEINDX),
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1 */
    ORDERS(SR4, CALL),       /* "COMPUTE THE INCREMENTAL ROTATION MATRIX" */
    ADDRESS(DELCOMP),        /* "DEL CORRESPONDING TO A 1 SEC ROTATION" */
    ORDERS(DLOAD_STAR, VXSC),
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1 */
    ADDRESS_PLUS_ONE(COF),
    ORDER(MXV),
    ADDRESS_PLUS_ONE(QUADROT),
    STODL(BRATE),
    ADDRESS_PLUS_ONE(AM),
    ORDERS(DMP, DDV_STAR),
    ADDRESS_PLUS_ONE(ANGLTIME),
    ADDRESS_PLUS_ONE(ARATE), /* ARATE,1 */
    ORDER(SR),
    SR_COUNT(5),
    STOVL(TM),
    ADDRESS_PLUS_ONE(BRATE),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(BIASCALE),
    STORE(BIASTEMP),         /* "ATTITUDE ERROR BIAS TO PREVENT OVERSHOOT" */
    ORDER(SETGO),            /* "STATE SWITCH CALCMAN2 (43D)" */
    SWITCH(SETGO, CALCMAN2), /* "0(OFF) = BYPASS STARTING PROCEDURE" */
    ADDRESS_AT(NEWANGL, 1),  /* "1(ON) = START MANEUVER" */
};

const struct interpretive_code WCALC_CODE = {022u, 02737u, COUNT(wcalc_words), wcalc_words};

#endif
