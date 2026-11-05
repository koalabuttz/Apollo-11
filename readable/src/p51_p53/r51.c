/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 3 (Luminary099 pp. 941-944, bank 14): R51, the fine
 * alignment, and GYCOARS, the gyro torque coarse alignment.
 *
 * R51, "TO ALIGN THE STABLE MEMBER TO REFSSMAT": the crew picks (V50N25,
 * option 15) whether the computer picks the stars, then marks two stars
 * or planets with the AOT (R52, AOTMARK), each star's catalog vector or a
 * planet's (PLANET) taken to stable member axes; R54 checks the two
 * against each other, AXISGEN and R55 torque the gyros to REFSMMAT, and
 * the crew can do it again (V50N25, option 14) or go on.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, ALARM, CURTAINS) leaves
 * Z at the routine and Q at the word after the TC, where the routine
 * comes back to; GOTOPOOH doesn't come back. The faithful port keys a
 * routine at each such return, at each TC's return in the file, and at
 * each TC INTPRET's NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r51_4(void);

/* ---- R51 -- pp. 941-943 ---- */

/* R51: the caller's return to QMAJ; then a restart point (PHASCHNG,
 * 04024: on a restart, R51.1's EXIT, back to R51C). */
static const word r51_words[] = {
    /* R51 */ ORDER(STQ),
    ADDRESS(QMAJ),
    /* R51.1 */ ORDER(EXIT),
};

const struct interpretive_code R51_CODE = {014u, R51, COUNT(r51_words), r51_words};

void r51_after_exit(void)
{
    tc_to(R51_1 + 1u, PHASCHNG); /* OCT 04024 */
}

/* R51C: V50N25, option 15 (GOPERF1): the computer picks the stars (V33)
 * or the crew does (ENTER). */
void r51c(void)
{
    set_A(OCT15);
    tc_to(R51C + 1u, BANKCALL); /* CADR GOPERF1 */
}

void r51c_terminate(void)
{
    tc_to(R51C + 3u, GOTOPOOH);
}

/* R51C +4: "V33E". */
void r51c_proceed(void)
{
    tc_at(R51C + 4u); /* TC +2 */
    r51c_pick_stars();
}

/* R51C +5: "ENTER". */
void r51c_enter(void)
{
    tc_at(R51C + 5u);
    r51e();
}

/* R51C +6: the sun, the earth and the moon six minutes from now (TSIGHT1:
 * "6 MIN TO MARKING", from LOADTIME, LOCSAM); then R56 picks the stars. */
void r51c_pick_stars(void)
{
    tc_at(R51C + 6u);
    intpret();
}

static const word r51c_words[] = {
    /* R51C +7 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TSIGHT1),
    ORDER(CALL),
    ADDRESS(LOCSAM),
    ORDER(EXIT),
};

const struct interpretive_code R51C_CODE = {014u, R51C + 7u, COUNT(r51c_words), r51c_words};

void r51c_after_exit(void)
{
    tc_to(R51C + 015u, BANKCALL); /* CADR R56 */
}

/* R56's returns: +2, no pair of stars in the field of view (R51I); +3, a
 * pair, on to the marks (R51F). */
void r51c_no_stars(void)
{
    tc_at(R51F - 1u);
    r51i();
}

void r51f(void)
{
    tc_at(R51F);
    r51e();
}

/* R51I: alarm 405, then V05N09 flashed: terminate, proceed (the crew
 * picks the stars, R51E) or enter (try again, R51C). */
void r51i(void)
{
    tc_to(R51I, ALARM); /* OCT 405 */
}

void r51i_after_alarm(void)
{
    set_A(VB05N09);
    tc_to(R51I + 3u, BANKCALL); /* CADR GOFLASH */
}

void r51i_terminate(void)
{
    tc_to(R51I + 5u, GOTOPOOH);
}

void r51i_proceed(void)
{
    tc_at(R51I + 6u);
    r51e();
}

void r51i_enter(void)
{
    tc_at(R51I + 7u);
    r51c();
}

/* R51E: STARIND +0, the first of the two marks; then (R51.3) a restart
 * point. */
void r51e(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(STARIND));
    tc_at(R51_2);
    intpret();
}

static const word r51_3_words[] = {
    /* R51.3 */ ORDER(EXIT),
};

const struct interpretive_code R51_3_CODE = {014u, R51_3, COUNT(r51_3_words), r51_3_words};

void r51_3_after_exit(void)
{
    tc_to(R51_3 + 1u, PHASCHNG); /* OCT 04024 */
}

void r51_3_restart(void)
{
    tc_at(R51_3 + 3u);
    intpret();
}

/* The mark: R52 picks the star or planet ("AOP WILL MAKE CALLS TO
 * SIGHTING"), AOTMARK takes the marks, OPTSTALL waits for them (CURTAINS
 * when they fail). */
static const word r52_words[] = {
    /* R51.3 +4 */ ORDER(CALL),
    ADDRESS(R52),
    ORDER(EXIT),
};

const struct interpretive_code R51_MARK_CODE = {014u, R51_3 + 4u, COUNT(r52_words), r52_words};

void r51_mark_after_exit(void)
{
    tc_to(R51_3 + 7u, BANKCALL); /* CADR AOTMARK */
}

void r51_mark_after_aotmark(void)
{
    tc_to(R51_3 + 011u, BANKCALL); /* CADR OPTSTALL */
}

void r51_mark_failed(void)
{
    tc_to(R51_3 + 013u, CURTAINS);
}

/* The marks taken: STARIND +0 after the first (R51.4 keeps it and marks
 * again, STARIND 1), positive after the second. */
void r51_marked(void)
{
    switch (ccs_at(ADDRESS_OF(STARIND))) {
    case POSITIVE: r51_both_marked(); break; /* TCF +2 */
    case IS_PLUS_ZERO:
        tc_at(R51_3 + 016u); /* TC R51.4 */
        r51_4();
        break;
    case NEGATIVE: r51_both_marked(); break;
    case IS_MINUS_ZERO: not_modelled(); /* (its word is interpretive code) */
    }
}

/* R51.3 +017: the second star to STARSAV2; then a restart point. */
void r51_both_marked(void)
{
    tc_at(R51_3 + 017u);
    intpret();
}

static const word r51_second_words[] = {
    /* R51.3 +020 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(STARAD, 6),
    STORE(STARSAV2),
    ORDER(EXIT),
};

const struct interpretive_code R51_SECOND_CODE = {014u, R51_3 + 020u, COUNT(r51_second_words),
                                                  r51_second_words};

void r51_second_after_exit(void)
{
    tc_to(R51_3 + 024u, PHASCHNG); /* OCT 04024 */
}

void r51_second_restart(void)
{
    tc_at(R51_3 + 026u);
    intpret();
}

/* The two directions in stable member axes (the planet's at the mark,
 * PLANET, to STARAD +6; STARAD the first), the stars' in reference axes
 * (STARSAV1, STARSAV2) to 6D and 12D; R54 tests them ("STAR DATA TEST").
 * With FREEFLAG set after it, AXISGEN, the stable member's axes, and R55,
 * the torquing ("GYRO TORQUE"); with it clear, on to R51K. */
static const word r51_torque_words[] = {
    /* R51.3 +027 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL_AT(STARAD, 6),
    ADDRESS_PLUS_ONE(PLANVEC),
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE(STARSAV1),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(STARSAV2),
    STCALL_D(12),
    ADDRESS(R54), /* "STAR DATA TEST" */
    ORDERS(BOFF, CALL),
    SWITCH(BOFF, FREEFLAG),
    ADDRESS(R51K),
    ADDRESS(AXISGEN),
    ORDER(CALL),
    ADDRESS(R55), /* "GYRO TORQUE" */
    ORDER(CLEAR),
    SWITCH(CLEAR, PFRATFLG),
    /* R51K */ ORDER(EXIT),
};

const struct interpretive_code R51_TORQUE_CODE = {014u, R51_3 + 027u, COUNT(r51_torque_words),
                                                  r51_torque_words};

/* R51P63: V50N25, option 14: terminate, proceed (back to the caller,
 * QMAJ) or enter (align again, R51C). */
void r51p63(void)
{
    set_A(OCT14);
    tc_to(R51P63 + 1u, BANKCALL); /* CADR GOPERF1 */
}

void r51p63_terminate(void)
{
    tc_to(R51P63 + 3u, GOTOPOOH);
}

void r51p63_enter(void)
{
    tc_at(R51P63 + 4u);
    r51c();
}

void r51p63_proceed(void)
{
    tc_at(R51P63 + 5u);
    intpret();
}

static const word r51p63_words[] = {
    /* R51P63 +6 */ ORDER(GOTO),
    ADDRESS(QMAJ),
};

const struct interpretive_code R51P63_CODE = {014u, R51P63 + 6u, COUNT(r51p63_words), r51p63_words};

/* R51.4: the first star (STARSAV1) and, at the mark, the planet's
 * direction (PLANVEC); STARIND 1, and the second mark. */
static void r51_4(void)
{
    tc_at(R51_4);
    intpret();
}

static const word r51_4_words[] = {
    /* R51.4 +1 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(STARAD, 6),
    STORE(STARSAV1),
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    STORE(PLANVEC),
    ORDER(SSP),
    ADDRESS_PLUS_ONE(STARIND),
    NUMBER(1),
    ORDER(GOTO),
    ADDRESS(R51_3),
};

const struct interpretive_code R51_4_CODE = {014u, R51_4 + 1u, COUNT(r51_4_words), r51_4_words};

/* ---- GYCOARS -- p. 944 ---- */

/* GYCOARS, the "GYRO TORQUE COARSE ALGNMENT": the gyro torque angles to
 * the new orientation (CALCGTA), DRIFTFLG and REFSMFLG cleared; then the
 * gimbals shown ("MONITOR GIMBALS", V16N20) while IMUPULSE torques the
 * gyros (OGC, R55CDR) and IMUSTALL waits (CURTAINS when it fails). */
static const word gycoars_words[] = {
    /* GYCOARS */ ORDERS(STQ, CALL),
    ADDRESS(QMAJ),
    ADDRESS(CALCGTA),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DRIFTFLG),
    SWITCH(CLEAR, REFSMFLG),
    ORDER(EXIT),
};

const struct interpretive_code GYCOARS_CODE = {014u, GYCOARS, COUNT(gycoars_words), gycoars_words};

void gycoars_after_exit(void)
{
    set_A(V16N20);
    tc_to(GYCOARS + 010u, BANKCALL); /* CADR GODSPR */
}

void gycoars_torque(void)
{
    set_A(R55CDR);
    tc_to(GYCOARS + 013u, BANKCALL); /* CADR IMUPULSE */
}

void gycoars_after_imupulse(void)
{
    tc_to(GYCOARS + 015u, BANKCALL); /* CADR IMUSTALL */
}

void gycoars_failed(void)
{
    tc_to(GYCOARS + 017u, CURTAINS);
}

void gycoars_torqued(void)
{
    tc_to(GYCOARS + 020u, PHASCHNG); /* OCT 04024 */
}

void gycoars_restart(void)
{
    tc_at(GYCOARS + 022u);
    intpret();
}

/* The new orientation (XSMD) to REFSMMAT ("STORE DESIRED REFSMMAT",
 * MATMOVE), PFRATFLG cleared and REFSMFLG set; NCOARSE ("SET DRIFT AND
 * INITIALIZE 1/PIPADT"); and on as R51 ends (R51K). */
static const word gycoars_done_words[] = {
    /* GYCOARS +023 */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSMD),
    ADDRESS(REFSMMAT),
    ORDER(CALL), /* "STORE DESIRED REFSMMAT" */
    ADDRESS(MATMOVE),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, PFRATFLG),
    SWITCH(SET, REFSMFLG),
    ORDER(CALL),
    ADDRESS(NCOARSE), /* "SET DRIFT AND INITIALIZE 1/PIPADT" */
    ORDER(GOTO),
    ADDRESS(R51K),
};

const struct interpretive_code GYCOARS_DONE_CODE = {014u, GYCOARS + 023u, COUNT(gycoars_done_words),
                                                    gycoars_done_words};

#endif
