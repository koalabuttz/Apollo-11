/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_ALARM_H
#define DSKY_CHARIN_ALARM_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
/* Actual CHARIN invalid-key fanout and octal8/9 rejection to CHARALRM/FALTON
 * or genuine negative-DSPCOUNT blocked ENDOFJOB. No mutating unsupported keys.
 * Bank40/anyEB/SUPER100; one canonical image, caller excludes producers. */
enum p66_bias_status dsky_charin_alarm(apollo_overlay_state *,dsky_charin_io *);
enum exec_status cm_charin_alarm(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
