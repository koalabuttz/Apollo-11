/* SPDX-License-Identifier: MIT */
/* MPAC +3 after an operator's request (verbs 30 and 31): the readable
 * binding's table of one key (request_return, src/binding/
 * pinball_routines.c) against the faithful port's (c64-port/src/
 * lanec_pin_request_routines.c), side by side, both missions (make
 * pinball-cases' last step).
 *
 * The Pinball tests before this one run the port's own tables with the
 * readable routines in place, and no scenario keys verb 30 or 31, so
 * nothing else enters the key. REQUESTC ends in TC MPAC; the Executive or
 * the Waitlist returns to MPAC +3, where REQUESTC stored TC ENDOFJOB, and
 * the routine runs that word alone.
 *
 * The readable tables are built with readable_ in front of their names
 * (the Makefile renames them). Each mission's key from 4,096 states: every
 * word random, then for most Z at MPAC +3 (now and then with bits above
 * the address, or a word off) and the word there TC ENDOFJOB (now and
 * then the other mission's, a neighbour, or any). Where either runs, the
 * other must too, with the same memory word for word; where the readable
 * one refuses, it must leave everything as it found it; and the faithful
 * routine must run in at least half of the states. */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#include "lanec.h"
#include "lanec_symbols.h"

extern const agc_routine_table lanec_pin_request_lm_routines, lanec_pin_request_cm_routines;
extern const agc_routine_table readable_pin_request_lm_routines, readable_pin_request_cm_routines;

/* The binding's rope: none is read at this key. */
static bool no_rope(enum lanec_mission mission, uint16_t physical, agc_word *value)
{
    (void)mission;
    (void)physical;
    (void)value;
    return false;
}
bool (*lanec_fixed_read)(enum lanec_mission, uint16_t physical, agc_word *) = no_rope;

#define STATES 4096u
#define MPAC3  (LANEC_LM_MPAC + 3u)

static uint32_t seed = 3031u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static void state(apollo_overlay_state *s, uint16_t tc_endofjob, uint16_t other)
{
    unsigned w;

    for (w = 0; w < APOLLO_SHARED_WORDS; ++w) s->words[w] = (uint16_t)(rnd() & 077777u);
    switch (rnd() % 16u) {
    case 0: break; /* Z anywhere */
    case 1: s->words[LANEC_Z] = (uint16_t)(MPAC3 + 1u - 2u * (rnd() % 2u)); break;
    case 2: s->words[LANEC_Z] = (uint16_t)(MPAC3 | (rnd() & 070000u)); break;
    default: s->words[LANEC_Z] = MPAC3; break;
    }
    switch (rnd() % 16u) {
    case 0: break; /* any word at MPAC +3 */
    case 1: s->words[MPAC3] = other; break;
    case 2: s->words[MPAC3] = (uint16_t)(tc_endofjob + 1u - 2u * (rnd() % 2u)); break;
    default: s->words[MPAC3] = tc_endofjob; break;
    }
}

static unsigned mission(const char *name, const agc_routine_table *readable,
                        const agc_routine_table *faithful, uint16_t tc_endofjob, uint16_t other)
{
    unsigned n, ran = 0, bad = 0;

    if (readable->count != 1u || faithful->count != 1u ||
        readable->entries[0].physical != faithful->entries[0].physical) {
        printf("FAIL %s: the tables are not one key, the same\n", name);
        return 1;
    }
    for (n = 0; n < STATES; ++n) {
        apollo_overlay_state start, fa, ra;
        enum exec_status sf, sr;
        const char *why = 0;

        state(&start, tc_endofjob, other);
        fa = ra = start;
        sf = faithful->entries[0].call(0, &fa, 0);
        sr = readable->entries[0].call(0, &ra, 0);
        ran += sf == EXEC_OK;
        if (sr != EXEC_OK && memcmp(&ra, &start, sizeof ra))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf != EXEC_OK && sr == EXEC_OK)
            why = "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        if (why && ++bad <= 3u)
            printf("FAIL %s state %u (Z %06o, MPAC +3 %06o): %s\n", name, n, start.words[LANEC_Z],
                   start.words[MPAC3], why);
    }
    if (ran < STATES / 2u || ran == STATES) {
        printf("FAIL %s: the faithful routine ran in %u of %u states\n", name, ran, STATES);
        ++bad;
    }
    printf("pin-request-cases %s: %u states, %u run by the faithful routine, %u failures\n", name,
           STATES, ran, bad);
    return bad;
}

int main(void)
{
    unsigned failures = 0;

    failures += mission("LM", &readable_pin_request_lm_routines, &lanec_pin_request_lm_routines,
                        LANEC_LM_ENDOFJOB, LANEC_CM_ENDOFJOB);
    failures += mission("CM", &readable_pin_request_cm_routines, &lanec_pin_request_cm_routines,
                        LANEC_CM_ENDOFJOB, LANEC_LM_ENDOFJOB);
    return failures != 0;
}
