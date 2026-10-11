/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MAJOR_REQUEST_H
#define DSKY_MAJOR_REQUEST_H
#include "dsky_charin_nv.h"
#include "dsky_pinball.h"
#include "cm_native_image.h"
enum dsky_major_request_route {DSKY_MAJOR_REQUEST_END,DSKY_MAJOR_REQUEST_NVSUB_END,DSKY_MAJOR_REQUEST_PASTE,DSKY_MAJOR_REQUEST_ALMCYCLE,DSKY_MAJOR_REQUEST_V37};
/* Actual MMCHANG41,3430, operator return+1 or REQMM41,3452. Canonical
 * image/rawCPU, any EB/four selectingSUPER/IRQ. REQMM ordinaryQ0100..7777;
 * request branches source-owned ENDINST/NVSBENDL/MONBACK returns. Caller
 * holds restoring producer exclusion. V37 and ALMCYCLE are actual callees. */
enum p66_bias_status dsky_major_request(apollo_overlay_state *,dsky_charin_io *,enum dsky_major_request_route *);
enum exec_status cm_major_request(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_major_request_route *);
#endif
