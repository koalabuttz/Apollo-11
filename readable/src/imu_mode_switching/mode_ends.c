/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc, part 3: the end of a mode switch, the
 * cage tests, the stalls and GOMANUR (LM pages 1327-1331, CM pages
 * 1438-1442).
 *
 * MODECADR, MODECADR +1 and MODECADR +2 (IMUCADR, OPTCADR, RADCADR) say
 * where each device's operation stands: +0 while it runs and nothing
 * waits, the CADR of a program asleep in a stall, "-0 INDICATES OPERATION
 * COMPLETE BUT UNSUCCESSFUL: -1 INDICATES COMPLETE AND SUCCESSFUL." */
#include "subroutines.h"

/* ENDIMU -- LM p. 1327, CM p. 1438.
 * "MODE IS BAD IF CAGE HAS OCCURRED OR IF ISS WARNING IS ON." (channel
 * 11 bit 1). IMUGOOD: TCF GOODEND, "WITH C(A) = 0." */
void endimu(void)
{
    read_channel(DSALMOUT);
    mask(BIT1);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        imubad();
        return;
    }
    goodend();
}

/* IMUBAD -- LM p. 1327, CM p. 1438: BADEND for the IMU (index 0). */
void imubad(void)
{
    set_A(ZERO);
    badend();
}

/* ENDMODE +1 -- LM p. 1328, CM p. 1439: A to the device's MODECADR. */
static void endmode_plus1(void)
{
    ts(indexed(ADDRESS_OF(MODECADR), RUPTREG2));
    continue_at(TASKOVER);
}

/* GOODEND +2 -- LM p. 1328, CM p. 1439.
 * The outcome (-1 or -0) to RUPTREG3, and "SEE IF USING PROGRAM ASLEEP.":
 * "YES -- WAKE IT UP." with MODECADR +0; "IF 0, PROGRAM NOT IN YET." and
 * ENDMODE leaves the outcome there. A MODECADR of -1 or -0 the LM sets to
 * +0, at ENDMODE +1 with A +0 from the CCS ("BZF = TCF IF MODECADR =
 * -0.": the BZF word, reached without its EXTEND, is a TCF); the CM wakes
 * any negative MODECADR as a CADR. */
static void report(void)
{
    enum sign state;

    ts(ADDRESS_OF(RUPTREG3));
    state = ccs_at(indexed(ADDRESS_OF(MODECADR), RUPTREG2));
    if (state == IS_PLUS_ZERO) {
        ca(ADDRESS_OF(RUPTREG3)); /* ENDMODE */
        endmode_plus1();
        return;
    }
#if LUMINARY099
    if (state == IS_MINUS_ZERO || (state == NEGATIVE && A_is_zero())) {
        endmode_plus1();
        return;
    }
    set_A(ZERO);
#else
    if (state != IS_MINUS_ZERO) set_A(ZERO); /* -0 comes to the INDEX with A +0 */
#endif
    xch(indexed(ADDRESS_OF(MODECADR), RUPTREG2)); /* "WAKE SLEEPING PROGRAM." */
    tc_at((word)(ENDMODE - 5u));                  /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* BADEND -- LM p. 1328, CM p. 1439: "DEVICE INDEX." in A, -0 "FOR
 * FAILURE." (An overflow in A skips the CS.) */
void badend(void)
{
    if (!ts(ADDRESS_OF(RUPTREG2))) set_A(negate(ZERO)); /* CS ZERO */
    report();
}

/* GOODEND -- LM p. 1328, CM p. 1439: -1 "FOR SUCCESS." */
void goodend(void)
{
    if (!ts(ADDRESS_OF(RUPTREG2))) set_A(negate(ONE)); /* CS ONE */
    report();
}

/* ENDMODE -4, where JOBWAKE returns: "ADVANCE LOC IF SUCCESSFUL." (the
 * woken job's LOC, by one for -1), and TASKOVER. */
void goodend_after_jobwake(void)
{
    cs(ADDRESS_OF(RUPTREG3));
    ads(indexed(ADDRESS_OF(LOC), LOCCTR));
    continue_at(TASKOVER);
}

/* ======================================================================
 * The cage tests (IMODES30 bit 6: the IMU being caged)
 * ====================================================================== */

/* CAGETEST -- LM p. 1327, CM p. 1438.
 * "SUBROUTINE TO TERMINATE IMU MODE SWITCH IF IMU HAS BEEN CAGED." */
bool cagetest(void)
{
    set_A(BIT6);
    mask_at(ADDRESS_OF(IMODES30));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        imubad(); /* "DIRECTLY." */
        return false;
    }
    tc_through(Q_ADDRESS); /* TC Q, "WITH C(A) = +0." */
    return true;
}

/* CAGETSTQ -- LM p. 1327, CM p. 1438: "SKIP IF IMU NOT BEING CAGED." */
bool cagetstq(void)
{
    bool free = false;

    cs(ADDRESS_OF(IMODES30));
    mask(BIT6);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        incr(Q_ADDRESS);
        free = true;
    }
    tc_through(Q_ADDRESS); /* TC Q */
    return free;
}

/* CAGETSTJ -- LM p. 1327, CM p. 1438.
 * "IF DURING MODE SWITCH INITIALIZATION IT IS FOUND THAT THE IMU IS BEING
 * CAGED, SET IMUCADR TO -0 TO INDICATE OPERATION COMPLETE BUT FAILED." */
bool cagetstj(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT6);
    if (ccs_register(A_ADDRESS) != POSITIVE) {
        cagetstj_failed();
        return false;
    }
    tc_through(Q_ADDRESS); /* TC Q */
    return true;
}

/* CAGETSTJ +4: "RETURN IMMEDIATELY TO SWRETURN." */
void cagetstj_failed(void)
{
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(IMUCADR));
    modeexit();
}

/* ======================================================================
 * The stalls
 * ====================================================================== */

/* MODABORT -- LM p. 1330, CM p. 1441: "TWO PROGRAMS USING THE SAME
 * DEVICE.", abort 1210. The LM puts BUF2 in A,L for BAILOUT1. */
static void modabort(void)
{
#if LUMINARY099
    dxch(ADDRESS_OF(BUF2));
    tc_at((word)(MODABORT + 1u)); /* TC BAILOUT1; OCT 1210 */
    continue_at(BAILOUT1);
#else
    tc_at(MODABORT); /* TC POODOO; OCT 1210 */
    continue_at(POODOO);
#endif
}

/* MG2 -- LM p. 1330, CM p. 1441: MODECADR to +0 and "RETURN TO CALLER." */
static void mg2(void)
{
    ts(indexed(ADDRESS_OF(MODECADR), RUPTREG2));
    modeexit();
}

/* STALL, MODEGOOD, MODESLP -- LM p. 1330, CM p. 1441.
 * "SAVE DEVICE INDEX." and "SEE IF OPERATION COMPLETE.": "ALLOWABLE
 * STATES ARE +0, -1, AND -0." "OPERATION INCOMPLETE." puts the program to
 * sleep (MODESLP: its CADR to MODECADR, MAKECADR's "CALL FROM SWITCHABLE
 * FIXED ONLY."); -1, "COMPLETE AND GOOD", returns to L+3 ("INCREMENT
 * RETURN ADDRESS"); -0, "COMPLETE AND FAILED", to L+2. */
static void stall(void)
{
    inhint();
    ts(ADDRESS_OF(RUPTREG2));
    switch (ccs_at(indexed(ADDRESS_OF(MODECADR), a_word()))) { /* INDEX A; CCS MODECADR */
    case POSITIVE: modabort(); return;
    case IS_PLUS_ZERO:
        tc_at(MODESLP); /* TC MAKECADR */
        continue_at(MAKECADR);
        return;
    case NEGATIVE: /* MODEGOOD: "MAKE SURE INITIAL STATE -1." (the CCS left A +0 or more) */
        if (ccs_register(A_ADDRESS) == POSITIVE) {
            modabort();
            return;
        }
        incr(ADDRESS_OF(BUF2));
        break;
    case IS_MINUS_ZERO: break;
    }
    mg2();
}

/* AOTSTALL -- LM p. 1329, CM p. 1440: device 1, "AOT." (OPTSTALL). */
void aotstall(void)
{
    set_A(ONE);
    tc_at((word)(AOTSTALL + 1u)); /* TC STALL */
    stall();
}

/* RADSTALL -- LM p. 1329, CM p. 1440: device 2. */
void radstall(void)
{
    set_A(TWO);
    stall();
}

/* IMUSTALL -- LM p. 1330, CM p. 1441: device 0, "IMU." */
void imustall(void)
{
    set_A(ZERO);
    stall();
}

/* MODESLP +1, where MAKECADR returns: the program's CADR to MODECADR, and
 * JOBSLEEP. (An overflow in A skips into MODABORT.) */
void modeslp_after_makecadr(void)
{
    if (ts(indexed(ADDRESS_OF(MODECADR), RUPTREG2))) {
        modabort();
        return;
    }
    continue_at(JOBSLEEP);
}

/* GOMANUR -- LM p. 1331, CM p. 1442.
 * "IS KALCMANU FREE" (ATTCADR +0); if not, "2 TRYING TO USE SAME DEVICE"
 * (POODOO 1210). Otherwise "SAVE FINAL RETURN FOR KALCMAN3" (BUF2, with
 * the caller's EBANK bits), "SAVE USERS PRIO", "SET EBANK FOR KALCMAN3",
 * and on to KALCMAN3. */
void gomanur(void)
{
    ca(ADDRESS_OF(ATTCADR));
    if (!A_is_zero()) {
        tc_at((word)(GOMANUR + 3u)); /* TC POODOO; OCT 1210 */
        continue_at(POODOO);
        return;
    }
    dca(ADDRESS_OF(BUF2));
    dxch(ADDRESS_OF(ATTCADR));
    ca(BBANK_ADDRESS);
    mask(SEVEN);
    ads(ADDRESS_OF(ATTCADR) + 1u);
    ca(ADDRESS_OF(PRIORITY));
    mask(PRIO37);
    ts(ADDRESS_OF(ATTPRIO));
    set_A(KALEBCON); /* CAF KALEBCON: ECADR BCDU */
    ts(EBANK_ADDRESS);
    tc_at((word)(GOMANUR + 020u)); /* TC POSTJUMP; CADR KALCMAN3 */
    continue_at(POSTJUMP);
}
