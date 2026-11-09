/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, INTSTALL and INTWAKE (LM pp. 1219-1220,
 * CM pp. 1324-1326), and the basic code after INTEXIT's EXIT (LM p. 1217,
 * CM p. 1321), which goes on into INTWAKE. Basic code in C; the EXITs and
 * GOBAC's RVQ are interpretive, in the listing's notation
 * (src/interpreter/notation.h).
 *
 * Orbital integration has one set of erasables, so one job at a time may
 * use it. INTSTALL takes it: with RASFLAG's INTBITAB bits clear, the job
 * sets INTFLBIT and goes on; otherwise it sleeps (JOBSLEEP) on WAKESTAL
 * until the user before it is done. INTWAKE gives it back: with REINTFLG
 * set (a phase change since INTSTALL) the caller's return is kept in
 * TBASE2 across a phase change of restart group 2; then every job asleep
 * on WAKESTAL is woken (JOBWAKE, until LOCCTR says none was found), and
 * the bits cleared ("RELEASE STALL AREA"). Either way the job is back in
 * the interpreter at GOBAC, and returns (RVQ).
 *
 * The CM has three stall areas, one bit and one WAKESTAL each: INTSTALL
 * and INTWAKE use the first (L, STALTEM 0); ERASTAL1, ERASTAL2, ERASWAK1
 * and ERASWAK2 the others (1, 2), INDEX picking the words. */
#include "integration_init.h"
#include "../agc/names.h"
#include "../executive/executive.h"
#include "../interpreter/interpreter.h"

/* ---- The interpretive words ---- */

static const word intstall_words[] = {
    /* INTSTALL -- LM p. 1219 (CM p. 1324): out of the interpreter. */
    /* INTSTALL */ ORDER(EXIT),
};

const struct interpretive_code INTSTALL_CODE = {CODE_BANK(I_INTSTALL), CODE_Z(I_INTSTALL),
                                                COUNT(intstall_words), intstall_words};

static const word intwake0_words[] = {
    /* INTWAKE0 -- LM p. 1220 (CM p. 1325): out of the interpreter, to
     * INTWAKE1. */
    /* INTWAKE0 */ ORDER(EXIT),
};

const struct interpretive_code INTWAKE0_CODE = {CODE_BANK(I_INTWAKE0), CODE_Z(I_INTWAKE0),
                                                COUNT(intwake0_words), intwake0_words};

static const word rvq_words[] = {
    /* GOBAC +1: back to the caller. */
    ORDER(RVQ),
};

/* The RVQ after GOBAC's TC INTPRET. */
const struct interpretive_code GOBAC_RVQ_CODE = {CODE_BANK(I_GOBAC), (word)(CODE_Z(I_GOBAC) + 1u),
                                                 COUNT(rvq_words), rvq_words};

#if COMANCHE055
static const word erastal1_words[] = {
    /* ERASTAL1 -- CM p. 1326: out of the interpreter, for the second
     * stall area. */
    /* ERASTAL1 */ ORDER(EXIT),
};

const struct interpretive_code ERASTAL1_CODE = {CODE_BANK(I_ERASTAL1), CODE_Z(I_ERASTAL1),
                                                COUNT(erastal1_words), erastal1_words};

static const word erastal2_words[] = {
    /* ERASTAL2: the third. */
    /* ERASTAL2 */ ORDER(EXIT),
};

const struct interpretive_code ERASTAL2_CODE = {CODE_BANK(I_ERASTAL2), CODE_Z(I_ERASTAL2),
                                                COUNT(erastal2_words), erastal2_words};
#endif

/* ---- The basic code ---- */

/* TC at "at" to a routine outside the file (PHASCHNG, JOBSLEEP,
 * JOBWAKE), which reads the words after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* A word's value as a signed number (ones' complement). */
static long value_of(word w)
{
    return (w & 040000u) ? -(long)(~w & 077777u) : (long)w;
}

/* INDEX: the address an instruction names plus the index, as the
 * faithful port takes it: from 0 to the last address given, and not Z (nor FB or BB for a
 * store, which would switch banks). Anything else is not modelled. */
static unsigned indexed_at(unsigned address, word index, unsigned last, bool store)
{
    long at = (long)address + value_of(index);

    if (at < 0 || at > (long)last || at == Z_ADDRESS) not_modelled();
    if (store && (at == FBANK_ADDRESS || at == BBANK_ADDRESS)) not_modelled();
    return (unsigned)at;
}

/* INDEX before a read: in fixed memory only the stall tables, three
 * words each (INTBITAB, WAKESTAL, and INTFLBIT, which is BIT14 and the
 * two bits after it), as the faithful port reads them. */
static unsigned indexed_read(unsigned address, word index)
{
    static const unsigned tables[] = {K_INTBITAB, K_WAKESTAL, K_INTFLBIT};
    unsigned at = indexed_at(address, index, 07777u, false), i;

    if (at < 02000u) return at;
    for (i = 0; i < COUNT(tables); ++i)
        if (at >= tables[i] && at < tables[i] + 3u) return at;
    not_modelled();
    return at;
}

/* The caller's return, QPRET, in the job's work area (INDEX FIXLOC), to
 * read or to store. */
static unsigned qpret_at(void)
{
    return indexed_read(ADDRESS_OF(QPRET), FIXLOC);
}

static unsigned qpret_store_at(void)
{
    return indexed_at(ADDRESS_OF(QPRET), FIXLOC, 01777u, true);
}

/* GOBAC: back into the interpreter, at the RVQ. */
static void gobac(void)
{
    tc_at(GOBAC); /* TC INTPRET */
    intpret();
}

/* OKTOGRAB -- LM p. 1220 (CM p. 1325): the stall area free, the job takes
 * it (its INTFLBIT bit into RASFLAG) and goes back to its caller. */
static void oktograb(void)
{
#if LUMINARY099
    ca(ADDRESS_OF(INTFLBIT)); /* CAF INTFLBIT */
    inhint();
#else
    ca(indexed_read(ADDRESS_OF(INTFLBIT), overflow_corrected(L))); /* INDEX L; CAF INTFLBIT */
#endif
    ads(ADDRESS_OF(RASFLAG));
    gobac();
}

/* After INTSTALL's EXIT: "IS THIS STALL AREA FREE". If so OKTOGRAB;
 * otherwise sleep on WAKESTAL (JOBSLEEP), until INTWAKE wakes the job at
 * INTSTALL +1 to try again. */
#if LUMINARY099
void intstall_after_exit(void)
{
    ca(ADDRESS_OF(RASFLAG));
    mask_at(INTBITAB); /* "IS THIS STALL AREA FREE" */
    if (A_is_zero()) { /* EXTEND; BZF OKTOGRAB: "YES" */
        oktograb();
        return;
    }
    ca(WAKESTAL); /* CAF WAKESTAL */
    tc_to((word)(INTSTALL + 6u), JOBSLEEP);
}
#else
/* ALLSTALL -- CM p. 1324: the stall area in A (0, 1 or 2), kept in L to
 * index its words. */
static void allstall(void)
{
    ts(L_ADDRESS); /* ALLSTALL TS L */
    ca(ADDRESS_OF(RASFLAG));
    mask_at(indexed_read(INTBITAB, overflow_corrected(L))); /* INDEX L; MASK INTBITAB */
    if (A_is_zero()) {                                      /* EXTEND; BZF OKTOGRAB: "YES" */
        oktograb();
        return;
    }
    ca(indexed_read(WAKESTAL, overflow_corrected(L))); /* INDEX L; CAF WAKESTAL */
    tc_to((word)(ALLSTALL + 010u), JOBSLEEP);
}

void intstall_after_exit(void)
{
    set_A(ZERO); /* CAF ZERO: the first stall area */
    allstall();
}

/* ERASTAL1 and ERASTAL2 -- CM p. 1326: the second and third areas. */
void erastal1_after_exit(void)
{
    set_A(ONE);
    allstall();
}

void erastal2_after_exit(void)
{
    set_A(TWO);
    allstall();
}
#endif

/* INTWAKE1 -- LM p. 1220 (CM p. 1325): wake a job asleep on WAKESTAL
 * (JOBWAKE, with interrupts inhibited); back at intwake1_after_jobwake.
 * In the CM, WAKE takes the stall area in A and keeps it in STALTEM
 * ("INDEX OF ANY STALL USER"); INTWAKE1 is the first. */
#if LUMINARY099
void intwake1(void)
{
    ca(WAKESTAL); /* CAF WAKESTAL */
    inhint();
    tc_to((word)(INTWAKE1 + 2u), JOBWAKE);
}
#else
static void wake1(void)
{
    ca(indexed_read(WAKESTAL, STALTEM)); /* WAKE1 INDEX STALTEM; CAF WAKESTAL */
    inhint();
    tc_to((word)(WAKE1 + 3u), JOBWAKE);
}

static void wake(void)
{
    ts(ADDRESS_OF(STALTEM)); /* WAKE TS STALTEM */
    wake1();
}

void intwake1(void)
{
    set_A(ZERO); /* CAF ZERO */
    wake();
}

/* ERASWAK1 and ERASWAK2 -- CM p. 1326: the second and third areas. */
void eraswak1(void)
{
    set_A(ONE);
    wake();
}

void eraswak2(void)
{
    set_A(TWO);
    wake();
}
#endif

/* After JOBWAKE: LOCCTR is the woken job's core set, so wake another
 * ("MAY BE MORE TO WAKE UP"); or -1, none, and the stall area's bits
 * are cleared ("RELEASE STALL AREA"). A +0 (the core set's offset can't
 * be) runs FORTYONE, a constant, and is not modelled; at -0 the CCS skips
 * the CS (in the CM, the INDEX). */
void intwake1_after_jobwake(void)
{
    switch (ccs_at(ADDRESS_OF(LOCCTR))) {
    case POSITIVE:
#if LUMINARY099
        intwake1(); /* TCF INTWAKE1 */
#else
        wake1(); /* TCF WAKE1 */
#endif
        return;
    case IS_PLUS_ZERO: not_modelled(); return; /* FORTYONE DEC 41 */
    case NEGATIVE:
#if LUMINARY099
        cs(INTBITAB);
        break;
    case IS_MINUS_ZERO: break;
#else
        cs(indexed_read(INTBITAB, STALTEM)); /* INDEX STALTEM; CS INTBITAB */
        break;
    case IS_MINUS_ZERO: cs(INTBITAB); break;
#endif
    }
    mask_at(ADDRESS_OF(RASFLAG));
    if (!ts(ADDRESS_OF(RASFLAG))) /* "RELEASE STALL AREA" */
        relint();
    gobac(); /* TCF GOBAC */
}

/* INTWAKE -- LM p. 1220 (CM p. 1325): "IS THIS INTSTALLED ROUTINE TO BE
 * RESTARTED". REINTBIT clear in RASFLAG: "NO", INTWAKE1. Set: the
 * caller's return (QPRET) kept in TBASE2 ("DON'T RESTART WITH SOMEONE
 * ELSE'S Q") across group 2's phase change (OCT 04022). */
static void keep_qpret(void)
{
    ca(qpret_at()); /* INDEX FIXLOC; CA QPRET */
    ts(ADDRESS_OF(TBASE2));
    tc_to((word)(INTWAKE + 7u), PHASCHNG); /* OCT 04022 */
}

void intwake(void)
{
    cs(ADDRESS_OF(RASFLAG));
    mask(REINTBIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        tc_at((word)(INTWAKE + 3u)); /* TC INTWAKE1: "NO" */
        intwake1();
        return;
    case IS_PLUS_ZERO: keep_qpret(); return;
    case NEGATIVE: ca(ADDRESS_OF(QPRET)); break; /* CA QPRET, unindexed */
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(TBASE2));
    tc_to((word)(INTWAKE + 7u), PHASCHNG);
}

#if LUMINARY099
/* INTWAKE +4 in the LM, the return from INTWAKE +3's TC INTWAKE1. */
void intwake_after_intwake1(void)
{
    keep_qpret();
}
#endif

/* After the phase change: the return back in QPRET, and "DON'T INTWAKE
 * IF WE CAME HERE VIA RESTART" (REINTBIT clear); otherwise INTWAKE1. */
void intwake_after_phaschng(void)
{
    ca(ADDRESS_OF(TBASE2));
    if (!ts(qpret_store_at())) /* INDEX FIXLOC; TS QPRET */
        set_A(REINTBIT);       /* CAF REINTBIT */
    mask_at(ADDRESS_OF(RASFLAG));
    if (A_is_zero()) { /* EXTEND; BZF GOBAC */
        gobac();
        return;
    }
    intwake1();
}

/* INTEXIT's EXIT -- LM p. 1217 (CM p. 1321): the caller's return (from
 * IRETURN, in MPAC) into QPRET, and INTWAKE, which ends at the RVQ. */
void intexit_after_exit(void)
{
    ca(ADDRESS_OF(MPAC));
    if (ts(qpret_store_at()))      /* INDEX FIXLOC; TS QPRET */
        not_modelled();            /* (the skip would run RVCON's words) */
    tc_at((word)(INTEXIT + 015u)); /* TC INTWAKE */
    intwake();
}
