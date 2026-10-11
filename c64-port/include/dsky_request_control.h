/* SPDX-License-Identifier: MIT */
#ifndef DSKY_REQUEST_CONTROL_H
#define DSKY_REQUEST_CONTROL_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
enum dsky_request_control_route {DSKY_REQUEST_RELEASE,DSKY_REQUEST_NOVAC,DSKY_REQUEST_FINDVAC,DSKY_REQUEST_WAITLIST};
/* Actual VBRQEXEC/REQEX1/REQUESTC/SETVAC/VBRQWAIT/ENDRQWT prefix through
 * literal RELDSP, or source3500 after release through literal native
 * NOVAC/FINDVAC/WAITLIST entry. Canonical image, rawCPU, any retainedEB,
 * bank41/four selectingSUPER/IRQ. Service words are exact source stores,
 * not an instruction interpreter. Caller retains producer exclusion. */
enum p66_bias_status dsky_request_control(apollo_overlay_state *,dsky_charin_io *,enum dsky_request_control_route *);
enum exec_status cm_request_control(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_request_control_route *);
#endif
