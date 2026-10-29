/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_SOURCE_DISPATCH_CHECK_H
#define NATIVE_SOURCE_DISPATCH_CHECK_H
#include "p66_bias.h"
#ifndef NATIVE_SOURCE_DISPATCH_CASES
#define NATIVE_SOURCE_DISPATCH_CASES 64u
#endif
uint16_t native_source_dispatch_check(uint8_t service,uint8_t number,uint8_t irq,
 apollo_overlay_state *,apollo_overlay_state *);
uint16_t native_source_dispatch_guards(void);
#endif
