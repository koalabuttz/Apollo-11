/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 6: the clock update, verb 55, and the LM's
 * ATTCK2 -- LM pp. 275 and 282, CM p. 246. The faithful C is
 * c64-port/src/lanec_ev_time.c.
 *
 * Verb 55 adds a time the crew loads to the computer's clock (TIME2,
 * TIME1): flashing V25 N24, and only on verb 23's data in. The CM's
 * ALINTIME does it in bank 43; the LM's ALINTIME (part 2's) has "NO ROOM
 * IN 43" and goes on in R33, in bank 42. ATTCK2, also in the LM's bank 42,
 * is the task V43 (part 3's IMUATTCK) starts: it puts the attitude errors
 * out to the CDUs' error needles.
 *
 * Each call out of the file ends a C function with Q and Z set (tc()
 * below, or continue_at()), and the word it returns to is an entry of its
 * own; the binding goes on from one entry of this part to the next while Z
 * is one. A routine that falls into the next label calls it. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* A flashing display from the word at: CAF <verb-noun>, TC BANKCALL, CADR
 * GOXDSPF (the CM's GOMARKF, the same routine). */
static void show(word at, word verb_noun)
{
    set_A(verb_noun);
    tc(at + 1u, BANKCALL);
}

/* "DATA IN OR RESEQUENCE (UNLIKELY)": the verb that answered the display,
 * which PINBALL leaves in MPAC ("RECALL LEFT VERB IN MPAC"), is 23. */
static bool data_in(void)
{
    set_A(negate(DEC23)); /* CS DEC23 */
    ad_at(ADDRESS_OF(MPAC));
    return A_is_zero(); /* EXTEND; BZF UPDATIME */
}

#if LUMINARY099
/* ---- V55 (bank 42) ---- */

/* R33 -- LM p. 275, where ALINTIME's POSTJUMP goes: at priority 7. */
void r33(void)
{
    set_A(PRIO7);
    tc(R33 + 1u, PRIOCHNG);
}

/* R33 +2 -- LM p. 275: V25 N24, "LOAD DELTA TIME FOR AGC CLOCK". */
void r33_after_priochng(void)
{
    show(R33 + 2u, VNLODDT); /* CADR GOXDSPF */
}

/* R33 +5 and +6 -- LM p. 275: terminate, and proceed, end it. */
void r33_after_terminate(void)
{
    tc(R33 + 5u, ENDEXT);
}

void r33_after_proceed(void)
{
    tc(R33 + 6u, ENDEXT);
}

/* R33 +7 -- LM p. 275: "GO AHEAD WITH UPDATE ONLY IF RECALL WITH V23
 * (DATA IN)". */
void r33_after_enter(void)
{
    if (data_in()) {
        continue_at(UPDATIME);
        return;
    }
    tc(R33 + 11u, ENDEXT);
}
#else
/* ---- V55 (bank 43) ---- */

/* ALINTIME -- CM p. 246.
 * Verb 55, the clock update (R33): claim the display. */
void alintime(void)
{
    tc(ALINTIME, TESTXACT);
}

/* ALINTIME +1 -- CM p. 246: V25 N24, "LOAD DELTA TIME FOR AGC CLOCK". */
void alintime_after_testxact(void)
{
    show(ALINTIME + 1u, VNLODDT); /* CADR GOMARKF */
}

/* ALINTIME +4 and +5 -- CM p. 246: terminate, and proceed, end it. */
void alintime_after_terminate(void)
{
    tc(ALINTIME + 4u, ENDEXT);
}

void alintime_after_proceed(void)
{
    tc(ALINTIME + 5u, ENDEXT);
}

/* ALINTIME +6 -- CM p. 246: "GO AHEAD WITH UPDATE ONLY IF RECALL WITH V23
 * (DATA IN)". */
void alintime_after_enter(void)
{
    if (data_in()) {
        continue_at(UPDATIME);
        return;
    }
    tc(ALINTIME + 10u, ENDEXT);
}
#endif

/* UPDATIME -- LM p. 275, CM p. 246.
 * With interrupts held: "DELTA TIME IS IN DSPTEM1, +1" (DSPTEM2 +1 and +2
 * as the listing addresses it). MPAC +2 zero, "NEEDED FOR TP AGREE"; the
 * clock into MPAC and zero in its place ("ZERO T1 + 2 WHILE ALIGNING");
 * the increment added; then TPAGREE, "FORCE SIGN AGREEMENT". */
void updatime(void)
{
    inhint();
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 2u);
    ts(L_ADDRESS);
    dxch(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(DSPTEM2) + 1u); /* "INCREMENT" */
    das(ADDRESS_OF(MPAC));
    tc(UPDATIME + 8u, TPAGREE);
}

/* UPDATIME +9 -- LM p. 275, CM p. 246: the "NEW CLOCK" added to what the
 * clock counted meanwhile. */
void updatime_after_tpagree(void)
{
    dxch(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(TIME2));
    relint();
    updtmend();
}

/* UPDTMEND -- LM p. 275, CM p. 246. */
void updtmend(void)
{
    tc(UPDTMEND, ENDEXT);
}

#if LUMINARY099
/* ---- ATTCK2 (bank 42) ---- */

/* ATTCK2 -- LM p. 282.
 * "PUT OUT COMMANDS": for the axes 2, 1 and 0 (Q the index), THETAD's
 * angle times 0.1 (ATTSCALE) into CDUZCMD, CDUYCMD and CDUXCMD ("CDU
 * WILL LIMIT EXCESS DATA"). */
void attck2(void)
{
    set_A(TWO);
    attck2_plus1();
}

/* ATTCK2 +1 -- LM p. 282, where the loop goes round with the next axis
 * (CCS Q leaves it in A: 1, then 0). Entered with any other A it does
 * not model. With the three done, the CDU drives (channel 14's bits 13-15)
 * on and the task over: "LEAVE ERROR COUNTERS ENABLED". */
void attck2_plus1(void)
{
    if (A > 2u) not_modelled();
    ts(Q_ADDRESS);
    ca(indexed(ADDRESS_OF(THETAD), A)); /* INDEX A; CA THETAD */
    mp(ATTSCALE);
    xch(indexed(ADDRESS_OF(CDUXCMD), Q)); /* INDEX Q; XCH CDUXCMD */
    if (ccs_register(Q_ADDRESS) == POSITIVE) {
        continue_at(ATTCK2 + 1u); /* TCF ATTCK2 +1 */
        return;
    }
    set_A(BITS13_14_15);
    wor_channel(014);
    continue_at(TASKOVER); /* TCF TASKOVER */
}
#endif
