/* SPDX-License-Identifier: MIT */
/* TVCSTROKETEST.agc (Comanche055 pp. 979-983, bank 20): the "STROKE TEST
 * PACKAGE", which "GENERATES A WAVEFORM DESIGNED TO EXCITE BENDING" by
 * "DUMPING PULSE BURSTS" of the pad-loaded size ESTROKER into the pitch
 * outcounter, TVCPITCH, a burst every 20 ms: from each DAP pass (HACK,
 * which the pitch and yaw DAPs call while STROKER is not +0) and from a
 * WAITLIST task between them (HACKWLST). The waveform is a run of "STROKE
 * SETS", each a number of reversals of a slope a number of bursts long
 * (CARD, REVS and CADDY count them). STRKTSTI, the job verb 68 starts,
 * sets it up; the faithful C is c64-port/src/cm_tvc_stroke.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- STRKTSTI -- p. 981 ---- */

/* TSTINIT: CADDY and N the bursts of the first set's half amplitude, less
 * ("NOTE SGN CHNG FCADDY(+) TO CADDY(-)"); REVS its reversals less one,
 * FREVS; CARD the number of sets, less. */
static void tstinit(void)
{
    cs(ADDRESS_OF(FCADDY));
    ts(ADDRESS_OF(CADDY));
    ts(ADDRESS_OF(N));
    ca(ADDRESS_OF(FREVS));
    ts(ADDRESS_OF(REVS));
    cs(ADDRESS_OF(FCARD));
    ts(ADDRESS_OF(CARD));
}

/* STRKTSTI, the "STROKE TEST INITIALIZATION PKG": TSTINIT (TCR; its TC Q
 * leaves Q 3). Then STRKCHK, "CHECK FOR CSM/LM CONFIGURATION" (bit 14 of
 * DAPDATR1): CSM/LM, STROKER the burst size ESTROKER, to "BEGIN ON NEXT
 * DAP PASS"; ENDOFJOB. */
void strktsti(void)
{
    tstinit();
    tc_at(Q_ADDRESS);
    inhint(); /* STRKCHK */
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT14);
    if (!A_is_zero()) { /* EXTEND; BZF +3 */
        ca(ADDRESS_OF(ESTROKER));
        ts(ADDRESS_OF(STROKER));
    }
    continue_at(ENDOFJOB);
}

/* ---- HACK and HACKWLST -- pp. 982-983 ---- */

/* The words the file reads under INDEX: FCARD on (FCARD +4 and +7 with
 * CARD -1 to -4: the sets' reversals and bursts). */
static bool a_constant(unsigned address)
{
    return address >= ADDRESS_OF(FCARD) && address <= ADDRESS_OF(FCARD) + 011u;
}

/* INDEX CARD, then an order that reads the word at field: the field plus
 * CARD, a 12-bit address. Not modelled (the faithful port refuses them
 * too): a sum outside 0-07777 or at Z, or a word of fixed memory outside
 * FCARD's. */
static unsigned card_field(unsigned field)
{
    int sum = (int)field + value_of(fetch(ADDRESS_OF(CARD)));

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !a_constant((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* TC BUNKER, at the word at: on at the erasable word, which the program
 * runs as TC <its word> (bunker_return). */
static void tc_bunker(word at)
{
    tc_to(at, ADDRESS_OF(BUNKER));
}

/* The next set, "NEGATIVE REVS SET LAST PASS": CARD one nearer +0 (REVS
 * -0 skips the INCR). "ALL
 * SETS COMPLETED" (+0 or -0), STROKILL: STROKER that zero, "RESET (TO +0)
 * TO END TEST". Otherwise the set's "NO. REVERSALS (-)" to REVS, and its
 * "NO. BURSTS IN 1/2AMP", less, to N and CADDY. */
static void next_set(bool count)
{
    if (count) incr(ADDRESS_OF(CARD));
    cs(ADDRESS_OF(CARD));
    if (A_is_zero()) { /* EXTEND; BZF STROKILL */
        ts(ADDRESS_OF(STROKER));
        tc_bunker(HACK + 043u);
        return;
    }
    ca(card_field(ADDRESS_OF(FCARD) + 4u));
    ts(ADDRESS_OF(REVS));
    cs(card_field(ADDRESS_OF(FCARD) + 7u));
    ts(ADDRESS_OF(N));
    ts(ADDRESS_OF(CADDY));
    tc_bunker(HACK + 041u);
}

/* A burst, HACK +011: STROKER added to TVCPITCH, "RELEASE THE ERROR
 * COUNTERS" (WOR bit 11 of channel 14); "COUNT DOWN THE NO. BURSTS, THIS
 * SLOPE" (INCR CADDY). Bursts left (CADDY negative), "EXIT, WHILE ON A
 * SLOPE". Otherwise CCS REVS: positive, REVUP, a reversal (REVS one less;
 * CADDY twice N, "2 x 1/2AMP"); +0, REVUP +4, the "FINAL REVERSAL, THIS
 * SET" (REVS -1; CADDY N, "JUST RETURN TO ZERO"); either way "CHANGE SIGN
 * OF SLOPE" (STROKER), "EXIT AT A REVERSAL". (An overflow at the TS
 * CADDY skips the CS STROKER: STROKER takes the +1 or -1 it leaves in A.)
 * Negative or -0: the next set. */
static void burst(void)
{
    ca(ADDRESS_OF(STROKER));
    ads(ADDRESS_OF(TVCPITCH));
    set_A(BIT11);
    wor_channel(CHAN14);
    incr(ADDRESS_OF(CADDY));
    cs(ADDRESS_OF(CADDY));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        tc_bunker(HACK + 022u);
        return;
    }
    switch (ccs_at(ADDRESS_OF(REVS))) {
    case POSITIVE: /* REVUP */
        ts(ADDRESS_OF(REVS));
        ca(ADDRESS_OF(N));
        double_A();
        break;
    case IS_PLUS_ZERO: /* REVUP +4 */
        set_A(negate(ONE));
        ts(ADDRESS_OF(REVS));
        ca(ADDRESS_OF(N));
        break;
    case NEGATIVE: next_set(true); return;
    case IS_MINUS_ZERO: next_set(false); return;
    }
    if (!ts(ADDRESS_OF(CADDY))) cs(ADDRESS_OF(STROKER));
    ts(ADDRESS_OF(STROKER));
    tc_bunker(HACK + 056u);
}

/* HACK, "ENTRY (IN T5 RUPT) FROM TVCDAPS": "SAVE Q FOR DAP RETURN" in
 * BUNKER (QXCH); HACKWLST by WAITLIST in 20 ms; on at HACK +6. */
void hack(void)
{
    qxch(ADDRESS_OF(BUNKER));
    set_A(TWENTY_MS);
    tc_to(HACK + 3u, WAITLIST); /* 2CADR HACKWLST */
}

/* HACK +6, after the WAITLIST: TCF +3, the burst. */
void hack_after_waitlist(void)
{
    burst();
}

/* HACKWLST, the WAITLIST task: "BUNKER IS TC TASKOVER" (TCTSKOVR); the
 * burst. */
void hackwlst(void)
{
    set_A(TCTSKOVR);
    ts(ADDRESS_OF(BUNKER));
    burst();
}

/* BUNKER run as an instruction: TC <its word> (the DAP's return, or TC
 * TASKOVER). */
void bunker_return(void)
{
    tc_through(ADDRESS_OF(BUNKER));
}

#endif
