/* SPDX-License-Identifier: MIT */
/* Pinball head at DSPDCEND +2 (41,2570), Luminary099 and Comanche055: the
 * decimal display's component loop, from where DSPDECWD returns to the next
 * registered point. See docs/LANEC-PIN-HEADS-DISPLAY.md. */
#include "lanec_pw_runs.h"
#include "lanec_symbols.h"

typedef char same_words[(LANEC_LM_DECOUNT == LANEC_CM_DECOUNT && LANEC_LM_DECOUNT == 0117u &&
    LANEC_LM_ENTEXIT == 0136u && LANEC_CM_ENTEXIT == 0136u &&
    LANEC_LM_DSPDCEND == 02566u && LANEC_CM_DSPDCEND == 02566u &&
    LANEC_LM_DSPDCEND_BANK == 041u && LANEC_CM_DSPDCEND_BANK == 041u &&
    LANEC_LM_DSPDCPUT == 02537u && LANEC_CM_DSPDCPUT == 02537u) ? 1 : -1];
#define DECOUNT 0117u
#define ENTEXIT 0136u
#define DSPDCPUT 02537u
#define LOOP 02570u                         /* DSPDCEND +2 */

/* +0 CCS DECOUNT, +1 TC +2, +2 TC ENTEXIT, +3 TS DECOUNT, +4 TC DSPDCPUT.
 * CCS leaves A = DABS(DECOUNT): positive goes on to TC +2 (TS DECOUNT, the
 * count less one, and DSPDCPUT for the next component), +0 to TC ENTEXIT,
 * negative to TS DECOUNT and -0 to TC DSPDCPUT. */
enum lanec_status lanec_pin_head_decount_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    uint16_t *w = s->words;
    (void)mission;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u || w[LANEC_Z] != LOOP) return LANEC_OUT_OF_SCOPE;
    switch (lanec_ccs(s, w[DECOUNT])) {
    case 0: case 2:                         /* positive (via TC +2) or negative: TS DECOUNT */
        (void)lanec_ts(s, DECOUNT);
        lanec_tc(s, DSPDCPUT, (uint16_t)(LOOP + 5u));
        break;
    case 1:                                 /* +0: the last component is out */
        lanec_tc(s, ENTEXIT, (uint16_t)(LOOP + 3u));
        break;
    default:                                /* -0 */
        lanec_tc(s, DSPDCPUT, (uint16_t)(LOOP + 5u));
        break;
    }
    return LANEC_OK;
}
