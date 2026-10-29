/* SPDX-License-Identifier: MIT */
#ifndef P66_DIRECT_LAUNCH_H
#define P66_DIRECT_LAUNCH_H
#include "p66_basic.h"
#include "p66_launch.h"
/* Distinct source services for resident disk routing. Existing basic selectors
 * 1/2 are unchanged. STARTP66 retains its qualified NEWJOB0 arithmetic domain;
 * VRTSTART has no interpreter poll and retains a valid pending NEWJOB. */
#define P66_DIRECT_LAUNCH_SERVICE 3u
#define P66_DIRECT_VERTICAL_SERVICE 4u
uint16_t p66_direct_launch_entry(apollo_overlay_state *,p66_bias_io *,uint16_t);
/* Existing arithmetic-first p66_basic_binding; direct source-selected RODCOMP
 * adoption. Requires valid restoring exclusion hooks and mapped RODCOMP. */
enum exec_action p66_direct_launch_callback(executive *,void *,exec_address *);
#define P66_DIRECT_LAUNCH_MAPPING_COUNT 8
extern const exec_context_mapping p66_direct_launch_mappings[P66_DIRECT_LAUNCH_MAPPING_COUNT];
#endif
