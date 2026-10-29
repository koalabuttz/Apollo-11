/* SPDX-License-Identifier: MIT */
/* ev-refuse: inputs outside the extended-verb modules' domain. Each case
 * takes an accepted case of one module's suite (all eight, both missions
 * where the module has them) and breaks the entry contract every module
 * checks first: BB not FB|EB>>8, the ZERO register nonzero, or FB selecting
 * bank 7, where the file has no code. The module must return
 * LANEC_OUT_OF_SCOPE with the image and I/O unchanged (the native runner
 * prints status 2 otherwise); diff.py --refusals-only. */
#include "lanec_suites.h"
#include "lanec_ev_front.h"
#include "lanec_ev_short.h"
#include "lanec_ev_align.h"
#include "lanec_ev_radar.h"
#include "lanec_ev_dap.h"
#include "lanec_ev_time.h"
#include "lanec_ev_track.h"
#include "lanec_ev_lmi.h"

#define MODULES 8u
static const lanec_suite *const modules[MODULES] = { &lanec_suite_ev_front, &lanec_suite_ev_short,
    &lanec_suite_ev_align, &lanec_suite_ev_radar, &lanec_suite_ev_dap, &lanec_suite_ev_time,
    &lanec_suite_ev_track, &lanec_suite_ev_lmi };
static uint8_t module;   /* the module of the case make() built last */

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t i = n / MODULES, inner;
    uint16_t *w = s->words, z;
    module = (uint8_t)(n % MODULES);
    inner = (i * 2654435761u + module) % modules[module]->count;
    modules[module]->make(inner, s, io, m);
    z = w[LANEC_Z];
    switch (i % 3u) {
    case 0: w[LANEC_BB] ^= 1u; break;                              /* BB disagrees with EB */
    case 1: w[7] = (uint16_t)(1u + i % 077776u); break;            /* ZERO nonzero */
    default:
        if (z >= 02000u && z < 04000u) lanec_enter(s, io, 07u, z);   /* bank 7 */
        else w[LANEC_BB] ^= 1u;
        break;
    }
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    switch (module) {
    case 0: return lanec_ev_front_dispatch(s, io, m);
    case 1: return lanec_ev_short_dispatch(s, io, m);
    case 2: return lanec_ev_align_dispatch(s, io, m);
    case 3: return lanec_ev_radar_dispatch(s, io, m);
    case 4: return lanec_ev_dap_dispatch(s, io, m);
    case 5: return lanec_ev_time_dispatch(s, io, m);
    case 6: return lanec_ev_track_dispatch(s, io, m);
    default: return lanec_ev_lmi_dispatch(s, io, m);
    }
}

const lanec_suite lanec_suite_ev_refuse = { "ev-refuse", MODULES * 64u, make, run };
