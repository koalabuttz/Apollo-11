/* SPDX-License-Identifier: MIT */
/* The readable AOTMARK against the faithful port's, key by key (make
 * aotmark-cases ROPE_LM ROPE_CM), the LM's only.
 *
 * Built with the readable tables (src/binding/aotmark_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (OPTAXIS, YZCHK,
 * AVESTAR with AVEIT, OANB and SURFSTAR, src/aotmark/aotmark.c), written in
 * the listing's notation, must be the original program's rope, word for
 * word. Then the keys, which must be the faithful tables'. Then the runs:
 * each key from 4,096 states, each one of the port's frozen test states
 * (c64-port/tests/lm_aotmark_vectors.h: memory, latches and input
 * channels) entered at the key, most stirred further: the sighting's
 * words as AOTMARK keeps them (MARKSTAT a VAC area with its flag bits,
 * MARKCNTR, MKDEX, XYMARK, a detent code, SURFFLAG), the mark buttons of
 * channel 16, the VAC areas, EXTVBACT, NEWJOB, the work area, now and then
 * anything. The index registers X1 and X2 are always as the program sets
 * them, the complement of a place in the VAC areas: one that takes an
 * indexed operand into fixed memory, which the AGC (and the readable
 * interpreter) reads from the rope, the faithful port reads from erasable
 * memory. Each runs once on each side, as the program's trampoline
 * would call the key's routine. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run from some state of every key.
 *
 * Not counted: a detent code above 0366 at GETDAT +7, where the faithful
 * port's CA AOTEL -1 / CA AOTAZ -1 wraps around into erasable memory and
 * the readable code, as on the AGC (fixed memory), does not model it. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_aotmark_vectors.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code OPTAXIS_CODE, YZCHK_CODE, AVESTAR_CODE, OANB_CODE,
    SURFSTAR_CODE;
extern const agc_routine_table lm_aotmark_basic, lm_aotmark_interpretive;
extern const agc_routine_table faithful_lm_aotmark_basic, faithful_lm_aotmark_interpretive;

/* Luminary099's addresses of the words AOTMARK keeps. */
#define NEWJOB_AT   0067u
#define FLAGWRD8_AT 0104u
#define INTBIT15_AT 0115u
#define FIXLOC_AT   0120u
#define LOC_AT      0164u
#define BANKSET_AT  0165u
#define PUSHLOC_AT  0166u
#define VAC1USE_AT  0400u
#define AOTCODE_AT  0735u
#define EXTVBACT_AT 01044u
#define MARKSTAT_AT 01312u
#define MARKCNTR_AT 01550u
#define XYMARK_AT   01551u
#define MKDEX_AT    01552u
#define NEWOPS_AT   06070u
/* GETDAT +7, where the detent code is read. */
#define GETDAT_PLUS7 02072u

#define CASES 4096u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned words_differ(const struct interpretive_code *code)
{
    unsigned i, failures = 0;

    for (i = 0; i < code->count; ++i) {
        agc_word rope = 0;
        unsigned physical = code->bank * 1024u + code->first - 02000u + i;
        if (!apollo_rope_read(0, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++failures;
        }
    }
    return failures;
}

static unsigned tables_differ(const agc_routine_table *readable, const agc_routine_table *faithful,
                              const char *what)
{
    unsigned i, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL: %s has %u keys, faithful %u\n", what, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < readable->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: %s key %u is %u, faithful %u\n", what, i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    return failures;
}

/* One state at the key (its physical address; an order pair's for an
 * interpretive key): frozen case n's, and for most n more. */
static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t key, int interpretive,
                  unsigned n)
{
    const irc_case *c = &lm_aotmark_cases[(n * 7u + rnd()) % LM_AOTMARK_VECTORS];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    eb = (uint16_t)(s->words[3] >> 8);
    s->words[4] = (uint16_t)(bank << 10);
    if (interpretive) {
        s->words[5] = NEWOPS_AT;
        s->words[LOC_AT] = z;
        s->words[INTBIT15_AT] = 0u;
    } else {
        s->words[5] = z;
    }
    if (n % 4u != 0u) {
        uint16_t area = (uint16_t)(rnd() % 5u), x;

        s->words[FIXLOC_AT] = (uint16_t)(0401u + 054u * area);
        x = (uint16_t)(rnd() % 8u);
        s->words[MARKSTAT_AT] =
            x == 0u  ? 0u
            : x < 7u ? (uint16_t)((0400u + 054u * (rnd() % 5u)) | (rnd() & rnd() & 037000u))
                     : (uint16_t)(rnd() & 077777u);
        s->words[MARKCNTR_AT] = rnd() % 8u ? (uint16_t)(rnd() % 6u) : (uint16_t)(rnd() & 077777u);
        s->words[MKDEX_AT] = rnd() % 8u ? (uint16_t)(rnd() % 4u) : (uint16_t)(rnd() & 077777u);
        s->words[XYMARK_AT] = rnd() % 2u   ? (uint16_t)(rnd() % 8u)
                              : rnd() % 2u ? (uint16_t)(rnd() & 01400u)
                                           : (uint16_t)(rnd() & 0777u);
        s->words[AOTCODE_AT] = rnd() % 8u ? (uint16_t)((rnd() % 9u) << 6 | (rnd() & 077u))
                                          : (uint16_t)(rnd() & 077777u);
        s->words[FLAGWRD8_AT] = (uint16_t)((s->words[FLAGWRD8_AT] & ~0200u) | (rnd() % 2u) << 7);
        s->words[EXTVBACT_AT] = rnd() % 4u ? 0u : (uint16_t)(rnd() & 077777u);
        for (x = 0; x < 5u; ++x)
            s->words[VAC1USE_AT + 054u * x] =
                rnd() % 2u ? 0u : (uint16_t)(VAC1USE_AT + 054u * x + 1u);
        x = (uint16_t)(rnd() % 8u);
        s->words[NEWJOB_AT] = x < 5u   ? (uint16_t)(rnd() % 8u * 12u)
                              : x < 6u ? 077777u
                                       : (uint16_t)(rnd() & 077777u);
        io->in.channel16 =
            rnd() % 4u ? (uint16_t)(1u << (2u + rnd() % 5u)) : (uint16_t)(rnd() & 077777u);
        if (rnd() % 16u == 0u) io->io.superbank = (agc_word)(rnd() & 0160u);
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 32u == 0u) s->words[INTBIT15_AT] = (uint16_t)(rnd() & 077777u);
    }
    /* The push-down list in the work area, below X1. */
    if (s->words[PUSHLOC_AT] < s->words[FIXLOC_AT] ||
        s->words[PUSHLOC_AT] > s->words[FIXLOC_AT] + 037u || n % 4u != 0u)
        s->words[PUSHLOC_AT] = (uint16_t)(s->words[FIXLOC_AT] + rnd() % 040u);
    /* X1 and X2 (the work area's 046 and 047) as the program sets them:
     * the complement of a place in the VAC areas, which an indexed operand
     * adds. */
    s->words[(s->words[FIXLOC_AT] + 046u) & 03777u] = (uint16_t)(077777u - 0400u - rnd() % 0340u);
    s->words[(s->words[FIXLOC_AT] + 047u) & 03777u] = (uint16_t)(077777u - 0400u - rnd() % 0340u);
    s->words[6] = (uint16_t)(s->words[4] | eb);
    s->words[BANKSET_AT] = rnd() % 32u ? s->words[6] : (uint16_t)(rnd() & 077777u);
}

/* A detent code the faithful port's AOT tables wrap around at. */
static int detent_wraps(const apollo_overlay_state *s, uint16_t z)
{
    return z == GETDAT_PLUS7 && ((s->words[AOTCODE_AT] & 077700u) >> 6) > 0366u;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     int interpretive, unsigned *skipped)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_channel_io io, fio, rio;
            agc_hardware hw;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, interpretive, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            memset(&hw, 0, sizeof hw);
            p.hardware = n % 2u ? &hw : 0;
            sf = faithful->entries[i].call(&p, &fa, &fio);
            sr = readable->entries[i].call(&p, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK && !interpretive && detent_wraps(&start, z))
                ++*skipped;
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %02o,%04o case %u: %s\n", key / 1024u, z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine never ran\n", key / 1024u, z);
            ++bad;
        }
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures, skipped = 0;
    const struct interpretive_code *codes[] = {&OPTAXIS_CODE, &YZCHK_CODE, &AVESTAR_CODE,
                                               &OANB_CODE, &SURFSTAR_CODE};
    unsigned i, words = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = 0;
    for (i = 0; i < sizeof codes / sizeof codes[0]; ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_aotmark_basic, &faithful_lm_aotmark_basic, "basic");
    failures +=
        tables_differ(&lm_aotmark_interpretive, &faithful_lm_aotmark_interpretive, "interpretive");
    if (failures) {
        printf("aotmark-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&lm_aotmark_basic, &faithful_lm_aotmark_basic, 0, &skipped);
    failures += keys(&lm_aotmark_interpretive, &faithful_lm_aotmark_interpretive, 1, &skipped);
    printf("aotmark-cases: %u words as the rope's, %u keys (%u basic, %u order pairs) from %u "
           "states each, %u not counted (a detent code past the AOT tables), %u failures\n",
           words, lm_aotmark_basic.count + lm_aotmark_interpretive.count, lm_aotmark_basic.count,
           lm_aotmark_interpretive.count, CASES, skipped, failures);
    return failures != 0;
}
