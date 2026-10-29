/* SPDX-License-Identifier: MIT */
#ifndef LM_DELAY_IMAGE_H
#define LM_DELAY_IMAGE_H
#include "lm_waitlist_image.h"
enum lm_delay_entry {LM_FIXDELAY_ENTRY,LM_VARDELAY_ENTRY};
/* FIXDELAY/VARDELAY through actual TASKOVER entry or source1203/1204 alarm.
 * fixed_delay is compiled inline metadata; erasable INDEXQ reads canonical input.
 * Task producer owns exclusion; this does not complete TASKOVER or poll jobs. */
enum wl_status lm_delay_image(apollo_overlay_state *,p66_bias_io *,enum lm_delay_entry,agc_word fixed_delay);
/* Literal shared WAIT2 entry, source caller state already in WAITEXIT/WAITADR,
 * A contains original caller BB, L the task BBCON, Q delay. */
enum wl_status lm_waitlist_internal(apollo_overlay_state *,p66_bias_io *);
#endif
