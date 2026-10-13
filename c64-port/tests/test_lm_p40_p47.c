/* SPDX-License-Identifier: MIT */
/* lm-p40-p47 host test: the frozen cases (every 32nd of the
 * differential's, tests/lm_p40_p47_vectors.h) run
 * through the compiled Luminary099 P40-P47
 * to their stops; each digest must equal the yaAGC oracle's. */
#include <stdio.h>
#include "interp_case.h"
#include "lm_p40_p47_vectors.h"
const agc_native_program *irc_program(uint8_t cm);
static unsigned failures, checks;
static void run(const char *set, const irc_case *cases, const uint32_t *expected, unsigned n) {
    unsigned i;
    for (i = 0; i < n; ++i) {
        static apollo_overlay_state s;
        uint16_t ie = 0;
        unsigned kind;
        uint32_t d;
        irc_build(&s, &cases[i]);
        kind = irc_run(irc_program(cases[i].cm), &s, &cases[i], 200, &ie);
        d = irc_digest(kind, &s, ie);
        ++checks;
        if (kind != IRC_STOPPED || d != expected[i]) {
            ++failures;
            printf("%s case %u: kind %u digest %08lx, oracle %08lx\n", set, i, kind,
                   (unsigned long)d, (unsigned long)expected[i]);
        }
    }
}
int main(void) {
    run("frozen", lm_p40_p47_cases, lm_p40_p47_expected, LM_P40_P47_VECTORS);
    printf("lm-p40-p47: %u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
