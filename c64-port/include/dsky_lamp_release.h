/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LAMP_RELEASE_H
#define DSKY_LAMP_RELEASE_H
#include "dsky_lamp_test.h"
#include "dsky_display_release.h"
enum dsky_lamp_release_route {DSKY_LAMP_RELEASE_END,DSKY_LAMP_RELEASE_PIN,DSKY_LAMP_RELEASE_BLANK};
/* Actual TSTLTS4 LM40,3471 / CM40,3376 through real RELDSP/JOBWAKE
 * and selected ENDOFJOB/PINBRNCH/BLNKSUB1 entries. CADRSTOR includes -0:
 * its source skip really executes inline CADR as DAS and falls into
 * UNSUSPEN, then BLNKSUB1. No invented END. Any EB/selectingSUPER/IRQ/raw
 * registers; selected nonempty DSPLIST requires frozen wake domain, empty
 * list does not inspect unrelated Executive words. Producers excluded;
 * unsupported image/IO/events/route unchanged. */
enum p66_bias_status dsky_lamp_release(apollo_overlay_state *,dsky_lamp_io *,enum dsky_lamp_release_route *);
enum exec_status cm_lamp_release(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_lamp_release_route *);
#endif
