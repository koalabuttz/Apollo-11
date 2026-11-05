/* SPDX-License-Identifier: MIT */
/* P12.agc, pp. 841-842 (Luminary099 only, bank 23): GUIDINIT, which
 * "INITIALZE WM AND /LAND/" for the ascent guidance. The faithful C is
 * c64-port/src/lm_p12.c. */
#include "p12.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* GUIDINIT: the return in TEMPR60 (STQ), the push-down list from 0D;
 * UNITZ and the time now (LOADTIME) pushed, and RP-TO-R, which turns the
 * moon's pole into the reference frame at that time:
 *
 *     WM = REFSMMAT (RP-TO-R(UNITZ)) MOONRATE    (the moon's rotation)
 *     /LAND/ = /RLS/                             (the landing site's radius)
 *
 * and back through TEMPR60 (STCALL). */
static const word guidinit_words[] = {
    /* GUIDINIT */ ORDERS(STQ, SETPD),
    ADDRESS(TEMPR60),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(UNITZ),
    ORDERS(RTB, PUSH),
    ADDRESS(LOADTIME),
    ORDER(CALL),
    ADDRESS(RP_TO_R),
    ORDERS(MXV, VXSC),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ADDRESS_PLUS_ONE(MOONRATE),
    STOVL(WM),
    ADDRESS_PLUS_ONE(RLS),
    ORDERS(ABVAL, SL3),
    STCALL(_LAND_),
    ADDRESS(TEMPR60),
};

const struct interpretive_code GUIDINIT_CODE = {023u, GUIDINIT, COUNT(guidinit_words),
                                                guidinit_words};

#endif
