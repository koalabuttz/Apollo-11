/* SPDX-License-Identifier: MIT */
#ifndef DSKY_ENTER_H
#define DSKY_ENTER_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
enum dsky_enter_route {DSKY_ENTER_PASS_ZERO,DSKY_ENTER_ALARM,DSKY_ENTER_REQUEST};
/* Actual CHARIN key034 or ENTER41,2002. CHARIN bank40/SUPER100;
 * ENTER bank41/SUPER100; both any retained aligned EB and IRQ0/1.
 * Complete pass selection, higher-pass decimal admission/MM special test,
 * complement REQRET and actual FLASHOFF. Stops at ENTPAS0, GODSPALM,
 * or physical REQRET1013 before its stored transfer executes. Both signed
 * zeros in REQRET/DSPCOUNT supported. No inferred job completion; caller
 * excludes producers. Unsupported inputs preserve image, IO and route. */
enum p66_bias_status dsky_enter(apollo_overlay_state *,dsky_charin_io *,enum dsky_enter_route *);
enum exec_status cm_enter(apollo_overlay_state *,cm_native_io *,uint16_t *,enum dsky_enter_route *);
#endif
