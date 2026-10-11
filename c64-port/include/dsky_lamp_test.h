/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LAMP_TEST_H
#define DSKY_LAMP_TEST_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
typedef struct {
 p66_bias_io control;
 uint16_t channel11,channel12,channel13,wrote_channels,restart_light;
} dsky_lamp_io;
enum dsky_lamp_route {DSKY_LAMP_WAIT,DSKY_LAMP_ADMIT,DSKY_LAMP_MODE,DSKY_LAMP_RELEASE,DSKY_LAMP_PIN,DSKY_LAMP_CALLER};
/* Original VBTSTLTS, TSTLTS1/2/3 and post-DSPMM controls, plus actual CM
 * CHKPOOH. Canonical source image/any EB/four selecting SUPER latches,
 * producer exclusion; ordinary readable channel13 excludes trap bits034000.
 * Direct TSTLTS1 is source-produced counter0..10. Direct CM CHKPOOH zero
 * modes require ordinary Q0100..7777; nonzero mode does not consume Q.
 * Stops at genuine WAITLIST/NOVAC/DSPMM/TSTLTS4/PINBRNCH/caller entries.
 * These entry controls do not claim the deferred job/task consumers. */
enum p66_bias_status dsky_lamp_test(apollo_overlay_state *,dsky_lamp_io *,enum dsky_lamp_route *);
enum exec_status cm_lamp_test(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_lamp_route *);
#endif
