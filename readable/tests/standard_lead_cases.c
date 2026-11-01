/* SPDX-License-Identifier: MIT */
/* The readable SYSTEM_TEST_STANDARD_LEAD_INS against the faithful port's,
 * case by case (make standard-lead-cases).
 *
 * Built once per mission, with the readable tables (src/binding/
 * standard_lead_routines.c) and the faithful port's under the same names
 * with faithful_ in front (the Makefile renames them). First E/CALL's
 * interpretive words, in the listing's notation, must be the original
 * program's rope, word for word. Then every entry of every table runs on
 * random machine states from the same start through both: each must give
 * the same status (run, JOBWAKE's no such job, or refused as out of
 * scope), the same erasable memory and registers, and the same channels,
 * word for word. The faithful routines are certified against the original
 * program (c64-port/tests/test_standard_lead_oracle.c, test_cm_lead_tests.c,
 * test_interp_entry.c).
 *
 * The states are mostly ones the routines take (the bank registers in
 * agreement, the CADR after the call in erasable memory or a compiled
 * operand, a sleeping job to wake, the interpreter's registers as it sets
 * them), with every condition broken now and then, and overflows in A and
 * L. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#include "rope_image.h"
#include "../src/standard_lead/standard_lead.h"
#if LUMINARY099
#include "p66_bias.h"
typedef p66_bias_io mission_io;
#define MISSION       "LM"
#define MISSION_IS_CM 0u
#define LEAD_BANK     04u
#define E_BKCALL_AT   02576u
#define E_JOBWAK_AT   02631u
#define INTPRET_AT    06037u
#define NEWOPS_AT     06070u
#define EBUF2_AT      01167u
#define CORE_SETS     8u
#define TABLES(X)                                                                                  \
    X(standard_lead_routines)                                                                      \
    X(lm_interpreter_routines)                                                                     \
    X(standard_lead_interpretive_routines)
#else
#include "cm_native_image.h"
typedef cm_native_io mission_io;
#define MISSION       "CM"
#define MISSION_IS_CM 1u
#define LEAD_BANK     013u
#define E_BKCALL_AT   02036u
#define E_JOBWAK_AT   02606u
#define INTPRET_AT    06006u
#define NEWOPS_AT     06037u
#define EBUF2_AT      0364u
#define CORE_SETS     7u
#define TABLES(X)                                                                                  \
    X(cm_standard_lead_routines)                                                                   \
    X(cm_interpreter_routines)                                                                     \
    X(cm_standard_lead_interpretive_routines)
#endif
#define E_CALL_AT (E_BKCALL_AT + 017u)

#define DECLARE(t) extern const agc_routine_table t, faithful_##t;
TABLES(DECLARE)
#define PAIR(t) {#t, &t, &faithful_##t},
static const struct {
    const char *name;
    const agc_routine_table *readable, *faithful;
} tables[] = {TABLES(PAIR)};

#define CASES 50000u
#define SITES 64u

static uint32_t seed = 12345u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* The compiled operands: the CADRs after calls from fixed memory. */
static agc_site_operand sites[SITES];

/* A register value: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w & 040000u) << 1));
}

/* An address for an inline CADR: erasable, switched erasable, in this
 * file's bank or fixed-fixed at a compiled operand, or in fixed memory at
 * none. */
static uint16_t cadr_place(void)
{
    const agc_site_operand *site = &sites[rnd() % SITES];
    uint16_t bank = (uint16_t)(site->physical / 1024u), offset = (uint16_t)(site->physical % 1024u);

    switch (rnd() % 6u) {
    case 0: return (uint16_t)(0100u + rnd() % 01300u);
    case 1: return (uint16_t)(01400u + rnd() % 0400u);
    case 2:
        if (bank == 2u || bank == 3u) return (uint16_t)(bank * 02000u + offset);
        return bank == LEAD_BANK ? (uint16_t)(02000u + offset)
                                 : (uint16_t)(02000u + rnd() % 02000u);
    case 3: return (uint16_t)(02000u + rnd() % 06000u);
    default: return (uint16_t)(0100u + rnd() % 07700u);
    }
}

/* A fixed CADR: a bank of 020-037 and an address in it. */
static uint16_t callee(void)
{
    return (uint16_t)(((020u + rnd() % 020u) << 10) | (rnd() % 02000u));
}

static void state(apollo_overlay_state *s, mission_io *io, uint16_t z, uint16_t loc)
{
    unsigned i, eb = rnd() % 8u, fbank = LEAD_BANK;
    word interpretive = loc != 0u;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = (uint16_t)(rnd() & 077777u);
    io->superbank = (uint16_t)(rnd() & 0160u);
    io->interrupt_enabled = rnd() % 2u;
    if (z == E_JOBWAK_AT) fbank = 04u;
    if (z >= 06000u && rnd() % 2u) fbank = rnd() % 040u;
#if COMANCHE055
    if (z <= 02002u) fbank = 033u;
#endif
    s->words[3] = (uint16_t)(eb << 8);
    s->words[4] = (uint16_t)(fbank << 10);
    s->words[5] = interpretive ? NEWOPS_AT : z;
    /* The words the lead-ins read. */
    s->words[0133] = cadr_place();                  /* BUF2: E/BKCALL's CADR's address */
    s->words[0164] = (uint16_t)(cadr_place() - 1u); /* LOC: E/CALL's, less one */
    s->words[EBUF2_AT] = (uint16_t)(rnd() % 4u ? 0100u + rnd() % 07676u : rnd() & 077777u);
    s->words[EBUF2_AT + 1u] = (uint16_t)(rnd() & 077777u);
    s->words[0120] = (uint16_t)(rnd() % 8u ? 0401u + 054u * (rnd() % 5u) : rnd() & 01777u);
    /* E/JOBWAK: called inhibited, returning through RUPTREG3, core sets in
     * use (half asleep), A one of their LOCs, NEWJOB -0 or a core set. */
    if (z == E_JOBWAK_AT) {
        io->interrupt_enabled = rnd() % 8u == 0u;
        s->words[072] = (uint16_t)(rnd() % 8u ? 0100u + rnd() % 07700u : rnd() & 077777u);
        for (i = 0; i < CORE_SETS; ++i) {
            uint16_t p = (uint16_t)(1u + rnd() % 037776u);
            s->words[0167u + 12u * i] = rnd() % 2u ? (uint16_t)(p | 040000u) : p;
            if (rnd() % 4u == 0u) s->words[0167u + 12u * i] = 077777u; /* free */
            if (rnd() % 64u == 0u) s->words[0167u + 12u * i] = 0u;
            s->words[0164u + 12u * i] =
                (uint16_t)(rnd() % 3u ? 0100u + rnd() % 01700u : rnd() & 077777u);
            s->words[0165u + 12u * i] =
                (uint16_t)(rnd() % 8u ? (rnd() & 7u) | io->superbank : rnd() & 077777u);
        }
        s->words[067] = (uint16_t)(rnd() % 3u ? (rnd() % CORE_SETS) * 12u : 077777u);
        if (rnd() % 16u == 0u) s->words[067] = (uint16_t)(rnd() & 077777u);
        if (rnd() % 16u == 0u) s->words[067] = (uint16_t)(CORE_SETS * 12u); /* one past */
        s->words[0] = (uint16_t)(rnd() % 4u ? s->words[0164u + 12u * (rnd() % CORE_SETS)] : reg());
        if (rnd() % 8u == 0u) s->words[0] = 0u;
        s->words[0] = (uint16_t)(s->words[0] | ((s->words[0] & 040000u) << 1));
        if (rnd() % 16u == 0u) s->words[0] ^= 0100000u;
#if COMANCHE055
        io->executive_alarm = rnd() % 16u == 0u;
#endif
    }
    /* INTPRET from E/CALL: Q at its CALL, in its bank. */
    if (z == INTPRET_AT && rnd() % 2u) {
        s->words[2] = (uint16_t)(E_CALL_AT + 7u);
        s->words[4] = (uint16_t)(LEAD_BANK << 10);
    }
    if (z == INTPRET_AT + 2u && rnd() % 2u) s->words[1] = (uint16_t)(rnd() % 04000u);
    /* E/CALL's CALL or EXIT at the fetch point: BANKSET in this file's
     * bank, BBANK loaded from it, a fixed CADR in EBUF2. */
    if (interpretive) {
        s->words[4] = (uint16_t)(LEAD_BANK << 10);
        s->words[0164] = loc;
        s->words[0165] = (uint16_t)(s->words[4] | eb | (rnd() % 4u ? 0u : 040000u));
        if (rnd() % 4u) s->words[EBUF2_AT] = callee();
    }
#if COMANCHE055
    /* yaAGC sign-corrects L after every instruction, so the one-TC entry
     * words see no overflow in L (the faithful port's pass one on). */
    if (z <= 02002u)
        s->words[1] = (uint16_t)((s->words[1] & 077777u) | ((s->words[1] & 040000u) << 1));
#endif
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    /* Now and then break one condition. */
    switch (rnd() % 32u) {
    case 0: s->words[7] = 1; break;
    case 1: s->words[6] ^= 1; break;
    case 2: io->superbank = 1; break;
    case 3: io->interrupt_enabled = 2; break;
    case 4: s->words[3] = 0100; break;
    case 5: s->words[4] |= 1u; break;
    case 6:
        s->words[4] ^= 04000u;
        s->words[6] ^= 04000u;
        break;
    case 7: s->words[0165] ^= 02000u; break;
    case 8: s->words[0133] = (uint16_t)(rnd() % 0100u); break;
    case 9: s->words[0133] = (uint16_t)(07776u + rnd() % 2u); break;
    case 10: s->words[EBUF2_AT] = (uint16_t)(rnd() % 0100u); break;
    case 11: s->words[0164] = (uint16_t)(rnd() % 2u ? rnd() % 077u : 07775u + rnd() % 2u); break;
    case 12: s->words[0164] ^= 1u; break;
    default: break;
    }
}

/* Whether the call's CADR is in a central register, through EBANK 0's
 * window (src/binding/standard_lead_routines.c leaves those calls to the
 * faithful port: it reads the word plainly, where the AGC's CA rewrites
 * an editing register). */
static bool cadr_in_a_register(const apollo_overlay_state *s, uint16_t z)
{
    uint16_t q;

    if (z == E_BKCALL_AT)
        q = s->words[0133];
    else if (z == E_CALL_AT)
        q = (uint16_t)((s->words[0164] + 1u) & 07777u);
    else
        return false;
    if (q >= 01400u && q < 02000u) q = (uint16_t)(s->words[3] + (q & 0377u));
    return q < 0100u;
}

/* E/CALL's words are the rope's. */
static unsigned words_as_the_rope(void)
{
    const struct interpretive_code *code = &STANDARD_LEAD_CODE;
    unsigned i, bad = 0;

    for (i = 0; i < code->count; ++i) {
        unsigned physical = code->bank * 1024u + code->first + i - 02000u;
        agc_word rope = 0;
        if (!apollo_rope_read(MISSION_IS_CM, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++bad;
        }
    }
    return bad;
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

int main(int argc, char **argv)
{
    agc_native_program program;
    unsigned t, i, j, checks = 0, failures = 0, keys = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures += words_as_the_rope();
    for (i = 0; i < SITES; ++i) {
        static const uint16_t banks[] = {2u, 3u, LEAD_BANK, 020u, 033u};
        sites[i].physical = (uint16_t)(banks[i % 5u] * 1024u + (i * 37u) % 1024u);
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

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t) {
        const agc_routine_table *readable = tables[t].readable, *faithful = tables[t].faithful;
        if (readable->count != faithful->count) {
            printf("FAIL %s: %u entries, faithful %u\n", tables[t].name, readable->count,
                   faithful->count);
            ++failures;
            continue;
        }
        for (i = 0; i < faithful->count; ++i) {
            uint16_t key = faithful->entries[i].physical, z, loc = 0;
            agc_routine r = find(readable, key), f = faithful->entries[i].call;
            unsigned ran = 0, left = 0, n, bad = 0;
            ++keys;
            if (key >= 2048u && key < 4096u)
                z = (uint16_t)(04000u + key - 2048u);
            else
                z = (uint16_t)(02000u + key % 1024u);
            if (t == 2u) loc = z; /* an interpretive word: LOC names it */
            if (!r) {
                printf("FAIL %s: no readable entry at %05o\n", tables[t].name, z);
                ++failures;
                continue;
            }
            for (n = 0; n < CASES; ++n) {
                static apollo_overlay_state a, b;
                mission_io ia, ib;
                enum exec_status sa, sb;
                state(&a, &ia, z, loc);
                b = a;
                ib = ia;
                if (cadr_in_a_register(&a, z)) {
                    /* Left to the faithful port: refused, nothing changed. */
                    sb = r(&program, &b, &ib);
                    ++checks;
                    ++left;
                    if (sb != EXEC_OUT_OF_SCOPE || memcmp(&a, &b, sizeof a) ||
                        memcmp(&ia, &ib, sizeof ia)) {
                        if (++bad <= 3)
                            printf("FAIL %s %05o case %u: a CADR in a register run\n",
                                   tables[t].name, z, n);
                    }
                    continue;
                }
                sa = f(&program, &a, &ia);
                sb = r(&program, &b, &ib);
                ++checks;
                ran += sa == EXEC_OK || sa == EXEC_NOT_FOUND;
                if (sa != sb || memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia)) {
                    if (++bad <= 3) {
                        unsigned w;
                        printf("FAIL %s %05o case %u: status %d, faithful %d\n", tables[t].name, z,
                               n, (int)sb, (int)sa);
                        for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                            if (a.words[w] != b.words[w])
                                printf("  word %04o: %06o, faithful %06o\n", w, b.words[w],
                                       a.words[w]);
                        if (memcmp(&ia, &ib, sizeof ia)) printf("  channels differ\n");
                    }
                }
            }
            failures += bad;
            printf("%s %s %05o: %u cases, %u run, %u left to the faithful port, %u failures\n",
                   MISSION, tables[t].name, z, CASES, ran, left, bad);
        }
    }
    printf("standard-lead-cases (%s): %u words as the rope's, %u keys, %u checks, %u failures\n",
           MISSION, STANDARD_LEAD_CODE.count, keys, checks, failures);
    return failures != 0;
}
