/* SPDX-License-Identifier: MIT */
/* P30_P37.agc (Luminary099 pp. 614-617): P30, the external delta-V
 * targeting program, and S30.1. Its basic code and its interpretive code
 * in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs (src/binding/p30_routines.c). The faithful C
 * is c64-port/src/lm_p30_file.c.
 *
 * P30, as its program description has it: "ACCEPT ASTRONAUT INPUTS OF
 * TIG.DELV(LV)" and "DISPLAY TIME TO GO, APOGEE, PERIGEE, DELV(MAG), MGA
 * AT IGN", then "REQUEST BURN PROGRAM". It starts "VIA JOB FROM V37". The
 * flashing displays are VNPOOH's (P34-35_P74-75.agc): "RETURNS ON
 * PROCEED, POOH ON TERMINATE", and on recycle it shows the display again.
 *
 * S30.1: "BASED ON STORED TARGET PARAMETERS(R OF IGNITION(RTIG),V OF
 * IGNITION(VTIG),TIME OF IGNITION (TIG)),COMPUTE PERIGEE ALTITUDE APOGEE
 * ALTITUDE AND DELTAV REQUIRED(DELVSIN)."
 *
 * Each call out of the file (UPFLAG, VNPOOH, DOWNFLAG) leaves Z at the
 * routine and Q at the word after the TC (or after its ADRES), where the
 * routine comes back to. */
#include "p30.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P30 -- LM pp. 614-615 ---- */

/* P30: "SET UPDATE FLAG" (UPFLAG, ADRES UPDATFLG). */
void p30(void)
{
    tc_at(P30);
    continue_at(UPFLAG); /* ADRES UPDATFLG */
}

/* P30 +2: "SET TRACK FLAG" (UPFLAG, ADRES TRACKFLG). */
void p30_after_updatflg(void)
{
    tc_at(P30 + 2u);
    continue_at(UPFLAG); /* ADRES TRACKFLG */
}

/* P30N33: "T OF IGN", verb 06 noun 33. VNPOOH "RETURNS ON PROCEED, POOH ON
 * TERMINATE". */
void p30n33(void)
{
    set_A(V06N33);
    tc_at(P30N33 + 1u);
    continue_at(VNPOOH);
}

/* P30N33 +2: "DISPLAY DELTA V (LV)", verb 06 noun 81 ("REDISPLAY ON
 * RECYCLE"). */
void p30_after_v06n33(void)
{
    set_A(V06N81);
    tc_at(P30N33 + 3u);
    continue_at(VNPOOH);
}

/* P30N33 +4: "RESET UPDATE FLAG" (DOWNFLAG, ADRES UPDATFLG). */
void p30_after_v06n81(void)
{
    tc_at(P30N33 + 4u);
    continue_at(DOWNFLAG); /* ADRES UPDATFLG */
}

/* P30N33 +6: to the interpreter, for P30_CODE. */
void p30_after_downflag(void)
{
    tc_at(P30N33 + 6u);
    intpret();
}

/* P30N33 +7: S30.1, and the update flag set again. */
static const word p30_words[] = {
    ORDER(CALL),
    ADDRESS(S30_1),
    ORDERS(SET, EXIT),
    SWITCH(SET, UPDATFLG),
};

/* PARAM30, after the EXIT: "DISPLAY APOGEE,PERIGEE ,DELTA V", verb 06 noun
 * 42. */
void param30(void)
{
    set_A(V06N42);
    tc_at(PARAM30 + 1u);
    continue_at(VNPOOH);
}

/* PARAM30 +2: to the interpreter, for PARAM30_CODE. */
void param30_after_v06n42(void)
{
    tc_at(PARAM30 + 2u);
    intpret();
}

/* PARAM30 +3: XDELVFLG "FOR P40'S: EXTERNAL DELTA-V GUIDANCE.", and on to
 * REVN1645, the "TRKMKCNT, T60, +MGA DISPLAY". */
static const word param30_words[] = {
    ORDER(SETGO),
    SWITCH(SETGO, XDELVFLG),
    ADDRESS(REVN1645),
};

/* ---- S30.1 -- LM pp. 616-617 ---- */

/* The return to QTEMP; the state vector at ignition ("ENCKE ROUTINE FOR
 * LEM", LEMPREC, to TIG): RTIG and VTIG. The local vertical frame's unit
 * vectors at ignition, from RTIG and VTIG, go on the push-down list as a
 * matrix at 0D, and DELVSLV, the delta V in local vertical coordinates,
 * times it (VXM) is DELVSIN, the delta V in inertial coordinates. Its
 * length, and the perigee and apogee (PERIAPO1) of the orbit after it:
 * HPER and HAPO, each rescaled if needed (SHIFTR1) and at most 9999.9
 * nautical miles (MAXCHK). Back by QTEMP. */
static const word s30_1_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(TIG), /* "TIME IGNITION SCALED AT 2(+28)CS" */
    STCALL(TDEC1),
    ADDRESS(LEMPREC), /* "ENCKE ROUTINE FOR LEM" */
    ORDERS(VLOAD, SXA_2),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS(RTX2),
    STORE(RTIG), /* "RADIUS VECTOR AT IGNITION TIME" */
    ORDERS(UNIT, VCOMP),
    STOVL(DELVSIN),         /* "ZRF/LV IN DELVSIN SCALED AT 2" */
    ADDRESS_PLUS_ONE(VATT), /* "VELOCITY VECTOR AT TIG, SCALED 2(7) M/CS" */
    STORE(VTIG),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    ORDERS(SETPD, SXA_1),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(RTX1),
    ORDERS(PUSH, VXV), /* "YRF/LV PDL 0 SCALED AT 2" */
    ADDRESS_PLUS_ONE(DELVSIN),
    ORDERS(VSL1, PDVL),        /* (the next word is an order pair: PUSHUP) */
    ORDERS(PDVL, PDVL),        /* "YRF/LV PDL 6 SCALED AT 2" */
    ADDRESS_PLUS_ONE(DELVSIN), /* "ZRF/LV PDL 12D SCALED AT 2" */
    ADDRESS_PLUS_ONE(DELVSLV),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE_D(0),
    STORE(DELVSIN), /* "DELTAV IN INERT. COOR. SCALED TO B+7M/CS" */
    ORDER(ABVAL),
    STOVL(DELVSAB),         /* "DELTA V MAG." */
    ADDRESS_PLUS_ONE(RTIG), /* "(FOR PERIAPO)" */
    ORDERS(PDVL, VAD),      /* "VREQUIRED = VTIG + DELVSIN (FOR PERIAPO)" */
    ADDRESS_PLUS_ONE(VTIG),
    ADDRESS_PLUS_ONE(DELVSIN),
    ORDER(CALL),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1), /* "RESCALE IF NEEDED" */
    ORDER(CALL),      /* "LIMIT DISPLAY TO 9999.9 N. MI." */
    ADDRESS(MAXCHK),
    STODL(HPER), /* "PERIGEE ALT 2(29) METERS, FOR DISPLAY" */
    ADDRESS_PLUS_ONE_D(4),
    ORDER(CALL),
    ADDRESS(SHIFTR1), /* "RESCALE IF NEEDED" */
    ORDER(CALL),      /* "LIMIT DISPLAY TO 9999.9 N. MI." */
    ADDRESS(MAXCHK),
    STCALL(HAPO), /* "APOGEE ALT 2(29) METERS, FOR DISPLAY" */
    ADDRESS(QTEMP),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P30_CODE = {035u, P30N33 + 7u, COUNT(p30_words), p30_words};
const struct interpretive_code PARAM30_CODE = {035u, PARAM30 + 3u, COUNT(param30_words),
                                               param30_words};
const struct interpretive_code S30_1_CODE = {034u, S30_1, COUNT(s30_1_words), s30_1_words};

#endif
