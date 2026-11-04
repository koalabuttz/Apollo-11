/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 5 (Luminary099 pp. 694-699, Comanche055 pp.
 * 495-500): PERIAPO, the two-body apocenter and pericenter; PRECSET, both
 * vehicles' states at a time; VECSHIFT and SHIFTR1, the scale shifts of
 * the primary body's sphere. In bank 23 of the LM and bank 22 of the CM.
 * All interpretive code, in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/p34_routines.c). The faithful C is
 * c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * The scales follow the primary body: positions +29 and velocities +7 in
 * the Earth's sphere, +27 and +5 in the Moon's. RTX1 and RTX2, which
 * SELECTMU sets (midgim.c), say which: -2 and 0 for the Earth, -10D and 2
 * for the Moon. An index register is subtracted from an address, so a
 * shift "0,2" is X2 places the other way. */
#include "p34_p35.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's bank, and its address in the bank (02000-03777). */
#define BANK_OF(label) ((word)(I_##label / 02000u))
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* PERIAPO1: the velocity in MPAC and the position on the push-down list
 * (the caller's PDVL then CALL), each shifted by RTX2 to the sphere's scale, to
 * VVEC and RVEC.
 *
 * PERIAPO: "THIS SUBROUTINE COMPUTES THE TWO BODY APOCENTER AND PERICENTER
 * ALTITUDES GIVEN THE POSITION AND VELOCITY VECTORS FOR A POINT ON
 * TRAJECTORY AND THE PRIMARY BODY." The return to NORMEX; SETRAD, the
 * primary body's radius, to XXXALT; APSIDES, the apocenter radius (in
 * MPAC) and the pericenter's (0D); then from 2D the push-down list holds
 * the apocenter radius, its altitude (less XXXALT), the pericenter radius
 * and its altitude, which MPAC keeps too.
 *
 * SETRAD: RPAD ("STANDARD RADIUS OF PAD 37-B.") on the list; X1 to X2 and
 * 2 more, zero for the Earth (RTX1 -2). In the Moon's sphere the landing
 * site's radius, the length of RLS: PDDL pops RPAD, pushes it and loads
 * RPAD, so SETRADX's DLOAD pops the landing site's. SETRADX: the radius
 * loaded from the list, and back. */
static const word periapo_words[] = {
    ORDERS(LXA_2, VSR_STAR),
    ADDRESS(RTX2),
    SR_X2(0),
    STOVL(VVEC),
    ORDERS(LXA_1, VSR_STAR),
    ADDRESS(RTX1),
    SR_X2(0),
    STORE(RVEC),
    /* PERIAPO */
    ORDERS(STQ, CALL),
    ADDRESS(NORMEX),
    ADDRESS(SETRAD),
    STCALL(XXXALT),
    ADDRESS(APSIDES),
    ORDERS(SETPD, PUSH), /* "2D = APOCENTER RADIUS" */
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DSU, PDDL), /* "4D = APOGEE ALTITUDE" */
    ADDRESS_PLUS_ONE(XXXALT),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PUSH, DSU), /* "6D = PERICENTER RADIUS" */
    ADDRESS_PLUS_ONE(XXXALT),
    ORDERS(PUSH, GOTO), /* "8D = PERIGEE ALTITUDE" */
    ADDRESS(NORMEX),
    /* SETRAD */
    ORDERS(DLOAD, PUSH),
    ADDRESS_PLUS_ONE(RPAD),
    ORDERS(SXA_1, INCR_2),
    ADDRESS(X2),
    NUMBER(2),
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS(SETRADX),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(RLS),
    ORDER(PDDL),
    /* SETRADX */
    ORDERS(DLOAD, RVQ),
};

/* PRECSET: both vehicles' states at the time in MPAC, by precision
 * integration: the LM's (LEMPREC, to TDEC2) and the CSM's (CSMPREC, to
 * TDEC1), each stored by LEMSTORE or CSMSTORE; the return by NORMEX.
 *
 * LEMSTORE and CSMSTORE: the integrated position (RATT) and velocity
 * (VATT) to the active vehicle's RACT3 and VACT3 (ACTIVE) or the passive
 * one's RPASS3 and VPASS3 (PASSIVE), by AVFLAG (set when the LM is the
 * active vehicle). */
static const word precset_words[] = {
    ORDER(STQ),
    ADDRESS(NORMEX),
    STCALL(TDEC2),
    ADDRESS(LEMPREC),
    ORDER(CALL),
    ADDRESS(LEMSTORE),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TDEC2),
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(CALL),
    ADDRESS(CSMSTORE),
    ORDER(GOTO),
    ADDRESS(NORMEX),
    /* LEMSTORE */
    ORDERS(VLOAD, BOFF),
    ADDRESS_PLUS_ONE(RATT),
    SWITCH(BOF, AVFLAG),
    ADDRESS(PASSIVE),
    /* ACTIVE */
    STOVL(RACT3),
    ADDRESS_PLUS_ONE(VATT),
    STORE(VACT3),
    ORDER(RVQ),
    /* CSMSTORE */
    ORDERS(VLOAD, BOFF),
    ADDRESS_PLUS_ONE(RATT),
    SWITCH(BOF, AVFLAG),
    ADDRESS(ACTIVE),
    /* PASSIVE */
    STOVL(RPASS3),
    ADDRESS_PLUS_ONE(VATT),
    STORE(VPASS3),
    ORDER(RVQ),
};

/* VECSHIFT: the vector in MPAC and the one on the push-down list, each
 * shifted by RTX2 to the sphere's scale, in the same places (each PDVL
 * pops the list's vector and pushes MPAC's). SHIFTR1: MPAC shifted the
 * other way, from the sphere's scale to the Earth's. */
static const word vecshift_words[] = {
    ORDERS(LXA_2, VSR_STAR),
    ADDRESS(RTX2),
    SR_X2(0),
    ORDERS(LXA_1, PDVL),
    ADDRESS(RTX1),
    ORDERS(VSR_STAR, PDVL),
    SR_X2(0),
    ORDER(RVQ),
    /* SHIFTR1 */
    ORDERS(LXA_2, SL_STAR),
    ADDRESS(RTX2),
    VSL_X2(0), /* "0,2", as SL* assembles it: a left shift's */
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code PERIAPO_CODE = {BANK_OF(PERIAPO1), IN_BANK(PERIAPO1),
                                               COUNT(periapo_words), periapo_words};
const struct interpretive_code PRECSET_CODE = {BANK_OF(PRECSET), IN_BANK(PRECSET),
                                               COUNT(precset_words), precset_words};
const struct interpretive_code VECSHIFT_CODE = {BANK_OF(VECSHIFT), IN_BANK(VECSHIFT),
                                                COUNT(vecshift_words), vecshift_words};
