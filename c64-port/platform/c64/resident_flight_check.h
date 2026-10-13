/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_FLIGHT_CHECK_H
#define RESIDENT_FLIGHT_CHECK_H
/* The flight check's case modules (lane A numbers 0xf1-0xf7, test stores
 * only) and the dispatcher probe's resolver record. */
#include "overlay_v2.h"
#include "agc_transfer.h"
#include "resident_rupt_cases.h"
/* What the data module (resident_flight_data.c) hands the check. */
typedef struct {
    const uint16_t *cases;            /* flat case numbers (tests/<m>_kernel_c64_rows.h) */
    const uint32_t *digests;          /* their frozen yaAGC digests */
    uint16_t rows;
    const agc_routine_table *restart; /* LM: RESTARTS' routines; CM: none */
} rk_check_data;
#define RK_MODULE_SERVICES APOLLO_OV2_ID(APOLLO_LANE_A,0xf1)
#define RK_MODULE_JOBS(part) APOLLO_OV2_ID(APOLLO_LANE_A,0xf1+(part))   /* part 1..3 */
#define RK_MODULE_IDLE APOLLO_OV2_ID(APOLLO_LANE_A,0xf5)
#define RK_MODULE_DATA APOLLO_OV2_ID(APOLLO_LANE_A,0xf6)   /* task slot */
#define RK_MODULE_RUPT APOLLO_OV2_ID(APOLLO_LANE_A,0xf7)   /* T3RUPT cases */
/* What the T3RUPT case module (resident_flight_rupt_cases.c) fills. */
typedef struct { rr_case c; uint16_t n; } rk_rupt_row;
#define RK_PROBE_FB 024000u
#define RK_PROBE_LOC 02000u
#define RK_PROBE_Z 02001u
#define RK_PROBE_VALUE 0x5aa5u
/* The same probe at fixed-fixed 07753, under its canonical bank 06000. */
#define RK_PROBE_FF_LOC 07753u
/* SPSIN's address (Luminary099 05033, Comanche055 05075) and a return. */
#if RK_LM
#define RK_SPSIN 05033u
#else
#define RK_SPSIN 05075u
#endif
#define RK_SP_RETURN 02345u
#endif
