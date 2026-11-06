/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 1 (Comanche055 pp. 803-805, bank 26):
 * S61.1, which P61 and P62 call.
 *
 * "FIRST, TEST TO SEE IF AVERAGEG IS ON. IF NOT, UPDATE THE STATE VECTOR
 * TO PRESENT TIME + TOLERANCE AND TURN ON AVERAGEG AT THAT TIME, AND
 * CONTINUE." Then the IMU's orientation: its Y axis must be within 30
 * degrees of UNI's direction (the orbit's normal, VN cross RN, in stable
 * member axes); if -Y is, alarm 01427, "IMU REVERSED"; if neither, alarm
 * 01426, "IMU UNSATISFACTORY". After an alarm the job waits 10 seconds
 * (DELAYJOB) with the alarm code shown (V05N09), and goes on.
 *
 * The return is kept in 60GENRET (E6,1773: "SAVE RET ADDR IN EB 6"), a
 * TC there (TC 60GENRET) runs the word as TC <return>: a key of its own
 * (s61_1_return). */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void retrn2_display(void);

/* S61.1, from TC S61.1 at at: the return (Q, the word after the TC) to
 * 60GENRET; the IMU checked (R02BOTH). */
void s61_1(word at)
{
    tc_at(at);
    qxch(ADDRESS_OF(60GENRET));  /* EXTEND; QXCH 60GENRET */
    tc_to(S61_1 + 2u, BANKCALL); /* CADR R02BOTH */
}

void s61_1_after_r02both(void)
{
    tc_at(S61_1 + 4u);
    intpret();
}

/* S61.1 +5: "IS AVERAGEG ON": YES, the IMU check at S61.1A; NO, MIDTOAV2,
 * "GET FUTURE STATE VECTOR SOON AS CAN", which returns in basic with the
 * time to wait in MPAC +1. */
static const word s61_1_words[] = {
    /* S61.1 +5 */ ORDERS(BON, CALRB),
    SWITCH(BON, AVEGFLAG),
    ADDRESS(S61_1A),
    ADDRESS(MIDTOAV2),
};

const struct interpretive_code S61_1_CODE = {026u, S61_1 + 5u, COUNT(s61_1_words), s61_1_words};

/* S61.1 +011: "RETURN INHINTED ***": the wait to S61DT, "FOR RESTART.",
 * and S61.1C as a task after it (WAITLIST); a restart point (PHASCHNG, OCT
 * 40434); ENDOFJOB. (CA leaves no overflow in A: TS does not skip.) */
void s61_1_future_state(void)
{
    ca(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(S61DT));
    tc_to(S61_1 + 013u, WAITLIST); /* 2CADR S61.1C */
}

void s61_1_after_waitlist(void)
{
    tc_to(S61_1 + 016u, PHASCHNG); /* OCT 40434 */
}

void s61_1_after_phaschng(void)
{
    tc_to(S61_1 + 020u, ENDOFJOB);
}

/* S61.1C, the task: the IMU check as a job (FINDVAC, priority 13, at
 * S61.1A -1). */
void s61_1c(void)
{
    set_A(PRIO13);
    tc_to(S61_1C + 1u, FINDVAC); /* 2CADR S61.1A -1 */
}

/* S61.1C +4: "HE WHO START AVERAGEG MUST SERVICE THE EXIT." (SERVCAD1 to
 * AVEGEXIT); a restart point (2PHSCHNG, OCT 00454 and OCT 00415). */
void s61_1c_after_findvac(void)
{
    dca_2cadr(SERVCAD1);
    dxch(ADDRESS_OF(AVEGEXIT));
    tc_to(S61_1C + 7u, TWO_PHSCHNG); /* OCT 00454, OCT 00415 */
}

/* S61.1C +012: "SET EB= 7 FOR PREREAD." (EBENTRY); "PREREAD DOES TC
 * TASKOVER." (POSTJUMP). (A is a constant: TS does not skip.) */
void s61_1c_after_2phschng(void)
{
    set_A(EBENTRY);
    ts(EBANK_ADDRESS);
    tc_to(S61_1C + 014u, POSTJUMP); /* CADR PREREAD */
}

/* S61.1A -1, where FINDVAC's job starts. */
void s61_1a_job(void)
{
    tc_at(S61_1C + 016u);
    intpret();
}

/* S61.1A: "TURN OFF OVFIND, IF ON" (BOVB TCDANZIG); UNI's direction in
 * stable member axes: VN cross RN through REFSMMAT, unit; "GET
 * COS(THETA)/2", its Y component (MPAC +3). Negative: S61.1B, "DO TEST ON
 * -YSM"; else C(30)LIM (1.0 - .5 COS(30)) added: an overflow (cos theta
 * over cos 30) is good, RETRN1; else RETRN3, alarm 01426. S61.1B: the
 * same for -Y: an overflow is RETRN2, alarm 01427; else EXIT, to RETRN3. */
static const word s61_1a_words[] = {
    /* S61.1A */ ORDERS(BOVB, VLOAD),
    ADDRESS(TCDANZIG),
    ADDRESS_PLUS_ONE(VN), /* "VN (-7) M/CS" */
    ORDERS(VXV, MXV),
    ADDRESS_PLUS_ONE(RN),       /* "RN (-29) M" */
    ADDRESS_PLUS_ONE(REFSMMAT), /* ".5 UNIT MATRIX" */
    ORDERS(UNIT, DLOAD),
    ADDRESS_PLUS_ONE_AT(MPAC, 3),
    ORDERS(BMN, DAD),
    ADDRESS(S61_1B),
    ADDRESS_PLUS_ONE(C_30LIM),
    ORDERS(BOVB, RTB),
    ADDRESS(RETRN1),
    ADDRESS(RETRN3),
    /* S61.1B */ ORDERS(DCOMP, DAD),
    ADDRESS_PLUS_ONE(C_30LIM),
    ORDERS(BOVB, EXIT),
    ADDRESS(RETRN2),
};

const struct interpretive_code S61_1A_CODE = {026u, S61_1C + 017u, COUNT(s61_1a_words),
                                              s61_1a_words};

/* RETRN3: "IMU UNSATISFACTORY" (ALARM, OCT 01426); then as RETRN2 after
 * its alarm (TC RETRN2 +2). The EXIT of S61.1B comes on here too. */
void retrn3(void)
{
    tc_to(RETRN3, ALARM); /* OCT 01426 */
}

void retrn3_after_alarm(void)
{
    tc_at(RETRN3 + 2u);
    retrn2_display();
}

/* RETRN2: "IMU REVERSED" (ALARM, OCT 01427). */
void retrn2(void)
{
    tc_to(RETRN2, ALARM); /* OCT 01427 */
}

void retrn2_after_alarm(void)
{
    retrn2_display();
}

/* RETRN2 +2: "DO DISPLAY" (V05N09, GODSPR); then 10 seconds' wait
 * (DELAYJOB). */
static void retrn2_display(void)
{
    set_A(V05N09);
    tc_to(RETRN2 + 3u, BANKCALL); /* CADR GODSPR */
}

void retrn2_after_godspr(void)
{
    set_A(TEN_SECS);
    tc_to(RETRN2 + 6u, BANKCALL); /* CADR DELAYJOB */
}

/* RETRN1: back to S61.1's caller (TC 60GENRET). */
void retrn1(void)
{
    tc_to(RETRN1, ADDRESS_OF(60GENRET));
}

/* 60GENRET, TC'd to: the AGC runs the word there, a TC to the return. */
void s61_1_return(void)
{
    tc_through(ADDRESS_OF(60GENRET));
}

#endif
