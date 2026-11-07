/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 2 (Comanche055 pp. 900-904, bank 32): the
 * "ALARM DISPLAY SUBROUTINE" (P370ALRM), the "TIME RADIUS CALLING
 * SUBROUTINE" (TMRAD100) and the "DISPLAY CALCULATION SUBROUTINE"
 * (RTEDISP), which turns the trajectory home into what the crew sees: the
 * speed, time and flight path angle at entry, the change in velocity in
 * local vertical coordinates, and the landing site. */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P370ALRM -- p. 900 ---- */

/* P370ALRM, called by STQ with the alarm's code in MPAC: its return in
 * SPRTEX, then out of the interpreter. */
static const word p370alrm_words[] = {
    /* P370ALRM */ ORDERS(STQ, EXIT),
    ADDRESS(SPRTEX),
};

const struct interpretive_code P370ALRM_CODE = {032u, P370ALRM, COUNT(p370alrm_words),
                                                p370alrm_words};

/* P370ALRM +2, after the EXIT: the alarm (VARALARM, the code from MPAC in
 * A), */
void p370alrm_after_exit(void)
{
    ca(ADDRESS_OF(MPAC));
    tc_to(P370ALRM + 3u, VARALARM);
}

/* then verb 05 noun 09 flashed (V5N09RTE, GOFLASH by BANKCALL). */
void p370alrm_after_varalarm(void)
{
    set_A(V5N09RTE);
    tc_to(P370ALRM + 5u, BANKCALL); /* CADR GOFLASH */
}

/* P370ALRM +7: terminate (TCF GOTOPOOH). */
void p370alrm_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* P370ALRM +011: recycle, back into the interpreter; proceed (TCF -4, at
 * +010) shows the alarm again. */
void p370alrm_recycle(void)
{
    tc_at(P370ALRM + 011u);
    intpret();
}

/* where the program goes back to the caller (GOTO SPRTEX). */
static const word p370alrm_goto_words[] = {
    /* P370ALRM +012 */ ORDER(GOTO),
    ADDRESS(SPRTEX),
};

const struct interpretive_code P370ALRM_GOTO_CODE = {
    032u, P370ALRM + 012u, COUNT(p370alrm_goto_words), p370alrm_goto_words};

/* ---- TMRAD100 and RTEDISP -- pp. 901-904 ---- */

/* TMRAD100, called by STQ: its return in RTENCKEX. TIMERAD, the conic
 * from RVEC and VVEC to the radius RDESIRED: RVSW clear, for the new state
 * as well as the time; SGNRDOT -.5 (AXC,2 OCT 20000, then SXA,2), for the
 * radius reached on the way in ("RADIAL VELOCITY AT RDESIRED IS TO BE"
 * "NEGATIVE"); X1 from CONICX1 (LXC,1), the central body. Its outputs
 * stored as the "FINAL VELOCITY VECTOR" V(T2)/, the "FINAL POSITION
 * VECTOR" R(T2)/ and the "TRANSFER TIME TO FINAL RADIUS" T12, and back
 * through RTENCKEX.
 *
 * RTEDISP, called by STQ: its return in SPRTEX. The output "CONVERTED TO
 * PROPER UNITS": VPRED the speed at entry (|V(T2)/|, which UNIT leaves at
 * 36D); T3TOT4 the time from ignition to entry (T2 less SPRTETIG); GAMMAEI
 * the "FLIGHT PATH ANGLE T2", a quarter turn (1RTEB2) less the angle
 * between R(T2)/ and V(T2)/ (ARCCOS). DELVLVC the change in velocity at
 * ignition (V2(T1)/ less V(T1)/) in local vertical coordinates: its part
 * along UH/, 0, and minus its part along UR1/; doubled (VSL1), and its
 * first component negated if the orbit is retrograde (RETROFLG set);
 * VGDISP its size. */
static const word tmrad100_words[] = {
    /* TMRAD100 */ ORDERS(STQ, CLEAR),
    ADDRESS(RTENCKEX),
    SWITCH(CLEAR, RVSW),
    ORDERS(AXC_2, SXA_2),
    NUMBER(020000),
    ADDRESS(SGNRDOT),
    ORDERS(LXC_1, CALL),
    ADDRESS(CONICX1),
    ADDRESS(TIMERAD),
    STOVL(V_T2_),
    ORDER(STADR),
    AFTER_STADR(STODL(R_T2_)),
    ADDRESS_PLUS_ONE(T),
    STCALL(T12),
    ADDRESS(RTENCKEX),
    /* RTEDISP */ ORDERS(STQ, VLOAD), /* "DISPLAY" */
    ADDRESS(SPRTEX),
    ADDRESS_PLUS_ONE(V_T2_),
    ORDERS(UNIT, PDDL),
    ADDRESS_PLUS_ONE_D(36),
    STODL(VPRED), /* "V(T2)" */
    ADDRESS_PLUS_ONE(T2),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(SPRTETIG),
    STOVL(T3TOT4), /* "T21" */
    ADDRESS_PLUS_ONE(R_T2_),
    ORDERS(UNIT, DOT),
    ORDER(SL1),
    ORDERS(ARCCOS, BDSU),
    ADDRESS_PLUS_ONE(1RTEB2),
    STOVL(GAMMAEI), /* "FLIGHT PATH ANGLE T2" */
    ADDRESS_PLUS_ONE(V2_T1_),
    ORDERS(VSU, PUSH),
    ADDRESS_PLUS_ONE(V_T1_),
    ORDERS(DOT, DCOMP),
    ADDRESS_PLUS_ONE(UR1_),
    ORDERS(PDVL, PUSH),
    ORDERS(DLOAD, PDVL),
    ADDRESS_PLUS_ONE(ZERORTE),
    ORDERS(DOT, VDEF),
    ADDRESS_PLUS_ONE(UH_),
    ORDER(VSL1),
    STODL(DELVLVC),
    ADDRESS_PLUS_ONE(DELVLVC),
    ORDERS(BOFF, DCOMP),
    SWITCH(BOFF, RETROFLG),
    ADDRESS(RTD18),
    STORE(DELVLVC), /* "NEGATE X COMPONENT, RETROGRADE" */
    /* RTD18 */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(DELVLVC),
    STOVL(VGDISP),
    ADDRESS_PLUS_ONE(R_T2_),
    STORE(RVEC), /* "***** LANDING SITE COMPUTATION *****" */
};

/* RTD18 +5, the "LANDING SITE COMPUTATION" (RVEC R(T2)/). TMRAD100
 * again, from R(T2)/ and V(T2)/ to a radius 30480 meters lower
 * (30480RTE): "R3,V3,T23 FROM TIMERAD". The flight path
 * angle there ("GAMMAE=ARCSIN(UR3 . UV3)"), the speed, and AUGEKUGL the
 * range of the entry from them ("PHIE"), unless P37RANGE holds one
 * ("OVERRIDE RANGE (PCR 261)"); and with T12 and T2 the time of landing,
 * "T(LS)=T2&T23&TE". The landing site's direction ALPHAV, at that range
 * from UR3 along the horizontal UH3 ("UH3=UNIT(UR3 X UV3 X UR3)",
 * "ALPHAV=UR3(COSPHIE)+UH3(SINPHIE)"), with the sine and cosine held in
 * LNG(SPL) and LAT(SPL) on the way. LAT-LONG, for the earth (LUNAFLAG
 * clear) with its fixed radius (ERADFLAG clear), turns it into the
 * "LATITUDE LANDING SITE" LAT(SPL) and "LONGITUDE LANDING SITE" LNG(SPL);
 * and back through SPRTEX. */
static const word rtd18_words[] = {
    /* RTD18 +5 */ ORDERS(ABVAL, DSU),
    ADDRESS_PLUS_ONE(30480RTE),
    STOVL(RDESIRED),
    ADDRESS_PLUS_ONE(V_T2_),
    STCALL(VVEC),
    ADDRESS(TMRAD100), /* "R3,V3,T23 FROM TIMERAD" */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R_T2_),
    ORDERS(PDVL, UNIT), /* "UR3" */
    ADDRESS_PLUS_ONE(V_T2_),
    ORDERS(DOT, SL1),     /* "GAMMAE=ARCSIN(UR3 . UV3)" */
    ORDERS(ARCSIN, PDDL), /* "V(T3)" */
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(PDDL, ABS),
    ORDERS(PUSH, CALL), /* "/GAMMAE/" */
    ADDRESS(AUGEKUGL),  /* "PHIE" */
    ORDERS(DAD, DAD),
    ADDRESS_PLUS_ONE(T12), /* "T23" */
    ADDRESS_PLUS_ONE(T2),
    STORE_D(2), /* "T(LS)=T2&T23&TE" */
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(P37RANGE),
    ADDRESS(RTD22),
    STORE_D(4), /* "OVERRIDE RANGE (PCR 261)" */
    /* RTD22 */ ORDERS(DLOAD, SIN),
    ADDRESS_PLUS_ONE_D(4),
    STODL(LNG_SPL), /* "LNG(SPL)=SIN(PHIE)" */
    ORDER(COS),
    STORE(LAT_SPL), /* "LAT(SPL)=COS(PHIE)" */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R_T2_),
    ORDERS(PUSH, PUSH),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(V_T2_),
    ORDERS(PDVL, VXV),
    ORDERS(VXV, UNIT), /* "UH3=UNIT(UR3 X UV3 X UR3)" */
    ORDERS(VXSC, PDVL),
    ADDRESS_PLUS_ONE(LNG_SPL),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(LAT_SPL),
    ORDERS(CLEAR, CLEAR), /* "T(LS) IN MPAC" */
    SWITCH(CLEAR, ERADFLAG),
    SWITCH(CLEAR, LUNAFLAG),
    STODL(ALPHAV), /* "ALPHAV=UR3(COSPHIE)+UH3(SINPHIE)" */
    ORDER(CALL),
    ADDRESS(LAT_LONG),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(LAT),
    STODL(LAT_SPL), /* "LATITUDE LANDING SITE  *****" */
    ADDRESS_PLUS_ONE(LONG),
    STCALL(LNG_SPL), /* "LONGITUDE LANDING SITE *****" */
    ADDRESS(SPRTEX),
};

const struct interpretive_code TMRAD100_CODE = {032u, TMRAD100, COUNT(tmrad100_words),
                                                tmrad100_words};
const struct interpretive_code RTD18_CODE = {032u, TMRAD100 + COUNT(tmrad100_words),
                                             COUNT(rtd18_words), rtd18_words};

#endif
