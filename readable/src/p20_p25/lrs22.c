/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 9 (Luminary099 pp. 568-573): LRS22.1 and LRS22.2,
 * R22's data read subroutines.
 *
 * LRS22.1 (bank 32) "READS RENDEZVOUS RADAR RANGE AND RANGE-RATE, TRUNNION
 * AND SHAFT ANGLES, THREE CDU VALUES AND TIME", one sample each, for the
 * measurement incorporation (LSR22.3): the range rate first, then, at the
 * midpoint of that reading (the mark time), the IMU's CDU angles, the time
 * and the radar's own angles, then the range. A change of the range scale
 * on the way (RNGSCFLG) reads the range rate again (CHEXERR). The
 * readings, converted to meters and revolutions, go to RDOTM, RM, RRTRUN
 * and RRSHAFT; the radar's line of sight in navigation base axes (RRNB) to
 * RRBORSIT. LPS20.1 then gives the line of sight from the state vectors at
 * the mark time, turned to navigation base axes with the CDU angles of the
 * mark (TRG*SMNB), and the two must agree within three degrees. It returns
 * to its caller (LRS22.1X, BANKJUMP) with MPAC +0, or 1 for "RR DATA NO
 * GOOD", or 2 for "RR LOS NOT WITHIN THREE DEGREES OF LOS COMPUTED FROM
 * STATE VECTORS".
 *
 * LRS22.2 (bank 24) "CHECKS IF THE RR LOS (I.E., THE RADAR BORESIGHT
 * VECTOR) IS WITHIN 30 DEGREES OF THE LM +Z AXIS": MPAC +0 if it is, 1 if
 * not.
 *
 * As in the other parts, each call out of the file leaves Z at the
 * routine and Q at the word after the TC, and a TC or a branch to a label
 * the faithful port keys a routine at stops there. READRDOT, EREXIT1,
 * OUT22.1 and OUT22.2 have no keys: they run inside their callers'
 * routines. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- LRS22.1 ---- */

/* READRDOT -- p. 569: "READ RANGE-RATE (ONE SAMPLE)". */
static void readrdot(void)
{
    tc_to(READRDOT, BANKCALL); /* CADR RRRDOT */
}

/* OUT22.1 -- p. 571: "EXIT FROM LRS22.1", to the caller's CADR. */
static void out22_1(void)
{
    ca(ADDRESS_OF(LRS22_1X));
    tc_to(OUT22_1 + 1u, BANKJUMP);
}

/* EREXIT1: "SET ERROR CODE" 1, "COULD NOT READ RADAR". */
static void erexit1(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(MPAC));
    tc_at(EREXIT1 + 2u); /* TC OUT22.1 */
    out22_1();
}

/* LRS22.1 -- p. 569: the caller's CADR. */
void lrs22_1(void)
{
    tc_to(LRS22_1, MAKECADR);
}

void lrs22_1_after_makecadr(void)
{
    if (ts(ADDRESS_OF(LRS22_1X))) not_modelled();
    tc_to(LRS22_1 + 2u, DOWNFLAG); /* ADRES RNGSCFLG */
}

/* LRS22.1 +4: "GET RR RANGE SCALE FROM CHANNEL 33 BIT 3" into RADMODES's
 * bit 3 (RRRSBIT), then READRDOT. (An overflow would skip the CS and the
 * RELINT, as the listing runs it.) */
void lrs22_1_after_downflag(void)
{
    inhint();
    set_A(BIT3);
    rand_channel(033);
    if (!ts(L_ADDRESS)) set_A(negate(RRRSBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ad_at(L_ADDRESS);
    if (!ts(ADDRESS_OF(RADMODES))) relint();
    readrdot();
}

void readrdot_after_rrrdot(void)
{
    tc_to(READRDOT + 2u, BANKCALL); /* CADR RADSTALL: "WAIT FOR DATA READ COMPLETION" */
}

/* READRDOT +4: RADSTALL's bad return, "COULD NOT READ RADAR-ERROR EXIT 1". */
void readrdot_failed(void)
{
    erexit1();
}

/* READRDOT +5: "NO INTERRUPTS WHILE READING TIME AND CDU". "SET MARK TIME
 * EQUAL TO THE MID-POINT" (TIMEHOLD) of the range rate reading, kept in
 * MPAC +5 ("TEMP BUFFER FOR DOWNLINK"); "SAVE RANGE-RATE READING" to
 * RDOTMSAV; "SAVE ICDU ANGLES" (CDUY, CDUZ to MPAC +3, CDUX to MPAC +2);
 * "SAVE TIME" (TIME2, TIME1) in MPAC; and the radar's trunnion and shaft
 * angles "FOR RRNB" to TANG. Then "READ RR RANGE (ONE SAMPLE)". */
void readrdot_read(void)
{
    inhint();
    dxch(ADDRESS_OF(TIMEHOLD));
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(ADDRESS_OF(SAMPLSUM));
    dxch(ADDRESS_OF(RDOTMSAV));
    dca(ADDRESS_OF(CDUY)); /* EXTEND; DCA CDUY */
    dxch(ADDRESS_OF(MPAC) + 3u);
    ca(ADDRESS_OF(CDUX));
    if (ts(ADDRESS_OF(MPAC) + 2u)) not_modelled();
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(CDUT));
    dxch(ADDRESS_OF(TANG));
    relint();
    tc_to(READRDOT + 026u, BANKCALL); /* CADR RRRANGE */
}

void readrdot_after_rrrange(void)
{
    tc_to(READRDOT + 030u, BANKCALL); /* CADR RADSTALL: "WAIT FOR READ COMPLETE" */
}

/* READRDOT +032: RADSTALL's bad return, "CHECK FOR ERRORS DURING READ". */
void readrdot_range_failed(void)
{
    tc_at(READRDOT + 032u);
    continue_at(CHEXERR);
}

/* READRDOT +033: "COPY CYCLE FOR MARK DATA ON DOWNLINK": the raw range
 * and range rate (DNRRANGE) to RANGRDOT, the mark time to MKTIME, the CDU
 * angles to AIG, AMG and AOG, and the radar's angles to TANGNB ("PRESERVE
 * TANG"). The time, swapped out of MPAC by the first DXCH, is the
 * interpretive code's first operand. */
void readrdot_range_read(void)
{
    inhint();
    dxch(ADDRESS_OF(DNRRANGE));
    dxch(ADDRESS_OF(RANGRDOT));
    dxch(ADDRESS_OF(MPAC) + 5u);
    dxch(ADDRESS_OF(MKTIME));
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(AIG)); /* CDUY, CDUZ */
    dca(ADDRESS_OF(TANG));
    dxch(ADDRESS_OF(TANGNB));
    ca(ADDRESS_OF(MPAC) + 2u);
    if (ts(ADDRESS_OF(AOG))) not_modelled(); /* CDUX */
    tc_at(READRDOT + 047u);
    intpret();
}

/* READRDOT +050 -- p. 570: "SAVE TIME OF CDU READINGS IN 20D"; the range
 * rate "SCALED B-28, -.6278 FT./SECOND PER BIT" to "METERS/CENTISECOND,
 * B-7" (RDOTCONV) in RDOTM; the radar's trunnion and shaft angles to
 * revolutions (CDULOGIC) in RRTRUN and RRSHAFT; the range to "METERS,
 * SCALED -29" (RANGCONV) in RM; "COMPUTE RADAR LOS USING RRNB" in
 * RRBORSIT; then "GET STATE VECTOR LOS AT TIME OF CDU READ" (LPS20.1). */
static const word lrs22_1_convert_words[] = {
    STODL_D(20),
    ADDRESS_PLUS_ONE(RDOTMSAV), /* "CONVERT RDOT UNITS AND SCALING" */
    ORDERS(SL, DMPR),
    SL_COUNT(14),
    ADDRESS_PLUS_ONE(RDOTCONV),
    STORE(RDOTM),
    ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE(TANG), /* "GET TRUNNION ANGLE" */
    ADDRESS(CDULOGIC),
    STORE(RRTRUN),
    ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE_AT(TANG, 1), /* "DITTO FOR SHAFT ANGLE" */
    ADDRESS(CDULOGIC),
    STODL(RRSHAFT),
    ADDRESS_PLUS_ONE(SAMPLSUM),
    ORDERS(DMP, SL2R), /* "CONVERT UNITS AND SCALING DP RANGE" */
    ADDRESS_PLUS_ONE(RANGCONV),
    STCALL(RM),
    ADDRESS(RRNB),
    STODL(RRBORSIT),
    ADDRESS_PLUS_ONE_D(20),
    STCALL(TDEC1),
    ADDRESS(LPS20_1),
    ORDER(EXIT),
};

/* READRDOT +0100: "STORE IMU CDU ANGLES AT MARKTIME IN CDUSPOT FOR
 * TRG*SMNB". (An overflow would skip the next CA, as the listing runs
 * it.) */
void lrs22_1_mark_angles(void)
{
    ca(ADDRESS_OF(AIG));
    if (!ts(ADDRESS_OF(CDUSPOT))) ca(ADDRESS_OF(AMG));
    if (!ts(ADDRESS_OF(CDUSPOT) + 2u)) ca(ADDRESS_OF(AOG));
    if (ts(ADDRESS_OF(CDUSPOT) + 4u)) not_modelled();
    tc_at(READRDOT + 0106u);
    intpret();
}

/* READRDOT +0107 -- p. 571: "ROTATE LOS AT MARKTIME FROM SM TO NB.", "DOT
 * WITH RADAR LOS TO GET ANGLE BETWEEN THEM" ("STORE FOR POSSIBLE DISPLAY"
 * in DSPTEM1): "IS IT LESS THAN 3 DEGREES" (THREEDEG); "YES -- NORMAL
 * EXIT". */
static const word lrs22_1_check_words[] = {
    ORDERS(VLOAD, CALL), /* "LOAD VECTOR AND CALL TRANSFORMATION" */
    ADDRESS_PLUS_ONE(RRTARGET), ADDRESS(TRG_STARSMNB), ORDER(DOT),       ADDRESS_PLUS_ONE(RRBORSIT),
    ORDERS(SL1, ACOS),          STORE(DSPTEM1),        ORDERS(DSU, BMN), ADDRESS_PLUS_ONE(THREEDEG),
    ADDRESS(NORMEXIT),          ORDER(EXIT), /* "ERROR EXIT 2" */
};

/* NORMEXIT -3: "SET ERROR CODE" 2. */
void lrs22_1_error_2(void)
{
    set_A(BIT2);
    ts(ADDRESS_OF(MPAC));
    out22_1();
}

static const word normexit_words[] = {
    /* NORMEXIT */ ORDER(EXIT), /* "NORMAL EXIT -- SET MPAC EQUAL ZERO" */
};

void normexit_after_exit(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC));
    out22_1();
}

/* CHEXERR: "CHECK IF RANGE SCALE CHANGED" (RNGSCFLG): "YES -- TAKE ANOTHER
 * READING"; else EREXIT1. (A, one bit of FLAGWRD5, is never negative.) */
void chexerr(void)
{
    ca(ADDRESS_OF(FLAGWRD5)); /* CAE */
    mask(RNGSCBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: readrdot(); return;
    case IS_PLUS_ZERO: erexit1(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* ---- LRS22.2 ---- */

/* OUT22.2 -- p. 573. */
static void out22_2(void)
{
    ca(ADDRESS_OF(LRS22_1X));
    tc_to(OUT22_2 + 1u, BANKJUMP);
}

/* LRS22.2 -- p. 572: the caller's CADR. */
void lrs22_2(void)
{
    tc_to(LRS22_2, MAKECADR);
}

void lrs22_2_after_makecadr(void)
{
    if (ts(ADDRESS_OF(LRS22_1X))) not_modelled();
    tc_at(LRS22_2 + 2u);
    intpret();
}

/* 30DEGCHK: "CHECK IF RR LOS IS WITHIN 30 DEG OF THE SPACECRAFT +Z AXIT
 * BY TAKING ARCCOS OF Z-COMP. OF THE RR LOS VECTOR" (RRBORSIT +4): "NORMAL
 * EXIT -- WITHIN 30 DEG." (OKEXIT), else "ERROR EXIT -- NOT WITHIN 30
 * DEG.". */
static const word thirty_degchk_words[] = {
    /* 30DEGCHK */ ORDERS(DLOAD, ACOS),
    ADDRESS_PLUS_ONE_AT(RRBORSIT, 4),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(30DEG),
    ADDRESS(OKEXIT),
    ORDER(EXIT),
};

/* OKEXIT -3: "SETS ERROR CODE IN MPAC". */
void lrs22_2_error(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(MPAC));
    out22_2();
}

static const word okexit_words[] = {
    /* OKEXIT */ ORDER(EXIT), /* "NORMAL EXIT -- SET MPAC = ZERO" */
};

void okexit_after_exit(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC));
    out22_2();
}

/* ---- The interpretive code ---- */

const struct interpretive_code LRS22_1_CONVERT_CODE = {
    032u, READRDOT + 050u, COUNT(lrs22_1_convert_words), lrs22_1_convert_words};
const struct interpretive_code LRS22_1_CHECK_CODE = {
    032u, READRDOT + 0107u, COUNT(lrs22_1_check_words), lrs22_1_check_words};
const struct interpretive_code NORMEXIT_CODE = {032u, NORMEXIT, COUNT(normexit_words),
                                                normexit_words};
const struct interpretive_code THIRTY_DEGCHK_CODE = {024u, LRS22_2 + 3u, COUNT(thirty_degchk_words),
                                                     thirty_degchk_words};
const struct interpretive_code OKEXIT_CODE = {024u, OKEXIT, COUNT(okexit_words), okexit_words};

#endif
