/* SPDX-License-Identifier: MIT */
/* SERVICER207.agc, pp. 832-834 (Comanche055, bank 37): the PIPA reader,
 * PIPASR, a "SUBROUTINE TO READ PIPA COUNTERS, TRYING TO BE VERY CAREFUL
 * SO THAT IT WILL BE RESTARTABLE.", and its restart, REREADAC. The
 * faithful C is c64-port/src/cm_servicer.c.
 *
 * Each PIPA counter is read and set to -0 in one exchange (DXCH PIPAX,
 * XCH PIPAZ), with its complement kept in TEMX, TEMY or TEMZ first: those
 * start at -0, and DELVX, DELVY and DELVZ at +0. So after a restart
 * REREADAC can tell, from which of them have changed, how far the reading
 * got. */
#include "servicer_cm.h"
#include "../agc/names.h"

#if COMANCHE055

static void repip1(void);
static void repip3(void);
static void dodelvz(void);
static void chktemx(void);

/* "EXIT IS VIA Q." */
static void return_to_q(void)
{
    tc_to_return(Q_ADDRESS, overflow_corrected(Q));
}

/* ---- PIPASR -- pp. 832-833 ---- */

/* PIPASR: the time of the reading (TIME2, TIME1) into PIPTIME1, the
 * "CURRENT TIME POSITIVE VALUE"; TEMX, TEMY and TEMZ -0 ("INITIALIZE THESE
 * AT NEG ZERO."), DELV +0 (DELVX's high word is written by the reading:
 * "LOW ORDER DELV'S ARE ZEROED FOR TM"), and PIPAGE +0: "SHOW PIPA READING
 * IN PROGRESS"; ... */
void pipasr(void)
{
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(PIPTIME1));

    set_A(negate(ZERO));
    ts(ADDRESS_OF(TEMX));
    ts(ADDRESS_OF(TEMY));
    ts(ADDRESS_OF(TEMZ));

    set_A(ZERO);
    ts(ADDRESS_OF(DELVZ));
    ts(ADDRESS_OF(DELVY));
    ts(ADDRESS_OF(DELVX) + 1u);
    ts(ADDRESS_OF(DELVY) + 1u);
    ts(ADDRESS_OF(DELVZ) + 1u);
    ts(ADDRESS_OF(PIPAGE));
    repip1();
}

/* ... REPIP1: "X AND Y PIPS READ": their complements into TEMX and TEMY,
 * then the counts out and -0 in ("PIPAS SET TO NEG ZERO AS READ."), into
 * DELVX and DELVY; ... */
static void repip1(void)
{
    dcs(ADDRESS_OF(PIPAX));
    dxch(ADDRESS_OF(TEMX));
    dxch(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(DELVX));
    lxch(ADDRESS_OF(DELVY));
    repip3();
}

/* ... REPIP3: "REPEAT PROCESS FOR Z PIP", ... */
static void repip3(void)
{
    cs(ADDRESS_OF(PIPAZ));
    xch(ADDRESS_OF(TEMZ));
    xch(ADDRESS_OF(PIPAZ));
    dodelvz();
}

/* ... DODELVZ: into DELVZ, and back (TC Q). */
static void dodelvz(void)
{
    ts(ADDRESS_OF(DELVZ));
    return_to_q();
}

/* ---- REREADAC -- pp. 833-834 ---- */

/* REREADAC: the "LAST PASS CHECK": with PHASE5 +0 (group 5 off), the
 * task's end. Else 1/PIPADT two seconds, as LASTBIAS would have left it:
 * "RESTART MAY HAVE WIPED OUT LASTBIAS, AN UNPROTECTED NOVAC FROM
 * PREREAD,"; "(PRIO31 IS 2.0SEC SC.AT B+8CS)". (A PHASE5 of -0 skips the
 * CAF: 1/PIPADT gets the +0 the CCS leaves.) */
void rereadac(void)
{
    switch (ccs_at(ADDRESS_OF(PHASE5))) {
    case IS_PLUS_ZERO: continue_at(TASKOVER); return;
    case IS_MINUS_ZERO: break;
    case POSITIVE:
    case NEGATIVE: set_A(PRIO31); break;
    }
    ts(ADDRESS_OF(1_PIPADT));

    /* With PIPAGE positive, "PIP READING NOT STARTED.  GO TO BEGINNING"
     * (READACCS). Else the reading goes on where it stopped, its return
     * PIPSDONE ("SET UP RETURN FROM PIPASR": DONEADR's GENADR in Q). (CCS
     * PIPAGE's other outcomes, as the AGC runs them: negative, Q gets
     * PIPAGE's magnitude less one; -0, Q is left as it is.) */
    switch (ccs_at(ADDRESS_OF(PIPAGE))) {
    case POSITIVE: readaccs(); return;
    case IS_PLUS_ZERO:
        set_A(DONEADR);
        ts(Q_ADDRESS);
        break;
    case NEGATIVE: ts(Q_ADDRESS); break;
    case IS_MINUS_ZERO: break;
    }
    /* DELVZ +0: "Z NOT DONE, CHECK Y."; anything else, "Z DONE, GO DO
     * CDUS" (back, TC Q). */
    if (ccs_at(ADDRESS_OF(DELVZ)) != IS_PLUS_ZERO) {
        return_to_q();
        return;
    }
    /* DELVY +0: "Y NOT DONE, CHECK X." (CHKTEMX, L +0). Else "Y DONE, ZERO
     * Z PIP.", its count to L. */
    zl();
    if (ccs_at(ADDRESS_OF(DELVY)) == IS_PLUS_ZERO) {
        chktemx();
        return;
    }
    lxch(ADDRESS_OF(PIPAZ));
    /* "TEMZ NOT = -0, CONTAINS -PIPAZ VALUE.": the count is its
     * complement (CS TEMZ; but +0 for TEMZ +0, which the CCS leaves in A),
     * into DELVZ. "TEMZ = -0, L HAS ZPIP VALUE.": that, and back. */
    switch (ccs_at(ADDRESS_OF(TEMZ))) {
    case POSITIVE:
    case NEGATIVE: cs(ADDRESS_OF(TEMZ)); break;
    case IS_PLUS_ZERO: break;
    case IS_MINUS_ZERO:
        lxch(ADDRESS_OF(DELVZ));
        return_to_q();
        return;
    }
    dodelvz();
}

/* CHKTEMX: TEMX -0, unchanged: X and Y not read (REPIP1). Else ("YES")
 * their counts are the complements of TEMX and TEMY (+0 for TEMX +0, as
 * the CCS leaves it), into DELVX and DELVY; "ZERO X AND Y PIPS" (-0 and
 * the +0 left in L: "L STILL ZERO FROM ABOVE"), and Z (REPIP3). */
static void chktemx(void)
{
    switch (ccs_at(ADDRESS_OF(TEMX))) { /* "HAS THIS CHANGED" */
    case POSITIVE:
    case NEGATIVE: cs(ADDRESS_OF(TEMX)); break;
    case IS_PLUS_ZERO: break;
    case IS_MINUS_ZERO: repip1(); return; /* "NO" */
    }
    ts(ADDRESS_OF(DELVX));
    cs(ADDRESS_OF(TEMY));
    ts(ADDRESS_OF(DELVY));
    set_A(negate(ZERO));
    dxch(ADDRESS_OF(PIPAX));
    repip3();
}

#endif
