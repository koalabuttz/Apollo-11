/* SPDX-License-Identifier: MIT */
/* The port's frozen THE_LUNAR_LANDING cases on the readable routines (make
 * lunar-landing-cases ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_lunar_landing.c checks the faithful routines
 * against digests of the original program
 * (tests/lm_lunar_landing_vectors.h: every routine, 512 states each, run
 * from its key until control leaves the file, the 2048 erasable words,
 * channels 12 and 33 and the interrupt state compared) and checks its own
 * refusals; it also counts the faithful file's paths, which the readable
 * one has not. This runs the same cases and refusals on the readable
 * routines (src/binding/lunar_landing_routines.c), with the ropes loaded
 * that the readable code reads. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_lunar_landing_cases.h"
#include "lm_lunar_landing_vectors.h"

static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures <= 20u) printf("FAIL %s %u\n", what, n);
}

static apollo_overlay_state undo;
static agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};

/* Case n's image with edit applied must be refused, image unchanged. */
static void refused(uint16_t n, void (*edit)(apollo_overlay_state *, lm_lld_io *), const char *what)
{
    apollo_overlay_state s, before;
    lld_case c;
    lm_lld_io io;
    agc_routine call;

    lld_input(&s, &c, n);
    io.control.superbank = c.superbank;
    io.control.interrupt_enabled = c.interrupt_enabled;
    io.chan12 = c.chan12;
    io.chan33 = c.chan33;
    io.undo = &undo;
    call = agc_transfer_resolve(&program, &s, c.superbank);
    edit(&s, &io);
    before = s;
    check(call && call(&program, &s, &io) == EXEC_OUT_OF_SCOPE && !memcmp(&s, &before, sizeof s),
          what, n);
}
#define EDIT(name, body)                                                                           \
    static void name(apollo_overlay_state *s, lm_lld_io *io)                                       \
    {                                                                                              \
        (void)s;                                                                                   \
        (void)io;                                                                                  \
        body;                                                                                      \
    }
EDIT(fb10, s->words[4] = 020000u; s->words[6] = (agc_word)(020000u | (s->words[3] >> 8)))
EDIT(bankset10, s->words[0165] = 020000u)
EDIT(vac_bad, s->words[0120] = 0402u)
EDIT(pushloc_bad, s->words[0166] = (agc_word)(s->words[0120] + 043u))
EDIT(edop, s->words[023] = 1u)
EDIT(newjob_neg, s->words[067] = 040000u)
EDIT(a_overflow, s->words[0] = 040000u)
EDIT(no_undo, io->undo = 0)

/* The routine index of a key (bank 31 or 32, address a). */
static uint16_t slot(unsigned bank, unsigned a)
{
    unsigned i;

    for (i = 0; i < LLD_ROUTINES; ++i)
        if (lld_routine_keys[i] == bank * 1024u + a - 02000u) return (uint16_t)i;
    return 0;
}

int main(int argc, char **argv)
{
    static agc_routine_table basic[1], interp[1];
    apollo_overlay_state s;
    unsigned n, stopped = 0, refusals = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    basic[0] = lm_lunar_landing_routines;
    interp[0] = lm_lunar_landing_interpretive;
    program.basic = basic;
    program.basic_count = 1;
    program.interpretive = interp;
    program.interpretive_count = 1;
    program.interpreter_fetch = (uint16_t)(2048u + 06070u - 04000u);
    program.loc = 0164;
    program.bankset = 0165;
    for (n = 0; n < LLD_CASES; ++n) {
        unsigned kind;

        check(lld_digest(&s, (uint16_t)n) == lld_expected[n], "digest", n);
        kind = lld_run(&s, (uint16_t)n, 0);
        stopped += kind == LLD_STOPPED;
        refusals += kind == LLD_REFUSED;
        check(kind != 3u, "every case leaves the file or is refused", n);
    }
    refused(slot(032u, 02776u), fb10, "P63LM under FB 10");
    refused(slot(031u, 02144u), fb10, "LANDJUNK under FB 10");
    refused(slot(032u, 03040u), bankset10, "32,3040 with BANKSET bank 10");
    refused(slot(032u, 03044u), vac_bad, "32,3044 with a work area off the VAC areas");
    refused(slot(032u, 03051u), pushloc_bad, "32,3051 with PUSHLOC outside the work area");
    refused(slot(031u, 02167u), edop, "31,2167 with EDOP set");
    refused(slot(032u, 03044u), newjob_neg, "32,3044 with NEWJOB negative");
    refused(slot(032u, 03102u), a_overflow, "DDUMCALC with overflow in A");
    refused(slot(032u, 03002u), no_undo, "32,3002 without an undo image");
    printf("lunar-landing-oracle: %u cases left the file, %u refusals; %u checks, %u failures\n",
           stopped, refusals, checks, failures);
    return failures != 0u;
}
