/* SPDX-License-Identifier: MIT */
#ifndef P66_DESCENT_INVOKE_H
#define P66_DESCENT_INVOKE_H
#include "p66_guidance_invoke.h"
/* Fixed compiled arithmetic continuations. Internal ABVAL/UNIT returns retain
 * canonical ownership and pending EDOP, with no Executive handoff. */
uint16_t p66_descent_invoke(void *,apollo_overlay_state *,uint16_t selector);
#endif
