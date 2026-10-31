/* SPDX-License-Identifier: MIT */
/* The readable jet failure monitor against the faithful port's, case by
 * case (make rcs-monitor-cases), the LM's only.
 *
 * Built with the readable tables (src/binding/rcs_failure_monitor_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them). Each of the 2 entries runs on 65,536 random
 * states: random memory, mostly in bank 06; PVALVEST mostly a few of the
 * low 8 bits, as the monitor keeps it, now and then with one bit above
 * (which moves the bit count the loop starts from) or anything;
 * channel 32 mostly PVALVEST's valves (inverted) but for a bit or two, now
 * and then anything; CH5MASK and CH6MASK a few bits; NEWJOB mostly an
 * offset. Where either port runs, the other must too, with the same
 * memory and io record, word for word; where the readable one refuses, it
 * must leave everything as it found it. The faithful routines are
 * certified against the original program (c64-port/tests/test_lm_rcsmon.c),
 * as the readable ones are by make rcs-monitor-cases' oracle step. */
#include <stdio.h>
#include <string.h>
#include "lm_rcsmon.h"
#include "agc_input_channels.h"
extern const agc_routine_table faithful_lm_rcsmon_basic;

/* The erasable words the routine reads (Luminary099's addresses). */
#define NEWJOB_AT   0067u
#define CH5MASK_AT  01262u
#define CH6MASK_AT  01263u
#define PVALVEST_AT 01276u

#define CASES 65536u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)((w & 077777u) | ((w & 040000u) << 1));
}

/* A 16-bit register: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    return rnd() % 8u ? sign_extended(rnd()) : (uint16_t)(rnd() | rnd() << 1);
}

/* A few random bits of mask. */
static uint16_t few(uint16_t mask)
{
    return (uint16_t)(rnd() & rnd() & rnd() & mask);
}

/* Both ports loop for ever on a state the monitor never makes: PVALVEST
 * with bits above the low 8 whose changed bits (the low 8 of the word the
 * loop scans) leave nothing to shift into overflow. The changed word is
 * the valve bits that differ, with PVALVEST's high bits: positive, the
 * loop needs a low bit set; negative, a low bit clear (its magnitude's
 * set), unless the word is -0, which BZF takes. */
static int loops_for_ever(uint16_t pvalvest, uint16_t channel32)
{
    uint16_t differ = (uint16_t)((pvalvest ^ ~channel32) & 0377u), high = pvalvest & 077400u;

    if (pvalvest & 040000u) return differ == 0377u && high != 077400u;
    return differ == 0u && high != 0u;
}

static void state(apollo_overlay_state *s, lm_channel_io *io, uint16_t z)
{
    unsigned i;
    uint16_t eb = (uint16_t)(rnd() % 8u), x;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = reg();
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = rnd() % 16u ? 014000u : (uint16_t)((rnd() % 040u) << 10);
    s->words[5] = z;
    s->words[6] = (uint16_t)(s->words[4] | eb);
    if (rnd() % 64u == 0u) s->words[7] = 1u;
    if (rnd() % 64u == 0u) s->words[3] = (uint16_t)(rnd() & 077777u);
    x = (uint16_t)(rnd() % 8u);
    s->words[NEWJOB_AT] = x < 5u   ? (uint16_t)(rnd() % 8u * 12u)
                          : x < 7u ? 077777u
                                   : rnd() & 077777u;
    if (rnd() % 128u == 0u) s->words[010u + rnd() % (APOLLO_SHARED_WORDS - 010u)] = 0100000u;
    /* The monitor's words. */
    s->words[CH5MASK_AT] = few(0377u);
    s->words[CH6MASK_AT] = few(0377u);
    x = (uint16_t)(rnd() % 8u);
    s->words[PVALVEST_AT] = x < 6u   ? few(0377u)
                            : x < 7u ? (uint16_t)(few(0377u) | 0400u << rnd() % 7u)
                                     : (uint16_t)(rnd() & 077777u);
    io->io.superbank = (uint16_t)((rnd() % 8u) * 020u);
    io->io.interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 64u == 0u) io->io.interrupt_enabled = 2u;
    io->in.channel3 = (uint16_t)(rnd() & 077777u);
    io->in.channel4 = (uint16_t)(rnd() & 077777u);
    io->in.channel30 = (uint16_t)(rnd() & 077777u);
    io->in.channel31 = (uint16_t)(rnd() & 077777u);
    io->in.channel33 = (uint16_t)(rnd() & 077777u);
    io->in.channel16 = (uint16_t)(rnd() & 077777u);
    /* Channel 32 (inverted) mostly as PVALVEST last acted on it. */
    do {
        io->in.channel32 = rnd() % 4u ? (uint16_t)((~s->words[PVALVEST_AT] ^ few(0377u)) & 077777u)
                                      : (uint16_t)(rnd() & 077777u);
    } while (loops_for_ever(s->words[PVALVEST_AT], io->in.channel32));
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &lm_rcsmon_basic, *faithful = &faithful_lm_rcsmon_basic;

    if (readable->count != faithful->count) {
        printf("FAIL: %u entries, faithful %u\n", readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, closed = 0, opened = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL entry %u at key %u, faithful %u\n", i, readable->entries[i].physical, key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            lm_channel_io io, fio, rio;
            agc_hardware hw;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, z);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            memset(&hw, 0, sizeof hw);
            p.hardware = n % 2u ? &hw : 0;
            sf = f(&p, &fa, &fio);
            sr = r(&p, &ra, &rio);
            ran += sf == EXEC_OK;
            if (sf == EXEC_OK && fa.words[PVALVEST_AT] > start.words[PVALVEST_AT]) ++closed;
            if (sf == EXEC_OK && fa.words[PVALVEST_AT] < start.words[PVALVEST_AT]) ++opened;
            if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio)))
                why = "readable refusal changed the state";
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
                printf("FAIL %05o case %u: %s\n", z, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %05o: the faithful routine never ran\n", z);
            ++bad;
        }
        failures += bad;
        printf(
            "LM rcs_failure_monitor %05o: %u cases, %u run by the faithful routine (%u closing a "
            "valve, %u opening one), %u failures\n",
            z, CASES, ran, closed, opened, bad);
    }
    printf("rcs-monitor-cases LM: %u failures\n", failures);
    return failures != 0;
}
