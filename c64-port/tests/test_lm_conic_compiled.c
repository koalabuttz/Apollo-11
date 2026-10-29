/* SPDX-License-Identifier: MIT */
/* lm-conic-compiled host test: the frozen cases (every 128th of the
 * differential's, tests/lm_conic_compiled_vectors.h) run
 * through the compiled Luminary099 CONIC_SUBROUTINES
 * to their stops; each digest must equal the yaAGC oracle's. */
#include <stdio.h>
#include "interp_case.h"
#include "lm_conic_compiled_vectors.h"
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
        kind = irc_run(irc_program(cases[i].cm), &s, &cases[i], 5000, &ie);  /* (the spec's max_steps) */
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
    run("frozen", lm_conic_compiled_cases, lm_conic_compiled_expected, LM_CONIC_COMPILED_VECTORS);
    printf("lm-conic-compiled: %u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
