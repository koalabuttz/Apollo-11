/* SPDX-License-Identifier: MIT */
/* The readable noun table readers against the original program (make
 * pinball-noun-tables-cases, its second step; the first runs the faithful
 * port's own test, c64-port/tests/test_noun_reader.c, on them).
 *
 * The faithful port froze the original program's results, run in Virtual
 * AGC, for 304 of its cases (c64-port/tests/noun_reader_vectors.h: every
 * noun and scale factor of both missions): the 16 words a reader may
 * change. Each runs here under each of the four superbank settings with
 * bit 0100 (as the port's C64 diagnostic, platform/c64/
 * noun_reader_diagnostic.c, runs them): those 16 words must be the
 * original's, every other word as it was, and the latches untouched. */
#include <stdio.h>
#include <string.h>
#include "noun_reader_cases.h"
#include "noun_reader_vectors.h"

int main(void)
{
    static apollo_overlay_state state, before;
    apollo_noun_reader_io io, saved;
    unsigned row, form, i, changed, checked = 0, failures = 0;

    for (row = 0; row < NOUN_READER_TARGET_CASES; ++row)
        for (form = 0; form < 4u; ++form) {
            uint16_t n = noun_reader_target_ids[row];
            const char *why = NULL;

            noun_reader_prepare(&state, &io, n);
            io.superbank = (uint16_t)(0100u + form * 020u);
            before = state;
            saved = io;
            if (!apollo_noun_reader(&state, &io, noun_reader_mission(n), noun_reader_entry(n))) {
                why = "refused";
            } else {
                for (i = 0, changed = 0; i < APOLLO_SHARED_WORDS && !why; ++i) {
                    uint16_t expected = before.words[i];
                    if (changed < NOUN_READER_CHANGED_WORDS && noun_reader_changed[changed] == i)
                        expected = noun_reader_expected[row][changed++];
                    if (state.words[i] != expected) {
                        if (failures < 8u)
                            printf("  word %04o: %06o, original %06o\n", i, state.words[i],
                                   expected);
                        why = "a word differs";
                    }
                }
                if (!why && memcmp(&io, &saved, sizeof io)) why = "the latches changed";
            }
            if (why) {
                if (++failures <= 8u)
                    printf("FAIL case %u superbank %03o: %s\n", n, saved.superbank, why);
            } else {
                ++checked;
            }
        }
    printf("pinball-noun-tables-oracle: %u of %u runs match the original program, %u failures\n",
           checked, NOUN_READER_TARGET_CASES * 4u, failures);
    return failures != 0;
}
