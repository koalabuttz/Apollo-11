/* SPDX-License-Identifier: MIT */
#ifndef P66_DISPLAY_JOB_H
#define P66_DISPLAY_JOB_H
#include "p66_descent_runtime.h"
#define P66_DISPLAY_JOB_MAPPING_COUNT 60u
enum p66_display_job_selector {
 P66_DISPLAY_PRIORITY_ENTER=300,P66_DISPLAY_PRIORITY_CHANGE=301,
 P66_DISPLAY_SELECT=302,P66_DISPLAY_WAKE=303,P66_DISPLAY_WAKE_RETURN=304,
 P66_DISPLAY_SLEEP_ENTER=305,P66_DISPLAY_SLEEP_RETURN=306,
 P66_DISPLAY_SLEEP=307,P66_DISPLAY_RESUME=308,P66_DISPLAY_GOPLAY=309,
 P66_DISPLAY_NV_ENTER=310,P66_DISPLAY_NV_VERB=311,P66_DISPLAY_NV_NOUN=312,
 P66_DISPLAY_NV_PREPARE=313,P66_DISPLAY_NV_FACTOR=314,P66_DISPLAY_NV_SCALE=315,
 P66_DISPLAY_NV_DECIMAL=316,P66_DISPLAY_NV_NEXT=317,P66_DISPLAY_NV_FINISH=318,
 P66_DISPLAY_REST=319,P66_DISPLAY_BUSY=320,P66_DISPLAY_END=321
};
typedef struct {
 /* Prefix keeps all arithmetic/basic/descent callback user ABIs unchanged. */
 p66_descent_binding descent;
 agc_word channel11;
 uint16_t status;
 uint16_t (*invoke_display)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 void *display_modules;
} p66_display_job_binding;
/* Real admitted MAKEPLAY2546/020067, PLAYJUM1 wake2615 and fixed V06N60
 * consumer source continuations. Append to the descent mapping table. Canonical
 * basic services commit their source dispatch once, then DISPATCHED_BASIC
 * adopts the real next pair without a second exchange or NEWJOB poll.
 * Actual ENDOFJOB5155 executes canonical finish and adopts the alreadyselected
 * basic/interpreter/idle continuation without a second lifecycle. Alarm01206 retains
 * actual POODOO5652 and faults pending native alarm recovery; no placeholder.
 * The full original Pinball DSPTAB/source argument domains apply. */
/* Source END reached inside an already-running basic source continuation:
 * image/io and Executive view are already committed at actual05155. Preserve
 * that source PC/bank, regardless of the older runtime.current callback tuple;
 * execute321 and return actual adoption with no trampoline or extra poll. */
enum exec_action p66_display_job_finish(executive *,void *,exec_address *);
extern const exec_context_mapping p66_display_job_mappings[P66_DISPLAY_JOB_MAPPING_COUNT];
/* Individual overlay bodies. A resident disk build can define
 * P66_DISPLAY_JOB_DISPATCH_ONLY and provide invoke_display instead. */
uint16_t p66_display_job_entry(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
#endif
