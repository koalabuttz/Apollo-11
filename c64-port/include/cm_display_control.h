/* SPDX-License-Identifier: MIT */
#ifndef CM_DISPLAY_CONTROL_H
#define CM_DISPLAY_CONTROL_H
#include "cm_native_image.h"
enum cm_display_control_status {CM_DISPLAY_CONTROL_OK=0,CM_DISPLAY_CONTROL_UNSUPPORTED=1};
/* Original REGODSP10,2707/CLOCPLAY10,2715/GOFLASH10,2720 through SAVELOCS
 * to actual MAKEPLAY10,2770. Any valid EB and SUPER field are retained.
 * A is an ordinary sign-extended input word; BUF2 holds an ordinary saved
 * basic caller location0..07777 and aligned fixed bank0..076000. Other
 * non-register erasable inputs are ordinary15-bit caller words. Producer
 * exclusion is caller-owned; unsupported inputs preserve image and io. */
enum cm_display_control_status cm_display_control_enter(apollo_overlay_state *,cm_native_io *);
#endif
