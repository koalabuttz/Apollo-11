/* SPDX-License-Identifier: MIT */
#ifndef CM_DELAY_IMAGE_H
#define CM_DELAY_IMAGE_H
#include "cm_waitlist_image.h"
/* Actual FIXDELAY/VARDELAY insertion through TASKOVER. fixed=1 uses the
 * compiled source literal, then queues Q+1. fixed=0 uses A and queues Q.
 * Source caller must be under WAITLIST control with interrupts inhibited.
 * Successful bounded insertion has the same nonfull-list obligation as
 * cm_waitlist_image; no C frame survives the delayed continuation. */
enum cm_wl_status cm_delay_image(apollo_overlay_state *,cm_native_io *,agc_word,uint8_t);
#endif
