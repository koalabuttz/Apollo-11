/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 7 (Comanche055 pp. 772-777): R53, the
 * sighting mark routine (bank 14); S52.2, the gimbal angles for the
 * desired stable member (bank 11); and SR52.1, the optics angles to a
 * target (bank 13).
 *
 * Each call out of the file (BANKCALL, CHECKMM, CURTAINS, FALTON,
 * MAKECADR, SWCALL, GOTOPOOH) leaves Z at the routine and Q at the word
 * after the TC, where the routine comes back to; GOTOPOOH doesn't come
 * back. The faithful port keys a routine at each such return, at each
 * TC's return in the file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r53a(void);
static void r53b(void);
static void r53c(void);
static void sr52e1(void);

/* INDEX k; TS address: the index added to the whole instruction word
 * (indexed_instruction). Not modelled: an index that turns the TS into
 * another instruction, or one that stores to EBANK, FBANK, Z or BBANK. */
static unsigned indexed_ts(unsigned address, word index)
{
    word instruction = indexed_instruction((word)(054000u | address), index);

    if ((instruction & 076000u) != 054000u) not_modelled();
    address = instruction & 01777u;
    if (address >= 3u && address <= 6u) not_modelled();
    return address;
}

/* ---- R53 -- pp. 772-773 (bank 14) ---- */

/* R53, "SIGHTING MARK ROUTINE": the caller's return to R53EXIT, and
 * R53FLAG set, "SET SIGHTING MARK FLAG". */
static const word r53_words[] = {
    /* R53 */ ORDERS(STQ, SET),
    ADDRESS(R53EXIT),
    SWITCH(SET, R53FLAG), /* "SET SIGHTING MARK FLAG" */
    ORDER(EXIT),
};

const struct interpretive_code R53_CODE = {014u, R53, COUNT(r53_words), r53_words};

/* R53A: the marks, MARKINDX's low three bits, "NUMBER OF MARKS", by
 * SXTMARK; OPTSTALL waits for them (CURTAINS when it fails). */
static void r53a(void)
{
    ca(ADDRESS_OF(MARKINDX)); /* "NUMBER OF MARKS" */
    mask(LOW3);
    tc_to(R53A + 2u, BANKCALL); /* CADR SXTMARK */
}

void r53_after_exit(void)
{
    r53a();
}

void r53a_after_sxtmark(void)
{
    tc_to(R53A + 4u, BANKCALL); /* CADR OPTSTALL */
}

void r53a_failed(void)
{
    tc_to(R53A + 6u, CURTAINS);
}

/* The marks done: the mark area's QPRET (INDEX MARKSTAT), "NUMBER OF
 * MARKS ACTUALLY DONE": some, R53B. None (or the "CCS HOLE"): "HOUSEKEEP
 * VAC AREA SAVE AND MARKSTAT": MARKSTAT to zero, and the area given back
 * (MARKSTAT less one, by CCS, stored at that address: INDEX A; TS 0);
 * then the marks again. (Not modelled: an index that turns the CCS into
 * another instruction, a TCF too.) */
void r53a_marked(void)
{
    word instruction = indexed_instruction((word)(010000u | ADDRESS_OF(QPRET)),
                                           fetch(ADDRESS_OF(MARKSTAT))); /* INDEX MARKSTAT */

    if ((instruction & 076000u) != 010000u) not_modelled(); /* (past 01777, a TCF) */
    switch (ccs_at(ADDRESS_FIELD(instruction))) {           /* CCS QPRET */
    case POSITIVE: r53b(); return;                          /* TCF R53B */
    default: break;                                         /* TCF +2, TCF +1 ("CCS HOLE"), or on */
    }
    set_A(ZERO); /* "HOUSEKEEP VAC AREA SAVE" */
    xch(ADDRESS_OF(MARKSTAT));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        if (ts(indexed_ts(0u, A))) { /* INDEX A; TS 0 */
            r53b();
            return;
        }
        break;
    case IS_PLUS_ZERO:
        if (ts(A_ADDRESS)) { /* TS 0 */
            r53b();
            return;
        }
        break;
    case NEGATIVE: break;
    default: r53b(); return;
    }
    r53a(); /* TCF R53A */
}

/* R53B: in P22 or P23 (CHECKMM), straight to R53D; else the display
 * cleared (R53C1, CLEANDSP) and the star's code flashed. */
static void r53b(void)
{
    tc_to(R53B, CHECKMM); /* MM 22 */
}

void r53b_not_p22(void)
{
    tc_to(R53B + 4u, CHECKMM); /* TCF +2; TC CHECKMM; MM 23 */
}

void r53b_p22(void)
{
    r53d(); /* TCF R53D */
}

void r53b_not_p23(void)
{
    r53c1(); /* TCF R53C1 */
}

void r53b_p23(void)
{
    r53d(); /* TCF R53D */
}

void r53c1(void)
{
    set_A(ZERO);
    tc_to(R53C1 + 1u, BANKCALL); /* CADR CLEANDSP */
}

/* R53C: the star's code flashed (V01N71, GOFLASH): terminate, proceed
 * (CHKSCODE checks the code, at R53Z) or enter, "RECYCLE". */
static void r53c(void)
{
    set_A(V01N71);
    tc_to(R53C + 1u, BANKCALL); /* CADR GOFLASH */
}

void r53c_after_cleandsp(void)
{
    r53c();
}

void r53c_terminate(void)
{
    tc_to(R53C + 3u, GOTOPOOH); /* "TERM." */
}

void r53c_proceed(void)
{
    r53z(); /* TCF R53Z */
}

void r53c_enter(void)
{
    tc_at(R53C + 5u); /* TC R53C, "RECYCLE" */
    r53c();
}

/* R53Z: CHKSCODE; its return (+0), a code over 50 or below 0: the
 * operator error light (FALTON), and the code flashed again. */
void r53z(void)
{
    tc_at(R53Z); /* TC CHKSCODE */
    if (!chkscode()) r53z_bad_code();
}

void r53z_bad_code(void)
{
    tc_to(R53Z + 1u, FALTON);
}

void r53z_after_falton(void)
{
    tc_at(R53Z + 2u); /* TC R53C */
    r53c();
}

/* CHKSCODE's return (+2): a good code. The code (its low six bits: CS
 * HIGH9; MASK STARCODE) times SIGHTSIX, the star's index in the catalog,
 * to BESTI or BESTJ (INDEX STARIND), "STAR NUMBER SIGHTED". (MP leaves
 * the product in L, small: TS does not skip.) */
void r53z_code_good(void)
{
    set_A(negate(HIGH9)); /* CS HIGH9 */
    mask_at(ADDRESS_OF(STARCODE));
    mp(SIX);        /* EXTEND; MP SIGHTSIX */
    xch(L_ADDRESS); /* XCH L */
    if (ts(indexed_ts(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND))))) not_modelled();
    r53d();
}

/* R53D: R53OUT sets TERMIFLG, "SET TERMINATE FOR R52", and goes back to
 * the caller (R53EXIT). */
void r53d(void)
{
    tc_at(R53D);
    intpret();
}

static const word r53out_words[] = {
    /* R53OUT */ ORDER(SETGO),
    SWITCH(SETGO, TERMIFLG), /* "SET TERMINATE FOR R52" */
    ADDRESS(R53EXIT),
};

const struct interpretive_code R53OUT_CODE = {014u, R53D + 1u, COUNT(r53out_words), r53out_words};

/* ---- S52.2 -- p. 774 (bank 11) ---- */

/* S52.2: "COMPUTE GIMBAL ANGLES FOR DESIRED SM AND PRESENT VEHICLE". The
 * CDUs' sines and cosines (CDUTRIG) and the navigation base axes in the
 * present stable member (CALCSMSC, to XNB, YNB, ZNB); each of the three
 * (X1 from 18D, by 6) turned into the reference frame (VXM REFSMMAT),
 * unit; then the desired stable member (XSMD, YSMD, ZSMD) to XSM (MATMOVE)
 * and the gimbal angles (CALCGA, to THETAD); back (QMAJ). */
static const word s52_2_words[] = {
    /* S52.2 */ ORDER(STQ),
    ADDRESS(QMAJ),
    ORDER(CALL),
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

const struct interpretive_code S52_2_CODE = {011u, S52_2, COUNT(s52_2_words), s52_2_words};

/* ---- SR52.1 -- pp. 775-777 (bank 13) ---- */

/* SR52.1, called by BANKCALL: the caller's return (MAKECADR) to QMIN;
 * then the target's line of sight. (A as MAKECADR leaves it does not
 * overflow: TS does not skip.) */
void sr52_1(void)
{
    tc_to(SR52_1, MAKECADR);
}

void sr52_1_after_makecadr(void)
{
    if (ts(ADDRESS_OF(QMIN))) not_modelled();
    tc_at(SR52_1 + 2u);
    intpret();
}

/* The time 1.3 seconds on (LOADTIME, 1.3SECDP) to AOPTIME. The LM
 * (TARG1FLG, LEM52): both ships' positions then (LEMCONIC; CSMCONIC, at
 * LMKLMCOM), the line of sight their difference, unit. A landmark
 * (TARG2FLG, LMK52): its position then (LALOTORV, unless R52's tracking,
 * ADVTRK, has it: ADVTRACK), less the CSM's. A star (STAR52): its vector,
 * STARSAV1 or STARSAV2 by STARIND (S1 0; TIX,1). COM52: the line of sight
 * to the stable member (MXV REFSMMAT), unit, to STAR; the CDUs' sines and
 * cosines (CDUTRIG) and the optics angles (CALCSXA, to SAC and PAC).
 * CULTFLAG set (the trunnion past 90 degrees): SR52E1. */
static const word sr52_1_words[] = {
    /* SR52.1 +3 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(1_3SECDP),
    STORE(AOPTIME),
    ORDERS(BON, BON),
    SWITCH(BON, TARG1FLG),
    ADDRESS(LEM52),
    SWITCH(BON, TARG2FLG),
    ADDRESS(LMK52),
    ORDER(GOTO),
    ADDRESS(STAR52),
    /* LEM52 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(AOPTIME),
    STCALL(TDEC1),
    ADDRESS(LEMCONIC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(GOTO),
    ADDRESS(LMKLMCOM),
    /* LMK52 */ ORDERS(BON, DLOAD),
    SWITCH(BON, ADVTRK),
    ADDRESS(ADVTRACK),
    ADDRESS_PLUS_ONE(AOPTIME),
    ORDER(CALL),
    ADDRESS(LALOTORV),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ALPHAV),
    /* LMKLMCOM */ STODL(STAR),
    ADDRESS_PLUS_ONE(AOPTIME),
    STCALL(TDEC1),
    ADDRESS(CSMCONIC),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(STAR),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, GOTO),
    ADDRESS(COM52),
    /* STAR52 */ ORDERS(SSP, LXA_1),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(0),
    ADDRESS(STARIND),
    ORDER(TIX_1),
    ADDRESS(ST52ST),
    ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(STARSAV2),
    ADDRESS(COM52),
    /* ST52ST */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(STARSAV1),
    /* COM52 */ ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(STAR),
    ORDERS(SETPD, CALL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(CDUTRIG), /* "COMPUTES SINES AND COSINES FOR CALCSXA" */
    ORDER(CALL),
    ADDRESS(CALCSXA),
    ORDERS(BOFF, EXIT),
    SWITCH(BOFF, CULTFLAG),
    ADDRESS(TRUN38),
};

const struct interpretive_code SR52_1_CODE = {013u, SR52_1 + 3u, COUNT(sr52_1_words), sr52_1_words};

void sr52_1_cult(void)
{
    tc_at(SR52_1 + 075u); /* TC SR52E1 */
    sr52e1();
}

/* TRUN38: the trunnion angle (PAC) less 38TRDEG ("CORRESPONDS TO 50 DEGS
 * IN TRUNION"): not negative, SR52E22; else PAC less 20DEGSMN (-07199):
 * not negative, SR52E3, and negative, SR52E22 too. */
static const word trun38_words[] = {
    /* TRUN38 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(PAC),
    ADDRESS_PLUS_ONE(38TRDEG),
    ORDERS(BPL, DLOAD),
    ADDRESS(SR52E22),
    ADDRESS_PLUS_ONE(PAC),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(20DEGSMN),
    ADDRESS(SR52E3),
    /* SR52E22 */ ORDER(EXIT),
};

const struct interpretive_code TRUN38_CODE = {013u, TRUN38, COUNT(trun38_words), trun38_words};

static const word sr52e3_words[] = {
    /* SR52E3 */ ORDER(EXIT),
};

const struct interpretive_code SR52E3_CODE = {013u, SR52E3, COUNT(sr52e3_words), sr52e3_words};

/* The returns: SR52E1 to the caller's +2 ("TRUNNION GREATER THAN 90
 * DEG."), SR52E2 to its +3 ("TRUNNION GREATER THAN 50 DEG"), SR52E3 to
 * its +4 ("NORMAL RETURN"): QMIN counted on (INCR), back by SWCALL. */
void sr52e22_after_exit(void)
{
    tc_at(SR52E22 + 1u); /* TC SR52E2 */
    incr(ADDRESS_OF(QMIN));
    sr52e1();
}

void sr52e3_after_exit(void)
{
    incr(ADDRESS_OF(QMIN));
    incr(ADDRESS_OF(QMIN));
    sr52e1();
}

static void sr52e1(void)
{
    ca(ADDRESS_OF(QMIN));
    tc_to(SR52E1 + 1u, SWCALL);
}

#endif
