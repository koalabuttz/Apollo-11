/* SPDX-License-Identifier: MIT */
/* DAP_INTERFACE_SUBROUTINES.agc, the LM's: Luminary099 pages 1406-1409, bank
 * 20 (the CM has none of it). The calling sequence the listing gives is TC
 * IBNKCALL; FCADR ROUTINE, in an interrupt "OR WITH INTERRUPT INHIBITED".
 * The DAP's deadband, DB, is an angle "SCALED AT 45" (degrees): NARROWDB 0.3
 * degree, WIDEDB 5.0 degrees, POWERDB 1.0 degree. The faithful C is
 * c64-port/src/lm_dap_interface.c. */
#include "dap_interface.h"

#if LUMINARY099

/* TC to a word of fixed memory, the address in the word at address (Q,
 * RUPTREG1 or RUPTREG2: a caller's return). Not modelled: one that is not
 * an address of fixed memory, which the AGC would run as another
 * instruction. */
static void return_through(unsigned address)
{
    word to = fetch(address);

    if (to < 02000u || to > 07777u) not_modelled();
    tc_through(address);
}

/* SETMAXDB, SETMINDB, RESTORDB, PFLITEDB -- LM p. 1407.
 * "ALL ENTRIES SET UP A NOVAC JOB TO DO 1/ACCS SO THAT THE TJETLAW SWITCH
 * CURVES ARE POSITIONED TO REFLECT THE NEW DEADBAND." */

/* CALLACCS -- LM p. 1407: a NOVAC job at priority 27 to 1/ACCJOB (the 2CADR
 * after the TC); NOVAC returns past the 2CADR, to TC RUPTREG1. */
static void callaccs(void)
{
    set_A(PRIO27);
    tc_to(CALLACCS + 1u, NOVAC);
}

/* SETMAXDB +1 -- LM p. 1407: the deadband in A to DB; the caller's return
 * kept in RUPTREG1 ("SET UP JOB TO RE-POSITION SWITCH CURVES"). */
static void set_db(void)
{
    ts(ADDRESS_OF(DB));
    qxch(ADDRESS_OF(RUPTREG1));
    callaccs();
}

/* RESTORDB -- LM p. 1407: "SET DEADBAND TO MAX OR MIN ACCORDING TO SETTINGS
 * OF DBSELECT BIT OF DAPBOOLS". */
void restordb(void)
{
    ca(ADDRESS_OF(DAPBOOLS)); /* "DETERMINE CREW-SELECTED DEADBAND." */
    restordb_plus1();
}

void restordb_plus1(void)
{
    mask(DBSELECT);
    if (A == 0u) /* EXTEND; BZF SETMINDB */
        setmindb();
    else
        setmaxdb();
}

/* SETMAXDB -- LM p. 1407: "SET DEADBAND TO 5.0 DEGREES". */
void setmaxdb(void)
{
    set_A(WIDEDB); /* CAF WIDEDB */
    set_db();
}

/* SETMINDB -- LM p. 1407: "SET DEADBAND TO 0.3 DEGREE". */
void setmindb(void)
{
    set_A(NARROWDB); /* CAF NARROWDB */
    set_db();        /* TCF SETMAXDB +1 */
}

/* CALLACCS +4 -- LM p. 1407, where NOVAC returns: "RETURN TO CALLER." */
void callaccs_after_novac(void)
{
    return_through(ADDRESS_OF(RUPTREG1)); /* TC RUPTREG1 */
}

/* PFLITEDB -- LM p. 1407: "SET DEADBAND TO 1.0 DEGREE AND ZERO THE COMMANDED
 * ATTITUDE CHANGE AND COMMANDED RATE". The caller's return in RUPTREG1, for
 * CALLACCS ("THE RETURN FROM CALLACCS IS TO RUPTREG1."); then ZATTEROR. */
void pflitedb(void)
{
    qxch(ADDRESS_OF(RUPTREG1));
    tc_to(PFLITEDB + 2u, ZATTEROR); /* "ZERO THE ERRORS AND COMMANDED RATES." */
}

/* PFLITEDB +3 -- LM p. 1407, where ZATTEROR returns. */
void pflitedb_after_zatteror(void)
{
    set_A(POWERDB); /* "SET DB TO 1.0 DEG." */
    ts(ADDRESS_OF(DB));
    callaccs(); /* TCF CALLACCS: "SET UP 1/ACCS AND RETURN TO CALLER." */
}

/* ZATTEROR and STOPRATE -- LM p. 1408. Both work in EBANK 6 and keep the
 * caller's EBANK in L meanwhile ("SAVE CALLERS EBANK IN L."). */
static void save_ebank(void)
{
    set_A(EBANK6);
    xch(EBANK_ADDRESS);
    ts(L_ADDRESS);
}

/* STOPRATE +3 -- LM p. 1408: the commanded rates OMEGAPD, OMEGAQD and
 * OMEGARD, the commanded attitude changes DELCDUX, DELCDUY and DELCDUZ and
 * the attitude error offsets DELPEROR, DELQEROR and DELREROR to +0 (A +0);
 * the caller's EBANK back ("RESTORE CALLERS EBANK.", L left with EBANK6),
 * and TC Q. */
static void stoprate_plus3(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(OMEGAPD));
    ts(ADDRESS_OF(OMEGAQD));
    ts(ADDRESS_OF(OMEGARD));
    ts(ADDRESS_OF(DELCDUX));
    ts(ADDRESS_OF(DELCDUY));
    ts(ADDRESS_OF(DELCDUZ));
    ts(ADDRESS_OF(DELPEROR));
    ts(ADDRESS_OF(DELQEROR));
    ts(ADDRESS_OF(DELREROR));
    lxch(EBANK_ADDRESS);
    return_through(Q_ADDRESS); /* TC Q */
}

/* ZATTEROR -- LM p. 1408: the desired CDU angles (CDUXD, CDUYD, CDUZD) set
 * to the CDUs now, so that the attitude error is zero; then STOPRATE's
 * clears. */
void zatteror(void)
{
    save_ebank();
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUXD));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUYD));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUZD));
    stoprate_plus3(); /* TCF STOPRATE +3 */
}

void stoprate(void)
{
    save_ebank();
    stoprate_plus3();
}

/* ALLCOAST -- LM pp. 1408-1409. "WILL BE CALLED BY FRESH STARTS AND ENGINE
 * OFF ROUTINES." The caller's return kept in RUPTREG2 ("SAVE Q FOR
 * RETURN"); then STOPRATE. */
void allcoast(void)
{
    qxch(ADDRESS_OF(RUPTREG2));
    tc_to(ALLCOAST + 2u, STOPRATE); /* "CLEAR RATE INTERFACE." */
}

/* ALLCOAST +3 -- LM p. 1409, where STOPRATE returns ("RETURN WITH A=0", and
 * L EBANK6):
 * in EBANK 6
 * (the caller's EBANK in L meanwhile) the offset accelerations AOSQ and
 * AOSR (double words), ALPHAQ and ALPHAR ("FOR DOWNLIST.") and AOSQTERM
 * and AOSRTERM to +0. Then DRIFTBIT of DAPBOOLS set (CS; MASK; ADS adds
 * the bit only where it is clear), and RESTORDB +1 with A that bit, to
 * "RESTORE DEADBANK TO CREW-SELECTED VALUE." Not modelled: an A that
 * overflows, which STOPRATE never leaves (each TS would skip). */
void allcoast_after_stoprate(void)
{
    if (overflow_of(A)) not_modelled();
    lxch(EBANK_ADDRESS);
    ts(ADDRESS_OF(AOSQ));
    ts(ADDRESS_OF(AOSQ) + 1u);
    ts(ADDRESS_OF(AOSR));
    ts(ADDRESS_OF(AOSR) + 1u);
    ts(ADDRESS_OF(ALPHAQ));
    ts(ADDRESS_OF(ALPHAR));
    ts(ADDRESS_OF(AOSQTERM));
    ts(ADDRESS_OF(AOSRTERM));
    lxch(EBANK_ADDRESS); /* "RESTORE EBANK (EBANK6 NO LONGER NEEDED)" */

    cs(ADDRESS_OF(DAPBOOLS)); /* "SET UP DRIFTBIT" */
    mask(DRIFTBIT);
    ads(ADDRESS_OF(DAPBOOLS));
    tc_to(ALLCOAST + 020u, RESTORDB + 1u);
}

/* ALLCOAST +021 -- LM p. 1409, where RESTORDB +1 returns (by CALLACCS and
 * RUPTREG1). */
void allcoast_after_restordb(void)
{
    return_through(ADDRESS_OF(RUPTREG2)); /* "RETURN." */
}

#endif
