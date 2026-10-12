/* SPDX-License-Identifier: MIT */
/* di-refuse: inputs outside the display-interface modules' domain. Each case
 * takes an accepted case of one module's suite (all six, both missions) and
 * breaks the entry contract every module checks first: BB not FB|EB>>8, the
 * ZERO register nonzero, or (for a bank-10 entry) FB selecting bank 11. The
 * module must return LANEC_OUT_OF_SCOPE with the image and I/O unchanged
 * (the native runner prints status 2 otherwise); diff.py --refusals-only.
 * Refusals inside a module's own domain checks are in di-idle (COPINDEX and
 * COPMPAC above 2) and di-nvdsp (COPINDEX above 2). */
#include "lanec_suites.h"
#include "lanec_di_dsp.h"
#include "lanec_di_mark.h"
#include "lanec_di_play.h"
#include "lanec_di_sleep.h"
#include "lanec_di_idle.h"
#include "lanec_di_nvdsp.h"

static const lanec_suite *const modules[6] = { &lanec_suite_di_dsp, &lanec_suite_di_mark, &lanec_suite_di_play,
                                               &lanec_suite_di_sleep, &lanec_suite_di_idle, &lanec_suite_di_nvdsp };
static uint8_t module;   /* the module of the case make() built last */

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t i = n / 6u, inner;
    uint16_t *w = s->words, z;
    module = (uint8_t)(n % 6u);
    inner = (i * 2654435761u + module) % modules[module]->count;
    modules[module]->make(inner, s, io, m);
    z = w[LANEC_Z];
    switch (i % 3u) {
    case 0: w[LANEC_BB] ^= 1u; break;                              /* BB disagrees with EB */
    case 1: w[7] = (uint16_t)(1u + i % 077776u); break;            /* ZERO nonzero */
    default:
        if (z >= 02000u && z < 04000u) lanec_enter(s, io, 011u, z);  /* bank 11, not 10 */
        else w[LANEC_BB] ^= 1u;
        break;
    }
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    switch (module) {
    case 0: return lanec_di_dsp_dispatch(s, io, m);
    case 1: return lanec_di_mark_dispatch(s, io, m);
    case 2: return lanec_di_play_dispatch(s, io, m);
    case 3: return lanec_di_sleep_dispatch(s, io, m);
    case 4: return lanec_di_idle_dispatch(s, io, m);
    default: return lanec_di_nvdsp_dispatch(s, io, m);
    }
}

const lanec_suite lanec_suite_di_refuse = { "di-refuse", 6u * 64u, make, run };
