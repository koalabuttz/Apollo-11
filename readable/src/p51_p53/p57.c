/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 9 (Luminary099 pp. 975-983, bank 15; LSORIENT in
 * bank 4): P57, the "LUNAR SURFACE IMU ALIGNMENT PROGRAM".
 *
 * P57 checks the IMU is on (IMUCHK) and flashes the alignment option
 * (OPTION2): 0, the attitude stored before (YNBSAV, ZNBSAV); 1, the
 * landing site and that attitude's Z axis; 2, two stars; 3, the landing
 * site and one star. The orientation code picks the desired stable
 * member: the "LS ORIENTATION" at an align time (TDISP, LSORIENT), the
 * one REFSMMAT has, or the preferred one. Then the option code and flags
 * are flashed (PACKOPTN, DSPOPTN); with an attitude stored and option 1
 * or 3, gravity (GVDETER) first; and each option makes two vectors in
 * the reference frame (VEC1, VEC2) and the same two seen in the present
 * stable member (STARSAV1, STARSAV2) for SURFLINE (part 8). MFREF and
 * REFMF turn vectors from moon-fixed axes and back.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, ENDOFJOB, ALARM, UPFLAG,
 * DOWNFLAG, GOTOPOOH, SWRETURN) leaves Z at the routine and Q at the word
 * after the TC, where the routine comes back to; GOTOPOOH, ENDOFJOB and
 * SWRETURN don't come back. The faithful port keys a routine at each such
 * return, at each TC's return in the file, and at each TC INTPRET's
 * NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p57opt(void);
static void alignopt(void);
static void tdisp(void);
static void packoptn(void);
static void dspoptn(void);
static void getlmatt(void);
static void p57opt0(void);
static void p57opt1(void);
static void p57opt3(void);

/* ---- P57 -- p. 975 ---- */

/* P57: "IS ISS ON -- IF NOT, IMUCHK WILL SEND" alarm 210 (IMUCHK, by
 * BANKCALL). */
void p57(void)
{
    tc_to(P57, BANKCALL); /* CADR IMUCHK */
}

/* P57 +2, IMUCHK's return: "JAM REFSMMAT OPTION 3 FOR INITIAL DISP."
 * (A is a constant: TS does not skip.) */
void p57_after_imuchk(void)
{
    set_A(THREE);
    ts(ADDRESS_OF(OPTION2));
    p57opt();
}

/* P57OPT: "FLASH V04N06 FOR ALIGNMENT CODE" (GOPERF4R). */
static void p57opt(void)
{
    set_A(BIT1);
    tc_to(P57OPT + 1u, BANKCALL); /* CADR GOPERF4R */
}

void p57opt_terminate(void)
{
    tc_to(P57OPT + 3u, GOTOPOOH); /* "V34 TERMINATE" */
}

void p57opt_proceed(void)
{
    alignopt(); /* TCF ALIGNOPT */
}

void p57opt_recycle(void)
{
    p57opt(); /* TCF P57OPT */
}

/* P57OPT +6: the display's immediate return, a restart point (PHASCHNG,
 * 00014), then ENDOFJOB. */
void p57opt_immediate_return(void)
{
    tc_to(P57OPT + 6u, PHASCHNG); /* OCT 00014 */
}

void p57opt_after_phaschng(void)
{
    tc_to(P57OPT + 010u, ENDOFJOB);
}

/* ALIGNOPT: the option's last two bits pick one of four words (INDEX A;
 * TCF +1): "OPTION 4 LS ORIENTATION" (TDISP), "OPTION 1 PREFERRED"
 * (PACKOPTN), option 2 ("INVALID IN P57, RECYCLE") or "OPTION 3
 * REFSMMAT". */
static void alignopt(void)
{
    ca(ADDRESS_OF(OPTION2));
    mask(THREE);
    switch (a_word()) {
    case 0u: tdisp(); break;
    case 1u: packoptn(); break;
    case 2u: p57opt(); break;
    default: tc_at(ALIGNOPT + 7u); intpret();
    }
}

/* Option 3: REFSMMAT to XSMD ("JAM REFSMMAT IN XSMD LOC"), then the
 * option display (PACKOPTN -1 is the EXIT just before it). */
static const word alignopt_refsmmat_words[] = {
    /* ALIGNOPT +010 */ ORDERS(AXC_1, AXC_2),
    ADDRESS(REFSMMAT),
    ADDRESS(XSMD),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDER(GOTO),
    ADDRESS_AT(PACKOPTN, -1),
};

const struct interpretive_code ALIGNOPT_REFSMMAT_CODE = {
    015u, ALIGNOPT + 010u, COUNT(alignopt_refsmmat_words), alignopt_refsmmat_words};

/* TDISP: "LOAD ASCENT TIME FOR DISPLAY" (TIG) to DSPTEM1. */
static void tdisp(void)
{
    tc_at(TDISP);
    intpret();
}

static const word tdisp_words[] = {
    /* TDISP +1 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIG),
    STORE(DSPTEM1), /* P57A */
    ORDER(EXIT),
};

const struct interpretive_code TDISP_CODE = {015u, TDISP + 1u, COUNT(tdisp_words), tdisp_words};

/* P57AA: "DISPLAY TALIGN" (V06N34*, from DSPTEM1). */
void p57aa(void)
{
    set_A(V06N34_STAR);
    tc_to(P57AA + 1u, BANKCALL); /* CADR GOFLASH */
}

void p57aa_terminate(void)
{
    continue_at(GOTOPOOH); /* "V34 -- TERMINATE" */
}

void p57aa_proceed(void)
{
    tc_at(P57AA + 6u); /* TCF +2 */
    intpret();
}

void p57aa_recycle(void)
{
    p57aa(); /* TCF P57AA */
}

/* The align time: "IF KEY IN TIME ZERO - TALIGN=CURRENT TIME"; a time
 * later than now is also the ascent time (TIG). At it, the LM's state
 * (LEMPREC: RATT) gives XSMD, and LSORIENT the rest of the desired
 * stable member. */
static const word p57aa_time_words[] = {
    /* P57AA +7 */ ORDERS(RTB, PDDL),
    ADDRESS(LOADTIME), /* "PUSH CURRENT TIME AND PICK UP KEY IN" */
    ADDRESS_PLUS_ONE(DSPTEM1),
    ORDERS(BZE, PDDL),
    ADDRESS(P57C),
    ORDERS(DSU, BPL), /* "NOT ZERO SO EXCHANGE PD WITH DSPTEM1" */
    ADDRESS_PLUS_ONE(DSPTEM1),
    ADDRESS(P57C),
    ORDERS(DLOAD, STADR), /* "IF KEYIN TIME GREATER THAN CURRENT TIME" */
    AFTER_STADR(STORE(TIG)),
    STCALL(TALIGN),
    ADDRESS(P57D),
    /* P57C */ ORDERS(DLOAD, STADR),
    AFTER_STADR(STORE(TALIGN)),
    /* P57D */ STCALL(TDEC1),
    ADDRESS(LEMPREC),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    STCALL(XSMD),
    ADDRESS(LSORIENT),
    ORDER(EXIT), /* PACKOPTN -1 */
};

const struct interpretive_code P57AA_TIME_CODE = {015u, P57AA + 7u, COUNT(p57aa_time_words),
                                                  p57aa_time_words};

/* ---- The option display -- pp. 976-977 ---- */

/* One flag's bit in the display: the flag's bit masked (CCS A: +0 if
 * clear) and, if set, the display's bit, added to OPTION1 +2. */
static void pack_flag(word flag_bit, unsigned flagword, word display_bit)
{
    set_A(flag_bit);
    mask_at(flagword);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: set_A(display_bit); break; /* "SET" */
    case IS_PLUS_ZERO: break;                 /* "CLEAR -- ZERO IN A" */
    default: not_modelled(); return;
    }
    ads(ADDRESS_OF(OPTION1) + 2u);
}

/* PACKOPTN: "PACK FLAG BITS FOR OPTION DISPLAY": REFSMFLG (bit 7) and
 * ATTFLG (bit 4), and "JAM 00010 IN OPTION1 FOR CHECK LIST". (A is +0 or
 * a constant: TS does not skip.) */
static void packoptn(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(OPTION1) + 1u); /* "JAM ZERO IN ALIGNMENT OPTION" */
    ts(ADDRESS_OF(OPTION1) + 2u);
    pack_flag(REFSMBIT, ADDRESS_OF(FLAGWRD3), BIT7);
    pack_flag(ATTFLBIT, ADDRESS_OF(FLAGWRD6), BIT4);
    set_A(BIT4);
    ts(ADDRESS_OF(OPTION1));
    dspoptn();
}

void packoptn_after_exit(void)
{
    packoptn();
}

/* DSPOPTN: "DISPLAY OPTION CODE AND FLAG BITS" (V05N06). */
static void dspoptn(void)
{
    set_A(VB05N06);
    tc_to(DSPOPTN + 1u, BANKCALL); /* CADR GOFLASH */
}

void dspoptn_terminate(void)
{
    continue_at(GOTOPOOH); /* "VB34 -- TERMINATE" */
}

/* DSPOPTN +4, proceed (TCF +2): REFSMFLG set, "GO COMPUTE LM ATTITUDE"
 * (GETLMATT); else with an attitude stored (ATTFLG) or option 2 or 3,
 * BYLMATT; else "OPTION INCONSISTENT WITH FLAGS -- ALARM 701". */
void dspoptn_proceed(void)
{
    set_A(REFSMBIT);
    mask_at(ADDRESS_OF(FLAGWRD3));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: getlmatt(); return;
    case IS_PLUS_ZERO: break;
    default: not_modelled(); return;
    }
    set_A(ATTFLBIT);
    mask_at(ADDRESS_OF(FLAGWRD6));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: bylmatt(); return;
    case IS_PLUS_ZERO: break;
    default: not_modelled(); return;
    }
    set_A(BIT2); /* "SEE IF OPTION 2 OR 3" */
    mask_at(ADDRESS_OF(OPTION2));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: bylmatt(); break; /* "OPTION 2 OR 3 BUT DON'T HAVE ATTITUDE" */
    case IS_PLUS_ZERO: tc_to(DSPOPTN + 022u, ALARM); break; /* OCT 701 */
    default: not_modelled();
    }
}

void dspoptn_recycle(void)
{
    dspoptn(); /* TCF DSPOPTN */
}

/* DSPOPTN +024, ALARM's return: "DISPLAY ALARM FOR ACTION" (V05N09);
 * proceed or recycle, the option display again. */
void dspoptn_after_alarm(void)
{
    set_A(VB05N09);
    tc_to(DSPOPTN + 025u, BANKCALL); /* CADR GOFLASH */
}

void dspoptn_alarm_terminate(void)
{
    continue_at(GOTOPOOH); /* "VB34 -- TERMINATE" */
}

void dspoptn_alarm_proceed(void)
{
    dspoptn(); /* "V33 -- PROCEED *********TEMPORARY" */
}

void dspoptn_alarm_recycle(void)
{
    dspoptn(); /* "VB32 -- RECYCLE TO OPTION DISPLAY V 05N06" */
}

/* ---- MFREF and REFMF -- pp. 978-979 ---- */

/* MFREF: "TRANSFORM VEC1,2 FROM MOON FIXED TO REF AND JAM BACK IN VEC1,2"
 * (RP-TO-R, at the present time: TSIGHT). */
static const word mfref_words[] = {
    /* MFREF */ ORDERS(STQ, SETPD),
    ADDRESS(QMAJ),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STOVL(TSIGHT),
    ADDRESS_PLUS_ONE(VEC1),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TSIGHT),
    ORDER(CALL),
    ADDRESS(RP_TO_R),
    STOVL(VEC1),
    ADDRESS_PLUS_ONE(VEC2),
    ORDERS(SETPD, PDDL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(TSIGHT),
    ORDERS(PUSH, CALL),
    ADDRESS(RP_TO_R),
    STCALL(VEC2),
    ADDRESS(QMAJ),
};

const struct interpretive_code MFREF_CODE = {015u, MFREF, COUNT(mfref_words), mfref_words};

/* REFMF: "COMPUTE LM ATTITUDE IN MOON FIXED COORDINATES USING REFSMMAT AND"
 * "STORE IN YNBSAV AND ZNBSAV": the navigation base's Y and Z axes in the
 * present stable member (CDUTRIG, CALCSMSC), in the reference frame
 * (REFSMMAT) and in moon-fixed axes (R-TO-RP); then ATTFLAG set. */
static const word refmf_words[] = {
    /* REFMF */ ORDERS(STQ, CALL),
    ADDRESS(QMAJ),
    ADDRESS(CDUTRIG), /* "GET SIN AND COS OF CDUS" */
    ORDERS(RTB, SETPD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE_D(0),
    STCALL(TSIGHT),
    ADDRESS(CALCSMSC), /* "GET YNB IN SM" */
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(YNB),
    ADDRESS_PLUS_ONE(REFSMMAT), /* "YNB TO REF" */
    ORDERS(UNIT, PDDL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ORDERS(PUSH, CALL),
    ADDRESS(R_TO_RP),
    STOVL(YNBSAV), /* "YNB TO MF" */
    ADDRESS_PLUS_ONE(ZNB),
    ORDERS(VXM, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT), /* "ZNB TO REF" */
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TSIGHT),
    ORDER(CALL),
    ADDRESS(R_TO_RP), /* "ZNB TO MF" */
    STORE(ZNBSAV),
    ORDER(SETGO),
    SWITCH(SETGO, ATTFLAG),
    ADDRESS(QMAJ),
};

const struct interpretive_code REFMF_CODE = {015u, REFMF, COUNT(refmf_words), refmf_words};

/* ---- The alignment option -- p. 980 ---- */

/* GETLMATT: "GO TRANSFORM TO MF IN YNBSAV, ZNBSAV" (REFMF), then on to
 * BYLMATT. */
static void getlmatt(void)
{
    tc_at(GETLMATT);
    intpret();
}

static const word getlmatt_words[] = {
    /* GETLMATT +1 */ ORDER(CALL),
    ADDRESS(REFMF),
    ORDER(EXIT),
};

const struct interpretive_code GETLMATT_CODE = {015u, GETLMATT + 1u, COUNT(getlmatt_words),
                                                getlmatt_words};

/* BYLMATT: "SET INITIAL ALIGN FLAG" (INITALGN). */
void bylmatt(void)
{
    tc_to(BYLMATT, UPFLAG); /* ADRES INITALGN */
}

/* BYLMATT +2, UPFLAG's return: options 1 and 3 (bit 1) get gravity first
 * (GVDETER); else ATTCHK. */
void bylmatt_after_upflag(void)
{
    set_A(BIT1);
    mask_at(ADDRESS_OF(OPTION2)); /* "SEE IF OPTION 1 OR 3" */
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: gvdeter(); break; /* TCF GVDETER */
    case IS_PLUS_ZERO: attchk(); break;
    default: not_modelled();
    }
}

/* ATTCHK: a restart point (PHASCHNG, 04024), then "CHECK ATTFLAG". */
void attchk(void)
{
    tc_to(ATTCHK, PHASCHNG); /* OCT 04024 */
}

/* ATTCHK +2: an attitude stored (ATTFLG), "GET ALIGNMENT VECS FOR OPTION
 * 0"; else P57JUMP. */
void attchk_after_phaschng(void)
{
    set_A(ATTFLBIT);
    mask_at(ADDRESS_OF(FLAGWRD6));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: p57opt0(); break; /* TCF P57OPT0 */
    case IS_PLUS_ZERO: p57jump(); break;
    default: not_modelled();
    }
}

/* P57JUMP: a restart point (PHASCHNG, 04024), then "BRANCH ON OPTION
 * CODE". */
void p57jump(void)
{
    tc_to(P57JUMP, PHASCHNG); /* OCT 04024 */
}

void p57jump_after_phaschng(void)
{
    tc_to(P57JUMP + 2u, DOWNFLAG); /* ADRES INITALGN */
}

/* P57JUMP +4, DOWNFLAG's return: the option's last two bits pick one of
 * four words (INDEX A; TCF +1): options 0 to 3. */
void p57jump_after_downflag(void)
{
    set_A(THREE);
    mask_at(ADDRESS_OF(OPTION2)); /* "BRANCH ON OPTION CODE" */
    switch (a_word()) {
    case 0u: p57opt0(); break;
    case 1u: p57opt1(); break;
    case 2u: two_stars(); break; /* P57OPT2: TCF 2STARS */
    default: p57opt3();
    }
}

/* ---- The options -- pp. 981-982 ---- */

/* P57OPT0: "OPTION 0, GET TWO ATTITUDE VECS": the stored attitude
 * (YNBSAV, ZNBSAV) to VEC1 and VEC2, and the navigation base's Y and Z
 * axes in the present stable member (CDUTRIG, CALCSMSC) to STARSAV1 and
 * STARSAV2 (SAMETYP); VEC1 and VEC2 to the reference frame (MFREF), then
 * SURFLINE. */
static void p57opt0(void)
{
    tc_at(P57OPT0);
    intpret();
}

static const word p57opt0_words[] = {
    /* P57OPT0 +1 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(YNBSAV), /* "Y AND Z ATTITUDE WILL BE PUT IN REF" */
    STOVL(VEC1),
    ADDRESS_PLUS_ONE(ZNBSAV),
    STCALL(VEC2),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC), /* "COMPUTE SC AXIS WRT PRESENT SM" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(YNB),
    /* SAMETYP */ STOVL(STARSAV1), /* "Y SC AXIS WRT PRESENT SM" */
    ADDRESS_PLUS_ONE(ZNB),
    STCALL(STARSAV2), /* "Z SC AXIS WRT PRESENT SM" */
    ADDRESS(MFREF),   /* "TRANSFORM VEC1,2 FROM MF TO REF" */
    ORDER(GOTO),
    ADDRESS(SURFLINE),
};

const struct interpretive_code P57OPT0_CODE = {015u, P57OPT0 + 1u, COUNT(p57opt0_words),
                                               p57opt0_words};

/* P57OPT1: "OPTION 1, GET LANDING SITE AND Z-ATTITUDE VEC": the landing
 * site (RLS) and the stored Z axis (ZNBSAV) to VEC1 and VEC2; gravity
 * (GSAV) in the present stable member (CDU*NBSM) for STARSAV1, and the Z
 * axis for STARSAV2: "NOW DO SAME AS OPTION 0" (SAMETYP). */
static void p57opt1(void)
{
    tc_at(P57OPT1);
    intpret();
}

static const word p57opt1_words[] = {
    /* P57OPT1 +1 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RLS), /* "LANDING SITE VEC" */
    STOVL(VEC1),
    ADDRESS_PLUS_ONE(ZNBSAV), /* "Z ATTITUDE VEC" */
    STCALL(VEC2),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC), /* "GET ZNB AXIS WRT PRES SM FOR STARSAV2" */
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(GSAV), /* "TRANS GSAV FROM NB TO SM FOR STARSAV1" */
    ADDRESS(CDU_STARNBSM),
    ORDER(GOTO),
    ADDRESS(SAMETYP),
};

const struct interpretive_code P57OPT1_CODE = {015u, P57OPT1 + 1u, COUNT(p57opt1_words),
                                               p57opt1_words};

/* P57OPT3: "OPTION 3, GET LANDING SITE VEC AND ONE STAR SIGHTING": the
 * landing site (RLS) to VEC1, and to VEC2 for now ("DUMMY VEC2 FOR 2ND
 * CATALOG STAR"); gravity in the present stable member (CDU*NBSM) to
 * STARSAV1; VEC1 to the reference frame (MFREF); then the one star
 * (1STAR). */
static void p57opt3(void)
{
    tc_at(P57OPT3);
    intpret();
}

static const word p57opt3_words[] = {
    /* P57OPT3 +1 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RLS), /* "LANDING SITE VEC" */
    STORE(VEC1),
    STOVL(VEC2),
    ADDRESS_PLUS_ONE(GSAV), /* "GRAVITY VEC NB" */
    ORDER(CALL),
    ADDRESS(CDU_STARNBSM),
    STCALL(STARSAV1),
    ADDRESS(MFREF), /* "STARSAV2 IS STORED AS 2ND OBSERVED STAR" */
    ORDER(EXIT),
};

const struct interpretive_code P57OPT3_CODE = {015u, P57OPT3 + 1u, COUNT(p57opt3_words),
                                               p57opt3_words};

void p57opt3_after_exit(void)
{
    one_star(); /* "1STAR GET VEC2, STARSAV2 GOES TO SURFLINE." */
}

/* ---- IMUCHK -- p. 983 ---- */

/* IMUCHK: "CHECK IMODES30 TO VERIFY IMU IS ON" (bit 9 clear): "SET IMUSE
 * FLAG" and back (SWRETURN); "NO, SEND ALARM AND EXIT" (210, GOTOPOOH). */
void imuchk(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT9);
    switch (ccs_register(A_ADDRESS)) {                   /* "IS IMU ON" */
    case POSITIVE: tc_to(IMUCHK + 7u, UPFLAG); break;    /* ADRES IMUSE */
    case IS_PLUS_ZERO: tc_to(IMUCHK + 4u, ALARM); break; /* OCT 210 */
    default: not_modelled();
    }
}

void imuchk_after_alarm(void)
{
    tc_to(IMUCHK + 6u, GOTOPOOH);
}

void imuchk_after_upflag(void)
{
    tc_to(IMUCHK + 011u, SWRETURN);
}

/* ---- LSORIENT -- p. 983, bank 4 ---- */

/* LSORIENT: the desired stable member's Z axis normal to the CSM's orbit
 * (RRECTCSM x VRECTCSM, then x XSMD) to ZSMD, and Y (ZSMD x XSMD) to
 * YSMD. */
static const word lsorient_words[] = {
    /* LSORIENT */ ORDERS(STQ, VLOAD),
    ADDRESS(QMAJ),
    ADDRESS_PLUS_ONE(RRECTCSM),
    ORDERS(VXV, VXV),
    ADDRESS_PLUS_ONE(VRECTCSM),
    ADDRESS_PLUS_ONE(XSMD),
    ORDER(UNIT),
    STORE(ZSMD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(XSMD),
    STCALL(YSMD),
    ADDRESS(QMAJ),
};

const struct interpretive_code LSORIENT_CODE = {04u, LSORIENT, COUNT(lsorient_words),
                                                lsorient_words};

#endif
