/* SPDX-License-Identifier: MIT */
/* The readable SXTMARK against the faithful port's, key by key (make
 * sxtmark-cases), the CM's only.
 *
 * Built with the readable tables (src/binding/sxtmark_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them). First the keys, which must be the faithful
 * table's. Then each key runs from 4,096 states, each one of the port's
 * frozen test states (c64-port/tests/cm_sxtmark_vectors.h: memory and
 * latches) entered at the key, most stirred further: the marking's words
 * as SXTMARK keeps them (MARKSTAT a VAC area with marks wanted and bit
 * 10, the VAC areas' use words and mark slots, EXTVBACT, the R21 and V59
 * flags, CDUCHKWD), the marks wanted in A, the CDUs at the mark near the
 * CDUs now and DIFCHK's index, Q (where DIFCHK returns, and what MARKRUPT
 * keeps), channel 16's MARK and MARK REJECT bits and keys, now and then
 * anything. Each runs once on each side, as the program's trampoline
 * would call the key's routine. Where either port runs, the other must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it; and the
 * faithful routine must run from some state of every key. */
#include <stdio.h>
#include <string.h>
#include "interp_case.h"
#include "cm_sxtmark_vectors.h"

extern const agc_routine_table cm_sxtmark_basic;
extern const agc_routine_table faithful_cm_sxtmark_basic;

/* Comanche055's addresses of the words SXTMARK keeps. */
#define CDUX_AT     0032u
#define QPRET_AT    0052u
#define ITEMP1_AT   0061u
#define RUPTREG1_AT 0070u
#define STATE_AT    0074u
#define MKNDX_AT    0354u
#define MKCDUY_AT   0357u
#define VAC1USE_AT  0400u
#define EXTVBACT_AT 01044u
#define OPTIND_AT   01303u
#define MARKSTAT_AT 01330u
#define OPTMODES_AT 01331u
#define CDUCHKWD_AT 01341u
/* MARKDIF, MKACPT and DIFCHK in bank 10: where DIFCHK may return. */
#define MARKDIF_AT 02203u
#define MKACPT_AT  02216u
#define DIFCHK_AT  02220u
/* MKVACFND, entered with a free VACnUSE's word less one in A. */
#define MKVACFND_AT 02031u

#define CASES 4096u

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

/* A small signed word: +0 to 5, -0 to -4, now and then anything. */
static uint16_t small(void)
{
    uint16_t x = (uint16_t)(rnd() % 12u);
    if (x < 6u) return x;
    if (x < 11u) return (uint16_t)(077777u - (x - 6u));
    return (uint16_t)(rnd() & 077777u);
}

/* A VAC area's address: the word after its VACnUSE. */
static uint16_t area(void)
{
    return (uint16_t)(VAC1USE_AT + 1u + 054u * (rnd() % 5u));
}

/* Where DIFCHK may return to: the words after its TCs, its other labels,
 * 3 (RELINT), erasable or fixed addresses, anything. */
static uint16_t return_address(void)
{
    static const uint16_t to[] = {MARKDIF_AT + 7u, MARKDIF_AT + 011u, MKACPT_AT, MARKDIF_AT,
                                  DIFCHK_AT,       DIFCHK_AT + 016u,  3u,        3u};
    uint16_t x = (uint16_t)(rnd() % 12u);
    if (x < 8u) return to[x];
    if (x < 9u) return (uint16_t)(rnd() % 02000u);
    if (x < 10u) return (uint16_t)(02000u + rnd() % 06000u);
    if (x < 11u) return sign_extended(rnd());
    return (uint16_t)(rnd() | rnd() << 1);
}

/* One state at the key: frozen case n's, and for most n more. */
static void state(apollo_overlay_state *s, cm_channel_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_sxtmark_cases[(n * 7u + rnd()) % CM_SXTMARK_VECTORS];
    uint16_t bank = (uint16_t)(key / 1024u), z = (uint16_t)(02000u + key % 1024u), eb;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    irc_inputs(c, &io->in);
    io->io.superbank = c->superbank;
    io->io.interrupt_enabled = c->interrupt_enabled;
    s->words[4] = (uint16_t)(bank << 10);
    s->words[5] = z;
    if (n % 4u != 0u) {
        uint16_t x, a = area();

        eb = (uint16_t)(rnd() % 8u);
        s->words[3] = (uint16_t)(eb << 8);
        x = (uint16_t)(rnd() % 10u);
        s->words[MARKSTAT_AT] = x == 0u   ? 0u
                                : x == 1u ? (uint16_t)(rnd() % 2u ? 077777u : 040000u | rnd())
                                : x < 9u  ? (uint16_t)(a | (rnd() % 8u) << 11 | (rnd() % 2u) << 9)
                                          : (uint16_t)(rnd() & 077777u);
        for (x = 0; x < 5u; ++x) {
            uint16_t use = (uint16_t)(VAC1USE_AT + 054u * x), y = (uint16_t)(rnd() % 16u);
            s->words[use] = rnd() % 2u ? 0u : rnd() % 4u ? use : small();
            /* QPRET +area, the area's next slot (now and then one that
             * overflows when MARK2 adds seven, or ENTANSWR takes the area's
             * address off). */
            s->words[QPRET_AT + use + 1u] = y < 13u    ? (uint16_t)(use + 1u + 7u * (rnd() % 6u))
                                            : y == 13u ? (uint16_t)(037770u + rnd() % 8u)
                                            : y == 14u ? (uint16_t)(040000u + rnd() % 01000u)
                                                       : (uint16_t)(rnd() & 077777u);
        }
        s->words[EXTVBACT_AT] = rnd() % 2u ? 0u : (uint16_t)(rnd() & rnd() & 077777u);
        s->words[STATE_AT + 2u] = (uint16_t)(rnd() & 077777u);
        s->words[STATE_AT + 5u] = (uint16_t)(rnd() & 077777u);
        s->words[CDUCHKWD_AT] = small();
        s->words[RUPTREG1_AT] = small();
        s->words[ITEMP1_AT] = rnd() % 2u ? a : (uint16_t)(rnd() & 077777u);
        s->words[OPTIND_AT] = small();
        s->words[OPTMODES_AT] = (uint16_t)(rnd() & 077777u);
        /* DIFCHK: its index, and the CDUs at the mark a few bits from the
         * CDUs now. */
        s->words[MKNDX_AT] = small();
        for (x = 0; x < 3u; ++x) {
            uint16_t now = s->words[CDUX_AT + x];
            uint16_t mark = (uint16_t)((now + rnd() % 9u - 4u) & 077777u);
            /* MKCDUY, MKCDUZ, MKCDUX: 0357, 0361, 0363. */
            uint16_t at =
                x == 0u ? (uint16_t)(MKCDUY_AT + 4u) : (uint16_t)(MKCDUY_AT + 2u * (x - 1u));
            s->words[at] = rnd() % 8u ? mark : (uint16_t)(rnd() & 077777u);
        }
        /* A: the marks wanted, a sum that overflowed, or anything. */
        x = (uint16_t)(rnd() % 8u);
        s->words[0] = x < 5u ? (uint16_t)(1u + rnd() % 5u)
                      : x < 6u
                          ? (uint16_t)(rnd() % 2u ? 040000u + rnd() % 8u : 0137777u - rnd() % 8u)
                          : sign_extended(rnd());
        if (z == MKVACFND_AT && rnd() % 4u) s->words[0] = (uint16_t)(area() - 2u);
        s->words[2] = return_address();
        x = (uint16_t)(rnd() % 6u);
        io->in.channel16 = x == 0u   ? 040u
                           : x == 1u ? 0100u
                           : x == 2u ? (uint16_t)(rnd() % 040u)
                           : x == 3u ? 0u
                                     : (uint16_t)(rnd() & 077777u);
        io->io.channel12 = (uint16_t)(rnd() & 077777u);
        if (rnd() % 16u == 0u) io->io.superbank = (agc_word)(rnd() & 0160u);
        io->io.interrupt_enabled = (uint16_t)(rnd() % 2u);
        if (rnd() % 32u == 0u) io->io.interrupt_enabled = 2u;
        if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 64u == 0u) s->words[7] = 1u;
    }
    eb = (uint16_t)(s->words[3] >> 8);
    s->words[6] = (uint16_t)(s->words[4] | eb);
}

int main(void)
{
    unsigned i, n, failures = 0;
    const agc_routine_table *readable = &cm_sxtmark_basic, *faithful = &faithful_cm_sxtmark_basic;

    if (readable->count != faithful->count) {
        printf("FAIL: %u keys, faithful %u\n", readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    if (failures) {
        printf("sxtmark-cases: %u failures\n", failures);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical, z = (uint16_t)(02000u + key % 1024u);
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_channel_io io, fio, rio;
            agc_hardware hw;
            agc_native_program p;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, key, n);
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
                if (memcmp(&fio, &rio, sizeof fio))
                    printf("  channel 12 %05o, faithful %05o; interrupts %u, faithful %u\n",
                           rio.io.channel12, fio.io.channel12, rio.io.interrupt_enabled,
                           fio.io.interrupt_enabled);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %02o,%04o: the faithful routine never ran\n", key / 1024u, z);
            ++bad;
        }
        failures += bad;
    }
    printf("sxtmark-cases: %u keys from %u states each, %u failures\n", faithful->count, CASES,
           failures);
    return failures != 0;
}
