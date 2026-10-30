/* SPDX-License-Identifier: MIT */
/* The Executive: EXECUTIVE.agc of Luminary099 (LM, listing pages 1103-1116)
 * and Comanche055 (CM, pages 1208-1220). The faithful C is the EXECUTIVE
 * half of c64-port/src/resident_kernel_body.inc, one call per instruction;
 * this file says the same thing routine by routine.
 *
 * Every word of memory and every register ends as the original leaves it
 * (STYLE.md, Fidelity), including the debris the listing lists. Comments
 * quote the listing where the code follows an AGC idiom. */
#include "executive.h"
#include "../agc/names.h"

static void novac2(void);
static void setloc(void);
static void spectest(void);
static void endfind(void);
static void findvac2(void);
static void chanjob(void);
static void chanjob_minus1(void);
static void chanjob_minus2(void);
static void chanjob_plus4(void);
static void endprchg_minus1(void);
static void jobslp2(void);
static void jobwake2(void);
static void prioch2(void);
static void prioch2_plus1(void);
static void ejscan(void);
static void dummyjob_then(bool idle);
static void nudirect(void);

/* ======================================================================
 * Starting a job
 * ====================================================================== */

/* NOVAC -- LM p. 1103, CM p. 1208.
 * Start a job that needs no VAC area (a basic job): A is its priority, the
 * 2CADR after the TC NOVAC is where it starts. The priority word also keeps
 * FAKEPRET in its low nine bits, which marks "no VAC area" for ENDOFJOB. */
void novac(const word job[2])
{
    word callers_fbank;

    inhint();
    A = add16(A, sign_extend(FAKEPRET)); /* AD FAKEPRET; TS NEWPRIO */
    correct_L();
    ts(ADDRESS_OF(NEWPRIO));
    /* The new job's 2CADR waits in NEWLOC (A,L get NEWLOC's old words). */
    set_A(job[0]);
    set_L(job[1]);
    dxch(ADDRESS_OF(NEWLOC));
    /* Enter the Executive's bank, keeping the caller's in EXECTEM1. */
    callers_fbank = FBANK;
    set_FBANK(EXECBANK);
    set_A(callers_fbank);
    EXECTEM1 = callers_fbank;
    novac2();
}

/* FINDVAC -- LM p. 1103, CM p. 1208.
 * Start a job that needs a VAC area (an interpretive job): A is its
 * priority and the 2CADR after the TC FINDVAC is where it starts. */
void findvac(const word job[2])
{
    inhint();
    ts(ADDRESS_OF(NEWPRIO));
    set_A(job[0]);
    set_L(job[1]);
    spvacin();
}

/* SPVACIN -- LM p. 1103, CM p. 1208. FINDVAC with the 2CADR in A,L. */
void spvacin(void)
{
    dxch(ADDRESS_OF(NEWLOC));
    set_A(EXECBANK);
    xch(FBANK_ADDRESS);
    findvac2();
}

/* SPVAC -- LM p. 1103, CM p. 1208.
 * SPVACIN for a caller whose return is two words earlier: Q -= 2. */
void spvac(void)
{
    Q = add16(Q, sign_extend(NEG2));
    correct_L();
    spvacin();
}

/* FINDVAC2 -- LM p. 1106, CM p. 1211.
 * Find a free VAC area. Five times over, the listing has
 *     CCS VACnUSE
 *     TCF VACFOUND
 * A free area's use word holds its own (positive) address; a used one's
 * holds +0. CCS continues 1, 2, 3 or 4 words on for a positive, +0,
 * negative or -0 word: a free area (1 on) lands on its TCF VACFOUND and a
 * used one (2 on) on the next area's CCS. A use word is never negative or
 * -0; if one were, the same rule would carry on from the word it lands on,
 * which is what this loop does. */
static void vacfound(void);
static void findvac2(void)
{
    unsigned at = 1; /* FINDVAC2 +1: CCS VAC1USE */

    EXECTEM1 = a_word(); /* the caller's FBANK */
    while (at < 013u) {
        at += 1u + (unsigned)ccs(fetch(VACNUSE_ADDRESS(at / 2u + 1u)));
        if (at < 013u && at % 2u == 0u) {
            vacfound();
            return;
        }
    }
    /* All five areas used: alarm 1201 ("no VAC areas"). */
#if LUMINARY099
    /* FINDVAC2 +13: LXCH EXECTEM1; CA Q; TC BAILOUT1; OCT 1201. */
    if (at == 013u) lxch(ADDRESS_OF(EXECTEM1));
    if (at <= 014u) A = Q;
    tc_at((word)(FINDVAC2 + 015u));
    continue_at(BAILOUT1);
#else
    /* FINDVAC2 +13: TC BAILOUT; OCT 1201; then VACFOUND at +15. */
    if (at == 013u) {
        tc_at((word)(FINDVAC2 + 013u));
        continue_at(BAILOUT);
        return;
    }
    if (at == 014u) { /* the OCT 1201 runs as TC 1201 */
        tc_at((word)(FINDVAC2 + 014u));
        continue_at(01201u);
        return;
    }
    vacfound();
#endif
}

/* VACFOUND -- LM p. 1106, CM p. 1211.
 * A holds the area's use word less one (CCS). Mark the area used (+0 in its
 * use word) and add the area's first word to the low nine bits of NEWPRIO:
 * the job's FIXLOC will point there. */
static void vacfound(void)
{
    A = add16(A, TWO);
    L = 0;                    /* ZL */
    lxch((unsigned)(A - 1u)); /* INDEX A; LXCH 0 -1: the use word */
    A = add16(A, sign_extend(NEWPRIO));
    NEWPRIO = a_word(); /* ADS NEWPRIO */
    novac2();
}

/* NOVAC2, NOVAC3, CORFOUND, NEXTCORE -- LM pp. 1106-1107, CM pp. 1211-1212.
 * Find a free core set, the one whose PRIORITY holds -0. LOCCTR steps
 * through the core sets (12 words each) while EXECTEM2 counts down from
 * NO.CORES. */
static void corfound(void);
static void novac2(void)
{
    LOCCTR = ZERO;
    set_A(NO_CORES);
    for (;;) {
        /* NOVAC3 */
        EXECTEM2 = a_word();
        switch (ccs(IN_CORE_SET(PRIORITY, LOCCTR))) {
        case IS_MINUS_ZERO: /* free */ corfound(); return;
        case POSITIVE: /* a job that is running or ready */
        case NEGATIVE: /* a sleeping job */ break;
        case IS_PLUS_ZERO: /* never: priorities are never +0 */
#if LUMINARY099
            /* The CCS lands on NO.CORES (DEC 7), which runs as TC 7. */
            tc_at((word)(NOVAC3 + 4u));
            continue_at(NO_CORES);
            return;
#else
            /* The CCS lands on NO.CORES (DEC 6), which runs as EXTEND, so
             * the TCF NEXTCORE after it runs as BZF NEXTCORE, on A = +0. */
            break;
#endif
        }
        /* NEXTCORE */
        A = add16(sign_extend(COREINC), sign_extend(LOCCTR));
        LOCCTR = a_word();
        switch (ccs(EXECTEM2)) {
        case POSITIVE: continue; /* more core sets to look at */
        case IS_PLUS_ZERO:       /* none free: alarm 1202 */
#if LUMINARY099
            lxch(ADDRESS_OF(EXECTEM1));
            /* fall through */
        case NEGATIVE:
            A = Q;
            /* fall through */
        case IS_MINUS_ZERO:
            tc_at((word)(NEXTCORE + 6u)); /* LXCH EXECTEM1; CA Q; TC BAILOUT1; OCT 1202 */
            continue_at(BAILOUT1);
            return;
#else
            tc_at((word)(NEXTCORE + 4u)); /* TC BAILOUT; OCT 1202 */
            continue_at(BAILOUT);
            return;
        case NEGATIVE: /* the OCT 1202 runs as TC 1202 */
            tc_at((word)(NEXTCORE + 5u));
            continue_at(01202u);
            return;
        case IS_MINUS_ZERO: /* on into CHANJOB -2 */ chanjob_minus2(); return;
#endif
        }
    }
}

/* CORFOUND -- LM p. 1107, CM p. 1212.
 * Claim the free core set: its PRIORITY gets the new priority and its
 * PUSHLOC the VAC area's address (the low nine bits). If it is core set 0
 * (no job was running), FIXLOC and OVFIND are set up for it at once. */
static void corfound(void)
{
    set_A(NEWPRIO);
    IN_CORE_SET(PRIORITY, LOCCTR) = NEWPRIO;
    set_A((word)(NEWPRIO & LOW9));
    IN_CORE_SET(PUSHLOC, LOCCTR) = a_word();
    switch (ccs(LOCCTR)) {
    case POSITIVE: setloc(); return;
    case IS_PLUS_ZERO: /* core set 0 */
        OVFIND = ZERO;
        /* fall through */
    case NEGATIVE:
        set_A(PUSHLOC);
        /* fall through */
    case IS_MINUS_ZERO: FIXLOC = a_word(); break;
    }
    spectest();
}

/* SPECTEST -- LM p. 1107, CM p. 1212.
 * The job is in core set 0: none was running, or the job JOBWAKE woke was
 * the one in core set 0. If the computer was idle (NEWJOB = -0), the job is
 * simply in place: NEWJOB becomes +0 ("+0 shows active job already set")
 * and LOC and BANKSET take its 2CADR. If another job is already waiting for
 * a switch (NEWJOB positive, "rare"), SETLOC compares the two. */
static void spectest(void)
{
    switch (ccs(NEWJOB)) {
    case POSITIVE: setloc(); return;
    case IS_PLUS_ZERO:
        tc_at((word)(SPECTEST + 2u)); /* TC CCSHOLE */
        continue_at(CCSHOLE);
        return;
    case NEGATIVE:
        tc_at((word)(SPECTEST + 3u)); /* TC CCSHOLE */
        continue_at(CCSHOLE);
        return;
    case IS_MINUS_ZERO:
        NEWJOB = a_word(); /* +0 */
        dxch(ADDRESS_OF(NEWLOC));
        dxch(ADDRESS_OF(LOC));
        endfind();
        return;
    }
}

/* SETLOC -- LM p. 1107, CM p. 1212.
 * Store the job's 2CADR in its core set's LOC and BANKSET. If its priority
 * is higher than that of the job NEWJOB names (the running job when NEWJOB
 * is +0), NEWJOB names this core set: the next job switch goes to it. */
static void setloc(void)
{
    dxch(ADDRESS_OF(NEWLOC));
    dxch(indexed(ADDRESS_OF(LOC), LOCCTR));
    set_A(negate(IN_CORE_SET(PRIORITY, NEWJOB)));
    A = add16(A, sign_extend(NEWPRIO));
    correct_L();
    if (A != 0u && !(A & 0100000u)) { /* NEWPRIO > that priority: not BZMF */
        set_A(LOCCTR);
        NEWJOB = LOCCTR;
    }
    endfind();
}

/* ENDFIND -- LM p. 1105, CM p. 1210.
 * Back to the caller's bank and to the word after the inline 2CADR. The
 * listing's TCF Q+2 goes to a shared INDEX Q; TC 2 in fixed-fixed (at NEG
 * +1), which leaves Q pointing after it. */
static void endfind(void)
{
    word return_address = (word)(Q + 2u);

    set_A(EXECTEM1);
    set_FBANK(EXECTEM1);
    tc_at((word)(NEG + 1u));
    continue_at((word)(return_address & 07777u));
}

/* ======================================================================
 * Changing jobs
 * ====================================================================== */

/* CHANG1 -- LM p. 1103, CM p. 1208.
 * A basic job yields to the higher-priority job NEWJOB names; it carries
 * on, when its turn comes again, at the TC CHANG1's return (Q). */
void chang1(void)
{
    lxch(Q_ADDRESS);
    set_A(EXECBANK);
    xch(BBANK_ADDRESS);
    chanjob();
}

/* CHANG2 -- LM p. 1103, CM p. 1208.
 * An interpretive job yields. Its LOC is stored negated: "negative LOC
 * shows job = interpretive". */
void chang2(void)
{
    set_A(negate(LOC));
    ts(L_ADDRESS);
    set_A(EXECBANK);
    set_BBANK(EXECBANK);
    chanjob_minus1();
}

/* CHANJOB -2, -1, CHANJOB -- LM p. 1108, CM p. 1213.
 * Enter with the job's LOC in L (-2 takes it from LOC) and its bank word in
 * A (-1 takes BANKSET). The superbank bits join the bank word (the BBCON),
 * and then LOC is in A and the BBCON in L. */
static void chanjob_minus2(void)
{
    lxch(ADDRESS_OF(LOC));
    chanjob_minus1();
}

static void chanjob_minus1(void)
{
    set_A(BANKSET);
    chanjob();
}

static void chanjob(void)
{
    inhint();
    ror_superbnk();
    xch(L_ADDRESS);
    chanjob_plus4();
}

/* Exchange the words core set 0 and core set NEWJOB/12 hold under one name,
 * as the listing does with three DXCHs (A and L are kept). */
static void exchange_core_set_pair(unsigned address)
{
    dxch(address);
    dxch(indexed(address, NEWJOB));
    dxch(address);
}

/* CHANJOB +4 to ENDPRCHG -- LM pp. 1108-1109, CM pp. 1213-1214.
 * The job switch. The job in core set 0 (LOC and BBCON in A,L) and the job
 * in core set NEWJOB/12 exchange core sets: LOC and BANKSET, MPAC +0 to +6,
 * MODE, PUSHLOC and PRIORITY. The new job's superbank goes to channel 7
 * and its FIXLOC (VAC area) is the low nine bits of its priority. A
 * negative PUSHLOC carries "overflow" (OVFIND) for a job not running. */
static void chanjob_plus4(void)
{
    unsigned i;

    executive_exchanging_core_sets(NEWJOB);
    /* LOC and BANKSET: core set 0 gets the new job's; A,L the old pair. */
    dxch(indexed(ADDRESS_OF(LOC), NEWJOB));
    dxch(ADDRESS_OF(LOC));
    set_A(BANKSET);
    write_superbnk();
    for (i = 0; i < 8u; i += 2u) exchange_core_set_pair(ADDRESS_OF(MPAC) + i);
    /* The old job keeps its OVFIND as the sign of its PUSHLOC. */
    set_A(ZERO);
    xch(ADDRESS_OF(OVFIND));
    if (A != 0u && A != 0177777u) {
        set_A(negate(PUSHLOC));
        PUSHLOC = a_word();
    }
    exchange_core_set_pair(ADDRESS_OF(PUSHLOC)); /* PUSHLOC and PRIORITY */
    set_A((word)(LOW9 & PRIORITY));
    FIXLOC = a_word();
    /* ... and the new job gets its OVFIND back from its PUSHLOC's sign. */
    switch (ccs(PUSHLOC)) {
    case POSITIVE: set_A(ZERO); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
        set_A(negate(PUSHLOC));
        /* fall through */
    case IS_MINUS_ZERO:
        PUSHLOC = a_word();
        set_A(ONE);
        xch(ADDRESS_OF(OVFIND));
        break;
    }
    endprchg_minus1();
}

/* ENDPRCHG -1, ENDPRCHG -- LM p. 1109, CM p. 1214.
 * NEWJOB goes to +0 (the job is in place) and the job starts: a basic job
 * (positive LOC) by DTCB to its 2CADR; an interpretive one (negative LOC)
 * through INTRSM, which picks up its next interpretive instruction. */
static void intrsm(void);
static void endprchg_minus1(void)
{
    NEWJOB = a_word();
    relint();
    dxch(ADDRESS_OF(LOC));
    if (A != 0u && !(A & 0100000u)) { /* basic */
        dtcb((word)(ENDPRCHG + 4u));
        return;
    }
    A = (reg16)(~A & 0177777u); /* COM; AD ONE; TS LOC */
    A = add16(A, ONE);
    ts(ADDRESS_OF(LOC));
    intrsm();
}

/* INTRSM -- INTERPRETER.agc, LM p. 1002, CM p. 1107.
 * Resume an interpretive job: LXCH BBANK; TCF INTPRET +3. The interpreter
 * rebuilds BANKSET, INTBIT15 and EDOP from the new BBANK and fetches the
 * next instruction at NEWOPS. */
static void intrsm(void)
{
    lxch(BBANK_ADDRESS);
    BANKSET = BBANK;
    INTBIT15 = (word)(BBANK & SIGN_BIT);
    EDOP = ZERO;
    set_A(INTBIT15);
    continue_at(NEWOPS);
}

/* JOBSLEEP, JOBSLP1 -- LM pp. 1105, 1110, CM pp. 1210, 1215.
 * Put the running job to sleep until JOBWAKE names its LOC (A): its
 * priority goes negative, its bank word is saved, and the scan for the next
 * job starts with -0 as the highest priority found. */
void jobsleep(void)
{
    ts(ADDRESS_OF(LOC));
    set_A(EXECBANK);
    set_FBANK(EXECBANK);
    /* JOBSLP1 */
    inhint();
    set_A(negate(PRIORITY));
    PRIORITY = a_word();
    set_A((word)(LOW7 & BBANK));
    ror_superbnk();
    BANKSET = a_word();
    set_A(MINUS_ZERO);
    jobslp2();
}

/* JOBSLP2 -- LM p. 1110, CM p. 1215: BUF +1 holds -(highest priority). */
static void jobslp2(void)
{
    BUF(1) = a_word();
    ejscan();
}

/* JOBWAKE -- LM p. 1105, CM p. 1210.
 * Wake the sleeping job whose LOC is A. It returns to the caller as NOVAC
 * does (Q is moved back two words to match). */
void jobwake(void)
{
    inhint();
    NEWLOC(0) = a_word();
    set_A(negate(TWO));
    A = add16(A, Q); /* CS TWO; ADS Q */
    Q = A;
    correct_L();
    set_A(EXECBANK);
    xch(FBANK_ADDRESS);
    jobwake2();
}

/* JOBWAKE2, JOBWAKE4, JOBWAKE3, WAKETEST -- LM p. 1111, CM p. 1216.
 * Look through the core sets for a sleeping job (negative priority) whose
 * LOC matches. Wake it: its priority goes positive again and NEWLOC gets
 * its 2CADR (the FBANK half from LOC, the EBANK half from its BANKSET).
 * Then it is placed as a new job would be. If none matches, LOCCTR is -1. */
static void waketest(void);
static void jobwake2(void)
{
    EXECTEM1 = a_word();
    LOCCTR = ZERO;
    set_A(NO_CORES);
    for (;;) {
        /* JOBWAKE4 */
        EXECTEM2 = a_word();
        switch (ccs(IN_CORE_SET(PRIORITY, LOCCTR))) {
        case NEGATIVE: /* asleep: is it the one? */
            set_A(negate(NEWLOC(0)));
            A = add16(A, sign_extend(IN_CORE_SET(LOC, LOCCTR)));
            if (A == 0u || A == 0177777u) {
                waketest();
                return;
            }
            break;
        case IS_PLUS_ZERO: /* never: the CCS lands on COREINC (DEC 12), a TC 14 */
            tc_at((word)(JOBWAKE4 + 4u));
            continue_at(COREINC);
            return;
        case POSITIVE: /* active */
        case IS_MINUS_ZERO: /* free */ break;
        }
        /* JOBWAKE3 */
        A = add16(sign_extend(COREINC), sign_extend(LOCCTR));
        LOCCTR = a_word();
        switch (ccs(EXECTEM2)) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO: /* no such job: LOCCTR = -1 */
            set_A(negate(ONE));
            /* fall through */
        case NEGATIVE:
            LOCCTR = a_word();
            /* fall through */
        case IS_MINUS_ZERO: endfind(); return;
        }
    }
}

/* WAKETEST, from the match on -- LM p. 1111, CM p. 1216. */
static void waketest(void)
{
    word cadr;

    set_A(negate(IN_CORE_SET(PRIORITY, LOCCTR)));
    NEWPRIO = a_word();
    IN_CORE_SET(PRIORITY, LOCCTR) = a_word();
    /* NEWLOC: the address within its bank (bank 2K's address range) ... */
    cadr = NEWLOC(0);
    set_A((word)((negate(FBANKMSK) & cadr)));
    A = add16(A, TWO_K);
    NEWLOC(0) = a_word();
    /* ... and NEWLOC +1: the FBANK from the CADR, the EBANK from BANKSET. */
    set_A((word)(cadr & FBANKMSK));
    A = add16(A, sign_extend(IN_CORE_SET(BANKSET, LOCCTR)));
    ts(ADDRESS_OF(NEWLOC) + 1u);
    switch (ccs(LOCCTR)) {
    case POSITIVE: setloc(); return;
    case IS_PLUS_ZERO: spectest(); return; /* asleep in core set 0 */
    case NEGATIVE: prioch2(); return;
    case IS_MINUS_ZERO: prioch2_plus1(); return;
    }
}

/* PRIOCHNG -- LM p. 1105, CM p. 1210.
 * Change the running job's priority to A. It keeps running (from the TC
 * PRIOCHNG's return) only while no other job has a higher priority. */
void priochng(void)
{
    inhint();
    NEWPRIO = a_word();
    set_A(EXECBANK);
    xch(BBANK_ADDRESS);
    BANKSET = a_word();
    A = Q;
    correct_L();
    prioch2();
}

/* PRIOCH2 -- LM p. 1112, CM p. 1217.
 * The job's return becomes its LOC; BUF = +0 tells EJSCAN that this is a
 * priority change, so the job keeps running if it is still the highest. */
static void prioch2(void)
{
    ts(ADDRESS_OF(LOC));
    prioch2_plus1();
}

static void prioch2_plus1(void)
{
    set_A(ZERO);
    BUF(0) = ZERO;
    set_A((word)(LOW9 & PRIORITY));
    A = add16(A, sign_extend(NEWPRIO));
    ts(ADDRESS_OF(PRIORITY));
    A = (reg16)(~A & 0177777u); /* COM: -(new priority) */
    correct_L();
    jobslp2();
}

/* ENDOFJOB, ENDJOB1 -- LM pp. 1105, 1113, CM pp. 1210, 1218.
 * The running job ends. Its core set is freed (PRIORITY -0) and its VAC
 * area, if it had one, too: the area's use word gets back its own address.
 * A job without one has FAKEPRET in its priority's low nine bits. */
void endofjob(void)
{
    executive_ending_job();
    set_A(EXECBANK);
    set_FBANK(EXECBANK);
    /* ENDJOB1 */
    inhint();
    set_A(MINUS_ZERO);
    BUF(1) = MINUS_ZERO;
    xch(ADDRESS_OF(PRIORITY));
    set_A((word)(a_word() & LOW9));
    ts(L_ADDRESS); /* L: the VAC area's address */
    set_A(negate(FAKEPRET));
    A = add16(A, L);
    correct_L();
    if (A == 0u || (A & 0100000u)) { /* BZMF: no VAC area */
        ejscan();
        return;
    }
    switch (ccs_register(L_ADDRESS)) {
    case POSITIVE: /* INDEX A; TS 0: the use word, just before the area */ ts((unsigned)A); break;
    case IS_PLUS_ZERO: ts(A_ADDRESS); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: continue_at((word)(EJSCAN + 1u)); return;
    }
    ejscan();
}

/* EJSCAN, EJ1, EJ2 -- LM pp. 1113-1115, CM pp. 1218-1219.
 * Find the job of highest priority among core sets 1 up. Each core set k
 * has four words in the listing:
 *     CCS PRIORITY +12k
 *     TC  EJ1           positive: an active job, compare it
 *     TC  CCSHOLE       +0: never
 *     TCF +1            negative (asleep) or -0 (free): next
 * EJ1 compares it with BUF +1, -(highest so far); EJ2 keeps it, with the
 * place of its CCS in BUF (from Q). Then core set 0's own priority, in
 * BUF +1 from JOBSLEEP, PRIOCHNG or ENDOFJOB, decides:
 *   - no active job at all: idle (DUMMYJOB);
 *   - BUF +0 (a PRIOCHNG whose job is still highest): carry on with it;
 *   - otherwise NEWJOB = 12k for the winner, found from its CCS
 *     instruction word: (CCS PRIORITY +12k) + (-CCSPR) = 12k. */
static void ejscan(void)
{
    word at = EJSCAN; /* the CCS of the core set being looked at */
    word offset;
    unsigned k;

    for (k = 1; k < CORE_SETS; ++k, at = (word)(at + 4u)) {
        switch (ccs(ERASABLE(ADDRESS_OF(PRIORITY) + CORE_SET_WORDS * k))) {
        case IS_PLUS_ZERO:
            /* For core set 3 the third word is the constant -CCSPR, which
             * runs as an instruction; for the others it is TC CCSHOLE. */
            if (k == 3u) {
                continue_at((word)(at + 2u));
                return;
            }
            tc_at((word)(at + 2u));
            continue_at(CCSHOLE);
            return;
        case NEGATIVE:
        case IS_MINUS_ZERO: continue;
        case POSITIVE: break;
        }
        /* EJ1: is it higher than the highest so far? */
        tc_at((word)(at + 1u));
        BUF(2) = a_word();
        A = add16(A, sign_extend(BUF(1)));
        correct_L();
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: set_A(negate(BUF(2))); break;
        case IS_PLUS_ZERO: break;
        case NEGATIVE:
        case IS_MINUS_ZERO: /* no: NOOP; INDEX Q; TC 2 */ tc_at((word)(EJ1 + 7u)); continue;
        }
        /* EJ2: the new highest. */
        BUF(1) = a_word();
        qxch(ADDRESS_OF(BUF));
        tc_at((word)(EJ2 + 4u)); /* INDEX BUF; TC 2 */
    }

    /* at is now CCS BUF +1. */
    switch (ccs(BUF(1))) {
    case POSITIVE:
        tc_at((word)(at + 1u));
        continue_at(CCSHOLE);
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(at + 2u));
        continue_at(CCSHOLE);
        return;
    case IS_MINUS_ZERO: /* nothing to run */ dummyjob_then(false); return;
    case NEGATIVE: break;
    }
    switch (ccs(BUF(0))) {
    case IS_PLUS_ZERO: /* a PRIOCHNG's job is still the highest */ endprchg_minus1(); return;
    case IS_MINUS_ZERO: /* CAF 0 -1 unindexed: no CCS word to read */
        continue_at((word)(at + 9u));
        return;
    case POSITIVE:
    case NEGATIVE: break;
    }
    /* INDEX A; CAF 0 -1: A is BUF - 1, the winner's CCS +1. */
    offset = (word)(A - 1u - EJSCAN);
    if (A < EJSCAN + 1u || offset % 4u || offset / 4u > CORE_SETS - 2u) {
        continue_at((word)(at + 8u)); /* not a CCS word */
        return;
    }
    k = offset / 4u + 1u;
    set_A((word)(010000u | (ADDRESS_OF(PRIORITY) + CORE_SET_WORDS * k))); /* CCS PRIORITY +12k */
    A = add16(A, sign_extend(MINUS_CCSPR));
    correct_L();
    NEWJOB = a_word();
    chanjob_minus2();
}

/* ======================================================================
 * The idle loop
 * ====================================================================== */

/* DUMMYJOB -- LM p. 1116, CM p. 1220.
 * Nothing to run: NEWJOB = -0, interrupts on, the COMP ACTY light off, and
 * round the idle loop at ADVAN. A job switch that finds nothing ready stops
 * at ADVAN: in this port the idle loop is its own routine. */
void dummyjob(void)
{
    dummyjob_then(true);
}

static void dummyjob_then(bool idle)
{
    set_A(MINUS_ZERO);
    NEWJOB = MINUS_ZERO;
    if (idle)
        dummyjob_plus2();
    else {
        relint();
        set_A(negate(TWO));
        wand_dsalmout();
        continue_at(ADVAN);
    }
}

void dummyjob_plus2(void)
{
    relint();
    set_A(negate(TWO));
    wand_dsalmout();
    advan();
}

/* ADVAN -- LM p. 1116, CM p. 1220.
 * One pass of the idle loop. A job ready in another core set (NEWJOB
 * positive) gets core set 0 through NUCHANG2. One already in place (+0) or
 * woken in core set 0 starts through NUDIRECT. With nothing (-0) the
 * computer runs its self-check (SELFCHK) until the next interrupt. */
void advan(void)
{
    switch (ccs(NEWJOB)) {
    case POSITIVE: nuchang2(); return;
    case IS_PLUS_ZERO: set_A(TWO); /* fall through */
    case NEGATIVE: nudirect(); return;
    case IS_MINUS_ZERO:
        set_A(SELFRET);
        ts(L_ADDRESS);
        set_A(SELFBANK);
        supdxchz_plus1();
        return;
    }
}

/* NUDIRECT -- LM p. 1116, CM p. 1220.
 * Turn the COMP ACTY light back on and start the job in core set 0. */
static void nudirect(void)
{
    wor_dsalmout();
    dxch(ADDRESS_OF(LOC));
    supdxchz();
}

/* SUPDXCHZ, SUPDXCHZ +1 -- LM p. 1116, CM p. 1220.
 * A basic job's start: LOC in L, BBCON in A. Set the superbank and BBANK,
 * then TC L: the word in L runs as an instruction, which for a job's LOC is
 * a TC to its first word (Q gets 2, the address after L). */
void supdxchz(void)
{
    xch(L_ADDRESS);
    supdxchz_plus1();
}

void supdxchz_plus1(void)
{
    word loc;

    write_superbnk();
    ts(BBANK_ADDRESS);
    tc_at((word)(SUPDXCHZ + 4u));
    loc = overflow_corrected(L);
    if (loc & 070000u) { /* not a TC: stop at L */
        continue_at(1u);
        return;
    }
    tc_at(1u);
    continue_at(loc);
}

/* NUCHANG2 -- LM p. 1110, CM p. 1215.
 * From the idle loop to a job waiting in another core set: interrupts off
 * (so NEWJOB cannot change to +0 under it), the light on, and the job
 * switch from CHANJOB +4. If NEWJOB did change ("very rare case"), the job
 * is already in place and starts through NUDIRECT. */
void nuchang2(void)
{
    inhint();
    switch (ccs(NEWJOB)) {
    case IS_PLUS_ZERO: relint(); /* fall through */
    case NEGATIVE:
        set_A(TWO);
        nudirect();
        return;
    case POSITIVE:
    case IS_MINUS_ZERO:
        set_A(TWO);
        wor_dsalmout();
        dxch(ADDRESS_OF(LOC));
        chanjob_plus4();
        return;
    }
}
