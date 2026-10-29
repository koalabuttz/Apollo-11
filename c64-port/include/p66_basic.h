/* SPDX-License-Identifier: MIT */
#ifndef P66_BASIC_H
#define P66_BASIC_H
#include "p66_native.h"
#include "native_tasks.h"
#include "p66_bias.h"
#include "dsky_display.h"
#define P66_BASIC_MAPPING_COUNT 3u
#define P66_BASIC_TASK_MAPPING_COUNT 2u
enum p66_basic_selector {P66_BASIC_LAUNCH=1,P66_BASIC_MODE_DISPLAY=2};
typedef struct {
 /* Offset zero preserves the arithmetic callback ABI in a combined table. */
 p66_native_binding arithmetic;
 p66_bias_io control;
 wl_native *tasks;
 dsky_display_state *display;
 uint16_t status;
 /* Resident overlay router; each compiled body returns before any swap. */
 uint16_t (*invoke_basic)(void *,apollo_overlay_state *,p66_bias_io *,uint16_t);
 void *basic_modules;
} p66_basic_binding;
/* Source STARTP6631/EB7 SUPER0 or60; actual DSPMMJB40/EB2 SUPER100.
 * Merge these entries with the translated arithmetic table; same wrapper user.
 * Every entry commits the canonical image before lifecycle/scheduling occurs. */
extern const exec_context_mapping p66_basic_mappings[P66_BASIC_MAPPING_COUNT];
extern const wl_mapping p66_basic_task_mappings[P66_BASIC_TASK_MAPPING_COUNT];
/* These are compiled source bodies, not an AGC decoder. Unsupported selector
 * leaves unchanged. A disk build supplies an invoker and can omit linked bodies
 * with P66_BASIC_DISPATCH_ONLY. Startup admission errors stop at real BAILOUT. */
uint16_t p66_basic_entry(apollo_overlay_state *,p66_bias_io *,uint16_t);
/* Per-call transient Executive/Waitlist views are imported from and committed
 * to the one image. Init preserves all source state, pending tasks and jobs.
 * Producer hooks are mandatory, restore prior exclusion state and may nest.
 * No arithmetic table ownership or live overlay pointer is retained here. */
enum exec_status p66_basic_step(p66_basic_binding *);
enum wl_status p66_basic_task_init(p66_basic_binding *,wl_native *,waitlist *);
enum wl_status p66_basic_schedule(p66_basic_binding *,agc_word delay,exec_address task);
enum wl_status p66_basic_advance(p66_basic_binding *,uint16_t centiseconds);
enum wl_status p66_basic_task_step(p66_basic_binding *);
/* Actual DSPOUTSB boundary: caller supplies original CCS NOUT A and Q02070.
 * Only a real channel010 output event changes the native display latches.
 * Full T4RUPT entry/RESUME register restoration is outside this boundary. */
enum p66_bias_status p66_basic_relay(p66_basic_binding *);
#endif
