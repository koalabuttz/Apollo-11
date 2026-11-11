/* SPDX-License-Identifier: MIT */
/* pin-noun-z: the display verbs on a noun whose address is the Z register.
 *
 * A display verb reads the noun's words through NOUNADD (INDEX NOUNADD; CS
 * 2, and the like). When the address read is 5, the word is Z itself, which
 * by then holds the address after the instruction reading it: a routine
 * must have Z right at each such read (reads_at, src/pinball/pinball.h).
 * The port's own cases reach that at one of the eight reads only
 * (DSPDCGET's, pw-decimal-load). These start each of the verbs at its
 * entry with NOUNADD at 3, 4 and 5, so that each read in turn falls on Z:
 * DSPABC's, DSPAB's and DSPA's (CS 2, CS 1, CS 0), DSPB's (CS 1), DSPC's
 * (CS 2), DSPDPDEC's and READLO1's (DCA 0, which takes Z as either word).
 *
 * The states are the port's own cases from those entries, with NOUNADD
 * changed: its pin-transfer chains (c64-port/tests/
 * lanec_pin_transfer_cases.c), run on as chains until they leave the file,
 * and for DSPDPDEC, whose chains pin-transfer does not take (the verb
 * calls TPAGREE, outside the file, and comes back), its pin-gaps cases of
 * a normal noun (c64-port/tests/lanec_pin_gaps_cases.c), run as the one
 * step to that call.
 *
 * The native runner runs them through the program's routine tables, with
 * the readable Pinball in tests/pinball_transfer.py's runner; the oracle
 * runs the original program in Virtual AGC to the same place; and
 * tests/pinball_steps.c holds the readable steps to the faithful port's
 * (which refuses most of these states: the comparison that counts here is
 * the oracle's). Test only. */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"
#include "agc_transfer.h"

enum exec_status lanec_pin_chain_max(apollo_overlay_state *s, lanec_io *io,
                                     enum lanec_mission mission, uint16_t stop, uint16_t max_steps,
                                     uint16_t *steps);

#define ENTRIES   7u
#define PER_ENTRY 4u /* the port's cases from each entry, in each mission */
#define MAX_BASES (2u * ENTRIES * PER_ENTRY)
#define VARIANTS  3u
#define CASES     (MAX_BASES * VARIANTS)

/* The entries, by bank and address: DSPABC, DSPAB, DSPA, DSPB and DSPC in
 * bank 41 (the same words in both programs), then READLO, which runs on
 * into READLO1, and DSPDPDEC in bank 40 (LM p. 423 and p. 424, CM
 * pp. 341-342). */
static const uint16_t BANK41[5] = {02353u, 02360u, 02365u, 02373u, 02400u};
static const uint16_t BANK40[2][2] = {{02747u, 02771u}, {02655u, 02677u}};
#define DSPDPDEC_ENTRY 6

static struct {
    uint16_t number; /* the case of pin-transfer, or of pin-gaps for DSPDPDEC */
    int entry;
} bases[MAX_BASES];
static uint16_t base_count;
/* The entry of the case made last, for run. */
static int made_entry;

/* Which of the entries a state starts at, 0 to 6, or -1. */
static int entry_of(const apollo_overlay_state *s, const lanec_io *io, enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z], bank = lanec_current_bank(s, io);
    unsigned i;

    if (bank == 041u)
        for (i = 0; i < 5u; ++i)
            if (z == BANK41[i]) return (int)i;
    if (bank == 040u)
        for (i = 0; i < 2u; ++i)
            if (z == BANK40[m == LANEC_LM ? 0 : 1][i]) return (int)(5u + i);
    return -1;
}

/* The first PER_ENTRY cases of each entry in each mission. */
static void find_bases(void)
{
    static apollo_overlay_state s;
    static lanec_io io;
    static lanec_meta m;
    unsigned taken[2][ENTRIES] = {{0}};
    uint32_t k;
    int entry;

    for (k = 0; k < lanec_suite_pin_transfer.count; ++k) {
        lanec_suite_pin_transfer.make(k, &s, &io, &m);
        entry = entry_of(&s, &io, m.mission);
        if (entry >= 0 && entry != DSPDPDEC_ENTRY &&
            taken[m.mission == LANEC_LM ? 0 : 1][entry]++ < PER_ENTRY) {
            bases[base_count].number = (uint16_t)k;
            bases[base_count++].entry = entry;
        }
    }
    for (k = 0; k < lanec_suite_pin_gaps.count; ++k) {
        lanec_suite_pin_gaps.make(k, &s, &io, &m);
        if (entry_of(&s, &io, m.mission) == DSPDPDEC_ENTRY && s.words[LANEC_LM_MIXBR] == 1u &&
            taken[m.mission == LANEC_LM ? 0 : 1][DSPDPDEC_ENTRY]++ < PER_ENTRY) {
            bases[base_count].number = (uint16_t)k;
            bases[base_count++].entry = DSPDPDEC_ENTRY;
        }
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    if (!base_count) find_bases();
    made_entry = bases[n % base_count].entry;
    if (made_entry == DSPDPDEC_ENTRY)
        lanec_suite_pin_gaps.make(bases[n % base_count].number, s, io, m);
    else
        lanec_suite_pin_transfer.make(bases[n % base_count].number, s, io, m);
    /* NOUNADD (and MIXBR, above) are the same words in both programs. */
    s->words[LANEC_LM_NOUNADD] = (uint16_t)(3u + n / base_count % VARIANTS);
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static apollo_overlay_state entry;
    static lanec_io entry_io;
    uint16_t steps;

    if (made_entry != DSPDPDEC_ENTRY) return lanec_suite_pin_transfer.run(s, io, mission);
    /* DSPDPDEC's one step, to its TC TPAGREE, through the routine tables. */
    entry = *s;
    entry_io = *io;
    if (lanec_pin_chain_max(s, io, mission, (uint16_t)(AGC_TRANSFER_NONE - 1u), 1u, &steps) ==
            EXEC_OK &&
        steps == 1u)
        return LANEC_OK;
    *s = entry;
    *io = entry_io;
    return LANEC_OUT_OF_SCOPE;
}

const lanec_suite pin_noun_z = {"pin-noun-z", CASES, make, run};
