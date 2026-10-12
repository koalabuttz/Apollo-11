/* SPDX-License-Identifier: MIT */
/* Lane C native runner: same cases as tools/lanec/oracle.c, run through the
 * native routines, printed in the oracle's format. */
#include <stdio.h>
#include <string.h>
#include "lanec_case.h"
#include "lanec_suites.h"
static apollo_overlay_state s;

int main(int argc, char **argv)
{
    const lanec_suite *suite = argc == 2 ? lanec_find_suite(argv[1]) : 0;
    lanec_io io;
    lanec_meta meta;
    enum lanec_status status;
    uint32_t n;
    unsigned i;
    if (!suite) { fprintf(stderr, "usage: native SUITE\n"); return 2; }
    for (n = 0; n < suite->count; ++n) {
        memset(&meta, 0, sizeof meta);
        suite->make(n, &s, &io, &meta);
        status = suite->run(&s, &io, meta.mission);
        printf("%lu", (unsigned long)n);
        for (i = 0; i < APOLLO_SHARED_WORDS; ++i) printf(" %u", (unsigned)s.words[i]);
        for (i = 0; i < LANEC_CHANNELS; ++i) printf(" %u", (unsigned)io.channel[i]);
        printf(" %u %u %u\n", (unsigned)io.superbank, (unsigned)io.interrupt_enabled, (unsigned)status);
    }
    return 0;
}
