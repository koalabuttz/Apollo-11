/* SPDX-License-Identifier: MIT */
#ifndef CM_CLOCKJOB_H
#define CM_CLOCKJOB_H
#include "cm_native_image.h"
enum cm_clockjob_boundary {CM_CLOCKJOB_END=0,CM_CLOCKJOB_TRIG=1,CM_CLOCKJOB_CHECK=2,CM_CLOCKJOB_NORMAL=3,CM_CLOCKJOB_FLASH=4,CM_CLOCKJOB_UNSUPPORTED=5};
/* Actual selected CLOCKJOB24,3332/E6/SUPER0..60, QUICTRIG final TC A,
 * and CKNVWRD1 through the selected real REGODSP/CLOCPLAY/ENDOFJOB entry.
 * IRQ and sole canonical flight image retain exact raw source state; other
 * external io fields are preserved. The TC A return requires the sign-extended ordinary L word left by QUICTRIG.
 * All non-register erasable inputs are ordinary 15-bit caller words.
 * Caller owns producer exclusion. Each
 * helper returns before another module/service can run. Unsupported inputs
 * preserve both owned objects. QUICTRIG arithmetic is separately qualified. */
enum cm_clockjob_boundary cm_clockjob_enter(apollo_overlay_state *,cm_native_io *);
enum cm_clockjob_boundary cm_clockjob_return(apollo_overlay_state *,cm_native_io *);
enum cm_clockjob_boundary cm_clockjob_select(apollo_overlay_state *,cm_native_io *);
#endif
