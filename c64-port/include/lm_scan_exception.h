/* SPDX-License-Identifier: MIT */
#ifndef LM_SCAN_EXCEPTION_H
#define LM_SCAN_EXCEPTION_H
#include "p66_bias.h"
#include "executive.h"
enum lm_scan_entry {LM_SCAN_PRIORITY,LM_SCAN_SLEEP,LM_SCAN_END,LM_SCAN_DIRECT};
/* Literal entry preparations/EJSCAN through an actual CCSHOLE5705 transfer.
 * Selected ordinary dispatch/idle stays in the separately qualified groups. */
enum exec_status lm_scan_exception(apollo_overlay_state *,p66_bias_io *,enum lm_scan_entry);
#endif
