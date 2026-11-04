/* SPDX-License-Identifier: MIT */
/* GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc (Luminary099 pp. 651-653): P31,
 * "GENERAL LAMBERT AIMPOINT GUIDANCE", and REVN1645. Its basic code and
 * its interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/lambert_aimpoint_routines.c). The faithful C is
 * c64-port/src/lm_lambert_aim.c.
 *
 * P31, as its program description has it: "TO ACCEPT TARGETING
 * PARAMETERS OBTAINED FROM A SOURCE EXTERNAL TO THE LEM AND COMPUTE THERE
 * FROM THE REQUIRED-VELOCITY AND OTHER INITIAL CONDITIONS REQUIRED BY LM
 * FOR DESIRED MANEUVER." The targeting parameters are TIG, RTARG (the
 * target's position) and DELLT4 (the time from TIG to the target). It is
 * "SELECTED BY THE ASTRONAUT BY DSKY ENTRY V37 E31". The flashing
 * displays are VNPOOH's (P34-35_P74-75.agc), which returns on proceed.
 *
 * Each call out of the file (P20FLGON, VNPOOH) leaves Z at the routine
 * and Q at the word after the TC, where the routine comes back to. */
#include "lambert_aimpoint.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P31 -- LM pp. 652-653 ---- */

/* P31: P20's update and track flags set (P20FLGON). */
void p31(void)
{
    tc_at(P31);
    continue_at(P20FLGON);
}

/* P31 +1: "TIG", verb 06 noun 33, through VNPOOH. */
void p31_after_p20flgon(void)
{
    set_A(V06N33);
    tc_at(P31 + 2u);
    continue_at(VNPOOH);
}

/* P31 +3: to the interpreter, for P31_CODE. */
void p31_after_v06n33(void)
{
    tc_at(P31 + 3u);
    intpret();
}

/* P31 +4: the update flag cleared; the LM's state vector to TIG
 * ("INTEGRATE STATE VECTORS TO TIG", LEMPREC): RTIG and RINIT, VTIG and
 * VINIT. P30ZERO's zeros on the push-down list ("E4 AND NUMIT = 0") and
 * DELLT4: TPASS4, TIG plus DELLT4, the time the target is reached. With
 * X1 and X2 kept in RTX1 and RTX2 (the Earth's or the Moon's), INITVEL
 * finds VIPRIME, the velocity at RINIT that reaches RTARG in DELLT4 (by
 * LAMBERT), and DELVEET3, the delta V to it: DELVSIN. XDELVFLG cleared
 * (the burn's velocity to be gained from Lambert, not an external delta
 * V); VGDISP, the delta V's length, and GET.LVC, the delta V in local
 * vertical coordinates (DELVLVC), for the displays. The perigee and
 * apogee (PERIAPO1) of the orbit after the burn, from RTIG and VIPRIME:
 * HPER and HAPO, each rescaled if needed (SHIFTR1) and at most 9999.9
 * nautical miles (MAXCHK). */
static const word p31_words[] = {
    ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, UPDATFLG),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1), /* "INTEGRATE STATE VECTORS TO TIG" */
    ADDRESS(LEMPREC),
    ORDERS(VLOAD, SETPD),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS_PLUS_ONE_D(0),
    STORE(RTIG),
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VATT),
    STORE(VTIG),
    STODL(VINIT),
    ADDRESS_PLUS_ONE(P30ZERO),
    ORDERS(PUSH, PDDL), /* "E4 AND NUMIT = 0" */
    ADDRESS_PLUS_ONE(DELLT4),
    ORDERS(DAD, SXA_1),
    ADDRESS_PLUS_ONE(TIG),
    ADDRESS(RTX1),
    STORE(TPASS4),
    ORDERS(SXA_2, CALL),
    ADDRESS(RTX2),
    ADDRESS(INITVEL),
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(DELVEET3),
    STORE(DELVSIN),
    ORDERS(ABVAL, CLEAR),
    SWITCH(CLEAR, XDELVFLG),
    STCALL(VGDISP),
    ADDRESS(GET_LVC),
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(RTIG),
    ADDRESS_PLUS_ONE(VIPRIME),
    ORDER(CALL),
    ADDRESS(PERIAPO1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL), /* "LIMIT DISPLAY TO 9999.9 N. MI." */
    ADDRESS(MAXCHK),
    STODL(HPER),
    ADDRESS_PLUS_ONE_D(4),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDER(CALL), /* "LIMIT DISPLAY TO 9999.9 N. MI." */
    ADDRESS(MAXCHK),
    STORE(HAPO),
    ORDER(EXIT),
};

/* P31 +63, after the EXIT: "DELVLVC", verb 06 noun 81. */
void p31_after_exit(void)
{
    set_A(V06N81);
    tc_at(P31 + 064u);
    continue_at(VNPOOH);
}

/* P31 +65: "HAPO, HPER, VGDISP", verb 06 noun 42. */
void p31_after_v06n81(void)
{
    set_A(V06N42);
    tc_at(P31 + 066u);
    continue_at(VNPOOH);
}

/* P31 +67: to the interpreter, for REVN1645_CODE. */
void p31_after_v06n42(void)
{
    tc_at(P31 + 067u);
    intpret();
}

/* REVN1645: "TRKMKCNT, TTOGO, +MGA". FINALFLG set, and VN1645 (in
 * P34-35_P74-75.agc) flashes verb 16 noun 45, the marks count, the time
 * from ignition and the middle gimbal angle. With FINALFLG set, proceed and terminate both
 * end in GOTOPOOH, to wait for the next program; a recycle comes back,
 * and REVN1645 starts again. P30 comes here too (its PARAM30's SETGO). */
static const word revn1645_words[] = {
    ORDERS(SET, CALL), /* "TRKMKCNT, TTOGO, +MGA" */
    SWITCH(SET, FINALFLG), ADDRESS(VN1645), ORDER(GOTO), ADDRESS(REVN1645),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P31_CODE = {035u, P31 + 4u, COUNT(p31_words), p31_words};
const struct interpretive_code REVN1645_CODE = {035u, I_REVN1645 % 02000u + 02000u,
                                                COUNT(revn1645_words), revn1645_words};

#endif
