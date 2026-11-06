/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 6 (Comanche055 pp. 766-771): P51, the IMU
 * orientation determination, and P53, its backup (bank 14); SET1/PDT
 * (bank 15), which GYCRS (part 1) also runs by RTB; and SXTSM, a mark's
 * line of sight in stable member axes (bank 17).
 *
 * P51 "DETERMINES THE INERTIAL ORIENTATION OF THE IMU." The crew coarse
 * aligns the IMU to zero gimbal angles (V41) if they like, then marks two
 * stars with the sextant (R53; in P53, the backup program, R56); SXTSM
 * turns each mark into its line of sight, CHKSDATA (R54) checks the angle
 * between the two, and AXISGEN makes REFSMMAT from them.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, CHECKMM, CURTAINS,
 * ALARM, GOTOPOOH) leaves Z at the routine and Q at the word after the
 * TC, where the routine comes back to; GOTOPOOH doesn't come back. The
 * faithful port keys a routine at each such return, at each TC's return
 * in the file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p51a(void);
static void p51b(void);
static void p51c(void);
static void p51f(void);

/* ---- P51, P53 -- pp. 766-769 (bank 14) ---- */

/* P51 (and P53, its other name): the IMU must be on: IMODES30's bit 9
 * clear (CS, MASK: A is BIT9, positive) goes on to P51A; set (A +0), alarm
 * 210 (the IMU not operating) and GOTOPOOH. (MASK leaves A BIT9 or +0: CCS
 * takes its first two ways only.) */
void p51(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT9);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        tc_at(P51 + 3u); /* TC P51A */
        p51a();
        break;
    case IS_PLUS_ZERO: p51_alarm(); break;
    case IS_MINUS_ZERO: p51_pooh(); break;
    default: not_modelled();
    }
}

void p51_alarm(void)
{
    tc_to(P51 + 4u, ALARM); /* OCT 210 */
}

void p51_pooh(void)
{
    tc_to(P51 + 6u, GOTOPOOH);
}

/* P51A: R02ZERO, then P51AA. */
static void p51a(void)
{
    tc_to(P51A, BANKCALL); /* CADR R02ZERO */
}

/* P51AA: V50N25 with option 15 (GOPERF1, PRFMSTAQ): terminate, proceed
 * (V33: on to the sightings, P51B) or enter (a coarse alignment to zero
 * gimbal angles first, then this flash again). */
void p51aa(void)
{
    set_A(OCT15);                /* PRFMSTAQ */
    tc_to(P51AA + 1u, BANKCALL); /* CADR GOPERF1 */
}

void p51aa_terminate(void)
{
    tc_to(P51AA + 3u, GOTOPOOH); /* "TERM." */
}

void p51aa_proceed(void)
{
    tc_at(P51AA + 4u); /* TC P51B, "V33" */
    p51b();
}

/* The coarse alignment, after a restart point (05024, 13000). */
void p51aa_enter(void)
{
    tc_to(P51AA + 5u, PHASCHNG); /* OCT 05024 */
}

/* "ZERO THE GIMBALS": the desired angles (THETAD) to zero, shown with
 * V06N22, then "NOW DISPLAY COARSE ALIGN VERB 41". (A is +0: TS does not
 * skip.) */
void p51aa_zero_gimbals(void)
{
    set_A(ZERO); /* P51ZERO */
    ts(ADDRESS_OF(THETAD));
    ts(ADDRESS_OF(THETAD) + 1u);
    ts(ADDRESS_OF(THETAD) + 2u);
    set_A(V06N22);                 /* V6N22 */
    tc_to(P51AA + 015u, BANKCALL); /* CADR GODSPRET */
}

void p51aa_display_v41(void)
{
    set_A(V41K);
    tc_to(P51AA + 020u, BANKCALL); /* CADR GODSPRET */
}

/* The coarse alignment itself (IMUCOARS), then IMUFIN20, which schedules
 * the fine alignment mode, IMUSTALL waiting on each (CURTAINS when it
 * fails, "CAGING OR BAD END"); then a restart point and the flash again
 * ("COARSE ALIGN DONE: RECYCLE FOR FINE"). */
void p51aa_imucoars(void)
{
    tc_to(P51AA + 022u, BANKCALL); /* CADR IMUCOARS */
}

void p51aa_after_imucoars(void)
{
    tc_to(P51AA + 024u, BANKCALL); /* CADR IMUSTALL */
}

void p51aa_coarse_failed(void)
{
    tc_to(P51AA + 026u, CURTAINS);
}

void p51aa_imufin20(void)
{
    tc_to(P51AA + 027u, BANKCALL); /* CADR IMUFIN20 */
}

void p51aa_after_imufin20(void)
{
    tc_to(P51AA + 031u, BANKCALL); /* CADR IMUSTALL */
}

void p51aa_fine_failed(void)
{
    tc_to(P51AA + 033u, CURTAINS);
}

void p51aa_coarse_done(void)
{
    tc_to(P51AA + 034u, PHASCHNG); /* OCT 05024 */
}

void p51aa_again(void)
{
    p51aa(); /* TCF P51AA */
}

/* P51B: "DO STAR SIGHTING AND COMPUTE NEW REFSMMAT": a restart point
 * (00014); STARIND, "INDEX -- STAR 1 OR 2", to 0; the drift compensation
 * started afresh (SET1/PDT; GCOMP zeroed; DRIFTFLG set, "ENABLE T4
 * COMPENSATION"); TARG2FLG clear, "SHOW MARK IS STAR --- NOT LANDMARK";
 * then MARKINDX to 1, "INITIALIZE FOR ONE MARK". */
static void p51b(void)
{
    tc_to(P51B, PHASCHNG); /* OCT 00014 */
}

void p51b_plus2(void)
{
    tc_at(P51B + 2u);
    intpret();
}

static const word p51b_words[] = {
    /* P51B +3 */ ORDERS(SSP, SETPD),
    ADDRESS_PLUS_ONE(STARIND),
    NUMBER(0),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(RTB, VLOAD),
    ADDRESS(SET1_PDT),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(GCOMP),
    ORDERS(SET, CLEAR),
    SWITCH(SET, DRIFTFLG),
    SWITCH(CLEAR, TARG2FLG),
    ORDER(EXIT),
};

const struct interpretive_code P51B_CODE = {014u, P51B + 3u, COUNT(p51b_words), p51b_words};

/* (A is 1: TS does not skip.) */
void p51b_one_mark(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(MARKINDX));
    p51c();
}

/* P51C: a star to mark: a restart point (05024); then in P53, "BACKUP
 * PROGRAM" (CHECKMM 53), the marks by R56, else by R53, "SIGHTING
 * ROUTINE"; on at P51C.2. */
static void p51c(void)
{
    tc_to(P51C, PHASCHNG); /* OCT 05024 */
}

void p51c_checkmm(void)
{
    tc_to(P51C + 3u, CHECKMM); /* MM 53 */
}

void p51c_not_p53(void)
{
    tc_at(P51C_1); /* TCF P51C.1, "NOT P53" */
    intpret();
}

void p51c_p53(void)
{
    tc_at(P51C + 6u);
    intpret();
}

static const word p53_words[] = {
    /* P51C +7 */ ORDER(CALL),
    ADDRESS(R56),
    ORDER(GOTO),
    ADDRESS(P51C_2),
};

const struct interpretive_code P53_CODE = {014u, P51C + 7u, COUNT(p53_words), p53_words};

/* P51C.2: the marks' line of sight (SXTSM, "COMPUTE LOS IN SM FROM MARK
 * DATA"), kept in the push-down list: the first star's (STARIND 0, P51D)
 * to STARSAV1, and the planet's direction at the sighting time (PLANET,
 * at TSIGHT) to PLANVEC; the second's to STARSAV2. */
static const word p51c_1_words[] = {
    /* P51C.1 +1 */ ORDER(CALL),
    ADDRESS(R53), /* "SIGHTING ROUTINE" */
    /* P51C.2 */ ORDER(CALL),
    ADDRESS(SXTSM), /* "COMPUTE LOS IN SM FROM MARK DATA" */
    ORDER(PUSH),
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(STARIND),
    ADDRESS(P51D),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STORE(STARSAV2)), /* "DOWNLINK" */
    ORDER(GOTO),
    ADDRESS(P51E),
    /* P51D */ ORDERS(VLOAD, STADR),
    AFTER_STADR(STODL(STARSAV1)),
    ADDRESS_PLUS_ONE(TSIGHT),
    ORDER(CALL),
    ADDRESS(PLANET),
    STORE(PLANVEC),
    /* P51E */ ORDER(EXIT),
};

const struct interpretive_code P51C_1_CODE = {014u, P51C_1 + 1u, COUNT(p51c_1_words), p51c_1_words};

/* P51E: a restart point (05024); the marks released (MKRELEAS, "ZERO
 * MARKSTAT"); then the second star (STARIND +1) on to P51F; the first:
 * a restart point, STARIND to 1, and P51C again, "GO DO SECOND STAR".
 * (STARIND is 0 or 1: CCS takes its first two ways only.) */
void p51e_plus1(void)
{
    tc_to(P51E + 1u, PHASCHNG); /* OCT 05024 */
}

void p51e_mkreleas(void)
{
    tc_to(P51E + 4u, BANKCALL); /* CADR MKRELEAS */
}

void p51e_star(void)
{
    switch (ccs_at(ADDRESS_OF(STARIND))) {
    case POSITIVE: p51f(); break;                           /* TCF P51F, "STAR 2" */
    case IS_PLUS_ZERO: tc_to(P51E + 010u, PHASCHNG); break; /* OCT 05024 */
    default: not_modelled();
    }
}

/* (A is 1: TS does not skip.) */
void p51e_second_star(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(STARIND));
    p51c(); /* TCF P51C, "GO DO SECOND STAR" */
}

/* P51F: both stars marked: a restart point (05024); the planet's
 * direction again; the two catalog vectors (PLANVEC and STARSAV1, to 12D
 * and 6D) and the two lines of sight (STARSAV2 and the push-down list's,
 * to STARAD and STARAD +6) to CHKSDATA, "CHECK STAR ANGLES IN STARAD AND"
 * 6D and 12D. FREEFLAG set (the crew accepts them): P51G; clear: the
 * flash again (TC P51AA). */
static void p51f(void)
{
    tc_to(P51F, PHASCHNG); /* OCT 05024 */
}

void p51f_plus3(void)
{
    tc_at(P51F + 3u);
    intpret();
}

static const word p51f_words[] = {
    /* P51F +4 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(PLANET),
    STOVL_D(12),
    ADDRESS_PLUS_ONE(PLANVEC),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(STARSAV1),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE(STARSAV2),
    STCALL_AT(STARAD, 6),
    ADDRESS(CHKSDATA), /* "CHECK STAR ANGLES IN STARAD AND" */
    ORDERS(BON, EXIT),
    SWITCH(BON, FREEFLAG),
    ADDRESS(P51G),
};

const struct interpretive_code P51F_CODE = {014u, P51F + 4u, COUNT(p51f_words), p51f_words};

void p51f_again(void)
{
    tc_at(P51F + 022u); /* TC P51AA */
    p51aa();
}

/* P51G: AXISGEN, "COME BACK WITH REFSMMAT IN XDC"; the matrix to
 * REFSMMAT (MATMOVE) with REFSMFLG clear while it moves, then set; and
 * ENDP50S. */
static const word p51g_words[] = {
    /* P51G */ ORDER(CALL),  ADDRESS(AXISGEN), /* "COME BACK WITH REFSMMAT IN XDC" */
    ORDERS(AXC_1, AXC_2),    ADDRESS(XDC),     ADDRESS(REFSMMAT), ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, REFSMFLG), ADDRESS(MATMOVE), ORDERS(SET, GOTO), SWITCH(SET, REFSMFLG),
    ADDRESS(ENDP50S),
};

const struct interpretive_code P51G_CODE = {014u, P51G, COUNT(p51g_words), p51g_words};

/* ---- SET1/PDT -- pp. 769-770 (bank 15) ---- */

/* SET1/PDT: the present time (TIME1) to 1/PIPADT, back to DANZIG. (A is a
 * counter's word: TS does not skip.) */
void set1_pdt(void)
{
    ca(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(ONE_PIPADT));
    continue_at(DANZIG); /* TCF DANZIG */
}

/* ---- SXTSM -- p. 771 (bank 17) ---- */

/* SXTSM "COMPUTES AN LOS VECTOR IN SM COORD FROM OCDU AND ICDU MARK
 * DATA": the mark's time (the mark's words, at MARKSTAT's address) to
 * TSIGHT; the mark's words 0-6 (its time and angles) to the star's
 * downlink words (MARKDOWN or MARK2DWN by STARIND, through MKDNCDR, an
 * ECADR loaded to MPAC: X2 its complement); SXTNB, "COMPUTE LOS
 * VECTOR FROM OCDU", then NBSM by the mark's IMU angles (MARKSTAT +2,
 * "BASE ADR OF ICDU", in S1), "TRANSFORM LOS TO SM"; back (QMAJ). */
static const word sxtsm_words[] = {
    /* SXTSM */ ORDER(STQ),
    ADDRESS(QMAJ),
    ORDERS(LXC_1, DLOAD_STAR),
    ADDRESS(MARKSTAT),
    ADDRESS_PLUS_ONE_X1(0),
    STORE(TSIGHT),
    ORDERS(LXC_2, SLOAD_STAR),
    ADDRESS(STARIND),
    ADDRESS_PLUS_ONE_AT_X2(MKDNCDR, 0),
    ORDERS(LXC_2, VLOAD_STAR),
    ADDRESS(MPAC),
    ADDRESS_PLUS_ONE_X1(0),
    STORE_X2_D(0),
    ORDER(DLOAD_STAR),
    ADDRESS_PLUS_ONE_X1(5),
    STORE_X2_D(5),
    ORDER(CALL),
    ADDRESS(SXTNB), /* "COMPUTE LOS VECTOR FROM OCDU IN MKVAC" */
    ORDERS(LXA_1, INCR_1),
    ADDRESS(MARKSTAT),
    NUMBER(2), /* "INCREMENT TO BASE ADR OF ICDU" */
    ORDERS(SXA_1, CALL),
    ADDRESS(S1),
    ADDRESS(NBSM), /* "TRANSFORM LOS TO SM" */
    ORDER(GOTO),
    ADDRESS(QMAJ),
};

const struct interpretive_code SXTSM_CODE = {017u, SXTSM, COUNT(sxtsm_words), sxtsm_words};

#endif
