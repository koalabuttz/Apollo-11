/* SPDX-License-Identifier: MIT */
/* P30-P37.agc (Comanche055 pp. 639-642): S30.1 and S31.1, the
 * computations of P30 and P31 (bank 31), in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/cm_p30_routines.c).
 *
 * S30.1: "BASED ON STORED TARGET PARAMETERS(R OF IGNITION(RTIG),V OF
 * IGNITION(VTIG),TIME OF IGNITION(TIG),DELV(LV),COMPUTE PERIGEE ALTITUDE
 * A+OGEE ALTITUDE AND DELTA-V REQUIRED IN REF. COORDS.(DELVSIN)".
 *
 * S31.1 "COMPUTES DELV IN REF AND LV COORDS,MAG OF DELV,INTERCEPT TIME,
 * APOGEE AND PERIGEE ALT FOR REQUIRED MANEUVER".
 *
 * Both are CALLed, keep the return in QTEMP and go back through it: "AT L
 * +2 OF CALLING SEQUENCE(GOTO L+2)", as S31.1's description has it. */
#include "p30_cm.h"
#include "../agc/names.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S30.1 -- pp. 639-640 ---- */

/* The return to QTEMP; the CSM's state vector at ignition (THISPREC, the
 * "ENCKE ROUTINE", to TDEC1 = TIG) as VTIG and RTIG (and RACT3). LOMAT,
 * from RTIG and UNRM, the unit normal RTIG x VTIG, leaves the local
 * vertical frame's matrix at 0D: DELVSLV, the delta V in local vertical
 * coordinates, times it (VXM) is DELVSIN, the delta V in the reference
 * coordinates, and its length VGDISP ("MAG DELV"). The orbit after the
 * burn (PERIAPO1, from RTIG and VTIG + DELVSIN): HPER, the "PERIGEE ALT
 * B+29", and HAPO, the "APOGEE ALT B+29", each rescaled (SHIFTR1) and
 * limited (MAXCHK) for the display. Back through QTEMP. */
static const word s30_1_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(TIG), /* "TIME IGNITION SCALED AT 2(+28)CS" */
    STCALL(TDEC1),
    ADDRESS(THISPREC), /* "ENCKE ROUTINE FOR" */
    ORDERS(VLOAD, SXA_2),
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS(RTX2),
    STOVL(VTIG),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RTIG),
    STORE(RACT3),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VTIG),
    STCALL(UNRM),
    ADDRESS(LOMAT),
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(DELVSLV),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VSL1, SXA_1),
    ADDRESS(RTX1),
    STORE(DELVSIN),
    ORDER(ABVAL),
    STOVL(VGDISP), /* "MAG DELV" */
    ADDRESS_PLUS_ONE(RTIG),
    ORDERS(PDVL, VAD),
    ADDRESS_PLUS_ONE(DELVSIN),
    ADDRESS_PLUS_ONE(VTIG),
    ORDER(CALL),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL),
    ADDRESS(MAXCHK),
    STODL(HPER), /* "PERIGEE ALT B+29" */
    ADDRESS_PLUS_ONE_D(4),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL),
    ADDRESS(MAXCHK),
    STCALL(HAPO), /* "APOGEE ALT B+29" */
    ADDRESS(QTEMP),
};

/* ---- S31.1 -- pp. 641-642 ---- */

/* The return to QTEMP; the Lambert solution at TIG (AGAIN, which
 * "RETURNS RTX2,RTX1,RATT,VATT,VIPRIME"): the orbit from RTIG with
 * VIPRIME, the velocity after the burn (PERIAPO1), HPER and HAPO as in
 * S30.1. DELVEET3, the delta V, at 0D; with AVFLAG set (the CSM's), MIDGIM
 * puts it in local vertical coordinates ("GET DELVLVC B7 FORDISPLAY") and
 * its length is VGDISP. TPASS4, the time of the intercept, is TIG +
 * DELLT4, the transfer time ("FOR S40.1"). Back through QTEMP. */
static const word s31_1_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1),
    ADDRESS(AGAIN),      /* "RETURNS RTX2,RTX1,RATT,VATT,VIPRIME" */
    ORDERS(VLOAD, PDVL), /* "DELUEET3" */
    ADDRESS_PLUS_ONE(RTIG),
    ADDRESS_PLUS_ONE(VIPRIME),
    ORDER(CALL),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL),
    ADDRESS(MAXCHK),
    STODL(HPER),
    ADDRESS_PLUS_ONE_D(4),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL),
    ADDRESS(MAXCHK),
    STOVL(HAPO),
    ADDRESS_PLUS_ONE(DELVEET3),
    STORE_D(0),
    ORDERS(SET, CALL),
    SWITCH(SET, AVFLAG),
    ADDRESS(MIDGIM), /* "GET DELVLVC B7 FORDISPLAY" */
    ORDER(ABVAL),
    STODL(VGDISP), /* "B+7 FOR DISPLAY" */
    ADDRESS_PLUS_ONE(DELLT4),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TPASS4), /* "FOR S40.1" */
    ADDRESS(QTEMP),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S30_1_CODE = {031u, S30_1, COUNT(s30_1_words), s30_1_words};
const struct interpretive_code S31_1_CODE = {031u, S31_1, COUNT(s31_1_words), s31_1_words};

#endif
