/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_FLIGHT_CHECK_H
#define RESIDENT_FLIGHT_CHECK_H
/* The flight check's case modules (lane A numbers 0xf1-0xf6, test stores
 * only) and the dispatcher probe's resolver record. */
#include "overlay_v2.h"
#define RK_MODULE_SERVICES APOLLO_OV2_ID(APOLLO_LANE_A,0xf1)
#define RK_MODULE_JOBS(part) APOLLO_OV2_ID(APOLLO_LANE_A,0xf1+(part))   /* part 1..3 */
#define RK_MODULE_IDLE APOLLO_OV2_ID(APOLLO_LANE_A,0xf5)
#define RK_MODULE_RESTART APOLLO_OV2_ID(APOLLO_LANE_A,0xf6)   /* LM, task slot */
#define RK_PROBE_FB 024000u
#define RK_PROBE_LOC 02000u
#define RK_PROBE_Z 02001u
#define RK_PROBE_VALUE 0x5aa5u
#endif
