/* SPDX-License-Identifier: MIT */
/* The LM's T6 interrupt: T6-RUPT_PROGRAMS.agc of Luminary099 (LM, listing
 * pages 1403-1405). The faithful C is c64-port/src/lm_t6rupt.c.
 *
 * TIME6 counts down, and its interrupt comes "1 DINC AFTER T6 = 77777":
 * one count after it reaches -0. So T6JOBCHK, which the autopilot also
 * calls itself to see whether a T6RUPT is waiting, reads -0 in TIME6 as
 * "time's up", +0 as a fault (CCSHOLE) and a positive count as "still
 * counting".
 *
 * Channel 13 bit 15 (BIT15) is TIME6's enable: the clock counts only while
 * it is set.
 *
 * The binding (src/binding/rupt_routines.c) runs these with TIME6 not
 * negative (a negative count has no producer: T6JOBCHK would loop for
 * ever) and, at -0, NXT6ADR naming one of the three writers in EBANK 6. */
#include "t6rupt.h"
#include "../agc/names.h"

#if LUMINARY099

static void t6job(void);
static void enablet6(void);
static void goch56(void);
static void writeu_plus2(void);

/* DOT6RUPT -- LM p. 1405.
 * The T6RUPT lead-in comes here, in bank 17, with the interrupted BBANK in
 * L. Save it (BANKRUPT) and Q (QRUPT), run T6JOBCHK, and end the
 * interrupt: "TCF RESUME". */
void dot6rupt(void)
{
    lxch(ADDRESS_OF(BANKRUPT)); /* "(INTERRUPT LEAD INS CONTINUED)" */
    qxch(ADDRESS_OF(QRUPT));
    tc_at((word)(DOT6RUPT + 3u)); /* "CALL T6JOBCHK." */
    t6jobchk();
    if (Z == DOT6RUPT + 4u) continue_at(RESUME); /* it returned */
}

/* T6JOBCHK -- LM p. 1404.
 * "CHECK TIME6 FOR WAITING T6RUPT". With TIME6 still counting, return.
 * With TIME6 at -0 the T6RUPT is due: run its job (T6JOB). TIME6 at +0 is
 * a fault: TC CCSHOLE. */
void t6jobchk(void)
{
    switch (ccs_at(ADDRESS_OF(TIME6))) {
    case POSITIVE: /* "NONE: CLOCK COUNTING DOWN." */ tc_through(Q_ADDRESS); return;
    case IS_PLUS_ZERO:
        tc_at((word)(T6JOBCHK + 2u));
        continue_at(CCSHOLE);
        return;
    case NEGATIVE: /* TC T6JOBCHK +3: this word, for ever */
        tc_at((word)(T6JOBCHK + 3u));
        continue_at((word)(T6JOBCHK + 3u));
        return;
    case IS_MINUS_ZERO: t6job(); return;
    }
}

/* T6JOB -- LM p. 1404.
 * Stop the clock ("DISABLE CLOCK: NEEDED SINCE RUPT OCCURS 1 DINC AFTER T6
 * = 77777. FOR 625 MUSECS MUST NOT HAVE T6 = +0 WITH ENABLE SET") and move
 * the queue up: T6FURTHA into T6NEXT, T6NEXT into TIME6 and NXT6ADR, and
 * POSMAX (nothing queued) into T6FURTHA. Then, unless the new TIME6 is
 * POSMAX, start the clock again (ENABLET6); either way, fire the jets of
 * the moment just reached (GOCH56). */
static void t6job(void)
{
    set_A(POSMAX);
    wand_channel(CHAN13); /* EXTEND; WAND CHAN13 */
    set_A(POSMAX);
    zl();
    dxch(ADDRESS_OF(T6FURTHA));
    dxch(ADDRESS_OF(T6NEXT));
    lxch(ADDRESS_OF(NXT6ADR));
    /* TS TIME6; AD PRIO37; TS A: overflows (and skips) only for POSMAX,
     * which PRIO37 takes past the top. */
    if (!ts(ADDRESS_OF(TIME6))) ad(PRIO37);
    if (!ts(A_ADDRESS)) {
        enablet6(); /* TCF ENABLET6 */
        return;
    }
    set_A(POSMAX);
    ts(ADDRESS_OF(TIME6));
    goch56();
}

/* ENABLET6 -- LM p. 1404.
 * Start the clock (channel 13 bit 15). If what is now T6NEXT is POSMAX, or
 * overflows the same way, store POSMAX there: nothing more is queued. */
static void enablet6(void)
{
    set_A(BIT15);
    wor_channel(CHAN13); /* EXTEND; WOR CHAN13 */
    ca(ADDRESS_OF(T6NEXT));
    ad(PRIO37);
    if (ts(A_ADDRESS)) {
        set_A(POSMAX);
        ts(ADDRESS_OF(T6NEXT));
    }
    goch56();
}

/* GOCH56 -- LM p. 1404.
 * "INDEX L; TCF WRITEP -1": L (the old NXT6ADR) picks the writer, 0 for
 * WRITEP -1, 4 for WRITEU -1, 13 for WRITEV -1. Each returns by TC Q to
 * T6JOBCHK's caller. */
static void goch56(void)
{
    word to = (word)indexed((word)(WRITEP - 1u), overflow_corrected(L));

    if (to == WRITEP - 1u)
        writep_minus1();
    else if (to == WRITEU - 1u)
        writeu_minus1();
    else if (to == WRITEV - 1u)
        writev_minus1();
    else
        not_modelled(); /* NXT6ADR names no writer */
}

/* ======================================================================
 * The jet writers, in fixed-fixed memory (the autopilot calls them too)
 * ====================================================================== */

/* WRITEP -1 -- LM p. 1404.
 * The P-axis jets queued in NEXTP, then WRITEP. */
void writep_minus1(void)
{
    ca(ADDRESS_OF(NEXTP));
    writep();
}

/* WRITEP -- LM p. 1404-1405.
 * The P-axis jets in A into channel 6, and return. */
void writep(void)
{
    write_channel(CHAN6); /* EXTEND; WRITE CHAN6 */
    tc_through(Q_ADDRESS);
}

/* WRITEU -1 -- LM p. 1405.
 * The U-axis jets queued in NEXTU, then WRITEU. */
void writeu_minus1(void)
{
    ca(ADDRESS_OF(NEXTU));
    writeu();
}

/* WRITEU -- LM p. 1405.
 * The U-axis jets in A into channel 5, keeping the V-axis jets there (the
 * bits of 00314OCT). A TS L that overflows skips the CS. */
void writeu(void)
{
    if (!ts(L_ADDRESS)) set_A(negate(OCT_00314)); /* CS 00314OCT */
    writeu_plus2();
}

/* WRITEU +2 -- LM p. 1405: WRITEU and WRITEV go on here.
 * Channel 5's bits that A keeps, plus the new jets in L, into channel 5;
 * return. */
static void writeu_plus2(void)
{
    rand_channel(CHAN5); /* EXTEND; RAND CHAN5 */
    ad_at(L_ADDRESS);
    write_channel(CHAN5); /* EXTEND; WRITE CHAN5 */
    tc_through(Q_ADDRESS);
}

/* WRITEV -1 -- LM p. 1405.
 * The V-axis jets queued in NEXTV, then WRITEV. */
void writev_minus1(void)
{
    ca(ADDRESS_OF(NEXTV));
    writev();
}

/* WRITEV -- LM p. 1405.
 * The V-axis jets in A into channel 5, keeping the U-axis jets (all but
 * the bits of 00314OCT): "TS L; CA 00314OCT; TCF -9D", back into WRITEU
 * at its RAND. */
void writev(void)
{
    if (!ts(L_ADDRESS)) set_A(OCT_00314);
    writeu_plus2();
}

#endif
