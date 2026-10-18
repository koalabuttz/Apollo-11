/* SPDX-License-Identifier: MIT */
/* The C64 resident's find (platform/c64/agc_find.S) against agc_transfer.c's
 * C find, the reference, in llvm-mos's simulator (mos-sim, whose clock()
 * counts cycles): random table lists (empty tables, single entries, up to
 * 320 entries, dense and sparse keys, ranges that overlap so a key can sit
 * in several tables), keys held by a table, keys between and beyond them,
 * AGC_TRANSFER_NONE and an absent list. Prints the cases, the mean cycles of
 * each and the differences; exits 1 on any difference.
 *
 * Build: mos-sim-clang -Oz -Iinclude -Itests tools/agc_find_check.c
 * platform/c64/agc_find.S (tools/mk/agc-find.mk). */
#include <stdio.h>
#include <time.h>
#include "../src/agc_transfer.c"
agc_routine agc_find(const agc_routine_table *tables, uint8_t count, uint16_t physical);

#define TABLES 6u
#define ENTRIES 320u
#define ROUNDS 120u
#define QUERIES 300u
static agc_routine_entry entry[TABLES][ENTRIES];
static agc_routine_table table[TABLES];
static unsigned long seed = 1;
static unsigned rnd(void)
{
    seed = seed * 1103515245ul + 12345ul;
    return (unsigned)(seed >> 16);
}
static void fill(unsigned round)
{
    unsigned i, j;
    for (i = 0; i < TABLES; ++i) {
        uint16_t key = (uint16_t)(rnd() % (round & 1u ? 40000u : 200u));
        unsigned size = rnd() % (round % 5u == 0u ? ENTRIES : i == 2u ? 2u : 60u);
        if (rnd() % 7u == 0u) size = 0;
        for (j = 0; j < size; ++j) {
            unsigned step = 1u + rnd() % (round & 2u ? 3u : 40u);
            if ((unsigned)key + step >= AGC_TRANSFER_NONE) break;
            key = (uint16_t)(key + step);
            entry[i][j].physical = key;
            entry[i][j].call = (agc_routine)(uintptr_t)(rnd() | 1u);
        }
        table[i].entries = entry[i];
        table[i].count = (uint16_t)j;
    }
}
static uint16_t query(void)
{
    unsigned r = rnd() % 10u, t = r % TABLES;
    if (r < 5u && table[t].count) return entry[t][rnd() % table[t].count].physical;
    if (r == 5u) return AGC_TRANSFER_NONE;
    if (r == 6u && table[t].count) return (uint16_t)(entry[t][table[t].count - 1u].physical + 1u);
    if (r == 7u && table[t].count) return (uint16_t)(entry[t][0].physical - 1u);
    return (uint16_t)rnd();
}
int main(void)
{
    unsigned round, i, differ = 0;
    long c_ref = 0, c_asm = 0, c_clock = 0, cases = 0, t0, t1;
    for (round = 0; round < ROUNDS; ++round) {
        fill(round);
        for (i = 0; i < QUERIES; ++i) {
            uint16_t key = query();
            uint8_t count = (uint8_t)(rnd() % (TABLES + 1u));
            const agc_routine_table *list = rnd() % 50u ? table : 0;
            agc_routine want, got;
            t0 = clock();
            t1 = clock();
            c_clock += t1 - t0;
            t0 = clock();
            want = find(list, count, key);
            t1 = clock();
            c_ref += t1 - t0;
            t0 = clock();
            got = agc_find(list, count, key);
            t1 = clock();
            c_asm += t1 - t0;
            ++cases;
            if (want != got) {
                if (differ < 8u)
                    printf("DIFF round %u key %u count %u: C %u, asm %u\n", round, key, count,
                           (unsigned)(uintptr_t)want, (unsigned)(uintptr_t)got);
                ++differ;
            }
        }
    }
    printf("agc_find: %ld cases, C %ld cycles, asm %ld cycles per lookup, %u differ\n", cases,
           (c_ref - c_clock) / cases, (c_asm - c_clock) / cases, differ);
    return differ != 0u;
}
