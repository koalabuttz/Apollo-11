/* SPDX-License-Identifier: MIT */
/* SERVICER207.agc, pp. 828-831 (Comanche055, bank 37): SERVICER, the job
 * READACCS sets up every two seconds. The PIPA counts checked for a
 * runaway PIPA and compensated (1/PIPA), their magnitude added up in
 * DVTOTAL, the state vector moved on by them and by gravity (AVERAGEG,
 * CALCRVG), and the exit through AVGEXIT: to the user's routine, which
 * returns to SERVEXIT, or on the last pass to AVGEND. NORMLIZE readies the
 * first pass. The faithful C is c64-port/src/cm_servicer.c. */
#include "servicer_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void pipcheck(void);
static void averageg(void);

/* ---- SERVICER -- p. 828 ---- */

/* SERVICER: the PIPA check, Z, Y and X (RUPTREG1 counting 2, 1, 0), ... */
void servicer(void)
{
    set_A(TWO);
    servicer_inhint();
}

/* ... with interrupts inhibited (SERVICER +1); ... */
void servicer_inhint(void)
{
    inhint();
    pipcheck();
}

/* ... PIPCHECK: "DO PIPA-SATURATION TEST BEFORE COMPENSATION.": a count
 * (DELVX, DELVY or DELVZ) whose magnitude less one is over 6398 (-MAXDELV:
 * "3200 PPS FOR 2 SEC CCS TAKES 1") is a "SATURATED-PIPA ALARM", and the
 * PIPAs are not compensated (TC AVERAGEG). A +0 count passes; -0 too (the
 * CCS leaves A +0 for the BZMF).
 *
 * Entered at PIPLOOP with an address in RUPTREG1, the index can reach
 * past erasable memory, where the CCS would read a fixed word: the
 * faithful port goes on at that address instead, and so does this. */
static void pipcheck(void)
{
    unsigned at;

    ts(ADDRESS_OF(RUPTREG1));
    double_A();
    at = indexed(ADDRESS_OF(DELVX), a_word());
    if (at > 07777u) not_modelled();
    if (at >= 02000u) {
        continue_at((word)at);
        return;
    }
    switch (ccs_at(at)) {
    case POSITIVE:
        tc_at(PIPCHECK + 4u); /* TC +2 */
        saturation_test();
        return;
    case IS_PLUS_ZERO: pipcheck_no_count(); return;
    case NEGATIVE: saturation_test(); return;
    case IS_MINUS_ZERO: piploop(); return;
    }
}

/* PIPCHECK +5: TC PIPLOOP. */
void pipcheck_no_count(void)
{
    tc_at(PIPCHECK + 5u);
    piploop();
}

/* PIPCHECK +6: the count's magnitude less one, less 6398. */
void saturation_test(void)
{
    ad(MINUS_MAXDELV);
    if (A_is_zero_or_minus()) {
        piploop();
        return;
    }
    tc_to(PIPCHECK + 011u, ALARM); /* OCT 00205 */
}

/* After the alarm: AVERAGEG, the compensation, DVTOTAL and DVMON
 * "BYPASSED". */
void pipcheck_after_alarm(void)
{
    tc_at(PIPCHECK + 013u);
    averageg();
}

/* PIPLOOP: the next count; after X, "RESTART REREADAC + SERVICER"
 * (PHASCHNG, OCT 16035, OCT 20000, 2CADR DVTOTUP). (A
 * negative RUPTREG1 would leave the CCS on those data words: not
 * modelled.) */
void piploop(void)
{
    switch (ccs_at(ADDRESS_OF(RUPTREG1))) {
    case POSITIVE: pipcheck(); return;
    case IS_PLUS_ZERO: tc_to(PIPLOOP + 2u, PHASCHNG); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* The "PIPA COMPENSATION CALL" (1/PIPA, through BANKCALL), back at
 * DVTOTUP. */
void piploop_after_phaschng(void)
{
    tc_to(PIPLOOP + 7u, BANKCALL);
}

/* DVTOTUP: to the interpreter, for DVTOTUP_CODE. */
void dvtotup(void)
{
    tc_at(DVTOTUP);
    intpret();
}

/* "GET ABS VALUE OF DELV", and KPIP1, "SCALE AT 2(+7)". */
static const word dvtotup_words[] = {
    /* DVTOTUP +1 */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(DELV),
    ORDERS(DMP, EXIT),
    ADDRESS_PLUS_ONE(KPIP1),
};

const struct interpretive_code DVTOTUP_CODE = {037u, DVTOTUP + 1u, COUNT(dvtotup_words),
                                               dvtotup_words};

/* After the EXIT: "ACCUMULATE DVTOTAL", and AVERAGEG. */
void dvtotup_after_exit(void)
{
    dca(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(DVTOTAL));
    averageg();
}

/* ---- AVERAGEG -- pp. 828-829 ---- */

/* AVERAGEG: a phase change (PHASCHNG, OCT 10035); ... */
static void averageg(void)
{
    tc_to(AVERAGEG, PHASCHNG);
}

/* ... CALCRVG: "RN1, VN1, GDT1/2 CALCULATED VIA CALCRVG ROUTINE BY
 * UPDATING RN, VN WITH DELV AND AN AVERAGED VALUE OF GDT/2." */
void averageg_after_phaschng(void)
{
    tc_at(AVERAGEG + 2u);
    intpret();
}

static const word averageg_words[] = {
    /* AVERAGEG +3 */ ORDER(CALL),
    ADDRESS(CALCRVG),
    ORDER(EXIT),
};

const struct interpretive_code AVERAGEG_CODE = {037u, AVERAGEG + 3u, COUNT(averageg_words),
                                                averageg_words};

/* After the EXIT: the phase again (PHASCHNG, OCT 10035); ... */
void averageg_after_exit(void)
{
    tc_to(AVERAGEG + 6u, PHASCHNG);
}

/* ... "COPY RN1,VN1,GOT102,GOBL1/2,PIPTIME1" (sic) "INTO RN ,VN ,GDT/12
 * ,GOBL/2 ,PIPTIME" (GENTRAN, OCT31 + 1 words, ADRES RN1, ADRES RN), "FOR
 * RESTART PROTECTION"; ... */
void averageg_copy(void)
{
    set_A(OCT31);
    tc_to(AVERAGEG + 011u, GENTRAN);
}

/* ... "GENTRAN DOES AN INHINT"; the phase once more; ... */
void averageg_after_gentran(void)
{
    relint();
    tc_to(AVERAGEG + 015u, PHASCHNG);
}

/* ... and "AVERAGEG EXIT": DXCH Z with AVGEXIT's 2CADR. */
void averageg_exit(void)
{
    dca(ADDRESS_OF(AVGEXIT));
    dtcb(AVERAGEG + 021u);
}

/* ---- AVGEND -- pp. 829-830 ---- */

/* AVGEND, the "FINAL AVERAGE G EXIT": OLDBT1, "SET UP FREE FALL GYRO
 * COMPENSATION" (PIPTIME's low word), and "SET DRIFTFLG" (UPFLAG); ... */
void avgend(void)
{
    ca(ADDRESS_OF(PIPTIME) + 1u);
    ts(ADDRESS_OF(OLDBT1));
    tc_to(AVGEND + 2u, UPFLAG);
}

/* ... "GROUP 5 OFF", "GROUP 2 ON FOR AVETOMID" (2PHSCHNG, OCT 5, OCT
 * 05022, OCT 20000); ... */
void avgend_after_upflag(void)
{
    tc_to(AVGEND + 4u, TWO_PHSCHNG);
}

/* ... AVETOMID: "CONVERT STATE VECTOR TO REFERENCE SCALE." */
void avgend_after_2phschng(void)
{
    tc_at(AVGEND + 010u);
    intpret();
}

static const word avgend_words[] = {
    /* AVGEND +011 */ ORDER(CALL),
    ADDRESS(AVETOMID),
    ORDER(EXIT),
};

const struct interpretive_code AVGEND_CODE = {037u, AVGEND + 011u, COUNT(avgend_words),
                                              avgend_words};

/* After the EXIT: "ZERO MARK COUNTERS." (VHFCNT, TRKMKCNT), and PIPFREE
 * (BANKCALL); ... */
void avgend_after_exit(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(VHFCNT));
    ts(ADDRESS_OF(TRKMKCNT));
    tc_to(AVGEND + 017u, BANKCALL);
}

/* ... "INVALIDATE MARK BUFFER" (MRKBUF2, -BIT9), the "TEST CONNECTOR
 * OUTBIT" off (DSALMOUT bit 9), and CM/DSTBY down (DOWNFLAG); ... */
void avgend_after_pipfree(void)
{
    set_A(negate(BIT9));
    ts(ADDRESS_OF(MRKBUF2));
    wand_dsalmout();
    tc_to(AVGEND + 025u, DOWNFLAG);
}

/* ... V37FLAG down; ... */
void avgend_after_downflag(void)
{
    tc_to(AVGEND + 027u, DOWNFLAG);
}

/* ... "RESTORE GROUP 1 + 2 IF P20 IS RUNNING." (FLAGWRD0 bit 7: 2PHSCHNG,
 * "1.11SPOT", "2.13SPOT"); ... */
void avgend_after_downflag_2(void)
{
    set_A(BIT7);
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (A_is_zero()) {
        avgend_after_2phschng_2();
        return;
    }
    tc_to(AVGEND + 035u, TWO_PHSCHNG);
}

/* ... and CANV37 (POSTJUMP): "CONTROL TRANSFERRED TO CANV37 TO CONTINUE MM
 * CHANGE ROUTINE (R00)." */
void avgend_after_2phschng_2(void)
{
    tc_to(AVGEND + 040u, POSTJUMP);
}

/* SERVEXIT: "A, 5.3 = REREADAC (ONLY)" (PHASCHNG, OCT 00035), ... */
void servexit(void)
{
    tc_to(SERVEXIT, PHASCHNG);
}

/* ... and the job's end. */
void servexit_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* ---- NORMLIZE -- p. 831 ---- */

/* NORMLIZE "PERFORMS THE INITIALIZATION REQUIRED PRIOR TO THE FIRST ENTRY
 * TO AVERAGEG": "SET UP TO COPY 14 REGS- RN1,VN1,PIPTIME1" "INTO RN,VN,
 * PIPTIME" (GENTRAN, THIRTEEN + 1 words); ... */
void normlize(void)
{
    set_A(THIRTEEN);
    tc_to(NORMLIZE + 1u, GENTRAN);
}

/* ... then GDT/2 from RN (CALCGRAV), and GOBL/2. */
void normlize_after_gentran(void)
{
    relint();
    tc_at(NORMLIZE + 5u);
    intpret();
}

static const word normlize_words[] = {
    /* NORMLIZE +6 */ ORDERS(VLOAD, CALL), /* "LOAD RN FOR CALCGRAV" */
    ADDRESS_PLUS_ONE(RN),
    ADDRESS(CALCGRAV), /* "INITIALISE UNITR RMAG GDT1" */
    STOVL(GDT_2),
    ADDRESS_PLUS_ONE(GOBL1_2),
    STORE(GOBL_2),
    ORDER(EXIT),
};

const struct interpretive_code NORMLIZE_CODE = {037u, NORMLIZE + 6u, COUNT(normlize_words),
                                                normlize_words};

/* After the EXIT, the job's end. */
void normlize_after_exit(void)
{
    continue_at(ENDOFJOB);
}

#endif
