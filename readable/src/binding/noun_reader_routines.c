/* SPDX-License-Identifier: MIT */
/* The readable noun table readers (src/pinball_noun_tables/) as the
 * faithful port's apollo_noun_reader (c64-port/include/noun_reader.h,
 * c64-port/src/noun_reader.c), which PINBALL's faithful routines call: the
 * flight program links this one in its place.
 *
 * Plumbing only: point the machine at the caller's state and latches, with
 * the tables as its fixed memory; take the states the faithful reader
 * takes (its "documented caller domain"): this build's mission; Z the
 * reader's entry; FB bank 32 with the superbank bit set (bank 42), EB
 * agreeing with BBANK, ZERO +0, the latches in range; the words a reader
 * reads ordinary 15-bit ones (the faithful reader takes that as given, and
 * flight memory never holds anything else); the return address in A an
 * ordinary one (0000-7777, either sign of overflow); and the index
 * the reader takes in range (a noun 0-99 or -0 in NOUNREG; an even scale
 * factor index within the table, or -0, in SFTEMP1). Anything else is
 * refused, leaving every word as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "noun_reader.h"
#include "../pinball_noun_tables/pinball_noun_tables.h"
#include "../agc/names.h"

#if LUMINARY099
#define MISSION APOLLO_FIXED_LM
#else
#define MISSION APOLLO_FIXED_CM
#endif

#define BANK32 064000u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The index the entry reads its table by: in range, or -0 (which INDEX
 * takes as +0). */
static bool index_in_range(enum apollo_noun_reader_entry entry)
{
    word k;

    if (entry == APOLLO_LODNNTAB) {
        k = NOUNREG;
        return k == MINUS_ZERO || k <= 99u;
    }
    k = ERASABLE(ADDRESS_OF(SFTEMP1));
    return k == MINUS_ZERO || (!(k & 1u) && k + 1u < SFTAB_WORDS);
}

static bool as_expected(enum apollo_noun_reader_entry entry)
{
    static const word entries[3] = {LODNNTAB, GTSFOUT, GTSFIN};
    static const unsigned read[] = {ADDRESS_OF(NOUNREG), ADDRESS_OF(IDAD2TEM), ADDRESS_OF(IDAD3TEM),
                                    ADDRESS_OF(SFTEMP1), ADDRESS_OF(SFTEMP2)};
    unsigned i;

    if (Z != entries[entry] || FBANK != BANK32 || (EBANK & ~03400u) ||
        BBANK != (word)(BANK32 | (EBANK >> 8)) || agc.memory[ZERO_ADDRESS] != 0u ||
        (*agc.superbank & ~0160u) || !(*agc.superbank & 0100u) || *agc.interrupts > 1u ||
        overflow_corrected(A) > 07777u)
        return false;
    for (i = 0; i < sizeof read / sizeof read[0]; ++i)
        if (agc.memory[read[i]] > 077777u) return false;
    return index_in_range(entry);
}

bool apollo_noun_reader(apollo_overlay_state *state, apollo_noun_reader_io *io,
                        enum apollo_fixed_mission mission, enum apollo_noun_reader_entry entry)
{
    apollo_overlay_state before;

    if (!state || !io || mission != MISSION || (unsigned)entry > APOLLO_GTSFIN) return false;
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
    agc.dsalmout = NULL;
    agc.in_interrupt = NULL;
    agc.fixed_memory = noun_table_word;
    agc.channel_in = NULL;
    agc.channel_out = NULL;
    agc.leave = leave;
    if (!as_expected(entry)) return false;

    before = *state;
    if (setjmp(back)) {
        *state = before;
        return false;
    }
    if (entry == APOLLO_LODNNTAB)
        lodnntab();
    else if (entry == APOLLO_GTSFOUT)
        gtsfout();
    else
        gtsfin();
    return true;
}
