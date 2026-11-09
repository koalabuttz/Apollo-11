/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part A (Comanche055 pp. 1063-1065, bank
 * 15 and fixed-fixed): READGYMB, the "SUBROUTINE TO READ GIMBAL ANGLES AND
 * FORM DIFFERENCES", a WAITLIST task every 0.1 s ("ENTER AND READ ANGLES
 * EACH .1 SEC."); ANGOVCOR, which corrects an angle's overflow; and
 * FLUSHJET, which turns all the jets off.
 *
 * "GIMBAL ANGLES ARE SAVED IN 2S COMPLEMENT, BUT THE DIFFERENCES ARE IN
 * 1S COMP." CM/FLAGS' two low bits steer the task: CM/DSTBY (bit 2), "= 1
 * FOR DAP OPERATION", "= 0 TO TERMINATE DAP OPERATION"; GYMDIFSW (bit 1),
 * clear until the first pass has angles to difference. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void cm_gymic(void);
static void cm_gymic_zero(void);
static void dobrate1(void);

/* ---- FLUSHJET -- p. 1065 ---- */

/* FLUSHJET: "COME HERE TO TURN OFF ALL JETS.": +0 (CA 7, the ZERO
 * register) to channel 6, ROLLJETS ("ZERO CHANNEL 6"), and channel 5,
 * PYJETS ("ZERO CHANNEL 5"); back by TC Q. */
void flushjet(void)
{
    ca(ZERO_ADDRESS);
    write_channel(6); /* EXTEND; WRITE ROLLJETS */
    write_channel(5); /* EXTEND; WRITE PYJETS */
    tc_q();
}

/* ---- READGYMB -- pp. 1063-1064 ---- */

/* READGYMB: "KEEP RESTART DT GOING RELATIVE TO PIPTIME. (GROUP 6)"
 * (CM/GYMDT, ten more centiseconds; "IF A RESTART OCCURS, SKIP PRESENT
 * CYCLE."). Then "CHECK FOR FINE ALIGN MODE OF CDU." (IMODES33 bit 6
 * clear), which READGYM1 needs. "NOT IN FINE ALIGN, SO IDLE.": "SET
 * GYMDIFSW = 0" (a MASK leaves no overflow for the TS to skip on), and
 * "QUENCH JETS, SINCE MAY BE A WHILE." */
void readgymb(void)
{
    set_A(TEN);
    ads(ADDRESS_OF(CM_GYMDT));
    set_A(BIT6);
    mask_at(ADDRESS_OF(IMODES33));
    if (A_is_zero()) { /* EXTEND; BZF READGYM1 */
        readgym1();
        return;
    }
    set_A(negate(BIT1)); /* CS */
    mask_at(ADDRESS_OF(CM_FLAGS));
    ts(ADDRESS_OF(CM_FLAGS));
    tc_to(READGYMB + 011u, FLUSHJET);
}

/* READGYMB +012: on at CM/GYMIC +2, the rates' words zeroed. */
void readgymb_after_flushjet(void)
{
    tc_at(READGYMB + 012u);
    cm_gymic_zero();
}

/* READGYM1: each gimbal angle read from its CDU (CDUX, CDUY, CDUZ: AOG,
 * AIG, AMG, the outer, inner and middle gimbals), the last one's less the
 * new one: "-DELAOG=AOG(N-1) - AOG(N)" (EXTEND; MSU, in ones' complement
 * from the CDUs' two's), and -DELAIG and -DELAMG the same. On at DOBRATE?:
 * CM/DSTBY and GYMDIFSW, inverted, pick the next step (INDEX A; TC +1),
 * Q the word after the TC. */
void readgym1(void)
{
    ca(ADDRESS_OF(CDUX));
    xch(ADDRESS_OF(AOG));
    msu_at(ADDRESS_OF(AOG));
    ts(ADDRESS_OF(MDELAOG));
    ca(ADDRESS_OF(CDUY));
    xch(ADDRESS_OF(AIG));
    msu_at(ADDRESS_OF(AIG));
    ts(ADDRESS_OF(MDELAIG));
    ca(ADDRESS_OF(CDUZ));
    xch(ADDRESS_OF(AMG));
    msu_at(ADDRESS_OF(AMG));
    ts(ADDRESS_OF(MDELAMG));
    /* DOBRATE?: "CM/DSTBY=103D BIT2  GYMDIFSW=104D BIT1" */
    cs(ADDRESS_OF(CM_FLAGS));
    mask(THREE);
    tc_at(DOBRATE_Q + 3u);
    continue_at((word)(DOBRATE_Q + 4u + A));
}

/* DOBRATE? +4: both set, "OK, GO ON" (DOBRATE). */
void dobrate_q_ok(void)
{
    tc_at(DOBRATE_Q + 4u);
    set_A(ONE); /* DOBRATE: "DO BODYRATE" */
    dobrate1();
}

/* DOBRATE? +5: CM/DSTBY set, GYMDIFSW clear: "DONT CALC BODYRATE ON
 * FIRST PASS." (CM/GYMIC). */
void dobrate_q_first_pass(void)
{
    tc_at(DOBRATE_Q + 5u);
    cm_gymic();
}

/* DOBRATE? +6: CM/DSTBY clear (NOOP), */
void dobrate_q_noop(void)
{
    dobrate_q_standby();
}

/* DOBRATE? +7: "TURN OFF ALL JETS", */
void dobrate_q_standby(void)
{
    tc_to(DOBRATE_Q + 7u, FLUSHJET);
}

/* DOBRATE? +010: "DEACTIVATE DAP GROUP 6." (PHASCHNG, OCT 00006), */
void dobrate_q_group6(void)
{
    tc_to(DOBRATE_Q + 010u, PHASCHNG);
}

/* DOBRATE? +012: and the task ends. */
void dobrate_q_after_phaschng(void)
{
    tc_to(DOBRATE_Q + 012u, TASKOVER);
}

/* DOBRATE1: A to JETEM, 1 to do BODYRATE, 0 to "SKIP BODYRATE." (A from
 * a CAF, or the TSs before, has no overflow for the TS to skip on); and
 * READGYMB again in 0.1 s, "KEEP CDU READ GOING." (WAITLIST, 2CADR
 * READGYMB). */
static void dobrate1(void)
{
    ts(ADDRESS_OF(JETEM));
    set_A(TEN);
    tc_to(DOBRATE1 + 2u, WAITLIST);
}

/* DOBRATE1 +5: JETEM, through CCS: 1 (+0 after CCS), BODYRATE; 0, "SKIP
 * CALC ON INITIAL PASS. (PASSES)"; negative, CM/GYMIC; -0, on at CM/GYMIC
 * +1. ("DOES NOT PROTECT TEMK, SQ IN SPSIN/COS") */
void dobrate1_after_waitlist(void)
{
    switch (ccs_at(ADDRESS_OF(JETEM))) {
    case POSITIVE: tc_to(DOBRATE1 + 6u, BODYRATE); break;
    case IS_PLUS_ZERO: dobrate1_taskover(); break;
    case NEGATIVE: cm_gymic(); break;
    case IS_MINUS_ZERO:
        set_A(ZERO); /* CM/GYMIC +1: CAF ZERO */
        cm_gymic_zero();
        break;
    }
}

/* DOBRATE1 +7: the task ends. */
void dobrate1_taskover(void)
{
    tc_to(DOBRATE1 + 7u, TASKOVER);
}

/* CM/GYMIC: "GYMDIFSW: C(A)=1, KNOW BIT IS 0" (ADS CM/FLAGS), */
static void cm_gymic(void)
{
    ads(ADDRESS_OF(CM_FLAGS));
    set_A(ZERO);
    cm_gymic_zero();
}

/* CM/GYMIC +2: A to the jet tag JETAG, the last rate changes OLDELP,
 * OLDELQ, OLDELR and GAMDOT ("NO GYM DIF, PROB NO GAM DIF."). An overflow
 * in A (READGYMB +012's A, as FLUSHJET leaves it) skips OLDELP, A then +1
 * or -1; on at DOBRATE1, JETEM that A. */
static void cm_gymic_zero(void)
{
    if (!ts(ADDRESS_OF(JETAG))) ts(ADDRESS_OF(OLDELP));
    ts(ADDRESS_OF(OLDELQ));
    ts(ADDRESS_OF(OLDELR));
    ts(ADDRESS_OF(GAMDOT));
    tc_at(CM_GYMIC + 7u);
    dobrate1();
}

/* ---- ANGOVCOR -- p. 1065 ---- */

/* ANGOVCOR: "COME HERE TO CORRECT FOR OVERFLOW IN ANGULAR CALCULATIONS"
 * ("THIS COSTS 2 MCT TO USE."): A to L, and with "NO OVFL" back by TC Q;
 * with an overflow, A +1 or -1, LIMITS +1 or -1 added into L (ADS), and
 * on at ANGOVCOR +5, its TC Q. */
void angovcor(void)
{
    if (!ts(L_ADDRESS)) {
        tc_q();
        return;
    }
    set_A(fetch_fixed(indexed(LIMITS, a_word()))); /* INDEX A; CAF LIMITS */
    ads(L_ADDRESS);
    continue_at(ANGOVCOR + 5u);
}

#endif
