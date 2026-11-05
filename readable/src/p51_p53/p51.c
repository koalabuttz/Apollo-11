/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 5 (Luminary099 pp. 949-954, bank 14): P51, the IMU
 * orientation determination; COARSE and NCOARSE, its coarse alignment;
 * S52.2, the gimbal angles; and S52.3, the nominal orientation.
 *
 * P51 "DETERMINES THE INERTIAL ORIENTATION OF THE IMU." The crew coarse
 * aligns the IMU to zero gimbal angles (V41) if they like, then sights two
 * stars or planets with the AOT (AOTMARK, R53); CHKSDATA (R54) checks the
 * angle between them, and AXISGEN makes REFSMMAT from the two sightings.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, CURTAINS, GOTOPOOH)
 * leaves Z at the routine and Q at the word after the TC, where the
 * routine comes back to; GOTOPOOH doesn't come back. The faithful port
 * keys a routine at each such return, at each TC's return in the file,
 * and at each TC INTPRET's NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p51b(void);
static void p51e(void);

/* ---- P51 -- pp. 949-951 ---- */

/* P51: "IS ISS ON" (IMUCHK, which alarms and goes to GOTOPOOH if not). */
void p51(void)
{
    tc_to(P51, BANKCALL); /* CADR IMUCHK */
}

/* P51 +2: V50N25 with option 15 (GOPERF1, OCT15): terminate, proceed
 * (V33: on to the sightings, P51B) or enter (a coarse alignment to zero
 * gimbal angles first, then this flash again). */
void p51_plus2(void)
{
    set_A(OCT15);
    tc_to(P51 + 3u, BANKCALL); /* CADR GOPERF1 */
}

void p51_terminate(void)
{
    tc_to(P51 + 5u, GOTOPOOH); /* "TERM." */
}

void p51_proceed(void)
{
    p51b(); /* TCF P51B, "V33" */
}

/* The coarse alignment, after a restart point (04024). */
void p51_enter(void)
{
    tc_to(P51 + 7u, PHASCHNG); /* OCT 04024 */
}

/* "ZERO THE GIMBALS": the desired angles (THETAD) to zero, shown with
 * V06N22, then "NOW DISPLAY COARSE ALIGN VERB 41". (A is +0: TS does not
 * skip.) */
void p51_zero_gimbals(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(THETAD));
    ts(ADDRESS_OF(THETAD) + 1u);
    ts(ADDRESS_OF(THETAD) + 2u);
    set_A(V06N22);
    tc_to(P51 + 016u, BANKCALL); /* CADR GODSPRET */
}

void p51_display_v41(void)
{
    set_A(V41K);
    tc_to(P51 + 021u, BANKCALL); /* CADR GODSPRET */
}

/* The coarse alignment itself (COARSE). */
void p51_coarse(void)
{
    tc_at(P51 + 023u);
    intpret();
}

static const word p51_words[] = {
    /* P51 +024 */ ORDER(CALL),
    ADDRESS(COARSE),
    ORDER(EXIT),
};

const struct interpretive_code P51_CODE = {014u, P51 + 024u, COUNT(p51_words), p51_words};

/* A restart point (04024), then back to the flash (TCF P51 +2). */
void p51_after_coarse(void)
{
    tc_to(P51 + 027u, PHASCHNG); /* OCT 04024 */
}

void p51_again(void)
{
    p51_plus2(); /* TCF P51 +2 */
}

/* P51B: a restart point (00014), then NCOARSE, which zeroes the drift
 * terms; STARIND, "INDEX -- STAR 1 OR 2", to 0, the first star. */
static void p51b(void)
{
    tc_to(P51B, PHASCHNG); /* OCT 00014 */
}

void p51b_ncoarse(void)
{
    tc_at(P51B + 2u);
    intpret();
}

static const word p51b_words[] = {
    /* P51B +3 */ ORDER(CALL), ADDRESS(NCOARSE), ORDERS(SSP, SETPD),
    ADDRESS_PLUS_ONE(STARIND), NUMBER(0),        ADDRESS_PLUS_ONE_D(0),
    /* P51C */ ORDER(EXIT),
};

const struct interpretive_code P51B_CODE = {014u, P51B + 3u, COUNT(p51b_words), p51b_words};

/* P51C +1: a star sighted: a restart point (04024); AOTMARK (R53), the
 * marks, and AOTSTALL waits for them (CURTAINS when it fails). */
void p51c_plus1(void)
{
    tc_to(P51C + 1u, PHASCHNG); /* OCT 04024 */
}

void p51c_aotmark(void)
{
    tc_to(P51C + 3u, BANKCALL); /* CADR AOTMARK, "R53" */
}

void p51c_after_aotmark(void)
{
    tc_to(P51C + 5u, BANKCALL); /* CADR AOTSTALL */
}

void p51c_failed(void)
{
    tc_to(P51C + 7u, CURTAINS);
}

/* The first star (STARIND +0): its line of sight (STARAD +6) to STARSAV1.
 * The second (+1) goes on at P51D +1. (STARIND is 0 or 1: CCS takes its
 * first two ways only.) */
void p51c_star(void)
{
    switch (ccs_at(ADDRESS_OF(STARIND))) {
    case POSITIVE: p51d_plus1(); break; /* TCF P51D +1 */
    case IS_PLUS_ZERO:
        tc_at(P51C + 012u);
        intpret();
        break;
    default: not_modelled();
    }
}

static const word p51c_words[] = {
    /* P51C +013 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(STARAD, 6),
    STORE(STARSAV1),
    /* P51D */ ORDER(EXIT),
};

const struct interpretive_code P51C_CODE = {014u, P51C + 013u, COUNT(p51c_words), p51c_words};

/* P51D +1: a restart point (04024); then the second star on to P51E, the
 * first to a restart point again and the planet's direction (PLANET at
 * the sighting time, TSIGHT, to PLANVEC). (STARIND is 0 or 1; a -0 would
 * take the fourth way, to the TC INTPRET.) */
void p51d_plus1(void)
{
    tc_to(P51D + 1u, PHASCHNG); /* OCT 04024 */
}

void p51d_star(void)
{
    switch (ccs_at(ADDRESS_OF(STARIND))) {
    case POSITIVE: p51e(); break;                         /* TCF P51E */
    case IS_PLUS_ZERO: tc_to(P51D + 5u, PHASCHNG); break; /* OCT 04024 */
    case IS_MINUS_ZERO: p51d_planet(); break;
    default: not_modelled();
    }
}

void p51d_planet(void)
{
    tc_at(P51D + 7u);
    intpret();
}

static const word p51d_words[] = {
    /* P51D +010 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    STORE(PLANVEC),
    ORDER(EXIT),
};

const struct interpretive_code P51D_CODE = {014u, P51D + 010u, COUNT(p51d_words), p51d_words};

/* STARIND to 1: "DO SECOND STAR". (A is 1: TS does not skip.) */
void p51d_second_star(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(STARIND));
    p51c_plus1(); /* TCF P51C +1 */
}

/* P51E: the second star: a restart point (04024); the planet's direction
 * again; the two catalog vectors (PLANVEC and STARSAV1, to 12D and 6D)
 * and the two sightings (STARSAV2 and STARAD +6, to STARAD and STARAD +6)
 * to CHKSDATA, which checks the angles between them. */
static void p51e(void)
{
    tc_to(P51E, PHASCHNG); /* OCT 04024 */
}

void p51e_planet(void)
{
    tc_at(P51E + 2u);
    intpret();
}

static const word p51e_words[] = {
    /* P51E +3 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    STOVL_D(12),
    ADDRESS_PLUS_ONE(PLANVEC),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(STARSAV1),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE(STARSAV2),
    STCALL_AT(STARAD, 6),
    ADDRESS(CHKSDATA),
    ORDERS(BON, EXIT),
    SWITCH(BON, FREEFLAG),
    ADDRESS(P51G),
};

const struct interpretive_code P51E_CODE = {014u, P51E + 3u, COUNT(p51e_words), p51e_words};

/* FREEFLAG clear (the crew marks again): back to the flash (TC P51 +2). */
void p51e_again(void)
{
    tc_at(P51E + 021u);
    p51_plus2();
}

/* P51G: AXISGEN, "COME BACK WITH REFSMMAT IN XDC"; the matrix to
 * REFSMMAT (MATMOVE), and REFSMFLG set. */
static const word p51g_words[] = {
    /* P51G */ ORDER(CALL), ADDRESS(AXISGEN),  ORDERS(AXC_1, AXC_2),
    ADDRESS(XDC),           ADDRESS(REFSMMAT), ORDER(CALL),
    ADDRESS(MATMOVE),       ORDERS(SET, EXIT), SWITCH(SET, REFSMFLG),
};

const struct interpretive_code P51G_CODE = {014u, P51G, COUNT(p51g_words), p51g_words};

void p51g_finis(void)
{
    tc_to(P51G + 011u, GOTOPOOH); /* "FINIS" */
}

/* ---- COARSE, NCOARSE -- p. 952 ---- */

/* COARSE: the coarse alignment to THETAD (IMUCOARS), then the fine
 * alignment mode (IMUFINE), IMUSTALL waiting on each (CURTAINS when it
 * fails); back to the caller (RVQ). */
static const word coarse_words[] = {
    /* COARSE */ ORDER(EXIT),
};

const struct interpretive_code COARSE_CODE = {014u, COARSE, COUNT(coarse_words), coarse_words};

void coarse_imucoars(void)
{
    tc_to(COARSE + 1u, BANKCALL); /* CADR IMUCOARS */
}

void coarse_after_imucoars(void)
{
    tc_to(COARSE + 3u, BANKCALL); /* CADR IMUSTALL */
}

void coarse_failed(void)
{
    tc_to(COARSE + 5u, CURTAINS);
}

void coarse_imufine(void)
{
    tc_to(COARSE + 6u, BANKCALL); /* CADR IMUFINE */
}

void coarse_after_imufine(void)
{
    tc_to(COARSE + 010u, BANKCALL); /* CADR IMUSTALL */
}

void coarse_fine_failed(void)
{
    tc_to(COARSE + 012u, CURTAINS);
}

void coarse_return(void)
{
    tc_at(COARSE + 013u);
    intpret();
}

static const word coarse_return_words[] = {
    /* COARSE +014 */ ORDER(RVQ),
};

const struct interpretive_code COARSE_RETURN_CODE = {
    014u, COARSE + 014u, COUNT(coarse_return_words), coarse_return_words};

/* NCOARSE: the drift compensation started afresh: the time to 1/PIPADT,
 * the PIPAs to -0, the gyro compensation (GCOMP) to zero, and DRIFTFLG
 * set; back to the caller (RVQ). (A is a word: TS does not skip.) */
static const word ncoarse_words[] = {
    /* NCOARSE */ ORDER(EXIT),
};

const struct interpretive_code NCOARSE_CODE = {014u, NCOARSE, COUNT(ncoarse_words), ncoarse_words};

void ncoarse_plus1(void)
{
    ca(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(1_PIPADT)); /* 1/PIPADT */
    set_A(MINUS_ZERO);        /* CS ZERO */
    ts(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(PIPAZ));
    tc_at(NCOARSE + 7u);
    intpret();
}

static const word ncoarse_gcomp_words[] = {
    /* NCOARSE +010 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(GCOMP),
    ORDERS(SET, RVQ),
    SWITCH(SET, DRIFTFLG),
};

const struct interpretive_code NCOARSE_GCOMP_CODE = {
    014u, NCOARSE + 010u, COUNT(ncoarse_gcomp_words), ncoarse_gcomp_words};

/* ---- S52.2, S52.3 -- pp. 953-954 ---- */

/* S52.2: "COMPUTE GIMBAL ANGLES FOR DESIRED SM AND PRESENT VEHICLE". The
 * CDUs' sines and cosines (CDUTRIG) and the navigation base axes in the
 * present stable member (CALCSMSC, to XNB, YNB, ZNB); each of the three
 * (X1 from 18D, by 6) turned into the reference frame (VXM REFSMMAT),
 * unit; then the desired stable member (XSMD, YSMD, ZSMD) to XSM (MATMOVE)
 * and the gimbal angles (CALCGA, to THETAD); back (QMAJ). */
static const word s52_2_words[] = {
    /* S52.2 */ ORDERS(STQ, CALL),
    ADDRESS(QMAJ),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC),
    ORDERS(AXT_1, SSP),
    NUMBER(18),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    /* S52.2A */ ORDERS(VLOAD_STAR, VXM),
    ADDRESS_PLUS_ONE_AT_X1(XNB, 18),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(UNIT),
    STORE_AT_X1(XNB, 18),
    ORDER(TIX_1),
    ADDRESS(S52_2A),
    /* S52.2.1 */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSMD),
    ADDRESS(XSM),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDER(CALL),
    ADDRESS(CALCGA),
    ORDER(GOTO),
    ADDRESS(QMAJ),
};

const struct interpretive_code S52_2_CODE = {014u, S52_2, COUNT(s52_2_words), s52_2_words};

/* S52.3: the nominal orientation at the "TIME OF ALIGNMENT IN MPAC":
 * "XSMD= UNIT R", "YSMD= UNIT(V X R)", "ZSMD= UNIT(XSMD X YSMD)", from the
 * LM's state then (LEMCONIC, to RATT and VATT); back (QMAJ). */
static const word s52_3_words[] = {
    /* S52.3 */ ORDER(STQ),
    ADDRESS(QMAJ),
    STCALL(TDEC1),
    ADDRESS(LEMCONIC),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(XSMD),
    ADDRESS_PLUS_ONE(VATT),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(YSMD),
    ADDRESS_PLUS_ONE(XSMD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(YSMD),
    STCALL(ZSMD),
    ADDRESS(QMAJ),
};

const struct interpretive_code S52_3_CODE = {014u, S52_3, COUNT(s52_3_words), s52_3_words};

#endif
