/* SPDX-License-Identifier: MIT */
/* The port's frozen BURN, BABY, BURN cases on the readable routines (make
 * burn-baby-cases ROPE_LM ROPE_CM).
 *
 * c64-port/tests/test_lm_burn_baby.c checks the faithful routines against
 * digests of the original program (tests/lm_burn_baby_vectors.h: every
 * routine, 128 states each, run from its key until control leaves the
 * file, the 2048 erasable words, channels 11, 14 and 30 and the interrupt
 * state compared) and checks its own refusals; it also counts the faithful
 * file's paths, which the readable one has not. This runs the same cases
 * and refusals on the readable routines (src/binding/burn_baby_routines.c),
 * with the ropes loaded that the readable code reads.
 *
 * Not counted: a case the readable routines refuse where a computed
 * transfer (a TC Q, DEAD's DTCB) took the run to a word of the file no
 * readable function starts at, which the faithful port runs on from. The
 * case states draw Q and ITEMP1 at random, so the original program goes on
 * from the middle of a routine there; the flight program never does. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_burn_baby_cases.h"
#include "lm_burn_baby_vectors.h"

extern uint16_t lm_burn_baby_unentered;

/* Whether the word the readable run stopped at (lm_burn_baby_unentered) is
 * where the case's own Q or ITEMP1 sent it: a TC Q, KILLTASK's return (Q
 * +1) or DEAD's DTCB (ITEMP1). */
static int from_q_or_itemp1(const apollo_overlay_state *start)
{
    unsigned at = lm_burn_baby_unentered, address;

    if (!at) return 0;
    address = at >= 04000u && at < 010000u ? at : at % 1024u + 02000u;
    return address == (start->words[2] & 07777u) || address == ((start->words[2] + 1u) & 07777u) ||
           address == (start->words[061] & 07777u);
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
static void refused(uint16_t n, void (*edit)(apollo_overlay_state *, lm_bb_io *), const char *what)
{
    apollo_overlay_state s, before;
    lmbb_case c;
    lm_bb_io io;
    agc_routine call;

    lmbb_input(&s, &c, n);
    io.control.superbank = c.superbank;
    io.control.interrupt_enabled = c.interrupt_enabled;
    io.chan11 = c.chan11;
    io.chan14 = c.chan14;
    io.chan30 = c.chan30;
    io.undo = &undo;
    call = agc_transfer_resolve(&program, &s, c.superbank);
    edit(&s, &io);
    before = s;
    check(call && call(&program, &s, &io) == EXEC_OUT_OF_SCOPE && !memcmp(&s, &before, sizeof s),
          what, n);
}
#define EDIT(name, body)                                                                           \
    static void name(apollo_overlay_state *s, lm_bb_io *io)                                        \
    {                                                                                              \
        (void)s;                                                                                   \
        (void)io;                                                                                  \
        body;                                                                                      \
    }
#define E7(a) (7u * 0400u + (a) - 01400u)
EDIT(fb10, s->words[4] = 020000u; s->words[6] = (agc_word)(020000u | (s->words[3] >> 8)))
EDIT(bankset10, s->words[0165] = 020000u)
EDIT(vac_bad, s->words[0120] = 0402u)
EDIT(edop, s->words[023] = 1u)
EDIT(which_bad, s->words[E7(01455u)] = 02023u)
EDIT(phase5_neg, s->words[0763] = 070000u)
EDIT(phase3_neg, s->words[0757] = 077777u)
EDIT(dispdex_m0, s->words[01163] = 077777u)
EDIT(dispdex_bad, s->words[01163] = 077777u - 5u)
EDIT(p40ret_bad, s->words[01142] = 03015u)
EDIT(q_erasable, s->words[2] = 01000u)
EDIT(q_not_site, s->words[2] = 02000u)
EDIT(a_overflow, s->words[0] = 0100000u)
EDIT(q_dependent, s->words[2] = 02327u; s->words[4] = 074000u;
     s->words[6] = (agc_word)(074000u | (s->words[3] >> 8)))
EDIT(no_undo, io->undo = 0)

/* The first case of the routine at physical key k. */
static uint16_t slot(uint16_t k)
{
    unsigned i;

    for (i = 0; i < LMBB_ROUTINES; ++i)
        if (lmbb_routine_keys[i] == k) return (uint16_t)i;
    return 0;
}
#define K36(a) (uint16_t)(036u * 1024u + ((a) - 02000u))
#define K27(a) (uint16_t)(027u * 1024u + ((a) - 02000u))

int main(int argc, char **argv)
{
    static agc_routine_table basic[1], interp[1];
    apollo_overlay_state s;
    unsigned n, stopped = 0, refusals = 0, unentered = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    basic[0] = lm_burn_baby_routines;
    interp[0] = lm_burn_baby_interpretive;
    program.basic = basic;
    program.basic_count = 1;
    program.interpretive = interp;
    program.interpretive_count = 1;
    program.interpreter_fetch = (uint16_t)(2048u + 06070u - 04000u);
    program.loc = 0164;
    program.bankset = 0165;
    for (n = 0; n < LMBB_CASES; ++n) {
        unsigned kind;
        apollo_overlay_state start;
        lmbb_case c;
        uint32_t digest = lmbb_digest(&s, (uint16_t)n);

        lmbb_input(&start, &c, (uint16_t)n);
        kind = lmbb_run(&s, (uint16_t)n, 0);
        if (digest != lmbb_expected[n] && kind == LMBB_REFUSED && from_q_or_itemp1(&start)) {
            ++unentered;
            continue;
        }
        check(digest == lmbb_expected[n], "digest", n);
        stopped += kind == LMBB_STOPPED;
        refusals += kind == LMBB_REFUSED;
        check(kind != 3u, "every case leaves the file or is refused", n);
    }
    refused(slot(K36(02126u)), fb10, "BURNBABY under FB 10");
    refused(slot(K36(02157u)), bankset10, "36,2157 with BANKSET bank 10");
    refused(slot(K36(02163u)), vac_bad, "36,2163 with a work area off the VAC areas");
    refused(slot(K36(02175u)), edop, "36,2175 with EDOP set");
    refused(slot(K36(02143u)), which_bad, "B*RNB*B* with WHICH not a table");
    refused(slot(K36(02342u)), phase5_neg, "REDO4.2 with PHASE5 negative");
    refused(slot(K36(02366u)), phase3_neg, "P40SJUNK with PHASE3 -0");
    refused(slot(K36(02737u)), dispdex_m0, "CLOKJOB with DISPDEX -0");
    refused(slot(K36(02737u)), dispdex_bad, "CLOKJOB with DISPDEX -5");
    refused(slot(K36(03025u)), p40ret_bad, "NULLCLOK's TC P40/RET with an unknown word");
    refused(slot(K36(02653u)), q_erasable, "NOULLAGE's TC Q to erasable");
    refused(slot(06027u), q_not_site, "KILLTASK not from a TC KILLTASK");
    refused(slot(K27(02200u)), a_overflow, "KILLTSK2 with overflow in A");
    refused(slot(K36(02657u)), q_dependent, "ONULLAGE's TC Q onto an indexed word");
    refused(slot(K36(02126u)), no_undo, "BURNBABY without an undo image");
    printf("burn-baby-oracle: %u cases left the file, %u refusals, %u not counted (on from a "
           "word no readable function starts at); %u checks, %u failures\n",
           stopped, refusals, unentered, checks, failures);
    return failures != 0u;
}
