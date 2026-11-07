/* SPDX-License-Identifier: MIT */
/* SERVICER207.agc, pp. 822-827 (Comanche055, bank 37): PREREAD, which
 * starts average G, and READACCS, the task that reads the PIPAs every two
 * seconds and sets up the SERVICER job; with them the entry's half-second
 * PIPA readings for the downlink (ONMNITOR, QUIKREAD). Restart group 5
 * protects them: its phase is set here by NEWPHASE, or by hand (-PHASE5
 * and PHASE5 from A and L). The faithful C is c64-port/src/cm_servicer.c. */
#include "servicer_cm.h"
#include "../agc/names.h"

#if COMANCHE055

static void nosavpip(void);
static void avegout(void);
static void onmnitor(void);

/* ---- PREREAD -- pp. 822-823 ---- */

/* PREREAD ("CALLER MUST PROTECT PREREAD"): the LASTBIAS job, "DO LAST
 * GYRO COMPENSATION IN FREE FALL" (NOVAC at priority 21). Neither the call
 * nor LASTBIAS is protected: "REREADAC SETS 1/PIPADT TO 2.0 SECS IN CASE
 * LASTBIAS LOST." */
void preread(void)
{
    set_A(PRIO21);
    tc_to(PREREAD + 1u, NOVAC);
}

/* REDO5.31: PREREAD1, which comes back here through RUPTREG1, ... */
void redo5_31(void)
{
    tc_at(REDO5_31);
    preread1();
}

/* ... then the NORMLIZE job (FINDVAC at priority 32): "SET UP NORMLIZE JOB
 * REQUIRED PRIOR TO FIRST AVERAGE G PASS"; ... */
void redo5_31_after_preread1(void)
{
    set_A(PRIO32);
    tc_to(REDO5_31 + 2u, FINDVAC);
}

/* ... READACCS in two seconds (WAITLIST); ... */
void redo5_31_after_findvac(void)
{
    set_A(TWO_SECS);
    tc_to(REDO5_31 + 6u, WAITLIST);
}

/* ... phase 5.2 (NEWPHASE: CS TWO, a negative phase, sets TBASE5 too);
 * ... */
void redo5_31_after_waitlist(void)
{
    set_A(negate(TWO));
    tc_to(REDO5_31 + 012u, NEWPHASE);
}

/* ... and the task's end. */
void redo5_31_after_newphase(void)
{
    continue_at(TASKOVER);
}

/* PREREAD1, a subroutine (its return kept in RUPTREG1): "CLEAR + READ PIPS
 * LAST TIME IN FREE FALL" (PIPASR), and on. */
void preread1(void)
{
    qxch(ADDRESS_OF(RUPTREG1));
    tc_at(PREREAD1 + 2u);
    pipasr();
    preread1_after_pipasr();
}

/* PIPAGE 1, "SET UP PIPAGE FOR REREADAC IN CASE A RESTART OCCURS BEFORE
 * READACCS"; "SET AVEG FLAG" (FLAGWRD1 bit 1); "KNOCK DOWN DRIFT FLAG"
 * (FLAGWRD2 bit 15); "SET V37 FLAG" (FLAGWRD7 bit 6); "CLEAR DVTOTAL"; and
 * back (TC RUPTREG1). */
void preread1_after_pipasr(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(PIPAGE));

    cs(ADDRESS_OF(FLAGWRD1));
    mask(BIT1);
    ads(ADDRESS_OF(FLAGWRD1));

    set_A(POSMAX);
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));

    cs(ADDRESS_OF(FLAGWRD7));
    mask(BIT6);
    ads(ADDRESS_OF(FLAGWRD7));

    set_A(ZERO);
    ts(ADDRESS_OF(DVTOTAL));
    ts(ADDRESS_OF(DVTOTAL) + 1u);

    tc_to_return(ADDRESS_OF(RUPTREG1), fetch(ADDRESS_OF(RUPTREG1)));
}

/* ---- READACCS -- pp. 824-826 ---- */

/* READACCS: the PIPAs read (PIPASR), and on at PIPSDONE. */
void readaccs(void)
{
    tc_at(READACCS);
    pipasr();
    pipsdone();
}

/* PIPSDONE: phase 5.5, by hand: 5 into PHASE5 (from L), its complement
 * into -PHASE5 (from A). */
void pipsdone(void)
{
    set_A(FIVE);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE5));
    redo5_5();
}

/* REDO5.5: PIPAGE 1, "SHOW PIPS HAVE BEEN READ", and PIPCTR 2, "SET PIPCTR
 * FOR ONMINTOR AFTER ABOVE PHASCHNG". With CM/DSTBY (CM/FLAGS bit 2)
 * clear, CHEKAVEG. (The CCS A has a MASK's word: never negative.) */
void redo5_5(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(PIPAGE));
    set_A(TWO);
    ts(ADDRESS_OF(PIPCTR));

    cs(ADDRESS_OF(CM_FLAGS));
    mask(BIT2); /* CM/DSTBY */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(REDO5_5 + 7u);
        chekaveg();
        return;
    }
    redo5_5_entry_dap();
}

/* REDO5.5 +010, "CONTINUE FOR ENTRY DAP": TBASE6 "FOR RESTARTS" (the time
 * of the reading, negated), and the DAP's attitude inputs as of the
 * reading (AOG, AIG, AMG, ROLL/180, ALFA/180 and BETA/180 into the /PIP
 * words). "DURING ENTRY, WHEN RCS DAP IS INACTIVE," (CM/DAPARM, CM/FLAGS
 * bit 12, clear) "SAVE PIPAS EACH 0.5 SEC FOR TM.": QUIKREAD in half a
 * second (WAITLIST). */
void redo5_5_entry_dap(void)
{
    cs(ADDRESS_OF(PIPTIME1) + 1u);
    ts(ADDRESS_OF(TBASE6));
    dca(ADDRESS_OF(AOG));
    dxch(ADDRESS_OF(AOG_PIP));
    ca(ADDRESS_OF(AMG));
    xch(ADDRESS_OF(AMG_PIP));
    dca(ADDRESS_OF(ROLL_180));
    dxch(ADDRESS_OF(ROLL_PIP));
    ca(ADDRESS_OF(BETA_180));
    xch(ADDRESS_OF(BETA_PIP));

    ca(ADDRESS_OF(CM_FLAGS));
    mask(BIT12);
    if (A_is_zero()) {
        nosavpip();
        return;
    }
    set_A(POINT5_SEC);
    tc_to(REDO5_5 + 031u, WAITLIST);
}

/* "SAVE PIPAS AS READ (BUT NOT COMPENSATED)": each into X, Y or ZPIPBUF,
 * whose last word goes to X, Y or ZOLDBUF ("NO NEED TO RESTART PROTECT
 * THIS."). */
void redo5_5_after_quikread(void)
{
    ca(ADDRESS_OF(DELVX));
    xch(ADDRESS_OF(XPIPBUF));
    ts(ADDRESS_OF(XOLDBUF));

    ca(ADDRESS_OF(DELVY));
    xch(ADDRESS_OF(YPIPBUF));
    ts(ADDRESS_OF(YOLDBUF));

    ca(ADDRESS_OF(DELVZ));
    xch(ADDRESS_OF(ZPIPBUF));
    ts(ADDRESS_OF(ZOLDBUF));
    nosavpip();
}

/* NOSAVPIP: CM/GYMDT 5, and SETJTAG in JTAGTIME: "ACTIVATE CM/RCS AFTER
 * PIPUP TO GO IN JTAGTIME +5 CS." (WAITLIST); ... */
static void nosavpip(void)
{
    set_A(FIVE);
    ts(ADDRESS_OF(CM_GYMDT));
    set_A(JTAGTIME);
    tc_to(NOSAVPIP + 3u, WAITLIST);
}

/* ... "1.3SPOT FOR SETJTAG" (NEWPHASE, CS THREE for group 1); ... */
void nosavpip_after_waitlist(void)
{
    set_A(negate(THREE));
    tc_to(NOSAVPIP + 7u, NEWPHASE);
}

/* ... and group 5's phase 37 by hand (OCT37), on to CHEKAVEG. */
void nosavpip_after_newphase(void)
{
    set_A(OCT37);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE5));
    chekaveg();
}

/* CHEKAVEG: "IF AVEG FLAG DOWN SET FINAL EXIT AVEG" (AVEGOUT); else
 * READACCS again. (A MASK's word again for the CCS.) */
void chekaveg(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(BIT1);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(CHEKAVEG + 3u);
        avegout();
        return;
    }
    recall_readaccs();
}

/* CHEKAVEG +4: READACCS in two seconds (WAITLIST), and on at MAKESERV. */
void recall_readaccs(void)
{
    set_A(TWO_SECS);
    tc_to(CHEKAVEG + 5u, WAITLIST);
}

/* MAKESERV: "ESTABLISH SERVICER ROUTINE" (FINDVAC at priority 20); ... */
void makeserv(void)
{
    set_A(PRIO20);
    tc_to(MAKESERV + 1u, FINDVAC);
}

/* ... phase 5.4 (NEWPHASE, CS FOUR): "RESTART SERVICER AND READACCS"; ... */
void makeserv_after_findvac(void)
{
    set_A(negate(FOUR));
    tc_to(MAKESERV + 5u, NEWPHASE);
}

/* ... "TURN TEST CONNECTOR OUTBIT ON" (DSALMOUT bit 9), and "END PREVIOUS
 * READACCS WAITLIST TASK". */
void makeserv_after_newphase(void)
{
    set_A(BIT9);
    wor_dsalmout();
    continue_at(TASKOVER);
}

/* AVEGOUT: the last pass's exit, AVGEXIT AVOUTCAD's 2CADR AVGEND; the
 * SERVICER job (MAKESERV), and no more READACCS. */
static void avegout(void)
{
    dca_2cadr(AVOUTCAD);
    dxch(ADDRESS_OF(AVGEXIT));
    makeserv();
}

/* ---- ONMNITOR and QUIKREAD -- p. 827 ---- */

/* ONMNITOR: "A SEQUENCE OF THREE PASSES THROUGH QUICREAD", at half
 * seconds, counted by PIPCTR: one less, and QUIKREAD in half a second
 * (FIXDELAY, DEC 50). */
static void onmnitor(void)
{
    ts(ADDRESS_OF(PIPCTR));
    tc_to(ONMNITOR + 1u, FIXDELAY);
}

/* QUIKREAD: "READS CURRENT PIPS INTO X,Y,ZPIPBUF.  READS OLD X,Y,ZPIPBUF
 * INTO X,Y,ZOLDBUF.", Z, Y and X, RUPTREG1 counting 2, 1, 0 (CHKCTR). Then,
 * PIPCTR positive, ONMNITOR again; +0, the task's end. A negative PIPCTR
 * falls into SERVICER (with A its magnitude less one), and -0 into
 * SERVICER +1 (with A +0). */
void quikread(void)
{
    set_A(TWO);
    do {
        word i = a_word();
        ts(ADDRESS_OF(RUPTREG1));
        ca(indexed(ADDRESS_OF(PIPAX), i)); /* "SAVE ACTUAL PIPAS FOR TM." */
        xch(indexed(ADDRESS_OF(XPIPBUF), i));
        ts(indexed(ADDRESS_OF(XOLDBUF), i));
    } while (ccs_at(ADDRESS_OF(RUPTREG1)) == POSITIVE); /* "LOOP AGAIN" */

    switch (ccs_at(ADDRESS_OF(PIPCTR))) {
    case POSITIVE: onmnitor(); return;
    case IS_PLUS_ZERO: tc_to(CHKCTR + 4u, TASKOVER); return;
    case NEGATIVE: servicer(); return;
    case IS_MINUS_ZERO: servicer_inhint(); return;
    }
}

#endif
