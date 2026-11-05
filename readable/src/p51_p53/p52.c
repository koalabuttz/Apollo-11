/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 1 (Luminary099 pp. 926-933, banks 15 and 14): P52.
 *
 * PROG52 "ALIGNS THE IMU TO ONE OF THREE ORIENTATIONS SELECTED BY THE
 * ASTRONAUT. THE PRESENT IMU ORIENTATION IS KNOWN AND IS STORED IN
 * REFSMMAT." The options (OPTION2, flashed with V04N06): 1, the preferred
 * orientation, "AN OPTIMUM ORIENTATION FOR A PREVIOUSLY CALCULATED
 * MANEUVER"; 2, the nominal one, from the state at the align time; 3,
 * REFSMMAT's own; 4, the landing site's. "AFTER A IMU ORIENTATION HAS BEEN
 * SELECTED ROUTINE S52.2 IS OPERATED TO COMPUTE THE GIMBAL ANGLES USING
 * THE NEW ORIENTATION AND THE PRESENT VEHICLE ATTITUDE." The crew sees
 * the gimbal angles (V06N22) and picks a coarse alignment (CAL53A) or,
 * through COARSTYP, a gyro torque one (GYCOARS); then R51 fine aligns.
 *
 * Each call out of the file (BANKCALL, PHASCHNG) leaves Z at the routine
 * and Q at the word after the TC, where the routine comes back to;
 * ENDOFJOB and GOTOPOOH don't come back. The faithful port keys a routine
 * at each such return, and at each TC INTPRET's NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p52a_plus1(void);
static void p52b(void);
static void p52_option(void);
static void opt4(void);
static void p52t(void);
static void p52t_plus2(void);
static void p52t_display(void);
static void p52v(void);
static void p52w(void);
static void p52h(void);
static void p52e(void);
static void coarstyp(void);

/* ---- PROG52 -- pp. 927-928 ---- */

/* PROG52: "IMU STATUS CHECK" (R02BOTH, through BANKCALL). */
void prog52(void)
{
    tc_to(PROG52, BANKCALL); /* CADR R02BOTH */
}

/* PROG52 +2: "IS PFRATFLG SET?" YES: TC P52A, option 1 (preferred); NO:
 * option 2 (nominal). (After the MASK, A is +0 or the bit: CCS takes its
 * first two ways only.) */
void prog52_after_r02both(void)
{
    set_A(PFRATBIT);
    mask_at(ADDRESS_OF(FLAGWRD2));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(PROG52 + 5u); /* TC P52A */
        p52a();
    } else {
        prog52_no_preferred();
    }
}

/* PROG52 +6, P52A's return: "NO": option 2, TC P52A +1. */
void prog52_no_preferred(void)
{
    set_A(BIT2);
    tc_at(PROG52 + 7u);
    p52a_plus1();
}

/* P52A: option 1. */
void p52a(void)
{
    set_A(BIT1);
    p52a_plus1();
}

/* P52A +1: the option to OPTION2. (A is a word: TS does not skip.) */
static void p52a_plus1(void)
{
    ts(ADDRESS_OF(OPTION2));
    p52b();
}

/* P52B: "FLASH OPTION CODE AND ORIENTATION CODE" (GOPERF4R, "FLASH
 * V04N06"). */
static void p52b(void)
{
    set_A(BIT1);
    tc_to(P52B + 1u, BANKCALL); /* CADR GOPERF4R */
}

/* P52B +3: terminate. */
void p52b_terminate(void)
{
    tc_to(P52B + 3u, GOTOPOOH);
}

/* P52B +4: "V33 -- PROCEED" (TCF +5). */
void p52b_proceed(void)
{
    p52_option();
}

/* P52B +5: "NEW CODE -- NEW ORIENTATION CODE INPUT": flash it again. */
void p52b_enter(void)
{
    tc_at(P52B + 5u); /* TC P52B */
    p52b();
}

/* P52B +6: "DISPLAY RETURN": PHASCHNG, OCT 00014. */
void p52b_immediate_return(void)
{
    tc_to(P52B + 6u, PHASCHNG);
}

/* P52B +010: PHASCHNG's return. */
void p52b_after_phaschng(void)
{
    tc_to(P52B + 010u, ENDOFJOB);
}

/* P52B +011: the option's last two bits pick one of four words (INDEX A;
 * TC +1): "OPTION 4 LANDING SITE", "OPTION 1 PREFERRED", "OPTION 2
 * NOMINAL" or, P52E, "OPTION 3 REFSMMAT". The TC leaves Q at the first
 * of them. */
static void p52_option(void)
{
    ca(ADDRESS_OF(OPTION2));
    mask(THREE);
    tc_at(P52B + 014u);
    switch (a_word()) {
    case 0u: opt4(); break;
    case 1u: p52h(); break;
    case 2u: p52t(); break;
    default: p52e(); break;
    }
}

/* The option table's first word, TCF OPT4. */
void p52_option4(void)
{
    opt4();
}

/* P52E: option 3 ("GO DO R51"). */
static void p52e(void)
{
    tc_at(P52E);
    intpret();
}

static const word p52e_words[] = {
    /* P52E +1 */ ORDER(GOTO),
    ADDRESS(P52F),
};

const struct interpretive_code P52E_CODE = {015u, P52E + 1u, COUNT(p52e_words), p52e_words};

/* OPT4: "IF OPTION 4 DISPLAY TLAND". */
static void opt4(void)
{
    dca(ADDRESS_OF(TLAND));
    p52t_plus2();
}

/* P52T: the align time, +0 to begin with (DCA NEG0: -0 and +0), flashed
 * with V06N34*. */
static void p52t(void)
{
    dca(ADDRESS_OF(NEG0));
    p52t_plus2();
}

static void p52t_plus2(void)
{
    dxch(ADDRESS_OF(DSPTEM1));
    p52t_display();
}

static void p52t_display(void)
{
    set_A(V06N34_STAR);
    tc_to(P52T + 4u, BANKCALL); /* CADR GOFLASH */
}

/* P52T +6: terminate. */
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

/* P52T +011: the time to TALIGN: "IF TIME ZERO OR NEG USE TIME2", the
 * present time. */
void p52t_time(void)
{
    dxch(ADDRESS_OF(DSPTEM1));
    if (A_is_zero_or_minus()) dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(TALIGN));
    p52v();
}

/* P52V: options 2 and 4 by OPTION2's bit 2: 2 to P52W; "OPTION 4 -- GET LS
 * ORIENTATION" (P52LS). (After the MASK, A is +0 or the bit.) */
static void p52v(void)
{
    ca(ADDRESS_OF(OPTION2));
    mask(BIT2);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(P52V + 3u); /* TC P52W */
        p52w();
    } else {
        p52v_option4();
    }
}

/* P52V +4, P52W's return. */
void p52v_option4(void)
{
    tc_at(P52V + 4u);
    intpret();
}

static const word p52v_words[] = {
    /* P52V +5 */ ORDER(GOTO),
    ADDRESS(P52LS),
};

const struct interpretive_code P52V_CODE = {015u, P52V + 5u, COUNT(p52v_words), p52v_words};

/* ---- P52W to P52OUT -- p. 930 ---- */

/* P52W: "PICK UP ALIGN TIME" and the "COMPUTED NOMINAL IMU ORIENTATION"
 * (S52.3); P52D: "READ VEHICLE ATTITUDE AND COMPUTE GIMBAL ANGLES"
 * (S52.2), and on in basic code. */
static void p52w(void)
{
    tc_at(P52W);
    intpret();
}

static const word p52w_words[] = {
    /* P52W +1 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TALIGN),
    ADDRESS(S52_3),
    /* P52D */ ORDER(CALL),
    ADDRESS(S52_2),
    ORDER(EXIT),
};

const struct interpretive_code P52W_CODE = {015u, P52W + 1u, COUNT(p52w_words), p52w_words};

/* P52D +3: "DISPLAY GIMBAL ANGLES" (V06N22, GOFLASH). */
void p52d_after_exit(void)
{
    set_A(V06N22);
    tc_to(P52D + 4u, BANKCALL); /* CADR GOFLASH */
}

/* P52D +6: terminate. */
void p52d_terminate(void)
{
    tc_to(P52D + 6u, GOTOPOOH);
}

/* P52D +7: "V33 -- PROCEED, SEE IF GYRO TORQUE COARSE". */
void p52d_proceed(void)
{
    coarstyp();
}

/* P52H: option 1, the preferred orientation, straight to P52D. */
static void p52h(void)
{
    tc_at(P52H);
    intpret();
}

static const word p52h_words[] = {
    /* P52H +1 */ ORDER(GOTO),
    ADDRESS(P52D),
};

const struct interpretive_code P52H_CODE = {015u, P52H + 1u, COUNT(p52h_words), p52h_words};

/* REGCOARS: "DO COARSE ALIGN ROUTINE" (CAL53A); COARSRET: REFSMFLG set,
 * PFRATFLG clear; P52F: R51, the fine alignment; P52OUT: on in basic
 * code. */
void regcoars(void)
{
    tc_at(REGCOARS);
    intpret();
}

static const word regcoars_words[] = {
    /* REGCOARS +1 */ ORDER(CALL),
    ADDRESS(CAL53A),
    /* COARSRET */ ORDERS(SET, CLEAR),
    SWITCH(SET, REFSMFLG),
    SWITCH(CLEAR, PFRATFLG),
    /* P52F */ ORDER(CALL),
    ADDRESS(R51),
    /* P52OUT */ ORDER(EXIT),
};

const struct interpretive_code REGCOARS_CODE = {015u, REGCOARS + 1u, COUNT(regcoars_words),
                                                regcoars_words};

/* P52OUT +1. */
void p52out_after_exit(void)
{
    tc_to(P52OUT + 1u, GOTOPOOH);
}

/* ---- COARSTYP -- p. 931 ---- */

/* COARSTYP: "CHECK FOR GYRO TORQUE COARSE ALIGNMENT": "DISPLAY V 50N25
 * WITH COARSE ALIGN OPTION" (OCT13, GOPERF1). */
static void coarstyp(void)
{
    set_A(OCT13);
    tc_to(COARSTYP + 1u, BANKCALL); /* CADR GOPERF1 */
}

/* COARSTYP +3: "V34 -- TERMIN&OE". */
void coarstyp_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* COARSTYP +4: "V33 -- NORMAL COARSE". */
void coarstyp_normal(void)
{
    regcoars();
}

/* COARSTYP +5: "V32 -- GYRO TORQUE COARSE". */
void coarstyp_gyro_torque(void)
{
    tc_at(COARSTYP + 5u);
    intpret();
}

/* The desired stable member axes XSMD, YSMD and ZSMD in the present ones'
 * (times REFSMMAT: "GET SM(DESIRED) WRT SM(PRESENT)"), made unit vectors,
 * to XDC, YDC and ZDC for GYCOARS, the gyro torque coarse alignment; then
 * P52OUT. */
static const word coarstyp_words[] = {
    /* COARSTYP +6 */ ORDERS(VLOAD, MXV),
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
    ADDRESS(GYCOARS),
    ORDER(GOTO),
    ADDRESS(P52OUT),
};

const struct interpretive_code COARSTYP_CODE = {015u, COARSTYP + 6u, COUNT(coarstyp_words),
                                                coarstyp_words};

/* ---- P52LS -- p. 932 ---- */

/* P52LS: "COMPUTE LANDING ORIENTATION FOR OPTION 4". LUNAFLAG set and
 * ERADFLAG clear "TO PICK UP RLS", the landing site in the moon's frame;
 * "JAM ALIGN TIME IN TLAND FOR OPTION 4"; RP-TO-R takes the site to the
 * reference frame ("TRANS RLS TO REF"), the "INPUT TO LAT-LONG" in N89DISP,
 * which displays it. Then "COMPUTE LANDING SITE ORIENT (XSMD)" (LSORIENT)
 * and "NOW GO COMPUTE GIMBAL ANGLES" (P52D). */
static const word p52ls_words[] = {
    /* P52LS */ ORDERS(SET, CLEAR),
    SWITCH(SET, LUNAFLAG),
    SWITCH(CLEAR, ERADFLAG),
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RLS), /* "PICK UP LANDING SITE VEC IN MF" */
    ORDERS(PDDL, PUSH),    /* "RLS PD 0-5" */
    ADDRESS_PLUS_ONE(TALIGN),
    STCALL(TLAND),
    ADDRESS(RP_TO_R),
    ORDER(VSR2),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(TALIGN),
    ORDER(CALL),
    ADDRESS(N89DISP),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(ALPHAV),
    STCALL(XSMD),
    ADDRESS(LSORIENT),
    ORDER(GOTO),
    ADDRESS(P52D),
};

const struct interpretive_code P52LS_CODE = {015u, P52LS, COUNT(p52ls_words), p52ls_words};

/* ---- N89DISP -- p. 933 ---- */

/* N89DISP: "SUBROUTINE TO CALCULATE AND DISPLAY THE LUNAR LANDING SITE".
 * The return to QMAJ; the time to GDT/2 +4 ("TEMP STORE TIME"); LAT-LONG
 * makes the site's latitude, longitude and altitude, kept in LANDLAT,
 * LANDLONG (halved) and LANDALT; on in basic code. */
static const word n89disp_words[] = {
    /* N89DISP */ ORDER(STQ),
    ADDRESS(QMAJ),
    STCALL_AT(GDT_2, 4), /* "TEMP STORE TIME" */
    ADDRESS(LAT_LONG),
    ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(LONG),
    STODL(LANDLONG),
    ADDRESS_PLUS_ONE(ALT),
    STODL(LANDALT),
    ADDRESS_PLUS_ONE(LAT),
    STODL(LANDLAT),
    ORDER(EXIT),
};

const struct interpretive_code N89DISP_CODE = {014u, N89DISP, COUNT(n89disp_words), n89disp_words};

/* LSDISP: "DISPLAY LAT,LONG/2,ALT" (V06N89*, GOFLASH). */
void lsdisp(void)
{
    set_A(V06N89_STAR);
    tc_to(LSDISP + 1u, BANKCALL); /* CADR GOFLASH */
}

/* LSDISP +3: "V34 -- TERMINATE -- EXIT P57". */
void lsdisp_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* LSDISP +4: "V33 -- PROCEED -- ACCEPT LS DATA" (TCF +2). */
void lsdisp_proceed(void)
{
    tc_at(LSDISP + 6u);
    intpret();
}

/* LSDISP +5: "V32 OR E -- LOOK AGAIN AND/OR LOAD NEW LS". */
void lsdisp_enter(void)
{
    lsdisp();
}

/* The site, as displayed or loaded, back to LAT, LONG and ALT, and the
 * time: LALOTORV, "GET RLS BACK FROM LAT,LONG,ALT" ("RLS B-29 IN MPAC AND
 * ALPHAV"); then back to the caller (GOTO QMAJ). */
static const word lsdisp_words[] = {
    /* LSDISP +7 */ ORDERS(DLOAD, SL1),
    ADDRESS_PLUS_ONE(LANDLONG),
    STODL(LONG),
    ADDRESS_PLUS_ONE(LANDALT),
    STODL(ALT),
    ADDRESS_PLUS_ONE(LANDLAT),
    STODL(LAT),
    ADDRESS_PLUS_ONE_AT(GDT_2, 4), /* "PICK UP TIME" */
    ORDER(CALL),
    ADDRESS(LALOTORV),
    ORDER(GOTO),
    ADDRESS(QMAJ),
};

const struct interpretive_code LSDISP_CODE = {014u, LSDISP + 7u, COUNT(lsdisp_words), lsdisp_words};

#endif
