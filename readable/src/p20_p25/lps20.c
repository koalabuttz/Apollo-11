/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 8 (Luminary099 pp. 564-567): LPS20.1, "VECTOR
 * EXTRAPOLATION AND LOS COMPUTATION", and LPS20.2, the "400 NM RANGE
 * CHECK".
 *
 * LPS20.1 brings both vehicles' state vectors to the time in TDEC1: the
 * LM's by LEMCONIC (unless R21 called it on the lunar surface), kept in
 * LMPOS and LMVEL, then the CSM's by CSMCONIC. The line of sight from the
 * LM to the CSM, in stable member axes, goes to RRTARGET as a unit vector
 * (half unit), and its length to MLOSV; their relative velocity, also in
 * stable member axes, to LOSVEL. On the way it kills the task that starts
 * DODES (DESLOOP +2), since DODES uses those words. With RNDVZFLG set the
 * line of sight is shifted up 9 before UNIT; an overflow there means "RANGE
 * MUST BE GREATER THAN 400 N. M.": alarm 526 (526ALARM). RRNBSW cleared, it
 * returns through the address its STQ kept in LS21X.
 *
 * LPS20.2 compares MLOSV with 400 nautical miles (FHNM) and returns with
 * MPAC 0 when the range is less, 1 when it is as much or more. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- LPS20.1 ---- */

/* LPS20.1 -- p. 565: the return kept in LS21X. "LOSCMFLG = 0 MEANS NOT
 * CALLED BY R21", "SO CALL LEMCONIC TO GET LM STATE"; "IF IN R21 AND ON
 * LUNAR SURFACE" (SURFFLAG) "DON'T CALL LEMCONIC". The LM's position
 * (RATT, "B-29") and velocity (VATT, "B-7") to LMPOS and LMVEL; then the
 * CSM at the same time, TAT (STCALL TDEC1). Their relative velocity,
 * "V(CSM) - V(LM)", to stable member axes. */
static const word lps20_1_words[] = {
    /* LPS20.1 */ ORDERS(STQ, BOFF),
    ADDRESS(LS21X),
    SWITCH(BOF, LOSCMFLG),
    ADDRESS(LMINT),
    ORDER(BON),
    SWITCH(BON, SURFFLAG),
    ADDRESS(CSMINT),
    /* LMINT */ ORDER(CALL),
    ADDRESS(LEMCONIC), /* "EXTRAPOLATE LEM" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(LMPOS), /* "SAVE LM POSITION B-29" */
    ADDRESS_PLUS_ONE(VATT),
    STODL(LMVEL), /* "SAVE LM VELOCITY B-7" */
    ADDRESS_PLUS_ONE(TAT),
    /* CSMINT */ STCALL(TDEC1),
    ADDRESS(CSMCONIC),  /* "EXTRAPOLATE CSM" */
    ORDERS(VLOAD, VSU), /* "COMPUTE RELATIVE VELOCITY V(CSM) - V(LM)" */
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(LMVEL),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(EXIT),
};

/* LPS20.1 +027: "KILL THE TASK WHICH CALLS DODES SINCE STORING INTO
 * ERASEABLES DODES USES". */
void lps20_1_after_exit(void)
{
    tc_to(LPS20_1 + 027u, KILLTASK); /* CADR DESLOOP +2 */
}

void lps20_1_after_killtask(void)
{
    tc_at(LPS20_1 + 031u);
    intpret();
}

/* LPS20.1 +032 -- the relative velocity to LOSVEL; the line of sight, the
 * CSM's position less the LM's. RNDVZFLG clear goes straight to NOTSHIFT;
 * set, an overflow so far goes to TCDANZIG, which only resets OVFIND, and
 * the vector is shifted up 9. Made a unit vector: "IF OVERFLOW, RANGE MUST
 * BE GREATER THAN 400 N. M." (526ALARM). To stable member axes ("CONVERT TO
 * STABLE MEMBER") in RRTARGET, its length (36D) to MLOSV, "SAVE MAGNITUDE
 * OF LOS VECTOR FOR VELOCITY CORRECTION IN DESIGNATE"; RRNBSW cleared, and
 * back through LS21X. */
static const word lps20_1_los_words[] = {
    STOVL(LOSVEL),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(VSU, BOFF),
    ADDRESS_PLUS_ONE(LMPOS),
    SWITCH(BOF, RNDVZFLG),
    ADDRESS(NOTSHIFT),
    ORDER(BOVB),
    ADDRESS(TCDANZIG),
    ORDER(VSL),
    SL_COUNT(9),
    /* NOTSHIFT */ ORDERS(UNIT, BOVB),
    ADDRESS(526ALARM),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STODL(RRTARGET),
    ADDRESS_PLUS_ONE_D(36),
    STORE(MLOSV),
    ORDER(CLRGO),
    SWITCH(CLRGO, RRNBSW),
    ADDRESS(LS21X),
};

/* ---- LPS20.2 ---- */

/* LPS20.2 -- p. 566: "MAGNITUDE OF LOS" less FHNM, "400 NAUTICAL MILES IN
 * METERS B-20": positive or zero (BPL) is "OVER 400NM", TOFAR, MPAC 1
 * (ONE/SP); negative, 0 (ZERO/SP). Both return by RVQ. */
static const word lps20_2_words[] = {
    /* LPS20.2 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(MLOSV),
    ADDRESS_PLUS_ONE(FHNM),
    ORDER(BPL),
    ADDRESS(TOFAR),
    ORDERS(SLOAD, RVQ),
    ADDRESS_PLUS_ONE(ZERO_SP),
    /* TOFAR */ ORDERS(SLOAD, RVQ),
    ADDRESS_PLUS_ONE(ONE_SP),
};

/* ---- The interpretive code ---- */

const struct interpretive_code LPS20_1_CODE = {024u, LPS20_1, COUNT(lps20_1_words), lps20_1_words};
const struct interpretive_code LPS20_1_LOS_CODE = {024u, LPS20_1 + 032u, COUNT(lps20_1_los_words),
                                                   lps20_1_los_words};
const struct interpretive_code LPS20_2_CODE = {025u, LPS20_2, COUNT(lps20_2_words), lps20_2_words};

#endif
