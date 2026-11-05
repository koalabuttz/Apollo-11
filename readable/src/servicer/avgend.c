/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 863-866 (Luminary099 only, banks 22, 27 and 32):
 * AVGEND, "FINAL AVERAGE G EXIT", where AVEGOUT points AVGEXIT to end
 * average G; SERVIDLE, which keeps SERVICER running with no guidance; and
 * SERVEXIT, the job's end. The faithful C is c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* ---- AVGEND -- pp. 863-864 (bank 22) ---- */

/* AVGEND: "SET UP FREE FALL GYRO COMPENSATION." (1/PIPADT the time of the
 * PIPA reading's low word), and "SET DRIFT FLAG." (UPFLAG, ADRES
 * DRIFTFLG); ... */
void avgend(void)
{
    ca(ADDRESS_OF(PIPTIME) + 1u);
    ts(ADDRESS_OF(1_PIPADT));
    tc_to(AVGEND + 2u, UPFLAG);
}

/* ... the PIPAs' free-fall compensation (PIPFREE, through BANKCALL), ... */
void avgend_after_upflag(void)
{
    tc_to(AVGEND + 4u, BANKCALL); /* CADR PIPFREE */
}

/* ... the test connector outbit off (DSALMOUT bit 9), and the restart
 * groups: "GROUP 5 OFF", "GROUP 2 ON" (2PHSCHNG, OCT 5, OCT 05022, OCT
 * 20000); ... */
void avgend_after_pipfree(void)
{
    set_A(negate(BIT9));
    wand_dsalmout();
    tc_to(AVGEND + 011u, TWO_PHSCHNG);
}

/* ... and the interpreter, for AVGEND_CODE. */
void avgend_after_2phschng(void)
{
    tc_at(AVGEND + 015u);
    intpret();
}

/* "SHUT OFF R29 WHEN SERVICER ENDS." (NOR29FLG set), "SHUT OFF R10 WHEN
 * SERVICER ENDS." (SWANDISP clear), "RESET MUNFLAG.", the state vector
 * moved to its permanent place (AVETOMID), and V37FLAG clear. */
static const word avgend_words[] = {
    /* AVGEND +016 */ ORDERS(SET, CLEAR),
    SWITCH(SET, NOR29FLG),
    SWITCH(CLEAR, SWANDISP),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, MUNFLAG),
    ADDRESS(AVETOMID),
    ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, V37FLAG),
};

const struct interpretive_code AVGEND_CODE = {022u, AVGEND + 016u, COUNT(avgend_words),
                                              avgend_words};

/* AVERTRN (OUTGOAVE): "RETURN TO DESIRED POINT." (OUTROUTE's CADR, through
 * BANKJUMP). */
void avertrn(void)
{
    ca(ADDRESS_OF(OUTROUTE));
    tc_to(AVERTRN + 1u, BANKJUMP);
}

/* ---- SERVIDLE -- p. 865 (bank 27) ---- */

/* SERVIDLE: "DISCONNECT SERVICER FROM ALL GUIDANCE" (AVGEXIT gets
 * SVEXTADR's 2CADR SERVEXIT), "DISCONNECT THE DELTA-V MONITOR" (IDLEFLAG
 * set), "TERMINATE R12 IS RUNNING." (FLGWRD11 just LRBYBIT), and the
 * restart groups off: 1, 2 unless MUNFLAG is set ("DO NOT TURN OFF PHASE 2
 * IF MUNFLAG SET."), 3 and 6; group 4 at 4.33, the "4.33SPOT FOR
 * GOP00FIX". Then WHIMPER: "PERFORM A SOFTWARE RESTART AND PROCEED TO
 * GOTOPOOH WHILE SERVICER CONTINUES TO RUN". (The CCS tests a MASK result:
 * positive or +0.) */
void servidle(void)
{
    dca_constant(SVEXTADR);
    dxch(ADDRESS_OF(AVGEXIT));

    cs(ADDRESS_OF(FLAGWRD7));
    mask(IDLEFBIT);
    ads(ADDRESS_OF(FLAGWRD7));

    set_A(LRBYBIT);
    ts(ADDRESS_OF(FLGWRD11));

    group_off(ADDRESS_OF(MINUS_PHASE1));
    ca(ADDRESS_OF(FLAGWRD6));
    mask(MUNFLBIT);
    if (ccs_register(A_ADDRESS) != POSITIVE) group_off(ADDRESS_OF(MINUS_PHASE2));
    group_off(ADDRESS_OF(MINUS_PHASE3));
    group_off(ADDRESS_OF(MINUS_PHASE6));

    set_A(OCT33);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE4));
    continue_at(WHIMPER);
}

/* ---- SERVEXIT -- p. 866 (bank 32) ---- */

/* SERVEXIT: restart group 5 at 5.3 (PHASCHNG, OCT 00035), ... */
void servexit(void)
{
    tc_to(SERVEXIT, PHASCHNG);
}

/* ... and the job ends (SERVEXIT +2, where OUTROUTE points while average
 * G runs: ENDJBCAD). */
void servexit_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

#endif
