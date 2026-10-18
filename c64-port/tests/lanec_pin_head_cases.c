/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_pin_head_cases.h"
#include "lanec_pw.h"

int lanec_pin_head_reach(uint32_t n, uint32_t tries, const lanec_suite *src, uint16_t bank, uint16_t point,
                         enum lanec_mission m, apollo_overlay_state *s, lanec_io *io)
{
    static apollo_overlay_state t;
    static lanec_meta scratch;
    lanec_io u;
    uint32_t k;
    uint16_t steps, stop = point >= 04000u ? point : (uint16_t)(bank * 1024u + point - 02000u);
    for (k = 0; k < tries; ++k) {
        memset(&scratch, 0, sizeof scratch);
        src->make((n * 977u + k) % src->count, s, io, &scratch);
        if (scratch.mission != m) continue;
        t = *s; u = *io;
        if (src->run(&t, &u, m) != LANEC_OK) continue;
        if (s->words[LANEC_Z] == point && (point >= 04000u || lanec_current_bank(s, io) == bank)) return 1;
        t = *s; u = *io;
        if (lanec_pin_chain(&t, &u, m, stop, &steps) == EXEC_OK && t.words[LANEC_Z] == point &&
            (point >= 04000u || lanec_current_bank(&t, &u) == bank)) {
            *s = t; *io = u;
            return 1;
        }
    }
    return 0;
}
