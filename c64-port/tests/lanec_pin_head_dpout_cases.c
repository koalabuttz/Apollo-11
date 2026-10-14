/* SPDX-License-Identifier: MIT */
/* pin-head-dpout: Pinball DP1OUTSF, DP2OUTSF and DP3OUTSF (bank 40), both
 * missions (src/lanec_pin_head_dpout.c). The scale modules' own states and
 * the Comanche display consumers' at DP3OUTSF, each under every superbank
 * that selects bank 40 (0100, 0120, 0140, 0160): the decimal and monitor
 * consumers reach these words under all four. */
#include "lanec_pin_head.h"
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"

#define CASES 8192u
static const lanec_suite *const sources[] = { &lanec_suite_pw_scale, &lanec_suite_pw_cm_scale, &lanec_suite_pw_cm_dynamic_display,
                                              &lanec_suite_pw_cm_normal_display, &lanec_suite_pw_cm_nv85, 0 };
static const lanec_pin_head_point points[] = {
    { LANEC_LM, 040u, LANEC_LM_DP1OUTSF }, { LANEC_LM, 040u, LANEC_LM_DP2OUTSF }, { LANEC_LM, 040u, LANEC_LM_DP3OUTSF },
    { LANEC_CM, 040u, LANEC_CM_DP1OUTSF }, { LANEC_CM, 040u, LANEC_CM_DP2OUTSF }, { LANEC_CM, 040u, LANEC_CM_DP3OUTSF } };
static const lanec_pin_head_family family = { sources, points, 6u, lanec_pin_head_dpout_run };

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t round;
    lanec_pin_head_make(&family, n, s, io, m, &round);
    if (io->superbank & 0100u) io->superbank = (uint16_t)(0100u | (round % 4u) * 020u);
    lanec_pw_exit_stop(m, s, io, family.run);
}
const lanec_suite lanec_suite_pin_head_dpout = { "pin-head-dpout", CASES, make, lanec_pin_head_dpout_run };
