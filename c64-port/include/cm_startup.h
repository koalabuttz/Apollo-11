/* SPDX-License-Identifier: MIT */
#ifndef CM_STARTUP_H
#define CM_STARTUP_H
#include "cm_native_image.h"
typedef struct { agc_word channel5,channel6; } cm_startup_ports;
enum cm_startup_entry { CM_STARTUP_DOFSTART,CM_STARTUP_SLAP1,
 CM_STARTUP_MR_KLEAN,CM_STARTUP_P00_KLEAN,CM_STARTUP_V37_KLEAN };
enum cm_startup_status { CM_STARTUP_OK,CM_STARTUP_OUT_OF_SCOPE };
/* Original CM initialization to ENDRSTRT, or cleanup through TC Q return.
 * Existing sole canonical image, external I/O metadata and source exclusion.
 * Physical bank5/EB3 is required. This does not load or execute later flight
 * programs. Z is caller-owned, not a native instruction counter. */
enum cm_startup_status cm_startup_begin(apollo_overlay_state *,cm_native_io *,cm_startup_ports *,enum cm_startup_entry);
#endif
