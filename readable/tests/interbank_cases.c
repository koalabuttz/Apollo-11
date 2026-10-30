/* SPDX-License-Identifier: MIT */
/* The readable inter-bank routines against the faithful port's, case by
 * case (make interbank-cases).
 *
 * Built once per mission, with the readable tables (src/binding/
 * interbank_tables.c) and the faithful port's under the same names with
 * faithful_ in front (the Makefile renames them). Every entry of every
 * table runs on random machine states from the same start through both:
 * each must give the same status (run, or refused as out of scope), the
 * same erasable memory and registers, and the same channels, word for
 * word. The faithful routines are certified against the original program
 * (c64-port/tests/test_lm_interbank.c, test_cm_interbank_complete.c).
 *
 * The states are mostly ones the routines take (the bank registers in
 * agreement, Q a return address, the CADR after the TC in erasable memory
 * or a compiled operand), with every condition broken now and then, and
 * overflows in A and L. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION "LM"
#define TABLES(X)                                                                                  \
    X(lm_interbank_routines)                                                                       \
    X(lm_interbank_idle_chunk)                                                                     \
    X(lm_interbank_dap_hot_routines)                                                               \
    X(lm_interbank_postjump_hot_routines)                                                          \
    X(lm_interbank_postjump_t4_hot_routines)
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION "CM"
#define TABLES(X)                                                                                  \
    X(cm_interbank_routines)                                                                       \
    X(cm_interbank_idle_chunk)                                                                     \
    X(cm_interbank_postjump_hot_routines)
#endif

#define DECLARE(t) extern const agc_routine_table t, faithful_##t;
TABLES(DECLARE)
#define PAIR(t) {#t, &t, &faithful_##t},
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {TABLES(PAIR)};

#define CASES 100000u
#define SITES 64u

static uint32_t seed = 12345u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* The compiled operands (the CADRs after TCs in fixed memory) and the rope. */
static agc_site_operand sites[SITES];
static uint16_t hash(uint16_t physical)
{
    uint32_t x = physical * 2654435761u;
    return (uint16_t)(x >> 13);
}
static bool rope(const agc_native_program *p, uint16_t physical, agc_word *out)
{
    uint16_t h = hash(physical);
    (void)p;
    if (h % 16u == 0u) return false;                        /* not in this program's rope */
    *out = (agc_word)(h % 9u ? h & 077777u : h | 0100000u); /* now and then no word */
    return true;
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

/* A register value: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w & 040000u) << 1));
}

static void state(apollo_overlay_state *s, mission_io *io, uint16_t z, uint32_t n)
{
    unsigned i, kind = rnd() % 16u;
    const agc_site_operand *site = &sites[n % SITES];

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    s->words[4] = (uint16_t)((rnd() % 040u) << 10);
    io->superbank = (uint16_t)(rnd() & 0160u);
    io->interrupt_enabled = rnd() % 2u;
    /* Q: after a compiled TC in fixed memory, after one with no operand,
     * in erasable memory, under EB, small, or a bank address (QJUMP). */
    if (kind < 5u) {
        uint16_t bank = (uint16_t)(site->physical / 1024u),
                 offset = (uint16_t)(site->physical % 1024u);
        if (bank == 2u || bank == 3u)
            s->words[2] = (uint16_t)(bank * 02000u + offset);
        else {
            if (bank >= 040u) {
                bank = (uint16_t)(bank - 010u);
                io->superbank = 0100u;
            } else if (bank >= 030u)
                io->superbank = 0u;
            s->words[4] = (uint16_t)(bank << 10);
            s->words[2] = (uint16_t)(02000u + offset);
        }
    } else if (kind < 7u)
        s->words[2] = (uint16_t)(02000u + rnd() % 06000u);
    else if (kind < 10u)
        s->words[2] = (uint16_t)(0100u + rnd() % 01300u);
    else if (kind < 12u)
        s->words[2] = (uint16_t)(01400u + rnd() % 0400u);
    else if (kind < 13u)
        s->words[2] = (uint16_t)(rnd() % 0100u);
    else
        s->words[2] = (uint16_t)(0100u + rnd() % 01700u);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    s->words[5] = z;
    /* The returns kept in BUF2 and RUPTREG3: mostly addresses. */
    if (rnd() % 8u) s->words[0133] = (uint16_t)(0100u + rnd() % 07700u);
    if (rnd() % 8u) s->words[072] = (uint16_t)(0100u + rnd() % 07700u);
    /* Now and then break one condition. */
    switch (rnd() % 32u) {
    case 0: s->words[7] = 1; break;
    case 1: s->words[6] ^= 1; break;
    case 2: io->superbank = 1; break;
    case 3: io->interrupt_enabled = 2; break;
    case 4: s->words[3] = 0100; break;
    case 5: s->words[2] = (uint16_t)(07776u + rnd() % 2u); break;
    case 6: s->words[2] = (uint16_t)(rnd() | (rnd() << 1)); break;
    case 7: s->words[4] |= 1u; break;
    case 8: /* Z in the switched window at the same physical word */
        if (z < 06000u) {
            s->words[5] = (uint16_t)(z - 02000u);
            s->words[4] = 04000u;
            s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
        }
        break;
    case 9: s->words[2] = 0177776u; break;
    default: break;
    }
}

int main(void)
{
    agc_native_program program;
    unsigned t, i, j, checks = 0, failures = 0;

    for (i = 0; i < SITES; ++i) {
        sites[i].physical = (uint16_t)(2u * 1024u + (i * 1031u) % (42u * 1024u));
        sites[i].word = (uint16_t)(rnd() % 8u ? rnd() & 077777u : rnd() | 0100000u);
    }
    for (i = 1; i < SITES; ++i) /* sorted, for the lookup */
        for (j = i; j && sites[j - 1u].physical > sites[j].physical; --j) {
            agc_site_operand swap = sites[j];
            sites[j] = sites[j - 1u];
            sites[j - 1u] = swap;
        }
    memset(&program, 0, sizeof program);
    program.operands = sites;
    program.operand_count = SITES;
    program.fixed_data = rope;

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t) {
        const agc_routine_table *readable = tables[t].readable, *faithful = tables[t].faithful;
        if (readable->count != faithful->count) {
            printf("FAIL %s: %u entries, faithful %u\n", tables[t].name, readable->count,
                   faithful->count);
            ++failures;
            continue;
        }
        for (i = 0; i < faithful->count; ++i) {
            uint16_t key = faithful->entries[i].physical, z = (uint16_t)(04000u + key - 2048u);
            agc_routine r = find(readable, key), f = faithful->entries[i].call;
            unsigned ran = 0, n, bad = 0;
            if (!r) {
                printf("FAIL %s: no readable entry at %05o\n", tables[t].name, z);
                ++failures;
                continue;
            }
            for (n = 0; n < CASES; ++n) {
                apollo_overlay_state a, b;
                mission_io ia, ib;
                enum exec_status sa, sb;
                state(&a, &ia, z, n);
                b = a;
                ib = ia;
                sa = f(&program, &a, &ia);
                sb = r(&program, &b, &ib);
                ++checks;
                ran += sa == EXEC_OK;
                if (sa != sb || memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia)) {
                    if (++bad <= 3) {
                        unsigned w;
                        printf("FAIL %s %05o case %u: status %d, faithful %d\n", tables[t].name, z,
                               n, (int)sb, (int)sa);
                        for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                            if (a.words[w] != b.words[w])
                                printf("  word %04o: %06o, faithful %06o\n", w, b.words[w],
                                       a.words[w]);
                        if (ia.superbank != ib.superbank ||
                            ia.interrupt_enabled != ib.interrupt_enabled)
                            printf("  channels differ\n");
                    }
                }
            }
            failures += bad;
            printf("%s %s %05o: %u cases, %u run, %u failures\n", MISSION, tables[t].name, z, CASES,
                   ran, bad);
        }
    }
    printf("interbank-cases %s: %u checks, %u failures\n", MISSION, checks, failures);
    return failures != 0;
}
