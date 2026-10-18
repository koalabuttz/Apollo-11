/* SPDX-License-Identifier: MIT */
/* cm_auto_man_hot_routines against cm_auto_man_basic: the hot table (the
 * spec's 6 hot_points, the flight store's cm_man pack) holds each of its
 * keys once, in key order, and calls the same function the full table calls
 * for that key, so a pass runs the same routine through either table. */
#include <stdio.h>
#include "cm_auto_man.h"
#define HOT_POINTS 6u /* tools/interp_specs.py cm-auto-man hot_points */
static agc_routine find(const agc_routine_table *t, uint16_t physical)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i) {
        if (t->entries[i].physical == physical) return t->entries[i].call;
    }
    return 0;
}
int main(void)
{
    const agc_routine_table *h = &cm_auto_man_hot_routines;
    unsigned i, failures = 0;
    if (h->count != HOT_POINTS) {
        printf("FAIL: %u hot entries, the spec names %u\n", (unsigned)h->count, HOT_POINTS);
        ++failures;
    }
    if (h->entries[h->count].physical != 0xffffu || h->entries[h->count].call) {
        printf("FAIL: no sentinel after the hot entries\n");
        ++failures;
    }
    for (i = 0; i < h->count; ++i) {
        uint16_t k = h->entries[i].physical;
        if (i && k <= h->entries[i - 1u].physical) {
            printf("FAIL: hot key %u out of order\n", (unsigned)k);
            ++failures;
        }
        if (!h->entries[i].call || find(&cm_auto_man_basic, k) != h->entries[i].call) {
            printf("FAIL: hot key %u calls another function than cm_auto_man_basic\n",
                   (unsigned)k);
            ++failures;
        }
    }
    printf("cm-auto-man-hot: %u hot entries, %u failures\n", (unsigned)h->count, failures);
    return failures != 0;
}
