/* SPDX-License-Identifier: MIT */
/* Pinball heads in M/SOUT (bank 42), Luminary099 and Comanche055: the words
 * where SEPSEC, DSPDC2NR, DSPIN and SEPMIN return, each run to the next
 * registered point as the minutes-seconds display module does it. See
 * docs/LANEC-PIN-HEADS-DISPLAY.md. */
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"
#include "dsky_minutes_output.h"

/* M/SNORM +1 (CAF HISECON), the seconds display's BANKCALL, CAF ZERO after
 * it, TC SEPMIN and the minutes display's BANKCALL: the module's stages
 * after the fresh READLO. */
static const uint16_t points[2][5] = { { 03343u, 03347u, 03351u, 03361u, 03371u },
                                       { 03256u, 03262u, 03264u, 03274u, 03304u } };
typedef char in_bank_42[(LANEC_LM_M_S_SOUT == 03303u && LANEC_CM_M_S_SOUT == 03216u &&
                         LANEC_LM_M_S_SOUT_BANK == 042u && LANEC_CM_M_S_SOUT_BANK == 042u) ? 1 : -1];

enum lanec_status lanec_pin_head_minsec_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    unsigned k, i;
    for (k = 0; k < 5u && s->words[LANEC_Z] != points[mission][k]; ++k) {}
    if (k == 5u || lanec_current_bank(s, io) != 042u) return LANEC_OUT_OF_SCOPE;
    for (i = 3; i < APOLLO_SHARED_WORDS; ++i)   /* every word in its 15-bit memory form */
        if (s->words[i] > AGC_MASK) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_minutes_output(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM,
                            (enum dsky_minutes_stage)(DSKY_MINUTES_SECONDS + k)) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
