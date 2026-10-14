/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_pin_head.h"

#define FAMILIES 8u
#define FOUND 4096u
typedef struct { uint8_t source; uint16_t n; } found_case;
static struct { const lanec_pin_head_family *family; uint16_t count; found_case found[FOUND]; } cache[FAMILIES];
static apollo_overlay_state t;

static int at_point(const lanec_pin_head_family *f, const apollo_overlay_state *s, const lanec_io *io,
                    enum lanec_mission mission)
{
    uint16_t z = s->words[LANEC_Z], bank = z >= 02000u && z < 04000u ? lanec_current_bank(s, io) : LANEC_ANY_BANK, i;
    for (i = 0; i < f->point_count; ++i)
        if (f->points[i].mission == mission && f->points[i].z == z && f->points[i].bank == bank) return 1;
    return 0;
}

/* The source cases at the family's points that their own module accepts,
 * found once, in source and case order. */
static unsigned scan(const lanec_pin_head_family *f)
{
    unsigned k, source;
    uint32_t n;
    lanec_io io;
    lanec_meta m;
    for (k = 0; k < FAMILIES && cache[k].family && cache[k].family != f; ++k) {}
    if (k == FAMILIES) return FAMILIES;
    if (cache[k].family) return k;
    cache[k].family = f;
    for (source = 0; f->sources[source]; ++source)
        for (n = 0; n < f->sources[source]->count && cache[k].count < FOUND; ++n) {
            memset(&m, 0, sizeof m);
            f->sources[source]->make(n, &t, &io, &m);
            if (!at_point(f, &t, &io, m.mission) || f->sources[source]->run(&t, &io, m.mission) != LANEC_OK) continue;
            cache[k].found[cache[k].count].source = (uint8_t)source;
            cache[k].found[cache[k].count].n = (uint16_t)n;
            ++cache[k].count;
        }
    return k;
}

void lanec_pin_head_make(const lanec_pin_head_family *f, uint32_t n, apollo_overlay_state *s, lanec_io *io,
                         lanec_meta *m, uint32_t *round)
{
    unsigned k = scan(f);
    const found_case *c;
    enum lanec_mission mission;
    memset(m, 0, sizeof *m);
    if (k == FAMILIES || !cache[k].count) {     /* no source case: an empty image the head refuses */
        memset(s, 0, sizeof *s); memset(io, 0, sizeof *io);
        *round = 0;
        return;
    }
    c = &cache[k].found[n % cache[k].count];
    *round = n / cache[k].count;
    f->sources[c->source]->make(c->n, s, io, m);
    mission = m->mission;
    memset(m, 0, sizeof *m);       /* the caller sets the stop where the head ends */
    m->mission = mission;
}
