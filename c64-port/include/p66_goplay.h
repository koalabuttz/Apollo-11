/* SPDX-License-Identifier: MIT */
#ifndef P66_GOPLAY_H
#define P66_GOPLAY_H
#include "p66_nv_render.h"
#include "agc_word.h"
/* Fixed nonflashing V06N60 normal GOPLAY. Entry3070/EB7/bank10 with
 * COPINDEX2, EBANKTEM023400 and NVWORD[2]01474. Ends actual NVMONOPT4155.
 * Finish enters at actual NVSUB return3122 and follows FLASHOFF/COPYTOGO,
 * DOWNFLAG, zero BLANKSUB and source nonflashing ENDJOB branch to5155.
 * Caller owns canonical image, channel11 and producer/IRQ exclusion. */
enum p66_bias_status p66_goplay_enter(apollo_overlay_state *,p66_bias_io *);
enum p66_bias_status p66_goplay_finish(apollo_overlay_state *,p66_bias_io *,agc_word *channel11);
#endif
