/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 5 (Comanche055 pp. 759-763): R54, alias
 * CHKSDATA, the star data test; CAL53A, the coarse alignment if one is
 * needed, and MATMOVE (bank 14); RDCDUS, the CDUs read for an RTB (bank
 * 15); and CALCSMSC, the navigation base in stable member axes (bank 16).
 *
 * Each call out of the file (BANKCALL, PHASCHNG, CURTAINS) leaves Z at
 * the routine and Q at the word after the TC, where the routine comes
 * back to; GOTOPOOH doesn't come back. The faithful port keys a routine
 * at each such return, at each TC's return in the file, and at each TC
 * INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void chksda(void);

/* ---- R54, CHKSDATA -- pp. 759-761 (bank 14) ---- */

/* CHKSDATA checks "A PAIR OF STAR SIGHTINGS": the angle between STARAD
 * and STARAD +6 to THETA (20D) on the first pass, kept in 18D, and
 * between 6D and 12D on the second; their difference, made positive, to
 * NORMTEM1, for the display. FREEFLAG is set for the first pass, flipped
 * for the second (INVERT), and set again for the crew's answer. */
static const word chksdata_words[] = {
    /* CHKSDATA */ ORDERS(STQ, SET),
    ADDRESS(QMIN),
    SWITCH(SET, FREEFLAG),
    /* CHKSAB */ ORDER(AXC_1), /* "SET X1 TO STORE EPHEMERIS DATA" */
    ADDRESS(STARAD),
    /* CHKSB */ ORDERS(VLOAD_STAR, DOT_STAR), /* "CAL. ANGLE THETA" */
    ADDRESS_PLUS_ONE_X1(0),
    ADDRESS_PLUS_ONE_X1(6),
    ORDERS(SL1, ACOS),
    STORE_D(20),          /* THETA */
    ORDERS(BOFF, INVERT), /* "BRANCH TO CHKSD IF THIS IS 2ND PASS" */
    SWITCH(BOFF, FREEFLAG),
    ADDRESS(CHKSD),
    SWITCH(INVERT, FREEFLAG), /* "CLEAR FREEFLAG" */
    ORDERS(AXC_1, DLOAD),     /* "SET X1 TO MARK ANGLES" */
    NUMBER(6),
    ADDRESS_PLUS_ONE_D(20), /* THETA */
    STORE_D(18),
    ORDER(GOTO),
    ADDRESS(CHKSB), /* "RETURN TO CAL. 2ND ANGLE" */
    /* CHKSD */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(20), /* THETA, "COMPUTE POS DIFF" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(ABS, RTB),
    ADDRESS(SGNAGREE),
    STORE(NORMTEM1),
    ORDERS(SET, EXIT),
    SWITCH(SET, FREEFLAG),
};

const struct interpretive_code CHKSDATA_CODE = {014u, CHKSDATA, COUNT(chksdata_words),
                                                chksdata_words};

/* The display cleared (CLEANDSP), then the difference flashed (VB6N5):
 * terminate, proceed (FREEFLAG stays set: CHKSDA, back to the caller) or
 * enter (FREEFLAG cleared: sight again). */
void chksdata_after_exit(void)
{
    set_A(ZERO);
    tc_to(CHKSDATA + 035u, BANKCALL); /* CADR CLEANDSP */
}

void chksdata_after_cleandsp(void)
{
    set_A(VB6N5);
    tc_to(CHKSDATA + 040u, BANKCALL); /* CADR GOFLASH */
}

void chksdata_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void chksdata_proceed(void)
{
    tc_at(CHKSDATA + 043u); /* TC CHKSDA */
    chksda();
}

void chksdata_enter(void)
{
    tc_at(CHKSDATA + 044u);
    intpret();
}

static const word chksdata_enter_words[] = {
    /* CHKSDATA +045 */ ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, FREEFLAG),
    ADDRESS(QMIN),
};

const struct interpretive_code CHKSDATA_ENTER_CODE = {
    014u, CHKSDATA + 045u, COUNT(chksdata_enter_words), chksdata_enter_words};

/* CHKSDA: back to the caller (QMIN). */
static void chksda(void)
{
    tc_at(CHKSDA);
    intpret();
}

static const word chksda_words[] = {
    /* CHKSDA +1 */ ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code CHKSDA_CODE = {014u, CHKSDA + 1u, COUNT(chksda_words), chksda_words};

/* ---- CAL53A, MATMOVE -- pp. 761-762 (bank 14) ---- */

/* CAL53A: "COARSE ALIGN THE IMU, IF NECESSARY". The gimbal angles once
 * more (S52.2, "MAKE FINAL COMP OF GIMBAL ANGLES", to THETAD), the CDUs
 * now (RDCDUS, to the work area's words 1-3; S1 1); then, for each of the
 * three (X1 from 3), the difference of the two, each halved, made
 * positive: less than DEGREE1, or more than DEG359, and the axis needs no
 * coarse alignment (CALOOP1). An axis that needs it: COARFINE. */
static const word cal53a_words[] = {
    /* CAL53A */ ORDER(CALL),
    ADDRESS(S52_2),
    ORDERS(RTB, SSP),
    ADDRESS(RDCDUS), /* "READ CDUS" */
    ADDRESS_PLUS_ONE(S1),
    NUMBER(1),
    ORDERS(AXT_1, SETPD),
    NUMBER(3),
    ADDRESS_PLUS_ONE_D(4),
    /* CALOOP */ ORDERS(DLOAD_STAR, SR1),
    ADDRESS_PLUS_ONE_AT_X1(THETAD, 3),
    ORDERS(PDDL_STAR, SR1),
    ADDRESS_PLUS_ONE_X1(4),
    ORDERS(DSU, ABS),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(DEGREE1),
    ORDERS(BMN, DLOAD),
    ADDRESS(CALOOP1),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(DEG359),
    ADDRESS(CALOOP1),
    /* COARFINE */ ORDER(EXIT),
};

const struct interpretive_code CAL53A_CODE = {014u, CAL53A, COUNT(cal53a_words), cal53a_words};

/* COARFINE: a restart point (PHASCHNG, 04024); then IMUCOARS, "PERFORM
 * COARSE ALIGNMENT", and IMUFIN20, the IMU back to fine align, each
 * waited for by IMUSTALL (CURTAINS when it fails). */
void coarfine_after_exit(void)
{
    tc_to(COARFINE + 1u, PHASCHNG); /* OCT 04024 */
}

void coarfine_after_phaschng(void)
{
    tc_to(COARFINE + 3u, BANKCALL); /* CADR IMUCOARS */
}

void coarfine_after_imucoars(void)
{
    tc_to(COARFINE + 5u, BANKCALL); /* CADR IMUSTALL */
}

void coarfine_coarse_failed(void)
{
    tc_to(COARFINE + 7u, CURTAINS);
}

void coarfine_coarse_aligned(void)
{
    tc_to(COARFINE + 010u, BANKCALL); /* CADR IMUFIN20 */
}

void coarfine_after_imufin20(void)
{
    tc_to(COARFINE + 012u, BANKCALL); /* CADR IMUSTALL */
}

void coarfine_fine_failed(void)
{
    tc_to(COARFINE + 014u, CURTAINS); /* "TEST FOR MALFUNCTION" */
}

void coarfine_fine(void)
{
    tc_at(COARFINE + 015u);
    intpret();
}

/* The drift reset (SET1/PDT; GCOMP zeroed, DRIFTFLG set), and FINEONLY.
 * CALOOP1: the next axis. FINEONLY: the new orientation (XSM) to
 * REFSMMAT (MATMOVE), and back (CAL53RET). MATMOVE, "TRANSFER MATRIX":
 * three vectors from X1's address to X2's. */
static const word coarfine_words[] = {
    /* COARFINE +016 */ ORDERS(RTB, VLOAD),
    ADDRESS(SET1_PDT),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(GCOMP),
    ORDERS(SET, GOTO),
    SWITCH(SET, DRIFTFLG),
    ADDRESS(FINEONLY),
    /* CALOOP1 */ ORDER(TIX_1),
    ADDRESS(CALOOP),
    /* FINEONLY */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSM),
    ADDRESS(REFSMMAT),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDER(GOTO),
    ADDRESS(CAL53RET),
    /* MATMOVE */ ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_X1(0),
    STORE_X2_D(0),
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_X1(6),
    STORE_X2_D(6),
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_X1(12),
    STORE_X2_D(12),
    ORDER(RVQ),
};

const struct interpretive_code COARFINE_CODE = {014u, COARFINE + 016u, COUNT(coarfine_words),
                                                coarfine_words};

/* ---- RDCDUS -- p. 762 (bank 15) ---- */

/* RDCDUS, "READ CDUS", an RTB's routine: CDUX, CDUY and CDUZ to the work
 * area's words 1-3, with interrupts held off; back to DANZIG. */
void rdcdus(void)
{
    inhint();
    ca(ADDRESS_OF(CDUX));
    ts(indexed(1u, FIXLOC)); /* INDEX FIXLOC; TS 1 */
    ca(ADDRESS_OF(CDUY));
    ts(indexed(2u, FIXLOC)); /* INDEX FIXLOC; TS 2 */
    ca(ADDRESS_OF(CDUZ));
    ts(indexed(3u, FIXLOC)); /* INDEX FIXLOC; TS 3 */
    relint();
    tc_to(RDCDUS + 013u, DANZIG);
}

/* ---- CALCSMSC -- p. 763 (bank 16) ---- */

/* CALCSMSC: the navigation base's axes in stable member axes, from the
 * gimbal angles' sines and cosines (CDUTRIG's, SINCDUX to COSCDUZ),
 * each made of the three rotations: XNB, then ZNB (26D the sine of X
 * times the sine of Z), and YNB, ZNB crossed with XNB. The vectors are
 * built at half scale (SR1, VDEF) and doubled (VSL1). */
static const word calcsmsc_words[] = {
    /* CALCSMSC */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(SINCDUY),
    ADDRESS_PLUS_ONE(COSCDUZ),
    ORDER(DCOMP),
    ORDERS(PDDL, SR1),
    ADDRESS_PLUS_ONE(SINCDUZ),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(COSCDUY),
    ADDRESS_PLUS_ONE(COSCDUZ),
    ORDERS(VDEF, VSL1),
    STORE(XNB),
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(SINCDUX),
    ADDRESS_PLUS_ONE(SINCDUZ),
    ORDER(SL1),
    STORE_D(26),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(SINCDUY),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(COSCDUX),
    ADDRESS_PLUS_ONE(COSCDUY),
    ORDER(DSU),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(SINCDUX),
    ADDRESS_PLUS_ONE(COSCDUZ),
    ORDER(DCOMP),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(COSCDUX),
    ADDRESS_PLUS_ONE(SINCDUY),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(COSCDUY),
    ADDRESS_PLUS_ONE_D(26),
    ORDERS(DAD, VDEF),
    ORDER(VSL1),
    STORE(ZNB),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(XNB),
    STORE(YNB),
    ORDER(RVQ),
};

const struct interpretive_code CALCSMSC_CODE = {016u, CALCSMSC, COUNT(calcsmsc_words),
                                                calcsmsc_words};

#endif
