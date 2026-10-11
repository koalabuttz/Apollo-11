/* SPDX-License-Identifier: MIT */
#ifndef CM_NORMAL_DISPLAY_H
#define CM_NORMAL_DISPLAY_H
#include "cm_native_image.h"
/* Actual normal verb6 mixed-noun40/85 source caller stages0GOPLAY,
 *1NVMONOPT,2UPDATNN,3ENTPAS0,4DSPDCPUT table selection,5normal completion.
 * Normal admission uses active basic core0/priority023000, ordinary
 * source display fields and no external monitor/blocked display.
 * Data tables are immutable original Comanche words; N40 uses M/S then
 * two DP3 rows, N85 uses three DP3 rows. Caller owns canonical exclusion. */
uint8_t cm_normal_display_caller(apollo_overlay_state *,cm_native_io *,uint8_t);
/* Linked source routes0..29 for N40,0..25 for N85; no pending NEWJOB,
 * normal unblocked display/no external monitor, otherwise source-selected
 * callee failures are native faults. No frame survives an overlay reload. */
uint8_t cm_normal_display_step(apollo_overlay_state *,cm_native_io *,uint8_t);
#endif
