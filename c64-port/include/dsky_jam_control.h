/* SPDX-License-Identifier: MIT */
#ifndef DSKY_JAM_CONTROL_H
#define DSKY_JAM_CONTROL_H
#include "dsky_load_control.h"
/* Actual fixed-fixed JAMPROC/JAMTERM through source SUPER write, request
 * setup, POSTJUMP and complete stable VBPROC/VBTERM→RECALTST→END.
 * Any FB/EB, eight SUPER, IRQ0/1; selected wake source domains only.
 * Explicit source channel7 write event. Unsupported image/IO/events unchanged. */
enum p66_bias_status dsky_jam_control(apollo_overlay_state *,dsky_charin_io *,uint16_t *wrote_superbank);
enum exec_status cm_jam_control(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,uint16_t *wrote_superbank);
#endif
