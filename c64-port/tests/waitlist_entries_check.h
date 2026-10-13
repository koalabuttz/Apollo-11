/* SPDX-License-Identifier: MIT */
#ifndef WAITLIST_ENTRIES_CHECK_H
#define WAITLIST_ENTRIES_CHECK_H
#include <stdint.h>
/* Case n of tests/waitlist_entries_cases.c against its frozen yaAGC digest;
 * for the first case of each kind also a refusal that must leave the image
 * unchanged (T3RUPT with an EXTEND pending, DLY2 -1 with interrupts enabled,
 * NOQBRSM +1 outside an interrupt). Adds to *checks, returns failures. */
uint16_t wle_check(uint16_t n, uint16_t *checks);
#endif
