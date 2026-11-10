/* SPDX-License-Identifier: MIT */
/* R31.agc (Luminary099 pp. 703-708, Comanche055 pp. 505-510): R31, the
 * rendezvous parameter display, verb 83. Its basic code, and its
 * interpretive code in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs (src/binding/r31_routines.c). The
 * faithful C is c64-port/src/lm_r31_file.c and cm_r31.c.
 *
 * Two jobs share the work. The first, from V83CALL, extrapolates both
 * vehicles' state vectors to now and works out the range (RANGE), the
 * range rate (RRATE) and theta (RTHETA), the angle of the vehicle's own
 * axis (the LM's Z, the CSM's X) out of its local horizontal plane; then,
 * unless the display has been answered, it sets bit 12 of EXTVBACT "AND
 * START AGAIN" (REV83, the CM's V83). The second, DISPN5X, shows them,
 * verb 16 noun 54, until the astronaut answers it; R31CALL starts it once
 * bit 12 says the first values are there (DSPDELAY, a second at a
 * time).
 *
 * The missions differ. The LM's R31CALL is a waitlist task (bank 40: it
 * waits by FIXDELAY and starts the display by NOVAC); its base vectors are
 * kept from the first pass and each later one extrapolates them by conic
 * (or for the LM on the surface by precision integration), and with average
 * G on it takes the LM's state from RN and VN. The CM's R31CALL is a job
 * that waits by DELAYJOB and becomes the display; its later passes
 * integrate from the base vectors too, and each pass also finds the
 * optics' line of sight (R34LOS), which R34's angle would use. All in the
 * LM's bank 37 or the CM's bank 31.
 *
 * Each routine ends where its code leaves it. A call out of the file
 * (FINDVAC, BANKCALL, B5OFF, PRIOCHNG) or a TC INTPRET leaves Z at the
 * routine and Q at the word after the TC (for INTPRET the first
 * interpretive word); a jump (TCF, BZF) to another of these routines
 * leaves Z at its label. */
#include "r31.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * (for INTPRET, the first interpretive word), and the routine runs
 * next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- R31CALL and the display -- LM p. 705, CM p. 507 ---- */

/* R31CALL: a job at V83CALL, priority 3 (FINDVAC, 2CADR V83CALL). */
void r31call(void)
{
    set_A(PRIO3);
    tc_to(R31CALL + 1u, FINDVAC);
}

#if LUMINARY099
/* DSPDELAY: the task waits a second (TC FIXDELAY, DEC 100). */
void dspdelay(void)
{
    tc_to(DSPDELAY, FIXDELAY);
}

/* DSPDELAY +2: until bit 12 of EXTVBACT is set, wait; then the
 * display, a job at DISPN5X, priority 5 (NOVAC, 2CADR DISPN5X). */
void dspdelay_wake(void)
{
    ca(ADDRESS_OF(EXTVBACT));
    mask(BIT12);
    if (A_is_zero()) { /* EXTEND; BZF DSPDELAY */
        continue_at(DSPDELAY);
        return;
    }
    set_A(PRIO5);
    tc_to(DSPDELAY + 7u, NOVAC);
}

/* DSPDELAY +12: the task ends (TCF TASKOVER). */
void r31_taskover(void)
{
    continue_at(TASKOVER);
}

/* DISPN5X: verb 16 noun 54 (BANKCALL, CADR GOMARKF). */
void dispn5x(void)
{
    set_A(V16N54);
    tc_to(DISPN5X + 1u, BANKCALL);
}
#else
/* DSPDELAY: the job sleeps a second (BANKCALL, CADR DELAYJOB). */
void dspdelay(void)
{
    set_A(ONE_SEC);
    tc_to(DSPDELAY + 1u, BANKCALL);
}

/* DISPN5X +6: "R34 USE NOUN 53", verb 16 noun 53 (BANKCALL, CADR
 * GOMARKF). */
void dispn5x_r34(void)
{
    set_A(V16N53);
    tc_to(DISPN5X + 7u, BANKCALL);
}

/* DISPN5X: R31FLAG, bit 4 of FLAGWRD9, set for R31 ("IN SUNDANCE R31FLAG
 * WILL ALWAYS BE SET AS R34 DOES NOT EXIST"): "R31 USE NOUN 54", verb 16
 * noun 54 (TC +2), else R34's. */
void dispn5x(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(BIT4);
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        continue_at(DISPN5X + 6u);
        return;
    }
    set_A(V16N54);
    tc_at(DISPN5X + 5u);           /* TC +2 */
    tc_to(DISPN5X + 7u, BANKCALL); /* CADR GOMARKF */
}

/* DSPDELAY +3: until bit 12 of EXTVBACT is set, sleep; then on
 * into the display. */
void dspdelay_wake(void)
{
    ca(ADDRESS_OF(EXTVBACT));
    mask(BIT12);
    if (A_is_zero()) { /* EXTEND; BZF DSPDELAY */
        continue_at(DSPDELAY);
        return;
    }
    dispn5x();
}
#endif

/* GOMARKF's returns: terminate and proceed both end the display (B5OFF:
 * bit 5 of EXTVBACT clear, which the other job reads as answered), */
void dispn5x_terminate(void)
{
#if LUMINARY099
    tc_to(DISPN5X + 3u, B5OFF);
#else
    tc_to(DISPN5X + 011u, B5OFF);
#endif
}

void dispn5x_proceed(void)
{
#if LUMINARY099
    tc_to(DISPN5X + 4u, B5OFF);
#else
    tc_to(DISPN5X + 012u, B5OFF);
#endif
}

/* and a recycle shows it again (TCF DISPN5X). */
void dispn5x_recycle(void)
{
    continue_at(DISPN5X);
}

/* ---- The state vectors -- LM pp. 705-706, CM pp. 507-509 ---- */

#if LUMINARY099
/* V83CALL: with average G on ("TEST AVERAGE G FLAG", AVEGFLAG in
 * FLAGWRD7) "ON - TEST MUNFLAG" (MUNG?); with the LM "ON SURFACE -
 * BYPASS LEMPREC" (SURFFLAG, ONEBASE); else "EXTRAPOLATE BOTH STATE
 * VECTORS", to the interpreter, for V83CALL_CODE. */
void v83call(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(AVEGFBIT);
    if (A_is_zero()) { /* EXTEND; BZF MUNG? */
        continue_at(MUNG_Q);
        return;
    }
    cs(ADDRESS_OF(FLAGWRD8));
    mask(SURFFBIT);
    if (A_is_zero()) { /* EXTEND; BZF ONEBASE */
        continue_at(ONEBASE);
        return;
    }
    tc_to(V83CALL + 010u, INTPRET);
}

/* V83CALL +11: both vehicles' state vectors to now (LOADTIME) by
 * precision integration, kept as the base vectors: the LM's ("PRECISION
 * BASE VECTOR FOR LM", LEMPREC) in BASETHP and BASETHV, then DOCMBASE,
 * the time in BASETIME and the CSM's ("PRECISION BASE VECTOR FOR CM",
 * CSMPREC) in BASEOTP and BASEOTV. Then the basic code (EXIT), on into
 * REV83. */
static const word v83call_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STCALL(TDEC1),
    ADDRESS(LEMPREC), /* "PRECISION BASE VECTOR FOR LM" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(BASETHP),
    ADDRESS_PLUS_ONE(VATT1),
    STODL(BASETHV),
    ADDRESS_PLUS_ONE(TAT),
    /* DOCMBASE */
    STORE(BASETIME), /* "PRECISION BASE VECTOR FOR CM" */
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(BASEOTP),
    ADDRESS_PLUS_ONE(VATT1),
    STORE(BASEOTV),
    ORDER(EXIT),
};

/* REV83: "IF AVEGFLAG SET, USE RN,VN" (GETRVN); "IF ON SURFACE, USE
 * LEMAREC" (R31SURF); else "DO CONIC EXTRAPOLATION FOR BOTH VEHICLES", to
 * the interpreter, for REV83_CODE. */
void rev83(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(AVEGFBIT);
    if (A_is_zero()) { /* EXTEND; BZF GETRVN */
        continue_at(GETRVN);
        return;
    }
    cs(ADDRESS_OF(FLAGWRD8));
    mask(SURFFBIT);
    if (A_is_zero()) { /* EXTEND; BZF R31SURF */
        continue_at(R31SURF);
        return;
    }
    tc_to(REV83 + 010u, INTPRET);
}

/* REV83 +11: the LM's base vector to now by conic (INTYPFLG set: "CONIC
 * EXTRAP."), from BASETHP, BASETHV and BASETIME, with MOONFLAG as
 * MOONTHIS ("GET APPROPRIATE MOONFLAG SETTING"), "INTEGRATION --- AT LAST
 * ---". OTHCONIC: that position and velocity to RONE and VONE, "GET SET
 * FOR CONIC EXTRAP., OTHER."; OTHINT: the CSM's base vector the same way
 * to the time in MPAC. COMPDISP then has the CSM's in RATT and VATT. */
static const word rev83_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STCALL(TDEC1),
    ADDRESS(INTSTALL),
    ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(BASETHP),
    SWITCH(CLEAR, MOONFLAG),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(BASETHV),
    STODL(VCV),
    ADDRESS_PLUS_ONE(BASETIME),
    ORDERS(BOF, SET), /* "GET APPROPRIATE MOONFLAG SETTING" */
    SWITCH(BOF, MOONTHIS),
    ADDRESS_AT(REV83, 030u), /* "+2" */
    SWITCH(SET, MOONFLAG),
    ORDER(SET),
    SWITCH(SET, INTYPFLG), /* "CONIC EXTRAP." */
    STCALL(TET),
    ADDRESS(INTEGRVS), /* "INTEGRATION --- AT LAST ---" */
    /* OTHCONIC */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RONE),
    ADDRESS_PLUS_ONE(VATT),
    STCALL(VONE), /* "GET SET FOR CONIC EXTRAP., OTHER." */
    ADDRESS(INTSTALL),
    ORDERS(SET, DLOAD),
    SWITCH(SET, INTYPFLG),
    ADDRESS_PLUS_ONE(TAT),
    /* OTHINT */
    STORE(TDEC1),
    ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(BASEOTP),
    SWITCH(CLEAR, MOONFLAG),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(BASEOTV),
    STODL(VCV),
    ADDRESS_PLUS_ONE(BASETIME),
    ORDERS(BOF, SET),
    SWITCH(BOF, MOONTHIS),
    ADDRESS_AT(OTHINT, 014u), /* "+2" */
    SWITCH(SET, MOONFLAG),
    STCALL(TET),
    ADDRESS(INTEGRVS),
    /* COMPDISP: the line of sight, the CSM's position less the LM's
     * (RATT - RONE): "UNIT(RANGE) TO PD 0-5" (NORMUNX1), its length
     * rescaled "AFTER NORMUNIT" (SL* by X1) to RANGE ("SCALED 2(29)M");
     * the relative velocity along it, "(VCM-VLM).UNIT(LOS)", to RRATE
     * ("SCALED 2(7)M/CS"). Then theta: the LM's Z axis (UNITZ, by
     * CDU*NBSM and REFSMMAT) less its part along the LM's position,
     * "UNIT(P)=UNIT(UZ-(UZ)PROJ(UR))"; the sign of "(UR * VL) * UR . U(P)";
     * "ARCCOS(UP.UZ(SIGN))" to RTHETA, made "1 - RTHETA" if the Z axis
     * points down ("IF UR.UZ NEG"). */
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(RTB, PDDL),
    ADDRESS(NORMUNX1), /* "UNIT(RANGE) TO PD 0-5" */
    ADDRESS_PLUS_ONE_D(36),
    ORDER(SL_STAR), /* "RESCALE AFTER NORMUNIT" */
    SL_X1(0),
    STOVL(RANGE), /* "SCALED 2(29)M" */
    ADDRESS_PLUS_ONE(VATT),
    ORDERS(VSU, DOT), /* "(VCM-VLM).UNIT(LOS), PD=0" */
    ADDRESS_PLUS_ONE(VONE),
    ORDER(SL1), /* "SCALED 2(7)M/CS" */
    STOVL(RRATE),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(UNIT, PDVL), /* "UNIT(R) TO PD 0-5" */
    ADDRESS_PLUS_ONE(UNITZ),
    ORDER(CALL),
    ADDRESS(CDU_STAR_NBSM),
    ORDERS(VXM, PUSH), /* "UNIT(Z)/4 TO PD 6-11" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VPROJ, VSL2), /* "UNIT(P)=UNIT(UZ-(UZ)PROJ(UR))" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(BVSU, UNIT),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(PDVL, VXV), /* "UNIT(P) TO PD 12-17" */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VONE),
    ORDERS(VXV, DOT), /* "(UR * VL) * UR . U(P)" */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(PDVL), /* "SIGN TO 12-13, LOAD U(P)" */
    ORDERS(DOT, SIGN),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(SL2, ACOS), /* "ARCCOS(UP.UZ(SIGN))" */
    STOVL(RTHETA),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DOT, BPL), /* "IF UR.UZ NEG," */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_AT(COMPDISP, 055u), /* "+5" */
    ORDERS(DLOAD, DSU),         /* "RTHETA = 1 - RTHETA" */
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ADDRESS_PLUS_ONE(RTHETA),
    STORE(RTHETA),
    ORDER(EXIT),
};
#else
/* V83: the later passes, to the interpreter: GOTO HAVEBASE, "INTEG STATE
 * VECTORS". */
void v83(void)
{
    tc_to(V83, INTPRET);
}

static const word v83_words[] = {
    ORDER(GOTO),
    ADDRESS(HAVEBASE), /* "INTEG STATE VECTORS" */
};

/* V83CALL: the first, GOTO STATEXTP, "EXTRAPOLATE STATE VECTORS". */
void v83call(void)
{
    tc_to(V83CALL, INTPRET);
}

static const word v83call_words[] = {
    ORDER(GOTO),
    ADDRESS(STATEXTP), /* "EXTRAPOLATE STATE VECTORS" */
};

/* COMPDISP: the line of sight, the LM's position less the CSM's (RATT -
 * RONE), its length to RANGE ("METERS B-29"), its unit vector (NORM,
 * VSL* by X1); the relative velocity along it, "(VATT-VONE).UNIT(LOS)",
 * to RRATE ("RANGE RATE M/CS B-7"); CDUTRIG "TO INITIALIZE FOR *NBSM*",
 * and R34LOS ("PDL MUST = 0"). R34ANG: the angle, from the CSM's X axis
 * (THISAXIS: "UNITX FOR CM, UNITZ FOR LM"; with R31FLAG clear, R34's
 * PHI, from the optics' axis in 12D) by *NBSM* and REFSMMAT, less its
 * part along the CSM's position; the sign from the orbit's plane;
 * "UP.UXORZ" and its arc cosine to RTHETA; "IF UXORZ.R NEG, RTHETA = 1 -
 * RTHETA", "RTHETA BETWEEN 0 AND 1 REV.". Then the basic code (EXIT). */
static const word compdisp_words[] = {
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(PUSH, ABVAL), /* "RATT-RONE TO 0D" */
    STORE(RANGE),        /* "METERS B-29" */
    ORDERS(NORM, VLOAD),
    ADDRESS_PLUS_ONE(X1),
    ORDER(VSR1),
    ORDERS(VSL_STAR, UNIT),
    SL_X1(0),
    ORDERS(PDVL, VSU), /* "UNIT(LOS) TO 0D" */
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(VONE),
    ORDER(DOT), /* "(VATT-VONE).UNIT(LOS)" */
    ORDER(SL1),
    STCALL(RRATE),    /* "RANGE RATE M/CS B-7" */
    ADDRESS(CDUTRIG), /* "TO INITIALIZE FOR *NBSM*" */
    ORDER(CALL),
    ADDRESS(R34LOS), /* "NOTE.  PDL MUST = 0." */
    /* R34ANG */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RONE),
    ORDER(PDVL),                /* "UR TO 0D" */
    ADDRESS_PLUS_ONE(THISAXIS), /* "UNITX FOR CM, UNITZ FOR LM" */
    ORDERS(BON, VLOAD),         /* "CHK R31FLAG.  ON=R31 THETA, OFF=R34 PHI" */
    SWITCH(BON, R31FLAG),
    ADDRESS_AT(R34ANG, 010u), /* "+2": "R31-THETA" */
    ADDRESS_PLUS_ONE_D(12),
    ORDER(CALL),
    ADDRESS(STAR_NBSM_STAR),
    ORDERS(VXM, PUSH), /* "UXORZ TO 6D" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VPROJ, VSL2),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(BVSU, UNIT),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(PDVL, VXV), /* "UP/2 TO 12D" */
    ADDRESS_PLUS_ONE(RONE),
    ADDRESS_PLUS_ONE(VONE),
    ORDERS(UNIT, VXV),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(DOT, PDVL), /* "SIGN TO 12D, UP/2 TO MPAC" */
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(VSL1, DOT), /* "UP.UXORZ" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(SIGN, SL1),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(ACOS),
    STOVL(RTHETA),
    ADDRESS_PLUS_ONE(RONE),
    ORDERS(DOT, BPL),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_AT(R34ANG, 045u), /* "+5" */
    ORDERS(DLOAD, BDSU),      /* "IF UXORZ.R NEG, RTHETA = 1 - RTHETA" */
    ADDRESS_PLUS_ONE(RTHETA),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STORE(RTHETA), /* "RTHETA BETWEEN 0 AND 1 REV." */
    ORDER(EXIT),
};

/* STATEXTP: "INITIAL INTEGRATION". The time: now with average G off
 * (V37FLAG: "AV G OFF, USE PRES TIME"), else GETRVN's PIPTIME, with RN
 * and VN in RONE and VONE ("ON, USE RN VN PIPTIME"). The LM's state
 * vector to it by precision integration (LEMPREC), the "BASE VECTOR, LM"
 * in BASEOTP and BASEOTV; with average G on, straight to COMPDISP
 * ("COMPUTE R RDOT RTHETA FROM RONE(RN) VONE(VN) RATT+VATT(LEMPREC)").
 * Else the CSM's too (CSMPREC), the "BASE VECTOR, CM" in BASETHP and
 * BASETHV.
 *
 * HAVEBASE: "SUBSEQUENT INTEGRATIONS". With average G off, the CSM's base
 * vector to now ("AV G OFF. SET INTEG. OF CM") with MOONFLAG as MOONTHIS,
 * by precision with the LM on the surface ("PREC. IF LM DOWN"), else by
 * conic ("CONIC IF LM NOT DOWN"), "INTEGRATION --- AT LAST---", to RONE
 * and VONE ("GET SET FOR CONIC EXTRAP.,OTHER"); then the LM's, by
 * LEMPREC "IF LM DOWN" (GETRVN6), else OTHINT ("ENTERED IF AV G ON TO
 * INTEG LM"): its base vector by conic (or with average G on by
 * precision, GETRVN5) to the time, and COMPDISP ("COMPUTE R RDOT
 * RTHETA").
 *
 * GETRVN: "AV G ON, RONE = RN VONE = VN", "AND USE PIPTIME" in MPAC; the
 * return kept in 0D (STQ) and taken by GOTO 0D. Each GOTO +1 goes on to
 * the next word. Then R34LOS's EXIT, to the basic code. */
static const word statextp_words[] = {
    ORDERS(RTB, BOF), /* "INITIAL INTEGRATION" */
    ADDRESS(LOADTIME),
    SWITCH(BOF, V37FLAG),
    ADDRESS_AT(STATEXTP, 6u), /* "+3": "AV G OFF, USE PRES TIME" */
    ORDER(CALL),
    ADDRESS(GETRVN), /* "ON,  USE RN VN PIPTIME" */
    STORE(BASETIME), /* "PRES TIME OR PIPTIME" */
    STCALL(TDEC1),
    ADDRESS(LEMPREC),
    ORDER(VLOAD), /* "BASE VECTOR, LM" */
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(BASEOTP), /* "POS." */
    ADDRESS_PLUS_ONE(VATT1),
    STORE(BASEOTV), /* "VEL." */
    ORDERS(BON, DLOAD),
    SWITCH(BON, V37FLAG),
    ADDRESS(COMPDISP), /* "COMPUTE R RDOT RTHETA FROM" */
    ADDRESS_PLUS_ONE(TAT),
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(VLOAD), /* "BASE VECTOR, CM" */
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(BASETHP), /* "POS." */
    ADDRESS_PLUS_ONE(VATT1),
    STORE(BASETHV), /* "VEL." */
    /* HAVEBASE */
    ORDERS(BON, RTB), /* "SUBSEQUENT INTEGRATIONS" */
    SWITCH(BON, V37FLAG),
    ADDRESS(GETRVN5),
    ADDRESS(LOADTIME),
    STCALL(TDEC1), /* "AV G OFF. SET INTEG. OF CM" */
    ADDRESS(INTSTALL),
    ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(BASETHP),
    SWITCH(CLEAR, MOONFLAG),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(BASETHV),
    STODL(VCV),
    ADDRESS_PLUS_ONE(BASETIME),
    ORDERS(BOF, SET), /* "GET APPROPRIATE MOONFLAG SETTING" */
    SWITCH(BOF, MOONTHIS),
    ADDRESS_AT(HAVEBASE, 021u), /* "+2" */
    SWITCH(SET, MOONFLAG),
    ORDER(CLEAR),
    SWITCH(CLEAR, INTYPFLG),
    ORDERS(BON, SET),
    SWITCH(BON, SURFFLAG),
    ADDRESS_AT(HAVEBASE, 027u), /* "+2": "PREC. IF LM DOWN" */
    SWITCH(SET, INTYPFLG),      /* "CONIC IF LM NOT DOWN" */
    STCALL(TET),
    ADDRESS(INTEGRVS), /* "INTEGRATION --- AT LAST---" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RONE),
    ADDRESS_PLUS_ONE(VATT),
    STODL(VONE), /* "GET SET FOR CONIC EXTRAP.,OTHER" */
    ADDRESS_PLUS_ONE(TAT),
    ORDERS(BON, CALL),
    SWITCH(BON, SURFFLAG),
    ADDRESS(GETRVN6),  /* "LEMPREC IF LM DOWN" */
    ADDRESS(INTSTALL), /* "..CONIC IF NOT DOWN" */
    ORDER(SET),
    SWITCH(SET, INTYPFLG),
    /* OTHINT */
    STORE(TDEC1), /* "ENTERED IF AV G ON TO INTEG LM" */
    ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(BASEOTP),
    SWITCH(CLEAR, MOONFLAG),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(BASEOTV),
    STODL(VCV),
    ADDRESS_PLUS_ONE(BASETIME),
    ORDERS(BOF, SET),
    SWITCH(BOF, MOONTHIS),
    ADDRESS_AT(OTHINT, 014u), /* "+2" */
    SWITCH(SET, MOONFLAG),
    STCALL(TET),
    ADDRESS(INTEGRVS),
    ORDER(GOTO),
    ADDRESS(COMPDISP), /* "COMPUTE R RDOT RTHETA" */
    /* GETRVN5 */
    ORDER(CALL), /* "AV G ON" */
    ADDRESS(GETRVN),
    ORDERS(BON, CALL),
    SWITCH(BON, SURFFLAG),
    ADDRESS(GETRVN6), /* "LM DOWN, LMPREC" */
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, INTYPFLG),
    ADDRESS(OTHINT),
    /* GETRVN6 */
    STCALL(TDEC1),
    ADDRESS(LEMPREC),
    ORDER(GOTO),
    ADDRESS(COMPDISP), /* "COMPUTE R RDOT RTHETA" */
    /* GETRVN */
    ORDER(STQ),
    ADDRESS_D(0),
    ORDERS(VLOAD, GOTO), /* "AV G ON, RONE = RN VONE = VN" */
    ADDRESS_PLUS_ONE(RN),
    ADDRESS_AT(GETRVN, 5u), /* "+1" */
    STCALL(RONE),
    ADDRESS_AT(GETRVN, 7u), /* "+1" */
    ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(VN),
    ADDRESS_AT(GETRVN, 012u), /* "+1" */
    STODL(VONE),
    ADDRESS_PLUS_ONE(PIPTIME),
    ORDER(GOTO),
    ADDRESS_D(0),
    /* R34LOS */
    ORDER(EXIT),
};
#endif

/* The word after COMPDISP's EXIT (the LM's GETRVN -10, the CM's V83 +77):
 * "IF ANSWERED, TERMINATE" (bit 5 of EXTVBACT clear: ENDEXT); else "SET
 * BIT 12" of EXTVBACT "AND START AGAIN." (REV83, the CM's V83). */
void r31_after_exit(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(EXTVBACT));
    if (A_is_zero()) { /* EXTEND; BZF ENDEXT */
        continue_at(ENDEXT);
        return;
    }
    cs(ADDRESS_OF(EXTVBACT));
    mask(BIT12);
    ads(ADDRESS_OF(EXTVBACT));
#if LUMINARY099
    continue_at(REV83); /* TCF REV83 */
#else
    continue_at(V83); /* TCF V83 */
#endif
}

#if LUMINARY099
/* ---- Average G on, and the LM on the surface -- LM pp. 707-708 ---- */

/* GETRVN: "INHIBIT SERVICER" (PRIOCHNG to priority 22), */
void getrvn(void)
{
    set_A(PRIO22);
    tc_to(GETRVN + 1u, PRIOCHNG);
}

/* and on to the interpreter, for GETRVN_CODE. */
void getrvn_intpret(void)
{
    tc_to(GETRVN + 2u, INTPRET);
}

/* GETRVN +3: "LM STATE VECTOR IN RN,VN" to RONE and VONE; "LOAD
 * R(CSM),V(CSM) IN CASE MUNFLAG SET" "(TO INSURE TIME COMPATABILITY)",
 * on the push-down list with PIPTIME in MPAC. */
static const word getrvn_words[] = {
    ORDERS(VLOAD, SETPD),      ADDRESS_PLUS_ONE(RN), /* "LM STATE VECTOR IN RN,VN" */
    ADDRESS_PLUS_ONE_D(0),     STOVL(RONE),
    ADDRESS_PLUS_ONE(VN),      STOVL(VONE), /* "LOAD R(CSM),V(CSM) IN CASE MUNFLAG SET" */
    ADDRESS_PLUS_ONE(V_CSM),                /* "(TO INSURE TIME COMPATABILITY)" */
    ORDERS(PDVL, PDDL),        ADDRESS_PLUS_ONE(R_CSM),
    ADDRESS_PLUS_ONE(PIPTIME), ORDER(EXIT),
};

/* GETRVN +16, after the EXIT: priority 3 again (PRIOCHNG), */
void getrvn_after_exit(void)
{
    set_A(PRIO3);
    tc_to(GETRVN + 017u, PRIOCHNG);
}

/* and on to the interpreter, for GETRVN_EXIT_CODE. */
void getrvn_after_exit_intpret(void)
{
    tc_to(GETRVN + 020u, INTPRET);
}

/* GETRVN +21: "IF MUNFLAG RESET, DO CM DELTA PRECISION" (GETRVN2: the
 * CSM's base vector to PIPTIME by precision, OTHINT with INTYPFLG clear,
 * "PREC EXTRAP FOR OTHER"). With MUNFLAG set, R(CSM) and V(CSM): "CHANGE
 * TO REFERENCE SYSTEM AND RESCALE" (REFSMMAT), "R TO PD 0-5", "V TO PD
 * 5-11", then COMPDISP. */
static const word getrvn_exit_words[] = {
    ORDERS(BOFF, VLOAD),
    SWITCH(BOF, MUNFLAG),
    ADDRESS(GETRVN2),  /* "IF MUNFLAG RESET, DO CM DELTA PRECISION" */
    ORDERS(VXM, VSR4), /* "CHANGE TO REFERENCE SYSTEM AND RESCALE" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(PDVL), /* "R TO PD 0-5" */
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(PUSH, SETPD), /* "V TO PD 5-11" */
    ADDRESS_PLUS_ONE_D(0),
    ORDER(GOTO),
    ADDRESS(COMPDISP),
    /* GETRVN2 */
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, INTYPFLG), /* "PREC EXTRAP FOR OTHER" */
    ADDRESS(OTHINT),
};

/* R31SURF: to the interpreter, for R31SURF_CODE. */
void r31surf(void)
{
    tc_to(R31SURF, INTPRET);
}

/* R31SURF +1: "LM IS ON SURFACE, SO PRECISION" "INTEGRATION USES
 * PLANETARY INERTIAL" "ORIENTATION SUBROUTINE" (LEMPREC to now), and "DO
 * CSM CONIC" (OTHCONIC). */
static const word r31surf_words[] = {
    ORDER(RTB),        ADDRESS(LOADTIME), STCALL(TDEC1),
    ADDRESS(LEMPREC),  ORDER(GOTO), /* "DO CSM CONIC" */
    ADDRESS(OTHCONIC),
};

/* MUNG?: average G on; "IF MUNFLAG SET, CSM BASE NOT NEEDED" (GETRVN),
 * else on to ONEBASE. */
void mung(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(MUNFLBIT);
    if (A_is_zero()) { /* EXTEND; BZF GETRVN */
        continue_at(GETRVN);
        return;
    }
    onebase(); /* on into ONEBASE */
}

/* ONEBASE: "GET CSM BASE VECTOR", to the interpreter, for ONEBASE_CODE: */
void onebase(void)
{
    tc_to(ONEBASE, INTPRET);
}

/* the time now, and DOCMBASE. */
static const word onebase_words[] = {
    ORDERS(RTB, GOTO),
    ADDRESS(LOADTIME),
    ADDRESS(DOCMBASE),
};
#else
/* ---- R34LOS -- CM p. 510 ---- */

/* R34LOS +1, after its EXIT: the optics' CDUs, the shaft (CDUS) to 9D
 * and the trunnion (CDUT) to 11D of the work area, and X1 the work area's
 * -(FIXLOC +6), for SXTNB; to the interpreter, for R34LOS_CODE. Each TS
 * would skip the next word on an overflow, which a CA never leaves. */
void r34los_after_exit(void)
{
    ca(ADDRESS_OF(CDUS));
    ts(indexed(9u, FIXLOC)); /* INDEX FIXLOC; TS 9D */
    ca(ADDRESS_OF(CDUT));
    ts(indexed(11u, FIXLOC)); /* INDEX FIXLOC; TS 11D */
    ca(ADDRESS_OF(FIXLOC));
    ad(SIX);
    com();
    ts(indexed(ADDRESS_OF(X1), FIXLOC)); /* INDEX FIXLOC; TS X1 */
    tc_to(R34LOS + 014u, INTPRET);
}

/* R34LOS +15: the optics' line of sight (SXTNB) to 12D, and R34ANG. */
static const word r34los_words[] = {
    ORDER(CALL),
    ADDRESS(SXTNB),
    STCALL_D(12),
    ADDRESS(R34ANG),
};
#endif

/* ---- The interpretive code ---- */

#if LUMINARY099
#define R31_BANK 037u
const struct interpretive_code V83CALL_CODE = {R31_BANK, V83CALL + 011u, COUNT(v83call_words),
                                               v83call_words};
const struct interpretive_code REV83_CODE = {R31_BANK, REV83 + 011u, COUNT(rev83_words),
                                             rev83_words};
const struct interpretive_code GETRVN_CODE = {R31_BANK, GETRVN + 3u, COUNT(getrvn_words),
                                              getrvn_words};
const struct interpretive_code GETRVN_EXIT_CODE = {R31_BANK, GETRVN + 021u,
                                                   COUNT(getrvn_exit_words), getrvn_exit_words};
const struct interpretive_code R31SURF_CODE = {R31_BANK, R31SURF + 1u, COUNT(r31surf_words),
                                               r31surf_words};
const struct interpretive_code ONEBASE_CODE = {R31_BANK, ONEBASE + 1u, COUNT(onebase_words),
                                               onebase_words};
#else
#define R31_BANK 031u
const struct interpretive_code V83_CODE = {R31_BANK, V83 + 1u, COUNT(v83_words), v83_words};
const struct interpretive_code V83CALL_CODE = {R31_BANK, V83CALL + 1u, COUNT(v83call_words),
                                               v83call_words};
const struct interpretive_code COMPDISP_CODE = {R31_BANK, IN_BANK(COMPDISP), COUNT(compdisp_words),
                                                compdisp_words};
const struct interpretive_code STATEXTP_CODE = {R31_BANK, IN_BANK(STATEXTP), COUNT(statextp_words),
                                                statextp_words};
const struct interpretive_code R34LOS_CODE = {R31_BANK, R34LOS + 015u, COUNT(r34los_words),
                                              r34los_words};
#endif
