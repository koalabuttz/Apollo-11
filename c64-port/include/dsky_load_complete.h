/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LOAD_COMPLETE_H
#define DSKY_LOAD_COMPLETE_H
#include "dsky_response_recall.h"
/* Actual LOADLV41,2775 through RELDSP, POSTJUMP/RECALTST and genuine END.
 * Existing source-qualified wakes are composed; no extra scheduler poll. */
enum p66_bias_status dsky_load_complete(apollo_overlay_state *,dsky_lamp_io *);
enum exec_status cm_load_complete(apollo_overlay_state *,cm_native_io *,uint16_t *,uint16_t *);
#endif
