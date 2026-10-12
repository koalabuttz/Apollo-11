/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_EXTENDED_H
#define CM_WAITLIST_EXTENDED_H
#include "cm_native_image.h"
/* Actual WAITLIST/TWIDDLE through successful caller return or source
 * BAILOUT07755 / POODOO05721 entry. Alarm body is a later continuation.
 * task is an explicit compiled inline source literal, never rope fetched.
 * When WTLST5's last CCS sees +0 (later tasks due together), the source
 * executes the erasable word 01203 (TC 1203): CM_WL_SOURCE_OVERFLOW with the
 * image and io unchanged. */
enum cm_wl_status cm_waitlist_extended(apollo_overlay_state *,cm_native_io *,exec_address,uint8_t);
#endif
