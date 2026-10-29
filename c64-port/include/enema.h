/* SPDX-License-Identifier: MIT */
#ifndef ENEMA_H
#define ENEMA_H
#include "restart.h"
typedef struct {
 agc_word flagword10,main_keys,navigation_keys;
 agc_register a,l;
 agc_word q,bbank,superbank,interrupt_enabled;
} enema_io;
enum enema_result { ENEMA_DISPLAY=0, ENEMA_FRESH_START=1,
 ENEMA_OUT_OF_SCOPE=2, ENEMA_BAD_PHASE=01107 };
/* Original ENEMA through cleanup, LIGHTSET, RSFLGBTS and phase agreement;
 * stop BEFORE MMDSPLAY, PTBAD or STARTSUB (fresh start request).
 * Stable normal15-bit tables/channel inputs and distinct owned objects,
 * caller exclusion and stopped counters/IRQs required. Source bank5.
 * Does not display, perform fresh start, admit/execute restart jobs or resume
 * a C stack. Native alarm markers clear by existing rs_cleanup adaptation. */
enum enema_result enema_begin(restart_state *,executive *,waitlist *,
                             const restart_phases *,enema_io *);
#endif
