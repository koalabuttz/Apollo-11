/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 4 (Luminary099 pp. 945-948, bank 14): R55, the gyro
 * torque; R54, alias CHKSDATA, the star data test; CAL53A, the coarse
 * alignment if one is needed and the fine alignment; MATMOVE; and RDCDUS,
 * the CDUs read for an RTB.
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

/* ---- R55 -- p. 945 ---- */

/* R55, "GYRO TORQUE": "COMPUTE AND SEND GYRO PULSES". The caller's
 * return to QMIN, and the gyro torque angles (CALCGTA, from XDC, YDC and
 * ZDC, "REFSMMAT WRT PRESENT STABLE MEMBER"). */
static const word r55_words[] = {
    /* R55 */ ORDER(STQ),     ADDRESS(QMIN), ORDER(CALL), ADDRESS(CALCGTA),
    /* PULSEM */ ORDER(EXIT),
};

const struct interpretive_code R55_CODE = {014u, R55, COUNT(r55_words), r55_words};

/* R55.1: the angles flashed (V06N93): terminate, proceed (torque, R55.2)
 * or enter (no torque, R55RET). */
void r55_1(void)
{
    set_A(V06N93);
    tc_to(R55_1 + 1u, BANKCALL); /* CADR GOFLASH */
}

void r55_terminate(void)
{
    tc_to(R55_1 + 3u, GOTOPOOH);
}

void r55_proceed(void)
{
    tc_at(R55_1 + 4u);
    r55_2();
}

void r55_enter(void)
{
    tc_at(R55_1 + 5u);
    r55ret();
}

/* R55.2: a restart point (PHASCHNG, 00214); then IMUPULSE torques the
 * gyros (OGC, R55CDR) and IMUSTALL waits (CURTAINS when it fails); then
 * a restart point again (04024). */
void r55_2(void)
{
    tc_to(R55_2, PHASCHNG); /* OCT 00214 */
}

void r55_torque(void)
{
    set_A(R55CDR);
    tc_to(R55_2 + 3u, BANKCALL); /* CADR IMUPULSE */
}

void r55_after_imupulse(void)
{
    tc_to(R55_2 + 5u, BANKCALL); /* CADR IMUSTALL */
}

void r55_failed(void)
{
    tc_to(R55_2 + 7u, CURTAINS);
}

void r55_torqued(void)
{
    tc_to(R55_2 + 010u, PHASCHNG); /* OCT 04024 */
}

/* R55RET: back to the caller (QMIN). */
void r55ret(void)
{
    tc_at(R55RET);
    intpret();
}

static const word r55ret_words[] = {
    /* R55RET +1 */ ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code R55RET_CODE = {014u, R55RET + 1u, COUNT(r55ret_words), r55ret_words};

/* ---- R54, CHKSDATA -- pp. 945-947 ---- */

/* CHKSDATA "CHECKS THE VALIDITY OF A PAIR OF STAR SIGHTINGS": the angle
 * between the two marks (STARAD, STARAD +6) to THETA (20D) on the first
 * pass, kept in 18D, and between the two catalog stars (6D, 12D) on the
 * second; "THE ANGULAR DIFFERENCE" to NORMTEM1, for the display. FREEFLAG
 * is set for the first pass, flipped for the second (INVERT), and set
 * again for the crew's answer. */
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
    ADDRESS_PLUS_ONE_D(20), /* THETA */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(ABS, RTB), /* "COMPUTE POS DIFF" */
    ADDRESS(SGNAGREE),
    STORE(NORMTEM1),
    ORDERS(SET, EXIT),
    SWITCH(SET, FREEFLAG),
};

const struct interpretive_code CHKSDATA_CODE = {014u, CHKSDATA, COUNT(chksdata_words),
                                                chksdata_words};

/* The difference flashed (VB6N5): terminate, proceed (FREEFLAG stays set:
 * on to the torquing, CHKSDA) or enter (FREEFLAG cleared: mark again). */
void chksdata_after_exit(void)
{
    set_A(VB6N5);
    tc_to(CHKSDATA + 035u, BANKCALL); /* CADR GOFLASH */
}

void chksdata_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void chksdata_proceed(void)
{
    tc_at(CHKSDATA + 040u); /* TC CHKSDA */
    chksda();
}

void chksdata_enter(void)
{
    tc_at(CHKSDATA + 041u);
    intpret();
}

static const word chksdata_enter_words[] = {
    /* CHKSDATA +042 */ ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, FREEFLAG),
    ADDRESS(QMIN),
};

const struct interpretive_code CHKSDATA_ENTER_CODE = {
    014u, CHKSDATA + 042u, COUNT(chksdata_enter_words), chksdata_enter_words};

/* CHKSDA: back to the caller (QMIN). */
void chksda(void)
{
    tc_at(CHKSDA);
    intpret();
}

static const word chksda_words[] = {
    /* CHKSDA +1 */ ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code CHKSDA_CODE = {014u, CHKSDA + 1u, COUNT(chksda_words), chksda_words};

/* ---- CAL53A -- pp. 947-948 ---- */

/* CAL53A: "COMPUTE DESIRED GIMBAL ANGLES AND COARSE ALIGN IF NECESSARY".
 * The gimbal angles once more (S52.2, to THETAD), the CDUs now (RDCDUS,
 * to the work area's words 1-3; S1 1); then, for each of the three
 * (X1 from 3), the difference of the two, each halved, made positive;
 * less than DEGREE1, or more than DEG359, and the axis needs no coarse
 * alignment (CALOOP1). */
static const word cal53a_words[] = {
    /* CAL53A */ ORDER(CALL),
    ADDRESS(S52_2), /* "MAKE ONE FINAL COMP OF GIMBAL ANGLES" */
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
    ORDER(EXIT),
};

const struct interpretive_code CAL53A_CODE = {014u, CAL53A, COUNT(cal53a_words), cal53a_words};

/* An axis that needs it: a restart point (PHASCHNG, 04024), then the
 * coarse alignment (COARFINE). */
void cal53a_after_exit(void)
{
    tc_to(CAL53A + 026u, PHASCHNG); /* OCT 04024 */
}

void coarfine(void)
{
    tc_at(COARFINE - 1u);
    intpret();
}

/* COARFINE: the coarse alignment (COARSE) and the drift (NCOARSE), then
 * FINEONLY; CALOOP1, the next axis. FINEONLY: the new orientation (XSM)
 * to REFSMMAT (MATMOVE), and back (COARSRET). */
static const word coarfine_words[] = {
    /* COARFINE */ ORDER(CALL),
    ADDRESS(COARSE),
    ORDER(CALL),
    ADDRESS(NCOARSE),
    ORDER(GOTO),
    ADDRESS(FINEONLY),
    /* CALOOP1 */ ORDER(TIX_1),
    ADDRESS(CALOOP),
    /* FINEONLY */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSM),
    ADDRESS(REFSMMAT),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDER(GOTO),
    ADDRESS(COARSRET),
};

const struct interpretive_code COARFINE_CODE = {014u, COARFINE, COUNT(coarfine_words),
                                                coarfine_words};

/* ---- MATMOVE, RDCDUS -- p. 948 ---- */

/* MATMOVE: "TRANSFER MATRIX", three vectors from X1's address to X2's. */
static const word matmove_words[] = {
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

const struct interpretive_code MATMOVE_CODE = {014u, MATMOVE, COUNT(matmove_words), matmove_words};

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

#endif
