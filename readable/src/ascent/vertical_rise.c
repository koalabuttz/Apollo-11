/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc, pp. 851-852 (Luminary099 only, banks 27, 7 and
 * 14): the vertical rise. P12 lifts off straight up (FLVR set); CHECKALT
 * (guidance.c) sends each cycle here while the LM is under 25,000 feet,
 * and ANG1CHEK while it turns to the guidance's attitude (ROTFLAG). The
 * faithful C is c64-port/src/lm_ascent.c. */
#include "ascent.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ANG1CHEK (bank 27): with the thrust direction UNFC/2 within THETA1 of
 * the LM's X axis (XNBPIP; the cosine at least COSTHET1), the turn is
 * done: OFFROT. Else with the X axis more than THETA2 off the vertical
 * (XNBPIP . UNIT/R/ under COSTHET2), thrust straight up for now
 * (KEEPVR1). OFFROT: ROTFLAG clear, and CLRXFLAG. */
static const word ang1chek_words[] = {
    /* ANG1CHEK */ ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(UNFC_2),
    ADDRESS_PLUS_ONE(XNBPIP),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(COSTHET1),
    ADDRESS(OFFROT),
    ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(XNBPIP),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(COSTHET2),
    ADDRESS(KEEPVR1),
    /* OFFROT */ ORDER(CLRGO),
    SWITCH(CLRGO, ROTFLAG),
    ADDRESS(CLRXFLAG),
};

const struct interpretive_code ANG1CHEK_CODE = {027u, ANG1CHEK, COUNT(ang1chek_words),
                                                ang1chek_words};

/* CHECKYAW (bank 7; SETXFLAG is its other name): XOVINFLG set, "PROHIBIT
 * X-AXIS OVERRIDE". The horizontal thrust's direction, UNIT(ATY LAXIS +
 * ATP ZAXIS1), pushed for KEEPVR. With the LM's Y axis (YNBPIP) more than
 * 5 degrees out of the plane normal to it (|its dot product| at least
 * SIN5DEG), KEEPVR; else once RDOT reaches 40 feet a second (40FPS), the
 * vertical rise ends (EXITVR1); before, KEEPVR. */
static const word checkyaw_words[] = {
    /* CHECKYAW */ ORDER(SET),
    SWITCH(SET, XOVINFLG), /* "PROHIBIT X-AXIS OVERRIDE" */
    ORDERS(DLOAD, VXSC),
    ADDRESS_PLUS_ONE(ATY),
    ADDRESS_PLUS_ONE(LAXIS),
    ORDERS(PDDL, VXSC),
    ADDRESS_PLUS_ONE(ATP),
    ADDRESS_PLUS_ONE(ZAXIS1),
    ORDERS(VAD, UNIT),
    ORDERS(PUSH, DOT), /* the horizontal thrust's direction, pushed */
    ADDRESS_PLUS_ONE(YNBPIP),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(SIN5DEG),
    ORDERS(BPL, DLOAD),
    ADDRESS(KEEPVR), /* the yaw more than 5 degrees off */
    ADDRESS_PLUS_ONE(RDOT),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(40FPS),
    ADDRESS(EXITVR1), /* rising at 40 feet a second */
    ORDER(GOTO),
    ADDRESS(KEEPVR),
};

const struct interpretive_code CHECKYAW_CODE = {07u, CHECKYAW, COUNT(checkyaw_words),
                                                checkyaw_words};

/* KEEPVR (bank 14): "RECALL LOSVEC FROM PUSHLIST" (the direction CHECKYAW
 * pushed) to UNWC/2; KEEPVR1: the thrust straight up (UNFC/2 = UNIT/R/),
 * and ASCTERM. */
static const word keepvr_words[] = {
    /* KEEPVR */ ORDERS(VLOAD, STADR), /* "RECALL LOSVEC FROM PUSHLIST" */
    AFTER_STADR(STORE(UNWC_2)),
    /* KEEPVR1 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNIT_R_),
    STCALL(UNFC_2),
    ADDRESS(ASCTERM),
};

const struct interpretive_code KEEPVR_CODE = {014u, KEEPVR, COUNT(keepvr_words), keepvr_words};

#endif
