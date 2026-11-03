/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 19 (Comanche055 p. 631): LOWMEMRY and LDPLANET, the
 * star or planet P23 marks on (banks 14 and 16).
 *
 * LOWMEMRY loads a star's unit vector from the star catalog: CATLOG less
 * X1, which is the star's number times 6, and returns (RVQ).
 *
 * LDPLANET is used when P23 marks on a planet instead of a stored star
 * ("LDPLANET ALLOWS VECTOR TO PLANET TO BE" "STORED IN STARSAV2 IF STORED
 * STARS ARE" "NOT VISIBLE"). It flashes verb 06 noun 88 (VNPLAN23) by
 * GOFLASH for the astronaut to load the planet's direction (STARSAV3):
 * terminate ends P23 (GOTOPOOH); proceed takes the direction as loaded,
 * times 1/SQR3, makes it a unit vector, stores it in STARSAV2 and goes back
 * to P23 (P23.31); enter flashes the display again. ("KEEP THIS OPEN
 * SUBROUTINE IN EBANK=5" "BECAUSE STAR IS EBANK=5".)
 *
 * The call out of the file (BANKCALL) leaves Z at the routine and Q at
 * the word after the TC; GOFLASH comes back to the word after its CADR on
 * terminate, the next on proceed, and the next on enter. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* LOWMEMRY: the star's vector, CATLOG less X1. */
static const word lowmemry_words[] = {
    ORDERS(VLOAD_STAR, RVQ),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
};

/* LDPLANET: EXIT, to basic code. */
static const word ldplanet_words[] = {
    ORDER(EXIT),
};

/* LDPLANET +1 -- after the EXIT: verb 06 noun 88 flashed by GOFLASH. */
void ldplanet_after_exit(void)
{
    set_A(VNPLAN23);
    tc_to(LDPLANET + 2u, BANKCALL); /* CADR GOFLASH */
}

/* LDPLANET +4: terminate. */
void ldplanet_terminate(void)
{
    tc_to(LDPLANET + 4u, GOTOPOOH);
}

/* LDPLANET +7: TC INTPRET. */
void ldplanet_intpret(void)
{
    tc_at(LDPLANET + 7u);
    intpret();
}

/* LDPLANET +5: proceed: TC +2, to the TC INTPRET. */
void ldplanet_proceed(void)
{
    tc_at(LDPLANET + 5u);
    ldplanet_intpret();
}

/* LDPLANET +6: enter: TC -5, the display again. */
void ldplanet_enter(void)
{
    tc_at(LDPLANET + 6u);
    ldplanet_after_exit();
}

/* LDPLANET +010: the loaded direction a unit vector, to STARSAV2, and
 * back to P23. */
static const word ldplanet_load_words[] = {
    ORDER(VLOAD),               /* the direction */
    ADDRESS_PLUS_ONE(STARSAV3), /* noun 88 loaded, */
    ORDERS(VXSC, UNIT),         /* times */
    ADDRESS_PLUS_ONE(1_SQR3),   /* 1/SQR3, a unit vector, */
    STORE(STARSAV2),            /* to STARSAV2, */
    ORDER(GOTO),                /* and back */
    ADDRESS(P23_31),            /* to P23 */
};

/* ---- The interpretive code ---- */

const struct interpretive_code LOWMEMRY_CODE = {014u, LOWMEMRY, COUNT(lowmemry_words),
                                                lowmemry_words};
const struct interpretive_code LDPLANET_CODE = {016u, LDPLANET, COUNT(ldplanet_words),
                                                ldplanet_words};
const struct interpretive_code LDPLANET_LOAD_CODE = {
    016u, LDPLANET + 010u, COUNT(ldplanet_load_words), ldplanet_load_words};

#endif
