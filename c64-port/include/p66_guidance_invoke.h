/* SPDX-License-Identifier: MIT */
#ifndef P66_GUIDANCE_INVOKE_H
#define P66_GUIDANCE_INVOKE_H
#include "p66_native_guidance.h"
typedef struct {
 uint16_t (*call)(void *,apollo_overlay_state *,uint16_t internal_selector);
 void *user;
} p66_guidance_modules;
/* Resident composition of fixed compiled phases. Each call returns its full
 * C frame before the next module loads; caller exclusion spans this entire
 * function. No Executive exchange occurs at an internal module return. */
uint16_t p66_guidance_invoke(void *,apollo_overlay_state *,uint16_t selector);
#endif
