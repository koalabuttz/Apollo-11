/* SPDX-License-Identifier: MIT */
/* Pinball heads at DP1OUTSF, DP2OUTSF and DP3OUTSF (bank 40), Luminary099
 * and Comanche055: the double-precision scale-factor routines, from their
 * entry through DPOUT, READLO, TPAGREE and DMP to SCOUTEND's POSTJUMP to
 * DSPDCEND. See docs/LANEC-PIN-HEADS-DISPLAY.md. */
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"
#include "dsky_scale.h"
#include "cm_scale.h"

/* The scale modules run with the superbank of their own suites, 0100 (bank
 * 40 selected as FB 30). The code reads no other superbank bit: READLO
 * reads erasable and the scaling is arithmetic, so any superbank that
 * selects bank 40 (0100 with the unused bits 020 and 040 in any state)
 * gives the same result and stays as it was. */
enum lanec_status lanec_pin_head_dpout_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    uint16_t z = s->words[LANEC_Z];
    enum dsky_scale_entry entry;
    if (io->superbank > 0160u || (io->superbank & 017u) || !(io->superbank & 0100u) || io->interrupt_enabled > 1u ||
        lanec_current_bank(s, io) != 040u)
        return LANEC_OUT_OF_SCOPE;
    if (mission == LANEC_LM) {
        p66_bias_io p;
        if (z == LANEC_LM_DP1OUTSF) entry = DSKY_DP1OUT;
        else if (z == LANEC_LM_DP2OUTSF) entry = DSKY_DP2OUT;
        else if (z == LANEC_LM_DP3OUTSF) entry = DSKY_DP3OUT;
        else return LANEC_OUT_OF_SCOPE;
        p.superbank = 0100u; p.interrupt_enabled = io->interrupt_enabled;
        if (dsky_scale_output(s, &p, entry) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
        io->interrupt_enabled = p.interrupt_enabled;
        return LANEC_OK;
    }
    if (mission == LANEC_CM) {
        cm_native_io c = {0};
        if (z == LANEC_CM_DP1OUTSF) entry = DSKY_DP1OUT;
        else if (z == LANEC_CM_DP2OUTSF) entry = DSKY_DP2OUT;
        else if (z == LANEC_CM_DP3OUTSF) entry = DSKY_DP3OUT;
        else return LANEC_OUT_OF_SCOPE;
        c.superbank = 0100u; c.interrupt_enabled = io->interrupt_enabled;
        if (cm_scale_output(s, &c, entry) != EXEC_OK) return LANEC_OUT_OF_SCOPE;
        io->interrupt_enabled = c.interrupt_enabled;
        return LANEC_OK;
    }
    return LANEC_OUT_OF_SCOPE;
}
