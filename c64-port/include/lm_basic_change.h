/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_CHANGE_H
#define LM_BASIC_CHANGE_H
#include "p66_bias.h"
#include "executive.h"
enum lm_basic_change_entry {LM_CHANG1_ENTRY,LM_CHANJOB_ENTRY};
/* Literal CHANG1 or its basic CHANJOB boundary through selected basic DTCB
 * or interpreter NEWOPS. CHANG1 saves canonical Q; CHANJOB receives source
 * callerBB in A and outgoing Q in L, already at EXECBANK02001. No callback,
 * second lifecycle exchange or native IRQ poll; caller owns producers. */
enum exec_status lm_basic_change(apollo_overlay_state *,p66_bias_io *,enum lm_basic_change_entry,exec_transfer *);
#endif
