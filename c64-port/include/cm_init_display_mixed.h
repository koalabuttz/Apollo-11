/* SPDX-License-Identifier: MIT */
#ifndef CM_INIT_DISPLAY_MIXED_H
#define CM_INIT_DISPLAY_MIXED_H
#include "cm_priority_complete.h"
/* Explicit original source stages:0 INITDSP3261 through SUPERSW toPRIOCHNG;
 * 1 complete mixed PRIOCHNG;2 genuine3267 BANKJUMP caller return.
 * Source producer exclusion is caller-owned; no selected body is invoked.
 * Stage2 requires ordinary saved callerCADR with low10>=4 so subtract3 stays
 * inside its compiled bank. Unsupported inputs preserve all output objects. */
enum exec_status cm_init_display_mixed_stage(apollo_overlay_state *,cm_native_io *,uint8_t,exec_transfer *);
#endif
