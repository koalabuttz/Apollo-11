/* SPDX-License-Identifier: MIT */
#ifndef P66_PENDING_INITIALIZER_H
#define P66_PENDING_INITIALIZER_H
#include "p66_pending_payload.h"
#include "p66_recurrence_fixture.h"
/* Immutable resident function addresses only. The native test constructor
 * returns before the input fixture or any source callback module loads. */
typedef struct {
 const exec_exclusion *guard;
 p66_guidance_modules *math;
 void (*observe)(void *,const apollo_overlay_state *,agc_word);
 uint16_t (*control)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 uint16_t (*display)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 uint16_t (*application)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 uint16_t (*recurrence)(void *,p66_application_recurrence *,exec_address *,uint16_t,uint16_t,uint16_t *);
 uint16_t (*pending)(void *,p66_application_pending *,exec_address *,uint16_t);
} p66_pending_initializer_callbacks;
typedef struct {p66_application_pending *binding;
 const p66_pending_initializer_callbacks *callbacks;} p66_pending_initializer_arguments;
#endif
