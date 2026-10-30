/* SPDX-License-Identifier: MIT */
/* Restarts: RESTARTS_ROUTINE.agc of Luminary099 (LM, listing pages
 * 1303-1308) and Comanche055 (CM, pages 1414-1419), the same code in both.
 * The faithful C is c64-port/src/lm_restart_select.c and
 * cm_restart_routines.c.
 *
 * RESTARTS reads the group's phase (TEMPPHS) and decides what to start:
 *   - a variable phase (bits 9-10 set, "TYPE A" or "B" in PHASE_TABLE_
 *     MAINTENANCE's terms) starts the job, task or LONGCALL whose 2CADR,
 *     priority or delta time the group keeps (PHSNAME, PHSPRDT);
 *   - phase 1 (an "X.1 restart") starts the display restart job INITDSP;
 *   - any other phase is a table phase: its entry in RESTART_TABLES gives
 *     the priority or delta time and the 2CADR. An odd phase has one entry,
 *     an even phase two (PHSPART2 starts the second).
 * A positive priority starts a job by FINDVAC, a negative one by NOVAC. A
 * negative 2CADR is a task: its delta time is direct, or indirect (stored
 * at an address), or -0 (now). A negative GENADR with a positive BBCON is
 * a LONGCALL, its double delta time at an address.
 *
 * Most of it runs in bank 01; ITSINDIR and ITSLGCL1, which switch to the
 * task's banks to read its delta time, are in fixed-fixed memory ("THIS
 * MUST BE IN FIXED FIXED").
 *
 * Every word of memory and every register ends as the original leaves it
 * (STYLE.md, Fidelity). Each entry ends where the faithful port's routine
 * does: at TC GOLOC -1, at the display job's TC FINDVAC, or at SWRETURN. */
#include "restart.h"
#include "../agc/names.h"

static void itsavar(void);
static void itsawait(void);
static void timetest(void);
static void itsindir(void);
static void findtime(void);
static void imediate(void);
static void itslikeb(void);
static void itsajob(void);
static void chknovac(void);
static void itsnovac(void);
static void itsatbl(void);
static void contbl2(void);
static void itslgcl1(void);
static void itslgcl2(void);
static void longclcl(void);
static void itslngcl(void);
static void itswtlst(void);
static void itsajob2(void);
static void itseven(void);

/* The erasable addresses of GOLOC -1 to GOLOC +2. */
#define GOLOC_ADDRESS(n) ((unsigned)(K_GOLOC + (unsigned)(n)))

/* TC GOLOC -1 at fixed address at: run the call set up in GOLOC -1. */
static void tc_goloc_minus1(word at)
{
    tc_at(at);
    continue_at((word)GOLOC_ADDRESS(-1));
}

/* TC RTRNCADR, whose instruction is TC SWRETURN: on to the next group. */
static void tc_rtrncadr(void)
{
    tc_at(RTRNCADR);
    continue_at(SWRETURN);
}

/* NDX TEMP2G on a group's word (TEMP2G is the group less one, doubled). */
static word of_group(unsigned address)
{
    return fetch(indexed(address, TEMP2G));
}

/* NDX POINTER on a word of the restart tables (bank 01). */
static word in_table(word address)
{
    return fetch_fixed((word)indexed(address, POINTER));
}

/* ======================================================================
 * RESTARTS: which kind of phase
 * ====================================================================== */

/* RESTARTS -- LM p. 1303, CM p. 1414.
 * GOPROG's entry, for one group. TEMP2G gets the group (less one, doubled,
 * "FOR INDEXING"); TEMPSWCH and GOLOC +2 the two ways out after the call. */
void restarts(void)
{
    set_A(MPAC(5)); /* "GET GROUP NUMBER -1" */
    double_A();
    ts(ADDRESS_OF(TEMP2G));
    set_A(PHS2CADR); /* "SET UP EXIT IN CASE IT IS AN EVEN TABLE PHASE" */
    ts(ADDRESS_OF(TEMPSWCH));
    set_A(RTRNCADR_WORD); /* "TO SAVE TIME ASSUME IT WILL GET NEXT GROUP AFTER THIS" */
    ts(GOLOC_ADDRESS(2));
    set_A(TEMPPHS);
    mask(OCT1400);
    /* CCS A: "IS IT A VARIABLE OR TABLE RESTART" (A is never negative) */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        itsavar(); /* "IT'S A VARIABLE RESTART" */
        return;
    }
    getpart2();
}

/* GETPART2 -- LM p. 1303, CM p. 1414.
 * A table phase, or the table part of a type B phase (GOLOC +2 comes here
 * after its job starts). Phase 1 starts the display restart job. */
void getpart2(void)
{
    switch (ccs(TEMPPHS)) { /* "IS IT AN X.1 RESTART" */
    case POSITIVE:
        /* CCS A: the phase less one, never negative. */
        if (ccs_register(A_ADDRESS) == POSITIVE) {
            itsatbl(); /* "NO, ITS A TABLE RESTART" */
            return;
        }
        set_A(PRIO14); /* "IT IS AN X.1 RESTART, THEREFORE START THE DISPLAY RESTART JOB" */
        break;
    case IS_PLUS_ZERO: itsatbl(); return;
    case NEGATIVE: set_A(PRIO14); break;
    case IS_MINUS_ZERO: break;
    }
    tc_at((word)(GETPART2 + 4u)); /* TC FINDVAC; 2CADR INITDSP */
    continue_at(FINDVAC);
}

/* GETPART2, after FINDVAC returns -- LM p. 1303, CM p. 1414.
 * "FINISHED WITH THIS GROUP, GET NEXT ONE". */
void getpart2_after_findvac(void)
{
    tc_at((word)(GETPART2 + 7u)); /* TC RTRNCADR */
    tc_rtrncadr();
}

/* ======================================================================
 * Variable phases: what the group keeps
 * ====================================================================== */

/* ITSAVAR -- LM p. 1303, CM p. 1414.
 * A is the phase's bits 9-10, less one: bit 10 set is type B. Otherwise the
 * phase's low bits say what the group's 2CADR is: 2 a job, 0 or 1 a task,
 * 3 or more a LONGCALL. */
static void itsavar(void)
{
    mask(OCT1400);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        itslikeb(); /* "YES, IT IS TYPE B" */
        return;
    }
    dca(indexed(ADDRESS_OF(PHSNAME1), TEMP2G)); /* "STORE THE JOB (OR TASK) 2CADR FOR EXIT" */
    dxch(GOLOC_ADDRESS(0));
    set_A(TEMPPHS); /* "SEE IF THIS IS A JOB, TASK, OR A LONGCALL" */
    mask(OCT7);
    ad(MINUS2);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: itslngcl(); return;        /* "ITS A LONGCALL" */
    case IS_PLUS_ZERO: tc_rtrncadr(); return; /* "CANT GET HERE" */
    case NEGATIVE: itsawait(); return;
    case IS_MINUS_ZERO: itsajob(); return; /* "ITS A JOB" */
    }
}

/* ITSAWAIT -- LM p. 1304, CM p. 1415.
 * A task: its delta time is the group's PHSPRDT ("DIRECTLY STORED"). */
static void itsawait(void)
{
    set_A(WTLTCADR); /* "SET UP WAITLIST CALL" */
    ts(GOLOC_ADDRESS(-1));
    set_A(of_group(ADDRESS_OF(PHSPRDT1)));
    timetest();
}

/* TIMETEST -- LM p. 1304, CM p. 1415.
 * The delta time in A: positive (or +0) is the time itself, negative the
 * complement of the address that holds it, -0 "NOW". */
static void timetest(void)
{
    switch (ccs_register(A_ADDRESS)) { /* "IS IT AN IMMEDIATE RESTART" */
    case POSITIVE:
        incr_A();
        findtime();
        return; /* "NO." */
    case IS_PLUS_ZERO: findtime(); return;
    case NEGATIVE: itsindir(); return;      /* "STORED INDIRECTLY" */
    case IS_MINUS_ZERO: imediate(); return; /* "IT WANTS AN IMMEDIATE RESTART" */
    }
}

/* ITSINDIR -- LM p. 1304, CM p. 1415 (in fixed-fixed).
 * The time at an address in the task's banks: A holds the address less one.
 * The task's BBCON (GOLOC +1) goes into BB for one CA, then back. */
static void itsindir(void)
{
    lxch(GOLOC_ADDRESS(1)); /* "GET THE CORRECT E BANK IN CASE THIS IS SWITCHED ERASABLE" */
    lxch(BBANK_ADDRESS);
    set_A(read_word((word)indexed(1u, a_word()))); /* "GET THE TIME INDIRECTLY" */
    lxch(BBANK_ADDRESS);                           /* "RESTORE THE BB AND GOLOC" */
    lxch(GOLOC_ADDRESS(1));
    findtime();
}

/* FINDTIME and IMEDIATE -- LM pp. 1304-1305, CM pp. 1415-1416.
 * How long the task still has to wait: its delta time less the time since
 * the group's TBASE (when it was put on the WAITLIST), at least 1 (10 ms).
 * IMEDIATE starts it in 10 ms. */
static void findtime(void)
{
    com();         /* "MAKE NEGATIVE SINCE IT WILL BE SUBTRACTED" */
    ts(L_ADDRESS); /* "AND SAVE" */
    set_A(negate(of_group(ADDRESS_OF(TBASE1))));
    su(TIME1);
    /* -(TBASE - TIME1): the time since TBASE, with TIME1 counted modulo
     * 2^14 (OCT37776 + 1 puts back what a wrap took). */
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: com();            /* fall through */
    case IS_PLUS_ZERO: ad(OCT37776); /* fall through */
    case NEGATIVE: ad(ONE);          /* fall through */
    case IS_MINUS_ZERO: break;
    }
    A = add16(A, L);                                      /* AD L */
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(ZERO); /* CA ZERO; TCF +2 */
    imediate();
}

static void imediate(void)
{
    ad(ONE);
    tc_goloc_minus1((word)(IMEDIATE + 1u));
}

/* ITSLIKEB -- LM p. 1305, CM p. 1416.
 * Type B: start the group's job, then (GOLOC +2 = GETPART2) the table
 * phase in the phase's low 7 bits. */
static void itslikeb(void)
{
    set_A(RTRNCADR_WORD); /* "TYPE B, SO STORE RETURN IN TEMPSWCH IN CASE OF AN EVEN PHASE" */
    ts(ADDRESS_OF(TEMPSWCH));
    set_A(PRT2CADR); /* "SET UP EXIT TO GET TABLE PART OF THIS VARIABLE TYPE OF PHASE" */
    ts(GOLOC_ADDRESS(2));
    set_A(TEMPPHS); /* "MAKE THE PHASE LOOK RIGHT FOR THE TABLE PART" */
    mask(OCT177);
    ts(ADDRESS_OF(TEMPPHS));
    dca(indexed(ADDRESS_OF(PHSNAME1), TEMP2G)); /* "OBTAIN THE JOB'S 2CADR" */
    dxch(GOLOC_ADDRESS(0));
    itsajob();
}

/* ITSAJOB, CHKNOVAC and ITSNOVAC -- LM p. 1305, CM p. 1416.
 * A job: "NOW ADD THE PRIORITY AND LET'S GO". A positive priority starts it
 * by FINDVAC, a negative (or zero) one, complemented, by NOVAC. */
static void itsajob(void)
{
    set_A(of_group(ADDRESS_OF(PHSPRDT1)));
    chknovac();
}

static void chknovac(void)
{
    ts(GOLOC_ADDRESS(-1));      /* "SAVE PRIO UNTIL WE SEE IF ITS A FINDVAC OR A NOVAC" */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF ITSNOVAC */
        itsnovac();
        return;
    }
    set_A(FVACCADR);                        /* "POSITIVE, SET UP FINDVAC CALL." */
    xch(GOLOC_ADDRESS(-1));                 /* "PICK UP PRIO," */
    tc_goloc_minus1((word)(CHKNOVAC + 5u)); /* "AND GO" */
}

static void itsnovac(void)
{
    set_A(NOVACADR); /* "NEGATIVE, SET UP NOVAC CALL," */
    xch(GOLOC_ADDRESS(-1));
    com();                                  /* "CORRECT PRIO," */
    tc_goloc_minus1((word)(ITSNOVAC + 3u)); /* "AND GO" */
}

/* ======================================================================
 * Table phases: what RESTART_TABLES says
 * ====================================================================== */

/* ITSATBL -- LM p. 1305, CM p. 1416.
 * A is the phase less two. CYR cycles its low bit into the sign: "FIND OUT
 * IF THE PHASE IS ODD OR EVEN". An odd phase's entry is at SIZETAB +2g -1
 * plus 3/2 of the phase (SR halves it). */
static void itsatbl(void)
{
    ts(CYR_ADDRESS);
    switch (ccs_at(CYR_ADDRESS)) {
    case POSITIVE: /* TCF +1: "IT'S EVEN" */
    case IS_PLUS_ZERO: itseven(); return;
    case NEGATIVE: set_A(RTRNCADR_WORD); break;
    case IS_MINUS_ZERO: break;
    }
    /* "IN CASE THIS IS THE SECOND PART OF A TYPE B RESTART, WE NEED PROPER EXIT" */
    ts(GOLOC_ADDRESS(2));
    set_A(TEMPPHS); /* "SET UP POINTER FOR FINDING OUR PLACE IN THE RESTART TABLES" */
    ts(SR_ADDRESS);
    ad_at(SR_ADDRESS);
    ad(fetch_fixed((word)indexed(SIZETAB + 1u, TEMP2G)));
    ts(ADDRESS_OF(POINTER));
    contbl2();
}

/* CONTBL2 -- LM p. 1306, CM p. 1417.
 * "FIND OUT WHAT'S IN THE TABLE": a positive 2CADR is a job; a negative
 * one a task or a LONGCALL, told apart by the BBCON's bit 10 (a task's
 * BBCON is stored complemented too). */
static void contbl2(void)
{
    L = sign_extend(in_table((word)(CADRTAB + 1u))); /* EXTEND; NDX POINTER; DCA CADRTAB */
    A = sign_extend(in_table(CADRTAB));
    lxch(GOLOC_ADDRESS(1));            /* "STORE THE BB INFORMATION" */
    switch (ccs_register(A_ADDRESS)) { /* "IS IT A JOB OR IS IT TIMED" */
    case POSITIVE:
        incr_A();
        itsajob2();
        return; /* "POSITIVE. MUST BE A JOB" */
    case IS_PLUS_ZERO: itsajob2(); return;
    case NEGATIVE: incr_A(); break; /* "MUST BE EITHER A WAITLIST OR LONGCALL" */
    case IS_MINUS_ZERO: break;
    }
    ts(GOLOC_ADDRESS(0)); /* "LET'S STORE THE CORRECT CADR" */
    set_A(WTLTCADR);      /* "SET UP OUR EXIT TO WAITLIST" */
    ts(GOLOC_ADDRESS(-1));
    set_A(GOLOC(1)); /* "NOW FIND OUT IF IT IS A WAITLIST CALL" */
    mask(BIT10);     /* "THIS SHOULD BE ONE IF WE HAVE -BB" */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        itswtlst(); /* "IT IS A WAITLIST CALL" */
        return;
    }
    set_A(in_table(PRDTTAB)); /* "OBTAIN THE ORIGINAL DELTA T ADDRESS FOR THIS LONGCALL" */
    itslgcl1();
}

/* ITSLGCL1 -- LM p. 1306, CM p. 1417 (in fixed-fixed).
 * A LONGCALL's double delta time, at the address in A, in the task's
 * banks. */
static void itslgcl1(void)
{
    lxch(GOLOC_ADDRESS(1)); /* "OBTAIN THE CORRECT E BANK" */
    lxch(BBANK_ADDRESS);
    lxch(GOLOC_ADDRESS(1)); /* "AND PRESERVE OUR E AND F BANKS" */
    dca(a_word());          /* EXTEND; NDX A; DCA 0: "GET THE DELTA TIME" */
    lxch(GOLOC_ADDRESS(1)); /* "RESTORE OUR E AND F BANK" */
    lxch(BBANK_ADDRESS);    /* "RESTORE THE TASKS E AND F BANKS" */
    lxch(GOLOC_ADDRESS(1)); /* "AND PRESERVE OUR L" */
    itslgcl2();
}

/* ITSLGCL2 -- LM p. 1307, CM p. 1418.
 * "CALCULATE TIME LEFT": the delta time less the time since LONGBASE (when
 * the LONGCALL was made). Time left: LONGCALL with it; none: IMEDIATE. */
static void itslgcl2(void)
{
    dxch(ADDRESS_OF(LONGTIME));
    dcs(ADDRESS_OF(TIME2));
    das(ADDRESS_OF(LONGTIME));
    dca(ADDRESS_OF(LONGBASE));
    das(ADDRESS_OF(LONGTIME));
    switch (ccs(LONGTIME(0))) { /* "FIND OUT HOW THIS SHOULD BE RESTARTED" */
    case POSITIVE: longclcl(); return;
    case IS_PLUS_ZERO: break; /* TCF +2 */
    case NEGATIVE:
        set_A(ZERO);
        imediate();
        return; /* TCF IMEDIATE -3: CA ZERO */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs(LONGTIME(1))) {
    case POSITIVE: longclcl(); return;
    case IS_PLUS_ZERO:                 /* NOOP: "CAN'T GET HERE" */
    case NEGATIVE: set_A(ZERO); break; /* TCF IMEDIATE -3 */
    case IS_MINUS_ZERO: break;         /* TCF IMEDIATE */
    }
    imediate();
}

/* LONGCLCL -- LM p. 1307, CM p. 1418. "WE WILL GO TO LONGCALL". */
static void longclcl(void)
{
    set_A(LGCLCADR);
    ts(GOLOC_ADDRESS(-1));
    dca(ADDRESS_OF(LONGTIME)); /* "PREPARE OUR ENTRY TO LONGCALL" */
    tc_goloc_minus1((word)(LONGCLCL + 4u));
}

/* ITSLNGCL -- LM p. 1307, CM p. 1418.
 * A variable LONGCALL: the group's PHSPRDT is the complement of the
 * address of its delta time. */
static void itslngcl(void)
{
    set_A(WTLTCADR); /* "ASSUME IT WILL GO TO WAITLIST" */
    ts(GOLOC_ADDRESS(-1));
    set_A(negate(of_group(ADDRESS_OF(PHSPRDT1)))); /* "GET THE DELTA T ADDRESS" */
    itslgcl1();
}

/* ITSWTLST -- LM p. 1307, CM p. 1418. A table task. */
static void itswtlst(void)
{
    set_A(negate(GOLOC(1))); /* "CORRECT THE BBCON INFORMATION" */
    ts(GOLOC_ADDRESS(1));
    set_A(
        in_table(PRDTTAB)); /* "GET THE DT AND FIND OUT IF IT WAS STORED DIRECTLY OR INDIRECTLY" */
    timetest();
}

/* ITSAJOB2 -- LM p. 1308, CM p. 1419. A table job. */
static void itsajob2(void)
{
    xch(GOLOC_ADDRESS(0));    /* "STORE THE CADR" */
    set_A(in_table(PRDTTAB)); /* "ADD THE PRIORITY AND LET'S GO" */
    chknovac();
}

/* ITSEVEN -- LM p. 1308, CM p. 1419.
 * An even phase: its first entry is at SIZETAB +2g -2 plus three times the
 * phase. GOLOC +2 goes on to PHSPART2 for the second (or, for the table
 * part of a type B phase, to the next group). */
static void itseven(void)
{
    set_A(TEMPSWCH); /* "SET UP FOR EITHER THE SECOND PART OF THE TABLE, OR A RETURN" */
    ts(GOLOC_ADDRESS(2));
    set_A(fetch_fixed((word)indexed(SIZETAB, TEMP2G)));
    ad(TEMPPHS); /* "THIS MAY LOOK BAD BUT LET'S SEE YOU DO BETTER IN TIME OR NUMBER */
    ad(TEMPPHS); /*  OF LOCATIONS" */
    ad(TEMPPHS);
    ts(ADDRESS_OF(POINTER));
    contbl2();
}

/* PHSPART2 -- LM p. 1308, CM p. 1419.
 * The second entry of an even phase, three words on. "THIS WILL BE OUR LAST
 * TIME THROUGH THE EVEN TABLE, SO AFTER IT GET THE NEXT GROUP". */
void phspart2(void)
{
    set_A(THREE);
    ads(ADDRESS_OF(POINTER));
    set_A(RTRNCADR_WORD);
    ts(GOLOC_ADDRESS(2));
    contbl2();
}

/* ======================================================================
 * The call RESTARTS set up
 * ====================================================================== */

/* GOLOC -1 -- LM p. 1308, CM p. 1419.
 * TC GOLOC -1 runs the word there as an instruction: TC FINDVAC, NOVAC,
 * WAITLIST or LONGCALL, with the 2CADR to start in the next two words
 * (GOLOC). The call returns past them, to GOLOC +2. */
void goloc_minus1(void)
{
    tc_through(GOLOC_ADDRESS(-1));
}

/* GOLOC +2 -- LM p. 1308, CM p. 1419.
 * TC SWRETURN (the next group), TC PHSPART2 or TC GETPART2. */
void goloc_plus2(void)
{
    tc_through(GOLOC_ADDRESS(2));
}
