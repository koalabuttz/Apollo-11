/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 3 (Luminary099 pp. 672-677, Comanche055 pp.
 * 474-479): the subroutines the TPI and mid-course programs share, all
 * interpretive code, in the listing's notation (src/interpreter/
 * notation.h), which the readable interpreter runs (src/binding/
 * p34_routines.c). The faithful C is c64-port/src/lm_p34_p35.c and
 * cm_p34_p35.c.
 *
 * S34/35.1, the unit normal to the active vehicle's orbit and the line of
 * sight; S34/35.2, the velocity the active vehicle needs at TIG to reach
 * the passive vehicle at TPASS4 (INITVEL, Lambert), and its delta V in
 * local vertical coordinates; S34/35.3, the target again from a delta V
 * the astronaut keyed in, and S34/35.4, without one, both
 * ending at NOVRWRT, the delta V along the line of sight; LOMAT, the
 * local vertical matrix; GOINT and INTINT, a state vector integrated to
 * a time. The missions' code is the same but for one word of S34/35.2,
 * and the CM's S34/35.1 is in bank 36. */
#include "p34_p35.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

#if LUMINARY099
#define S34_35_1_BANK 035u
#else
#define S34_35_1_BANK 036u
#endif

/* ---- S34/35.1 -- LM p. 672, CM p. 474 ---- */

/* S34/35.1: ULOS, the unit line of sight from the active vehicle to the
 * passive one at T3 (RPASS3 less RACT3, its length on the push-down
 * list); UNRM, the unit normal to the active vehicle's orbit (RACT3 x
 * VACT3). */
static const word s34_35_1_words[] = {
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(RPASS3),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(UNIT, PUSH),
    STOVL(ULOS),
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VACT3),
    STORE(UNRM),
    ORDER(RVQ),
};

/* ---- S34/35.2 -- LM pp. 673-674, CM pp. 475-476 ---- */

/* S34/35.2: "ADVANCE PASSIVE VEH TO RENDEZVOUS TIME AND GET REQ VEL FROM
 * LAMBERT". The passive vehicle from INTIME to TPASS4 by INTINT: by
 * conic (TWOPI) when NN, the number of offsets, is zero, else with the
 * precision integration (ZEROVECS). RTARG, the target, and VPASS4 its
 * velocity there. ACTCENT, the active vehicle's central angle of
 * transfer: the angle from UNIT RA to UNIT RP, past 180 degrees when
 * RA*RP.U is negative. DELLT4, the time of transfer; INITVEL (with NN
 * and EPSFOUR) for the velocity at TIG, and its delta V DELVEET3 turned
 * into local vertical coordinates (LOMAT's matrix), DELVLVC. */
static const word s34_35_2_words[] = {
    ORDERS(STQ, VLOAD),
    ADDRESS(SUBEXIT),
    ADDRESS_PLUS_ONE(VPASS3),
    ORDERS(PDVL, PDDL),
    ADDRESS_PLUS_ONE(RPASS3),
    ADDRESS_PLUS_ONE(INTIME),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TPASS4),
    ADDRESS_PLUS_ONE(TWOPI), /* "CONIC" */
    ORDERS(PDDL, BHIZ),
    ADDRESS_PLUS_ONE(NN),
    ADDRESS(S3435_23),
#if LUMINARY099
    ORDER(DLOAD),
    ORDERS(DLOAD, PUSH),
#else
    ORDERS(DLOAD, PDDL),
#endif
    ADDRESS_PLUS_ONE(ZEROVECS), /* "PRECISION" */
    /* S3435.23 */
    ORDER(CALL),
    ADDRESS(INTINT), /* "GET TARGET VECTOR" */
    /* S3435.25 */
    STOVL(RTARG),
    ADDRESS_PLUS_ONE(VATT),
    STOVL(VPASS4),
    ADDRESS_PLUS_ONE(RTARG),
    ORDERS(UNIT, PDVL), /* "UNIT RP" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(UNIT, PUSH), /* "UNIT RA" */
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(UNRM), /* "RA*RP.U" */
    ORDER(PDVL),
    ORDERS(DOT, SL1), /* "UNIT RA.UNIT RP" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(ACOS, SIGN),
    ORDERS(BPL, DAD),
    ADDRESS(NOPIE),
    ADDRESS_PLUS_ONE(DPPOSMAX), /* "REASONABLE TWO PI" */
    /* NOPIE */
    STODL(ACTCENT),
    ADDRESS_PLUS_ONE(TPASS4),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(INTIME),
    STORE(DELLT4),
    ORDERS(SLOAD, SETPD),
    ADDRESS_PLUS_ONE(NN), /* "NUMBER OF OFFSETS" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PDDL, PDVL),
    ADDRESS_PLUS_ONE(EPSFOUR),
    ADDRESS_PLUS_ONE(RACT3),
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VACT3),
    STCALL(VINIT),
    ADDRESS(INITVEL),
    ORDER(CALL),
    ADDRESS(LOMAT),
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(DELVEET3),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(VSL1),
    STCALL(DELVLVC),
    ADDRESS(SUBEXIT),
};

/* ---- S34/35.3 and S34/35.4 -- LM pp. 675-676, CM pp. 477-478 ---- */

/* S34/35.3: DELVLVC, the delta V the astronaut keyed in, turned back by
 * LOMAT's matrix into DELVEET3 ("SAVE FOR TRANSFORM"); the active
 * vehicle with that velocity added, integrated from TIG to TPASS4
 * (INTINT) for RTARG, the new target. NOVRWRT: DVLOS, the delta V in the
 * frame of the line of sight (ULOS, UNRM x ULOS and their cross
 * product). S34/35.4, "NO ASTRONAUT OVERWRITE", goes to NOVRWRT with the
 * push-down list at 0D. */
static const word s34_35_3_words[] = {
    ORDERS(STQ, CALL),
    ADDRESS(NORMEX),
    ADDRESS(LOMAT), /* "GET MATRIX IN PUSH LIST" */
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(DELVLVC), /* "NEW DEL V TPI" */
    ADDRESS_PLUS_ONE_D(0),
    ORDER(VSL1),
    STORE(DELVEET3), /* "SAVE FOR TRANSFORM" */
    ORDERS(VAD, PDVL),
    ADDRESS_PLUS_ONE(VACT3), /* "NEW V REQ" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TIG),
    ADDRESS_PLUS_ONE(TPASS4),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ORDER(CALL), /* "INTEG. FOR NEW TARGET VEC" */
    ADDRESS(INTINT),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RTARG),
    /* NOVRWRT */
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(ULOS),
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE(UNRM),
    ORDERS(UNIT, PUSH),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(ULOS),
    ORDER(PDVL),
    ORDERS(PDVL, MXV),
    ADDRESS_PLUS_ONE(DELVEET3),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(VSL1),
    STCALL(DVLOS),
    ADDRESS(NORMEX),
    /* S34/35.4 */
    ORDERS(STQ, SETPD), /* "NO ASTRONAUT OVERWRITE" */
    ADDRESS(NORMEX),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(GOTO),
    ADDRESS(NOVRWRT),
};

/* ---- LOMAT, GOINT and INTINT -- LM p. 677, CM p. 479 ---- */

/* LOMAT: the local vertical matrix on the push-down list, its rows Z*-Y
 * (0D), Y, the negative unit normal (6D), and the negative unit radius
 * (12D); the list at 18D after it. GOINT: a state vector on the list
 * from S33/34.1, to TTPI plus NOMTPI by precision integration (ZEROVECS),
 * then on into INTINT, which must follow it. INTINT: the state on the list
 * integrated to the time there (INTSTALL; INTYPFLG set for a conic, a
 * zero precision word cleared), in the Moon's sphere or the Earth's by
 * RTX2 (MOONFLAG), its vectors shifted by X2 to RCV and VCV, by
 * INTEGRVS; back by RTRN with RATT in MPAC. */
static const word lomat_words[] = {
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(UNRM),
    STOVL_D(6), /* "Y" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(UNIT, VCOMP),
    STORE_D(12),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UNRM), /* "Z*-Y" */
    STORE_D(0),
    ORDERS(SETPD, RVQ),
    ADDRESS_PLUS_ONE_D(18),
    /* GOINT */
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ADDRESS_PLUS_ONE(NOMTPI),
    ORDERS(PUSH, PUSH),
    /* INTINT */
    ORDERS(STQ, CALL),
    ADDRESS(RTRN),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, INTYPFLG),
    ORDERS(BZE, SET),
    ADDRESS_AT(INTINT, 010), /* "+2" */
    SWITCH(SET, INTYPFLG),
    ORDERS(DLOAD, STADR),
    AFTER_STADR(STODL(TDEC1)),
    ORDERS(SET, LXA_2),
    SWITCH(SET, MOONFLAG),
    ADDRESS(RTX2),
    ORDERS(BON, CLEAR),
    SWITCH(BON, CMOONFLG),
    ADDRESS(ALLSET),
    SWITCH(CLEAR, MOONFLAG),
    /* ALLSET */
    STOVL(TET),
    ORDER(VSR_STAR),
    SR_X2(0),
    STOVL(RCV),
    ORDER(VSR_STAR),
    SR_X2(0),
    STCALL(VCV),
    ADDRESS(INTEGRVS),
    ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS(RTRN),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S34_35_1_CODE = {S34_35_1_BANK, IN_BANK(S34_35_1),
                                                COUNT(s34_35_1_words), s34_35_1_words};
const struct interpretive_code S34_35_2_CODE = {035u, IN_BANK(S34_35_2), COUNT(s34_35_2_words),
                                                s34_35_2_words};
const struct interpretive_code S34_35_3_CODE = {035u, IN_BANK(S34_35_3), COUNT(s34_35_3_words),
                                                s34_35_3_words};
const struct interpretive_code LOMAT_CODE = {035u, IN_BANK(LOMAT), COUNT(lomat_words), lomat_words};
