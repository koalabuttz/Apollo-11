/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_RUNTIME_H
#define P66_DESCENT_RUNTIME_H
#include "p66_basic.h"
#include "p66_descent_invoke.h"
#define P66_DESCENT_MAPPING_COUNT 107u
enum p66_descent_control_selector {P66_DESCENT_THROTTLE_FETCH=90,
 P66_DESCENT_THROTTLE_SERVICE=91,P66_DESCENT_THROTTLE_RETURN=92,
 P66_DESCENT_DISPLAY_EXIT=93,P66_DESCENT_REGODSPR=94};
typedef struct {
 p66_basic_binding basic;
 agc_word channel14;
 p66_guidance_modules *math_modules;
 /* Display control distinguishes source suppressed=1 from a transport
  * failure. A missing/failed control module returns0xffff. */
 uint16_t (*invoke_control)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 void *control_modules;
 exec_continuation (*resolve_extra)(void *,exec_address,enum exec_context_kind);
 void *extra_registry;
 /* Actual source ENDOFJOB callback; invoked directly after verified DISPEXIT
  * and admission, without an intervening scheduler poll. Optional for bounded
  * observations that stop before END. Production complete bindings install it. */
 exec_continuation finish_source;
 void (*observe_source)(void *,const apollo_overlay_state *,agc_word);
 void *source_observer;
 agc_word prefix_ddv_q;
} p66_descent_binding;
/* The arithmetic/basic wrapper is first, so earlier resident callbacks share
 * this user pointer. Additional real basic mappings (e.g. MAKEPLAY) may be
 * supplied. Storage and metadata are caller owned and must outlive runtime. */
enum exec_status p66_descent_init(exec_native *,executive *,p66_descent_binding *,
 exec_context_mapping *,uint8_t capacity,const exec_context_mapping *extra,uint8_t extra_count);
/* Same explicit entries with a compiled resolver; no RAM registry copy.
 * Optional resolve_extra supplies real downstream basic program callbacks. */
exec_continuation p66_descent_resolve(void *,exec_address,enum exec_context_kind);
enum exec_status p66_descent_init_resolved(exec_native *,executive *,p66_descent_binding *);
#endif
