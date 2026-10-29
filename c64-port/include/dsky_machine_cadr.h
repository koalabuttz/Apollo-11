/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MACHINE_CADR_H
#define DSKY_MACHINE_CADR_H
#include "dsky_octal_display.h"
enum dsky_machine_route {DSKY_MACHINE_VERB_FAN,DSKY_MACHINE_CCS_HOLE};
/* Actual INTMCTBS41,2116 or INTMCTBS+2 through VERBFAN41,2133 or real
 * selected relay CCSHOLE. Canonical image/rawCPU; any EB, physical bank41,
 * SUPER100/120/140/160, IRQ0/1. Holds restoring producer exclusion.
 * Unsupported image, external IO and route unchanged. */
enum p66_bias_status dsky_machine_cadr(apollo_overlay_state *,p66_bias_io *,enum dsky_machine_route *);
enum exec_status cm_machine_cadr(apollo_overlay_state *,cm_native_io *,enum dsky_machine_route *);
#endif
