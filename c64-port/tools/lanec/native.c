/* SPDX-License-Identifier: MIT */
/* Lane C native runner: same cases as tools/lanec/oracle.c, run through the
 * native routines, printed in the oracle's format. With --entries it prints
 * instead each case's entry point ("LM|CM bank z n", bank 0 for fixed-fixed)
 * when the native routine accepted the case: the entry set a suite verifies,
 * from which tools/lanec/routines.py builds the routine tables. An entry at
 * the interpreter (Z at NEWOPS, or at INTPRET with Q the first interpretive
 * word, which INTPRET makes LOC) is printed "LMI|CMI bank loc n": the
 * interpretive point keyed by LOC under BANKSET. --exits adds to each entry
 * line where the routine stopped, "fb z q", and the Q it started with (fb the
 * bank then selected). "native SUITE CASE" runs that one case (for
 * branch-coverage traces, tools/lanec/cover.sh).
 * A refused case (LANEC_OUT_OF_SCOPE) must leave the image and I/O exactly as
 * they were; one that does not is printed with status 2, which no oracle row
 * matches. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lanec_case.h"
#include "lanec_suites.h"
#include "dsky_fixed_display.h"
static apollo_overlay_state s, before;

/* The rope for modules that read it as data (lanec_fixed_read): the
 * missions' fixed-memory images the earlier display modules use. */
static bool fixed(enum lanec_mission mission, uint16_t physical, agc_word *word)
{
    return dsky_fixed_display_data(mission == LANEC_LM ? DSKY_NV_LM : DSKY_NV_CM, physical, word, 0);
}

int main(int argc, char **argv)
{
    int exits = argc == 3 && !strcmp(argv[1], "--exits");
    int entries = (argc == 3 && !strcmp(argv[1], "--entries")) || exits;
    const lanec_suite *suite = argc == 2 + entries || (!entries && argc == 3) ? lanec_find_suite(argv[1 + entries]) : 0;
    uint32_t only = !entries && argc == 3 ? (uint32_t)strtoul(argv[2], 0, 10) : UINT32_MAX;   /* SUITE CASE: that case alone */
    lanec_io io, io_before;
    lanec_meta meta;
    enum lanec_status status;
    uint32_t n;
    unsigned i, bank, z, interp;
    lanec_fixed_read = fixed;
    if (!suite) { fprintf(stderr, "usage: native [--entries|--exits] SUITE | native SUITE CASE\n"); return 2; }
    for (n = only == UINT32_MAX ? 0 : only; n < suite->count && n <= only; ++n) {
        memset(&meta, 0, sizeof meta);
        suite->make(n, &s, &io, &meta);
        z = s.words[LANEC_Z] & 07777u;
        bank = z >= 02000u && z < 04000u ? lanec_current_bank(&s, &io) : 0u;
        /* NEWOPS and INTPRET: LM 06070/06037, CM 06037/06006 */
        interp = meta.mission == LANEC_CM ? (z == 06037u ? 0164u : z == 06006u ? LANEC_Q : 0u)
                                          : (z == 06070u ? 0164u : z == 06037u ? LANEC_Q : 0u);
        if (interp) { z = s.words[interp] & 07777u; bank = lanec_current_bank(&s, &io); }
        before = s; io_before = io;
        status = suite->run(&s, &io, meta.mission);
        if (status != LANEC_OK && (memcmp(&s, &before, sizeof s) || memcmp(&io, &io_before, sizeof io)))
            status = (enum lanec_status)2;
        if (entries) {
            if (status == LANEC_OK && !exits)
                printf("%s%s %o %o %lu\n", meta.mission == LANEC_CM ? "CM" : "LM", interp ? "I" : "", bank, z, (unsigned long)n);
            else if (status == LANEC_OK) {
                unsigned x = s.words[LANEC_Z] & 07777u;
                printf("%s%s %o %o %lu %o %o %o %o\n", meta.mission == LANEC_CM ? "CM" : "LM", interp ? "I" : "", bank, z,
                       (unsigned long)n, lanec_current_bank(&s, &io), x, s.words[LANEC_Q] & 07777u,
                       before.words[LANEC_Q] & 07777u);
            }
            continue;
        }
        printf("%lu", (unsigned long)n);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) printf(" %u", (unsigned)s.words[i]);
        for (i = 0; i < LANEC_CHANNELS; ++i) printf(" %u", (unsigned)io.channel[i]);
        printf(" %u %u %u\n", (unsigned)io.superbank, (unsigned)io.interrupt_enabled, (unsigned)status);
    }
    return 0;
}
