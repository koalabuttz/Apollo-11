/* SPDX-License-Identifier: MIT */
#ifndef GIMBAL_STAGE_H
#define GIMBAL_STAGE_H
#include "overlay_runtime.h"
#include "fixed_data.h"
typedef struct {uint16_t superbank,interrupt_enabled;} gimbal_io;
/* Source-exact WCALC entry stage through external DELCOMP entry. The external
 * matrix routine and later file stages are not completed by this API.
 * Callers exclude all producers; one authoritative canonical RAM image.
 * Entry is original NEWOPS with LOC=NOGIMLOC/WCALC, source bank22, EDOP0,
 * NEWJOB0 and matching BANKSET/BB/INTBIT15; active source FIXLOC VAC base.
 * Distinct owned image/io objects; ordinary words15-bit, A/L/Q raw16-bit;
 * latch0..160 by020. Unsupported contexts return zero unchanged. */
uint16_t gimbal_entry(apollo_overlay_state *,gimbal_io *,enum apollo_fixed_mission);
/* Exactly40 immutable file-data words at original physical fixed addresses;
 * missing/invalid/null outputs return zero and preserve caller output. */
uint16_t gimbal_fixed_read(enum apollo_fixed_mission,uint16_t,uint16_t *);
#endif
