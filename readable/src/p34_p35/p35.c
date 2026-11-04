/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 2 (Luminary099 pp. 664-671, Comanche055 pp.
 * 466-473): P35 and P75, "RENDEZVOUS MID-COURSE MANEUVER PROGRAMS", and
 * S33/34.1, the search for the time of TPI from the elevation angle (or
 * the angle from the time), which P33, P34 and P74 call. The same code
 * in both missions (bank 35). Its basic code, and its interpretive code
 * in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs (src/binding/p34_routines.c). The faithful C
 * is c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * P35 is the active vehicle's program, P75 the passive vehicle's. Each
 * sets the active vehicle flag and takes its own time to the maneuver
 * (ATIGINC or PTIGINC) to KT; then, round and round until the astronaut
 * picks the burn program, the program takes the time now plus KT for the
 * maneuver, TIG, moves both vehicles there, and works out the delta V
 * that takes the active vehicle to the intercept at TPASS4 (saved by
 * P34 or P74), and shows it (S34/35.5, VN1645).
 *
 * Each call out of the file (AVFLAGA, AVFLAGP, P20FLGON) leaves Z at the
 * routine and Q at the word after the TC. */
#include "p34_p35.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- P35 and P75 -- LM p. 666, CM p. 468 ---- */

/* P35: this vehicle the active one (AVFLAGA). */
void p35(void)
{
    tc_to(P35, AVFLAGA);
}

/* P35 +1: the active vehicle's time to the maneuver, then P35/P75A. */
void p35_after_avflaga(void)
{
    dca(ADDRESS_OF(ATIGINC)); /* EXTEND; DCA */
    tc_at(P35 + 3u);
    p35_p75a();
}

/* P75: the other vehicle the active one (AVFLAGP). */
void p75(void)
{
    tc_to(P75, AVFLAGP);
}

/* P75 +1: the passive vehicle's time to the maneuver; on into
 * P35/P75A. */
void p75_after_avflagp(void)
{
    dca(ADDRESS_OF(PTIGINC)); /* EXTEND; DCA */
    p35_p75a();
}

/* P35/P75A: the time to KT, and P20's flags, "SET UPDATFLG, TRACKFLG"
 * (P20FLGON). */
void p35_p75a(void)
{
    dxch(ADDRESS_OF(KT));
    tc_to(P35_P75A + 1u, P20FLGON);
}

/* P35/P75A +2: to the interpreter, for P35_CODE. */
void p35_p75a_after_p20flgon(void)
{
    tc_at(P35_P75A + 2u);
    intpret();
}

/* P35_CODE: SELECTMU, the Earth's or the Moon's. P35/P75B: the time now
 * (LOADTIME) to TSTRT; plus KT, to TIG and to INTIME, "FOR INITVEL";
 * both vehicles there (PRECSET); S34/35.1, the normal and the line of
 * sight; S34/35.2, the delta V; S34/35.5 and VN1645, the displays; and
 * round again. */
static const word p35_words[] = {
    ORDER(CALL),
    ADDRESS(SELECTMU),
    /* P35/P75B */
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STORE(TSTRT),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(KT),
    STORE(TIG),
    STORE(INTIME), /* "FOR INITVEL" */
    STCALL(TDEC1),
    ADDRESS(PRECSET), /* "ADVANCE BOTH VEHICLES" */
    ORDER(CALL),
    ADDRESS(S34_35_1), /* "GET NORM AND LOS FOR TRANSFORM" */
    ORDER(CALL),
    ADDRESS(S34_35_2), /* "GET DELTA V(LV)" */
    ORDER(CALL),
    ADDRESS(S34_35_5),
    ORDER(CALL),
    ADDRESS(VN1645),
    ORDER(GOTO),
    ADDRESS(P35_P75B),
};

/* ---- S33/34.1 -- LM pp. 667-671, CM pp. 469-473 ---- */

/* S33/34.1: the return to NORMEX; TITER, the iteration count, at 40000
 * (negative: the first pass); SECMAX, the largest step in time (MAX250);
 * and both vehicles' states at TTPI kept (RAPREC, VAPREC, RPPREC,
 * VPPREC). ELCALC: EA, the elevation angle of the line of sight at TTPI,
 * from S34/35.1's normal and line of sight (ULOS); with the line of
 * sight below the horizontal (ULOS.RACT3 negative), a full turn less
 * it. TESTY: with ITSWICH clear (the time given), on to ELEX. Otherwise
 * the last DELEL to DELELO, and DELEL, EA less ELEV; within ELEPS of
 * zero, done (TIMEX). An order whose address the listing leaves out
 * takes its operand from the push-down list. */
static const word s33_34_1_words[] = {
    ORDERS(STQ, SSP),
    ADDRESS(NORMEX),
    ADDRESS_PLUS_ONE(TITER),
    NUMBER(040000),
    ORDERS(DLOAD, SETPD),
    ADDRESS_PLUS_ONE(MAX250),
    ADDRESS_PLUS_ONE_D(0),
    STOVL(SECMAX),
    ADDRESS_PLUS_ONE(RACT3),
    STOVL(RAPREC),
    ADDRESS_PLUS_ONE(VACT3),
    STOVL(VAPREC),
    ADDRESS_PLUS_ONE(RPASS3),
    STOVL(RPPREC),
    ADDRESS_PLUS_ONE(VPASS3),
    STORE(VPPREC),
    /* ELCALC */
    ORDER(CALL),
    ADDRESS(S34_35_1), /* "NORMAL AND LOS" */
    ORDERS(VXV, PDVL),
    ADDRESS_PLUS_ONE(RACT3), /* "(RA*VA)*RA 0D" */
    ORDERS(PDVL, UNIT),      /* "ULOS AT 6D" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PDVL, VPROJ), /* "XCHNJ AND UP" */
    ORDERS(VSL2, BVSU),
    ADDRESS_PLUS_ONE(ULOS),
    ORDERS(UNIT, PDVL),    /* "UP AT 0D" */
    ORDERS(DOT, PDVL),     /* "UP.UN*RA AT 0D" */
    ADDRESS_PLUS_ONE_D(0), /* "UP IN MPAC" */
    ORDERS(DOT, SIGN),
    ADDRESS_PLUS_ONE(ULOS),
    ORDERS(SL1, ACOS),
    ORDERS(PDVL, DOT), /* "EA AT 0D" */
    ADDRESS_PLUS_ONE(ULOS),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(BPL, DLOAD),
    ADDRESS(TESTY),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ORDERS(DSU, PUSH),
    /* TESTY */
    ORDERS(BOFF, DLOAD),
    SWITCH(BOF, ITSWICH),
    ADDRESS(ELEX),
    ADDRESS_PLUS_ONE(DELEL),
    STODL(DELELO),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(ELEV),
    STORE(DELEL),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(ELEPS),
    ORDER(BMN),
    ADDRESS(TIMEX), /* "COMMERCIALS EVERYWHERE" */
};

/* FIGTIME: TITER halved (SR1); once it is zero, too many passes, and
 * back (BHIZ NORMEX). Then the step in time for the next pass: from the
 * vehicles' radii and their difference (30D), the angle the active
 * vehicle must move through for the line of sight to reach ELEV (back by
 * NORMEX where there is none), over the difference of the vehicles'
 * angular rates (WA less WP, normalized: X1 the shift), as a fraction of
 * a turn (TWOPI) unnormalized again (SL*); at most SECMAX, signed. */
static const word figtime_words[] = {
    ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(TITER),
    ORDERS(BHIZ, LXA_1),
    ADDRESS(NORMEX), /* "TOO MANY ITERATIONS" */
    ADDRESS(MPAC),
    ORDERS(SXA_1, VLOAD),
    ADDRESS(TITER),
    ADDRESS_PLUS_ONE(RPASS3),
    ORDERS(UNIT, PDDL),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(RACT3),
    ORDER(PDDL),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE_D(36),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE_D(12),
    STODL_D(30), /* the radii's difference */
    ADDRESS_PLUS_ONE(DPHALF),
    ORDERS(DSU, PUSH),
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(SIGN, BMN),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS(NORMEX),
    ORDERS(DLOAD, COS),
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE_D(14),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(DCOMP), /* "SINCE COS(180-A)=-COS A" */
    STORE_D(28),
    ORDERS(ABS, BDSU),
    ADDRESS_PLUS_ONE(DPHALF),
    ORDERS(BMN, VLOAD),
    ADDRESS(NORMEX),
    ADDRESS_PLUS_ONE(UNRM),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE_D(6), /* "UN*RA" */
    ORDERS(DOT, DMP),
    ADDRESS_PLUS_ONE(VACT3),
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(PDVL, VXV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VPASS3),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE_D(0), /* "(RP*VP)*RP" */
    ORDERS(DOT, DMP),
    ADDRESS_PLUS_ONE(VPASS3),
    ADDRESS_PLUS_ONE_D(14),
    ORDER(BDSU),
    ORDERS(NORM, PDVL), /* the rates' difference, normalized, at 12D */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(UNRM), /* "RA*RP.UN 14D" */
    ORDERS(PDVL, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(SL1, ACOS),
    ORDER(SIGN),
    ORDERS(DSU, DAD), /* "ALPHA PI" */
    ADDRESS_PLUS_ONE(DPHALF),
    ADDRESS_PLUS_ONE(ELEV),
    ORDERS(PDDL, ACOS),
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(BDSU, SIGN),
    ADDRESS_PLUS_ONE(DPHALF),
    ADDRESS_PLUS_ONE_D(30), /* "CONTAINS RP-RA" */
    ORDER(DAD),
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(TWOPI),
    ORDER(DMP),
    ORDERS(SL_STAR, DMP),
    SL_X1(-3),
    ORDERS(PUSH, ABS),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(SECMAX),
    ADDRESS(OKMAX),
    ORDERS(DLOAD, SIGN), /* "REPLACE TIME WITH MAX TIME SIGNED" */
    ADDRESS_PLUS_ONE(SECMAX),
    ORDER(PUSH),
};

/* OKMAX: on the first pass (TITER negative, "TEST FIRST ITERATION"),
 * TITER to 37777 and the step taken as it is. REPETE: when the last pass
 * crossed the angle (DELEL and DELELO of opposite signs), SECMAX a third
 * of what it was, and the step half as long, against the last one's
 * sign. NEXTES: when the last pass took the angle further away (REVERS,
 * "WRONG DIRECTION"), back past the last point by half the last step;
 * otherwise (RESIGN) the step with the last one's sign. STORDELT: the
 * step to DELTEEO. ADTIME: NOMTPI, the sum of the steps, plus it, both
 * vehicles from their kept states to TTPI plus NOMTPI (GOINT, then
 * ACTIVE and PASSIVE), and ELCALC again. ELEX: TTPI plus NOMTPI the new
 * TTPI, and with ETPIFLAG clear EA to ELEV. TIMEX: back by NORMEX with
 * zero in MPAC. */
static const word okmax_words[] = {
    ORDERS(SLOAD, BPL), /* "TEST FIRST ITERATION" */
    ADDRESS_PLUS_ONE(TITER),
    ADDRESS(REPETE),
    ORDERS(SSP, DLOAD),
    ADDRESS_PLUS_ONE(TITER),
    NUMBER(037777),
    ORDER(GOTO),
    ADDRESS(STORDELT),
    /* REPETE */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(DELEL),
    ADDRESS_PLUS_ONE(DELELO),
    ORDERS(BPL, DLOAD),
    ADDRESS(NEXTES),
    ADDRESS_PLUS_ONE(SECMAX),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(THIRD),
    STODL(SECMAX),
    ORDERS(ABS, SR1),    /* "CROSSED OVER SOLUTION" */
    ORDERS(DCOMP, GOTO), /* "DT=(-SIGN(DTO)//DT//)/2" */
    ADDRESS(RESIGN),
    /* NEXTES */
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(DELEL),
    ORDERS(PDDL, ABS),
    ADDRESS_PLUS_ONE(DELELO),
    ORDER(DSU),
    ORDERS(BMN, DLOAD),
    ADDRESS(REVERS), /* "WRONG DIRECTION" */
    ORDER(ABS),
    /* RESIGN */
    ORDERS(SIGN, GOTO),
    ADDRESS_PLUS_ONE(DELTEEO),
    ADDRESS(STORDELT),
    /* REVERS */
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(DELTEEO),
    ORDERS(PUSH, SR1),
    STORE(DELTEEO),
    ORDER(DAD),
    ORDER(GOTO),
    ADDRESS(ADTIME),
    /* STORDELT */
    STORE(DELTEEO),
    /* ADTIME */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(NOMTPI), /* the sum of the steps */
    STORE(NOMTPI),
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(VAPREC),
    ADDRESS_PLUS_ONE(RAPREC),
    ORDER(CALL),
    ADDRESS(GOINT),
    ORDER(CALL),
    ADDRESS(ACTIVE), /* "STORE NEW RACT3 VACT3" */
    ORDERS(VLOAD, PDVL),
    ADDRESS_PLUS_ONE(VPPREC),
    ADDRESS_PLUS_ONE(RPPREC),
    ORDER(CALL),
    ADDRESS(GOINT),
    ORDER(CALL),
    ADDRESS(PASSIVE), /* "STORE NEW RPASS3 VPASS3" */
    ORDER(GOTO),
    ADDRESS(ELCALC),
    /* ELEX */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(TTPI),
    ADDRESS_PLUS_ONE(NOMTPI),
    STODL(TTPI),
    ORDER(BON),
    SWITCH(BON, ETPIFLAG),
    ADDRESS(TIMEX),
    STORE(ELEV),
    /* TIMEX */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ADDRESS(NORMEX),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P35_CODE = {035u, P35_P75A + 3u, COUNT(p35_words), p35_words};
const struct interpretive_code S33_34_1_CODE = {035u, I_S33_34_1 % 02000u + 02000u,
                                                COUNT(s33_34_1_words), s33_34_1_words};
const struct interpretive_code FIGTIME_CODE = {035u, I_FIGTIME % 02000u + 02000u,
                                               COUNT(figtime_words), figtime_words};
const struct interpretive_code OKMAX_CODE = {035u, I_OKMAX % 02000u + 02000u, COUNT(okmax_words),
                                             okmax_words};
