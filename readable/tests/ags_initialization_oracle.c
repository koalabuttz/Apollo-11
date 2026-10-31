/* SPDX-License-Identifier: MIT */
/* The port's frozen R47 cases on the readable routines (make ags-cases
 * ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_ags_init.c checks the faithful routines against
 * digests of the original program (tests/lm_ags_init_vectors.h: every
 * routine, 768 states each, run from its key until control leaves the
 * file, the 2048 erasable words and the interrupt state compared) and
 * checks its own refusals; it also counts the faithful file's paths, which
 * the readable one has not. This runs the same cases and refusals on the
 * readable routines (src/binding/ags_initialization_routines.c), with the
 * ropes loaded that the readable interpreter reads. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_ags_init_cases.h"
#include "lm_ags_init_vectors.h"

static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures <= 20u) printf("FAIL %s %u\n", what, n);
}

static apollo_overlay_state undo;
static agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};

/* Case n's image with edit applied must be refused, image unchanged. */
static void refused(uint16_t n, void (*edit)(apollo_overlay_state *, lm_ags_io *), const char *what)
{
    apollo_overlay_state s, before;
    ags_case c;
    lm_ags_io io;
    agc_routine call;

    ags_input(&s, &c, n);
    io.control.superbank = c.superbank;
    io.control.interrupt_enabled = c.interrupt_enabled;
    io.undo = &undo;
    call = agc_transfer_resolve(&program, &s, c.superbank);
    edit(&s, &io);
    before = s;
    check(call && call(&program, &s, &io) == EXEC_OUT_OF_SCOPE && !memcmp(&s, &before, sizeof s),
          what, n);
}
static void fb10(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[4] = 020000u;
    s->words[6] = (agc_word)(020000u | (s->words[3] >> 8));
}
static void bankset10(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[0165] = 020000u;
}
static void vac_bad(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[0120] = 0402u;
}
static void pushloc_bad(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[0166] = (agc_word)(s->words[0120] + 043u);
}
static void edop(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[023] = 1u;
}
static void newjob_neg(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[067] = 040000u;
}
static void qpret_erasable(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)io;
    s->words[s->words[0120] + 052u] = 01234u;
}
static void no_undo(apollo_overlay_state *s, lm_ags_io *io)
{
    (void)s;
    io->undo = 0;
}

int main(int argc, char **argv)
{
    static agc_routine_table basic[1], interp[1];
    apollo_overlay_state s;
    unsigned n, stopped = 0, refusals = 0;
    uint16_t ie;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    basic[0] = lm_ags_init_routines;
    interp[0] = lm_ags_init_interpretive;
    program.basic = basic;
    program.basic_count = 1;
    program.interpretive = interp;
    program.interpretive_count = 1;
    program.interpreter_fetch = (uint16_t)(2048u + 06070u - 04000u);
    program.loc = 0164;
    program.bankset = 0165;
    for (n = 0; n < AGS_CASES; ++n) {
        unsigned kind;
        check(ags_digest(&s, (uint16_t)n) == ags_expected[n], "digest", n);
        kind = ags_run(&s, (uint16_t)n, &ie);
        stopped += kind == AGS_STOPPED;
        refusals += kind == AGS_REFUSED;
        check(kind != 3u, "every case leaves the file or is refused", n);
    }
    refused(0, fb10, "AGSINIT under FB 10");
    refused(AGS_BASIC_ROUTINES + 3u, bankset10, "32,2051 with BANKSET bank 10");
    refused(AGS_BASIC_ROUTINES + 4u, vac_bad, "32,2053 with a work area off the VAC areas");
    refused(AGS_BASIC_ROUTINES + 5u, pushloc_bad, "32,2055 with PUSHLOC outside the work area");
    refused(AGS_BASIC_ROUTINES + 7u, edop, "32,2061 with EDOP set");
    refused(AGS_BASIC_ROUTINES + 8u, newjob_neg, "32,2063 with NEWJOB negative");
    refused(AGS_BASIC_ROUTINES + 20u, qpret_erasable, "32,2156 with QPRET in erasable");
    refused(5, no_undo, "32,2046 without an undo image");
    printf("ags-oracle: %u cases left the file, %u refusals; %u checks, %u failures\n", stopped,
           refusals, checks, failures);
    return failures != 0u;
}
