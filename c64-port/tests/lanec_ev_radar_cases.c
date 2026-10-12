/* SPDX-License-Identifier: MIT */
/* Cases for the Luminary099 radar test verbs (R04, R77, R77END): every
 * entry, R04FLAG set and clear, each RTSTDEX radar selection, the channel 33
 * radar-power and LR position bits, and random EB at the later entries.
 * R04X -1 and R04C +4 store their caller's A, which never overflows. */
#include "lanec_suites.h"
#include "lanec_ev_radar.h"
#include "lanec_symbols.h"

#define ENTRIES 51u

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, count, z, eb4;
    const uint16_t *entry = lanec_ev_radar_entries(&count);
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES;
    lanec_fill(s, io, n);
    m->mission = LANEC_LM;
    z = entry[n % count];
    lanec_enter(s, io, 043u, z);
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    w[LANEC_LM_FLAGWRD3] = (uint16_t)(v & 1u ? w[LANEC_LM_FLAGWRD3] | LANEC_LM_R04FLBIT_WORD
                                             : w[LANEC_LM_FLAGWRD3] & ~LANEC_LM_R04FLBIT_WORD);
    /* RTSTDEX (E4, and under the current EB) selects RR (bits 2-3 clear) or LR */
    eb4 = (uint16_t)(04u * 0400u + (LANEC_LM_RTSTDEX & 0377u));
    if ((v >> 1) & 1u) w[eb4] &= (uint16_t)~06u;
    if ((v >> 1) & 1u) w[lanec_erasable(s, LANEC_LM_RTSTDEX)] &= (uint16_t)~06u;
    if ((v >> 2) & 1u) io->channel[033] &= (uint16_t)~02u;
    if ((v >> 3) & 1u) w[LANEC_LM_OPTIONX + 1u] &= (uint16_t)~07u;
    if (z == LANEC_LM_R04X - 1u || z == LANEC_LM_R04C + 4u) w[LANEC_A] = lanec_a16(w[LANEC_A]);
    m->ranges = 1;
    m->range[0].bank = 043u; m->range[0].lo = LANEC_LM_R77; m->range[0].hi = LANEC_LM_R77END + 11u;
}

const lanec_suite lanec_suite_ev_radar = { "ev-radar", ENTRIES * 48u, make, lanec_ev_radar_dispatch };
