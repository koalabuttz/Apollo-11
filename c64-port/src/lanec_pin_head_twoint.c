/* SPDX-License-Identifier: MIT */
/* Pinball heads in 2INTOUT (bank 40), Luminary099 and Comanche055: the
 * words where 5BLANK, +ON, DSPDECVN and READLO return, each run to the next
 * registered point as the two-integer display module does it. See
 * docs/LANEC-PIN-HEADS.md. */
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"
#include "dsky_two_integer.h"

/* 2INTOUT +1 (TC +ON), +2 (CA MPAC), +4 (CS THREE) and +013 (TC POSTJUMP),
 * the module's sign, first, second and return stages. */
static const uint16_t points[2][4] = {
    { LANEC_LM_2INTOUT + 1u, LANEC_LM_2INTOUT + 2u, LANEC_LM_2INTOUT + 4u, LANEC_LM_2INTOUT + 013u },
    { LANEC_CM_2INTOUT + 1u, LANEC_CM_2INTOUT + 2u, LANEC_CM_2INTOUT + 4u, LANEC_CM_2INTOUT + 013u } };
typedef char in_bank_40[(LANEC_LM_2INTOUT_BANK == 040u && LANEC_CM_2INTOUT_BANK == 040u) ? 1 : -1];

enum lanec_status lanec_pin_head_twoint_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    p66_bias_io p;
    unsigned k, i;
    for (k = 0; k < 4u && s->words[LANEC_Z] != points[mission][k]; ++k) {}
    if (k == 4u || lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
    for (i = 3; i < APOLLO_SHARED_WORDS; ++i)   /* every word in its 15-bit memory form */
        if (s->words[i] > AGC_MASK) return LANEC_OUT_OF_SCOPE;
    p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (dsky_two_integer(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM,
                         (enum dsky_two_integer_stage)(DSKY_TWO_INTEGER_SIGN + k)) != P66_BIAS_OK)
        return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
