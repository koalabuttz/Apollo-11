/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch f (include/lanec_pw_f_run.h). */
#include "lanec_pw_f_run.h"
#include "cm_charin_number.h"
#include "cm_charin_nv.h"
#include "cm_decimal_pair.h"
#include "cm_dynamic_display.h"
#include "cm_formatter.h"
#include "cm_minutes_seconds.h"
#include "cm_normal_display.h"
#include "cm_nv85.h"
#include "cm_pinball.h"
#include "dsky_pinball.h"

LANEC_PW_F_CM_RUN(lanec_pw_cm_charin_number_run, cm_charin_number(s, &c, &(uint16_t){ 0 }) == EXEC_OK)

LANEC_PW_F_CM_RUN(lanec_pw_cm_charin_nv_run, cm_charin_nv(s, &c, &(uint16_t){ 0 }) == EXEC_OK)

LANEC_PW_F_CM_RUN(lanec_pw_cm_decimal_pair_run, cm_decimal_pair(s, &c) == EXEC_OK)

/* Routes 11-14 and 18-21 run the same MXV code (Z 07303-07341) on the
 * matrix the interpretive caller names: LOC 03436 (erasable, routes 11-14) or
 * 03441 (the fixed one, routes 18-21); the module takes the matrix from the
 * route, so LOC picks the routes to try. */
static uint8_t dynamic_display_step(apollo_overlay_state *s, cm_native_io *c, uint8_t route)
{
    uint16_t loc = s->words[0164];
    if (route >= 11u && route <= 14u && loc == 03441u) return 0;
    if (route >= 18u && route <= 21u && loc == 03436u) return 0;
    return cm_dynamic_display_step(s, c, route);
}
LANEC_PW_F_CM_STEPS(lanec_pw_cm_dynamic_display_run, dynamic_display_step, 53u)

enum lanec_status lanec_pw_cm_formatter_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    cm_native_io c;
    uint16_t z = s->words[LANEC_Z];
    enum cm_formatter_status st;
    if (mission != LANEC_CM) return LANEC_OUT_OF_SCOPE;
    lanec_pw_f_to_cm(&c, io);
    if (z == 03213u) st = cm_two_digit(s, &c);
    else if (z == 03133u || z == 03167u) st = cm_decimal_display(s, &c, (uint8_t)(z == 03133u));
    else return LANEC_OUT_OF_SCOPE;
    if (st != CM_FORMATTER_OK) return LANEC_OUT_OF_SCOPE;
    lanec_pw_f_from_cm(io, &c);
    return LANEC_OK;
}

LANEC_PW_F_CM_STEPS(lanec_pw_cm_minutes_seconds_run, cm_minutes_seconds_phase, 6u)

LANEC_PW_F_CM_STEPS(lanec_pw_cm_normal_display_run, cm_normal_display_step, 30u)

LANEC_PW_F_CM_STEPS(lanec_pw_cm_nv85_run, cm_nv85_step, 26u)

enum lanec_status lanec_pw_cm_pinball_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{ LANEC_ONE_MISSION(mission);
    cm_native_io c;
    uint16_t z = s->words[LANEC_Z];
    enum dsky_pinball_entry e = z == 03227u ? DSKY_DSPIN : z == 03311u ? DSKY_11DSPIN : DSKY_2BLANK;
    if (mission != LANEC_CM) return LANEC_OUT_OF_SCOPE;
    lanec_pw_f_to_cm(&c, io);
    if (cm_pinball_write(s, &c, e) != CM_PINBALL_OK) return LANEC_OUT_OF_SCOPE;
    lanec_pw_f_from_cm(io, &c);
    return LANEC_OK;
}

/* DP1OUTSF, DP2OUTSF or DP3OUTSF, by Z (src/cm_scale.c checks it). */
#include "cm_scale.h"
LANEC_PW_F_CM_RUN(lanec_pw_cm_scale_run, cm_scale_output(s, &c, s->words[LANEC_Z] == 02614u ? DSKY_DP1OUT
                  : s->words[LANEC_Z] == 02621u ? DSKY_DP2OUT : DSKY_DP3OUT) == EXEC_OK)
