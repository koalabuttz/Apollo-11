/* SPDX-License-Identifier: MIT */
/* Cases for CHARIN and its key labels (src/lanec_pin_keys.c), drawn from the
 * CHARIN states of the earlier key suites (pw-charin-*, pw-cm-charin-*,
 * pw-enter), so every key code, both missions and the generators' domains
 * are covered. Odd cases start at the key's label, where CHARIN's dispatch
 * leaves the state; even cases start at CHARIN. Every fourth pair replaces
 * CADRSTOR with +0, -0 or another full value, so all four CCS branches and
 * the KEY REL light run for every key. */
#include <string.h>
#include "lanec_pw.h"
#include "lanec_pin_keys.h"
#include "lanec_symbols.h"

#define SOURCES 11u
#define PER 256u    /* pairs per source */
static const lanec_suite *const source[SOURCES] = {
    &lanec_suite_pw_charin_alarm, &lanec_suite_pw_charin_clear, &lanec_suite_pw_charin_number,
    &lanec_suite_pw_charin_nv, &lanec_suite_pw_charin_release, &lanec_suite_pw_charin_reset,
    &lanec_suite_pw_charin_sign, &lanec_suite_pw_cm_charin_number, &lanec_suite_pw_cm_charin_nv,
    &lanec_suite_pw_enter, &lanec_suite_pw_charin_alarm };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    const lanec_suite *from = source[(n / 2u) % SOURCES];
    uint32_t pair = n / 2u / SOURCES, k = pair * (from->count / PER), r = n * 2654435761u + 3u;
    uint16_t *w = s->words, charin;
    unsigned tries;
    for (tries = 0; tries < 64u; ++tries, k = (k + 1u) % from->count) {
        from->make(k, s, io, m);
        charin = m->mission == LANEC_LM ? LANEC_LM_CHARIN : LANEC_CM_CHARIN;
        if (w[LANEC_Z] == charin) break;
    }
    { enum lanec_mission mission = m->mission; memset(m, 0, sizeof *m); m->mission = mission; }
    if ((n / 2u) % SOURCES == SOURCES - 1u)     /* the alarm codes 12-17, 23-30, 35 */
        w[LANEC_LM_MPAC] = (uint16_t)(pair % 3u == 0u ? 012u + pair % 6u : pair % 3u == 1u ? 023u + pair % 6u : 035u);
    switch (pair % 8u) {
    case 1: w[LANEC_LM_CADRSTOR] = 0u; break;
    case 3: w[LANEC_LM_CADRSTOR] = 077777u; break;
    case 5: w[LANEC_LM_CADRSTOR] = (uint16_t)(1u + lanec_rand(&r) % 077776u); break;
    default: break;
    }
    if (n & 1u) (void)lanec_pin_keys_charin(s, io, m->mission);
    lanec_pw_exit_stop(m, s, io, lanec_pin_keys_dispatch);
}

const lanec_suite lanec_suite_pin_keys = { "pin-keys", 2u * SOURCES * PER, make, lanec_pin_keys_dispatch };
