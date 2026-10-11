/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LOAD_REQUEST_H
#define DSKY_LOAD_REQUEST_H
#include "dsky_noun_check.h"
#include "dsky_nv_update_general.h"
#include "dsky_address_request.h"
enum dsky_load_request_route {DSKY_LOAD_REQUEST_END,DSKY_LOAD_REQUEST_NVSUB,DSKY_LOAD_REQUEST_PASTE,DSKY_LOAD_REQUEST_ALARM,DSKY_LOAD_REQUEST_HOLE};
/* Original ABCLOAD/ABLOAD/ALOAD/BLOAD/CLOAD and three next-component
 * request continuations through actual noun/component checks, verb update,
 * five-character blanking and FLASHON to source selected ENTRET target.
 * Canonical ordinary RAM/rawCPU, anyEB/selecting SUPER/IRQ; noun selectors
 * only when read, supported compiled ENTRET only when reached. No data
 * store/scaling or scheduler body yet. Unsupported is atomic; exclusion
 * belongs to caller. No opcode fetch/extra scheduling poll. */
enum p66_bias_status dsky_load_request(apollo_overlay_state *,dsky_lamp_io *,enum dsky_load_request_route *);
enum exec_status cm_load_request(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_load_request_route *);
#endif
