/* SPDX-License-Identifier: MIT */
/* The trampoline's fast paths (src/agc_transfer.c, lane D2) against the slow
 * forms they replace. agc_fixed_physical's byte form against the word form:
 * every logical address under every FB bank, with junk in FB's other bits,
 * and the superbank values that matter plus junk; then random words.
 * agc_transfer_next against agc_transfer_point plus agc_transfer_resolve, on
 * random states over basic and interpretive tables, and
 * agc_transfer_next_paged against agc_transfer_next: the same answer with
 * every key's page marked, and no search with none; and agc_transfer_run_at,
 * given the point agc_transfer_run works out, against agc_transfer_run: the
 * same status, steps and state. Any difference fails. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "agc_transfer.h"

static uint16_t reference(agc_word logical, agc_word fb, agc_word superbank) {
    agc_word bank;
    logical &= 07777u;
    if (logical < 02000u) return AGC_TRANSFER_NONE;
    if (logical >= 06000u) bank = 3;
    else if (logical >= 04000u) bank = 2;
    else {
        bank = (agc_word)((fb & 076000u) >> 10);
        if (bank >= 030u && (superbank & 0100u)) bank = (agc_word)(bank + 8u);
    }
    return (uint16_t)(bank * 1024u + (logical & 01777u));
}

static enum exec_status r0(const agc_native_program *p, apollo_overlay_state *s, void *io) { (void)p; (void)s; (void)io; return EXEC_OK; }
static enum exec_status r1(const agc_native_program *p, apollo_overlay_state *s, void *io) { (void)p; (void)s; (void)io; return EXEC_OK; }
static enum exec_status r2(const agc_native_program *p, apollo_overlay_state *s, void *io) { (void)p; (void)s; (void)io; return EXEC_OK; }
/* For the run checks: moves Z on, so a run takes more than one step. */
static enum exec_status r3(const agc_native_program *p, apollo_overlay_state *s, void *io) { (void)p; (void)io; s->words[5] = (agc_word)(s->words[5] + 0335u); s->words[0100] ^= 01u; return (s->words[5] & 7u) == 7u ? EXEC_UNMAPPED : EXEC_OK; }
static const agc_routine_entry basic0[] = {{02010u, r0}, {02345u, r1}, {010000u, r2}, {014321u, r0}, {AGC_TRANSFER_ERASABLE_BASE + 01234u, r1}};
static const agc_routine_entry basic1[] = {{037777u, r2}, {040000u, r1}, {044000u, r0}};
static const agc_routine_entry interp0[] = {{02011u, r2}, {03000u, r0}, {043210u, r1}};
static const agc_routine_table basic[] = {{basic0, 5}, {basic1, 3}, {0, 0}}, interp[] = {{interp0, 3}};
static const uint16_t keys[] = {02010u, 02345u, 010000u, 014321u, 037777u, 040000u, 044000u, 02011u, 03000u, 043210u, 015000u};

static unsigned long next_checks(unsigned long *checks) {
    agc_native_program p = {basic, 3, interp, 1, 0, 0, 014321u, 0100u, 0101u, 0, 0, 0};
    apollo_overlay_state s;
    unsigned long failures = 0;
    uint32_t i, x = 777u;
    uint8_t all[8] = {0}, none[8] = {0};
    for (i = 0; i < sizeof keys / sizeof keys[0]; ++i) all[(keys[i] >> 11) & 7u] |= (uint8_t)(1u << (keys[i] >> 8 & 7u));
    all[((AGC_TRANSFER_ERASABLE_BASE + 01234u) >> 11) & 7u] |= (uint8_t)(1u << ((AGC_TRANSFER_ERASABLE_BASE + 01234u) >> 8 & 7u));
    for (i = 0; i < 2000000u; ++i) {
        uint16_t k, stop, point;
        agc_routine call;
        agc_word sb;
        x = x * 1103515245u + 12345u; k = keys[(x >> 16) % (sizeof keys / sizeof keys[0])];
        /* Z: a keyed point's logical address (bank 2/3 fixed-fixed or its FB bank), or junk. */
        x = x * 1103515245u + 12345u;
        if (x & 0x10000u) {
            s.words[4] = (agc_word)((k >> 10) << 10 | (x >> 20 & 01777u));
            s.words[5] = (agc_word)(k >= 010000u && k < 020000u ? (k >> 10 == 2 ? 04000u : 06000u) + (k & 01777u) : 02000u + (k & 01777u));
        } else { s.words[4] = (agc_word)(x >> 8); s.words[5] = (agc_word)(x >> 3); }
        x = x * 1103515245u + 12345u; s.words[3] = (agc_word)(x >> 8);
        x = x * 1103515245u + 12345u; s.words[0100] = (agc_word)(x & 0x10000u ? 02000u + (x >> 20 & 01777u) : x >> 8);
        x = x * 1103515245u + 12345u; s.words[0101] = (agc_word)(x >> 8);
        sb = (agc_word)(x & 0x100u ? 0100u : 0u);
        x = x * 1103515245u + 12345u;
        stop = (x & 0x30000u) == 0 ? agc_transfer_point(&p, &s, sb) : (x & 0x30000u) == 0x10000u ? AGC_TRANSFER_NONE : (uint16_t)(x >> 8);
        ++*checks;
        point = agc_transfer_next(&p, &s, sb, stop, &call);
        if (point != agc_transfer_point(&p, &s, sb) || call != (point == stop ? 0 : agc_transfer_resolve(&p, &s, sb))) ++failures;
        {
            agc_routine paged;
            ++*checks;
            if (agc_transfer_next_paged(&p, &s, sb, stop, &paged, all) != point || paged != call) ++failures;
            ++*checks;
            if (agc_transfer_next_paged(&p, &s, sb, stop, &paged, none) != point || paged) ++failures;
        }
    }
    return failures;
}

/* run_at with the point run works out, against run, from the same state. */
static unsigned long run_checks(unsigned long *checks) {
    static const agc_routine_entry rb0[] = {{02010u, r3}, {02345u, r3}, {010000u, r3}, {014321u, r0}, {AGC_TRANSFER_ERASABLE_BASE + 01234u, r3}};
    static const agc_routine_entry ri0[] = {{02011u, r3}, {03000u, r3}, {043210u, r1}};
    static const agc_routine_table rb[] = {{rb0, 5}}, ri[] = {{ri0, 3}};
    agc_native_program p = {rb, 1, ri, 1, 0, 0, 014321u, 0100u, 0101u, 0, 0, 0};
    agc_native_program plain = p;
    apollo_overlay_state a, b;
    unsigned long failures = 0;
    uint32_t i, x = 4242u;
    plain.interpretive_count = 0;
    plain.interpreter_fetch = AGC_TRANSFER_NONE;
    for (i = 0; i < 1000000u; ++i) {
        uint16_t k, stop, first, z, na = 0, nb = 0;
        enum exec_status sa, sb2;
        agc_word sb;
        bool in;
        memset(&a, 0, sizeof a);
        x = x * 1103515245u + 12345u; k = keys[(x >> 16) % (sizeof keys / sizeof keys[0])];
        x = x * 1103515245u + 12345u;
        if (x & 0x10000u) {
            a.words[4] = (agc_word)((k >> 10) << 10 | (x >> 20 & 01777u));
            a.words[5] = (agc_word)(k >= 010000u && k < 020000u ? (k >> 10 == 2 ? 04000u : 06000u) + (k & 01777u) : 02000u + (k & 01777u));
        } else { a.words[4] = (agc_word)(x >> 8); a.words[5] = (agc_word)(x >> 3); }
        x = x * 1103515245u + 12345u; a.words[3] = (agc_word)(x >> 8);
        x = x * 1103515245u + 12345u; a.words[0100] = (agc_word)(x & 0x10000u ? 02000u + (x >> 20 & 01777u) : x >> 8);
        x = x * 1103515245u + 12345u; a.words[0101] = (agc_word)(x >> 8);
        sb = (agc_word)(x & 0x100u ? 0100u : 0u);
        x = x * 1103515245u + 12345u;
        stop = (x & 0x30000u) == 0 ? AGC_TRANSFER_NONE : (uint16_t)(x >> 8);
        if ((x & 0xc0000u) == 0) {
            /* Z at the interpreter fetch, LOC under BANKSET at an interpretive key. */
            a.words[4] = 06u << 10; a.words[5] = 02000u + 0321u;
            a.words[0101] = 01u << 10; a.words[0100] = 03000u;
        }
        b = a;
        /* The point, when it is not interpretive: a program without the
         * fetch or interpretive tables gives Z's own point. An interpretive
         * point (or maybe one, where the two agree) is left to run_at. */
        first = agc_transfer_point(&p, &b, sb);
        z = agc_transfer_point(&plain, &b, sb);
        in = first != z || p.interpreter_fetch == z;
        if (in) first = AGC_TRANSFER_NONE;
        sa = agc_transfer_run(&p, &a, 0, &sb, stop, (uint16_t)(x >> 28), &na);
        sb2 = agc_transfer_run_at(&p, &b, 0, &sb, stop, (uint16_t)(x >> 28), &nb, first);
        ++*checks;
        if (sa != sb2 || na != nb || memcmp(&a, &b, sizeof a)) ++failures;
    }
    return failures;
}

int main(void) {
    static const agc_word superbanks[] = {0u, 040u, 060u, 0100u, 0140u, 0160u, 0177u, 0177777u};
    unsigned long checks = 0, failures = 0;
    uint32_t logical, b, i, x = 12345u;
    for (logical = 0; logical <= 0xffffu; ++logical)
        for (b = 0; b < 32u; ++b)
            for (i = 0; i < sizeof superbanks / sizeof superbanks[0]; ++i) {
                agc_word fb = (agc_word)(b << 10 | (logical * 2654435761u >> 7 & 0101777u));
                ++checks;
                if (agc_fixed_physical((agc_word)logical, fb, superbanks[i]) !=
                    reference((agc_word)logical, fb, superbanks[i])) {
                    if (++failures < 10)
                        printf("logical %05o fb %06o superbank %06o: %u, expected %u\n", (unsigned)logical,
                               (unsigned)fb, (unsigned)superbanks[i],
                               (unsigned)agc_fixed_physical((agc_word)logical, fb, superbanks[i]),
                               (unsigned)reference((agc_word)logical, fb, superbanks[i]));
                }
            }
    for (i = 0; i < 4000000u; ++i) {
        agc_word l, f, s;
        x = x * 1103515245u + 12345u; l = (agc_word)(x >> 8);
        x = x * 1103515245u + 12345u; f = (agc_word)(x >> 8);
        x = x * 1103515245u + 12345u; s = (agc_word)(x >> 8);
        ++checks;
        if (agc_fixed_physical(l, f, s) != reference(l, f, s)) ++failures;
    }
    failures += next_checks(&checks);
    failures += run_checks(&checks);
    printf("agc-transfer-fast: %lu checks, %lu failures\n", checks, failures);
    return failures != 0;
}
