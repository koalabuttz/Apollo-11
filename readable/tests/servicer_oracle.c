/* SPDX-License-Identifier: MIT */
/* The port's frozen SERVICER cases on the readable routines (make
 * servicer-cases ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_servicer.c checks the faithful routines against
 * digests of the original program (tests/lm_servicer_vectors.h: every
 * routine, 128 states each, and 16 continuations, run from its key until
 * control leaves the file, the 2048 erasable words, channels 11, 12, 32
 * and 33 and the interrupt state compared) and checks its own refusals; it
 * also counts the faithful file's paths, which the readable one has not.
 * This runs the same cases and refusals on the readable routines
 * (src/binding/servicer_routines.c), with the ropes loaded that the
 * readable code reads.
 *
 * Not counted: a case the readable routines refuse where a computed
 * transfer (a TC Q, SERVOUT's DXCH Z through AVGEXIT) took the run to a
 * word of the file no readable function starts at, which the faithful
 * port runs on from. The case states draw Q and AVGEXIT at random, so the
 * faithful port goes on from the middle of a routine there; the flight
 * program never does. The stop is recognised by its address, the case's
 * own Q or AVGEXIT: the binding records it (lm_servicer_unentered) only at
 * a word of the file no readable function starts at, which a readable
 * routine's own transfers never reach (servicer-cases checks each such run
 * again with the return made one a readable function starts at). */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_servicer_cases.h"
#include "lm_servicer_vectors.h"

extern uint16_t lm_servicer_unentered;

#define E7(a) (7u * 0400u + (a) - 01400u)

/* Whether the word the readable run stopped at (lm_servicer_unentered) is
 * where the case's own Q or AVGEXIT sent it. */
static int from_q_or_avgexit(const apollo_overlay_state *start)
{
    unsigned at = lm_servicer_unentered, address;

    if (!at) return 0;
    address = at >= 04000u && at < 010000u ? at : at % 1024u + 02000u;
    return address == (start->words[2] & 07777u) || address == (start->words[01252] & 07777u);
}

static unsigned checks, failures;
static void check(int ok, const char *what, unsigned n)
{
    ++checks;
    if (!ok && ++failures <= 20u) printf("FAIL %s %u\n", what, n);
}

static apollo_overlay_state undo;
static agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};

/* Case n's image with edit applied must be refused, image unchanged. */
static void refused(uint16_t n, void (*edit)(apollo_overlay_state *, lm_sv_io *), const char *what)
{
    apollo_overlay_state s, before;
    lmsv_case c;
    lm_sv_io io;
    agc_routine call;

    lmsv_input(&s, &c, n);
    io.control.superbank = c.superbank;
    io.control.interrupt_enabled = c.interrupt_enabled;
    io.chan11 = c.chan11;
    io.chan12 = c.chan12;
    io.chan32 = c.chan32;
    io.chan33 = c.chan33;
    io.undo = &undo;
    edit(&s, &io);
    before = s;
    call = agc_transfer_resolve(&program, &s, c.superbank);
    check(call && call(&program, &s, &io) == EXEC_OUT_OF_SCOPE && !memcmp(&s, &before, sizeof s),
          what, n);
}
#define EDIT(name, body)                                                                           \
    static void name(apollo_overlay_state *s, lm_sv_io *io)                                        \
    {                                                                                              \
        (void)s;                                                                                   \
        (void)io;                                                                                  \
        body;                                                                                      \
    }
EDIT(fb10, s->words[4] = 020000u; s->words[6] = (agc_word)(020000u | (s->words[3] >> 8)))
EDIT(vac_bad, s->words[0120] = 0402u)
EDIT(edop, s->words[023] = 1u)
EDIT(q_erasable, s->words[0] = 0; s->words[2] = 01000u)
EDIT(q_dependent, s->words[0] = 0; s->words[2] = 02333u)
EDIT(phase4_neg, s->words[E7(01515u)] = 0; s->words[0761] = 040000u)
EDIT(a_overflow, s->words[0] = 0100000u)
EDIT(avgexit_interpreter, s->words[01252] = 06040u)
EDIT(no_undo, io->undo = 0)

/* The first case of the routine at physical key k that the routine runs
 * to the file's exit unedited, so each refusal is the edit's. */
static uint16_t slot(uint16_t k)
{
    unsigned i, v;
    apollo_overlay_state t;

    for (i = 0; i < LMSV_ROUTINES; ++i)
        if (lmsv_routine_keys[i] == k) break;
    for (v = 0; v < LMSV_STATES; ++v)
        if (lmsv_run(&t, (uint16_t)(i + v * LMSV_ROUTINES), 0) == LMSV_STOPPED) break;
    check(i < LMSV_ROUTINES && v < LMSV_STATES, "refusal test case", k);
    return (uint16_t)(i + v * LMSV_ROUTINES);
}
#define K(b, a) (uint16_t)((b) * 1024u + ((a) - 02000u))

int main(int argc, char **argv)
{
    static agc_routine_table basic[1], interp[1];
    apollo_overlay_state s;
    unsigned n, stopped = 0, refusals = 0, unentered = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    basic[0] = lm_servicer_routines;
    interp[0] = lm_servicer_interpretive;
    program.basic = basic;
    program.basic_count = 1;
    program.interpretive = interp;
    program.interpretive_count = 1;
    program.interpreter_fetch = (uint16_t)(2048u + 06070u - 04000u);
    program.loc = 0164;
    program.bankset = 0165;
    for (n = 0; n < LMSV_CASES; ++n) {
        unsigned kind;
        apollo_overlay_state start;
        lmsv_case c;
        uint32_t digest = lmsv_digest(&s, (uint16_t)n);

        lmsv_input(&start, &c, (uint16_t)n);
        kind = lmsv_run(&s, (uint16_t)n, 0);
        if (digest != lmsv_expected[n] && kind == LMSV_REFUSED && from_q_or_avgexit(&start)) {
            ++unentered;
            continue;
        }
        check(digest == lmsv_expected[n], "digest", n);
        stopped += kind == LMSV_STOPPED;
        refusals += kind == LMSV_REFUSED;
        check(kind != 3u, "every case leaves the file or is refused", n);
    }
    refused(slot(K(023u, 02462u)), fb10, "NORMLIZE's first pair under FB 10");
    refused(slot(K(023u, 02462u)), vac_bad, "NORMLIZE's first pair with a work area off the VACs");
    refused(slot(K(033u, 02267u)), edop, "AVERAGEG with EDOP set");
    refused(slot(K(037u, 03531u)), q_erasable, "GNUFAZE5's TC Q to erasable");
    refused(slot(K(033u, 02440u)), q_dependent, "TMPTOSPT's TC Q onto an extracode's word");
    refused(slot(K(033u, 02353u)), phase4_neg, "LOTHRUST with PHASE4 negative");
    refused(slot(K(037u, 03570u)), a_overflow, "DODELVZ with an overflow in A");
    refused(slot(K(033u, 02411u)), avgexit_interpreter, "SERVOUT's DXCH Z onto the interpreter");
    refused(slot(K(033u, 02206u)), no_undo, "SERVICER without an undo image");
    printf("servicer-oracle: %u cases left the file, %u refusals, %u not counted (on from a word "
           "no readable function starts at); %u checks, %u failures\n",
           stopped, refusals, unentered, checks, failures);
    return failures != 0u;
}
