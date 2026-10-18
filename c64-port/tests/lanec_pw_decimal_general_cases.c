/* SPDX-License-Identifier: MIT */
/* pw-decimal-general: the general decimal formatter DSPDECVN/DSPRND through DSPIN (src/dsky_decimal_general.c).
 * The generator's lower half is LM, the upper half CM. The rounded or
 * unrounded entry is read back from Z; a caller inside the formatter's
 * own code moves out of it (lanec_pw_decimal_caller). */
#include "lanec_pw_a.h"
#include "dsky_decimal_general.h"
#include "dsky_decimal_general_cases.h"

#define CASES 2048u
#define EXTRA 8u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p; enum dsky_nv_mission mission; uint8_t rounded;
    uint32_t k = n < CASES ? n : (n - CASES) * 97u;
    lanec_fill(s, io, n);
    dsky_decimal_general_input(s, &p, (uint16_t)lanec_pw_spread(k, CASES, DSKY_DECIMAL_GENERAL_CASES), &mission, &rounded);
    /* past the spread: MPAC at or next to its largest magnitude, where
     * DSPRND's rounding carries out and it takes DPOSMAX instead */
    if (n >= CASES) {   /* the rounded entry: Luminary DSPDECWD, Comanche 40,3133 */
        s->words[5] = mission == DSKY_NV_LM ? 03226u : 03133u;
        s->words[0154] = 037777u; s->words[0155] = (uint16_t)(037777u - (n - CASES) % 2u * 3u);
    }
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_decimal_caller(s, mission);
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_decimal_general_run);
}
const lanec_suite lanec_suite_pw_decimal_general = { "pw-decimal-general", CASES + EXTRA, make, lanec_pw_decimal_general_run };
