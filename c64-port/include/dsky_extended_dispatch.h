/* SPDX-License-Identifier: MIT */
#ifndef DSKY_EXTENDED_DISPATCH_H
#define DSKY_EXTENDED_DISPATCH_H
#include "dsky_verb_dispatch.h"
/* Actual VERBFAN return02143→POSTJUMP→GOEXTVB43,2000→selected real
 * LST2FAN body, or direct GOEXTVB entry. MPAC0..59/-0, sourcephysicalbank
 * 41/43 respectively, all EB, SUPER100/120/140/160, IRQ0/1.
 * All60 mission-specific TC/TCF branches compile into explicit native cases;
 * no source instructions are stored/fetched/decoded. Producers excluded.
 * Stops at selected verb body entry, including real VERB69 restart loop. */
enum p66_bias_status dsky_extended_dispatch(apollo_overlay_state *,p66_bias_io *);
enum exec_status cm_extended_dispatch(apollo_overlay_state *,cm_native_io *);
#endif
