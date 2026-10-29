/* SPDX-License-Identifier: MIT */
#ifndef CM_MAKEPLAY_SLEEP_H
#define CM_MAKEPLAY_SLEEP_H
#include "cm_native_image.h"
enum cm_display_sleep_boundary {CM_DISPLAY_SLEEP_WAKE,CM_DISPLAY_SLEEP_EXECUTIVE,CM_DISPLAY_SLEEP_PLAY,CM_DISPLAY_SLEEP_UNSUPPORTED};
/* Normal display COPINDEX2 only. Caller owns exclusion. Enter is actual
 * GOSLEEPS bank10/3176; return is XCHTOEND3212 with FACEREG1 and LOCCTR
 * -2 or a seven-core displacement0..72. Resume is PLAYJUM1 bank10/3037.
 * All ordinary caller RAM and io metadata retained except source effects.
 * Unsupported contexts preserve the complete image/io. */
enum cm_display_sleep_boundary cm_makeplay_sleep_enter(apollo_overlay_state *,cm_native_io *);
enum cm_display_sleep_boundary cm_makeplay_sleep_return(apollo_overlay_state *,cm_native_io *);
enum cm_display_sleep_boundary cm_makeplay_sleep_resume(apollo_overlay_state *,cm_native_io *);
#endif
