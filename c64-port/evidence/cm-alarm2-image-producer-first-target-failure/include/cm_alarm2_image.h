/* SPDX-License-Identifier: MIT */
#ifndef CM_ALARM2_IMAGE_H
#define CM_ALARM2_IMAGE_H
#include "cm_alarm_image.h"
/* Actual self-check ERRORS43,3261 producer and direct ALARM25652 entry.
 * The surviving incomingA is stored to ALMCADR before the inline code read.
 * Fixed words are source data only; caller owns exclusion across slot swaps. */
enum exec_status cm_alarm2_producer(apollo_overlay_state *,cm_native_io *);
bool cm_alarm2_fixed_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *);
enum exec_status cm_alarm2_image(apollo_overlay_state *,cm_native_io *,agc_word);
#endif
