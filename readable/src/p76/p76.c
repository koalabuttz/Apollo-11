/* SPDX-License-Identifier: MIT */
/* P76.agc (Luminary099 pp. 709-711, Comanche055 pp. 511-513): P76, the
 * "TARGET DELTA V PROGRAM", in bank 13 of both missions. Its basic code,
 * and its interpretive code in the listing's notation (src/interpreter/
 * notation.h), which the readable interpreter runs (src/binding/
 * p76_routines.c). The faithful C is c64-port/src/lm_p76_file.c and
 * cm_p76_file.c, with the older modules lm_p76_routines.c and
 * cm_p76_routines.c.
 *
 * "UPON ENTRY BY ASTRONAUT ACTION, P76 FLASHES DSKY REQUESTS": the delta V
 * "TO BE APPLIED TO THE OTHER VEHICLE STATE VECTOR" (verb 06 noun 84) and
 * "THE TIME (TIG) AT WHICH THE OTHER VEHICLE VELOCITY WAS CHANGED" (noun
 * 33). It integrates the other vehicle's state vector to TIG, adds the
 * delta V, "TRANSFORMED FROM LV TO REF COSYS", and integrates the result
 * on to the time of this vehicle's state vector, "THUS INSURING THAT THE
 * W-MATRIX AND BOTH VEHICLE STATES CORRESPOND TO THE SAME TIME". The LM
 * first keeps the last delta V (DELVLVC) to show; the CM, on leaving,
 * also clears VHFCNT and invalidates the mark buffer.
 *
 * Each routine ends where its code leaves it: a call out of the file
 * (UPFLAG, BANKCALL, PHASCHNG) or a TC INTPRET leaves Z at the routine and
 * Q at the word after the TC (for INTPRET the first interpretive word); a
 * TCF ENDP76 leaves Z at ENDP76. A TC +2 or TC -5 to the next display
 * goes on into it. */
#include "p76.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define P76_BANK     013u

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* The words of the code from P76, as the listing places them. */
#if LUMINARY099
#define DELTA_V_AT  (P76 + 7u)    /* CAF V06N84 */
#define TIG_AT      (P76 + 015u)  /* CAF V06N84 +1 */
#define UPDATE_AT   (P76 + 024u)  /* DLOAD TIG */
#define PHASCHNG_AT (P76 + 0107u) /* after MINIRECT's EXIT */
#define INTWAKE_AT  (P76 + 0121u) /* after ATOPOTH's EXIT */
#else
#define DELTA_V_AT  (P76 + 2u)
#define TIG_AT      (P76 + 010u)
#define UPDATE_AT   (P76 + 017u)
#define PHASCHNG_AT (P76 + 0104u)
#define INTWAKE_AT  (P76 + 0116u)
#endif

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * (for INTPRET, the first interpretive word), and the routine runs
 * next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- P76 -- LM p. 709, CM p. 511 ---- */

/* P76: TRACKFLG set (UPFLAG), */
void p76(void)
{
    tc_to(P76, UPFLAG); /* ADRES TRACKFLG */
}

#if LUMINARY099
/* and to the interpreter, for P76_CODE: */
void p76_intpret(void)
{
    tc_to(P76 + 2u, INTPRET);
}

/* the last delta V (DELVLVC) kept in DELVOV, for the display. */
static const word p76_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(DELVLVC),
    STORE(DELVOV),
    ORDER(EXIT),
};
#endif

/* "FLASH LAST DELTA V," verb 06 noun 84, "AND WAIT FOR KEYBOARD ACTION"
 * (BANKCALL, CADR GOFLASH). */
void flash_delta_v(void)
{
    set_A(V06N84);
    tc_to(DELTA_V_AT + 1u, BANKCALL);
}

/* GOFLASH's returns: terminate ends P76 (TCF ENDP76), */
void delta_v_terminate(void)
{
    continue_at(ENDP76);
}

/* "PROCEED" goes on to TIG's display (TC +2), */
void delta_v_proceed(void)
{
    tc_at(DELTA_V_AT + 4u);
    flash_tig();
}

/* and a recycle, after the new data, "REPEAT FLASHING" (TC -5). */
void delta_v_recycle(void)
{
    tc_at(DELTA_V_AT + 5u);
    flash_delta_v();
}

/* "FLASH VERB 06 NOUN 33, DISPLAY LAST TIG," "AND WAIT FOR KEYBOARD
 * ACTION". */
void flash_tig(void)
{
    set_A(V06N84_PLUS_1);
    tc_to(TIG_AT + 1u, BANKCALL);
}

/* Its returns the same: terminate (TCF ENDP76), */
void tig_terminate(void)
{
    continue_at(ENDP76);
}

/* proceed (TC +2), */
void tig_proceed(void)
{
    tc_at(TIG_AT + 4u);
    tig_intpret();
}

/* recycle (TC -5); */
void tig_recycle(void)
{
    tc_at(TIG_AT + 5u);
    flash_tig();
}

/* and on proceed, "RETURN TO INTERPRETIVE CODE", for UPDATE_CODE: */
void tig_intpret(void)
{
    tc_to(TIG_AT + 6u, INTPRET);
}

/* "SET D(MPAC)=TIG IN CSEC B28", "SET TDEC1=TIG FOR ORBITAL INTEGRATION"
 * and the other vehicle's state vector to TIG (OTHPREC). COMPMAT: the
 * matrix that turns the delta V to the reference frame, from the unit
 * vectors "U(-R)", "U(V X R)" and "U((R X V) X R)" (to the push-down list
 * at 24D, 18D and 12D); "DELTA V IN REFCOSYS" added to VATT, "V(PD6)=VATT
 * + DELTA V". INTSTALL, to "PREVENT WOULD-BE USER OF ORBITAL INTEG FROM
 * INTERFERING WITH UPDATING"; P76SUB1 sets MOONFLAG (and the LM's X2, the
 * scaling shift) for the sphere of influence. Then the new state at TIG in RCV
 * and VCV (shifted by X2), TET TIG, and INTOTHIS: INTYPFLG clear, it
 * integrated to this vehicle's time (TETTHIS) by INTEGRVS, and stored
 * through MINIRECT. */
static const word update_words[] = {
    ORDER(DLOAD), /* "SET D(MPAC)=TIG IN CSEC B28" */
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1), /* "SET TDEC1=TIG FOR ORBITAL INTEGRATION" */
    ADDRESS(OTHPREC),
    /* COMPMAT */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(VCOMP),      /* "U(-R)" */
    STORE_D(24),       /* "U(-R) TO 24D" */
    ORDERS(VXV, UNIT), /* "U(-R) X V = U(V X R)" */
    ADDRESS_PLUS_ONE(VATT),
    STORE_D(18),
    ORDERS(VXV, UNIT), /* "U(V X R) X U(-R) = U((R X V) X R)" */
    ADDRESS_PLUS_ONE_D(24),
    STOVL_D(12),
    ADDRESS_PLUS_ONE(DELVOV),
    ORDERS(VXM, VSL1), /* "V(MPAC)=DELTA V IN REFCOSYS" */
    ADDRESS_PLUS_ONE_D(12),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(VATT),
    STORE_D(6),        /* "V(PD6)=VATT + DELTA V" */
    ORDER(CALL),       /* "PREVENT WOULD-BE USER OF ORBITAL" */
    ADDRESS(INTSTALL), /* "INTEG FROM INTERFERING WITH UPDATING" */
    ORDER(CALL),
    ADDRESS(P76SUB1),
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE_D(6),
    SR_X2(0),
    STOVL(VCV),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(VSR_STAR),
    SR_X2(0),
    STODL(RCV),
    ADDRESS_PLUS_ONE(TIG),
    STORE(TET),
    ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, INTYPFLG),
    ADDRESS_PLUS_ONE(TETTHIS),
    /* INTOTHIS */
    STCALL(TDEC1),
    ADDRESS(INTEGRVS),
    ORDER(CALL),
    ADDRESS(INTSTALL),
#if !LUMINARY099
    ORDER(CALL),
    ADDRESS(P76SUB1), /* "SET/CLEAR MOONFLAG" */
#endif
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT1),
    STORE(RRECT),
    STODL(RCV),
    ADDRESS_PLUS_ONE(TAT),
    STOVL(TET),
    ADDRESS_PLUS_ONE(VATT1),
    ORDER(CALL),
    ADDRESS(MINIRECT),
    ORDER(EXIT),
};

/* ---- After the update -- LM pp. 710-711, CM pp. 512-513 ---- */

/* The restart phase (PHASCHNG, OCT 04024), */
void p76_phaschng(void)
{
    tc_to(PHASCHNG_AT, PHASCHNG);
}

/* REINTFLG set (UPFLAG), */
void p76_reintflg(void)
{
    tc_to(PHASCHNG_AT + 2u, UPFLAG); /* ADRES REINTFLG */
}

/* and to the interpreter, for ATOPOTH_CODE: */
void p76_reintflg_intpret(void)
{
    tc_to(PHASCHNG_AT + 4u, INTPRET);
}

/* ATOPOTH, then QPRET OUT (SSP) and the basic code (EXIT). */
static const word atopoth_words[] = {
    ORDER(CALL), ADDRESS(ATOPOTH), ORDERS(SSP, EXIT), ADDRESS_PLUS_ONE(QPRET), ADDRESS(OUT),
};

/* "PERMIT USE OF ORBITAL INTEGRATION" (BANKCALL, CADR INTWAKE1), which
 * returns through QPRET to OUT: */
void p76_intwake(void)
{
    tc_to(INTWAKE_AT, BANKCALL);
}

/* OUT: the basic code again, on into ENDP76. */
static const word out_words[] = {
    ORDER(EXIT),
};

/* ENDP76: "CLEAR RR TRACKING MARK COUNTER" (MARKCTR) and, for the CM,
 * VHFCNT, and "INVALIDATE MARK BUFFER" (MRKBUF2 -1); then GOTOPOOH. Each
 * TS would skip the next word on an overflow, which a CAF never leaves. */
void endp76(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MARKCTR)); /* "CLEAR RR TRACKING MARK COUNTER" */
#if !LUMINARY099
    ts(ADDRESS_OF(VHFCNT));
    set_A(NEGONE);
    ts(ADDRESS_OF(MRKBUF2)); /* "INVALIDATE MARK BUFFER" */
#endif
    continue_at(GOTOPOOH);
}

/* P76SUB1: MOONFLAG ("SET MEANS MOON IS SPHERE OF INFLUENCE.") for the
 * other vehicle's state. The LM's sets it, with X2 2 (the scaling shift
 * for the moon), if CMOONFLG is set ("SET MEANS PERM CM STATE IN LUNAR
 * SPHERE."), else clears it with X2 0. The CM's clears it and sets it
 * again if X2, as the integration left it, is 2 ("X2=0...CLEAR MOONFLAG",
 * "=2.....SET MOONFLAG"). */
static const word p76sub1_words[] = {
#if LUMINARY099
    ORDERS(AXT_2, SET),    NUMBER(2),
    SWITCH(SET, MOONFLAG),                        /* "SET MEANS MOON IS SPHERE OF INFLUENCE." */
    ORDERS(BON, AXT_2),    SWITCH(BON, CMOONFLG), /* "SET MEANS PERM CM STATE IN LUNAR SPHERE." */
    ADDRESS(QPRET),        NUMBER(0),
    ORDERS(CLEAR, RVQ),    SWITCH(CLEAR, MOONFLAG),
#else
    ORDERS(CLEAR, SLOAD),    SWITCH(CLEAR, MOONFLAG),
    ADDRESS_PLUS_ONE(X2),    ORDERS(BHIZ, SET), /* "X2=0...CLEAR MOONFLAG" */
    ADDRESS_AT(P76SUB1, 6u),                    /* "+2" "=2.....SET MOONFLAG" */
    SWITCH(SET, MOONFLAG),   ORDER(RVQ),
#endif
};

#if LUMINARY099
const struct interpretive_code P76_CODE = {P76_BANK, P76 + 3u, COUNT(p76_words), p76_words};
#endif
const struct interpretive_code UPDATE_CODE = {P76_BANK, UPDATE_AT, COUNT(update_words),
                                              update_words};
const struct interpretive_code ATOPOTH_CODE = {P76_BANK, PHASCHNG_AT + 5u, COUNT(atopoth_words),
                                               atopoth_words};
const struct interpretive_code OUT_CODE = {P76_BANK, IN_BANK(OUT), COUNT(out_words), out_words};
const struct interpretive_code P76SUB1_CODE = {P76_BANK, IN_BANK(P76SUB1), COUNT(p76sub1_words),
                                               p76sub1_words};
