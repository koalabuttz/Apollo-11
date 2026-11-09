/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part F (Comanche055 pp. 1089-1091, bank
 * 15): the dead zone's end of the 2 s roll system, and "CM ENTRY FDAI
 * DISPLAY".
 *
 * "CALCULATE BY INTEGRATION THE ROLL ERROR BETWEEN THE 2 SEC CM/RCS
 * UPDATES." CM/FDAI comes each 0.1 s (EXDAPIN, part C); CM/FDAIR after
 * the roll system (ROLLDUMP). "THE SUBROUTINE  NEEDLER  EXPECTS
 * (ANGLE/180) AND SCALES TO 16.875 DEG FULL SCALE." CM/DUMPR, on
 * alternate passes, drives the needles or files the body rates for
 * telemetry in UPBUFF (CMTMTIME, SW/NDX and ENDBUF); CM/END ends the
 * 0.1 s cycle. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void cm_dumpr(void);

/* INDEX k: the erasable address base + C(k), at least lowest. A store
 * into FB, Z or BB is not modelled. */
static unsigned indexed_store(word base, word index, unsigned lowest)
{
    int at = (int)base + value_of(index);

    if (at < (int)lowest || at > 01777 || at == (int)FBANK_ADDRESS || at == (int)Z_ADDRESS ||
        at == (int)BBANK_ADDRESS)
        not_modelled();
    return (unsigned)at;
}

/* ---- DZCALL -- p. 1089 ---- */

/* DZCALL +3: "ERASABLE ORDER:  ROLLTM, ROLLC, ROLLC +1.": "GET ROLL/180
 * OR ROLLC (/360)." (INDEX A; CA ROLLTM), */
void dzcall_rolltm(void)
{
    index_a_ca(ADDRESS_OF(ROLLTM));
    dzcall_ts_a();
}

/* DZCALL +5: "IF C(L)=1, STORE 'ROLLC' IN 'L'." (INDEX L; TS A; an
 * overflow skips the AD L), */
void dzcall_ts_a(void)
{
    if (ts(indexed_store(A_ADDRESS, overflow_corrected(L), A_ADDRESS))) {
        tc_to(DZCALL + 010u, ANGOVCOR);
        return;
    }
    dzcall_ad_l();
}

/* DZCALL +7: and L added, "(BOTH MUST BE SCALED DEG/180)": "C(A)=ROLL/180
 * OR 2 ROLLC." through ANGOVCOR (part A), back at DZCALL +011. */
void dzcall_ad_l(void)
{
    ad_at(L_ADDRESS);
    tc_to(DZCALL + 010u, ANGOVCOR);
}

/* DZCALL +011: "IF CMDAPMOD =-0, SAVE ROLL ANGLE." "OTHERWISE, SAVE ROLL
 * COMMAND." (ROLLHOLD); "COME HERE IF IN DZ, AND CANCEL JETS." (CA ZERO;
 * an overflow at the TS skips it, and A +1 or -1 goes on), "TURN OFF ALL
 * ROLL JETS." (WRITE ROLLJETS, "INHINT NOT NEEDED HERE."), and VDT/180,
 * "SET =0 TO SHOW IN DEAD ZONE."; */
void dzcall_rollhold(void)
{
    if (!ts(ADDRESS_OF(ROLLHOLD))) set_A(ZERO); /* CA ZERO */
    write_channel(6);                           /* EXTEND; WRITE ROLLJETS */
    ts(ADDRESS_OF(VDT_180));
    dzcall1();
}

/* DZCALL1: "COME HERE WITH C(A)=0." (JETAG; an overflow skips the TC, on
 * at CM/FDAI); TC ROLLDUMP (a key of another of the faithful port's
 * parts). */
void dzcall1(void)
{
    if (ts(ADDRESS_OF(JETAG))) {
        cm_fdai();
        return;
    }
    tc_to(DZCALL1 + 1u, ROLLDUMP);
}

/* ---- CM/FDAI -- pp. 1090-1091 ---- */

/* CM/FDAI: "COME HERE EACH .1 SEC.  (CMDAPMOD=+1 COMES BELOW)": "INTEGRATE
 * ROLL ERROR 'TWEEN 2SEC UPDATES": -PHIDOT, */
void cm_fdai(void)
{
    cs(ADDRESS_OF(PHIDOT));
    cm_fdai_mp();
}

/* CM/FDAI +1: times CALFA, "FOR ASSUMED COORDINATION.", and HALF, */
void cm_fdai_mp(void)
{
    mp_at(ADDRESS_OF(CALFA)); /* EXTEND; MP CALFA */
    mp(HALF);                 /* EXTEND; MP HALF */
    cm_fdai_ads();
}

/* CM/FDAI +5: added to PAXERR1, "ROLL ERROR/360.  OVFL OK.", */
void cm_fdai_ads(void)
{
    ads(ADDRESS_OF(PAXERR1));
    cm_fdair();
}

/* CM/FDAIR: HALF times PAXERR1, "FULL SCALE FOR FDAI (ROLL) IS 67.5 D", to
 * PAXERR, ".25 (ROLL ERROR/180) FOR FDAI NEEDLE."; */
void cm_fdair(void)
{
    set_A(HALF);                /* CA HALF */
    mp_at(ADDRESS_OF(PAXERR1)); /* EXTEND; MP PAXERR1 */
    ts(ADDRESS_OF(PAXERR));
    cm_dumpr();
}

/* CM/DUMPR: "PROGRAM TO FILE BODY RATES FOR TM ON ONE PASS AND" "TO UPDATE
 * THE NEEDLE DISPLAY ON THE NEXT.": SW/NDX complemented, "COMBINED
 * ALTERNATION SWITCH AND FILE" "INDEX.": "INDEX IS NEG FOR TM FILE"
 * (BZMF CMTMFILE); "INDEX IS POS FOR NEEDLES.": NEEDLER (IBNKCALL; CADR
 * NEEDLER), back at CM/DUMPR +6. */
static void cm_dumpr(void)
{
    cs(ADDRESS_OF(SW_NDX));
    ts(ADDRESS_OF(SW_NDX));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF CMTMFILE */
        cmtmfile();
        return;
    }
    tc_to(CM_DUMPR + 4u, IBNKCALL);
}

/* CM/DUMPR +6: TC CM/END. */
void cm_dumpr_after_needler(void)
{
    tc_at(CM_DUMPR + 6u);
    cm_end();
}

/* CMTMFILE: "FILE STARTS WITH SW/NDX +1 AND GOES TO" "ENDBUF.": the index
 * plus 3; not positive, SAVENDX; else "INITIALIZE THE TM LIST IN UPBUFF."
 * (TIME1 to CMTMTIME), */
void cmtmfile(void)
{
    ad(THREE);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF SAVENDX */
        savendx();
        return;
    }
    ca(ADDRESS_OF(TIME1));
    cmtmfile_ts();
}

/* CMTMFILE +4: (an overflow skips the CS THIRTEEN), "INITIALIZE COUNTER"
 * (-13), */
void cmtmfile_ts(void)
{
    if (!ts(ADDRESS_OF(CMTMTIME))) set_A(negate(THIRTEEN)); /* CS THIRTEEN */
    savendx();
}

/* SAVENDX: to SW/NDX, "A NEGATIVE NUMBER."; PREL and QREL (EXTEND; DCA
 * PREL) to the pair ENDBUF -1 words on (INDEX SW/NDX; DXCH ENDBUF -1), and
 * RREL to the word after it (INDEX SW/NDX; TS ENDBUF +1). An overflow at
 * TS SW/NDX skips the EXTEND, and the DCA's word runs as CA PREL +1:
 * QREL in A alone. */
void savendx(void)
{
    if (ts(ADDRESS_OF(SW_NDX)))
        ca(ADDRESS_OF(PREL) + 1u);
    else
        dca(ADDRESS_OF(PREL));
    dxch(indexed_store(ADDRESS_OF(ENDBUF), fetch(ADDRESS_OF(SW_NDX)), 010u) - 1u);
    ca(ADDRESS_OF(RREL));
    ts(indexed_store(ADDRESS_OF(ENDBUF) + 1u, fetch(ADDRESS_OF(SW_NDX)), A_ADDRESS));
    cm_end();
}

/* CM/END: CM/SAVE back to SR ("DOES NOT PROTECT TEMK, SQ IN SPSIN/COS");
 * T5LOC to T5IDLOC (T5IDLER2), and RESUME. */
void cm_end(void)
{
    ca(ADDRESS_OF(CM_SAVE));
    ts(SR_ADDRESS);
    dca(T5IDLER2); /* EXTEND; DCA T5IDLER2 */
    dxch(ADDRESS_OF(T5LOC));
    tc_to(CM_END + 5u, RESUME);
}

#endif
