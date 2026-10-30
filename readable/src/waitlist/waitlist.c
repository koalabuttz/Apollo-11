/* SPDX-License-Identifier: MIT */
/* WAITLIST: WAITLIST.agc of Luminary099 (LM, listing pages 1117-1132) and
 * Comanche055 (CM, pages 1221-1235), with T3RUPT, TASKOVER and LONGCALL.
 * The faithful C is the WAITLIST half of
 * c64-port/src/resident_kernel_body.inc.
 *
 * Every word of memory and every register ends as the original leaves it
 * (STYLE.md, Fidelity). The time arithmetic below is the original's, in
 * the 16-bit accumulator: it depends on overflow, and the comments give the
 * listing's own account of it. */
#include "waitlist.h"
#include "../agc/names.h"

static void waitlist_after_inhint(const word task[2]);
static void dly2(void);
static void wait2(void);
static void wtlst2(unsigned place);
static void wtlst4(void);
static void wtlst5(void);
static void wtabort(void);
static void t3rupt2(void);

/* A step of the 16-bit accumulator: AD of a word. */
static void add_to_A(word value)
{
    A = add16(A, sign_extend(value));
    correct_L();
}

/* ======================================================================
 * Adding a task
 * ====================================================================== */

/* WAITLIST -- LM p. 1119, CM p. 1223.
 * Run the task whose 2CADR follows the TC WAITLIST in A centiseconds
 * (1 to 16250). The caller continues after the 2CADR. */
void waitlist(const word task[2])
{
    inhint();
    waitlist_after_inhint(task);
}

/* TWIDDLE -- LM p. 1119, CM p. 1223.
 * WAITLIST for a task in the caller's own banks: only its address (ADRES)
 * follows the TC, and its bank word is the caller's BBANK. TWIDDLE adds
 * POSMAX to Q, an overflow that makes WAITLIST's TS WAITEXIT skip the
 * EXTEND before its DCA: the DCA then runs as CAF, which reads the one
 * word. ("Creating overflow and Q-1 in Q".) */
void twiddle(word task_address)
{
    const word task[2] = {task_address, 0};

    inhint();
    ts(L_ADDRESS); /* the delay, kept in L */
    set_A(POSMAX);
    A = add16(A, Q); /* ADS Q */
    Q = A;
    correct_L();
    set_A(BBANK);
    ror_superbnk();
    xch(L_ADDRESS); /* delay in A, BBCON in L */
    waitlist_after_inhint(task);
}

static void waitlist_after_inhint(const word task[2])
{
#if COMANCHE055
    if (A == 0u || (A & 0100000u)) { /* WATLST0-: no delay, alarm 1204 */
        tc_at(WATLST0_MINUS);
        continue_at(POODOO);
        return;
    }
#endif
    xch(Q_ADDRESS); /* the delay to Q, the return to A */
    if (ts(ADDRESS_OF(WAITEXIT))) {
        set_A(task[0]); /* TWIDDLE: CAF of the ADRES; BBCON stays in L */
    } else {
        set_L(task[1]); /* DCA of the 2CADR */
        set_A(task[0]);
    }
    ts(ADDRESS_OF(WAITADR));
    dly2();
}

/* DLY2 -1: DELAYJOB's entry (TS WAITADR, then DLY2). */
void dly2_minus1(void)
{
    ts(ADDRESS_OF(WAITADR));
    dly2();
}

/* DLY2, WAIT2 -- LM pp. 1119, 1122, CM pp. 1223, 1226.
 * Into WAITLIST's bank, keeping the caller's BBANK in WAITBANK. The task's
 * place in the list comes from the delay (Q) and the time to the first
 * task: TIME3 holds 1.0 - (T1 - T). In the listing's words, the four
 * orders after CS TIME3; AD BIT8; CCS A "set C(A) = TD - T1 + 1 in either
 * case" (whether or not TIME3 has just overflowed), TD being the new task's
 * time. If the new task comes first (TD - T1 + 1 <= 0), TIME3 is reset to
 * 1.0 - (TD - T) and the old first task's gap goes into the list at
 * WTLST4; otherwise WTLST5 finds its place. */
static void dly2(void)
{
    set_A(WAITBB);
    xch(BBANK_ADDRESS);
    wait2();
}

static void wait2(void)
{
    WAITBANK = a_word();
#if LUMINARY099
    A = Q;
    correct_L();
    if (A == 0u || (A & 0100000u)) { /* WAITPOOH: no delay, alarm 1204 */
        dxch(ADDRESS_OF(WAITEXIT));
        tc_at((word)(WAITPOOH + 1u));
        continue_at(POODOO1);
        return;
    }
#endif
    set_A(negate(TIME3));
    add_to_A(BIT8);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: /* TIME3 has overflowed */
        add_to_A(OCT40001);
        /* fall through */
    case IS_PLUS_ZERO:
        A = (reg16)(~A & 0177777u); /* CS A */
        /* fall through */
    case NEGATIVE: add_to_A(OCT40201); /* fall through */
    case IS_MINUS_ZERO:
        A = add16(A, Q); /* TD - T1 + 1 */
        correct_L();
        break;
    }
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: add_to_A(LST1(0)); /* fall through */
    case IS_PLUS_ZERO: wtlst5(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: break;
    }
    /* The new task is first: TIME3 = 1.0 - delta T, and A the old first
     * task's gap, 1.0 - (T1 - T) - 1.0 + delta T. */
    A = (reg16)(~Q & 0177777u);
    add_to_A(POS1_2);
    add_to_A(POS1_2);
    xch(ADDRESS_OF(TIME3));
    add_to_A(NEGMAX);
    A = add16(A, Q);
    correct_L();
    qxch(7u); /* ZQ: the new task goes in at place 0 */
    wtlst4();
}

/* WTLST5 -- LM pp. 1124-1125, CM pp. 1228-1229.
 * Walk down the gaps: with A = TD - Tn + 1, adding LST1 +n-1 gives
 * TD - Tn+1 + 1. The task goes in before the first task it does not come
 * after (at TC WTLST2 with OCT n). Each step is
 *     CCS A
 *     AD  LST1 +n       positive: on to the next gap
 *     TCF +4            +0: on, too
 *     AD  ONE           negative: here
 *     TC  WTLST2        -0: here
 *     OCT n
 * After the eighth gap the list is full (WTABORT). */
static void wtlst5(void)
{
    unsigned n;
    enum sign sign;

    for (n = 1; n < 8u; ++n) {
        sign = ccs_register(A_ADDRESS);
        if (sign == POSITIVE) add_to_A(LST1(n));
        if (sign == POSITIVE || sign == IS_PLUS_ZERO) continue;
        if (sign == NEGATIVE) add_to_A(ONE);
        tc_at((word)(WTLST5 + 6u * n - 2u));
        wtlst2(n);
        return;
    }
    sign = ccs_register(A_ADDRESS);
    if (sign == POSITIVE) {
        wtabort();
        return;
    }
#if COMANCHE055
    if (sign == IS_PLUS_ZERO) { /* lands on the OCT 1203, which runs as TC 1203 */
        tc_at((word)(WTABORT + 1u));
        continue_at(01203u);
        return;
    }
#endif
    if (sign != IS_MINUS_ZERO) add_to_A(ONE);
    tc_at((word)(WTABORT + 3u)); /* TC WTLST2; OCT 10 */
    wtlst2(8u);
}

/* WTLST2 -- LM p. 1127, CM p. 1230.
 * The new task goes in at place n (Q). With A = -(TD - Tn+1 + 1): gap n-1
 * becomes -(TD - Tn) + 1, and the new task's gap after it,
 * -(Tn+1 - TD) + 1, goes in at WTLST4. */
static void wtlst2(unsigned place)
{
    unsigned gap;

    /* TS WAITTEMP; INDEX Q; CAF 0 (the OCT n). An overflow in the TS skips
     * the INDEX, and the CAF 0 reads A: the +1 the TS left. */
    if (!ts(ADDRESS_OF(WAITTEMP))) set_A((word)place);
    ts(Q_ADDRESS);
    set_A(ONE);
    add_to_A(WAITTEMP);
    gap = ADDRESS_OF(LST1) - 1u + Q; /* INDEX Q; ADS LST1 -1 */
    A = add16(A, sign_extend(fetch(gap)));
    store(gap, a_word());
    correct_L();
    set_A(negate(WAITTEMP));
    wtlst4();
}

/* WTLST4 -- LM p. 1123, CM p. 1227.
 * Insert at place Q: the gap in A and the 2CADR (WAITADR, and the BBCON
 * still in L) go in and everything after moves down one. What falls off
 * the end must be ENDTASK's task (an unused place), or the list was full. */
static void wtlst4(void)
{
    unsigned i, place = (uint8_t)Q;

    for (i = place; i < 8u; ++i) xch(ADDRESS_OF(LST1) + i);
    set_A(WAITADR);
    for (i = place; i < 9u; ++i) dxch(ADDRESS_OF(LST2) + 2u * i);
    add_to_A(ENDTASK[0]);
    if (A == 0u || A == 0177777u) {
        /* LVWTLIST: back to the caller, after the 2CADR. */
        dxch(ADDRESS_OF(WAITEXIT));
        add_to_A(TWO);
        dtcb((word)(LVWTLIST + 2u));
        return;
    }
    wtabort();
}

/* WTABORT -- LM pp. 1125-1126 (FILLED), CM p. 1229.
 * Ten tasks: alarm 1203 ("no room in the inn"). */
static void wtabort(void)
{
    tc_at(WTABORT);
#if LUMINARY099
    dxch(ADDRESS_OF(WAITEXIT));
    tc_at((word)(FILLED + 1u));
    continue_at(BAILOUT1);
#else
    continue_at(BAILOUT);
#endif
}

/* ======================================================================
 * Delays within a task
 * ====================================================================== */

/* FIXDELAY -- LM p. 1119, CM p. 1223.
 * End this task and carry on after the TC FIXDELAY's DEC (the delay) once
 * that many centiseconds have passed: a task to the rest of itself. */
void fixdelay(word delay)
{
    set_A(delay);
    Q = add16(1u, Q); /* INCR Q */
    correct_L();
    vardelay();
}

/* VARDELAY -- LM p. 1119, CM p. 1223.
 * FIXDELAY with the delay in A, carrying on after the TC VARDELAY. When the
 * new task is set up, WAITLIST returns to DELAYEX, which ends this task
 * (TCF TASKOVER). */
void vardelay(void)
{
    xch(Q_ADDRESS);
    ts(ADDRESS_OF(WAITADR));
    set_A(BBANK);
    ror_superbnk();
    ts(L_ADDRESS);
    set_A(DELAYEX);
    ts(ADDRESS_OF(WAITEXIT));
    dly2();
}

/* ======================================================================
 * T3RUPT: running the tasks
 * ====================================================================== */

/* The T3RUPT lead-in at 04014 -- INTERRUPT_LEAD_INS.agc (LM p. 153,
 * CM p. 131): DXCH ARUPT; CAF T3RPTBB; XCH BBANK; TCF T3RUPT. */
void t3rupt_lead_in(void)
{
    dxch(ADDRESS_OF(ARUPT));
    set_A(T3RPTBB);
    xch(BBANK_ADDRESS);
    /* T3RUPT -- LM p. 1128, CM p. 1231: save the superbank and Q. */
    ror_superbnk();
    ts(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    t3rupt2();
}

/* T3RUPT2 -- LM p. 1128, CM p. 1231.
 * TIME3 has overflowed: task 1 is due. Every gap moves up one (the last
 * place gets NEG1/2, the 81.91 s to ENDTASK's next round), TIME3 is set
 * from task 2's gap, RUPTAGN says whether that overflowed too (task 2 due
 * at once: +1) or not (-0), and every 2CADR moves up one (ENDTASK's goes
 * in at the end). Task 1 starts by DTCB, in interrupt. */
static void t3rupt2(void)
{
    int i;
    word endtask_cadr[2];

    set_A(NEG1_2);
    for (i = 7; i >= 0; --i) xch(ADDRESS_OF(LST1) + (unsigned)i);
    add_to_A(POSMAX);
    A = add16(A, sign_extend(TIME3)); /* ADS TIME3 */
    TIME3 = a_word();
    correct_L();
    /* TS RUPTAGN; CS ZERO; TS RUPTAGN: "sets RUPTAGN to +1 on overflow". */
    if (!ts(ADDRESS_OF(RUPTAGN))) set_A(MINUS_ZERO);
    ts(ADDRESS_OF(RUPTAGN));
    /* DCS ENDTASK: ENDTASK is -2CADR SVCT3, so this is SVCT3's 2CADR. */
    endtask_cadr[0] = negate(ENDTASK[0]);
    endtask_cadr[1] = negate(ENDTASK[1]);
    set_L(endtask_cadr[1]);
    set_A(endtask_cadr[0]);
    for (i = 8; i >= 0; --i) dxch(ADDRESS_OF(LST2) + 2u * (unsigned)i);
    /* The task's superbank from its BBCON. */
    xch(L_ADDRESS);
    write_superbnk();
    xch(L_ADDRESS);
    agc.memory[7] = 0;
    dtcb((word)(T3RUPT2 + 035u));
}

/* TASKOVER -- LM p. 1129, CM p. 1232.
 * A task has ended. If the next one is due too (RUPTAGN +1), run it: back
 * to T3RUPT2 in WAITLIST's bank. Otherwise restore what the interrupt
 * saved and RESUME the interrupted program. */
void taskover(void)
{
    switch (ccs(RUPTAGN)) {
    case POSITIVE: set_A(WAITBB);         /* fall through */
    case IS_PLUS_ZERO: ts(BBANK_ADDRESS); /* fall through */
    case NEGATIVE:
        if (FBANK != 02000u) { /* T3RUPT2 is in bank 01: carry on there */
            continue_at(T3RUPT2);
            return;
        }
        t3rupt2();
        return;
    case IS_MINUS_ZERO: break;
    }
    set_A(BANKRUPT);
    write_superbnk();
    /* RESUME, NOQRSM, NOQBRSM */
    qxch(ADDRESS_OF(QRUPT));
    set_A(BANKRUPT);
    xch(BBANK_ADDRESS);
    dxch(ADDRESS_OF(ARUPT));
    relint();
    *agc.in_interrupt = 0;
    /* RESUME: the instruction the interrupt displaced runs next, at ZRUPT -1. */
    continue_at((word)(ZRUPT - 1u));
    agc.memory[7] = 0;
}

/* ======================================================================
 * SVCT3: ENDTASK's task
 * ====================================================================== */

/* SVCT3 -- LM p. 1121, CM p. 1225.
 * The task in every unused place, run every 81.93 seconds. With the drift
 * flag set (FLAGWRD2 negative) it starts NBDONLY, the IMU's drift
 * compensation, as a job, unless IMUCADR shows someone else using the IMU;
 * then it waits 5 seconds (SVCT3X) and looks again. */
void svct3(void)
{
    enum sign flag = ccs(FLAGWRD2);

    if (flag == POSITIVE || flag == IS_PLUS_ZERO) { /* drift flag off */
        continue_at(TASKOVER);
        return;
    }
    if (ccs(IMUCADR) != IS_PLUS_ZERO) { /* IMU busy: SVCT3X */
        tc_at(SVCT3X);
        continue_at(FIXDELAY);
        return;
    }
    set_A(PRIO35);
    tc_at((word)(SVCT3 + 012u)); /* TC NOVAC; 2CADR NBDONLY */
    continue_at(NOVAC);
}

/* SVCT3 +15: TCF TASKOVER after the NOVAC. */
void svct3_after_novac(void)
{
    continue_at(TASKOVER);
}

/* SVCT3X +2: TC SVCT3 after the FIXDELAY. */
void svct3x_after_fixdelay(void)
{
    tc_at((word)(SVCT3X + 2u));
    svct3();
}

/* ======================================================================
 * LONGCALL: delays of more than 162.5 seconds
 * ====================================================================== */

/* LONGCALL -- LM p. 1130, CM p. 1233.
 * Run the task whose 2CADR follows the TC LONGCALL after the double
 * precision delay in A,L (scaled as TIME2,TIME1). LONGCALL keeps the delay
 * in LONGTIME and the task in LONGCADR, then goes on in its own bank at
 * LNGCALL2. */
void longcall(const word task[2])
{
    dxch(ADDRESS_OF(LONGTIME));
    set_L(task[1]);
    set_A(task[0]);
    dxch(ADDRESS_OF(LONGCADR));
    set_L(LGCL2CDR[1]);
    set_A(LGCL2CDR[0]);
    dtcb((word)(LONGCALL + 7u));
}

/* LNGCALL2 -- LM p. 1131, CM p. 1234.
 * The caller's return (after the 2CADR) goes to LONGEXIT. The LM checks the
 * delay first: one that is not positive is alarm 1204 (LONGPOOH). */
void lngcall2(void)
{
    lxch(ADDRESS_OF(LONGEXIT) + 1u);
    set_A(TWO);
    A = add16(A, Q); /* ADS Q */
    Q = A;
    correct_L();
    ts(ADDRESS_OF(LONGEXIT));
#if LUMINARY099
    set_A(LONGTIME(0));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: continue_at(LONGCYCL); return;
    case NEGATIVE: break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO:
        set_A(LONGTIME(1));
        if (A != 0u && !(A & 0100000u)) {
            continue_at(LONGCYCL);
            return;
        }
        break;
    }
    /* LONGPOOH */
    dxch(ADDRESS_OF(LONGEXIT));
    tc_at((word)(LONGPOOH + 3u));
    continue_at(POODOO1);
#else
    continue_at(LONGCYCL);
#endif
}

/* LONGCYCL, LASTTIME, MUCHTIME -- LM pp. 1131-1132, CM pp. 1234-1235.
 * A WAITLIST task (and LONGCALL's first step): take BIT14 (about 1.25
 * minutes) off LONGTIME. If that much is left, wait it and come round
 * again (MUCHTIME); if not, wait what is left and then start the task
 * (LASTTIME, through GETCADR). */
void longcycl(void)
{
    set_L(negate(DPBIT14[1]));
    set_A(negate(DPBIT14[0]));
    das(ADDRESS_OF(LONGTIME));
    if (ccs(LONGTIME(1)) != POSITIVE) {
        switch (ccs(LONGTIME(0))) {
        case POSITIVE: break;
        case IS_PLUS_ZERO: /* lands on DPBIT14's words ("can't get here") */
            continue_at((word)ADDRESS_OF(DPBIT14));
            return;
        case NEGATIVE: continue_at((word)(ADDRESS_OF(DPBIT14) + 1u)); return;
        case IS_MINUS_ZERO: /* LASTTIME */
            set_A(BIT14);
            A = add16(A, sign_extend(LONGTIME(1))); /* ADS LONGTIME +1 */
            LONGTIME(1) = a_word();
            correct_L();
            tc_at((word)(LASTTIME + 2u)); /* TC WAITLIST; 2CADR GETCADR */
            continue_at(WAITLIST);
            return;
        }
    }
    /* MUCHTIME */
    set_A(BIT14);
    tc_at((word)(MUCHTIME + 1u)); /* TC WAITLIST; 2CADR LONGCYCL */
    continue_at(WAITLIST);
}

/* LONGRTRN -- LM p. 1132, CM p. 1234.
 * Back to LONGCALL's caller the first time; later passes come here from
 * MUCHTIME as a task, so LONGEXIT is set to end at TASKOVER from then on. */
void longrtrn(void)
{
    set_A(TSKOVCDR);
    dxch(ADDRESS_OF(LONGEXIT));
    dtcb((word)(LONGRTRN + 2u));
}

/* GETCADR -- LM p. 1132, CM p. 1235: the delay is over, start the task. */
void getcadr(void)
{
    dxch(ADDRESS_OF(LONGCADR));
    dtcb((word)(GETCADR + 1u));
}
