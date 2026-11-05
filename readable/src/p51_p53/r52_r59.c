/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 6 (Luminary099 pp. 955-960, banks 14 and 15): R52,
 * the automatic optics positioning; and R59, the lunar surface star
 * acquisition.
 *
 * R52, the "AUTOMATIC OPTICS POSITIONING ROUTINE", is to "POINT THE AOT
 * APTICS AXIS BY MANEUVERING THE LEM TO A NAVIGATION" star: the crew
 * picks the star and the AOT detent (V01N70), R52 takes the detent's optic axis
 * (OANB) and the star's direction in stable member axes (PLANET, then
 * REFSMMAT) to POINTVSM, and R60LEM turns the LM. R59, on the moon's
 * surface, where the LM cannot turn: for the star the crew picks (the
 * star code, V01N70*), it looks through the AOT's detents in turn for
 * the one whose field of view holds the star, and shows the cursor and
 * spiral angles that will find it there (V06N79). Both then sight
 * through AOTMARK (R53).
 *
 * Each call out of the file (BANKCALL, DOWNFLAG, ALARM, CURTAINS,
 * GOTOPOOH) leaves Z at the routine and Q at the word after the TC, where
 * the routine comes back to; GOTOPOOH doesn't come back. The faithful
 * port keys a routine at each such return, at each TC's return in the
 * file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r52a(void);
static void r52b(void);
static void getazel(void);
static void azel(void);
static void r59a(void);
static void incaz(void);
static void r59alm(void);
static void seventy_nine_disp(void);
static void r59e(void);
static void r59out(void);

/* ---- R52 -- pp. 955-956 ---- */

/* R52: the caller's return to SAVQR52. */
static const word r52_words[] = {
    /* R52 */ ORDERS(STQ, EXIT),
    ADDRESS(SAVQR52),
};

const struct interpretive_code R52_CODE = {014u, R52, COUNT(r52_words), r52_words};

/* R52 +2, after the EXIT: "PICK UP STARCODE DETERMINED BY R56" (the star
 * number times 6, BESTI or BESTJ by STARIND), a sixth of it, with "DETENT
 * POSITION 2" (BIT8) to STARCODE. (A is under a sixth plus BIT8: TS does
 * not skip.) */
void r52_starcode(void)
{
    ca(indexed_ca(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    mp(ONE_6TH);                                                   /* EXTEND; MP 1/6TH */
    ad(BIT8);
    ts(ADDRESS_OF(STARCODE));
    r52a();
}

/* R52A: "DISPLAY STARCODE AND WAIT FOR RESPONSE" (V01N70): terminate,
 * "PROCEED TO ORIENT LEM" (R52B), or enter: "SELECT NEW STARCODE --
 * RECYCLE". */
static void r52a(void)
{
    set_A(V01N70);
    tc_to(R52A + 1u, BANKCALL); /* CADR GOFLASH */
}

void r52_terminate(void)
{
    tc_to(R52A + 3u, GOTOPOOH); /* "V34 -- TERMINATE" */
}

void r52_proceed(void)
{
    r52b(); /* TCF R52B */
}

void r52_enter(void)
{
    r52a(); /* TCF R52A */
}

/* R52B: 3AXISFLG cleared "TO ALLOW VECPOINT". */
static void r52b(void)
{
    tc_to(R52B, DOWNFLAG); /* ADRES 3AXISFLG */
}

/* R52B +2: the detent code (STARCODE's bits 7-9) to L. Codes 0 ("COAS
 * CALIBRATION") and 7 ("COAS SIGHTING") ask for the azimuth and the
 * elevation (GETAZEL); any other is a detent of the AOT, whose azimuth
 * (AOTAZ, by the code, in E-bank 7) goes with an elevation of 45
 * degrees (BIT13) to AZEL. (The MP leaves A a word: TS L does not skip.) */
void r52b_detent(void)
{
    ca(ADDRESS_OF(STARCODE));
    mask(HIGH9);
    mp(BIT9); /* EXTEND; MP BIT9 */
    ts(L_ADDRESS);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF GETAZEL */
        getazel();
        return;
    }
    ad(NEG7);
    if (A_is_zero()) { /* EXTEND; BZF GETAZEL */
        getazel();
        return;
    }
    set_A(EBANK7); /* (a constant: TS EBANK does not skip) */
    ts(EBANK_ADDRESS);
    ca(indexed_ca(ADDRESS_OF(AOTAZ) - 1u, overflow_corrected(L))); /* INDEX L; CA AOTAZ -1 */
    ts(L_ADDRESS);
    set_A(EBANK5);
    ts(EBANK_ADDRESS);
    set_A(BIT13);
    xch(L_ADDRESS); /* "SET C(A)=AZ, C(L)=45 DEG" */
    azel();
}

/* GETAZEL: the azimuth and elevation keyed in (V06N87): terminate,
 * proceed ("CALC OPTIC AXIS") or enter (again). */
static void getazel(void)
{
    set_A(V06N87);
    tc_to(GETAZEL + 1u, BANKCALL); /* CADR GOFLASH */
}

void getazel_terminate(void)
{
    tc_to(GETAZEL + 3u, GOTOPOOH); /* "V34 -- TERMINATE" */
}

void getazel_proceed(void)
{
    dca(ADDRESS_OF(AZ)); /* EXTEND; DCA AZ, AZ and EL */
    azel();
}

void getazel_enter(void)
{
    getazel(); /* TCF GETAZEL */
}

/* AZEL: "JAM AZ AND EL IN 8 AND 9 OF VAC" for OANB, which takes the optic
 * axis to SCAXIS; the star's direction (PLANET, at the time now) in
 * stable member axes to POINTVSM, "FOR VECPOINT". */
static void azel(void)
{
    dxch(indexed_dxch(8u, FIXLOC)); /* INDEX FIXLOC; DXCH 8D */
    tc_at(AZEL + 2u);
    intpret();
}

static const word azel_words[] = {
    /* AZEL +3 */ ORDER(CALL),  ADDRESS(OANB),   ORDERS(RTB, CALL),
    ADDRESS(LOADTIME),          ADDRESS(PLANET), ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT), STORE(POINTVSM), ORDER(EXIT),
};

const struct interpretive_code AZEL_CODE = {014u, AZEL + 3u, COUNT(azel_words), azel_words};

/* R60LEM: "GO TORQUE LEM OPTIC AXIS TO STAR LOS". */
void azel_r60lem(void)
{
    tc_to(AZEL + 014u, BANKCALL); /* CADR R60LEM */
}

/* After it: with detent code 0 (COAS calibration), the star code again
 * (R52A); else back to the caller (SAVQR52). */
void azel_after_r60lem(void)
{
    set_A(HIGH9);
    mask_at(ADDRESS_OF(STARCODE));
    if (A_is_zero()) { /* EXTEND; BZF R52A */
        r52a();
        return;
    }
    tc_at(AZEL + 022u);
    intpret();
}

static const word r52_return_words[] = {
    /* AZEL +023 */ ORDER(GOTO),
    ADDRESS(SAVQR52),
};

const struct interpretive_code R52_RETURN_CODE = {014u, AZEL + 023u, COUNT(r52_return_words),
                                                  r52_return_words};

/* ---- R59 -- pp. 957-960 ---- */

/* R59: "IF REFSMMAT FLAG CLEAR BYPASS STAR ACQUIRE" (R59OUT); else "SELECT
 * STAR CODE FOR ACQUISITION" (V01N70*): terminate, proceed (R59A) or
 * recycle. (After the MASK, A is +0 or the bit: CCS takes its first two
 * ways only.) */
void r59(void)
{
    cs(ADDRESS_OF(FLAGWRD3));
    mask(REFSMBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* CCS A */
        r59out();                              /* "NO REFSMMAT GO TO AOTMARK" */
        return;
    }
    set_A(V01N70_STAR);
    tc_to(R59 + 5u, BANKCALL); /* CADR GOFLASH */
}

void r59_terminate(void)
{
    tc_to(R59 + 7u, GOTOPOOH); /* "V34 -- TERMINATE" */
}

void r59_proceed(void)
{
    r59a(); /* TCF R59A */
}

void r59_recycle(void)
{
    r59(); /* TCF R59 */
}

/* R59A: "GRAB STARCODE FOR INDEX" (AOTCODE's bits 1-6), times 6 (REVCNT,
 * "JUST 6"), to BESTI (by STARIND) and X1, "CODE X 6 FOR CATLOG STAR
 * INDEX". "BYPASS ACQUISITION IF NOT CATLOG STAR" (0, or past 37: more
 * than 227); else the star's direction (CATLOG,1) in navigation base
 * axes (CDU*SMNB) to STAR. (A is L, a word: neither TS skips.) */
static void r59a(void)
{
    set_A(negate(HIGH9)); /* CS HIGH9 */
    mask_at(ADDRESS_OF(AOTCODE));
    mp(REVCNT); /* EXTEND; MP REVCNT */
    xch(L_ADDRESS);
    ts(indexed_ts(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    ts(indexed_ts(ADDRESS_OF(X1), FIXLOC));                        /* INDEX FIXLOC */
    if (A_is_zero()) {                                             /* EXTEND; BZF R59OUT */
        r59out();
        return;
    }
    com();
    ad(DEC227);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF R59OUT */
        r59out();
        return;
    }
    tc_at(R59A + 017u);
    intpret();
}

static const word r59a_words[] = {
    /* R59A +020 */ ORDERS(VLOAD_STAR, MXV),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(UNIT, CALL),
    ADDRESS(CDU_STARSMNB),
    STORE(STAR),
    ORDER(EXIT),
};

const struct interpretive_code R59A_CODE = {015u, R59A + 020u, COUNT(r59a_words), r59a_words};

/* "INITIALIZE AZ POSITION COD TO 1 (-60)" (POSCODE). (A is 1: TS does not
 * skip.) */
void r59a_after_exit(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(POSCODE));
    incaz();
}

/* INCAZ: "PICK UP AZ CORRESPONDING TO POSCODE" (AOTAZ, in E-bank 7), with
 * an elevation of 45 degrees, to the work area's 8D and 9D for OANB; the
 * azimuth to QMIN "FOR LATER". (The constants and the word leave A with
 * no overflow: no TS skips.) */
static void incaz(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    ca(indexed_ca(ADDRESS_OF(AOTAZ) - 1u, fetch(ADDRESS_OF(POSCODE)))); /* INDEX POSCODE */
    ts(L_ADDRESS);
    set_A(EBANK5);
    ts(EBANK_ADDRESS);
    set_A(BIT13);
    xch(L_ADDRESS); /* "SET C(A)=AZ, C(L)=45 DEG" */
    ts(ADDRESS_OF(QMIN));
    dxch(indexed_dxch(8u, FIXLOC)); /* INDEX FIXLOC; DXCH 8D */
    tc_at(INCAZ + 014u);
    intpret();
}

/* The optic axis (OANB, to SCAXIS) and the angle between it and the star,
 * to 24D. "NOT IN FIELD" (30 degrees or more): NXAX, "TRY NEXT POSITION";
 * near the centre (under half a degree): ZSPCR. Else the spiral angle,
 * "12(ARCCOS(AO.STAR)) SCALED IN REVS", to 24D, and the cursor angle: the
 * angle about the optic axis from the plane of the axis and the X axis to
 * the star's (THET, "360-THET" past half a revolution), plus the
 * detent's azimuth (QMIN), to CURSOR; the spiral angle plus it to
 * SPIRAL; both one's complement revolutions made two's complement half
 * revolutions (RTB 1STO2S). */
static const word incaz_words[] = {
    /* INCAZ +015 */ ORDER(CALL),
    ADDRESS(OANB),
    ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(STAR),
    ADDRESS_PLUS_ONE(SCAXIS),
    ORDERS(SL1, ARCCOS),
    STORE_D(24),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(DEG30),
    ADDRESS(NXAX),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(24),
    ADDRESS_PLUS_ONE(DEG_5),
    ORDERS(BMN, DLOAD),
    ADDRESS(ZSPCR),
    ADDRESS_PLUS_ONE_D(24),
    ORDERS(DMP, SL4),
    ADDRESS_PLUS_ONE(3_4),
    STOVL_D(24),
    ADDRESS_PLUS_ONE(SCAXIS),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(XUNIT),
    ORDERS(PUSH, VXV),
    ADDRESS_PLUS_ONE(SCAXIS),
    ORDER(VCOMP),
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(SCAXIS),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(STAR),
    ORDERS(PUSH, DOT),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SL1, ARCCOS),
    STOVL_D(26),
    ORDER(DOT),
    ORDERS(BPL, DLOAD),
    ADDRESS(R59D),
    ADDRESS_PLUS_ONE(ABOUTONE),
    ORDER(DSU),
    ADDRESS_PLUS_ONE_D(26),
    STORE_D(26),
    /* R59D */ ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(QMIN),
    ORDERS(DAD, PUSH),
    ADDRESS_PLUS_ONE_D(26),
    ORDER(RTB),
    ADDRESS(1STO2S),
    STODL(CURSOR),
    ADDRESS_PLUS_ONE_D(24),
    ORDER(DAD),
    ORDER(RTB),
    ADDRESS(1STO2S),
    STORE(SPIRAL),
    ORDER(EXIT),
};

const struct interpretive_code INCAZ_CODE = {015u, INCAZ + 015u, COUNT(incaz_words), incaz_words};

/* "GO DISPLAY CURSOR-SPIRAL-POS CODE". */
void r59d_display(void)
{
    seventy_nine_disp(); /* TCF 79DISP */
}

/* ZSPCR: "STAR ALMOST OPTIC AXIS, ZERO CURSOR" "AND SPIRAL ANGLES". (A
 * is +0: TS does not skip.) */
static const word zspcr_words[] = {
    /* ZSPCR */ ORDER(EXIT),
};

const struct interpretive_code ZSPCR_CODE = {015u, ZSPCR, COUNT(zspcr_words), zspcr_words};

void zspcr_plus1(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CURSOR));
    ts(ADDRESS_OF(SPIRAL));
    seventy_nine_disp(); /* TCF 79DISP */
}

/* NXAX: the next detent (POSCODE); past the sixth, "THIS STAR NOT AT ANY
 * POSITION" (R59ALM). */
static const word nxax_words[] = {
    /* NXAX */ ORDER(EXIT),
};

const struct interpretive_code NXAX_CODE = {015u, NXAX, COUNT(nxax_words), nxax_words};

void nxax_plus1(void)
{
    incr(ADDRESS_OF(POSCODE));
    cs(ADDRESS_OF(POSCODE));
    ad(SEVEN);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF R59ALM */
        r59alm();
        return;
    }
    incaz(); /* TCF INCAZ */
}

/* R59ALM: alarm 404, "THIS STAR CAN'T BE LOCATED IN AOT FIELD", shown
 * (VB05N09): terminate, "PROCEED, GO WITHOUT ACQUIRE" (R59OUT), or
 * "RECYCLE AND TRY ANOTHER STAR" (R59). */
static void r59alm(void)
{
    tc_to(R59ALM, ALARM); /* OCT 404 */
}

void r59alm_after_alarm(void)
{
    set_A(VB05N09);
    tc_to(R59ALM + 3u, BANKCALL); /* CADR GOFLASH */
}

void r59alm_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void r59alm_proceed(void)
{
    r59out(); /* TCF R59OUT */
}

void r59alm_recycle(void)
{
    r59(); /* TCF R59 */
}

/* 79DISP: "DISPLAY CURSOR, SPIRAL AND POS CODE" (V06N79): terminate,
 * "PROCEED TO MARK ROUTINE" (R59E), or "RECYCLE TO TOP OF R59 AGAIN". */
static void seventy_nine_disp(void)
{
    set_A(V06N79);
    tc_to(SEVENTY_NINE_DISP + 1u, BANKCALL); /* CADR GOFLASH */
}

void seventy_nine_disp_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void seventy_nine_disp_proceed(void)
{
    r59e(); /* TCF R59E */
}

void seventy_nine_disp_recycle(void)
{
    r59(); /* TCF R59 */
}

/* R59E: "GET DETENT CODE CORRESPONDING TO POSCODE", in bits 7-9 (MP BIT7
 * leaves it in L), with the star number (AOTCODE's bits 1-6) to AOTCODE.
 * (A is under 0777: TS does not skip.) */
static void r59e(void)
{
    set_A(SEVEN);
    mask_at(ADDRESS_OF(POSCODE));
    mp(BIT7);             /* EXTEND; MP BIT7 */
    set_A(negate(HIGH9)); /* CS HIGH9 */
    mask_at(ADDRESS_OF(AOTCODE));
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(AOTCODE));
    r59out();
}

/* R59OUT: "GO TO AOTMARK FOR SIGHTING"; AOTSTALL waits ("SLEEP TILL
 * SIGHTING DONE"), CURTAINS on its "BADEND RETURN". Then R59RET, in P57
 * (part 9), back to the one star or two star sighting. */
static void r59out(void)
{
    tc_to(R59OUT, BANKCALL); /* CADR AOTMARK */
}

void r59out_after_aotmark(void)
{
    tc_to(R59OUT + 2u, BANKCALL); /* CADR AOTSTALL */
}

void r59out_badend(void)
{
    tc_to(R59OUT + 4u, CURTAINS);
}

#endif
