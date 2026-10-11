/* SPDX-License-Identifier: MIT */
#ifndef CM_MAKEPLAY_H
#define CM_MAKEPLAY_H
#include "cm_native_image.h"
enum cm_makeplay_boundary {CM_MAKEPLAY_END=0,CM_MAKEPLAY_PRIORITY=1,CM_MAKEPLAY_WAKE=2,CM_MAKEPLAY_SLEEP=3,CM_MAKEPLAY_PLAY=4,CM_MAKEPLAY_UNSUPPORTED=5,CM_MAKEPLAY_ALARM=01502};
/* Original MAKEPLAY10,2770 to PRIOCHNG5210; normal/clock selection2775
 * through actual JOBWAKE5201, GOSLEEPS3176, END5217 or POODOO5721;
 * source wake return3036 through XCHTOEND to GOPLAY3312.
 * Legal bank10/EB/SUPER pairs, ordinary15-bit erasable caller words and
 * channel11; normal/clock PLAYTEM4 has bits15/7 clear, DSPLOCK0..7,
 * USERPRIO0..037000 upperpriority form. Return LOCCTR-2 or slot0..6*12.
 * The caller owns restoring producer exclusion. Unsupported inputs preserve
 * image and io. No source scheduler poll occurs inside these basic bodies. */
enum cm_makeplay_boundary cm_makeplay_priority(apollo_overlay_state *,cm_native_io *);
enum cm_makeplay_boundary cm_makeplay_select(apollo_overlay_state *,cm_native_io *);
enum cm_makeplay_boundary cm_makeplay_wake_return(apollo_overlay_state *,cm_native_io *);
#endif
