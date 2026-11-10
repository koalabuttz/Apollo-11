/* SPDX-License-Identifier: MIT */
/* The readable DAP_INTERFACE_SUBROUTINES against the faithful port's, key
 * by key (make dap-interface-cases), the LM's only.
 *
 * Built with the readable table (src/binding/dap_interface_routines.c)
 * and the faithful port's under the same name with faithful_ in front (the
 * Makefile renames it). First the keys, which must be the faithful
 * table's. Then the runs: each key from 4,096 states, each the port's case
 * image (c64-port/tests/lm_dap_interface_cases.c: any memory, FB bank 20,
 * any EBANK, a caller's return in Q) entered at the key, most stirred
 * further: DAPBOOLS (its DBSELECT and DRIFTBIT bits), A (now and then an
 * overflow), L (never overflowed), Q, RUPTREG1 and RUPTREG2 (a return in fixed memory, now
 * and then in erasable memory or past it), the CDUs, EBANK, the latches,
 * and now and then FB another bank. Each runs once on each side, as the
 * program's trampoline would call the key's routine. Where either port
 * runs, the other must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run in at least 1 of 16 of
 * each key's states. */
#include <stdio.h>
#include <string.h>
#include "lm_dap_interface_cases.h"

extern const agc_routine_table faithful_lm_dap_interface_routines;

#define CASES 4096u

static uint32_t seed = 1406u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A return word: mostly in fixed memory, now and then not. */
static uint16_t return_word(void)
{
    unsigned k = pick(8u);

    return k < 5u    ? (uint16_t)(02000u + pick(06000u))
           : k == 5u ? (uint16_t)pick(02000u)
           : k == 6u ? (uint16_t)(010000u + pick(070000u))
                     : (pick(2u) ? 01777u : 010000u);
}

static uint16_t sign_extended(uint16_t v)
{
    return (uint16_t)(v & 040000u ? v | 0100000u : v);
}

/* A: mostly a number sign-extended, now and then an overflow. (L is never
 * left overflowed between instructions, the readable machine corrects it
 * after each; the faithful routines leave it as it is.) */
static uint16_t register_word(uint16_t v)
{
    v = sign_extended(v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

static void state(apollo_overlay_state *s, p66_bias_io *io, uint16_t key, unsigned n)
{
    lm_dap_case c;
    uint16_t *w = s->words, eb;

    memset(io, 0, sizeof *io);
    lm_dap_input(s, io, &c, (uint16_t)(n % LM_DAP_CASES));
    w[5] = (uint16_t)(02000u + key % 1024u);
    if (n % 4u != 0u) {
        if (pick(2u)) w[LM_DAP_DAPBOOLS] = any_word();
        if (pick(2u)) w[LM_DAP_DAPBOOLS] ^= (uint16_t)(pick(2u) ? 010u : 0200u);
        w[0] = pick(2u) ? (uint16_t)0u : register_word(any_word());
        if (pick(2u)) w[1] = sign_extended(any_word());
        if (pick(4u) == 0u) w[2] = return_word();
        if (pick(2u)) w[LM_DAP_RUPTREG1] = return_word();
        if (pick(2u)) w[LM_DAP_RUPTREG2] = return_word();
        if (pick(4u) == 0u) w[032u + pick(3u)] = any_word();
        if (pick(4u) == 0u) {
            eb = (uint16_t)(pick(8u) << 8);
            w[3] = eb;
            w[6] = (uint16_t)((w[4] & 076000u) | (eb >> 8));
        }
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & 0160u);
        if (pick(16u) == 0u) io->interrupt_enabled = (uint16_t)pick(2u);
        if (pick(32u) == 0u) {
            w[4] = (uint16_t)(rnd() & 076000u);
            w[6] = (uint16_t)((w[4] & 076000u) | (w[3] >> 8));
        }
    }
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            p66_bias_io io, fio, rio;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
            fa = ra = start;
            fio = rio = io;
            memset(&p, 0, sizeof p);
            sf = faithful->entries[i].call(&p, &fa, &fio);
            sr = readable->entries[i].call(&p, &ra, &rio);
            ran += sf == EXEC_OK;
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
                printf("FAIL %02o,%04o case %u: %s (status %d, faithful %d)\n", key / 1024u,
                       02000u + key % 1024u, n, why, (int)sr, (int)sf);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine ran in %u of %u states\n", key / 1024u,
                   02000u + key % 1024u, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(void)
{
    unsigned i, failures = 0, runs = 0;
    const agc_routine_table *r = &lm_dap_interface_routines,
                            *f = &faithful_lm_dap_interface_routines;

    if (r->count != f->count) {
        printf("FAIL: %u keys, faithful %u\n", r->count, f->count);
        return 1;
    }
    for (i = 0; i < r->count; ++i)
        if (r->entries[i].physical != f->entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, r->entries[i].physical,
                   f->entries[i].physical);
            ++failures;
        }
    if (failures) return 1;
    failures = keys(r, f, &runs);
    printf("dap-interface-cases: %u keys from %u states each, %u faithful runs, %u failures\n",
           r->count, CASES, runs, failures);
    return failures != 0u;
}
