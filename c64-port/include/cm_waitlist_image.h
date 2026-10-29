/* SPDX-License-Identifier: MIT */
#ifndef CM_WAITLIST_IMAGE_H
#define CM_WAITLIST_IMAGE_H
#include "cm_native_image.h"
/* Original successful WAITLIST through actual DTCB return. A carries positive
 * bounded delay, Q points to the actual caller's source inline 2CADR. Task is
 * its compiled source-resolved pair, never fetched from an executable rope.
 * Valid nonfull source queue, ordinary banks and exclusion are obligations. */
enum cm_wl_status cm_waitlist_image(apollo_overlay_state *,cm_native_io *,exec_address);
#endif
