/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_RELEASE_H
#define DSKY_CHARIN_RELEASE_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
enum dsky_release_route {DSKY_RELEASE_END,DSKY_RELEASE_PINBRANCH,DSKY_RELEASE_BLANK};
/* Actual CHARIN KEYREL031 through VBRELDSP/RELDSP/real JOBWAKE.
 * Reports genuine ENDOFJOB, PINBRNCH or source -0 BLNKSUB1 fallthrough.
 * DSPLOCK0/1; only selected wake paths require qualified Executive contexts.
 * Canonical image, explicit I/O, and restoring producer exclusion. */
enum p66_bias_status dsky_charin_release(apollo_overlay_state *,dsky_charin_io *,enum dsky_release_route *);
enum exec_status cm_charin_release(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11,enum dsky_release_route *);
#endif
