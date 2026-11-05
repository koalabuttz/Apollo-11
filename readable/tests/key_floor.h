/* SPDX-License-Identifier: MIT */
/* The per-key floor of the side-by-side tests that draw their states at
 * random (burn-baby-cases, lunar-landing-cases, throttle-cases): each
 * key's faithful routine must run in at least 1 of 16 of the states
 * entered at it, so that a change to the states cannot quietly leave a
 * key with a handful of live ones. A key is the point the state's entry
 * resolves to in the readable tables (agc_transfer_next); a state whose
 * point has no routine there is no key's. */
#ifndef READABLE_TESTS_KEY_FLOOR_H
#define READABLE_TESTS_KEY_FLOOR_H

#include <stdio.h>
#include "agc_transfer.h"

static struct {
    unsigned tried, ran;
} key_tallies[65536];

/* One state entered at a point (its routine call, or none). */
static void key_tally(uint16_t point, agc_routine call, int faithful_ran)
{
    if (!call) return;
    ++key_tallies[point].tried;
    key_tallies[point].ran += faithful_ran != 0;
}

/* Whether a point's routine is the given table's: an order pair, when
 * the table is the interpretive one. (The flight builds stop at the
 * interpreter, so no order pair runs there, on either side: their floor
 * is the host build's.) */
static inline int key_in(const agc_routine_table *table, uint16_t point, agc_routine call)
{
    unsigned i;

    for (i = 0; i < table->count; ++i)
        if (table->entries[i].physical == point) return table->entries[i].call == call;
    return 0;
}

/* The keys under the floor, each a failure; and the fewest live states. */
static unsigned keys_under_floor(const char *test)
{
    unsigned i, failures = 0, keys = 0, least = 0;

    for (i = 0; i < 65536u; ++i) {
        if (!key_tallies[i].tried) continue;
        if (key_tallies[i].ran < key_tallies[i].tried / 16u) {
            printf("FAIL %s %02o,%04o: the faithful routine ran in %u of %u states (fewer than 1 "
                   "in 16)\n",
                   test, i / 1024u, 02000u + i % 1024u, key_tallies[i].ran, key_tallies[i].tried);
            ++failures;
        }
        if (!keys++ || key_tallies[i].ran * key_tallies[least].tried <
                           key_tallies[least].ran * key_tallies[i].tried)
            least = i;
    }
    if (keys)
        printf("%s: %u keys entered, the fewest live states at %02o,%04o: %u of %u\n", test, keys,
               least / 1024u, 02000u + least % 1024u, key_tallies[least].ran,
               key_tallies[least].tried);
    return failures;
}

/* The keys of a table no state was entered at, each a failure: a key the
 * cases never start at is a key the floor cannot see. */
static inline unsigned keys_unentered(const agc_routine_table *table, const char *test)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i)
        if (!key_tallies[table->entries[i].physical].tried) {
            printf("FAIL %s %02o,%04o: no state was entered at the key\n", test,
                   table->entries[i].physical / 1024u, 02000u + table->entries[i].physical % 1024u);
            ++failures;
        }
    return failures;
}

#endif
