/* SPDX-License-Identifier: MIT */
#ifndef P66_NV_ENTRY_H
#define P66_NV_ENTRY_H
#include "p66_bias.h"
enum p66_nv_boundary {P66_NV_VERB=0,P66_NV_BUSY=1,P66_NV_OUT_OF_SCOPE=2};
/* Fixed V06N60 NVMONOPT source entry from NVDSP. PC04155, Q03121, A01474,
 * L0, bank10/EB7 and callerSUPER0/20/40/60. DSPLOCK0..7 is the source keyboard
 * interlock count. Busy returns actualREST PC03121 without rendering; available
 * ends actualUPDATVB-1 PC02337 bank41/EB1/SUPER100 with native saved NV return.
 * All canonical words authoritative; caller excludes producers. */
enum p66_nv_boundary p66_nv_enter(apollo_overlay_state *,p66_bias_io *);
#endif
