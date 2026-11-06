/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 1 (Comanche055 pp. 737-742, bank 15): P52,
 * and P54 (P54 = PROG52).
 *
 * PROG52 "ALIGNS THE IMU TO ONE OF THREE ORIENTATIONS SELECTED BY THE
 * ASTRONAUT." The options (OPTION2, flashed with V04N06): 1, the preferred
 * orientation, "AN OPTIMUM ORIENTATION FOR A PREVIOUSLY CALCULATED
 * MANEUVER"; 2, the nominal one, from the state at the align time (S52.3);
 * 3, REFSMMAT's own; 4, the landing site's (P52LS). The crew sees the
 * gimbal angles (S52.2, V06N22) and picks a coarse alignment (CAL53A) or,
 * through COARSTYP, a gyro torque one (GYCRS); then the stars (LOCSAM,
 * PICAPAR) and R51, the fine alignment.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, DOWNFLAG, ALARM,
 * CURTAINS) leaves Z at the routine and Q at the word after the TC, where
 * the routine comes back to; ENDOFJOB and GOTOPOOH don't come back. The
 * faithful port keys a routine at each such return, at each TC's return
 * in the file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p52a_plus1(void);
static void p52b(void);
static void p52_option(void);
static void p52t(void);
static void p52t_display(void);
static void coarstyp(void);

/* ---- PROG52 -- pp. 738-739 ---- */

/* PROG52: a restart point (PHASCHNG, 00254). */
void prog52(void)
{
    tc_to(PROG52, PHASCHNG); /* OCT 00254 */
}

/* PROG52 +2 and +4: UPDATFLG and TRACKFLG cleared (DOWNFLAG). */
void prog52_after_phaschng(void)
{
    tc_to(PROG52 + 2u, DOWNFLAG); /* ADRES UPDATFLG */
}

void prog52_after_updatflg(void)
{
    tc_to(PROG52 + 4u, DOWNFLAG); /* ADRES TRACKFLG */
}

/* PROG52 +6: "IMU STATUS CHECK" (R02BOTH, through BANKCALL). */
void prog52_after_trackflg(void)
{
    tc_to(PROG52 + 6u, BANKCALL); /* CADR R02BOTH */
}

/* PROG52 +010: "IS PFRATFLG SET?" (STATE +2, bit 4) YES: TC P52A, option
 * 1 (preferred); NO: option 2 (nominal). (After the MASK, A is +0 or the
 * bit: CCS takes its first two ways only.) */
void prog52_after_r02both(void)
{
    set_A(BIT4);
    mask_at(ADDRESS_OF(STATE) + 2u);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(PROG52 + 013u); /* TC P52A */
        p52a();
    } else {
        prog52_no_preferred();
    }
}

/* PROG52 +014, P52A's return: "NO", option 2: TC P52A +1. */
void prog52_no_preferred(void)
{
    set_A(BIT2);
    tc_at(PROG52 + 015u);
    p52a_plus1();
}

/* P52A: option 1. */
void p52a(void)
{
    set_A(BIT1);
    p52a_plus1();
}

/* P52A +1: the option to OPTION2. (A is a constant: TS does not skip.) */
static void p52a_plus1(void)
{
    ts(ADDRESS_OF(OPTION2));
    p52b();
}

/* P52B: "FLASH OPTION CODE AND ORIENTATION CODE" (GOPERF4R). */
static void p52b(void)
{
    set_A(BIT1);
    tc_to(P52B + 1u, BANKCALL); /* CADR GOPERF4R */
}

void p52b_terminate(void)
{
    tc_to(P52B + 3u, GOTOPOOH);
}

/* P52B +4: proceed (TC +5). */
void p52b_proceed(void)
{
    tc_at(P52B + 4u);
    p52_option();
}

/* P52B +5: "NEW CODE -- NEW ORIENTATION CODE INPUT": flash it again. */
void p52b_enter(void)
{
    tc_at(P52B + 5u); /* TC P52B */
    p52b();
}

/* P52B +6: the display's immediate return, a restart point (PHASCHNG,
 * 00014), then ENDOFJOB. */
void p52b_immediate_return(void)
{
    tc_to(P52B + 6u, PHASCHNG); /* OCT 00014 */
}

void p52b_after_phaschng(void)
{
    tc_to(P52B + 010u, ENDOFJOB);
}

/* P52B +011: the option's last two bits pick one of four words (INDEX A;
 * TC +1): "L.S." (P52T), "PREF" (P52J), "NORM" (P52T) or "REF" (P52C).
 * The TC leaves Q at the first of them, and each TC there at the next. */
static void p52_option(void)
{
    ca(ADDRESS_OF(OPTION2));
    mask(THREE);
    tc_at(P52B + 014u);
    switch (a_word()) {
    case 0u: p52_landing_site(); break;
    case 1u: p52_preferred(); break;
    case 2u: p52_nominal(); break;
    default: p52_reference();
    }
}

void p52_landing_site(void)
{
    tc_at(P52B + 015u); /* TC P52T */
    p52t();
}

void p52_preferred(void)
{
    tc_at(P52B + 016u); /* TC P52J */
    p52j();
}

void p52_nominal(void)
{
    tc_at(P52B + 017u); /* TC P52T */
    p52t();
}

void p52_reference(void)
{
    p52c(); /* TCF P52C */
}

/* P52T: the align time, +0 to begin with (DCA NEG0: -0 and +0), flashed
 * with V06N34. */
static void p52t(void)
{
    dca(ADDRESS_OF(NEG0));
    dxch(ADDRESS_OF(DSPTEM1));
    p52t_display();
}

static void p52t_display(void)
{
    set_A(V06N34);
    tc_to(P52T + 4u, BANKCALL); /* CADR GOFLASH */
}

void p52t_terminate(void)
{
    tc_to(P52T + 6u, GOTOPOOH);
}

/* P52T +7: proceed (TC +2). */
void p52t_proceed(void)
{
    tc_at(P52T + 7u);
    p52t_time();
}

/* P52T +010: enter, the time loaded: flash it again (TC -5). */
void p52t_enter(void)
{
    tc_at(P52T + 010u);
    p52t_display();
}

/* P52T +011: the align time; none loaded (its first word +0 or -0, BZF),
 * the present time (TIME2). Then the option's bit 2: "NOM" (option 2)
 * the nominal orientation at that time (S52.3); option 4, the landing
 * site's (P52LS). (After the MASK, A is +0 or the bit.) */
void p52t_time(void)
{
    dca(ADDRESS_OF(DSPTEM1));
    if (A_is_zero()) { /* EXTEND; BZF +2 */
        dca(ADDRESS_OF(TIME2));
        dxch(ADDRESS_OF(DSPTEM1));
    }
    ca(ADDRESS_OF(OPTION2));
    mask(BIT2);
    tc_at(ccs_register(A_ADDRESS) == POSITIVE ? P52T + 032u : P52T + 025u); /* TC INTPRET */
    intpret();
}

/* "LS": the landing site's orientation (P52LS), then P52D. */
static const word p52t_landing_site_words[] = {
    /* P52T +026 */ ORDER(CALL),
    ADDRESS(P52LS),
    ORDER(GOTO),
    ADDRESS(P52D),
};

const struct interpretive_code P52T_LANDING_SITE_CODE = {
    015u, P52T + 026u, COUNT(p52t_landing_site_words), p52t_landing_site_words};

/* "COMPUTE NOMINAL IMU" "ORIENTATION" at the align time (S52.3); P52D,
 * "READ VEHICLE ATTITUDE AND" "COMPUTE GIMBAL ANGLES" (S52.2). */
static const word p52t_nominal_words[] = {
    /* P52T +033 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(DSPTEM1),
    ORDER(CALL),
    ADDRESS(S52_3),
    /* P52D */ ORDER(CALL),
    ADDRESS(S52_2),
    ORDER(EXIT),
};

const struct interpretive_code P52T_NOMINAL_CODE = {015u, P52T + 033u, COUNT(p52t_nominal_words),
                                                    p52t_nominal_words};

/* ---- P52D to P52I -- pp. 740-741 ---- */

/* P52D +3: "DISPLAY GIMBAL ANGLES" (V06N22). */
void p52d_after_exit(void)
{
    set_A(VB06N22);
    tc_to(P52D + 4u, BANKCALL); /* CADR GOFLASH */
}

void p52d_terminate(void)
{
    tc_to(P52D + 6u, GOTOPOOH);
}

/* P52D +7: proceed, the alignment's type (TC COARSTYP). */
void p52d_proceed(void)
{
    tc_at(P52D + 7u);
    coarstyp();
}

/* P52J: "RECYCLE: VEHICLE HAS BEEN MANEUVERED": the gimbal angles again
 * (GOTO P52D). */
void p52j(void)
{
    tc_at(P52J);
    intpret();
}

static const word p52j_words[] = {
    /* P52J +1 */ ORDER(GOTO),
    ADDRESS(P52D),
};

const struct interpretive_code P52J_CODE = {015u, P52J + 1u, COUNT(p52j_words), p52j_words};

/* P52J +3: "DO COARSE ALIGN" "ROUTINE" (CAL53A); then REFSMFLG set
 * (CAL53RET). */
void p52j_coarse(void)
{
    tc_at(P52J + 3u);
    intpret();
}

static const word p52j_coarse_words[] = {
    /* P52J +4 */ ORDER(CALL),
    ADDRESS(CAL53A),
    /* CAL53RET */ ORDERS(SET, EXIT),
    SWITCH(SET, REFSMFLG),
};

const struct interpretive_code P52J_COARSE_CODE = {015u, P52J + 4u, COUNT(p52j_coarse_words),
                                                   p52j_coarse_words};

/* P52C: a restart point (PHASCHNG, 04024), then the star selection
 * offered (GOPERF1, ALRM15: code 15). */
void p52c(void)
{
    tc_to(P52C, PHASCHNG); /* OCT 04024 */
}

void p52c_after_phaschng(void)
{
    set_A(ALRM15);
    tc_to(P52C + 3u, BANKCALL); /* CADR GOPERF1 */
}

void p52c_terminate(void)
{
    tc_to(P52C + 5u, GOTOPOOH);
}

/* P52C +6: "V33" (TC +2): the stars at TSIGHT1 from now. */
void p52c_proceed(void)
{
    tc_at(P52C + 6u);
    p52c_stars();
}

/* P52C +7: "E" (enter): TC P52F, the stars the crew picks. */
void p52c_enter(void)
{
    tc_at(P52C + 7u);
    p52f();
}

/* P52C +010: the sun's, the earth's and the moon's places (LOCSAM) at
 * TSIGHT1 from now. */
void p52c_stars(void)
{
    tc_at(P52C + 010u);
    intpret();
}

static const word p52c_stars_words[] = {
    /* P52C +011 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TSIGHT1),
    ORDER(CALL),
    ADDRESS(LOCSAM),
    ORDER(EXIT),
};

const struct interpretive_code P52C_STARS_CODE = {015u, P52C + 011u, COUNT(p52c_stars_words),
                                                  p52c_stars_words};

/* P52E: "DO STAR SELECTION" (PICAPAR); PICAPAR's return is "2 STARS NOT
 * AVAILABLE" (P52I), its +1 "2 STARS AVAILABLE" (P52F). */
void p52e(void)
{
    tc_to(P52E, BANKCALL); /* CADR PICAPAR */
}

void p52e_no_stars(void)
{
    tc_at(P52E + 2u);   /* TC P52I */
    tc_to(P52I, ALARM); /* OCT 405 */
}

/* P52F: R51, the fine alignment; then GOTOPOOH (ENDP50S +1). */
void p52f(void)
{
    tc_at(P52F);
    intpret();
}

static const word p52f_words[] = {
    /* P52F +1 */ ORDER(CALL),
    ADDRESS(R51),
    /* ENDP50S */ ORDER(EXIT),
};

const struct interpretive_code P52F_CODE = {015u, P52F + 1u, COUNT(p52f_words), p52f_words};

void p52f_after_exit(void)
{
    tc_to(ENDP50S + 1u, GOTOPOOH);
}

/* P52I, alarm 405's return: V05N09 flashed; "PROCEED:  DO FINE ALIGN-R51"
 * (P52F); "RECYCLE:  VEHICLE HAS BEEN MANEUVERED" (P52C). */
void p52i_after_alarm(void)
{
    set_A(V05N09);
    tc_to(P52I + 3u, BANKCALL); /* CADR GOFLASH */
}

void p52i_terminate(void)
{
    tc_to(P52I + 5u, GOTOPOOH);
}

void p52i_proceed(void)
{
    tc_at(P52I + 6u); /* TC P52F */
    p52f();
}

void p52i_recycle(void)
{
    tc_at(P52I + 7u); /* TC P52C */
    p52c();
}

/* ---- COARSTYP and GYCRS -- pp. 741-742 ---- */

/* COARSTYP: the alignment's type offered (GOPERF1, code 13): "V34", or
 * "NORMAL", the coarse alignment (P52J +3), or "GYRO COARSE" (GYCRS). */
static void coarstyp(void)
{
    set_A(OCT13);
    tc_to(COARSTYP + 1u, BANKCALL); /* CADR GOPERF1 */
}

void coarstyp_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void coarstyp_normal(void)
{
    p52j_coarse(); /* TCF P52J +3 */
}

void coarstyp_gyro(void)
{
    tc_at(COARSTYP + 5u);
    intpret();
}

/* GYCRS: the desired orientation (XSMD, YSMD, ZSMD) in the present stable
 * member (REFSMMAT) to XDC, YDC and ZDC, and the gyro torque angles
 * (CALCGTA); DRIFTFLG and REFSMFLG cleared. */
static const word gycrs_words[] = {
    /* GYCRS */ ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(XSMD),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(UNIT),
    STOVL(XDC),
    ADDRESS_PLUS_ONE(YSMD),
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(YDC),
    ADDRESS_PLUS_ONE(ZSMD),
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(ZDC),
    ADDRESS(CALCGTA),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DRIFTFLG),
    SWITCH(CLEAR, REFSMFLG),
    ORDER(EXIT),
};

const struct interpretive_code GYCRS_CODE = {015u, GYCRS, COUNT(gycrs_words), gycrs_words};

/* GYCRS +022: the gimbals shown (V16N20, GODSPR) while IMUPULSE torques
 * the gyros (R55CDR) and IMUSTALL waits; if it fails, CURTAINS. */
void gycrs_after_exit(void)
{
    set_A(V16N20);
    tc_to(GYCRS + 023u, BANKCALL); /* CADR GODSPR */
}

void gycrs_after_godspr(void)
{
    set_A(R55CDR);
    tc_to(GYCRS + 026u, BANKCALL); /* CADR IMUPULSE */
}

void gycrs_after_imupulse(void)
{
    tc_to(GYCRS + 030u, BANKCALL); /* CADR IMUSTALL */
}

void gycrs_torque_failed(void)
{
    tc_to(GYCRS + 032u, CURTAINS);
}

/* GYCRS +033: a restart point (PHASCHNG, 04024). */
void gycrs_torqued(void)
{
    tc_to(GYCRS + 033u, PHASCHNG); /* OCT 04024 */
}

void gycrs_after_phaschng(void)
{
    tc_at(GYCRS + 035u);
    intpret();
}

/* The new orientation (XSMD) to REFSMMAT (MATMOVE); PFRATFLG cleared and
 * REFSMFLG set; the time to 1/PIPADT (SET1/PDT) and GCOMP zeroed; DRIFTFLG
 * set, and on to R51K. */
static const word gycrs_done_words[] = {
    /* GYCRS +036 */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSMD),
    ADDRESS(REFSMMAT),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, PFRATFLG),
    SWITCH(SET, REFSMFLG),
    ORDERS(RTB, VLOAD),
    ADDRESS(SET1_PDT),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(GCOMP),
    ORDERS(SET, GOTO),
    SWITCH(SET, DRIFTFLG),
    ADDRESS(R51K),
};

const struct interpretive_code GYCRS_DONE_CODE = {015u, GYCRS + 036u, COUNT(gycrs_done_words),
                                                  gycrs_done_words};

#endif
