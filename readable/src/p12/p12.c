/* SPDX-License-Identifier: MIT */
/* P12.agc, pp. 838-841 (Luminary099 only, bank 30): P12LM, the ascent's
 * start, its two flashing displays (noun 33 and noun 76), and its
 * interpretive code; P12INIT and COMMINIT. The faithful C is
 * c64-port/src/lm_p12.c. */
#include "p12.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- P12LM -- p. 838 ---- */

/* P12LM: restart group 4 at 4.2 (PHASCHNG, OCT 04024), ... */
void p12lm(void)
{
    tc_to(P12LM, PHASCHNG);
}

/* ... "CHECK THE STATUS OF THE IMU." (R02BOTH, through BANKCALL), ... */
void p12lm_after_phaschng(void)
{
    tc_to(P12LM + 2u, BANKCALL);
}

/* ... MUNFLAG set, ... */
void p12lm_after_r02both(void)
{
    tc_to(P12LM + 4u, UPFLAG); /* ADRES MUNFLAG */
}

/* ... "INSURE 4-JET TRANSLATION CAPABILITY." (ACC4-2FL), ... */
void p12lm_after_munflag(void)
{
    tc_to(P12LM + 6u, UPFLAG); /* ADRES ACC4-2FL */
}

/* ... "PREVENT R10 FROM ISSUING CROSS-POINTER OUTPUTS." (R10FLAG), ... */
void p12lm_after_acc4_2fl(void)
{
    tc_to(P12LM + 010u, UPFLAG); /* ADRES R10FLAG */
}

/* ... "INITIALIZE RADMODES FOR R29." (CLRADMOD), ... */
void p12lm_after_r10flag(void)
{
    tc_to(P12LM + 012u, CLRADMOD);
}

/* ... "CLEAR RENDEZVOUS FLAG FOR P22" (RNDVZFLG), ... */
void p12lm_after_clradmod(void)
{
    tc_to(P12LM + 013u, DOWNFLAG); /* ADRES RNDVZFLG */
}

/* FLASH TIG: verb 06 noun 33 flashing (GOFLASH, through BANKCALL). */
static void flash_tig(void)
{
    set_A(V06N33A);
    tc_to(P12LM + 024u, BANKCALL); /* CADR GOFLASH */
}

/* ... "INITIALIZE DVMON" (DVTHRUSH THRESH2, DVCNTR four), TRKMKCNT +0:
 * "SHOW THAT R29 DOWNLINK DATA ISN'T READY.", and FLASH TIG. (Each TS
 * stores a constant: none skips.) */
void p12lm_after_rndvzflg(void)
{
    set_A(THRESH2);
    ts(ADDRESS_OF(DVTHRUSH));
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    ca(ZERO_ADDRESS);
    ts(ADDRESS_OF(TRKMKCNT));
    flash_tig();
}

/* GOFLASH's returns: terminate, to P00 (GOTOPOOH); ... */
void p12lm_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* ... "PROCEED": restart group 4 at 4.2 again, ... */
void p12lm_proceed(void)
{
    tc_to(P12LM + 031u, PHASCHNG);
}

/* ... or "ENTER": FLASH TIG again. */
void p12lm_enter(void)
{
    flash_tig();
}

/* Then the interpretive code (P12LM_CODE). */
void p12lm_intpret(void)
{
    tc_at(P12LM + 033u);
    intpret();
}

/* "INITIALZE WM AND /LAND/" (GUIDINIT); FLPI set and P12INIT. P12LMB:
 * "SET TGO TO AN INITIAL NOMINAL VALUE." ((TGO)A), the state vectors
 * rotated to the ignition time TIG (LEMPREC); V1S, the velocity, and R,
 * the position, in the stable member's frame (REFSMMAT); the gravity
 * (MUNGRAV); UNIT/R/ "FOR YCOMP"; "INITIALIZE XRANGE FOR NOUN 76." (the
 * cross-range, -Y); and the targets ZDOTD (VINJNOM) and RDOTD
 * (RDOTDNOM). */
static const word p12lm_words[] = {
    /* P12LM +034 */ ORDER(CALL), /* "INITIALZE WM AND /LAND/" */
    ADDRESS(GUIDINIT),
    ORDERS(SET, CALL),
    SWITCH(SET, FLPI),
    ADDRESS(P12INIT),
    /* P12LMB */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(_TGOA), /* "SET TGO TO AN INITIAL NOMINAL VALUE." */
    STODL(TGO),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1),
    ADDRESS(LEMPREC),   /* "ROTATE THE STATE VECTORS TO THE" */
    ORDERS(VLOAD, MXV), /* "IGNITION TIME." */
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(VSL1),
    STOVL(V1S), /* "COMPUTE V1S = VEL(TIG)*2(-7)M/CS." */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(MXV, VSL6),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(R),        /* "COMPUTE R = POS(TIG)*2(-24)M." */
    ADDRESS(MUNGRAV), /* "COMPUTE GDT1/2(TIG)*2(-7)M/CS." */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R),
    STCALL(UNIT_R_), /* "COMPUTE UNIT/R/ FOR YCOMP." */
    ADDRESS(YCOMP),
    ORDERS(SR, DCOMP),
    SR_COUNT(5),
    STODL(XRANGE), /* "INITIALIZE XRANGE FOR NOUN 76." */
    ADDRESS_PLUS_ONE(VINJNOM),
    STODL(ZDOTD),
    ADDRESS_PLUS_ONE(RDOTDNOM),
    STORE(RDOTD),
    ORDER(EXIT),
};

const struct interpretive_code P12LM_CODE = {030u, P12LM + 034u, COUNT(p12lm_words), p12lm_words};

/* ---- NEWLOAD -- p. 839 ---- */

/* After the EXIT: restart group 4 at 4.2, ... */
void p12lmb_after_exit(void)
{
    tc_to(NEWLOAD - 2u, PHASCHNG);
}

/* ... and NEWLOAD: "FLASH CROSS-RANGE, AND APOLUNE VALUES." (verb 06 noun
 * 76). */
void newload(void)
{
    set_A(V06N76);
    tc_to(NEWLOAD + 1u, BANKCALL); /* CADR GOFLASH */
}

/* GOFLASH's returns: terminate, to P00; ... */
void newload_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* ... "PROCEED": WHICH the ascent's table of the ignition steps
 * (P12ADRES, P12TABLE), and restart group 4 at 4.2; ... (A is a constant:
 * TS does not skip.) */
void newload_proceed(void)
{
    set_A(P12ADRES);
    ts(ADDRESS_OF(WHICH));
    tc_to(NEWLOAD + 010u, PHASCHNG);
}

/* ... or "ENTER NEW DATA.": NEWLOAD again. */
void newload_enter(void)
{
    newload();
}

/* Then the interpretive code (NEWLOAD_CODE). */
void newload_intpret(void)
{
    tc_at(NEWLOAD + 012u);
    intpret();
}

/* YCO, the cross-range target, from XRANGE and Y; "V(TIPOVER) = V(IGN) +
 * 57FPS (UNIT/R/)" (49FPS); RDOT; ZAXIS1, the unit normal to the plane of
 * QAXIS and UNIT/R/; FLVR set, and ASCENT. P12RET:
 *
 *     YAW = ARCSIN(ATY / SQRT(ATP^2 + ATY^2))   (0 when both are 0)
 *     PITCH = -ARCCOS(UNIT(UNFC/2) . UNIT/R/)
 *
 * (ATP and ATY, the ascent guidance's down-range and out-of-plane thrust
 * components.) */
static const word newload_words[] = {
    /* NEWLOAD +013 */ ORDERS(DLOAD, SL),
    ADDRESS_PLUS_ONE(XRANGE),
    SL_COUNT(5),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(Y),
    STOVL(YCO),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(49FPS),
    ADDRESS_PLUS_ONE(V1S),
    STORE(V), /* "V(TIPOVER) = V(IGN) + 57FPS (UNIT/R/)" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UNIT_R_),
    STOVL(RDOT), /* "RDOT * 2(-7)" */
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(QAXIS),
    STORE(ZAXIS1),
    ORDER(SETGO),
    SWITCH(SETGO, FLVR),
    ADDRESS(ASCENT),
    /* P12RET */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ATP), /* "ATP(2)*2(18)" */
    ORDERS(DSQ, PDDL),
    ADDRESS_PLUS_ONE(ATY), /* "ATY(2)*2(18)" */
    ORDERS(DSQ, DAD),
    ORDERS(BZE, SQRT),
    ADDRESS(YAWDUN),
    ORDERS(SL1, BDDV),
    ADDRESS_PLUS_ONE(ATY),
    ORDER(ARCSIN),
    /* YAWDUN */ STOVL(YAW),
    ADDRESS_PLUS_ONE(UNFC_2),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(SL1, ARCCOS),
    ORDER(DCOMP),
    STORE(PITCH),
    ORDER(EXIT),
};

const struct interpretive_code NEWLOAD_CODE = {030u, NEWLOAD + 013u, COUNT(newload_words),
                                               newload_words};

/* ---- P12RET's end -- p. 840 ---- */

/* After the EXIT: restart group 4 at 4.2, ... */
void p12ret_after_exit(void)
{
    tc_to(NEWLOAD + 062u, PHASCHNG);
}

/* ... FLPI clear, ... */
void p12ret_after_phaschng(void)
{
    tc_to(NEWLOAD + 064u, DOWNFLAG); /* ADRES FLPI */
}

/* ... with interrupts inhibited, PFLITEDB (through IBNKCALL), ... */
void p12ret_after_flpi(void)
{
    inhint();
    tc_to(NEWLOAD + 067u, IBNKCALL); /* CADR PFLITEDB */
}

/* ... and BURNBABY (POSTJUMP). */
void p12ret_after_pflitedb(void)
{
    relint();
    tc_to(NEWLOAD + 072u, POSTJUMP); /* CADR BURNBABY */
}

/* ---- P12INIT and COMMINIT -- pp. 840-841 ---- */

/* P12INIT: "INITIALIZE ENGINE DATA. USED FOR P12 AND P71.": the ascent
 * engine's 1/DV1 to 1/DV3, AT, TBUP and TTO (from ATDECAY), and VE (from
 * APSVEX); with FLAP clear (P70 has not run) on to COMMINIT, else back
 * (RVQ). COMMINIT: "INITIALIZE TARGET DATA. USED BY P12, P70 AND P71 IF
 * IT DOES NOT FOLLOW P70.": RCO the injection's radius (HINJECT + /LAND/),
 * TXO, YCO, RDOTD and YDOTD zero, and QAXIS, the unit normal to the CSM's
 * orbit (VRECTCSM x RRECTCSM) in the stable member's frame. */
static const word p12init_words[] = {
    /* P12INIT */ ORDER(DLOAD), /* "INITIALIZE ENGINE DATA. USED FOR P12 AND" */
    ADDRESS_PLUS_ONE(_1_DVA),   /* "P71." */
    STORE(1_DV3),
    STORE(1_DV2),
    STODL(1_DV1),
    ADDRESS_PLUS_ONE(_ATA),
    STODL(AT),
    ADDRESS_PLUS_ONE(_TBUPA),
    STODL(TBUP),
    ADDRESS_PLUS_ONE(ATDECAY),
    ORDERS(DCOMP, SL),
    SL_COUNT(11),
    STORE(TTO),
    ORDERS(SLOAD, DCOMP),
    ADDRESS_PLUS_ONE(APSVEX),
    ORDER(SR2),
    STORE(VE),
    ORDERS(BOFF, RVQ),
    SWITCH(BOFF, FLAP),
    ADDRESS(COMMINIT),
    /* COMMINIT */ ORDERS(DLOAD, DAD), /* "INITIALIZE TARGET DATA. USED BY P12, P70" */
    ADDRESS_PLUS_ONE(HINJECT),         /* "AND P71 IF IT DOES NOT FOLLOW P70." */
    ADDRESS_PLUS_ONE(_LAND_),
    STODL(RCO),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STORE(TXO),
    STORE(YCO),
    STORE(RDOTD),
    STOVL(YDOTD),
    ADDRESS_PLUS_ONE(VRECTCSM),
    ORDERS(VXV, MXV),
    ADDRESS_PLUS_ONE(RRECTCSM),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(UNIT),
    STORE(QAXIS),
    ORDER(RVQ),
};

const struct interpretive_code P12INIT_CODE = {030u, P12INIT, COUNT(p12init_words), p12init_words};

#endif
